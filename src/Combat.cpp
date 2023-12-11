#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "GLES.h"
#include "Combat.h"
#include "CombatEntity.h"
#include "Entity.h"
#include "EntityMonster.h"
#include "EntityDef.h"
#include "Player.h"
#include "Game.h"
#include "Canvas.h"
#include "Render.h"
#include "Hud.h"
#include "Text.h"
#include "Enums.h"
#include "ParticleSystem.h"
#include "Sound.h"

Combat::Combat() {
	std::memset(this, 0, sizeof(Combat));
}

Combat::~Combat() {
}

short Combat::getWeaponWeakness(int n, int n2, int n3) {
    return (short)((this->monsterWeakness[(n2 * 3 + n3) * 8 + n / 2] >> ((n & 0x1) << 2) & 0xF) + 1 << 5);
}

bool Combat::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	printf("Combat::startup\n");

	for (int n = 0, i = 0; i < 51; ++i, n += 6) {
		this->monsters[i] = new CombatEntity(5 * (this->monsterStats[n + 0] & 0xFF), this->monsterStats[n + 1], this->monsterStats[n + 2], this->monsterStats[n + 3], this->monsterStats[n + 4], this->monsterStats[n + 5]);
	}

	for (int j = 0; j < 16; j++) {
		this->tileDistances[j] = 64 * (j + 1) * (64 * (j + 1));
		//printf("this->tileDistances[%d] %d\n", j, this->tileDistances[j]);
	}

	this->gotCrit = false;
	this->gotHit = false;
	this->settingDynamite = false;
	this->oneShotCheat = false;

	return true;
}

void Combat::performAttack(Entity* curAttacker, Entity* curTarget, int attackX, int attackY, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    this->attackX = attackX;
    this->attackY = attackY;
    this->curAttacker = curAttacker;
    this->curTarget = curTarget;
    this->accumRoundDamage = 0;
    this->field_0x110_ = -1;
    if (curAttacker == nullptr || curTarget == nullptr) {
        app->player->updateStats();
    }
    if (curAttacker != nullptr) {
        curAttacker->info |= 0x200000;
    }
    if (curTarget != nullptr) {
        curTarget->info |= 0x200000;
    }
    if (curTarget == nullptr || (curAttacker == nullptr && curTarget->def->eType == 2)) {
        app->player->lastCombatTurn = app->player->totalMoves;
        app->player->inCombat = true;
    }
    if (curTarget != nullptr) {
        this->targetType = curTarget->def->eType;
        this->targetSubType = curTarget->def->eSubType;
        this->targetMonster = curTarget->monster;
    }
    else {
        this->targetType = (this->targetSubType = 0);
        this->targetMonster = nullptr;
    }
    if (this->curAttacker == nullptr) {
        this->attackerWeaponId = app->player->ce->weapon;
        this->attackerMonster = nullptr;
    }
    else {
        this->attackerMonster = this->curAttacker->monster;
        this->attackerWeaponId = this->attackerMonster->ce.weapon;
        if (this->attackerWeaponId == 18) {
            this->attackFrame = 64;
        }
        else if (this->attackerWeaponId == this->getMonsterField(this->curAttacker->def, 1)) {
            this->attackFrame = 80;
        }
        else {
            this->attackFrame = 64;
        }
    }
    this->attackerWeapon = this->attackerWeaponId * 9;
    if (this->punchingMonster > 0 && !this->punchMissed) {
        this->stage = -1;
        this->nextStage = 0;
        this->nextStageTime = app->gameTime + 300;
    }
    else {
        this->stage = 0;
        this->nextStageTime = 0;
    }
    this->animEndTime = 0;
    this->animLoopCount = this->weapons[this->attackerWeapon + 7];
    if (this->curAttacker == nullptr) {
        short n = app->player->ammo[this->weapons[this->attackerWeapon + 4]];
        int8_t b2 = this->weapons[this->attackerWeapon + 5];
        if (b2 > 0) {
            this->animLoopCount = std::min(n / b2, this->animLoopCount);
        }
    }
    this->attackerWeaponProj = this->weapons[this->attackerWeapon + 6];
    if (this->curAttacker != nullptr) {
        this->worldDist = this->curAttacker->distFrom(app->canvas->viewX, app->canvas->viewY);
        this->tileDist = this->WorldDistToTileDist(this->worldDist);
    }
    else {
        this->worldDist = this->curTarget->distFrom(app->canvas->viewX, app->canvas->viewY);
        this->tileDist = this->WorldDistToTileDist(this->worldDist);
    }
    short* mapSprites = app->render->mapSprites;
    if (this->attackerWeaponId == 18) {
        int sprite = this->curAttacker->getSprite();
        int n2 = mapSprites[app->render->S_X + sprite] - app->canvas->viewX;
        int n3 = mapSprites[app->render->S_Y + sprite] - app->canvas->viewY;
        int a = n2;
        int a2 = n3;
        if (a != 0) {
            a /= std::abs(a);
        }
        if (a2 != 0) {
            a2 /= std::abs(a2);
        }
        int n4 = std::abs(n2 + n3) >> 6;
        int n5 = app->canvas->viewX + a * 64;
        int n6 = app->canvas->viewY + a2 * 64;
        LerpSprite* allocLerpSprite = app->game->allocLerpSprite(nullptr, sprite, true);
        allocLerpSprite->startTime = app->gameTime;
        allocLerpSprite->travelTime = n4 * 200;
        allocLerpSprite->flags |= Enums::LS_FLAG_ENT_NORELINK;
        allocLerpSprite->srcScale = allocLerpSprite->dstScale = app->render->mapSprites[app->render->S_SCALEFACTOR + sprite];
        allocLerpSprite->srcX = mapSprites[app->render->S_X + sprite];
        allocLerpSprite->srcY = mapSprites[app->render->S_Y + sprite];
        allocLerpSprite->srcZ = mapSprites[app->render->S_Z + sprite];
        allocLerpSprite->dstX = n5 - a * 28;
        allocLerpSprite->dstY = n6 - a2 * 28;
        allocLerpSprite->dstZ = app->render->getHeight(allocLerpSprite->dstX, allocLerpSprite->dstY) + 32;
        app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | this->attackFrame << 8);
        this->curAttacker->monster->frameTime = 0x7FFFFFFF;//Integer.MAX_VALUE;
        app->game->unlinkEntity(this->curAttacker);
        app->game->linkEntity(this->curAttacker, n5 >> 6, n6 >> 6);
        this->stage = -1;
        this->nextStage = 0;
        this->nextStageTime = app->gameTime + allocLerpSprite->travelTime;
    }
    app->canvas->setState(Canvas::ST_COMBAT);
}

void Combat::checkMonsterFX() {
    Applet* app = CAppContainer::getInstance()->app;
    EntityMonster* monster = this->curTarget->monster;
    int monsterEffects = monster->monsterEffects;
    if (app->player->statusEffects[12] > 0) {
        monsterEffects |= (app->player->statusEffects[30] - 1 << 5 | 0x1);
    }
    monster->monsterEffects = monsterEffects;
}

int Combat::playerSeq() {
    Applet* app = CAppContainer::getInstance()->app;
    int animLoopCount = 0;
    bool b = false;
    if (this->nextStageTime != 0 && app->gameTime > this->nextStageTime && this->numActiveMissiles == 0 && app->game->animatingEffects == 0) {
        this->stage = this->nextStage;
        this->nextStageTime = 0;
        this->nextStage = -1;
        this->field_0x110_ = -1;
    }
    if (this->stage == 0) {
        this->isGibbed = false;
        this->totalDamage = 0;
        this->totalArmorDamage = 0;
        this->hitType = 0;
        this->deathAmt = 0;
        this->gotCrit = false;
        this->gotHit = false;
        this->crFlags = 0;
        this->damage = 0;
        this->targetKilled = false;
        this->flashTime = 1; // Old -> 0
        ++app->player->counters[6];
        if (this->punchingMonster == 1) {
            this->punchingMonster = 2;
        }
        if ((1 << this->attackerWeaponId & 0x77FF) == 0x0) {
            this->crFlags |= 0x40;
        }
        if (this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_AMMOTYPE] != 0) {
            b = true;
            if (this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_AMMOUSAGE] != 0) {
                animLoopCount = app->player->ammo[this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_AMMOTYPE]] / this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_AMMOUSAGE];
            }
        }
        if (b && this->targetType != 2 && this->attackerWeaponId != 14 && !app->player->weaponIsASentryBot(this->attackerWeaponId)) {
            if (animLoopCount == 2) {
                app->hud->addMessage((short)62);
            }
            else if (animLoopCount < 5 && animLoopCount > 1) {
                app->localization->resetTextArgs();
                app->localization->addTextArg(animLoopCount - 1);
                app->hud->addMessage((short)63);
            }
        }
        if (this->targetType == 2) {
            if ((1 << this->attackerWeaponId & 0x2) == 0x0) {
                this->crFlags |= 0x20;
            }
            app->player->ce->calcCombat(app->player->ce, this->curTarget, false, this->worldDist, this->targetSubType);
            if ((this->crFlags & 0x1007) != 0x0) {
                this->curTarget->info |= 0x4000000;
                if ((this->crFlags & 0x2) != 0x0) {
                    this->gotCrit = true;
                    this->hitType = 2;
                }
                else if ((this->crFlags & 0x1) != 0x0 || (this->crFlags & 0x4) != 0x0) {
                    this->hitType = 1;
                }
                this->gotHit = true;
                this->damage = this->crDamage;
                this->deathAmt = this->targetMonster->ce.getStat(0) - this->damage;
                if (this->damage > 0 && this->gotHit) {
                    app->player->counters[2] += this->damage;
                    if (this->damage > app->player->counters[1]) {
                        app->player->counters[1] = this->damage;
                    }
                }
            }
        }
        else {
            this->hitType = calcHit(this->curTarget);
            int damage = this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_STRMIN];
            int damageMax = this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_STRMAX];
            if (damage != damageMax) {
                damage += app->nextByte() % (damageMax - damage);
            }
            if (app->game->difficulty == 4) {
                damage -= damage >> 2;
            }
            this->damage = damage;
        }
        int n3 = -1;
        switch (this->attackerWeaponId) {
        case 3:
        case 5:
        case 9: {
            n3 = 1086;
            break;
        }
        case 2: {
            n3 = 1045;
            break;
        }
        case 7: {
            n3 = 1117;
            break;
        }
        case 0:
        case 8: {
            n3 = 1014;
            break;
        }
        case 10: {
            n3 = 1087;
            break;
        }
        case 11: {
            n3 = 1101;
            break;
        }
        case 12: {
            n3 = 1009;
            break;
        }
        case 13: {
            n3 = 1114;
            break;
        }
        case 14: {
            n3 = (app->player->characterChoice == 1) ? 1137 : 1136;
            break;
        }
        case 1: {
            n3 = 1015;
            break;
        }
        }
        if (n3 != -1) {
            app->sound->playCombatSound(n3, 0, 4);
        }
        this->totalDamage += this->damage;
        this->totalArmorDamage += this->crArmorDamage;
        this->animTime = this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_SHOTHOLD];
        if (app->player->statusEffects[2] > 0) {
            this->animTime *= 5;
        }
        else {
            this->animTime *= 10;
        }
        this->animStartTime = app->gameTime;
        this->animEndTime = this->animStartTime + this->animTime;
        this->flashDone = false;
        this->flashDoneTime = this->animStartTime + this->flashTime;
        if (b && animLoopCount < this->animLoopCount) {
            this->animLoopCount = animLoopCount;
        }
        this->launchProjectile();
        if ((1 << this->attackerWeaponId & 0x2406) == 0x0) {
            int n4 = 2;
            if (this->attackerWeaponId == 11) {
                n4 = 6;
            }
            app->render->rockView(this->animTime, app->canvas->viewX - n4 * (app->canvas->viewStepX >> 6), app->canvas->viewY - n4 * (app->canvas->viewStepY >> 6), app->canvas->viewZ);
        }
        if (this->totalDamage == 0) {
            if (this->targetType == 2) {
                if ((this->crFlags & 0x100) != 0x0) {
                    app->hud->addMessage((short)59, 1);
                }
                else if ((this->crFlags & 0x400) != 0x0) {
                    app->hud->addMessage((short)64);
                }
                else if ((this->crFlags & 0x1) == 0x0) {
                    app->hud->addMessage((short)68);
                }
            }
        }
        else if (this->targetType == 2) {
            this->accumRoundDamage += this->totalDamage;
        }
        else if (this->hitType != 0) {
            this->targetKilled = true;
        }
        this->stage = -1;
        app->hud->repaintFlags |= 0x4;
        //app->hud->repaintFlags |= 0x40; //J2ME
        int ammoUsage = this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_AMMOUSAGE];
        int ammoType = this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_AMMOTYPE];
        app->player->ammo[ammoType] -= ammoUsage;
        app->player->ammo[ammoType] = (short)std::max((int)app->player->ammo[ammoType], 0);
        this->nextStage = 1;
        this->nextStageTime = this->animEndTime;
        this->updateProjectile();
        app->canvas->invalidateRect();
        if (this->totalDamage == 0 || this->hitType == 0) {
            ++app->player->counters[7];
        }
    }
    else {
        if (this->stage == 1) {
            this->settingDynamite = false;
            this->dynamitePlaced = false;
            app->canvas->invalidateRect();
            if (this->targetType == 2) {
                this->curTarget->info &= 0xFBFFFFFF;
            }
            if (this->targetType == 9 && this->targetSubType == 17) {
                int sprite = this->curTarget->getSprite();
                app->game->executeTile(app->render->mapSprites[app->render->S_X + sprite] >> 6, app->render->mapSprites[app->render->S_Y + sprite] >> 6, 0xFF4 | app->canvas->flagForWeapon(this->attackerWeaponId), true);
            }
            if (this->attackerWeaponProj == 5) {
                this->checkForBFGDeaths(this->attackX >> 6, this->attackY >> 6);
            }
            int n6 = 4385;
            if (this->targetKilled || (this->targetType == 2 && this->targetMonster->ce.getStat(0) <= 0)) {
                this->curTarget->died(true, app->player->getPlayerEnt());
                this->targetKilled = true;
            }
            else if (--this->animLoopCount > 0 && (1 << this->targetType & n6) == 0x0 && !this->punchMissed && this->targetType != 10) {
                this->stage = 0;
                this->animEndTime = (this->animTime = 0);
                this->nextStageTime = 0;
                return 1;
            }
            if (this->punchingMonster > 0 && !this->targetKilled && !this->punchMissed) {
                int sprite = this->curTarget->getSprite();
                if (sprite != -1 && this->curTarget->monster != nullptr) {
                    app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x0);
                }
            }
            this->punchingMonster = 0;
            this->punchMissed = false;
            if (this->targetType == 2 && this->accumRoundDamage != 0 && (this->curTarget->info & 0x20000) != 0x0) {
                Text* messageBuffer = app->hud->getMessageBuffer();
                app->localization->resetTextArgs();
                if (this->gotCrit) {
                    app->localization->composeText((short)0, (short)70, messageBuffer);
                }
                app->localization->addTextArg(this->accumRoundDamage);
                app->localization->composeText((short)0, (short)71, messageBuffer);
                app->hud->finishMessageBuffer();
            }
            if (this->attackerWeaponId == 14) {
                app->player->weapons &= 0xFFFFBFFF;
                app->player->selectWeapon(app->player->currentWeaponCopy);
            }
            return 0;
        }
        if (this->stage == -1) {
            app->canvas->staleView = true;
            if (this->numActiveMissiles != 0) {
                this->updateProjectile();
            }
        }
    }
    return 1;
}

int Combat::monsterSeq() {
    Applet* app = CAppContainer::getInstance()->app;
    int sprite = this->curAttacker->getSprite();
    int n = app->render->mapSpriteInfo[sprite];
    int n2 = (n & 0xFF00) >> 8;
    int n3 = n2 & 0xF0;
    int n4 = n2 & 0xF;
    if ((n3 == 64 || n3 == 80) && n4 == 0) {
        if (app->time >= this->curAttacker->monster->frameTime) {
            this->curAttacker->monster->frameTime = app->time + this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_SHOTHOLD] * 10;
            this->nextStageTime = app->gameTime + this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_SHOTHOLD] * 10;
            app->render->mapSpriteInfo[sprite] = ((n & 0xFFFF00FF) | (n3 | n4 + 1) << 8);
            this->launchProjectile();
        }
    }
    else if (this->nextStageTime != 0 && app->gameTime > this->nextStageTime && this->numActiveMissiles == 0 && app->game->animatingEffects == 0) {
        if (this->reflectionDmg > 0 && this->curAttacker != nullptr && this->curAttacker->monster != nullptr) {
            app->particleSystem->spawnMonsterBlood(this->curAttacker, false);
            this->curAttacker->pain(this->reflectionDmg / 2, app->player->getPlayerEnt());
            this->reflectionDmg = 0;
            if (this->curAttacker->monster->ce.getStat(Enums::STAT_HEALTH) <= 0) {
                this->curAttacker->died(true, app->player->getPlayerEnt());
            }
            else {
                this->curAttacker->monster->frameTime = app->time + 250;
                this->nextStageTime = app->gameTime + 250;
            }
        }
        else {
            this->stage = this->nextStage;
            this->nextStageTime = 0;
            this->nextStage = -1;
        }
        this->field_0x110_ = -1;
    }
    if (this->stage == 0) {
        this->totalDamage = 0;
        this->totalArmorDamage = 0;
        this->hitType = 0;
        this->crFlags = 0;
        this->gotCrit = false;
        this->gotHit = false;
        this->targetKilled = false;
        this->reflectionDmg = 0;
#if 0 // J2ME
        if (this->curAttacker->def->eSubType == 11 || (this->curAttacker->def->eSubType == 14 && this->curAttacker->def->parm != 0)) {
            Sound.playSound(15);
        }
        else {
            Sound.playSound(5);
        }
#endif
        int soundType;
        if (this->attackerWeaponId == this->getMonsterField(this->curAttacker->def, 1)) {
            soundType = Enums::MSOUND_ATTACK2;
        }
        else {
            soundType = Enums::MSOUND_ATTACK1;
        }

        if (this->weapons[this->attackerWeapon + 1] > 0) {
            if (this->curTarget == nullptr) {
                app->player->ce->calcCombat(&this->attackerMonster->ce, app->player->getPlayerEnt(), true, app->player->distFrom(this->curAttacker), -1);
                if ((this->crFlags & 0x1007) != 0x0) {
                    if ((this->crFlags & 0x2) != 0x0) {
                        this->gotCrit = true;
                        this->hitType = 2;
                    }
                    else {
                        this->hitType = 1;
                    }
                    this->gotHit = true;
                    this->damage = this->crDamage;
                }
                else {
                    this->damage = 0;
                }
            }
            else {
                this->hitType = 1;
                this->damage = 1;
            }
            this->totalDamage += this->damage;
            this->totalArmorDamage += this->crArmorDamage;
        }
        int monsterSound;
        if ((this->attackerWeaponId == 17) && !(this->crFlags & Enums::CR_HITMASK)) {
            monsterSound = 1100; // "Revenant_swing.wav"
        }
        else if (this->attackerWeaponId == 28) {
            monsterSound = 1087; // "plasma.wav"
        }
        else {
            monsterSound = app->game->getMonsterSound(this->curAttacker->def->eSubType, this->curAttacker->def->parm, soundType);
        }
        app->sound->playCombatSound(monsterSound, 0, 4);

        --this->animLoopCount;
        this->animTime = this->weapons[this->attackerWeapon + Combat::WEAPON_FIELD_SHOTHOLD] * 10;
        this->animStartTime = app->gameTime;
        this->animEndTime = this->animStartTime + this->animTime;
        if (this->attackerWeaponId != 18) {
            app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | this->attackFrame << 8);
            this->curAttacker->monster->frameTime = app->time + this->animTime;
        }
        if (this->curTarget != nullptr) {
            this->targetKilled = true;
        }
        if (this->attackerWeaponId == 18) {
            LerpSprite* lerpSprite = app->game->allocLerpSprite(nullptr, sprite, true);
            lerpSprite->startTime = app->gameTime;
            lerpSprite->travelTime = 500;
            lerpSprite->flags |= Enums::LS_FLAG_ENT_NORELINK;
            lerpSprite->srcScale = lerpSprite->dstScale = app->render->mapSprites[app->render->S_SCALEFACTOR + sprite];
            lerpSprite->srcX = app->render->mapSprites[app->render->S_X + sprite];
            lerpSprite->srcY = app->render->mapSprites[app->render->S_Y + sprite];
            lerpSprite->srcZ = app->render->mapSprites[app->render->S_Z + sprite];
            lerpSprite->dstX = (this->curAttacker->linkIndex % 32 << 6) + 32;
            lerpSprite->dstY = (this->curAttacker->linkIndex / 32 << 6) + 32;
            lerpSprite->dstZ = app->render->getHeight(lerpSprite->dstX, lerpSprite->dstY) + 32;
            lerpSprite->calcDist();
            app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x0);
            this->curAttacker->monster->frameTime = app->time + lerpSprite->travelTime;
            this->launchProjectile();
        }
        if (this->gotHit && (this->attackerWeaponId == 23 || this->curAttacker->def->eSubType == 2)) {
            app->player->addStatusEffect(13, 5, 3);
            app->player->translateStatusEffects();
        }
        this->stage = -1;
        if (this->animLoopCount <= 0) {
            this->nextStage = 1;
        }
        else {
            this->nextStage = 0;
        }
        this->nextStageTime = this->animEndTime;
        if (app->render->isImp(this->curAttacker->def->tileIndex) && this->attackerWeaponId == 16 && this->animLoopCount == 0) {
            app->render->mapSpriteInfo[sprite] ^= 0x20000;
        }
        this->updateProjectile();
        if ((this->crFlags & 0x80) != 0x0) {
            app->hud->addMessage((short)67);
        }
        app->canvas->invalidateRect();
    }
    else {
        if (this->stage == 1) {
            if (this->curAttacker->def->eType == Enums::ET_MONSTER) {
                app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x0);
            }
            if (this->targetKilled) {
                this->curTarget->died(false, nullptr);
            }
            app->localization->resetTextArgs();
            if (this->accumRoundDamage > 0 && app->player->buffs[0] > 0 && !this->curAttacker->isBoss()) {
                app->localization->addTextArg(this->accumRoundDamage);
                app->hud->addMessage((short)0, (short)131);
            }
            else if (this->accumRoundDamage > 0) {
                app->localization->addTextArg((short)1, (short)(this->curAttacker->def->name & 0x3FF));
                app->localization->addTextArg(this->accumRoundDamage);
                app->hud->addMessage((short)0, (short)112);
            }
            app->canvas->shakeTime = 0;
            app->hud->damageDir = 0;
            app->hud->damageTime = 0;
            this->attackerMonster->flags |= 0x400;
            app->game->gsprite_clear(64);
            app->canvas->invalidateRect();
            return 0;
        }
        if (this->stage == -1) {
            app->canvas->staleView = true;
            if (this->numActiveMissiles != 0 || this->exploded) {
                this->updateProjectile();
            }
        }
    }
    if (app->canvas->state == Canvas::ST_DYING || app->canvas->state == Canvas::ST_BOT_DYING) {
        return 0;
    }
    return 1;
}

void Combat::drawEffects() {
    Applet* app = CAppContainer::getInstance()->app;
    if (app->player->statusEffects[13] > 0) {
        int n = 131072;
        int n2 = ((176 * n) / 65536);
        app->render->draw2DSprite(234, app->time / 256 & 0x3, (app->canvas->viewRect[2] / 2) - (n2 / 2), app->canvas->viewRect[3] - ((n2 / 4) + 132), 0, 3, 0, n);
    }
}

void Combat::drawWeapon(int sx, int sy) {
    Applet* app = CAppContainer::getInstance()->app;
    bool b = false;
    int frame = 0;
    int renderFlags = 0;
    int flags = 0;
    int scrX = (Applet::IOS_WIDTH / 2) - 44;
    int scrY = (Applet::IOS_HEIGHT / 2) - 29;
    int loweredWeaponY= Combat::LOWEREDWEAPON_Y;

    if (!app->render->_gles->isInit) {  // [GEC] Adjusted like this to match the XY position on the GL version
        scrX = (app->render->screenWidth / 2) - 62;
        scrY = (app->render->screenHeight / 2) - 18;
        loweredWeaponY += 13;
        renderFlags |= Render::RENDER_FLAG_SCALE_WEAPON; // [GEC]
        switch (app->player->ce->weapon)
        {
        case 1:
            scrX -= 7;
            scrY += 4;
            break;
        case 2:
            scrY += 3;
            break;
        case 3: // Shooting Bot Green
        case 4: // Exploding Bot Green
        case 5: // Shooting Bot Red
        case 6: // Exploding Bot Red
            scrX -= 4;
            scrY += 12;
            break;
        case 8:
            scrX -= 3;
            break;
        case 7:
            scrX += 1;
            break;
        case 10:
        case 11:
            scrX += 3;
            break;
        case 12:
            scrX -= 11;
            break;
        case 13:
            scrX -= 3;
            break;
        }
    }

    this->renderTime = app->upTimeMs;
    if (this->punchingMonster == 0 && !this->lerpingWeapon && this->weaponDown) {
        scrY += loweredWeaponY;
    }

    bool b2 = app->canvas->state == Canvas::ST_CAMERA && app->game->cinematicWeapon != -1;
    int weapon = app->player->ce->weapon;

    switch (weapon)
    {
    case 1:
        scrY += 3;
        break;
    case 2:
        scrY += 10;
        break;
    case 3:
    case 4:
    case 5:
    case 6:
        scrY += 12;
        break;
    }

    if (app->player->isFamiliar) {
        if (app->player->familiarType != 1 && app->player->familiarType != 3) {
            return;
        }
        weapon = 0;
    }
    if (b2) {
        weapon = app->game->cinematicWeapon;
        scrY -= app->canvas->CAMERAVIEW_BAR_HEIGHT;
    }
    int wpnField = weapon * Combat::FIELD_COUNT;
    if (app->canvas->state == Canvas::ST_DYING || app->player->weapons == 0 || weapon == -1) {
        return;
    }
    sy = -std::abs(sy);
    if (!b2 && app->time < app->hud->damageTime && app->hud->damageCount >= 0 && this->totalDamage > 0) {
        renderFlags |= Render::RENDER_FLAG_BRIGHTREDSHIFT;
    }

    int wpIdleX = this->wpinfo[wpnField + Combat::FLD_WPIDLEX];
    int wpIdleY = this->wpinfo[wpnField + Combat::FLD_WPIDLEY];
    int wpAtkX = this->wpinfo[wpnField + Combat::FLD_WPATKX];
    int wpAtkY = this->wpinfo[wpnField + Combat::FLD_WPATKY];

    if (app->player->isFamiliar && (app->player->familiarType == 1 || app->player->familiarType == 3)) {
        wpAtkX = 1 + (wpAtkX - wpIdleX);
        wpAtkY = 50 + (wpAtkY - wpIdleY);
        wpIdleX = 1;
        wpIdleY = 50;
    }
    int wpFlashX = this->wpinfo[wpnField + Combat::FLD_WPFLASHX];
    int wpFlashY = this->wpinfo[wpnField + Combat::FLD_WPFLASHY];

    int wpX = wpIdleX;
    int wpY = wpIdleY;
    int gameTime = app->gameTime;
    int n14 = 1 << weapon;
    if ((1 << weapon & 0x0) != 0x0 && (this->punchingMonster == 2 || this->punchingMonster == 3)) {
        b = true;
    }
    bool b5 = b2 || (app->canvas->state == Canvas::ST_COMBAT && this->curAttacker == nullptr && this->nextStage == 1);
    bool b6 = false;
    if (b5) {
        wpX = wpAtkX;
        wpY = wpAtkY;
        if (!this->flashDone) {
            b = ((n14 & 0x200) == 0x0);
            if (gameTime >= this->flashDoneTime) {
                this->flashDone = true;
            }
        }
        else {
            int n15 = (gameTime - this->animStartTime << 16) / this->animTime;
            if (n15 >= 65536) {
                wpX = wpIdleX;
                wpY = wpIdleY;
            }
            else if (weapon == 1) {
                if (n15 < 43690) {
                    wpX = wpAtkX + ((n15 & 0x8) ?  2 : 0);
                    wpY = wpAtkY + ((n15 & 0x8) ? -1 : 0);
                }
                else {
                    b6 = true;
                    wpX = 3 * ((65536 - n15) * wpAtkX + (n15 - 43690) * wpIdleX) >> 16;
                    wpY = 3 * ((65536 - n15) * wpAtkY + (n15 - 43690) * wpIdleY) >> 16;
                }
            }
            else {
                wpX = (65536 - n15) * wpAtkX + n15 * wpIdleX >> 16;
                wpY = (65536 - n15) * wpAtkY + n15 * wpIdleY >> 16;
            }
        }
    }
    else if (this->punchingMonster == 0 && this->lerpingWeapon) {
        if (this->lerpWpDown && this->lerpWpStartTime + this->lerpWpDur > gameTime) {
            scrY += (gameTime - this->lerpWpStartTime << 16) / this->lerpWpDur * loweredWeaponY >> 16;
        }
        else if (!this->lerpWpDown && this->lerpWpStartTime + this->lerpWpDur > gameTime) {
            scrY += (65536 - (gameTime - this->lerpWpStartTime << 16) / this->lerpWpDur) * loweredWeaponY >> 16;
        }
        else {
            this->lerpWpStartTime = 0;
            this->lerpingWeapon = false;
            this->weaponDown = this->lerpWpDown;
            if (this->lerpWpDown) {
                scrY += loweredWeaponY;
            }
        }
    }
    int renderMode = Render::RENDER_NORMAL;
    int x = scrX + (wpX + sx);
    int y = scrY - (wpY + sy);

    app->render->_gles->SetGLState();
    if (app->player->isFamiliar) {
        y -= 35;
        if (!app->render->_gles->isInit) {  // [GEC] Adjusted like this to match the XY position on the GL version
            x += 8;
            y -= 7;
        }
    }
    if (app->player->weaponIsASentryBot(weapon)) {
        int n21;
        if (weapon == 4 || weapon == 3) {
            n21 = 19;
        }
        else {
            n21 = 18;
        }
        int n22 = flags;
        int n23 = (app->time + n21 * 1337) / 1024 & 0xF;
        if (n23 == 0 || (n23 >= 4 && n23 <= 7) || (n23 >= 10 && n23 <= 12)) {
            n22 ^= 0x20000;
        }
        int n24 = (n23 & 0x2) ? 1 : 2;
        app->render->draw2DSprite(n21, 2, x, y, flags, renderMode, renderFlags, 0x10000);
        app->render->draw2DSprite(n21, 3, x, y + n24, n22, renderMode, renderFlags, 0x10000);
    }
    else if (weapon == 14) {
        if (app->player->ammo[8] > 0) {
            app->render->draw2DSprite(app->player->activeWeaponDef->tileIndex, 1, x + wpFlashX, y + wpFlashY, flags, renderMode, renderFlags, 0x10000);
        }
        else {
            app->render->draw2DSprite(this->getWeaponTileNum(weapon), 0, x, y, flags, renderMode, renderFlags, 0x10000);
        }
    }
    else {
        if (b6 | ((weapon == 13 || weapon == 8) && b5)) {
            frame = 1;
        }
        if (b && (1 << weapon & 0x181) != 0x0) {
            int xf = 40;
            int yf = 40;
            if (!app->render->_gles->isInit) {  // [GEC] Adjusted like this to match the XY position on the GL version
                xf += 15;
                yf += 20;
            }
            app->render->draw2DSprite(this->getWeaponTileNum(0), 3, x + wpFlashX + xf, y + wpFlashY + yf, flags, 5, (!app->render->_gles->isInit) ? 0x400 : 0, 0x8000);
        }
        if (weapon == 9) {
            app->render->draw2DSprite(this->getWeaponTileNum(0), frame, x, y, flags, renderMode, renderFlags, 0x10000);
        }

        app->render->draw2DSprite(this->getWeaponTileNum(weapon), frame, x, y, flags, renderMode, renderFlags, 0x10000);
    }
    app->render->_gles->ResetGLState();
    this->drawEffects();
    this->renderTime = app->upTimeMs - this->renderTime;
}

void Combat::shiftWeapon(bool lerpWpDown) {
    Applet* app = CAppContainer::getInstance()->app;
    if (lerpWpDown == this->lerpWpDown || lerpWpDown == this->weaponDown) {
        return;
    }
    this->lerpingWeapon = true;
    this->lerpWpDown = lerpWpDown;
    this->lerpWpStartTime = app->gameTime;
    this->lerpWpDur = Combat::LOWERWEAPON_TIME;
}

int Combat::runFrame() {
    if (this->curAttacker == nullptr) {
        return this->playerSeq();
    }
    return this->monsterSeq();
}

int Combat::calcHit(Entity* entity) {
    Applet* app = CAppContainer::getInstance()->app;
    int n = app->player->ce->weapon * 9;
    int worldDistToTileDist = this->WorldDistToTileDist(entity->distFrom(app->canvas->destX, app->canvas->destY));
    int8_t b = this->weapons[n + 3];
    if (worldDistToTileDist < this->weapons[n + 2] || worldDistToTileDist > b) {
        this->crFlags |= 0x400;
        return 0;
    }
    if ((entity->info & 0x20000) == 0x0) {
        return 0;
    }
    if (entity->def->eType != Enums::ET_ATTACK_INTERACTIVE) {
        return 1;
    }
    if ((this->tableCombatMasks[entity->def->parm] & 1 << app->player->ce->weapon) != 0x0) {
        return 1;
    }
    return 0;
}

void Combat::explodeOnMonster() {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->explodeThread != nullptr) {
        this->explodeThread->run();
        this->explodeThread = nullptr;
    }
    app->render->shotsFired = true;
    if ((1 << this->attackerWeaponId & 0x2) != 0x0 && this->hitType == 0) {
        app->render->shotsFired = false;
    }
    else if (this->curTarget->monster != nullptr && this->curTarget->def->eType == Enums::ET_MONSTER && (this->curTarget->info & 0x40000) == 0x0) {
        app->game->activate(this->curTarget, true, false, true, true);
    }
    if (this->hitType == 0) {
        if (this->targetType != 2 && this->attackerWeaponId == 11) {
            this->radiusHurtEntities(this->attackX >> 6, this->attackY >> 6, 1, this->damage + this->crArmorDamage >> 2, app->player->getPlayerEnt(), nullptr);
        }
        return;
    }
    if (this->targetType == 2) {
        if (this->totalDamage > 0) {
            this->checkMonsterFX();
            bool pain = this->curTarget->pain(this->totalDamage, app->player->getPlayerEnt());
            if (this->attackerWeaponId != 14) {
                app->particleSystem->spawnMonsterBlood(this->curTarget, false);
            }
            if (this->targetMonster->ce.getStat(Enums::STAT_HEALTH) > 0) {
                int n = 0;
                if (0x0 != (this->targetMonster->monsterEffects & 0x2) || (this->targetMonster->flags & 0x1000) != 0x0 || pain) {
                    n = 0;
                }
                if (n > 0) {
                    app->player->unlink();
                    this->curTarget->knockback(app->canvas->viewX, app->canvas->viewY, n);
                    app->player->link();
                }
            }
            if (this->attackerWeaponId == 11 || this->attackerWeaponId == 12) {
                this->radiusHurtEntities(this->attackX >> 6, this->attackY >> 6, 1, this->crDamage >> ((this->attackerWeaponId == 12) ? 1 : 2), app->player->getPlayerEnt(), this->curTarget);
            }
        }
        else if (this->totalDamage < 0) {
            if (this->targetMonster->ce.getStat(Enums::STAT_HEALTH) - this->totalDamage > this->targetMonster->ce.getStat(Enums::STAT_MAX_HEALTH)) {
                this->totalDamage = -(this->targetMonster->ce.getStat(Enums::STAT_MAX_HEALTH) - this->targetMonster->ce.getStat(Enums::STAT_HEALTH));
            }
            this->targetMonster->ce.setStat(Enums::STAT_HEALTH, this->targetMonster->ce.getStat(Enums::STAT_HEALTH) - this->totalDamage);
        }
    }
    else if (this->targetType == 10) {
        if (this->totalDamage > 0) {
            this->curTarget->pain(this->totalDamage, app->player->getPlayerEnt());
        }
    }
    else if (this->targetType == 9) {
        int sprite = this->curTarget->getSprite();
        app->render->mapSpriteInfo[sprite] |= 0x10000;
        this->isGibbed = true;
        app->particleSystem->spawnMonsterBlood(this->curTarget, this->isGibbed);
        app->game->spawnDropItem(this->curTarget);
        app->sound->playSound(1037, 0, 3, 0);
    }
}

void Combat::explodeOnPlayer() {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->curTarget != nullptr) {
        return;
    }
    int sprite = this->curAttacker->getSprite();
    int viewAngle = app->canvas->viewAngle;
    if (app->canvas->isZoomedIn) {
        viewAngle += app->canvas->zoomAngle;
    }
    app->hud->damageDir = app->player->calcDamageDir(app->canvas->viewX, app->canvas->viewY, viewAngle, app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite]);
    if (this->animLoopCount > 0) {
        app->hud->damageTime = app->time + 150;
    }
    else {
        app->hud->damageTime = app->time + 1000;
    }
    if (app->hud->damageDir != 3) {
        app->canvas->staleTime = app->hud->damageTime + 1;
    }
    bool b = false;
    if (app->player->buffs[0] > 0 && !this->curAttacker->isBoss()) {
        b = true;
    }
    if (this->gotHit && (this->totalDamage > 0 || this->totalArmorDamage > 0)) {
        app->player->painEvent(this->curAttacker, true);
        if (this->curAttacker != nullptr) {
            Entity* entity = &app->game->entities[1];
            app->game->unlinkEntity(this->curAttacker);
            if (this->attackerWeaponProj == 7) {
                app->player->addStatusEffect(13, 5, 3);
                app->player->translateStatusEffects();
            }
            app->player->addArmor(-this->totalArmorDamage);
            if (this->totalDamage > 0) {
                if (!b) {
                    if (app->game->difficulty != 1) {
                        int loadMapID = app->canvas->loadMapID;
                        if (app->game->difficulty == 4) {
                            loadMapID *= 2;
                        }
                        this->totalDamage += (this->totalDamage >> 1) + loadMapID / this->weapons[this->attackerWeapon + 7];
                    }
                    EntityDef* def = this->curAttacker->def;
                    if (def->eType == Enums::ET_MONSTER && def->eSubType == Enums::BOSS_VIOS) {
                        if (def->parm == 4) {
                            this->totalDamage += app->game->numMallocsForVIOS * 8;
                        }
                        else if (app->game->angryVIOS) {
                            this->totalDamage += 8;
                        }
                    }
                    this->accumRoundDamage += this->totalDamage;
                    app->player->pain(this->totalDamage, this->curAttacker, false);
                    if (app->player->ce->getStat(Enums::STAT_HEALTH) == 0) {
                        return;
                    }
                    if (app->canvas->knockbackDist == 0) {
                        int* calcPosition = this->curAttacker->calcPosition();
                        int a = app->canvas->viewX - calcPosition[0];
                        int a2 = app->canvas->viewY - calcPosition[1];
                        if (a != 0) {
                            a /= std::abs(a);
                        }
                        if (a2 != 0) {
                            a2 /= std::abs(a2);
                        }
                        app->render->rockView(200, app->canvas->viewX + a * 6, app->canvas->viewY + a2 * 6, app->canvas->viewZ);
                    }
                    if (app->player->ce->getStat(Enums::STAT_HEALTH) > 0 && this->curAttacker->def->eSubType == 5 && this->attackerWeaponId == 18) {
                        entity->knockback(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], 1);
                    }
                }
                else if (this->curAttacker->monster != nullptr) {
                    this->accumRoundDamage = (this->reflectionDmg = this->totalDamage);
                    this->animLoopCount = 0;
                    this->nextStage = 1;
                }
            }
            else {
                app->hud->damageCount = 0;
                this->totalDamage = 0;
            }
            if (this->totalDamage <= 0) {
                app->hud->addMessage((short)72);
            }
            app->game->linkEntity(this->curAttacker, this->curAttacker->linkIndex % 32, this->curAttacker->linkIndex / 32);
            if (app->player->isFamiliar && app->player->ammo[7] <= 0) {
                app->player->familiarDying(false);
            }
        }
    }
    else {
        app->hud->damageCount = 0;
        if ((this->crFlags & 0x100) != 0x0) {
            app->localization->resetTextArgs();
            app->hud->addMessage((short)73);
        }
        else if ((this->crFlags & 0x80) == 0x0) {
            app->localization->resetTextArgs();
            Text* smallBuffer = app->localization->getSmallBuffer();
            app->localization->composeTextField(this->curAttacker->name, smallBuffer);
            app->localization->addTextArg(smallBuffer);
            app->hud->addMessage((short)69);
            smallBuffer->dispose();
        }
        app->hud->damageCount = -1;
    }
}

int Combat::getMonsterField(EntityDef* entityDef, int n) {
    return this->monsterAttacks[(entityDef->eSubType * 9) + (entityDef->parm * 3) + n];
}

void Combat::checkForBFGDeaths(int x, int y) {
    Applet* app = CAppContainer::getInstance()->app;
    int n3 = (x << 6) + 32;
    int n4 = (y << 6) + 32;
    int n5 = app->render->getHeight(x << 6, y << 6) + 32;
    for (int i = y - 1; i <= y + 1; ++i) {
        for (int j = x - 1; j <= x + 1; ++j) {
            if (j != x || i != y) {
                app->game->trace(n3, n4, n5, (j << 6) + 32, (i << 6) + 32, app->render->getHeight(j << 6, i << 6) + 32, nullptr, 4129, 1, false);
                if (app->game->traceEntity == nullptr) {
                    Entity* nextOnTile;
                    for (Entity* mapEntity = app->game->findMapEntity(j << 6, i << 6, 30381); mapEntity != nullptr; mapEntity = nextOnTile) {
                        nextOnTile = mapEntity->nextOnTile;
                        if (mapEntity->monster != nullptr && mapEntity->monster->ce.getStat(0) <= 0 && mapEntity->def->eType != 9) {
                            mapEntity->died(true, app->player->getPlayerEnt());
                        }
                    }
                }
            }
        }
    }
}

void Combat::radiusHurtEntities(int n, int n2, int n3, int n4, Entity* entity, Entity* entity2) {
    Applet* app = CAppContainer::getInstance()->app;
    int n5 = (n << 6) + 32;
    int n6 = (n2 << 6) + 32;
    int n7 = app->render->getHeight(n << 6, n2 << 6) + 32;
    for (int i = n2 - 1; i <= n2 + 1; ++i) {
        for (int j = n - 1; j <= n + 1; ++j) {
            app->game->trace(n5, n6, n7, (j << 6) + 32, (i << 6) + 32, app->render->getHeight(j << 6, i << 6) + 32, nullptr, 4129, 1, true);
            if (app->game->traceEntity == nullptr) {
                this->hurtEntityAt(j, i, n, n2, (j == n && i == n2) ? 0 : n3, n4, entity, entity2);
            }
        }
    }
    for (int k = n2 - 1; k <= n2 + 1; ++k) {
        for (int l = n - 1; l <= n + 1; ++l) {
            int n8 = 0x4004 | app->game->eventFlagForDirection(n - l, n2 - k);
            if (app->game->doesScriptExist(l, k, n8)) {
                ScriptThread* allocScriptThread = app->game->allocScriptThread();
                allocScriptThread->queueTile(l, k, n8);
                allocScriptThread->run();
            }
        }
    }
}

void Combat::hurtEntityAt(int n, int n2, int n3, int n4, int n5, int n6, Entity* entity, Entity* entity2) {
    this->hurtEntityAt(n, n2, n3, n4, n5, n6, entity, false, entity2);
}

void Combat::hurtEntityAt(int n, int n2, int n3, int n4, int n5, int n6, Entity* entity, bool b, Entity* entity2) {
    Applet* app = CAppContainer::getInstance()->app;
    this->crFlags = 16;
    app->render->shotsFired = true;
    Entity* playerEnt = app->player->getPlayerEnt();
    Entity* nextOnTile;
    for (Entity* mapEntity = app->game->findMapEntity(n << 6, n2 << 6, 30383); mapEntity != nullptr; mapEntity = nextOnTile) {
        nextOnTile = mapEntity->nextOnTile;
        if (mapEntity != entity2 && (mapEntity->info & 0x20000) != 0x0) {
            if (mapEntity == playerEnt) {
                if (this->attackerWeaponProj != 5) {
                    n6 >>= 1;
                    int min = std::min(n6 * 10 / 100, app->player->ce->getStat(Enums::STAT_ARMOR));
                    n6 -= min;
                    n6 -= n6 * app->player->ce->getStat(Enums::STAT_DEFENSE) / 100;
                    app->player->painEvent(nullptr, false);
                    playerEnt->knockback((n3 << 6) + 32, (n4 << 6) + 32, n5);
                    if (n6 > 0) {
                        this->crArmorDamage = min;
                        app->player->pain(n6, nullptr, true);
                        app->player->addArmor(-this->crArmorDamage);
                        app->player->addStatusEffect(13, 5, 3);
                        app->player->translateStatusEffects();
                    }
                }
            }
            else if (mapEntity->def->eType == Enums::ET_CORPSE) {
                if (!b) {
                    return;
                }
                mapEntity->died(false, entity);
                mapEntity->info |= 0x10000;
                app->particleSystem->spawnMonsterBlood(mapEntity, true);
            }
            else if (mapEntity->monster != nullptr) {
                mapEntity->info |= 0x200000;
                if ((mapEntity->info & 0x40000) == 0x0) {
                    app->game->activate(mapEntity, true, false, true, true);
                }
                int n7 = this->getWeaponWeakness(this->attackerWeaponId, mapEntity->def->eSubType, mapEntity->def->parm) * n6 >> 8;
                int n8 = n7 - (mapEntity->monster->ce.getStatPercent(Enums::STAT_DEFENSE) * n7 >> 8);
                if (n8 > 0) {
                    if (this->attackerWeaponProj == 5) {
                        int sprite = mapEntity->getSprite();
                        int sX = app->render->mapSprites[app->render->S_X + sprite];
                        int sY = app->render->mapSprites[app->render->S_Y + sprite];
                        int sZ = app->render->mapSprites[app->render->S_Z + sprite];
                        GameSprite* gSprite = app->game->gsprite_allocAnim(244, sX, sY, sZ + (sZ >> 1));
                        gSprite->flags |= 0x800;
                        this->nextStageTime = app->gameTime + gSprite->duration;
                        app->render->mapSprites[app->render->S_RENDERMODE + gSprite->sprite] = 4;
                        app->render->mapSprites[app->render->S_SCALEFACTOR + gSprite->sprite] = 32;
                    }
                    bool pain = mapEntity->pain(n8, nullptr);
                    if ((mapEntity->monster->flags & 0x1000) == 0x0 && !pain) {
                        mapEntity->knockback((n3 << 6) + 32, (n4 << 6) + 32, n5);
                    }
                    app->localization->resetTextArgs();
                    app->localization->addTextIDArg(mapEntity->name);
                    app->localization->addTextArg(n8);
                    if (mapEntity->monster->ce.getStat(Enums::STAT_HEALTH) <= 0) {
                        app->hud->addMessage((short)74);
                        if (this->attackerWeaponProj != 5) {
                            if (b && entity != nullptr && entity->def->eType == Enums::ET_DECOR_NOCLIP && entity->def->eSubType == Enums::DECOR_DYNAMITE) {
                                mapEntity->info |= 0x10000;
                                app->particleSystem->spawnMonsterBlood(mapEntity, true);
                            }
                            mapEntity->died(true, entity);
                        }
                    }
                    else {
                        app->hud->addMessage((short)75);
                    }
                }
            }
            else if (mapEntity->def->eType != Enums::ET_ENV_DAMAGE) {
                if (mapEntity->def->eType == Enums::ET_ATTACK_INTERACTIVE) {}
                mapEntity->pain(n6, entity);
                mapEntity->died(true, entity);
            }
        }
    }
}

Text* Combat::getWeaponStatStr(int n) {
    Applet* app = CAppContainer::getInstance()->app;
    int n2 = n * Combat::WEAPON_MAX_FIELDS;
    Text* largeBuffer = app->localization->getLargeBuffer();
     app->localization->resetTextArgs();
     app->localization->addTextArg(this->weapons[n2 + Combat::WEAPON_FIELD_STRMIN]);
     app->localization->composeText((short)0, (short)77, largeBuffer);
     app->localization->resetTextArgs();
    if (this->weapons[n2 + Combat::WEAPON_FIELD_RANGEMAX] != 1) {
         app->localization->addTextArg(this->weapons[n2 + Combat::WEAPON_FIELD_RANGEMIN]);
         app->localization->addTextArg(this->weapons[n2 + Combat::WEAPON_FIELD_RANGEMAX]);
         app->localization->composeText((short)0, (short)78, largeBuffer);
    }
    else {
         app->localization->addTextArg(this->weapons[n2 + Combat::WEAPON_FIELD_RANGEMAX]);
         app->localization->composeText((short)0, (short)79, largeBuffer);
    }
    EntityDef* find = app->entityDefManager->find(6, 2, this->weapons[n2 + Combat::WEAPON_FIELD_AMMOTYPE]);
    if (find != nullptr) {
         app->localization->composeText((short)0, (short)76, largeBuffer);
         app->localization->composeText((short)1, find->name, largeBuffer);
    }
    return largeBuffer;
}

Text* Combat::getArmorStatStr(int n) {
    Applet* app = CAppContainer::getInstance()->app;
    Text* largeBuffer = app->localization->getLargeBuffer();
    if (n != -1) {
        app->localization->resetTextArgs();
        app->localization->addTextArg(app->player->ce->getStat(Enums::STAT_ARMOR));
        app->localization->composeText((short)0, (short)80, largeBuffer);
    }
    app->localization->composeText((short)0, (short)81, largeBuffer);
    return largeBuffer;
}

int Combat::WorldDistToTileDist(int n) {
	for (int i = 0; i < (Combat::MAX_TILEDISTANCES - 1); ++i) {
		if (n < this->tileDistances[i]) {
			return i;
		}
	}
	return (Combat::MAX_TILEDISTANCES - 1);
}

void Combat::cleanUpAttack() {
    this->stage = 1;
    this->animLoopCount = 0;
}

void Combat::updateProjectile() {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->numActiveMissiles > 0) {
        int renderMode = 0;
        for (int i = 0; i < this->numActiveMissiles; ++i) {
            GameSprite* actMissile = this->activeMissiles[i];
            if ((actMissile->flags & 0x2) != 0x0) {
                if (this->missileAnim == 248) {
                    app->render->mapSpriteInfo[actMissile->sprite] ^= 0x20000;
                }
                else if (this->missileAnim == 171) {
                    int sprite = this->curAttacker->getSprite();
                    int x1 = app->render->mapSprites[app->render->S_X + sprite];
                    int y1 = app->render->mapSprites[app->render->S_Y + sprite];
                    int x2 = app->game->viewX;
                    int y2 = app->game->viewY;
                    int dx = 0;
                    int dy = 0;
                    if (std::abs(y2 - y1) > std::abs(x2 - x1)) {
                        dy = (((y2 - y1) >> 31) << 1) + 1;
                    }
                    else {
                        dx = (((x2 - x1) >> 31) << 1) + 1;
                    }
                    int x = x1 + this->numThornParticleSystems * dx * 64 + dx * 32;
                    int y = y1 + this->numThornParticleSystems * dy * 64 + dy * 32;
                    if (std::abs(app->render->mapSprites[app->render->S_X + actMissile->sprite] - x) <= 4 && std::abs(app->render->mapSprites[app->render->S_Y + actMissile->sprite] - y) <= 4) {
                        int height = app->render->getHeight(x, y);
                        app->particleSystem->spawnParticles(1, 0xFF81603D, x, y, height);
                        GameSprite* gSprite = app->game->gsprite_allocAnim(170, x, y, height + 32);
                        app->render->mapSprites[app->render->S_SCALEFACTOR + gSprite->sprite] = 48;
                        gSprite->duration += actMissile->duration - (app->gameTime - actMissile->time);
                        ++this->numThornParticleSystems;
                    }
                    app->canvas->repaintFlags |= Canvas::REPAINT_PARTICLES;
                }
            }
            else {
                bool b = true;
                int scaleFactor = 64;
                int x = actMissile->pos[3];
                int y = actMissile->pos[4];
                int z = actMissile->pos[5];
                switch (this->attackerWeaponProj) {
                case 4: {
                    this->missileAnim = 242;
                    renderMode = 4;
                    app->canvas->startShake(500, 4, 500);
                    app->sound->playSound(1032, 0, 4, 0);
                    break;
                }
                case 10: {
                    this->missileAnim = 252;
                    renderMode = 4;
                    break;
                }
                case 3: {
                    this->missileAnim = 243;
                    renderMode = 4;
                    break;
                }
                case 5: {
                    this->missileAnim = 244;
                    renderMode = 4;
                    break;
                }
                case 2: {
                    this->missileAnim = 235;
                    renderMode = 4;
                    if (this->curTarget != nullptr && (this->crFlags & 0x1007) != 0x0) {
                        EntityMonster* monster = this->curTarget->monster;
                        if (monster != nullptr && (1 << this->curTarget->def->eSubType & Enums::FEAR_IMMUNE_MONSTERS) == 0x0) {
                            monster->resetGoal();
                            monster->goalType = 4;
                            monster->goalParam = 3;
                        }
                        break;
                    }
                    break;
                }
                case 7: {
                    app->sound->playSound(1034, 0, 4, 0);
                    if (this->curTarget == nullptr && this->hitType != 0 && app->player->buffs[9] > 0) {
                        this->missileAnim = 208;
                        x += app->canvas->viewStepX >> 1;
                        y += app->canvas->viewStepY >> 1;
                        renderMode = 3;
                        break;
                    }
                    this->missileAnim = 242;
                    renderMode = 4;
                    break;
                }
                case 13: {
                    this->missileAnim = 0;
                    app->particleSystem->spawnParticles(1, -1, actMissile->sprite);
                    app->sound->playSound(1135, 0, 4, 0);
                    break;
                }
                case 12: {
                    app->game->gsprite_destroy(actMissile);
                    for (int j = i + 1; j < this->numActiveMissiles; ++j) {
                        this->activeMissiles[j - 1] = this->activeMissiles[j];
                    }
                    --this->numActiveMissiles;
                    if (this->soulCubeIsAttacking) {
                        this->soulCubeIsAttacking = false;
                        this->exploded = true;
                        this->explodeOnMonster();
                        this->launchSoulCube();
                    }
                    return;
                }
                case 9: {
                    renderMode = 0;
                    this->missileAnim = 170;
                    app->particleSystem->spawnParticles(1, 0xFF81603D, x, y, z + 16);
                    z += 48;
                    break;
                }
                default: {
                    this->missileAnim = 0;
                    break;
                }
                }

                if (this->curAttacker != nullptr && (1 << app->hud->damageDir & 0x1C) == 0x0 && !b) {
                    this->missileAnim = 0;
                }

                app->game->gsprite_destroy(actMissile);
                if (this->missileAnim != 0 && (this->curAttacker == nullptr || (this->curTarget == nullptr && this->hitType != 0))) {
                    GameSprite* gSprite = app->game->gsprite_allocAnim(this->missileAnim, x, y, z);
                    gSprite->flags |= 0x800;
                    this->nextStageTime = app->gameTime + gSprite->duration;
                    app->render->mapSprites[app->render->S_RENDERMODE + gSprite->sprite] = (short)renderMode;
                    app->render->mapSprites[app->render->S_SCALEFACTOR + gSprite->sprite] = (short)scaleFactor;
                    if (this->missileAnim == 243) {
                        app->render->mapSprites[app->render->S_SCALEFACTOR + gSprite->sprite] -= 16;
                    }
                    if (this->curAttacker != nullptr && (app->render->mapSpriteInfo[(this->curAttacker->info & 0xFFFF) - 1] & 0x20000) != 0x0) {
                        app->render->mapSpriteInfo[gSprite->sprite] |= 0x20000;
                    }
                }
                for (int k = i + 1; k < this->numActiveMissiles; ++k) {
                    this->activeMissiles[k - 1] = this->activeMissiles[k];
                }
                --this->numActiveMissiles;
                this->exploded = true;
            }
        }
    }
    else if (this->gotHit && (this->totalDamage > 0 || this->totalArmorDamage > 0) && this->exploded && (this->attackerWeaponId == 16 || this->attackerWeaponId == 15 || this->attackerWeaponId == 18 || this->attackerWeaponId == 17)) {
        if (this->attackerWeaponId == 16) {
            this->missileAnim = 245;
        }
        else if (this->attackerWeaponId == 15) {
            this->missileAnim = 246;
        }
        else {
            this->missileAnim = 247;
        }
        int* calcPosition = this->curAttacker->calcPosition();
        if (app->game->isInFront(calcPosition[0] >> 6, calcPosition[1] >> 6)) {
            GameSprite* gSprite = app->game->gsprite_allocAnim(this->missileAnim, app->canvas->destZ, app->canvas->destY, app->canvas->destZ);
            gSprite->flags |= 0x800;
            app->render->mapSprites[app->render->S_RENDERMODE + gSprite->sprite] = 5;
            if ((app->render->mapSpriteInfo[(this->curAttacker->info & 0xFFFF) - 1] & 0x20000) != 0x0 && this->missileAnim == 245) {
                app->render->mapSpriteInfo[gSprite->sprite] |= 0x20000;
            }
        }
    }
    if (this->exploded) {
        if (this->curTarget == nullptr) {
            this->explodeOnPlayer();
            this->exploded = false;
        }
        else {
            this->explodeOnMonster();
            this->exploded = false;
        }
    }
}

void Combat::launchProjectile() {
    Applet* app = CAppContainer::getInstance()->app;
    int n = 256;
    int n2;
    int n3 = 16;
    int renderMode = 0;
    if (this->attackerWeaponProj == 12) {
        this->soulCubeIsAttacking = true;
        this->launchSoulCube();
        return;
    }
    this->dodgeDir = (((int)app->nextInt() > 0x3FFFFFF) ? 1 : 0);
    int n5 = ((this->dodgeDir << 1) - 1) * 16;
    int x1;
    int y1;
    int z1;
    if (this->curAttacker == nullptr) {
        x1 = app->game->viewX;
        y1 = app->game->viewY;
        z1 = app->render->getHeight(x1, y1) + 32;
        n2 = 0;
    }
    else {
        int sprite = (this->curAttacker->info & 0xFFFF) - 1;
        x1 = app->render->mapSprites[app->render->S_X + sprite];
        y1 = app->render->mapSprites[app->render->S_Y + sprite];
        z1 = app->render->mapSprites[app->render->S_Z + sprite];
        n2 = 16;
    }
    int x2;
    int y2;
    int z2;
    if (this->curTarget == nullptr) {
        x2 = app->game->viewX;
        y2 = app->game->viewY;
        z2 = app->render->getHeight(x2, y2) + 32;
    }
    else if (this->targetType == 0) {
        x2 = app->game->traceCollisionX;
        y2 = app->game->traceCollisionY;
        z2 = app->game->traceCollisionZ;
    }
    else {
        int n10 = (this->curTarget->info & 0xFFFF) - 1;
        x2 = app->render->mapSprites[app->render->S_X + n10];
        y2 = app->render->mapSprites[app->render->S_Y + n10];
        z2 = app->render->mapSprites[app->render->S_Z + n10];
    }
    bool b = false;
    switch (this->attackerWeaponProj) {
    case 2: {
        renderMode = 3;
        this->missileAnim = 240;
        b = false;
        n = 512;
        break;
    }
    case 3: {
        x1 += app->game->viewRightStepX >> 4;
        y1 += app->game->viewRightStepY >> 4;
        z1 += 15;
        renderMode = 3;
        this->missileAnim = 243;
        n = 512;
        if (app->render->mapSprites[app->render->S_SCALEFACTOR + this->curTarget->getSprite()] >= 64) {
            z2 += 16;
        }
        b = false;
        break;
    }
    case 4: {
        x1 += app->game->viewRightStepX >> 4;
        y1 += app->game->viewRightStepY >> 4;
        z1 += 10;
        renderMode = 0;
        if (this->curAttacker == nullptr) {
            this->missileAnim = 225;
        }
        else {
            this->missileAnim = 226;
        }
        n += 64;
        b = false;
        break;
    }
    case 5: {
        renderMode = 4;
        this->missileAnim = 244;
        if (this->curAttacker != nullptr) {
            n += n >> 1;
        }
        b = false;
        break;
    }
    case 6: {
        renderMode = 0;
        this->missileAnim = 227;
        b = false;
        break;
    }
    case 7: {
        renderMode = 3;
        this->missileAnim = 242;
        b = false;
        break;
    }
    case 8: {
        renderMode = 3;
        this->missileAnim = 241;
        b = false;
        break;
    }
    case 9: {
        this->missileAnim = 171;
        b = false;
        z1 -= 32;
        z2 -= 32;
        this->numThornParticleSystems = 0;
        break;
    }
    case 10: {
        renderMode = 3;
        this->missileAnim = 252;
        z1 += 48;
        b = false;
        break;
    }
    case 11: {
        renderMode = 3;
        this->missileAnim = 248;
        b = false;
        break;
    }
    case 13: {
        b = false;
        this->missileAnim = app->player->activeWeaponDef->tileIndex;
        renderMode = 0;
        break;
    }
    default: {
        this->missileAnim = 0;
        this->exploded = true;
        return;
    }
    }
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    if ((this->crFlags & 0x400) != 0x0) {
        int n11 = this->weapons[this->attackerWeapon + 3] * 64;
        n3 = 0;
        if (dx != 0 && dx > dy) {
            x2 = x1 + (x2 - x1) * n11 / dx;
        }
        else if (dy != 0) {
            y2 = y1 + (y2 - y1) * n11 / dy;
        }
        dx = std::abs(x2 - x1);
        dy = std::abs(y2 - y1);
        n5 = 0;
    }
    else if (this->hitType == 0) {
        if (this->targetType == 2 || this->curTarget == nullptr) {
            n3 = 0;
        }
        else {
            n5 = 0;
        }
    }
    else {
        n5 = 0;
    }

    if (dy > dx) {
        int n12 = (((y2 - y1) >> 31) << 1) + 1;
        y1 +=  (n2 * n12);
        y2 += -(n3 * n12);
        x2 += n5;
    }
    else {
        int n15 = (((x2 - x1) >> 31) << 1) + 1;
        x1 +=  (n2 * n15);
        x2 += -(n3 * n15);
        y2 += n5;
    }
    if (this->attackerWeaponProj == 4) {
        if (this->curAttacker == nullptr) {
            x1 += 4 * app->canvas->viewRightStepX >> 6;
            y1 += 4 * app->canvas->viewRightStepY >> 6;
            z1 -= 5;
        }
        else {
            int n16 = 16;
            if ((app->render->mapSpriteInfo[this->curAttacker->getSprite()] & 0x20000) != 0x0) {
                n16 = -16;
            }
            x1 += n16 * app->canvas->viewRightStepX >> 6;
            y1 += n16 * app->canvas->viewRightStepY >> 6;
        }
    }
    GameSprite* allocMissile = this->allocMissile(x1, y1, z1, x2, y2, z2, 1000 * std::max(dx, dy) / n, renderMode);
    if ((this->missileAnim == 4 && this->curTarget != nullptr) || this->missileAnim == 243) {
        app->render->mapSpriteInfo[allocMissile->sprite] = ((app->render->mapSpriteInfo[allocMissile->sprite] & 0xFFFF00FF) | 0x1000);
        if (this->missileAnim == 243) {
            app->render->mapSprites[app->render->S_SCALEFACTOR + allocMissile->sprite] -= 16;
        }
    }
    else if (this->missileAnim >= 241 && this->missileAnim < 244) {
        app->render->mapSpriteInfo[allocMissile->sprite] = ((app->render->mapSpriteInfo[allocMissile->sprite] & 0xFFFF00FF) | 0x2000);
    }
    else if (this->missileAnim == 244) {
        app->render->mapSpriteInfo[allocMissile->sprite] = ((app->render->mapSpriteInfo[allocMissile->sprite] & 0xFFFF00FF) | 0x3000);
        if (this->curAttacker != nullptr) {
            app->render->mapSprites[app->render->S_SCALEFACTOR + allocMissile->sprite] -= 16;
        }
    }
    if (this->attackerWeaponProj == 2) {
        if (this->curAttacker == nullptr) {
            app->render->mapSprites[app->render->S_ENT + allocMissile->sprite] = app->player->getPlayerEnt()->getIndex();
            app->render->relinkSprite(allocMissile->sprite, allocMissile->pos[0] << 4, allocMissile->pos[1] << 4, allocMissile->pos[2] << 4);
        }
        else {
            app->render->mapSprites[app->render->S_ENT + allocMissile->sprite] = this->curAttacker->getIndex();
            app->render->relinkSprite(allocMissile->sprite, allocMissile->pos[3] << 4, allocMissile->pos[4] << 4, allocMissile->pos[5] << 4);
        }
        allocMissile->flags |= 0x4;
        allocMissile->pos[0] = allocMissile->pos[3];
        allocMissile->pos[1] = allocMissile->pos[4];
        allocMissile->pos[2] = allocMissile->pos[5];
        allocMissile->vel[0] = allocMissile->vel[1] = allocMissile->vel[2] = 0;
    }
    if (!b) {
        int sprite = allocMissile->sprite;
        app->render->mapSpriteInfo[allocMissile->sprite] &= 0xFFF700FF;
    }
    this->exploded = false;
}

GameSprite* Combat::allocMissile(int x1, int y1, int z1, int x2, int y2, int z2, int duration, int renderMode) {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->numActiveMissiles == 8) {
        app->Error("MAX_ACTIVE_MISSILES", Enums::ERR_MAX_MISSILES);
        return nullptr;
    }
    int n8 = app->render->mediaMappings[this->missileAnim + 1] - app->render->mediaMappings[this->missileAnim];
    int numActiveMissiles = this->numActiveMissiles;
    GameSprite* gameSprite = app->game->gsprite_alloc(this->missileAnim, n8, 2562);
    this->activeMissiles[this->numActiveMissiles++] = gameSprite;

    short* mapSprites = app->render->mapSprites;
    mapSprites[app->render->S_ENT + gameSprite->sprite] = -1;
    mapSprites[app->render->S_RENDERMODE + gameSprite->sprite] = (short)renderMode;
    mapSprites[app->render->S_X + gameSprite->sprite] = (gameSprite->pos[0] = (short)x1);
    mapSprites[app->render->S_Y + gameSprite->sprite] = (gameSprite->pos[1] = (short)y1);
    mapSprites[app->render->S_Z + gameSprite->sprite] = (gameSprite->pos[2] = (short)z1);
    app->render->mapSpriteInfo[gameSprite->sprite] |= 0x80000;
    gameSprite->pos[3] = (short)x2;
    gameSprite->pos[4] = (short)y2;
    gameSprite->pos[5] = (short)z2;
    gameSprite->duration = duration;
    if (this->attackerWeaponProj == 13) {
        gameSprite->flags |= 0x4000;
        int n9 = 8;
        if (gameSprite->pos[5] > gameSprite->pos[2]) {
            n9 /= 4;
        }
        gameSprite->pos[5] += (short)(std::min(this->tileDist, 5) * n9);
        if (gameSprite->pos[5] < gameSprite->pos[2]) {
            gameSprite->pos[5] = gameSprite->pos[2];
        }
    }
    if (duration != 0) {
        gameSprite->vel[0] = 1000 * (gameSprite->pos[3] - gameSprite->pos[0]) / duration;
        gameSprite->vel[1] = 1000 * (gameSprite->pos[4] - gameSprite->pos[1]) / duration;
        gameSprite->vel[2] = 1000 * (gameSprite->pos[5] - gameSprite->pos[2]) / duration;
    }
    else {
        gameSprite->vel[0] = gameSprite->vel[1] = gameSprite->vel[2] = 0;
    }
    if (this->missileAnim == 0) {
        app->render->mapSpriteInfo[gameSprite->sprite] |= 0x10000;
        return gameSprite;
    }
    app->render->relinkSprite(gameSprite->sprite);
    return gameSprite;
}

void Combat::launchSoulCube() {
    Applet* app = CAppContainer::getInstance()->app;
    int sprite = this->curTarget->getSprite();
    int x1, y1, z1;
    int x2, y2, z2;

    if (this->soulCubeIsAttacking) {
        x1 = app->game->viewX;
        y1 = app->game->viewY;
        z1 = app->render->getHeight(x1, y1) + 32;
        x2 = app->render->mapSprites[app->render->S_X + sprite];
        y2 = app->render->mapSprites[app->render->S_Y + sprite];
        z2 = app->render->mapSprites[app->render->S_Z + sprite];
    }
    else {
        x1 = app->render->mapSprites[app->render->S_X + sprite];
        y1 = app->render->mapSprites[app->render->S_Y + sprite];
        z1 = app->render->mapSprites[app->render->S_Z + sprite];
        x2 = app->game->viewX;
        y2 = app->game->viewY;
        z2 = app->render->getHeight(x1, y1) + 32;
    }
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    if (dy > dx) {
        int n4 = (((y2 - y1) >> 31) << 1) + 1;
        if (this->soulCubeIsAttacking) {
            y1 += (64 * n4);
        }
        else {
            y2 += -(64 * n4);
        }
    }
    else {
        int n5 = (((x2 - x1) >> 31) << 1) + 1;
        if (this->soulCubeIsAttacking) {
            x1 += (64 * n5);
        }
        else {
            x2 += -(64 * n5);
        }
    }
    int duration = 1000 * std::max(dx, dy) / 256;
    this->missileAnim = 239;
    this->allocMissile(x1, y1, z1, x2, y2, z2, duration, 0);
}

int Combat::getWeaponTileNum(int n) {
    if (n < 5) {
        return 1 + n;
    }
    switch (n) {
        case 5: {
            return 13;
        }
        case 6: {
            return 14;
        }
        case 14: {
            return 15;
        }
        default: {
            return 1 + n - 2;
        }
    }
}