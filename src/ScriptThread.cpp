#include <stdexcept>

#include "CAppContainer.h"
#include "ScriptThread.h"
#include "JavaStream.h"
#include "Game.h"
#include "Render.h"
#include "Canvas.h"
#include "Enums.h"
#include "MayaCamera.h"
#include "Text.h"
#include "Player.h"
#include "Hud.h"
#include "Entity.h"
#include "EntityDef.h"
#include "Combat.h"
#include "ParticleSystem.h"
#include "MenuSystem.h"
#include "SentryBotGame.h"
#include "VendingMachine.h"
#include "HackingGame.h"
#include "Sound.h"

ScriptThread::ScriptThread() {
}

ScriptThread::~ScriptThread() {
}

void ScriptThread::saveState(OutputStream* OS) {
    if (this->unpauseTime == -1 || this->unpauseTime == 0) {
        OS->writeInt(this->unpauseTime);
    }
    else {
        OS->writeInt(this->unpauseTime - this->app->gameTime);
    }
    OS->writeByte((uint8_t)this->state);
    OS->writeInt(this->IP);
    OS->writeInt(this->FP);
    for (int i = 0; i < this->FP; i++) {
        OS->writeInt(this->scriptStack[i]);
    }
}

void ScriptThread::loadState(InputStream* IS) {
    this->init();
    this->unpauseTime = IS->readInt();
    if (this->unpauseTime != 0 && this->unpauseTime != -1) {
        this->unpauseTime += this->app->gameTime;
    }
    this->state = IS->readByte();
    this->IP = IS->readInt();
    this->FP = IS->readInt();
    for (int i = 0; i < this->FP; i++) {
        this->scriptStack[i] = IS->readInt();
    }
}

uint32_t ScriptThread::executeTile(int x, int y, int flags, bool b) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) {
        this->state = 0;
        return 0;
    }
    this->app->game->skipAdvanceTurn = false;
    int n4 = (y * 32) + x;
    if ((this->app->render->mapFlags[n4] & 0x40) != 0x0) {
        int run = 0;
        for (int i = this->app->render->findEventIndex(n4); i != -1; i = this->app->render->getNextEventIndex()) {
            //printf("ExecuteTile: (%d, %d), tileEvent[%d] = %x;\n", x, y, i + 1, this->app->render->tileEvents[i + 1]);
            int n5 = this->app->render->tileEvents[i + 1];
            int n6 = n5 & flags;

            //printf("ExecuteTile: (%d, %d), tileEvent[%d] = %x, f1= %x, f2= %x;\n", x, y, i + 1, this->app->render->tileEvents[i + 1], (n5 & 0xF), (n5 & 0xFF0));

            if ((n5 & 0x80000) == 0x0 && (n6 & 0xF) != 0x0 && (n6 & 0xFF0) != 0x0 && (((n5 & 0x7000) == 0x0 && (flags & 0x7000) == 0x0) || (n6 & 0x7000) != 0x0)) {
                if ((n5 & 0x40000) != 0x0) {
                    this->app->game->skipAdvanceTurn = true;
                    this->app->game->queueAdvanceTurn = false;
                }
                //printf("ExecuteTile Alloc: (%d, %d), tileEvent[%d] = %x;\n", x, y, i, this->app->render->tileEvents[i]);
                this->alloc(i, flags, b);
                run = this->run();
            }
        }
        return run;
    }
    this->state = 0;
    return 0;
}

int ScriptThread::queueTile(int x, int y, int flags) {
    return this->queueTile(x, y, flags, false);
}

int ScriptThread::queueTile(int x, int y, int flags, bool b) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) {
        return this->state = 0;
    }
    this->app->game->skipAdvanceTurn = false;
    int n4 = (y * 32) + x;
    if ((this->app->render->mapFlags[n4] & 0x40) != 0x0) {
        for (int i = this->app->render->findEventIndex(n4); i != -1; i = this->app->render->getNextEventIndex()) {
            int n5 = this->app->render->tileEvents[i + 1];
            int n6 = n5 & flags;
            if ((n5 & 0x80000) == 0x0 && (n6 & 0xF) != 0x0 && (n6 & 0xFF0) != 0x0 && (((n5 & 0x7000) == 0x0 && (flags & 0x7000) == 0x0) || (n6 & 0x7000) != 0x0)) {
                if ((n5 & 0x40000) != 0x0) {
                    this->app->game->skipAdvanceTurn = true;
                    this->app->game->queueAdvanceTurn = false;
                }
                this->alloc(i, flags, b);
                this->flags |= 0x2;
                return 2;
            }
        }
    }
    this->state = 0;
    return 0;
}

int ScriptThread::evWait(int time) {
    if (this->app->game->skippingCinematic) {
        return 1;
    }
    this->unpauseTime = this->app->gameTime + time;
    if ((this->flags & 0x1) != 0x0) {
        if (this->app->canvas->state != Canvas::ST_CAMERA) {
            this->app->canvas->blockInputTime = this->unpauseTime;
        }
        if (this->app->canvas->state == Canvas::ST_AUTOMAP || this->app->canvas->state == Canvas::ST_MENU) {
            this->app->canvas->setState(Canvas::ST_PLAYING);
        }
        if (this->app->canvas->state == Canvas::ST_PLAYING) {
            this->app->canvas->clearSoftKeys();
        }
    }
    return 2;
}

bool ScriptThread::evReturn() {
    while (this->FP < this->stackPtr - 2) {
        this->pop();
    }
    this->FP = this->pop();
    int IP = this->pop();
    if (IP != -1) {
        this->IP = IP;
        return false;
    }
    else if (this->stackPtr != 0) {
        this->app->Error("The frame pointer should be zero if the script has completed. %i", Enums::ERR_SCRIPTTHREAD_FREE);
    }
    return true;
}

void ScriptThread::alloc(int n, int type, bool b) {
    this->IP = (this->app->render->tileEvents[n] & (int)0xFFFF0000) >> 16;
    //printf("alloc : this->IP %d\n", this->IP);
    this->FP = 0;
    this->stackPtr = 0;
    this->push(-1);
    this->push(0);
    this->type = type;
    this->flags = 0;
    if (b) {
        this->flags = 1;
    }
}

void ScriptThread::alloc(int ip) {
    this->IP = ip;
    this->FP = 0;
    this->stackPtr = 0;
    this->push(-1);
    this->push(0);
    this->type = 0;
    this->flags = 1;
}

int ScriptThread::peekNextCmd() {
    return this->app->render->mapByteCode[this->IP + 1];
}

void ScriptThread::setupCamera(int n) {
    Game* game = this->app->game;
    game->cinUnpauseTime = this->app->gameTime + 1000;
    game->activeCameraView = true;
    MayaCamera* activeCamera = &game->mayaCameras[n];
    game->activeCameraKey = -1;
    game->activeCamera = activeCamera;
    activeCamera->complete = false;
    game->activeCameraTime = this->app->gameTime;
    game->camPlayerX = this->app->canvas->destX << 0;
    game->camPlayerY = this->app->canvas->destY << 0;
    game->camPlayerZ = this->app->canvas->destZ << 0;
    game->camPlayerYaw = (this->app->canvas->destAngle & 0x3FF);
    game->camPlayerPitch = this->app->canvas->viewPitch;

    activeCamera->x = game->mayaCameraKeys[game->OFS_MAYAKEY_X + activeCamera->keyOffset];
    if (activeCamera->x == -2) {
        activeCamera->x = game->camPlayerX;
    }

    activeCamera->y = game->mayaCameraKeys[game->OFS_MAYAKEY_Y + activeCamera->keyOffset];
    if (activeCamera->y == -2) {
        activeCamera->y = game->camPlayerY;
    }

    activeCamera->z = game->mayaCameraKeys[game->OFS_MAYAKEY_Z + activeCamera->keyOffset];
    if (activeCamera->z == -2) {
        activeCamera->z = game->camPlayerZ;
    }

    activeCamera->x <<= 4;
    activeCamera->y <<= 4;
    activeCamera->z <<= 4;

    activeCamera->pitch = game->mayaCameraKeys[game->OFS_MAYAKEY_PITCH + activeCamera->keyOffset];
    if (activeCamera->pitch == -2) {
        activeCamera->pitch = game->camPlayerPitch;
    }

    activeCamera->yaw = game->mayaCameraKeys[game->OFS_MAYAKEY_YAW + activeCamera->keyOffset];
    if (activeCamera->yaw == -2) {
        activeCamera->yaw = game->camPlayerYaw;
    }

    activeCamera->roll = game->mayaCameraKeys[game->OFS_MAYAKEY_ROLL + activeCamera->keyOffset];
}

uint32_t ScriptThread::run() {
    this->app->game->updateScriptVars();
    if (this->stackPtr == 0) {
        return 1;
    }
    int n = 1;
    uint8_t* mapByteCode = this->app->render->mapByteCode;
    //printf("mapByteCodeSize -> %d\n", this->app->render->mapByteCodeSize);
    while (this->IP < this->app->render->mapByteCodeSize && n != 2) {
        bool b = true;
        short n2 = 0;

        //printf("mapByteCode[%d] [%d]\n", this->IP, mapByteCode[this->IP]);
        switch (mapByteCode[this->IP]) {
            case Enums::EV_EVAL: {
                //printf("EV_EVAL -> %d\n", this->IP);

                short n3 = (short)this->getByteArg();
                b = false;
                while (true) {
                    if (--n3 < 0) {
                        break;
                    }
                    short uByteArg = this->getUByteArg();
                    //printf("uByteArg %d\n", uByteArg);
                    if ((uByteArg & Enums::EVAL_VARFLAG) != 0x0) {
                        //printf("this->app->game->scriptStateVars[%d] %d\n", uByteArg & Enums::EVAL_VARMASK, this->app->game->scriptStateVars[uByteArg & Enums::EVAL_VARMASK]);
                        this->push(this->app->game->scriptStateVars[uByteArg & Enums::EVAL_VARMASK]);
                    }
                    else if ((uByteArg & Enums::EVAL_CONSTFLAG) != 0x0) {
                        int args = this->getUByteArg();
                        int args2 = ((((uByteArg & Enums::EVAL_CONSTMASK) << 8) | args) << 18) >> 18;
                        //printf("args2 %d\n", args2);
                        this->push(args2);
                    }
                    else {
                        switch (uByteArg) {
                            case Enums::EVAL_AND: { // and
                                this->push(this->pop() == 1 && this->pop() == 1);
                                break;
                            }
                            case Enums::EVAL_OR: { // or
                                this->push(this->pop() == 1 || this->pop() == 1);
                                break;
                            }
                            case Enums::EVAL_LTE: { // less than or equal to
                                int a = this->pop();
                                int b = this->pop();
                                this->push((b <= a));
                                break;
                            }
                            case Enums::EVAL_LT: { // less than
                                int a = this->pop();
                                int b = this->pop();
                                this->push((b < a));
                                break;
                            }
                            case Enums::EVAL_EQ: { // equal to
                                int a = this->pop();
                                int b = this->pop();
                                this->push((a == b));
                                break;
                            }
                            case Enums::EVAL_NEQ: { // not equal to	
                                int a = this->pop();
                                int b = this->pop();
                                this->push((a != b));
                                break;
                            }
                            case Enums::EVAL_NOT: { // negation
                                this->push((this->pop() == 0) ? 1 : 0);
                                break;
                            }
                        }
                    }
                }
                short uByteArg2 = this->getUByteArg();
                if (this->pop() == 0) {
                    this->IP += uByteArg2;
                    break;
                }
                break;
            }

            case Enums::EV_JUMP: {
                //printf("EV_JUMP -> %d\n", this->IP);
                b = false;
                this->IP += this->getUShortArg();
                break;
            }

            case Enums::EV_RETURN: {
                //printf("EV_RETURN -> %d\n", this->IP);
                if (this->evReturn()) {
                    return 1;
                }
                break;
            }

            case Enums::EV_MESSAGE: {
                //printf("EV_MESSAGE -> %d\n", this->IP);
                int uShortArg = this->getUShortArg();
                short n4 = (short)(uShortArg & 0x7FFF);
                if ((uShortArg & 0x8000) >> 15 == 1) {
                    this->app->hud->addMessage(this->app->canvas->loadMapStringID, n4, 3);
                    break;
                }
                if (this->app->canvas->state == Canvas::ST_CAMERA) {
                    this->app->hud->msgCount = 0;
                }
                this->app->hud->addMessage(this->app->canvas->loadMapStringID, n4);
                break;
            }

            case Enums::EV_LERPSPRITE: {
                //printf("EV_LERPSPRITE -> %d\n", this->IP);
                int args = this->getUByteArg() | this->getUByteArg() << 8 | this->getUByteArg() << 16;
                int sprite = (args >> 14) & 0xFF;
                int dstX = (args >> 9) & 0x1F;
                int dstY = (args >> 4) & 0x1F;
                int flags = args & 0xF;
                int dstZ = (!(flags & Enums::SCRIPT_LS_DEFAULT_Z)) ? (this->getUByteArg() - 48) : 32;
                int time = (!(flags & Enums::SCRIPT_LS_NO_TIME)) ? (this->getUByteArg() * 100) : 0;
                if (time != 0 && (flags & Enums::SCRIPT_LS_FLAG_BLOCK) != 0x0) {
                    this->evWait(time);
                }
                bool aSync = (flags & Enums::SCRIPT_LS_FLAG_ASYNC) != 0x0;
                LerpSprite* lerpSprite = this->app->game->allocLerpSprite(this, sprite, (flags & Enums::SCRIPT_LS_FLAG_BLOCK) != 0x0);
                if (lerpSprite == nullptr) {
                    return 0;
                }
                lerpSprite->dstX = 32 + (dstX << 6);
                lerpSprite->dstY = 32 + (dstY << 6);
                short sEnt = this->app->render->mapSprites[this->app->render->S_ENT + sprite];
                if (sEnt != -1) {
                    this->app->game->entities[sEnt].info |= 0x400000;
                    if (this->app->game->entities[sEnt].monster != nullptr) {
                        this->app->game->entities[sEnt].monster->flags |= 0x4000;
                    }
                }
                lerpSprite->dstZ = this->app->render->getHeight(lerpSprite->dstX, lerpSprite->dstY) + dstZ;
                lerpSprite->srcX = this->app->render->mapSprites[this->app->render->S_X + sprite];
                lerpSprite->srcY = this->app->render->mapSprites[this->app->render->S_Y + sprite];
                lerpSprite->srcZ = this->app->render->mapSprites[this->app->render->S_Z + sprite];
                lerpSprite->srcScale = lerpSprite->dstScale = this->app->render->mapSprites[this->app->render->S_SCALEFACTOR + sprite];
                lerpSprite->startTime = app->gameTime;
                lerpSprite->travelTime = time;
                lerpSprite->flags = (flags & Enums::SCRIPT_LS_FLAG_ASYNC_BLOCK);
                lerpSprite->calcDist();
                if (time == 0) {
                    if ((this->app->game->updateLerpSprite(lerpSprite) & 0x1) != 0x0) {
                        this->app->canvas->invalidateRect();
                        break;
                    }
                    n = 0;
                    break;
                }
                else {
                    if (!aSync) {
                        this->app->game->skipAdvanceTurn = true;
                        this->app->game->queueAdvanceTurn = false;
                        this->unpauseTime = -1;
                        n = 2;
                        break;
                    }
                    break;
                }
                break;
            }

            case Enums::EV_STARTCINEMATIC: {
                //printf("EV_STARTCINEMATIC -> %d\n", this->IP);
                uint8_t byteArg = this->getByteArg();
                app->player->forceRemoveFromScopeZoom();
                this->setupCamera(byteArg);
                this->app->game->activeCamera->cameraThread = this;
                if (this->app->canvas->state != Canvas::ST_MENU && this->app->canvas->state != Canvas::ST_CAMERA) {
                    this->app->canvas->setState(Canvas::ST_CAMERA);
                }
                this->app->game->skipAdvanceTurn = true;
                this->app->game->queueAdvanceTurn = false;
                break;
            }

            case Enums::EV_SETSTATE: {
                //printf("EV_SETSTATE -> %d\n", this->IP);
                int index = this->getByteArg();
                short args = this->getShortArg();
                this->app->game->scriptStateVars[index] = args;
                break;
            }

            case Enums::EV_CALL_FUNC: {
                //printf("EV_CALL_FUNC -> %d\n", this->IP);
                int ip = this->getUShortArg();
                int fp = this->FP;
                this->FP = this->stackPtr;
                this->push(this->IP);
                this->push(fp);
                this->IP = ip - 1;
                break;
            }

            case Enums::EV_ITEM_COUNT: {
                //printf("EV_ITEM_COUNT -> %d\n", this->IP);
                int uShortArg3 = this->getUShortArg();
                int n21 = uShortArg3 & 0x1F;
                int n22 = (uShortArg3 & 0x3E0) >> 5;
                short n23 = 0;
                short index = this->getByteArg();
                if (n21 == 0) {
                    n23 = this->app->player->inventory[n22];
                }
                else if (n21 == 1) {
                    if ((this->app->player->weapons & 1 << n22) != 0x0) {
                        n23 = 1;
                    }
                }
                else if (n21 == 2) {
                    n23 = this->app->player->ammo[n22];
                }
                this->app->game->scriptStateVars[index] = n23;
                //printf("EventItemCount(%d, %d, %d) = %d;\n", n21, n22, index, n23);
                break;
            }

            case Enums::EV_TILE_EMPTY: {
                //printf("EV_TILE_EMPTY -> %d\n", this->IP);
                int args = this->getUShortArg();
                int srcX = args & 0x1F;
                int srcY = (args >> 5) & 0x1F;
                int index = this->getByteArg();
                short n26 = 1;
                Entity* entity = app->game->entityDb[srcX + (32 * srcY)];
                if (entity != nullptr) {
                    while (entity != nullptr) {
                        if (entity->def->eType != 12 && (1 << entity->def->eType & 0x6240) == 0x0) {
                            n26 = 0;
                            break;
                        }
                        entity = entity->nextOnTile;
                    }
                }
                app->game->scriptStateVars[index] = n26;
                break;
            }

            case Enums::EV_WEAPON_EQUIPPED: {
                //printf("EV_WEAPON_EQUIPPED -> %d\n", this->IP);
                int index = this->getByteArg() & 0x7F;
                app->game->scriptStateVars[index] = (short)app->player->ce->weapon;
                break;
            }

            case Enums::EV_CHANGE_MAP: {
                //printf("EV_CHANGE_MAP -> %d\n", this->IP);
                int uByteArg4 = this->getUByteArg();
                int uShortArg5 = this->getUShortArg();
                bool showStats = true;
                app->player->completedLevels |= 1 << (short)(app->canvas->loadMapID - 1);
                app->game->spawnParam = ((((uByteArg4 >> 4) & 0x7) << 10) | (uShortArg5 & 0x3FF));
                app->menuSystem->LEVEL_STATS_nextMap = (short)(uByteArg4 & 0xF);
                if (app->menuSystem->LEVEL_STATS_nextMap < app->canvas->loadMapID) {
                    showStats = false;
                }
                app->game->snapAllMovers();
                if ((uByteArg4 & 0x80) != 0x0) {
                    int fadeFlags = Render::FADE_FLAG_FADEOUT;
                    if (showStats) {
                        fadeFlags |= Render::FADE_FLAG_SHOWSTATS;
                    }
                    else {
                        fadeFlags |= Render::FADE_FLAG_CHANGEMAP;
                    }
                    if (app->canvas->state == Canvas::ST_AUTOMAP) {
                        app->canvas->setState(Canvas::ST_PLAYING);
                    }
                    app->render->startFade(1000, fadeFlags);
                }
                else if (showStats) {
                    app->canvas->saveState(51, (short)3, (short)194);
                }
                else {
                    app->canvas->loadMap(app->menuSystem->LEVEL_STATS_nextMap, false, false);
                }
                app->canvas->changeMapStarted = true;
                break;
            }

            case Enums::EV_CAMERA_STR: {
                //printf("EV_CAMERA_STR -> %d\n", this->IP);
                int uShortArg6 = this->getUShortArg();
                int time = this->getUShortArg();
                if (!this->app->game->skippingCinematic) {
                    short n29 = (short)(uShortArg6 & 0x3FFF);
                    int n30 = (uShortArg6 & 0x8000) >> 15;
                    this->app->hud->showCinPlayer = ((uShortArg6 & 0x4000) != 0x0);
                    if (n30 == 1) {
                        this->app->hud->showCinPlayer = false;
                    }
                    if (n30 == 0) {
                        this->app->hud->subTitleID = Localization::STRINGID(this->app->canvas->loadMapStringID, n29);
                        this->app->hud->subTitleTime = app->gameTime + time;
                    }
                    else {
                        this->app->hud->cinTitleID = Localization::STRINGID(this->app->canvas->loadMapStringID, n29);
                        this->app->hud->cinTitleTime = app->gameTime + time;
                    }
                    this->app->canvas->repaintFlags |= Canvas::REPAINT_HUD;
                    this->app->hud->repaintFlags = 16;
                    break;
                }
                break;
            }

            case Enums::EV_DIALOG: {
                //printf("EV_DIALOG -> %d\n", this->IP);
                short uByteArg5 = this->getUByteArg();
                short uByteArg6 = this->getUByteArg();
                int n31 = uByteArg6 >> 4;
                int n32 = uByteArg6 & 0xF;
                if (this->app->canvas->state == Canvas::ST_AUTOMAP) {
                    this->app->canvas->setState(Canvas::ST_PLAYING);
                    this->app->canvas->invalidateRect();
                }
                if (this->app->game->skipDialog) {
                    break;
                }
                if (n32 == 6 || n32 == 7 || n32 == 1) {
                    this->app->player->inCombat = false;
                }
                if (n32 == 2) {
                    bool enqueueHelpDialog = false;
                    this->app->player->prevWeapon = this->app->player->ce->weapon;
                    for (uint8_t b4 = 0; b4 < 20; ++b4) {
                        if (&this->app->game->scriptThreads[b4] == this) {
                            enqueueHelpDialog = this->app->canvas->enqueueHelpDialog(this->app->canvas->loadMapStringID, uByteArg5, b4);
                            break;
                        }
                    }
                    if (!enqueueHelpDialog) {
                        break;
                    }
                }
                else {
                    if (n32 == 4) {
                        this->app->player->prevWeapon = this->app->player->ce->weapon;
                    }
                    this->app->canvas->startDialog(this, this->app->canvas->loadMapStringID, uByteArg5, n32, n31, true);
                }
                this->app->game->skipAdvanceTurn = true;
                this->app->game->queueAdvanceTurn = false;
                this->unpauseTime = -1;
                n = 2;
                break;
            }

            case Enums::EV_WAIT: {
                //printf("EV_WAIT -> %d\n", this->IP);
                n = this->evWait(this->getUByteArg() * 100);
                break;
            }

            case Enums::EV_GOTO: {
                //printf("EV_GOTO -> %d\n", this->IP);
                if (this->app->game->interpolatingMonsters) {
                    this->app->game->snapMonsters(true);
                }
                while (this->app->game->combatMonsters != nullptr) {
                    this->app->game->combatMonsters->undoAttack();
                }
                this->app->game->endMonstersTurn();
                int args = this->getUShortArg();
                bool b5 = (args & 0x4000) != 0x0;
                this->app->canvas->destX = (((args >> 5) & 0x1F) << 6) + 32;
                this->app->canvas->destY = ((args & 0x1F) << 6) + 32;
                this->app->canvas->destZ = this->app->render->getHeight(this->app->canvas->destX, this->app->canvas->destY) + 36;
                int n33 = (args >> 10) & 0xF;
                this->app->canvas->viewPitch = (this->app->canvas->destPitch = 0);
                this->app->canvas->viewRoll = (this->app->canvas->destRoll = 0);
                this->app->canvas->knockbackDist = 0;
                if (b5) {
                    if (n33 != 15) {
                        int viewAngle = this->app->canvas->viewAngle & 0x3FF;
                        int destAngle = n33 << 7;
                        if (destAngle - viewAngle > 512) {
                            destAngle -= 1024;
                        }
                        else if (destAngle - viewAngle < -512) {
                            destAngle += 1024;
                        }
                        this->app->canvas->viewAngle = viewAngle;
                        this->app->canvas->destAngle = destAngle;
                    }
                    this->app->canvas->startRotation(false);
                    this->app->canvas->zStep = (std::abs(this->app->canvas->destZ - this->app->canvas->viewZ) + this->app->canvas->animFrames - 1) / this->app->canvas->animFrames;
                    if (this->app->canvas->destX != this->app->canvas->viewX || this->app->canvas->destY != this->app->canvas->viewY || this->app->canvas->viewAngle != this->app->canvas->destAngle) {
                        this->app->canvas->gotoThread = this;
                        this->unpauseTime = -1;
                        n = 2;
                    }
                    else {
                        this->app->canvas->viewPitch = this->app->canvas->destPitch;
                    }
                }
                else {
                    this->app->canvas->viewX = this->app->canvas->destX;
                    this->app->canvas->viewY = this->app->canvas->destY;
                    this->app->canvas->viewZ = this->app->canvas->destZ;
                    if (n33 != 15) {
                        this->app->canvas->destAngle = (this->app->canvas->viewAngle = n33 << 7);
                        this->app->canvas->finishRotation(true);
                    }
                    if ((args & 0x8000) != 0x0) {
                        this->app->game->advanceTurn();
                    }
                    if (this->app->canvas->state != Canvas::ST_CAMERA) {
                        this->app->canvas->startRotation(false);
                        this->app->canvas->viewPitch = this->app->canvas->destPitch;
                    }
                    else {
                        this->app->canvas->viewPitch = (this->app->canvas->destPitch = 0);
                    }
                    this->app->game->gotoTriggered = true;
                    this->app->canvas->automapDrawn = false;
                }
                this->app->player->relink();
                this->app->canvas->clearEvents(1);
                this->app->canvas->updateFacingEntity = true;
                this->app->canvas->invalidateRect();
                break;
            }

            case Enums::EV_ABORT_MOVE: {
                //printf("EV_ABORT_MOVE -> %d\n", this->IP);
                this->app->canvas->abortMove = true;
                break;
            }

            case Enums::EV_ENTITY_FRAME: {
                //printf("EV_ENTITY_FRAME -> %d\n", this->IP);
                short sprite = this->getUByteArg();
                short args = this->getUByteArg();
                int time = this->getUByteArg() * 100;
                short sEnt = this->app->render->mapSprites[this->app->render->S_ENT + sprite];
                this->app->render->mapSpriteInfo[sprite] = ((this->app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | (args << 8));
                this->app->canvas->staleView = true;
                if (sEnt != -1) {
                    this->app->game->entities[sEnt].info |= 0x400000;
                    if (this->app->game->entities[sEnt].monster != nullptr) {
                        this->app->game->entities[sEnt].monster->frameTime = 0x7FFFFFFF;
                    }
                }
                if (time > 0) {
                    n = this->evWait(time);
                    break;
                }
                break;
            }

            case Enums::EV_ADV_CAMERAKEY: {
                //printf("EV_ADV_CAMERAKEY -> %d\n", this->IP);
                if (this->app->canvas->state == Canvas::ST_CAMERA || this->app->canvas->state == Canvas::ST_INTER_CAMERA) {
                    this->app->game->activeCamera->keyThreadResumeCount = this->getUByteArg();
                    this->app->game->activeCamera->keyThread = this;
                    this->app->game->activeCamera->NextKey();
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                this->getUByteArg();
                break;
            }

            case Enums::EV_DAMAGEMONSTER: {
                //printf("EV_DAMAGEMONSTER -> %d\n", this->IP);
                short sprite = this->getUByteArg();
                int8_t dmgVal = this->getByteArg();
                short sEnt = this->app->render->mapSprites[this->app->render->S_ENT + sprite];
                if (sEnt != -1) {
                    Entity* entity = &this->app->game->entities[sEnt];
                    if (entity->monster != nullptr) {
                        entity->info |= 0x20000;
                        entity->pain(dmgVal, nullptr);
                        if (entity->monster->ce.getStat(Enums::STAT_HEALTH) <= 0) {
                            entity->died(false, nullptr);
                        }
                    }
                    else {
                        entity->died(false, nullptr);
                    }
                }
                this->app->canvas->staleView = true;
                break;
            }

            case Enums::EV_DAMAGEPLAYER: {
                //printf("EV_DAMAGEPLAYER -> %d\n", this->IP);
                int8_t dmgVal = this->getByteArg();
                int8_t dmgArm = this->getByteArg();
                int8_t dmgAng = this->getByteArg();
                if (dmgVal > 0) {
                    this->app->player->painEvent(nullptr, false);
                    this->app->hud->damageTime = this->app->time + 1000;
                    if (dmgAng != -1) {
                        this->app->hud->damageDir = (((uint8_t)dmgAng) + (256 - (this->app->canvas->viewAngle & 0x3FF) >> 7) + 1 & 0x7);
                    }
                    this->app->combat->totalDamage = 1;
                    this->app->player->pain(dmgVal, nullptr, true);
                }
                else if (dmgVal < 0) {
                    this->app->player->addHealth(-dmgVal);
                }
                this->app->player->addArmor(-dmgArm);
                break;
            }

            case Enums::EV_DOOROP: {
                //printf("EV_DOOROP -> %d\n", this->IP);
                int args = this->getUShortArg();
                int n42 = args >> 10;
                int n43 = ((n42 & 0x4) == 0x0 && this->app->canvas->state != Canvas::ST_AUTOMAP) ? 1 : 0;
                int n44 = n42 & 0x3;
                short sEnt = this->app->render->mapSprites[this->app->render->S_ENT + (args & 0x3FF)];
                if (sEnt != -1) {
                    Entity* entity = &this->app->game->entities[sEnt];
                    if (n44 == 1 || n44 == 0) {
                        if (n44 == 1 && entity->def->eType == Enums::ET_DOOR) {
                            this->app->game->setLineLocked(entity, false);
                        }
                        if (this->app->game->performDoorEvent(n44, (n43 != 0) ? this : nullptr, entity, n43, false) && n43) {
                            this->unpauseTime = -1;
                            n = 2;
                        }
                    }
                    else if (n44 == 2) {
                        this->app->game->setLineLocked(entity, true);
                        if (n43 != 0) {
                            this->app->sound->playSound(1065, 0, 3, 0);
                        }
                    }
                    else {
                        this->app->game->setLineLocked(entity, false);
                        if (n43 != 0) {
                            this->app->sound->playSound(1065, 0, 3, 0);
                        }
                    }
                    break;
                }
                break;
            }

            case Enums::EV_MONSTERFLAGOP: {
                //printf("EV_MONSTERFLAGOP -> %d\n", this->IP);
                short uByteArg14 = this->getUByteArg();
                short uByteArg15 = this->getUByteArg();
                int n46 = uByteArg15 >> 6 & 0x3;
                int n47 = 1 << (uByteArg15 & 0x3F);
                short n48 = this->app->render->mapSprites[this->app->render->S_ENT + uByteArg14];
                if (n48 != -1) {
                    Entity* entity12 = &this->app->game->entities[n48];
                    if (entity12->monster != nullptr) {
                        if (n46 == 0) {
                            entity12->monster->flags |= (short)n47;
                        }
                        else if (n46 == 1) {
                            entity12->monster->flags &= (short)~n47;
                        }
                        else {
                            entity12->monster->flags = (short)n47;
                        }
                        entity12->info |= 0x400000;
                    }
                    break;
                }
                break;
            }

            case Enums::EV_EVENTOP: {
                //printf("EV_EVENTOP -> %d\n", this->IP);
                int uShortArg10 = this->getUShortArg();
                int n49 = ((uShortArg10 & 0x8000) >> 15) << 19;
                int n50 = uShortArg10 & 0x7FFF;
                this->app->render->tileEvents[n50 * 2 + 1] = ((this->app->render->tileEvents[n50 * 2 + 1] & 0xFFF7FFFF) | n49);
                //printf("EventOP: tileEvents[%d] = %X;\n", (n50 * 2 + 1), this->app->render->tileEvents[n50 * 2 + 1]);
                break;
            }

            case Enums::EV_HIDE: {
                //printf("EV_HIDE -> %d\n", this->IP);
                short uByteArg16 = this->getUByteArg();
                this->app->render->mapSpriteInfo[uByteArg16] |= 0x10000;
                short n52 = this->app->render->mapSprites[this->app->render->S_ENT + uByteArg16];
                if (n52 != -1) {
                    Entity* entity14 = &this->app->game->entities[n52];
                    entity14->info |= 0x400000;
                    this->app->game->unlinkEntity(entity14);
                    EntityDef* def = entity14->def;
                    if (def->eType == 10) {
                        if (def->eSubType != 3) {
                            this->app->game->destroyedObject(uByteArg16);
                        }
                    }
                    else if (def->eType == 6 && (def->eSubType == 1 || def->eSubType == 2 || (def->eSubType == 0 && def->parm == 21))) {
                        this->app->game->foundLoot(uByteArg16, 1);
                    }
                    else if (def->eType == 2) {
                        this->corpsifyMonster(entity14->linkIndex % 32, entity14->linkIndex / 32, entity14, false);
                        this->app->game->removeEntity(entity14);
                        entity14->info |= 0x400000;
                    }
                }
                this->app->canvas->updateFacingEntity = true;
                break;
            }

            case Enums::EV_DROPITEM: {
                //printf("EV_DROPITEM -> %d\n", this->IP);
                int uShortArg11 = this->getUShortArg();
                short uByteArg19 = this->getUByteArg();
                short n56 = (short)(uShortArg11 & 0x1F);
                short n57 = (short)(uShortArg11 >> 5 & 0x1F);
                short n58 = (short)(uShortArg11 >> 10 & 0x1F);
                EntityDef* lookup2 = app->entityDefManager->lookup(uByteArg19);
                if (lookup2 == nullptr) {
                    this->app->Error("Cannot find an entity to drop. Err %i", 109); //ERR_EV_DROPITEM
                }
                this->app->game->spawnDropItem((n56 << 6) + 32, (n57 << 6) + 32, uByteArg19, lookup2->eType, lookup2->eSubType, lookup2->parm, n58, true);
                this->app->canvas->staleView = true;
                break;
            }

            case Enums::EV_PREVSTATE: {
                //printf("EV_PREVSTATE -> %d\n", this->IP);
                int index = this->getByteArg();
                --this->app->game->scriptStateVars[index];
                break;
            }

            case Enums::EV_NEXTSTATE: {
                //printf("EV_NEXTSTATE -> %d\n", this->IP);
                int index = this->getByteArg();
                ++this->app->game->scriptStateVars[index];
                break;
            }

            case Enums::EV_WAKEMONSTER: {
                //printf("EV_WAKEMONSTER -> %d\n", this->IP);
                int sprite = this->getUByteArg();
                short n53 = this->app->render->mapSprites[this->app->render->S_ENT + sprite];
                if (n53 == -1 || this->app->game->entities[n53].monster == nullptr) {
                    this->app->Error(23); //ERR_MISC_SCRIPT
                }
                Entity* entity17 = &this->app->game->entities[n53];
                if (entity17->def->eType == 2) {
                    int sprite = entity17->getSprite();
                    entity17->monster->frameTime = 0;
                    this->app->render->mapSpriteInfo[sprite] = ((this->app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x0);
                    this->app->game->activate(entity17, true, false, false, true);
                    break;
                }
                break;
            }

            case Enums::EV_SHOW_PLAYERATTACK: {
                //printf("EV_SHOW_PLAYERATTACK -> %d\n", this->IP);
                this->app->game->cinematicWeapon = this->getByteArg();
                if (!this->app->game->skippingCinematic && this->app->canvas->state == Canvas::ST_CAMERA) {
                    int n59 = this->app->game->cinematicWeapon * 9;
                    this->app->combat->animLoopCount = 10 * this->app->combat->weapons[n59 + 7];
                    this->app->combat->animTime = this->app->combat->weapons[n59 + 8];
                    this->app->combat->animTime *= 10;
                    this->app->combat->animStartTime = this->app->gameTime;
                    this->app->combat->animEndTime = this->app->combat->animStartTime + this->app->combat->animTime;
                    this->app->combat->flashTime = 1; // Old -> 0
                    this->app->combat->flashDoneTime = this->app->combat->animStartTime + this->app->combat->flashTime;
                    break;
                }
                break;
            }

            case Enums::EV_MONSTER_PARTICLES: {
                //printf("EV_MONSTER_PARTICLES -> %d\n", this->IP);
                int sprite = this->getUByteArg();
                int sEnt = this->app->render->mapSprites[this->app->render->S_ENT + sprite];
                if (sEnt != -1) {
                    this->app->particleSystem->spawnMonsterBlood(&this->app->game->entities[sEnt], false);
                }
                break;
            }

            case Enums::EV_SPAWN_PARTICLES: {
                //printf("EV_SPAWN_PARTICLES -> %d\n", this->IP);
                short uByteArg21 = this->getUByteArg();
                int n60 = uByteArg21 >> 3 & 0xF;
                int n61 = uByteArg21 & 0x7;
                int uShortArg12 = this->getUShortArg();
                int n62 = this->getUByteArg() - 48;
                if ((uByteArg21 & 0x80) != 0x0) {
                    int n63 = ((uShortArg12 >> 11 & 0x1F) << 6) + 32;
                    int n64 = ((uShortArg12 >> 6 & 0x1F) << 6) + 32;
                    this->app->particleSystem->spawnParticles(n60, ParticleSystem::levelColors[n61], n63, n64, n62 + this->app->render->getHeight(n63, n64));
                    break;
                }
                this->app->particleSystem->spawnParticles(n60, ParticleSystem::levelColors[n61], uShortArg12);
                break;
            }

            case Enums::EV_FADEOP: {
                //printf("EV_FADEOP -> %d\n", this->IP);
                int uShortArg13 = this->getUShortArg();
                bool b6 = (uShortArg13 & 0x8000) == 0x8000;
                if (this->app->game->skippingCinematic) {
                    if (b6) {
                        this->app->render->endFade();
                        break;
                    }
                    break;
                }
                else {
                    if (b6) {
                        this->app->render->startFade(uShortArg13 & 0x7FFF, 2);
                        break;
                    }
                    this->app->render->startFade(uShortArg13, 1);
                    break;
                }
                break;
            }

            case Enums::EV_GIVEITEM: {
                //printf("EV_GIVEITEM -> %d\n", this->IP);
                short uByteArg17 = this->getUByteArg();
                short uByteArg18 = this->getUByteArg();
                uint8_t byteArg9 = this->getByteArg();
                if (this->throwAwayLoot) {
                    n2 = 1;
                    this->app->game->foundLoot(this->app->canvas->viewX + this->app->canvas->viewStepX, this->app->canvas->viewY + this->app->canvas->viewStepY, this->app->canvas->viewZ, 1);
                    break;
                }
                if (byteArg9 == 0) {
                    short i = (short)(uByteArg17 << 8 | uByteArg18);
                    short n54 = this->app->render->mapSprites[this->app->render->S_ENT + i];
                    if (n54 == -1) {
                        app->Error("Sprite index %i error. Err %i", i, 16); //ERR_GIVE_ITEM
                    }
                    if (!this->app->game->entities[n54].touched()) {
                        n2 = 1;
                    }
                    break;
                }
                EntityDef* lookup = app->entityDefManager->lookup(uByteArg17);
                if (lookup == nullptr) {
                    app->Error("Cannot find an entity to give. Err %i", 109); //ERR_EV_DROPITEM
                }
                short n55 = (char)uByteArg18;
                if (n55 < 0) {
                    if (!this->app->player->give(lookup->eSubType, lookup->parm, n55, true)) {
                        n2 = 1;
                    }
                }
                else if (lookup->eType == 6 && lookup->eSubType == 1 && this->app->player->weaponIsASentryBot(lookup->parm)) {
                    if (!this->app->player->give(lookup->eSubType, lookup->parm, n55, true)) {
                        n2 = 1;
                    }
                }
                else {
                    Entity* spawnDropItem = this->app->game->spawnDropItem(0, 0, uByteArg17, lookup, n55, false);
                    uint8_t eType = spawnDropItem->def->eType;
                    if (!spawnDropItem->touched()) {
                        this->app->game->removeEntity(spawnDropItem);
                        n2 = 1;
                    }
                    else if (eType != 3 && n55 > 0) {
                        this->app->game->foundLoot(this->app->canvas->viewX + this->app->canvas->viewStepX, this->app->canvas->viewY + this->app->canvas->viewStepY, this->app->canvas->viewZ, 1);
                    }
                }
                break;
            }

            case Enums::EV_NAMEENTITY: {
                //printf("EV_NAMEENTITY -> %d\n", this->IP);
                short uByteArg7 = this->getUByteArg();
                short uByteArg8 = this->getUByteArg();
                short n37 = this->app->render->mapSprites[this->app->render->S_ENT + uByteArg7];
                if (n37 != -1) {
                    this->app->game->entities[n37].info |= 0x400000;
                    this->app->game->entities[n37].name = (short)(uByteArg8 | this->app->canvas->loadMapStringID << 10);
                    break;
                }
                break;
            }

            case Enums::EV_DROPMONSTERITEM: {
                //printf("EV_DROPMONSTERITEM -> %d\n", this->IP);
                bool b7 = false;
                int uShortArg14 = this->getUShortArg();
                int uByteArg22 = this->getUByteArg();
                if ((uShortArg14 & 0x8000) != 0x0) {
                    uByteArg22 = (uByteArg22 << 8 | this->getUByteArg());
                    uShortArg14 &= 0x7FFF;
                    b7 = true;
                }
                short uByteArg23 = this->getUByteArg();
                short n65 = this->app->render->mapSprites[this->app->render->S_X + uShortArg14];
                short n66 = this->app->render->mapSprites[this->app->render->S_Y + uShortArg14];
                if (!b7) {
                    EntityDef* lookup3 = app->entityDefManager->lookup(uByteArg22);
                    if (lookup3 == nullptr) {
                        this->app->Error("Cannot find an entity to drop. Err %i", 109); //ERR_EV_DROPITEM
                    }
                    this->app->game->spawnDropItem(n65, n66, uByteArg22, lookup3, uByteArg23, true);
                }
                else {
                    Entity* entity18 = &this->app->game->entities[this->app->render->mapSprites[this->app->render->S_ENT + uByteArg22]];
                    int height = app->render->getHeight(n65, n66);
                    this->app->render->mapSprites[app->render->S_X + uByteArg22] = n65;
                    this->app->render->mapSprites[app->render->S_Y + uByteArg22] = n66;
                    this->app->render->mapSprites[app->render->S_Z + uByteArg22] = (short)(32 + height);
                    this->app->render->relinkSprite(uByteArg22);
                    this->app->game->unlinkEntity(entity18);
                    this->app->game->linkEntity(entity18, n65 >> 6, n66 >> 6);
                    this->app->game->throwDropItem(n65, n66, height, entity18);
                    entity18->info |= 0x400000;
                }
                this->app->canvas->staleView = true;
                break;
            }

            case Enums::EV_SETDEATHFUNC: {
                //printf("EV_SETDEATHFUNC -> %d\n", this->IP);
                short uByteArg24 = this->getUByteArg();
                short shortArg = this->getShortArg();
                short n67 = this->app->render->mapSprites[this->app->render->S_ENT + uByteArg24];
                if (n67 != -1) {
                    Entity* entity20 = &this->app->game->entities[n67];
                    if (shortArg != -1) {
                        this->app->game->addEntityDeathFunc(entity20, shortArg);
                    }
                    else {
                        this->app->game->removeEntityFunc(entity20);
                    }
                    break;
                }
                break;
            }

            case Enums::EV_PLAYSOUND: {
                //printf("EV_PLAYSOUND -> %d\n", this->IP);
                short resID = this->getUByteArg();
                int args = this->getUByteArg();
                //printf("resID -> %d\n", resID);

                if (this->app->canvas->inInitMap && this->app->canvas->areSoundsAllowed) {
                    bool soundEnabled = this->app->sound->allowSounds;
                    this->app->sound->allowSounds = true;
                    this->app->sound->playSound(resID + 1000, args >> 4, args & 0xF, 0);
                    this->app->sound->allowSounds = soundEnabled;
                    break;
                }
                this->app->sound->playSound(resID + 1000, args >> 4, args & 0xF, 0);
                break;
            }

            case Enums::EV_NPCCHAT: {
                //printf("EV_NPCCHAT -> %d\n", this->IP);
                int uShortArg15 = this->getUShortArg();
                int param = uShortArg15 >> 14 & 0x3;
                int n68 = uShortArg15 & 0x3FFF;
                if (this->app->render->mapSprites[this->app->render->S_ENT + n68] != -1) {
                    Entity* entity21 = &this->app->game->entities[this->app->render->mapSprites[this->app->render->S_ENT + n68]];
                    if (entity21->def->eType == 3) {
                        entity21->param = param;
                        break;
                    }
                }
                this->app->Error(14); // ERR_EV_SHOWCHATBUBBLE

                if (this->app->canvas->showingLoot) {
                    this->unpauseTime = 1;
                    return 2;
                }
                this->composeLootDialog();
                if (!this->throwAwayLoot) {
                    this->app->game->skipAdvanceTurn = true;
                    this->app->game->queueAdvanceTurn = false;
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                break;
            }

            case Enums::EV_STOCKSTATION: {
                // No implementado
                break;
            }

            case Enums::EV_LERPFLAT: {
                //printf("EV_LERPFLAT -> %d\n", this->IP);
                this->getUByteArg();
                this->getUShortArg();
                break;
            }

            case Enums::EV_GIVELOOT: {
                //printf("EV_GIVELOOT -> %d\n", this->IP);
                if (this->app->canvas->showingLoot) {
                    this->unpauseTime = 1;
                    return 2;
                }
                this->composeLootDialog();
                if (!this->throwAwayLoot) {
                    this->app->game->skipAdvanceTurn = true;
                    this->app->game->queueAdvanceTurn = false;
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                break;
            }

            case Enums::EV_MARKTILE: {
                //printf("EV_MARKTILE -> %d\n", this->IP);
                int args = this->getUShortArg();
                int flags = (args >> 10) & 0x3F;
                int sX = (args >> 5) & 0x1F;
                int sY = args & 0x1F;
                if ((flags & 0x1) != 0x0) {
                    this->app->game->setMonsterClip(sX, sY);
                }
                if ((flags & 0x20) != 0x0) {
                    this->app->render->mapEntranceAutomap = (short)((sY * 32) + sX);
                }
                if ((flags & 0x10) != 0x0) {
                    this->app->render->mapExitAutomap = (short)((sY * 32) + sX);
                }
                if ((flags & 0x2) != 0x0) {
                    int n74 = 0;
                    while (n74 < Render::MAX_LADDERS_PER_MAP && this->app->render->mapLadders[n74] != -1) {
                        n74++;
                    }
                    if (n74 != Render::MAX_LADDERS_PER_MAP) {
                        this->app->render->mapLadders[n74] = (short)((sY * 32) + sX);
                    }
                }
                //int n75 = (flags & 0xFFFFFFFE & 0xFFFFFFDF & 0xFFFFFFEF & 0xFFFFFFFD) << 2; // J2ME Version
                //int n75 = (flags & 0xffffffcc) << 2; // Brew Version
                int n75 = (flags & 0xC) << 2; // IOS Version
                this->app->render->mapFlags[(sY * 32) + sX] |= (uint8_t)n75;
                break;
            }

            case Enums::EV_UPDATEJOURNAL: {
                //printf("EV_UPDATEJOURNAL -> %d\n", this->IP);
                short uByteArg27 = this->getUByteArg();
                short uByteArg28 = this->getUByteArg();
                this->app->player->updateQuests(uByteArg27, uByteArg28);
                if (uByteArg28 == 0) {
                    this->app->player->showHelp((short)5, true);
                    break;
                }
                break;
            }

            case Enums::EV_BRIBE_ENTITY: {
                // No implementado
                break;
            }

            case Enums::EV_PLAYER_ADD_STAT: {
                //printf("EV_PLAYER_ADD_STAT -> %d\n", this->IP);
                short uByteArg29 = this->getUByteArg();
                this->app->player->modifyStat(uByteArg29 >> 5 & 0x7, ((uByteArg29 & 0x1F) << 27) >> 27);
                break;
            }

            case Enums::EV_PLAYER_ADD_RECIPE: {
                // No implementado
                break;
            }

            case Enums::EV_RESPAWN_MONSTER: {
                //printf("EV_RESPAWN_MONSTER -> %d\n", this->IP);
                int uShortArg21 = this->getUShortArg();
                short uByteArg30 = this->getUByteArg();
                short uByteArg31 = this->getUByteArg();
                if (this->app->render->mapSprites[this->app->render->S_ENT + uShortArg21] != -1) {
                    Entity* entity23 = &this->app->game->entities[this->app->render->mapSprites[this->app->render->S_ENT + uShortArg21]];
                    if ((entity23->info & 0x1010000) != 0x0 && nullptr == this->app->game->findMapEntity(uByteArg30, uByteArg31, 1030)) {
                        entity23->resurrect((uByteArg30 << 6) + 32, (uByteArg31 << 6) + 32, 32);
                        break;
                    }
                }
                n2 = -1;
                break;
            }

            case Enums::EV_SCREEN_SHAKE: {
                //printf("EV_SCREEN_SHAKE -> %d\n", this->IP);
                int uShortArg22 = this->getUShortArg();
                int n90 = uShortArg22 >> 14 & 0x3;
                if (n90 >= 0) {
                    ++n90;
                }
                int n91 = uShortArg22 >> 7 & 0x7F;
                if (n91 > 0) {
                    n91 = n91 + 1 << 4;
                }
                int n92 = uShortArg22 & 0x7F;
                if (n92 > 0) {
                    n92 = n92 + 1 << 4;
                }
                this->app->canvas->startShake(n91, n90, n92);
                break;
            }

            case Enums::EV_SPEECHBUBBLE: {
                //printf("EV_SPEECHBUBBLE -> %d\n", this->IP);
                int16_t texId = (int16_t)this->getUShortArg();
                int colorIndex = this->getUByteArg();
                this->app->hud->showSpeechBubble(texId, colorIndex);
                break;
            
            }

            case Enums::EV_AWARDSECRET: {
                //printf("EV_AWARDSECRET -> %d\n", this->IP);
                this->app->game->awardSecret(false);
                break;
            }

            case Enums::EV_AIGOAL: {
                //printf("EV_AIGOAL -> %d\n", this->IP);
                int uShortArg23 = this->getUShortArg();
                int n93 = uShortArg23 >> 12 & 0xF;
                uint8_t byteArg10 = this->getByteArg();
                int n94 = uShortArg23 & 0xFFF;
                if (this->app->render->mapSprites[this->app->render->S_ENT + n94] != -1) {
                    this->setAIGoal(&this->app->game->entities[this->app->render->mapSprites[this->app->render->S_ENT + n94]], n93, byteArg10);
                    break;
                }
                this->app->Error(76); //ERR_EV_AIGOAL
                break;
            }

            case Enums::EV_ADVANCETURN: {
                //printf("EV_ADVANCETURN -> %d\n", this->IP);
                if (this->app->game->interpolatingMonsters) {
                    this->app->game->snapMonsters(true);
                }
                while (this->app->game->combatMonsters != nullptr) {
                    this->app->game->combatMonsters->undoAttack();
                }
                this->app->game->endMonstersTurn();
                this->app->canvas->clearEvents(1);
                this->app->game->advanceTurn();
                break;
            }

            case Enums::EV_MINIGAME: {
                //printf("EV_MINIGAME -> %d\n", this->IP);
                short n111 = this->getByteArg();
                short n112 = this->getByteArg();
                short n113 = this->getByteArg();
                short n114 = this->getByteArg();
                if (n111 == 0) {
                    this->app->sentryBotGame->initGame(this, n112);
                    if (this->app->game->skipMinigames) {
                        this->app->sentryBotGame->forceWin();
                    }
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                if (n111 == 4) {
                    this->app->vendingMachine->initGame(this, this->app->canvas->loadMapID, (this->app->canvas->loadMapID - 1) * 2 + n112);
                    if (this->app->game->skipMinigames) {
                        this->app->vendingMachine->forceWin();
                    }
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                if (n111 == 2) {
                    short n115 = n112;
                    short n116 = n114;
                    if (n115 == 1) {
                        app->Error("Custom Puzzles not supported on brew version of game!!");
    #if 0 // J2ME
                        short n117 = n113;
                        Text* smallBuffer = app->localization->getSmallBuffer();
                        app->localization->composeText(this->app->canvas->loadMapStringID, n117, smallBuffer);
                        if (n116 == -1) {
                            //this->app->hackingGame->initGame(this, smallBuffer);
                        }
                        else {
                            //this->app->hackingGame->initGame(this, smallBuffer, n116);
                        }
                        smallBuffer->dispose();
    #endif
                    }
                    else if (n115 == 0) {
                        short n118 = n113;
                        if (n116 == -1) {
                            this->app->hackingGame->initGame(this, n118);
                        }
                        else {
                            this->app->hackingGame->initGame(this, n118, n116);
                        }
                    }
                    if (this->app->game->skipMinigames) {
                        this->app->hackingGame->forceWin();
                    }
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                break;
            }

            case Enums::EV_ENDMINIGAME: {
                // No implementado
                break;
            }

            case Enums::EV_ENDROUND: {
                // No implementado
                break;
            }

            case Enums::EV_PLAYERATTACK: {
                //printf("EV_PLAYERATTACK -> %d\n", this->IP);
                int uShortArg24 = this->getUShortArg();
                int weapon = uShortArg24 >> 12 & 0xF;
                int n119 = uShortArg24 & 0xFFF;
                if (this->app->render->mapSprites[this->app->render->S_ENT + n119] != -1) {
                    Entity* entity24 = &this->app->game->entities[this->app->render->mapSprites[this->app->render->S_ENT + n119]];
                    this->app->player->ce->weapon = weapon;
                    this->app->combat->performAttack(nullptr, entity24, 0, 0, true);
                }
                n = this->evWait(1);
                break;
            }

            case Enums::EV_SET_FOG_COLOR: {
                //printf("EV_SET_FOG_COLOR -> %d\n", this->IP);
                this->app->render->buildFogTables(this->getIntArg());
                break;
            }

            case Enums::EV_LERP_FOG: {
                //printf("EV_LERP_FOG -> %d\n", this->IP);
                int intArg2 = this->getIntArg();
                this->app->render->startFogLerp(intArg2 & 0x7FF, (intArg2 >> 11) & 0x7FF, ((uint8_t)(intArg2 >> 22) & 0xFF) * 100);
                break;
            }

            case Enums::EV_LERPSPRITEOFFSET: {
                //printf("EV_LERPSPRITEOFFSET -> %d\n", this->IP);
                short uByteArg3 = this->getUByteArg();
                int travelTime2 = this->getUByteArg() * 100;
                int intArg = this->getIntArg();
                int dstY = intArg & 0x7FF;
                int dstX = intArg >> 11 & 0x7FF;
                int n13 = intArg >> 22 & 0x3;
                int n14 = (intArg >> 24 & 0xFF) - 48;
                LerpSprite* allocLerpSprite2 = this->app->game->allocLerpSprite(this, uByteArg3, (n13 & 0x2) != 0x0);
                if (allocLerpSprite2 == nullptr) {
                    return 0;
                }
                allocLerpSprite2->dstX = dstX;
                allocLerpSprite2->dstY = dstY;
                short n15 = this->app->render->mapSprites[this->app->render->S_ENT + uByteArg3];
                if (n15 != -1) {
                    this->app->game->entities[n15].info |= 0x400000;
                    if (this->app->game->entities[n15].monster != nullptr) {
                        this->app->game->entities[n15].monster->flags |= 0x4000;
                    }
                }
                allocLerpSprite2->dstZ = this->app->render->getHeight(allocLerpSprite2->dstX, allocLerpSprite2->dstY) + n14;
                allocLerpSprite2->srcX = this->app->render->mapSprites[this->app->render->S_X + uByteArg3];
                allocLerpSprite2->srcY = this->app->render->mapSprites[this->app->render->S_Y + uByteArg3];
                allocLerpSprite2->srcZ = this->app->render->mapSprites[this->app->render->S_Z + uByteArg3];
                allocLerpSprite2->srcScale = allocLerpSprite2->dstScale = this->app->render->mapSprites[this->app->render->S_SCALEFACTOR + uByteArg3];
                allocLerpSprite2->startTime = app->gameTime;
                allocLerpSprite2->travelTime = travelTime2;
                allocLerpSprite2->calcDist();
                allocLerpSprite2->flags = (n13 & 0x3);
                if (travelTime2 == 0) {
                    if ((this->app->game->updateLerpSprite(allocLerpSprite2) & 0x1) != 0x0) {
                        this->app->canvas->invalidateRect();
                        break;
                    }
                    n = 0;
                    break;
                }
                else {
                    if (travelTime2 != 0 && (n13 & 0x2) != 0x0) {
                        this->evWait(travelTime2);
                    }
                    if (!(allocLerpSprite2->flags & Enums::LS_FLAG_ASYNC)) {
                        this->app->game->skipAdvanceTurn = true;
                        this->app->game->queueAdvanceTurn = false;
                        this->unpauseTime = -1;
                        n = 2;
                        break;
                    }
                    break;
                }
                break;
            }

            case Enums::EV_DISABLED_WEAPONS: {
                //printf("EV_DISABLED_WEAPONS -> %d\n", this->IP);
                this->app->player->disabledWeapons = this->getShortArg();
                if ((this->app->player->disabledWeapons & (1 << this->app->player->ce->weapon)) != 0x0) {
                    this->app->player->selectNextWeapon();
                    break;
                }
                break;
            }

            case Enums::EV_LERPSCALE: {
                //printf("EV_LERPSCALE -> %d\n", this->IP);
                int uShortArg25 = this->getUShortArg();
                int n120 = uShortArg25 >> 4;
                int n121 = uShortArg25 & 0xF;
                int uShortArg26 = this->getUShortArg();
                short uByteArg32 = this->getUByteArg();
                LerpSprite* allocLerpSprite3 = this->app->game->allocLerpSprite(this, n120, (n121 & 0x2) != 0x0);
                if (allocLerpSprite3 == nullptr) {
                    return 0;
                }
                allocLerpSprite3->srcX = allocLerpSprite3->dstX = this->app->render->mapSprites[this->app->render->S_X + n120];
                allocLerpSprite3->srcY = allocLerpSprite3->dstY = this->app->render->mapSprites[this->app->render->S_Y + n120];
                allocLerpSprite3->srcZ = allocLerpSprite3->dstZ = this->app->render->mapSprites[this->app->render->S_Z + n120];
                allocLerpSprite3->srcScale = this->app->render->mapSprites[this->app->render->S_SCALEFACTOR + n120];
                allocLerpSprite3->dstScale = uByteArg32 << 1;
                short n125 = this->app->render->mapSprites[this->app->render->S_ENT + n120];
                if (n125 != -1) {
                    this->app->game->entities[n125].info |= 0x400000;
                }
                allocLerpSprite3->startTime = app->gameTime;
                allocLerpSprite3->travelTime = uShortArg26;
                allocLerpSprite3->flags = (n121 & 0x3);
                if (uShortArg26 == 0) {
                    if ((this->app->game->updateLerpSprite(allocLerpSprite3) & 0x1) != 0x0) {
                        this->app->canvas->invalidateRect();
                        break;
                    }
                    n = 0;
                    break;
                }
                else {
                    if (uShortArg26 != 0 && (n121 & 0x2) != 0x0) {
                        this->evWait(uShortArg26);
                    }
                    if (!(allocLerpSprite3->flags & Enums::LS_FLAG_ASYNC)) {
                        this->app->game->skipAdvanceTurn = true;
                        this->app->game->queueAdvanceTurn = false;
                        this->unpauseTime = -1;
                        n = 2;
                        break;
                    }
                    break;
                }
                break;
            }

            case Enums::EV_GIVEAWARD: {
                // No implementado
                break;
            }

            case Enums::EV_STARTMIXING: {
                // No implementado
                break;
            }
               
            case Enums::EV_DEBUGPRINT: {
                //printf("EV_DEBUGPRINT -> %d\n", this->IP);
                if (this->debugString == nullptr) {
                    this->debugString = this->app->localization->getLargeBuffer();
                }
                uint8_t byteArg11 = this->getByteArg();
                if (byteArg11 == 0) {
                    for (char c = (char)this->getUByteArg(); c != '\0'; c = (char)this->getUByteArg()) {
                        this->debugString->append(c);
                    }
                }
                else if (byteArg11 == 1) {
                    int index = this->getUByteArg();
                    this->debugString->append(this->app->game->scriptStateVars[index]);
                }
                if (this->peekNextCmd() != 66) {
                    this->debugString->dispose();
                    this->debugString = nullptr;
                    break;
                }
                break;
            }

            case Enums::EV_GOTO_MENU: {
                //printf("EV_GOTO_MENU -> %d\n", this->IP);
                this->app->menuSystem->setMenu(this->getUByteArg());
                break;
            }

            case Enums::EV_START_INTERCINEMATIC: {
                //printf("EV_START_INTERCINEMATIC -> %d\n", this->IP);
                this->setupCamera(this->getUByteArg() & 0x7F);
                this->app->game->activeCamera->cameraThread = this;
                this->app->canvas->setState(Canvas::ST_INTER_CAMERA);
                break;
            }

            case Enums::EV_TURN_PLAYER: {
                //printf("EV_TURN_PLAYER -> %d\n", this->IP);
                uint8_t byteArg4 = this->getByteArg();
                int n34 = byteArg4 >> 3;
                int n35 = byteArg4 & 0x7;
                int viewAngle2 = app->canvas->viewAngle & 0x3FF;
                int n36 = n35 << 7;
                if (n36 - viewAngle2 > 512) {
                    n36 -= 1024;
                }
                else if (n36 - viewAngle2 < -512) {
                    n36 += 1024;
                }
                if (viewAngle2 == n36) {
                    break;
                }
                if (n34 == 1) {
                    app->canvas->viewAngle = viewAngle2;
                    app->canvas->destAngle = n36;
                    app->canvas->startRotation(false);
                    app->canvas->gotoThread = this;
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                app->canvas->viewAngle = (app->canvas->destAngle = n36);
                break;
            }

            case Enums::EV_STATUS_EFFECT: {
                //printf("EV_STATUS_EFFECT -> %d\n", this->IP);
                short uByteArg33 = this->getUByteArg();
                if ((uByteArg33 & 0x80) != 0x0) {
                    app->player->removeStatusEffect(uByteArg33 & 0x7F);
                    break;
                }
                uint8_t byteArg12 = this->getByteArg();
                int n126 = 30;
                if (uByteArg33 == 2) {
                    n126 = 20;
                }
                else if (uByteArg33 == 9) {
                    n126 = 10;
                }
                else if (uByteArg33 == 1) {
                    n126 = 10;
                }
                else if (uByteArg33 == 11) {
                    n126 = 6;
                }
                else if (uByteArg33 == 17) {
                    n126 = 5;
                }
                app->player->addStatusEffect(uByteArg33, byteArg12, n126);
                app->player->translateStatusEffects();
                break;
            }

            case Enums::EV_JOURNAL_TILE: {
                //printf("EV_JOURNAL_TILE -> %d\n", this->IP);
                int arg1 = this->getUByteArg();
                int tileX = this->getUByteArg() & 0x1F;
                int tileY = this->getUByteArg() & 0x1F;
                app->player->setQuestTile(arg1, tileX, tileY);
                break;
            }

            case Enums::EV_MAKE_CORPSE: {
                //printf("EV_MAKE_CORPSE -> %d\n", this->IP);
                int uShortArg27 = this->getUShortArg();
                short n127 = (short)((this->getUByteArg() << 6) + 32);
                short n128 = (short)((this->getUByteArg() << 6) + 32);
                Entity* entity26 = &this->app->game->entities[this->app->render->mapSprites[this->app->render->S_ENT + uShortArg27]];
                if (entity26->monster != nullptr) {
                    this->corpsifyMonster(n127, n128, entity26, true);
                    break;
                }
                break;
            }

            case Enums::EV_INVENTORY_OP: {
                //printf("EV_INVENTORY_OP -> %d\n", this->IP);
                uint8_t byteArg13 = this->getByteArg();
                if (byteArg13 == 2) {
                    app->player->restoreInventory();
                    break;
                }
                if (byteArg13 == 1) {
                    app->player->stripInventoryForTargetPractice();
                    break;
                }
                if (byteArg13 == 0) {
                    app->player->stripInventoryForViosBattle();
                    break;
                }
                break;
            }

            case Enums::EV_END_GAME: {
                //printf("EV_END_GAME -> %d\n", this->IP);
                app->player->gameCompleted = true;
                app->canvas->endingGame = true;
                app->canvas->setState(Canvas::ST_EPILOGUE);
                break;
            }

            case Enums::EV_LERPSPRITEPARABOLA: {
                //printf("EV_LERPSPRITEPARABOLA -> %d\n", this->IP);
                int intArg3 = this->getIntArg();
                int uShortArg28 = this->getUShortArg();
                int n129 = intArg3 >> 22 & 0x3FF;
                int n130 = intArg3 >> 17 & 0x1F;
                int n131 = intArg3 >> 12 & 0x1F;
                int height2 = (intArg3 >> 4 & 0xFF) - 48;
                int n132 = intArg3 & 0xF;
                LerpSprite* allocLerpSprite4 = this->app->game->allocLerpSprite(this, n129, (n132 & 0x2) != 0x0);
                if (allocLerpSprite4 == nullptr) {
                    return 0;
                }
                allocLerpSprite4->dstX = 32 + (n130 << 6);
                allocLerpSprite4->dstY = 32 + (n131 << 6);
                short n133 = this->app->render->mapSprites[this->app->render->S_ENT + n129];
                if (n133 != -1) {
                    this->app->game->entities[n133].info |= 0x400000;
                    if (this->app->game->entities[n133].monster != nullptr) {
                        this->app->game->entities[n133].monster->flags &= 0xFFFFBFFF;
                    }
                }
                allocLerpSprite4->srcX = this->app->render->mapSprites[this->app->render->S_X + n129];
                allocLerpSprite4->srcY = this->app->render->mapSprites[this->app->render->S_Y + n129];
                allocLerpSprite4->srcZ = this->app->render->mapSprites[this->app->render->S_Z + n129];
                allocLerpSprite4->dstZ = this->app->render->getHeight(allocLerpSprite4->dstX, allocLerpSprite4->dstY) + (allocLerpSprite4->srcZ - this->app->render->getHeight(allocLerpSprite4->srcX, allocLerpSprite4->srcY));
                allocLerpSprite4->srcScale = allocLerpSprite4->dstScale = this->app->render->mapSprites[this->app->render->S_SCALEFACTOR + n129];
                allocLerpSprite4->height = height2;
                allocLerpSprite4->startTime = app->gameTime;
                allocLerpSprite4->travelTime = uShortArg28;
                allocLerpSprite4->calcDist();
                allocLerpSprite4->flags = (n132 & 0x3);
                allocLerpSprite4->flags |= Enums::LS_FLAG_PARABOLA;
                if (uShortArg28 == 0) {
                    if ((this->app->game->updateLerpSprite(allocLerpSprite4) & 0x1) != 0x0) {
                        this->app->canvas->invalidateRect();
                        break;
                    }
                    n = 0;
                    break;
                }
                else {
                    if (uShortArg28 != 0 && (n132 & 0x2) != 0x0) {
                        this->evWait(uShortArg28);
                    }
                    if (!(allocLerpSprite4->flags & Enums::LS_FLAG_ASYNC)) {
                        this->app->game->skipAdvanceTurn = true;
                        this->app->game->queueAdvanceTurn = false;
                        this->unpauseTime = -1;
                        n = 2;
                        break;
                    }
                    break;
                }
                break;
            }

            case Enums::EV_TOGGLE_OVERLAY: {
                //printf("EV_TOGGLE_OVERLAY -> %d\n", this->IP);
                this->app->hud->cockpitOverlayRaw ^= 1;
                break;
            }

            case Enums::EV_FOG_AFFECTS_SKYMAP: {
                //printf("EV_FOG_AFFECTS_SKYMAP -> %d\n", this->IP);
                this->app->render->fogAffectsSkyMap = (this->getByteArg() != 0);
                break;
            }

            case Enums::EV_ENABLE_HELP: {
                //printf("EV_ENABLE_HELP -> %d\n", this->IP);
                this->app->player->enableHelp = (this->getByteArg() != 0);
                break;
            }

            case Enums::EV_SET_MM_RENDER_HACK: {
                //printf("EV_SET_MM_RENDER_HACK -> %d\n", this->IP);
                this->app->render->useMastermindHack = (this->getByteArg() != 0);
                break;
            }

            case Enums::EV_START_ARMORREPAIR: {
                //printf("EV_START_ARMORREPAIR -> %d\n", this->IP);
                if (this->app->canvas->startArmorRepair(this)) {
                    this->unpauseTime = -1;
                    n = 2;
                    break;
                }
                break;
            }

            case Enums::EV_FORCE_BOT_RETURN: {
                //printf("EV_FORCE_BOT_RETURN -> %d\n", this->IP);
                if (this->app->player->isFamiliar) {
                    this->app->player->familiarReturnsToPlayer(false);
                    break;
                }
                break;
            }

            case Enums::EV_UNMARKTILE: {
                //printf("EV_UNMARKTILE -> %d\n", this->IP);
                int uShortArg18 = this->getUShortArg();
                int n77 = uShortArg18 >> 10 & 0x3F;
                int n78 = uShortArg18 >> 5 & 0x1F;
                int n79 = uShortArg18 & 0x1F;
                if ((n77 & 0x1) != 0x0) {
                    app->game->unsetMonsterClip(n78, n79);
                }
                if ((n77 & 0x20) != 0x0 && app->render->mapEntranceAutomap == n79 * 32 + n78) {
                    app->render->mapEntranceAutomap = -1;
                }
                if ((n77 & 0x10) != 0x0 && app->render->mapExitAutomap == n79 * 32 + n78) {
                    app->render->mapExitAutomap = -1;
                }
                if ((n77 & 0x2) != 0x0) {
                    for (int k = 0; k < Render::MAX_LADDERS_PER_MAP; ++k) {
                        if (app->render->mapLadders[k] == n79 * 32 + n78) {
                            app->render->mapLadders[k] = -1;
                        }
                    }
                }
                //int n80 = (n77 & 0xFFFFFFFE & 0xFFFFFFDF & 0xFFFFFFEF & 0xFFFFFFFD) << 2; //J2ME Version
                //int n80 = (n77 & 0xffffffcc) << 2; Brew Version
                int n80 = (n77 & 0xC) << 2; // IOS Version
                app->render->mapFlags[n79 * 32 + n78] &= (uint8_t)~n80;
                break;
            }

            case Enums::EV_ASSIGN_LOOTSET: {
                //printf("EV_ASSIGN_LOOTSET -> %d\n", this->IP);
                int n69 = this->getUShortArg() & 0xFFF;
                if (this->app->render->mapSprites[this->app->render->S_ENT + n69] != -1) {
                    Entity* entity22 = &this->app->game->entities[this->app->render->mapSprites[this->app->render->S_ENT + n69]];
                    bool b8 = entity22->lootSet != nullptr;
                    short uByteArg26 = this->getUByteArg();
                    for (short n70 = 0; n70 < uByteArg26; ++n70) {
                        int uShortArg16 = this->getUShortArg();
                        if (b8) {
                            entity22->lootSet[n70] = uShortArg16;
                        }
                    }
                    if (b8) {
                        for (int j = uByteArg26; j < 3; ++j) {
                            entity22->lootSet[j] = 0;
                        }
                    }
                    break;
                }
                app->Error(117); //ERR_ENT_LOOTSET
                break;
            }

            case Enums::EV_START_TARGETPRACTICE: {
                //printf("EV_START_TARGETPRACTICE -> %d\n", this->IP);
                int uShortArg20 = this->getUShortArg();
                this->app->player->enterTargetPractice(uShortArg20 >> 8 & 0x1F, uShortArg20 >> 3 & 0x1F, uShortArg20 & 0x7, this);
                this->unpauseTime = -1;
                n = 2;
                break;
            }

            case Enums::EV_GIVE_AUTOMAP: {
                //printf("EV_GIVE_AUTOMAP -> %d\n", this->IP);
                this->app->game->givemap(0, 0, 32, 32);
                break;
            }

            case Enums::EV_ANGER_VIOS: {
                //printf("EV_ANGER_VIOS -> %d\n", this->IP);
                this->app->game->angryVIOS = (this->getByteArg() != 0);
                break;
            }

            case Enums::EV_UNHIDE_AUTOMAP: {
                //printf("EV_UNHIDE_AUTOMAP -> %d\n", this->IP);
                short n97 = this->getByteArg();
                short n98 = this->getByteArg();
                short n99 = this->getByteArg();
                for (short n100 = this->getByteArg(), n101 = n98; n101 <= n100; ++n101) {
                    for (short n102 = n97; n102 <= n99; ++n102) {
                        this->app->render->mapFlags[n101 * 32 + n102] &= ~8;
                    }
                }
                this->app->canvas->automapDrawn = false;
                break;
            }

            case Enums::EV_HIDE_AUTOMAP: {
                //printf("EV_HIDE_AUTOMAP -> %d\n", this->IP);
                short n104 = this->getByteArg();
                short n105 = this->getByteArg();
                short n106 = this->getByteArg();
                for (short n107 = this->getByteArg(), n108 = n105; n108 <= n107; ++n108) {
                    for (short n109 = n104; n109 <= n106; ++n109) {
                        this->app->render->mapFlags[n108 * 32 + n109] |= 0x8;
                    }
                }
                this->app->canvas->automapDrawn = false;
                break;
            }

            case Enums::EV_PORTAL_EVENT: {
                //printf("EV_PORTAL_EVENT -> %d\n", this->IP);
                uint8_t byteArg14 = this->getByteArg();
                int portalState = byteArg14 & 0xF;
                int previousPortalState = byteArg14 >> 4 & 0xF;
                if (portalState == 4) {
                    app->render->portalScripted = false;
                    break;
                }
                app->render->portalScripted = true;
                app->render->portalState = portalState;
                app->render->previousPortalState = previousPortalState;
                break;
            }

            case Enums::EV_PITCH_CONTROL: {
                //printf("EV_PITCH_CONTROL -> %d\n", this->IP);
                int uShortArg19 = this->getUShortArg();
                int n82 = uShortArg19 & 0x7;
                int n83 = uShortArg19 >> 8 & 0x1F;
                int n84 = uShortArg19 >> 3 & 0x1F;
                int n85 = uShortArg19 >> 13 & 0x1;
                if (n85 == 0) {
                    int n86;
                    for (n86 = 0; n86 < Render::MAX_KEEP_PITCH_LEVEL_TILES && this->app->render->mapKeepPitchLevelTiles[n86] != -1; ++n86) {}
                    if (n86 != Render::MAX_KEEP_PITCH_LEVEL_TILES) {
                        this->app->render->mapKeepPitchLevelTiles[n86] = (short)n83;
                        this->app->render->mapKeepPitchLevelTiles[n86] |= (short)(n84 << 5);
                        this->app->render->mapKeepPitchLevelTiles[n86] |= (short)(n82 << 10);
                    }
                    break;
                }
                if (n85 == 1) {
                    for (int l = 0; l < Render::MAX_KEEP_PITCH_LEVEL_TILES; ++l) {
                        if (this->app->render->mapKeepPitchLevelTiles[l] != -1) {
                            short n89 = this->app->render->mapKeepPitchLevelTiles[l];
                            if ((n84 << 5) + n83 == (n89 & 0x3FF) && n82 == (n89 & 0xFFFFFC00) >> 10) {
                                this->app->render->mapKeepPitchLevelTiles[l] = -1;
                            }
                        }
                    }
                    break;
                }
                break;
            }

            case Enums::EV_USED_CHAINSAW: {
                //printf("EV_USED_CHAINSAW -> %d\n", this->IP);
                this->app->player->usedChainsaw(false);
                break;
            }

            case Enums::EV_ENTITY_BREATHES: {
                //printf("EV_ENTITY_BREATHES -> %d\n", this->IP);
                short uByteArg9 = this->getUByteArg();
                short uByteArg10 = this->getUByteArg();
                short n38 = this->app->render->mapSprites[this->app->render->S_ENT + uByteArg9];
                if (n38 != -1) {
                    Entity* entity5 = &this->app->game->entities[n38];
                    if (uByteArg10 == 1) {
                        entity5->info &= ~0x20000000;
                    }
                    else if (uByteArg10 == 0) {
                        entity5->info |= 0x20000000;
                    }
                    break;
                }
                break;
            }

            case Enums::EV_DESTROY_PLAYER: {
                //printf("EV_DESTROY_PLAYER -> %d\n", this->IP);
                short n95 = this->getByteArg();
                if (this->app->player->noclip) {
                    break;
                }
                int n96 = this->app->player->isFamiliar ? this->app->player->ammo[7] : this->app->player->ce->getStat(0);
                if (!this->app->player->isFamiliar) {
                    this->app->player->painEvent(nullptr, false);
                }
                this->app->hud->damageTime = this->app->time + 1000;
                this->app->combat->totalDamage = 1;
                this->app->player->pain(n96, nullptr, true);
                if (!this->app->player->isFamiliar) {
                    this->app->player->addArmor(-this->app->player->ce->getStat(2));
                    break;
                }
                if (n95 == 0) {
                    this->app->player->noFamiliarRemains = true;
                    break;
                }
                break;
            }

            case Enums::EV_START_TREADMILL: {
                //printf("EV_START_TREADMILL -> %d\n", this->IP);
                this->app->canvas->setState(Canvas::ST_TREADMILL);
                break;
            }

            case Enums::EV_STOPSOUND: {
                //printf("EV_STOPSOUND -> %d\n", this->IP);
                int resID = this->getUByteArg();
                app->sound->stopSound(resID + 1000, false);
                break;
            }

            case Enums::EV_LERPSPRITEPARABOLA_SCALE: {
                //printf("EV_LERPSPRITEPARABOLA_SCALE -> %d\n", this->IP);
                int intArg4 = this->getIntArg();
                int uShortArg29 = this->getUShortArg();
                int n135 = intArg4 >> 22 & 0x3FF;
                int n136 = intArg4 >> 17 & 0x1F;
                int n137 = intArg4 >> 12 & 0x1F;
                int height3 = (intArg4 >> 4 & 0xFF) - 48;
                int n138 = intArg4 & 0xF;
                short uByteArg34 = this->getUByteArg();
                LerpSprite* allocLerpSprite5 = this->app->game->allocLerpSprite(this, n135, (n138 & 0x2) != 0x0);
                if (allocLerpSprite5 == nullptr) {
                    return 0;
                }
                allocLerpSprite5->dstX = 32 + (n136 << 6);
                allocLerpSprite5->dstY = 32 + (n137 << 6);
                short n139 = this->app->render->mapSprites[this->app->render->S_ENT + n135];
                if (n139 != -1) {
                    this->app->game->entities[n139].info |= 0x400000;
                    if (this->app->game->entities[n139].monster != nullptr) {
                        this->app->game->entities[n139].monster->flags &= 0xFFFFBFFF;
                    }
                }
                allocLerpSprite5->srcX = this->app->render->mapSprites[this->app->render->S_X + n135];
                allocLerpSprite5->srcY = this->app->render->mapSprites[this->app->render->S_Y + n135];
                allocLerpSprite5->srcZ = this->app->render->mapSprites[this->app->render->S_Z + n135];
                allocLerpSprite5->dstZ = this->app->render->getHeight(allocLerpSprite5->dstX, allocLerpSprite5->dstY) + (allocLerpSprite5->srcZ - this->app->render->getHeight(allocLerpSprite5->srcX, allocLerpSprite5->srcY));
                allocLerpSprite5->srcScale = this->app->render->mapSprites[this->app->render->S_SCALEFACTOR + n135];
                allocLerpSprite5->dstScale = uByteArg34 << 1;
                allocLerpSprite5->height = height3;
                allocLerpSprite5->startTime = app->gameTime;
                allocLerpSprite5->travelTime = uShortArg29;
                allocLerpSprite5->calcDist();
                allocLerpSprite5->flags = (n138 & 0x3);
                allocLerpSprite5->flags |= Enums::LS_FLAG_PARABOLA;
                if (uShortArg29 == 0) {
                    if ((this->app->game->updateLerpSprite(allocLerpSprite5) & 0x1) != 0x0) {
                        this->app->canvas->invalidateRect();
                        break;
                    }
                    n = 0;
                    break;
                }
                else {
                    if (uShortArg29 != 0 && (n138 & 0x2) != 0x0) {
                        this->evWait(uShortArg29);
                    }
                    if (!(allocLerpSprite5->flags & Enums::LS_FLAG_ASYNC)) {
                        this->app->game->skipAdvanceTurn = true;
                        this->app->game->queueAdvanceTurn = false;
                        this->unpauseTime = -1;
                        n = 2;
                        break;
                    }
                    break;
                }
                break;
            }

            case Enums::EV_SET_CALDEX_RENDER_HACK: {
                //printf("EV_SET_CALDEX_RENDER_HACK -> %d\n", this->IP);
                this->app->render->useCaldexHack = (this->getByteArg() != 0);
                break;
            }

            case Enums::EV_END: { // EV_END
                if (this->stackPtr != 0) {
                    app->Error("The frame pointer should be zero if the script has completed.", 102);
                }
                return 1;
            }

            default: {
                app->Error("Cannot handle event: %d", mapByteCode[this->IP]);
                break;
            }
        }

        if (b) {
            this->app->game->scriptStateVars[7] = n2;
        }
        ++this->IP;
    }
    return n;
}

void ScriptThread::init() {
    //printf("ScriptThread::init\n");
    this->stackPtr = 0;
    this->IP = 0;
    this->FP = 0;
    this->unpauseTime = 0;
    this->state = 2;
    this->throwAwayLoot = false;
}

void ScriptThread::reset() {
    this->inuse = false;
    this->init();
}

int ScriptThread::attemptResume(int n) {
    if (this->stackPtr == 0) {
        return 1;
    }
    if (this->unpauseTime == -1 || n < this->unpauseTime) {
        return 2;
    }
    this->unpauseTime = 0;
    return this->run();
}

int ScriptThread::getIndex() {
    for (int i = 0; i < 20; ++i) {
        if (this == &this->app->game->scriptThreads[i]) {
            return i;
        }
    }
    return -1;
}

int ScriptThread::pop() {
    return this->scriptStack[--this->stackPtr];
}

void ScriptThread::push(bool b) {
    this->push(b ? 1 : 0);
}

void ScriptThread::push(int n) {
    this->scriptStack[this->stackPtr++] = n;
}

short ScriptThread::getUByteArg() {
    return (short)(this->app->render->mapByteCode[++this->IP] & 0xFF);
}

uint8_t ScriptThread::getByteArg() {
    return this->app->render->mapByteCode[++this->IP];
}

int ScriptThread::getUShortArg() {
    int n = ((this->app->render->mapByteCode[this->IP + 1] & 0xFF) << 8) | (this->app->render->mapByteCode[this->IP + 2] & 0xFF);
    this->IP += 2;
    return n;
}

short ScriptThread::getShortArg() {
    short n = (short)(this->app->render->mapByteCode[this->IP + 1] << 8 | (this->app->render->mapByteCode[this->IP + 2] & 0xFF));
    this->IP += 2;
    return n;
}

int ScriptThread::getIntArg() {
    int n = this->app->render->mapByteCode[this->IP + 1] << 24 | (this->app->render->mapByteCode[this->IP + 2] & 0xFF) << 16 | (this->app->render->mapByteCode[this->IP + 3] & 0xFF) << 8 | (this->app->render->mapByteCode[this->IP + 4] & 0xFF);
    this->IP += 4;
    return n;
}

void ScriptThread::composeLootDialog() {
    Text* largeBuffer = this->app->localization->getLargeBuffer();
    if (this->app->canvas->lootSource != -1) {
        this->app->localization->composeTextField(this->app->canvas->lootSource, largeBuffer);
        this->app->localization->composeText((short)0, (short)129, largeBuffer);
        this->app->canvas->lootSource = -1;
    }
    else {
        this->app->localization->composeText((short)0, (short)130, largeBuffer);
    }
    if (!this->throwAwayLoot) {
        this->app->canvas->showingLoot = true;
        this->app->canvas->setState(Canvas::ST_DIALOG);
    }
    int n = 0;
    uint8_t byteArg = this->getByteArg();
    int n2 = 0;
    for (uint8_t b = 0; b < byteArg; ++b) {
        int uShortArg = this->getUShortArg();
        int n3 = uShortArg >> 12 & 0xF;
        if (n3 == 6) {
            short n4 = (short)(uShortArg & 0xFFF);
            largeBuffer->append('\x88');
            this->app->localization->composeText(this->app->canvas->loadMapStringID, n4, largeBuffer);
            largeBuffer->append("|");
        }
        else if (n3 == 5) {
            this->app->player->updateQuests((short)(uShortArg & 0xFFF), 0);
        }
        else {
            int n5 = (uShortArg & 0xFC0) >> 6;
            int n6 = uShortArg & 0x3F;
            ++n;
            if (!this->throwAwayLoot) {
                if (n3 == 0) {
                    if (n5 == 24) {
                        n2 += n6;
                        continue;
                    }
                    if (n5 == 25) {
                        n2 += n6 * 100;
                        continue;
                    }
                }
                this->app->localization->resetTextArgs();
                this->app->localization->addTextArg('\x88');
                switch (n3) {
                case 0:
                case 3: {
                    this->app->player->give(n3, n5, n6, false);
                    EntityDef *find = this->app->entityDefManager->find(6, n3, n5);
                    this->app->localization->addTextArg(n6);
                    this->app->localization->addTextArg((short)1, find->longName);
                    this->app->localization->composeText((short)0, (short)90, largeBuffer);
                    break;
                }
                case 1: {
                    this->app->player->give(1, n5, n6, true);
                    int n7 = n5 * 9;
                    if (this->app->combat->weapons[n7 + 5] != 0) {
                        this->app->player->give(2, this->app->combat->weapons[n7 + 4], 10, true);
                    }
                    EntityDef* find = this->app->entityDefManager->find(6, n3, n5);
                    this->app->localization->addTextArg((short)1, find->longName);
                    this->app->localization->composeText((short)0, (short)91, largeBuffer);
                    break;
                }
                case 2: {
                    if (this->app->game->difficulty != 4) {
                        this->app->player->give(2, n5, n6, false);
                        EntityDef* find = this->app->entityDefManager->find(6, n3, n5);
                        this->app->localization->addTextArg(n6);
                        this->app->localization->addTextArg((short)1, find->longName);
                        this->app->localization->composeText((short)0, (short)90, largeBuffer);
                        break;
                    }
                    break;
                }
                }
            }
        }
    }
    if (n2 != 0) {
        this->app->player->give(0, 24, n2, false);
        this->app->localization->resetTextArgs();
        this->app->localization->addTextArg('\x88');
        this->app->localization->addTextArg(n2);
        this->app->localization->addTextArg((short)1, (short)157);
        this->app->localization->composeText((short)0, (short)90, largeBuffer);
    }
    if (!this->throwAwayLoot) {
        largeBuffer->setLength(largeBuffer->length() - 1);
        this->app->canvas->startDialog(this, largeBuffer, 4, 0, true);
    }
    else {
        largeBuffer->setLength(0);
        this->app->localization->resetTextArgs();
        this->app->localization->addTextArg(byteArg);
        this->app->localization->composeText((short)0, (short)145, largeBuffer);
        this->app->hud->addMessage(largeBuffer, 3);
    }
    largeBuffer->dispose();
    this->app->game->foundLoot(this->app->canvas->viewX + this->app->canvas->viewStepX, this->app->canvas->viewY + this->app->canvas->viewStepY, this->app->canvas->viewZ, n);
}

void ScriptThread::setAIGoal(Entity* entity, int n, int goalParam) {
    entity->monster->resetGoal();
    entity->monster->goalType = (uint8_t)n;
    if (n == 2 || n == 3) {
        entity->monster->goalParam = 1;
    }
    else if (n == 4 || n == 6) {
        entity->monster->goalParam = goalParam;
    }
    if (!this->app->player->noclip) {
        if ((entity->info & 0x40000) == 0x0) {
            this->app->game->activate(entity, true, false, false, true);
        }
        entity->aiThink(true);
    }
    if (n == 3) {
        entity->monster->goalFlags &= 0xf7;
        if (this->app->game->combatMonsters != nullptr) {
            this->app->combat->performAttack(this->app->game->combatMonsters, this->app->game->combatMonsters->monster->target, 0, 0, false);
        }
    }
}

void ScriptThread::corpsifyMonster(int x, int y, Entity* entity, bool b) {
    int sprite = entity->getSprite();
    this->app->game->snapLerpSprites(sprite);
    entity->monster->resetGoal();
    entity->monster->clearEffects();
    entity->undoAttack();
    this->app->game->deactivate(entity);
    this->app->render->mapSpriteInfo[sprite] = ((this->app->render->mapSpriteInfo[sprite] & 0xFFFE00FF) | 0x7000);
    this->app->render->mapSprites[this->app->render->S_X + sprite] = (short)x;
    this->app->render->mapSprites[this->app->render->S_Y + sprite] = (short)y;
    this->app->render->mapSprites[this->app->render->S_Z + sprite] = (short)(this->app->render->getHeight(x, y) + 32);
    this->app->render->relinkSprite(sprite);
    entity->info = ((entity->info & 0xFFFF) | 0x1000000 | 0x20000 | 0x400000);
    entity->def = this->app->entityDefManager->find(9, entity->def->eSubType, entity->def->parm);
    this->app->game->unlinkEntity(entity);
    this->app->game->linkEntity(entity, x >> 6, y >> 6);
    entity->checkMonsterDeath(false, b);
}
