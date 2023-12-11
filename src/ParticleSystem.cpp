#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "ParticleSystem.h"
#include "Graphics.h"
#include "Render.h"
#include "TinyGL.h"
#include "Canvas.h"
#include "Entity.h"
#include "EntityDef.h"
#include "Enums.h"

// ----------------------
// ParticleEmitter Class
// ----------------------

ParticleEmitter::ParticleEmitter() {
	std::memset(this, 0, sizeof(ParticleEmitter));
}

ParticleEmitter::~ParticleEmitter() {
}

bool ParticleEmitter::startup() {
	//printf("ParticleEmitter::startup\n");

	return false;
}

void ParticleEmitter::render(Graphics* graphics, int n) {
	Applet* app = CAppContainer::getInstance()->app;
    ParticleSystem* systems = this->systems;

    int n2 = n;
    int i = systems->particleNext[n] & 0xFF;
    if (i != 255) {
        int n3 = ((app->gameTime - this->startTime) << 10) / 1000;
        int n4 = n3 * n3 >> 10;
        int n5 = 64 * app->render->sinTable[app->render->viewAngle & 0x3FF] >> 12;
        int n6 = 64 * app->render->sinTable[app->render->viewAngle + 256 & 0x3FF] >> 12;
        TGLVert* tglVert = &app->tinyGL->mv[0];
        tglVert->x = this->pos[0] + n5;
        tglVert->y = this->pos[1] + n6;
        tglVert->z = this->pos[2] + 1024;
        TGLVert* tglVert2 = &app->tinyGL->mv[1];
        tglVert2->x = tglVert->x;
        tglVert2->y = tglVert->y;
        tglVert2->z = tglVert->z - 2048;
        TGLVert* tglVert3 = &app->tinyGL->mv[2];
        tglVert3->x = tglVert->x - (n5 << 1);
        tglVert3->y = tglVert->y - (n6 << 1);
        tglVert3->z = tglVert->z;
        TGLVert* transform3DVerts = app->tinyGL->transform3DVerts(app->tinyGL->mv, 3);
        if (transform3DVerts[0].w + transform3DVerts[0].z < 0 || transform3DVerts[1].w + transform3DVerts[1].z < 0 || transform3DVerts[2].w + transform3DVerts[2].z < 0) {
            return;
        }
        app->tinyGL->projectVerts(transform3DVerts, 3);
        int x = transform3DVerts[0].x;
        int x2 = transform3DVerts[2].x;
        int n7 = x >> 3;
        int n8 = x2 >> 3;
        int n9 = 8388607 / transform3DVerts[0].z;
        int n10 = (n7 + n8) >> 1;
        if (n10 < 0 || n10 >= app->tinyGL->screenWidth || n9 > app->tinyGL->columnScale[n10]) {
            return;
        }
        int n11 = transform3DVerts[1].x - transform3DVerts[2].x;
        int n12 = transform3DVerts[1].y - transform3DVerts[0].y;
        int n13 = 0;
        int n14 = 0;
        int a = (n11 << 7) / 128;
        int n15 = (n12 << 7) / 128;
        int n16 = n11 >> 3;
        int n17 = n12 >> 3;
        int n18 = n13 + (transform3DVerts[2].x >> 3);
        int n19 = n14 + (transform3DVerts[0].y >> 3);
        int* viewRect = app->canvas->viewRect;
        if (n18 < 0) {
            n16 -= -n18;
            n18 = 0;
        }
        if (n19 < 0) {
            n17 -= -n19;
            n19 = 0;
        }
        if (n16 + n18 > viewRect[2]) {
            n16 -= n18 + n16 - viewRect[2];
        }
        if (n17 + n19 > viewRect[3]) {
            n17 -= n19 + n17 - viewRect[3];
        }
        if (n16 < 0 || n17 < 0) {
            return;
        }
        graphics->clipRect(n18, n19, n16, n17);
        int n20 = n13 + ((transform3DVerts[1].x + transform3DVerts[2].x) >> 4);
        int n21 = n14 + ((transform3DVerts[0].y + transform3DVerts[1].y) >> 4);
        if (app->render->postProcessMode != 0) {
            this->color = app->render->convertToGrayscale(this->color);
        }
        graphics->setColor(this->color);
        while (i != 255) {
            int n22 = (int)systems->particleSize[i >> 1] >> ((i & 0x1) << 2);
            bool b = (n22 & 0x8) != 0x0;
            int n23 = n22 & 0x7;
            int n24 = ((char)systems->particleStartX[i] << 10) + systems->particleVelX[i] * n3;
            int n25 = ((char)systems->particleStartY[i] << 10) - (systems->particleVelY[i] * n3 + n4 * this->gravity);
            if (n24 < -65536 || n24 > 65536 || n25 > 65536 || n25 < -65536) {
                systems->linkParticle(63, systems->unlinkParticle(n2));
                i = n2;
            }
            else {
                int n26 = n20 + (n24 * a >> 20);
                int n27 = n21 + (n25 * n15 >> 20);
                if (!b) {
                    //graphics->fillCircle(n26, n27, n23 * std::min(a, 1024) >> 10); // J2ME
                    graphics->fillCircle(n26, n27, n23 * std::min(a, 1024) >> 11);
                }
                else {
                    //int n28 = std::min((a * 3) >> 10, 3); // J2ME
                    int n28 = std::min((a * 3) >> 11 /* Fixed Old ">> 10" */, 3);
                    if (n28 - 1 >= 0) {
                        int n29 = (((i << 6) + n3) >> 8) & 0x3;
                        if (systems->particleVelX[i] < 0) {
                            n29 = 3 - n29;
                        }
                        graphics->drawRegion(systems->imgGibs[this->gibType], (3 - n28) * 48, n23 * 48, 48, 48, n26, n27, 3, ParticleSystem::rotationSequence[n29], 0);
                    }
                }
            }
            n2 = i;
            i = (systems->particleNext[n2] & 0xFF);
        }
    }
}

// ---------------------
// ParticleSystem Class
// ---------------------

ParticleSystem::ParticleSystem() {
	std::memset(this, 0, sizeof(ParticleSystem));
}

ParticleSystem::~ParticleSystem() {
}

bool ParticleSystem::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	printf("ParticleSystem::startup\n");

	for (int i = 0; i < 4; i++) {
		this->particleEmitter[i].systems = this;
	}

	this->freeAllParticles();

	app->beginImageLoading();
	this->imgGibs[2] = app->loadImage("Skeletongibs_24.bmp", true);
	this->imgGibs[1] = app->loadImage("Stonegibs_24.bmp", true);
	this->imgGibs[0] = app->loadImage("woodgibs_24.bmp", true);
	app->endImageLoading();

	return true;
}

int ParticleSystem::unlinkParticle(int i) {
	int i2 = this->particleNext[i] & 0xFF;
	if (i2 != 0xff) {
		this->particleNext[i] = this->particleNext[i2];
		this->particleNext[i2] = 0xff;
	}
	return i2;
}

void ParticleSystem::linkParticle(int i, int i2) {
	this->particleNext[i2] = this->particleNext[i];
	this->particleNext[i] = (uint8_t)i2;
}

void ParticleSystem::freeAllParticles() {
	this->particleNext[63] = 0xff;
	for (int i = 0; i < 63; ++i) {
		if (i < 4) {
			this->particleNext[i] = 0xff;
		}
		else {
			this->linkParticle(63, i);
		}
	}
}

void ParticleSystem::freeSystem(int n) {
	for (int i = this->unlinkParticle(n); i != 255; i = this->unlinkParticle(n)) {
		this->linkParticle(63, i);
	}
}

void ParticleSystem::renderSystems(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->canvas->state == Canvas::ST_CAMERA) {
        this->clipRect[0] = app->canvas->cinRect[0];
        this->clipRect[1] = app->canvas->cinRect[1];
        this->clipRect[2] = app->canvas->cinRect[2];
        this->clipRect[3] = app->canvas->cinRect[3];
    }
    else {
        this->clipRect[0] = app->canvas->viewRect[0];
        this->clipRect[1] = app->canvas->viewRect[1];
        this->clipRect[2] = app->canvas->viewRect[2];
        this->clipRect[3] = app->canvas->viewRect[3];
    }

    this->clipRect[0] += 1;
    this->clipRect[1] += 1;
    this->clipRect[2] -= 2;
    this->clipRect[3] -= 2;
    for (int i = 0; i < 4; i++) {
        graphics->setScreenSpace(this->clipRect);
        this->particleEmitter[i].render(graphics, i);
        graphics->resetScreenSpace();
    }
}

void ParticleSystem::spawnMonsterBlood(Entity* entity, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    if (entity == nullptr) {
        return;
    }
    int sprite = entity->getSprite();
    uint16_t* palette = app->render->getPalette(app->render->mapSpriteInfo[sprite] & 0xFF, 0, 0);
    int palColor = palette[this->monsterColors[entity->def->eSubType]];
    int color = ((palColor & 0x7E0) << 5) | ((palColor & 0xF800) << 8) | ((palColor & 0x1F) << 3);

    short z = 0;
    if (entity->def->eType == Enums::ET_CORPSE) {
        z -= 26;
    }

    if (b && (ParticleSystem::GIB_BONE_MASK & 1 << entity->def->eSubType) != 0x0) {
        this->spawnParticles(4, color, app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], app->render->mapSprites[app->render->S_Z + sprite] + z);
    }
    else {
        this->spawnParticles(1, color, app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], app->render->mapSprites[app->render->S_Z + sprite] + z);
    }
}

void ParticleSystem::spawnParticles(int n, int n2, int n3) {
    Applet* app = CAppContainer::getInstance()->app;
    this->spawnParticles(n, n2, app->render->mapSprites[app->render->S_X + n3], app->render->mapSprites[app->render->S_Y + n3], app->render->mapSprites[app->render->S_Z + n3]);
}

void ParticleSystem::spawnParticles(int n, int color, int n2, int n3, int n4) {
    Applet* app = CAppContainer::getInstance()->app;
    int n5 = -64;
    int n6 = 64;
    int n7 = -64;
    int n8 = 64;
    int n9 = -128;
    int n10 = 128;
    int n11 = -32;
    int n12 = 32;
    int n13 = 4;
    int n14 = 7;
    int gravity = -196;
    int n15 = 15;
    int n16 = 0;
    int8_t gibType = -1;
    switch (n) {
    case 0: {
        n9 = 0;
        n10 = 0;
        n11 = 0;
        n12 = 0;
        gravity = 0;
        gibType = 0;
        n16 = 5;
        break;
    }
    case 1: {
        n5 >>= 2;
        n6 >>= 2;
        n7 >>= 2;
        n8 >>= 2;
        n11 = 96;
        n12 = 110;
        break;
    }
    case 7: {
        n5 >>= 3;
        n6 >>= 3;
        n7 >>= 3;
        n8 >>= 3;
        n9 = -128;
        n10 = 128;
        n11 = -128;
        n12 = 128;
        break;
    }
    case 5: {
        n5 >>= 3;
        n6 >>= 3;
        n7 >>= 3;
        n8 >>= 3;
        n15 = 8;
        n9 = -128;
        n10 = 128;
        n11 = -128;
        n12 = 128;
        break;
    }
    case 3: {
        n5 >>= 2;
        n6 >>= 2;
        n7 >>= 2;
        n8 >>= 2;
        n9 = -32;
        n10 = 32;
        n11 = 118;
        n12 = 128;
        n16 = 5;
        gibType = 1;
        break;
    }
    case 2: {
        n5 >>= 2;
        n6 >>= 2;
        n7 >>= 2;
        n8 >>= 2;
        n9 = -32;
        n10 = 32;
        n11 = 64;
        n12 = 78;
        n16 = 0;
        gibType = -1;
        break;
    }
    case 4: {
        n5 >>= 2;
        n6 >>= 2;
        n7 >>= 2;
        n8 >>= 2;
        n9 = -32;
        n10 = 32;
        n11 = 64;
        n12 = 78;
        n16 = 5;
        gibType = 2;
        break;
    }
    case 6: {
        n5 >>= 1;
        n6 >>= 1;
        n7 = -64;
        n8 = -40;
        n16 = 0;
        n9 = 0;
        n10 = 0;
        n11 = 0;
        n12 = -128;
        break;
    }
    case 8: {
        n5 >>= 2;
        n6 >>= 2;
        n7 = -15;
        n8 = 15;
        n15 = 8;
        n16 = 0;
        n9 = 0;
        n10 = -128;
        n11 = 0;
        n12 = -128;
        break;
    }
    }
    int systemIdx = this->systemIdx;
    this->systemIdx = (this->systemIdx + 1) % 4;
    ParticleEmitter* particleEmitter = &this->particleEmitter[systemIdx];
    this->freeSystem(systemIdx);
    particleEmitter->color = color;
    particleEmitter->gravity = gravity;
    particleEmitter->gibType = gibType;
    particleEmitter->startTime = app->gameTime;
    particleEmitter->pos[0] = (short)(n2 << 4);
    particleEmitter->pos[1] = (short)(n3 << 4);
    particleEmitter->pos[2] = (short)(n4 << 4);
    while (n16 > 0 || n15 > 0) {
        int unlinkParticle = this->unlinkParticle(63);
        if (unlinkParticle == 255) {
            return;
        }
        int n17 = 0;
        if (n15 > 0) {
            n17 = (n13 + ((n14 - n13) * app->nextByte() >> 8) & 0x7);
            --n15;
        }
        else if (n16 > 0) {
            n17 = (app->nextByte() % 4 | 0x8);
            --n16;
        }
        this->particleStartX[unlinkParticle] = (uint8_t)(n5 + ((n6 - n5) * app->nextByte() >> 8));
        this->particleStartY[unlinkParticle] = (uint8_t)(n7 + ((n8 - n7) * app->nextByte() >> 8));
        this->particleVelX[unlinkParticle] = (short)(n9 + ((n10 - n9) * app->nextByte() >> 8));
        this->particleVelY[unlinkParticle] = (short)(n11 + ((n12 - n11) * app->nextByte() >> 8));
        this->particleSize[unlinkParticle >> 1] &= (uint8_t)~(15 << ((unlinkParticle & 0x1) << 2));
        this->particleSize[unlinkParticle >> 1] |= (uint8_t)(n17 << ((unlinkParticle & 0x1) << 2));
        this->linkParticle(systemIdx, unlinkParticle);
    }
}
