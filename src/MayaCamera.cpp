#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Game.h"
#include "MayaCamera.h"
#include "Canvas.h"
#include "Render.h"
#include "Hud.h"
#include "Combat.h"
#include "Player.h"
#include "TinyGL.h"
#include "ScriptThread.h"


MayaCamera::MayaCamera() {
    //printf("MayaCamera::init\n");
    this->keyOffset = 0;
    this->numKeys = 0;
    this->curTween = 0;
    this->cameraThread = nullptr;
    this->keyThread = nullptr;
    this->keyThreadResumeCount = 0;
    this->complete = true;
    this->isTableCam = false;
    this->inheritYaw = false;
    this->inheritPitch = false;
    this->inheritX = false;
    this->inheritY = false;
    this->inheritZ = false;
}

MayaCamera::~MayaCamera() {
}

void MayaCamera::NextKey() {
    Applet* app = CAppContainer::getInstance()->app;

    app->game->activeCameraTime = app->gameTime;
    app->game->activeCameraKey++;
    this->curTween = -1;
    this->curTweenTime = 0;
    this->resetTweenBase(this->keyOffset + app->game->activeCameraKey);
}

void MayaCamera::Update(int i, int i2) {
    Applet* app = CAppContainer::getInstance()->app;
    Combat* combat = app->combat;
    Game* game = app->game;

    int16_t* mayaCameraKeys = game->mayaCameraKeys;
    int8_t* mayaCameraTweens = game->mayaCameraTweens;

    if (this->complete) {
        return;
    }
    this->complete = false;
    int n3 = this->keyOffset + i;
    if (game->cinematicWeapon != -1 && app->time > combat->animEndTime && combat->numActiveMissiles == 0) {
        if (combat->animLoopCount <= 0) {
            game->cinematicWeapon = -1;
        }
        else {
            --combat->animLoopCount;
            combat->animStartTime = app->gameTime;
            combat->animEndTime = combat->animStartTime + combat->animTime;
            combat->flashDone = false;
            combat->flashDoneTime = combat->animStartTime + combat->flashTime;
            combat->nextStageTime = combat->animEndTime;
        }
    }
    if (i2 >= (mayaCameraKeys[game->OFS_MAYAKEY_MS + n3] & 0xFFFF)) {
        if (this->keyThreadResumeCount > 0) {
            this->Snap(i);
        }
        return;
    }
    if (!this->hasTweens(n3)) {
        this->x = mayaCameraKeys[game->OFS_MAYAKEY_X + n3];
        this->y = mayaCameraKeys[game->OFS_MAYAKEY_Y + n3];
        this->z = mayaCameraKeys[game->OFS_MAYAKEY_Z + n3];
        this->pitch = mayaCameraKeys[game->OFS_MAYAKEY_PITCH + n3];
        this->yaw = mayaCameraKeys[game->OFS_MAYAKEY_YAW + n3];
        this->roll = mayaCameraKeys[game->OFS_MAYAKEY_ROLL + n3];
        if (this->x == -2) {
            this->x = game->camPlayerX;
        }
        if (this->y == -2) {
            this->y = game->camPlayerY;
        }
        if (this->z == -2) {
            this->z = game->camPlayerZ;
        }
        this->x <<= 4;
        this->y <<= 4;
        this->z <<= 4;
        if (this->yaw == -2) {
            this->yaw = game->camPlayerYaw;
        }
        if (this->pitch == -2) {
            this->pitch = game->camPlayerPitch;
        }
    }
    else {
        short* array = nullptr;
        int sampleRate = this->sampleRate;
        if (this->curTween == -1) {
            if (i2 >= this->curTweenTime + this->sampleRate) {
                this->curTween = 0;
                this->curTweenTime += this->sampleRate;
                this->updateTweenBase(n3, 0);
            }
            else {
                array = this->getTweenData(mayaCameraTweens, n3, 0);
            }
        }
        if (array == nullptr) {
            int n4 = this->estNumTweens(n3) - 1;
            while (this->curTween != n4 && i2 >= this->curTweenTime + this->sampleRate) {
                this->curTweenTime += this->sampleRate;
                this->updateTweenBase(n3, ++this->curTween);
            }
            if (this->curTween == n4) {
                array = this->getKeyOfs(mayaCameraKeys, n3 + 1);
                sampleRate = (mayaCameraKeys[game->OFS_MAYAKEY_MS + n3] & 0xFFFF) - this->curTweenTime;
            }
            else {
                array = this->getTweenData(mayaCameraTweens, n3, this->curTween + 1);
            }
        }
        this->Interpolate(array, ((i2 - this->curTweenTime) << 16) / sampleRate);
    }
    int n5 = (i2 << 16) / (mayaCameraKeys[game->OFS_MAYAKEY_MS + n3] & 0xFFFF);
    if (this->inheritX && mayaCameraKeys[game->OFS_MAYAKEY_X + n3 + 1] != -2) {
        this->x = ((this->aggComponents[0] << 20) + (n5 * ((mayaCameraKeys[game->OFS_MAYAKEY_X + n3 + 1] - this->aggComponents[0]) << 4)) + 32768) >> 16;
    }
    if (this->inheritY && mayaCameraKeys[game->OFS_MAYAKEY_Y + n3 + 1] != -2) {
        this->y = ((this->aggComponents[1] << 20) + (n5 * ((mayaCameraKeys[game->OFS_MAYAKEY_Y + n3 + 1] - this->aggComponents[1]) << 4)) + 32768) >> 16;
    }
    if (this->inheritZ && mayaCameraKeys[game->OFS_MAYAKEY_Z + n3 + 1] != -2) {
        this->z = ((this->aggComponents[2] << 20) + (n5 * ((mayaCameraKeys[game->OFS_MAYAKEY_Z + n3 + 1] - this->aggComponents[2]) << 4)) + 32768) >> 16;
    }
    if (this->inheritYaw && mayaCameraKeys[game->OFS_MAYAKEY_YAW + n3 + 1] != -2) {
        this->yaw = ((this->aggComponents[4] << 16) + (n5 * this->getAngleDifference(this->aggComponents[4], mayaCameraKeys[game->OFS_MAYAKEY_YAW + n3 + 1])) + 32768) >> 16;
    }
    if (this->inheritPitch && mayaCameraKeys[game->OFS_MAYAKEY_PITCH + n3 + 1] != -2) {
        this->pitch = ((this->aggComponents[3] << 16) + (n5 * this->getAngleDifference(this->aggComponents[3], mayaCameraKeys[game->OFS_MAYAKEY_PITCH + n3 + 1])) + 32768) >> 16;
    }
}

int MayaCamera::getAngleDifference(int i, int i2) {
    if (i2 - i > 512) {
        i2 -= 1024;
    }
    else if (i2 - i < -512) {
        i2 += 1024;
    }
    return i2 - i;
}

bool MayaCamera::hasTweens(int i) {
    Applet* app = CAppContainer::getInstance()->app;

    for (int j = 0; j < 6; j++) {
        int16_t idx = app->game->mayaTweenIndices[i * 6 + j];
        if (idx != -1 && idx != -2) {
            return true;
        }
    }
    return false;
}

int MayaCamera::estNumTweens(int i) {
    Applet* app = CAppContainer::getInstance()->app;
    if (i + 1 == app->game->totalMayaCameraKeys) {
        return 0;
    }
    return ((app->game->mayaCameraKeys[app->game->OFS_MAYAKEY_MS + i] & 0xFFFF) - 1) / this->sampleRate;
}

short* MayaCamera::getTweenData(int8_t* array, int i, int i2) {
    Applet* app = CAppContainer::getInstance()->app;

    short* ofsMayaTween = app->game->ofsMayaTween;
    for (int j = 0; j < 6; j++) {
        short indx = app->game->mayaTweenIndices[i * 6 + j];
        if (indx != -1 && indx != -2) {
            this->keyOfs[j] = array[ofsMayaTween[j] + indx + i2];
        }
        else {
            this->keyOfs[j] = 0;
        }   
    }
    return this->keyOfs;
}

short* MayaCamera::getKeyOfs(int16_t* array, int i) {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->inheritX) {
        this->keyOfs[0] = 0;
    }
    else {
        this->keyOfs[0] = (short)(array[app->game->OFS_MAYAKEY_X + i] - this->aggComponents[0]);
    }
    if (this->inheritY) {
        this->keyOfs[1] = 0;
    }
    else {
        this->keyOfs[1] = (short)(array[app->game->OFS_MAYAKEY_Y + i] - this->aggComponents[1]);
    }
    if (this->inheritZ) {
        this->keyOfs[2] = 0;
    }
    else {
        this->keyOfs[2] = (short)(array[app->game->OFS_MAYAKEY_Z + i] - this->aggComponents[2]);
    }
    if (this->inheritPitch) {
        this->keyOfs[3] = 0;
    }
    else {
        this->keyOfs[3] = (short)this->getAngleDifference(this->aggComponents[3] & 0x3FF, array[app->game->OFS_MAYAKEY_PITCH + i]);
    }
    if (this->inheritYaw) {
        this->keyOfs[4] = 0;
    }
    else {
        this->keyOfs[4] = (short)this->getAngleDifference(this->aggComponents[4] & 0x3FF, array[app->game->OFS_MAYAKEY_YAW + i]);
    }
    this->keyOfs[5] = (short)this->getAngleDifference(this->aggComponents[5] & 0x3FF, array[app->game->OFS_MAYAKEY_ROLL + i]);
    return this->keyOfs;
}

void MayaCamera::Interpolate(int16_t* array, int i) {
    this->pitch = ((this->aggComponents[3] << 16) + (i * array[3]) + 32768) >> 16;
    this->yaw = ((this->aggComponents[4] << 16) + (i * array[4]) + 32768) >> 16;
    this->roll = ((this->aggComponents[5] << 16) + (i * array[5]) + 32768) >> 16;
    this->x = ((this->aggComponents[0] << 20) + (i * (array[0] << 4)) + 32768) >> 16;
    this->y = ((this->aggComponents[1] << 20) + (i * (array[1] << 4)) + 32768) >> 16;
    this->z = ((this->aggComponents[2] << 20) + (i * (array[2] << 4)) + 32768) >> 16;
}

void MayaCamera::resetTweenBase(int i) {
    Applet* app = CAppContainer::getInstance()->app;

    this->aggComponents[0] = app->game->mayaCameraKeys[app->game->OFS_MAYAKEY_X + i];
    this->aggComponents[1] = app->game->mayaCameraKeys[app->game->OFS_MAYAKEY_Y + i];
    this->aggComponents[2] = app->game->mayaCameraKeys[app->game->OFS_MAYAKEY_Z + i];
    this->aggComponents[3] = app->game->mayaCameraKeys[app->game->OFS_MAYAKEY_PITCH + i];
    this->aggComponents[4] = app->game->mayaCameraKeys[app->game->OFS_MAYAKEY_YAW + i];
    this->aggComponents[5] = app->game->mayaCameraKeys[app->game->OFS_MAYAKEY_ROLL + i];

    if (this->aggComponents[0] == -2) {
        this->aggComponents[0] = (short)app->game->camPlayerX;
        this->inheritX = true;
    }
    else {
        this->inheritX = false;
    }
    if (this->aggComponents[1] == -2) {
        this->aggComponents[1] = (short)app->game->camPlayerY;
        this->inheritY = true;
    }
    else {
        this->inheritY = false;
    }
    if (this->aggComponents[2] == -2) {
        this->aggComponents[2] = (short)app->game->camPlayerZ;
        this->inheritZ = true;
    }
    else {
        this->inheritZ = false;
    }
    if (this->aggComponents[3] == -2) {
        this->aggComponents[3] = (short)app->game->camPlayerPitch;
        this->inheritPitch = true;
    }
    else {
        this->inheritPitch = false;
    }
    if (this->aggComponents[4] == -2) {
        this->aggComponents[4] = (short)app->game->camPlayerYaw;
        this->inheritYaw = true;
    }
    else {
        this->inheritYaw = false;
    }
}

void MayaCamera::updateTweenBase(int i, int i2) {
    Applet* app = CAppContainer::getInstance()->app;

    int8_t* mayaCameraTweens = app->game->mayaCameraTweens;
    for (int j = 0; j < 6; ++j) {
        short indx = app->game->mayaTweenIndices[i * 6 + j];
        if (indx != -1 && indx != -2) {
            this->aggComponents[j] += mayaCameraTweens[app->game->ofsMayaTween[j] + indx + i2];
        }
    }
}

void MayaCamera::Render() {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->canvas->state == Canvas::ST_DIALOG) {
        app->render->render(this->x, this->y, this->z, this->yaw, this->pitch, this->roll, 290);
    }
    else {
        app->render->render(this->x, this->y, this->z, this->yaw, this->pitch, this->roll, 315);
    }
    app->render->renderPortal();
    if (app->game->cinematicWeapon != -1) {
        app->combat->drawWeapon(0, 0);
    }

    if (app->hud->cockpitOverlayRaw) {
        app->hud->drawOverlay(&app->canvas->graphics);
    }
    if (app->render->postProcessMode != 0) {
        app->render->postProcessView(&app->canvas->graphics);
    }

    app->canvas->repaintFlags |= Canvas::REPAINT_VIEW3D;
}

void MayaCamera::Snap(int i) {
    Applet* app = CAppContainer::getInstance()->app;
    Canvas* canvas = app->canvas;
    Game* game = app->game;

    int16_t* mayaCameraKeys = game->mayaCameraKeys;
    if (this->complete) {
        return;
    }
    int n2 = this->keyOffset + i;
    if (n2 + 1 < this->keyOffset + this->numKeys) {
        int n3 = n2 + 1;
        this->x = mayaCameraKeys[game->OFS_MAYAKEY_X + n3] << 4;
        this->y = mayaCameraKeys[game->OFS_MAYAKEY_Y + n3] << 4;
        this->z = mayaCameraKeys[game->OFS_MAYAKEY_Z + n3] << 4;
        this->pitch = mayaCameraKeys[game->OFS_MAYAKEY_PITCH + n3];
        this->yaw = mayaCameraKeys[game->OFS_MAYAKEY_YAW + n3];
        this->roll = mayaCameraKeys[game->OFS_MAYAKEY_ROLL + n3];
        if (this->inheritX && mayaCameraKeys[game->OFS_MAYAKEY_X + n3] == -2) {
            this->x = game->camPlayerX << 4;
        }
        if (this->inheritY && mayaCameraKeys[game->OFS_MAYAKEY_Y + n3] == -2) {
            this->y = game->camPlayerY << 4;
        }
        if (this->inheritZ && mayaCameraKeys[game->OFS_MAYAKEY_Z + n3] == -2) {
            this->z = game->camPlayerZ << 4;
        }
        if (this->inheritPitch && mayaCameraKeys[game->OFS_MAYAKEY_PITCH + n3] == -2) {
            this->pitch = (short)game->camPlayerPitch;
        }
        if (this->inheritYaw && mayaCameraKeys[game->OFS_MAYAKEY_YAW + n3] == -2) {
            this->yaw = (short)game->camPlayerYaw;
        }
        if (this->keyThread) {
            if (--this->keyThreadResumeCount == 0) {
                ScriptThread* keyThread = this->keyThread;
                this->keyThread = nullptr;
                keyThread->run();
                return;
            }
        }

        if (this->keyThread && this->keyThreadResumeCount > 0) {
            this->NextKey();
        }
        else if (this->isTableCam) {
            this->NextKey();
        }
        return;
    }
    this->complete = true;
    game->skippingCinematic = false;
    game->cinUnpauseTime = 0;
    game->activeCameraView = false;
    if (canvas->state != Canvas::ST_CAMERA/*&& canvas->state != Canvas::ST_INTER_CAMERA*/) {
        return;
    }
    app->tinyGL->resetViewPort();
    canvas->setState(Canvas::ST_PLAYING);
    canvas->updateFacingEntity = true;
    if (app->player->unsetFamiliarOnceOutOfCinematic) {
        app->player->unsetFamiliarOnceOutOfCinematic = false;
        app->player->forceFamiliarReturnDueToMonster();
    }
    if (this->keyThread) {
        ScriptThread* keyThread = this->keyThread;
        this->keyThread = nullptr;
        keyThread->run();
    }
    this->cameraThread = nullptr;
    canvas->viewPitch = (canvas->destPitch = 0);
    canvas->startRotation(true);
}
