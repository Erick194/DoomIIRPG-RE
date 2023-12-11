#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Entity.h"
#include "EntityDef.h"
#include "EntityMonster.h"
#include "Combat.h"
#include "CombatEntity.h"
#include "Render.h"
#include "Game.h"
#include "Canvas.h"
#include "Player.h"
#include "Hud.h"
#include "Text.h"
#include "ParticleSystem.h"
#include "JavaStream.h"
#include "Enums.h"
#include "Sound.h"

Entity::Entity() {
    std::memset(this, 0, sizeof(Entity));
}

Entity::~Entity() {
}

bool Entity::startup() {
	//printf("Entity::startup\n");

	return false;
}

void Entity::reset() {
	this->def = nullptr;
	this->prevOnTile = nullptr;
	this->nextOnTile = nullptr;
	this->linkIndex = 0;
	this->info = 0;
	this->param = 0;
	this->monster = nullptr;
	this->name = -1;
	if (this->lootSet) {
		delete this->lootSet;
	}
	this->lootSet = nullptr;
}

void Entity::initspawn() {
    Applet* app = CAppContainer::getInstance()->app;

    uint8_t eType = this->def->eType;
    uint8_t eSubType = this->def->eSubType;
    this->name = (short)(this->def->name | 0x400);
    int sprite = this->getSprite();

    int tileNum = app->render->mapSpriteInfo[sprite] & 0xFF;
    if (eType == Enums::ET_MONSTER) {
        app->combat->monsters[eSubType * 3 + (int8_t)this->def->parm]->clone(&this->monster->ce);

        if (app->game->difficulty == 4 || (app->game->difficulty == 2 && !this->isBoss())) {
            int stat = this->monster->ce.getStat(1);
            int n2 = stat + (stat >> 2);
            this->monster->ce.setStat(1, n2);
            this->monster->ce.setStat(0, n2);
        }
        short n3 = 0;
        short n4 = 64;
        app->render->mapSprites[app->render->S_Z + sprite] = (short)(32 + app->render->getHeight(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite]));
        app->render->relinkSprite(sprite);
        if ((eSubType == Enums::BOSS_MASTERMIND || eSubType == Enums::MONSTER_PINKY) && this->def->parm == 0) {
            n4 = 42;
        }
        app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = n4;
        app->render->mapSprites[app->render->S_RENDERMODE + sprite] = n3;
        this->info |= 0x20000;
        if (eSubType == Enums::BOSS_VIOS || eSubType == Enums::BOSS_VIOS2) {
            this->param = app->nextInt() % 3 + 3;
        }
    }
    else if (eType == Enums::ET_DECOR && eSubType != Enums::DECOR_STATUE) {
        app->render->mapSpriteInfo[sprite] &= ~0x10000;
        if (tileNum == Enums::TILENUM_SWITCH) {
            app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = 32;
        }
    }
    else if (eType == Enums::ET_ATTACK_INTERACTIVE) {
        this->info |= 0x20000;
#if 0 // J2ME
        if (eSubType == 152) { // TILENUM_OBJ_CRATE
            app->render->mapSprites[app->render->S_Z + sprite] -= 224;
        }
#endif
    }
    else if (this->def->eType == Enums::ET_CORPSE && this->def->eSubType == Enums::CORPSE_SKELETON) {
        this->info |= 0x420000;
    }
    else if (eType == Enums::ET_NPC) {
        this->param = 1;
    }

    if (this->def->eType != Enums::ET_MONSTER && this->def->eType != Enums::ET_CORPSE) {
        if (this->lootSet) {
            delete this->lootSet;
        }
        this->lootSet = nullptr;
    }
    else {
        this->populateDefaultLootSet();
    }
}

int Entity::getSprite() {
	return (this->info & 0xFFFF) - 1;
}

bool Entity::touched() {
    Applet* app = CAppContainer::getInstance()->app;

    uint8_t eType = this->def->eType;
    //printf("Entity::touched %d -------------------------->\n", eType);
    if (eType == Enums::ET_MONSTERBLOCK_ITEM || eType == Enums::ET_ITEM) {
        if (this->touchedItem()) {
            app->game->scriptStateVars[11] = this->def->tileIndex;
            app->game->executeStaticFunc(11);
            if (this->isDroppedEntity()) {
                app->render->mapSprites[app->render->S_ENT + this->getSprite()] = -1;
                this->def = nullptr;
            }
            return true;
        }
    }
    else if (eType == Enums::ET_ENV_DAMAGE) {
        if (!app->player->isFamiliar) {
            if (this->def->eSubType == Enums::ENV_DAMAGE_FIRE && app->player->buffs[9] == 0) {
                app->player->painEvent(nullptr, false);
                app->player->pain(20, this, true);
                app->player->addStatusEffect(13, 5, 3);
                app->player->translateStatusEffects();
            }
            else if (this->def->eSubType == Enums::ENV_DAMAGE_SPIKES) {
                app->player->painEvent(nullptr, false);
                app->player->pain(20, this, true);
            }
        }
        return true;
    }
    return false;
}

bool Entity::touchedItem() {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->def->eSubType == Enums::IT_INVENTORY) {
        int param = 1;
        if (this->isDroppedEntity()) {
            param = this->param;
        }
        else if (this->def->parm == 24) {
            param = 2 + app->nextInt() % 3;
        }
        if (!app->player->give(this->def->eSubType, this->def->parm, param, false)) {
            Text* messageBuffer = app->hud->getMessageBuffer(2);
            app->localization->resetTextArgs();
            app->localization->addTextArg((short)1, this->def->name);
            app->localization->composeText((short)0, (short)83, messageBuffer);
            app->hud->finishMessageBuffer();
            return false;
        }
        app->localization->resetTextArgs();
        if (this->def->eSubType == Enums::IT_INVENTORY && (this->def->parm == 19 || this->def->parm == 20)) {
            app->localization->composeText((short)0, (short)84, app->hud->getMessageBuffer(3));
            app->hud->repaintFlags |= 0x4;
        }
        else if (this->def->parm == 24) {
            Text* messageBuffer2 = app->hud->getMessageBuffer(1);
            app->localization->resetTextArgs();
            app->localization->addTextArg(param);
            Text* smallBuffer = app->localization->getSmallBuffer();
            app->localization->composeText((short)1, this->def->longName, smallBuffer);
            app->localization->addTextArg(smallBuffer);
            app->localization->composeText((short)0, (short)86, messageBuffer2);
            smallBuffer->dispose();
        }
        else {
            Text* messageBuffer3 = app->hud->getMessageBuffer(1);
            app->localization->addTextArg((short)1, this->def->longName);
            app->localization->composeText((short)0, (short)85, messageBuffer3);
        }
        app->hud->finishMessageBuffer();
        if (!this->isDroppedEntity() && this->def->parm != 18) {
            app->game->foundLoot(this->getSprite(), 1);
        }
    }
    else if (this->def->eSubType == Enums::IT_FOOD) {
        int n;
        if (this->def->parm == 0 || this->def->parm == 2) {
            n = 40;
        }
        else {
            n = 20;
        }
        if (!app->player->addHealth(n)) {
            app->hud->addMessage((short)46, 2);
            return false;
        }
    }
    else if (this->def->eSubType == Enums::IT_AMMO) {
        int param2;
        if (this->isDroppedEntity()) {
            param2 = this->param;
        }
        else {
            param2 = 2 + app->nextInt() % 4;
            if (this->def->parm == 2) {
                param2 &= 0xFFFFFFFE;
            }
        }
        if (!app->player->give(2, this->def->parm, param2, false)) {
            app->hud->addMessage((short)87);
        }
        else {
            Text* messageBuffer4 = app->hud->getMessageBuffer(1);
            app->localization->resetTextArgs();
            app->localization->addTextArg(param2);
            Text* smallBuffer2 = app->localization->getSmallBuffer();
            app->localization->composeTextField(this->name, smallBuffer2);
            app->localization->addTextArg(smallBuffer2);
            app->localization->composeText((short)0, (short)86, messageBuffer4);
            app->hud->finishMessageBuffer();
            smallBuffer2->dispose();
            if (!this->isDroppedEntity()) {
                app->game->foundLoot(this->getSprite(), 1);
            }
        }
    }
    else if (this->def->eSubType == Enums::IT_WEAPON) {
        if (app->player->weaponIsASentryBot(this->def->parm)) {
            if (app->player->isFamiliar || app->player->hasASentryBot()) {
                return false;
            }
            int param3;
            if (this->isDroppedEntity()) {
                param3 = this->param;
            }
            else {
                param3 = 100;
            }
            app->player->give(1, this->def->parm, 1, false);
            app->player->ammo[7] = (short)param3;
            app->hud->addMessage((short)0, (short)223, 3);
        }
        else {
            app->player->give(1, this->def->parm, 1, false);
            int n2 = this->def->parm * 9;
            if (app->combat->weapons[n2 + 5] != 0) {
                if ((1 << this->def->parm & 0x200) != 0x0) {
                    app->player->give(2, app->combat->weapons[n2 + 4], 8, true);
                }
                else {
                    app->player->give(2, app->combat->weapons[n2 + 4], 10, true);
                }
            }
            Text* messageBuffer5 = app->hud->getMessageBuffer(1);
            app->localization->resetTextArgs();
            app->localization->addTextArg((short)1, this->def->longName);
            app->localization->composeText((short)0, (short)85, messageBuffer5);
            app->hud->finishMessageBuffer();
            app->player->showWeaponHelp(this->def->parm, false);
        }
        if (!this->isDroppedEntity()) {
            app->game->foundLoot(this->getSprite(), 1);
        }
    }
    app->game->removeEntity(this);
    app->sound->playSound(1054, 0, 4, false);
    return true;
}

bool Entity::pain(int n, Entity* entity) {
    Applet* app = CAppContainer::getInstance()->app;

    bool b = false;
    int sprite = this->getSprite();
    if ((this->info & 0x20000) == 0x0) {
        return b;
    }
    if (this->def->eType == Enums::ET_MONSTER) {
        int stat = this->monster->ce.getStat(0);
        int stat2 = this->monster->ce.getStat(1);
        int n2 = stat - n;
        if (this->isBoss()) {
            int n3 = stat2 >> 1;
            int n4 = n3 >> 1;
            int n5 = stat2 - n4;
            if (n2 <= n5 && n2 + n > n5) {
                if (!app->canvas->combatDone) {
                    b = (app->game->executeStaticFunc(2) != 0);
                    if (n2 < n3) {
                        n2 = n3 + 1;
                        this->monster->monsterEffects &= ~1u;
                    }
                }
                else {
                    n2 = this->monster->ce.getStat(0);
                    this->monster->monsterEffects &= ~1u;
                }
            }
            else if (n2 <= n3 && n2 + n > n3) {
                if (!app->canvas->combatDone) {
                    b = (app->game->executeStaticFunc(3) != 0);
                    if (n2 < n4) {
                        n2 = n4 + 1;
                        this->monster->monsterEffects &= ~1u;
                    }
                }
                else {
                    n2 = this->monster->ce.getStat(0);
                    this->monster->monsterEffects &= ~1u;
                }
            }
            else if (n2 <= n4 && n2 + n > n4) {
                if (!app->canvas->combatDone) {
                    b = (app->game->executeStaticFunc(4) != 0);
                    if (n2 < 0) {
                        n2 = 1;
                        this->monster->monsterEffects &= ~1u;
                    }
                }
                else {
                    n2 = this->monster->ce.getStat(0);
                    this->monster->monsterEffects &= ~1u;
                }
            }
            if (b && (this->monster->flags & 0x20) != 0x0) {
                app->combat->animLoopCount = 1;
            }
        }

        if ((this->monster->flags & 0x4) != 0x0 && n2 <= 0) {
            n2 = 1;
        }
        this->monster->ce.setStat(0, n2);
        if (n2 > 0) {

            int monsterSound = app->game->getMonsterSound(this->def->eSubType, (char)this->def->parm, Enums::MSOUND_PAIN);
            app->sound->playSound(monsterSound, 0, 3, 0);

            if (app->combat->punchingMonster == 0 && 0x0 == (this->monster->monsterEffects & 0x2)) {
                app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x6000);
                this->monster->frameTime = app->time + 250;
            }
            if (app->combat->attackerWeaponId != 2) {
                this->monster->resetGoal();
            }
        }
        else if ((this->monster->monsterEffects & 0x2) == 0x0) {
            app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x6000);
            if (app->combat->animLoopCount > 0) {
                this->monster->frameTime = app->time + 250;
            }
            if (app->combat->punchingMonster != 0) {
                this->monster->frameTime = app->combat->animEndTime + 200;
            }
            else {
                this->monster->frameTime = app->time + 250 + 200;
            }
        }
    }
    else if (this->def->eType == Enums::ET_ATTACK_INTERACTIVE) {
        uint8_t eSubType = this->def->eSubType;
        if (eSubType == 1) {
            app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x100);
            app->particleSystem->spawnParticles(2, -1, sprite);
            app->sound->playSound(1038, 0, 3, false);
            app->game->unlinkEntity(this);
        }
        else {
            if (eSubType == Enums::INTERACT_CRATE || eSubType == Enums::INTERACT_FURNITURE) {
                app->particleSystem->spawnParticles(2, -8421505, sprite);
            }
            else if (eSubType == Enums::INTERACT_PICKUP) {
                app->particleSystem->spawnParticles(1, -1, sprite);
                app->canvas->turnEntityIntoWaterSpout(this);
                return b;
            }
            app->game->removeEntity(this);
            this->info |= 0x400000;
            app->render->mapSpriteInfo[sprite] |= 0x10000;
        }
    }
    return b;
}

void Entity::checkMonsterDeath(bool b, bool b2) {
    Applet* app = CAppContainer::getInstance()->app;

    if (b && (app->player->weapons & 0x2000) != 0x0 && app->combat->attackerWeaponId != 13) {
        app->player->give(2, 6, 1);
    }
    if (this->monster != nullptr) {
        if (b2) {
            int resourceID = app->game->getMonsterSound(this->def->eSubType, this->def->parm, Enums::MSOUND_DEATH);
            app->sound->playSound(resourceID, 0, 5, false);
        }
        if (b) {
            int calcXP = this->monster->ce.calcXP();
            if (this->isBoss()) {
                calcXP += 130;
            }
            app->player->addXP(calcXP);
        }
        if ((this->monster->flags & 0x80) == 0x0) {
            app->player->fillMonsterStats();
            if (app->player->monsterStats[0] == app->player->monsterStats[1] && (app->player->killedMonstersLevels & 1 << (app->canvas->loadMapID - 1)) == 0x0) {
                app->player->showAchievementMessage(2);
                app->player->killedMonstersLevels |= 1 << (app->canvas->loadMapID - 1);
            }
        }
    }
}

void Entity::died(bool b, Entity* entity) {
    Applet* app = CAppContainer::getInstance()->app;

    int sprite = this->getSprite();
    short n = app->render->mapSprites[app->render->S_X + sprite];
    short n2 = app->render->mapSprites[app->render->S_Y + sprite];
    int n3 = app->render->mapSpriteInfo[sprite];
    if ((this->info & 0x20000) == 0x0 || (this->monster != nullptr && (this->monster->flags & 0x4) != 0x0)) {
        return;
    }
    this->info &= 0xFFFDFFFF;
    uint8_t  eType = this->def->eType;
    uint8_t  eSubType = this->def->eSubType;
    if (eType == Enums::ET_ATTACK_INTERACTIVE) {
        app->localization->resetTextArgs();
        Text* smallBuffer = app->localization->getSmallBuffer();
        app->localization->composeTextField(this->name, smallBuffer);
        app->localization->addTextArg(smallBuffer);
        app->hud->addMessage((short)89);
        smallBuffer->dispose();
        app->player->addXP(5);
        if (this->def->eSubType != 3 && this->def->eSubType != 2) {
            app->game->destroyedObject(sprite);
        }
    }
    else if (eType == Enums::ET_CORPSE) {
        n3 |= 0x10000;
        this->info |= 0x410000;
        this->info &= 0xFFF7FFFF;
        if (this->monster != nullptr) {
            this->monster->monsterEffects = 0;
        }
        app->player->counters[4]++;
        app->game->unlinkEntity(this);
    }
    else if (eType == Enums::ET_MONSTER) {
        this->info |= 0x400000;
        this->monster->resetGoal();
        app->game->snapLerpSprites(this->getSprite());
        n3 = ((n3 & 0xFFFF00FF) | 0x7000);
        this->monster->frameTime = app->time;
        if ((this->info & 0x10000) != 0x0) {
            n3 |= 0x17000;
        }
        else {
            this->info |= 0x1020000;
            this->trimCorpsePile(n, n2);
        }
        if ((this->monster->monsterEffects & 0x2) || (this->monster->monsterEffects & 0x1)) {
            this->monster->clearEffects();
        }
        else {
            this->monster->monsterEffects &= 0xFFFF801F;
            this->monster->monsterEffects |= 0x220220;
        }
        if (app->game->difficulty == 4 && (this->monster->flags & 0x40) == 0x0) {
            int n5 = 2 + app->nextInt() % 3;
            this->monster->monsterEffects &= 0xFFFE1FFB;
            this->monster->monsterEffects |= n5 << 13;
            this->monster->monsterEffects |= 0x4;
        }
        app->game->deactivate(this);
        this->undoAttack();
        if (this->isBoss()) {
            app->player->inCombat = false;
            app->game->executeStaticFunc(5);
        }
        else if (eSubType == Enums::MONSTER_LOST_SOUL || eSubType == Enums::MONSTER_CACODEMON) {
            app->game->gsprite_allocAnim(241, n, n2, app->render->mapSprites[app->render->S_Z + sprite]);
            n3 |= 0x10000;
            app->game->spawnDropItem(this);
        }
        this->checkMonsterDeath(b, true);
        if ((this->info & 0x10000) != 0x0 || eSubType == Enums::MONSTER_LOST_SOUL || eSubType == Enums::MONSTER_CACODEMON) {
            this->info = ((this->info & 0xFEFDFFFF) | 0x10000);
            app->game->unlinkEntity(this);
        }
        this->def = app->entityDefManager->find(9, eSubType, this->def->parm);
        this->name = (short)(this->def->name | 0x400);
        app->canvas->invalidateRect();
        if (app->game->difficulty == 4 && (this->monster->flags & 0x40) == 0x0 && (this->info & 0x80000) == 0x0) {
            int n6 = 2 + app->nextInt() % 3;
            this->monster->monsterEffects |= n6 << 13;
            this->monster->monsterEffects |= 0x4;
        }
        else if ((this->info & 0x80000) != 0x0) {
            n3 |= 0x10000;
            this->info |= 0x410000;
            this->info &= 0xFFF7FFFF;
            app->player->counters[4]++;
            app->game->unlinkEntity(this);
            GameSprite* gsprite_allocAnim = app->game->gsprite_allocAnim(241, app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], app->render->mapSprites[app->render->S_Z + sprite] - 20);
            gsprite_allocAnim->flags |= 0x400;
            gsprite_allocAnim->startScale = 96;
            gsprite_allocAnim->destScale = 127;
            gsprite_allocAnim->scaleStep = 38;
        }
    }
    if ((this->info & 0x2000000) != 0x0) {
        app->game->executeEntityFunc(this, this->deathByExplosion(entity));
        this->info &= 0xFDFFFFFF;
    }
    app->render->mapSpriteInfo[sprite] = n3;
    app->canvas->updateFacingEntity = true;
}

bool Entity::deathByExplosion(Entity* entity) {
    Applet* app = CAppContainer::getInstance()->app;
    return entity == app->player->getPlayerEnt() && app->player->ce->weapon == 11;
}

void Entity::aiCalcSimpleGoal(bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->def->eSubType == Enums::MONSTER_ARCH_VILE && this->aiCalcArchVileGoal()) {
        return;
    }
    if (app->player->buffs[11] > 0 && !(1 << this->def->eSubType & Enums::FEAR_IMMUNE_MONSTERS)) {
        this->monster->goalType = 4;
        this->monster->goalParam = 1;
        return;
    }
    int aiWeaponForTarget = this->aiWeaponForTarget(&app->game->entities[1]);
    bool b2 = false;
    if (aiWeaponForTarget != -1) {
        this->monster->ce.weapon = aiWeaponForTarget;
        b2 = true;
    }
    if (b2) {
        this->monster->goalType = 3;
        this->monster->goalParam = 1;
        if ((1 << this->def->eSubType & Enums::EVADING_MONSTERS)) {
            this->monster->flags |= 0x1;
        }
    }
    else {
        this->monster->goalType = 2;
        this->monster->goalParam = 1;
        if ((1 << this->def->eSubType & Enums::EVADING_MONSTERS)) {
            this->monster->flags &= ~0x1;
        }
    }
}

bool Entity::aiCalcArchVileGoal() {
    return this->findRaiseTarget(0x19000, 0, 0) != -1;
}

void Entity::aiMoveToGoal() {
    Applet* app = CAppContainer::getInstance()->app;

    uint8_t goalType = this->monster->goalType;
    EntityMonster* monster = this->monster;
    if (goalType == 2 || goalType == 1 || goalType == 4 || goalType == 5) {
        this->aiGoal_MOVE();
    }
    else if (goalType == 3) {
        if (monster->goalParam == 1) {
            monster->target = nullptr;
        }
        else {
            monster->target = &app->game->entities[monster->goalParam];
        }
        this->attack();
    }
    if (this->def->eSubType == 13) {
        int sprite = this->getSprite();
        app->game->scriptStateVars[9] = app->render->mapSprites[app->render->S_X + sprite];
        app->game->scriptStateVars[10] = app->render->mapSprites[app->render->S_Y + sprite];
    }
}

void Entity::aiChooseNewGoal(bool b) {
    uint8_t eSubType = this->def->eSubType;
    this->monster->resetGoal();
    this->aiCalcSimpleGoal(b);
    if ((1 << eSubType & Enums::EVADING_MONSTERS) != 0x0 && this->monster->goalType == 3) {
        this->monster->goalFlags |= 0x8;
    }
}

bool Entity::aiIsValidGoal() {
    Applet* app = CAppContainer::getInstance()->app;

    uint8_t goalType = this->monster->goalType;
    if (this->monster->goalTurns >= 16 || goalType == 3 || goalType == 0) {
        return false;
    }
    if (goalType == 2) {
        Entity* entity = &app->game->entities[this->monster->goalParam];
        if (this->monster->goalParam != 1) {
            int* calcPosition = entity->calcPosition();
            Entity* mapEntity = app->game->findMapEntity(calcPosition[0], calcPosition[1], 15535);
            if (entity->linkIndex != this->linkIndex && (mapEntity == nullptr || mapEntity == entity)) {
                return true;
            }
        }
    }
    else if (goalType == 1) {
        if (this->linkIndex != this->monster->goalX + this->monster->goalY * 32) {
            return true;
        }
    }
    else {
        if (goalType == 4 || goalType == 5) {
            return this->monster->goalTurns < this->monster->goalParam;
        }
        if (goalType == 6) {
            if (this->monster->goalTurns < this->monster->goalParam) {
                return true;
            }
            if (this->monster->goalTurns == this->monster->goalParam) {
                this->monster->resetGoal();
                return false;
            }
        }
    }
    return false;
}

bool Entity::aiIsAttackValid() {
    Applet* app = CAppContainer::getInstance()->app;

    EntityMonster* monster = this->monster;
    int weapon = monster->ce.weapon;
    if (weapon <= -1) { //[GEC]
        return false;
    }
    int* calcPosition = this->calcPosition();
    app->game->trace(calcPosition[0], calcPosition[1], app->game->destX, app->game->destY, this, 5295, 2);
    Entity* traceEntity = app->game->traceEntity;
    if (traceEntity != nullptr) {
        bool b = app->combat->weapons[weapon * 9 + 3] >= app->combat->WorldDistToTileDist(this->distFrom(app->game->destX, app->game->destY));
        bool b2 = false;
        if (b) {
            int n = calcPosition[0] - app->game->destX;
            int n2 = calcPosition[1] - app->game->destY;
            b2 = ((n != 0 || n2 != 0) && (n == 0 || n2 == 0));
        }
        if (monster->target == nullptr && traceEntity->def->eType == 1 && b && b2) {
            return true;
        }
        if (monster->target == traceEntity && b && b2) {
            return true;
        }
    }
    return false;
}

void Entity::aiThink(bool b) {
    EntityMonster* monster = this->monster;
    if (monster->flags & 0x400) {
        monster->flags &= ~0x400;
    }
    if (!(monster->flags & 0x20)) {
        if (!this->aiIsValidGoal()) {
            this->aiChooseNewGoal(b);
        }
        monster->goalTurns++;
        this->aiMoveToGoal();
    }
}

int Entity::aiWeaponForTarget(Entity* entity) {
    Applet* app = CAppContainer::getInstance()->app;

    int sprite = this->getSprite();
    int viewX;
    int viewY;
    if (entity->def->eType == Enums::ET_PLAYER) {
        viewX = app->canvas->viewX;
        viewY = app->canvas->viewY;
    }
    else {
        viewX = app->render->mapSprites[app->render->S_X + sprite];
        viewY = app->render->mapSprites[app->render->S_Y + sprite];
    }
    int a = viewX - app->render->mapSprites[app->render->S_X + sprite];
    int a2 = viewY - app->render->mapSprites[app->render->S_Y + sprite];
    int8_t* weapons = app->combat->weapons;
    if (this->def->eSubType == 13) {
        if (a != 0 && a2 != 0) {
            return -1;
        }
        int* calcPosition = entity->calcPosition();
        app->game->trace(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], calcPosition[0], calcPosition[1], this, 4131, 2);
        if (app->game->traceEntity == entity) {
            return app->combat->getMonsterField(this->def, 0);
        }
        return -1;
    }
    else {
        if (this->def->eSubType == 15) {
            if (this->param <= 0) {
                this->param = (int)app->nextInt() % 3 + 3;
                int n = 30;
                while (n-- > 0) {
                    short n2 = app->render->mapSprites[app->render->S_X + sprite];
                    short n3 = app->render->mapSprites[app->render->S_Y + sprite];
                    int n4 = 0;
                    int n5 = 0;

                    while (n-- > 0 && n4 == 0 && n5 == 0){
                        n4 = (int)app->nextInt() % 9 - 4;
                        n5 = (int)app->nextInt() % 9 - 4;
                    }
 
                    int n6 = n2 + (n4 << 6);
                    int n7 = n3 + (n5 << 6);
                    app->game->trace(n2, n3, n6, n7, this, 15535, 2);
                    if (app->game->numTraceEntities == 0) {
                        app->particleSystem->spawnParticles(7, -1, sprite);
                        app->sound->playSound(1123, 0, 1, 0);
                        app->game->unlinkEntity(this);
                        app->render->mapSprites[app->render->S_X + sprite] = (short)n6;
                        app->render->mapSprites[app->render->S_Y + sprite] = (short)n7;
                        app->render->mapSprites[app->render->S_Z + sprite] = (short)(app->render->getHeight(n6, n7) + 32);
                        app->game->linkEntity(this, n6 >> 6, n7 >> 6);
                        app->render->relinkSprite(sprite);
                        app->particleSystem->spawnParticles(7, -1, sprite);
                        break;
                    }
                }
            }
            else {
                --this->param;
            }
        }
        if (a != 0 && a2 != 0) {
            if (this->def->eSubType == 10 || (this->def->eSubType == 15 && this->def->parm != 0)) {
                int monsterField = app->combat->getMonsterField(this->def, (this->def->eSubType == 15) ? 1 : 0);
                int n8 = monsterField * 9;
                int worldDistToTileDist = app->combat->WorldDistToTileDist(entity->distFrom(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite]));
                if (weapons[n8 + 2] <= worldDistToTileDist && weapons[n8 + 3] >= worldDistToTileDist) {
                    int* calcPosition2 = entity->calcPosition();
                    app->game->trace(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], calcPosition2[0], calcPosition2[1], this, 4131, 2);
                    if (app->game->traceEntity == entity) {
                        return monsterField;
                    }
                }
                return -1;
            }
            return -1;
        }
        else {
            app->game->trace(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], viewX, viewY, this, 5295, 2);
            if (app->game->traceEntity != entity) {
                return -1;
            }
            int n9 = a;
            int n10 = a2;
            if (n9 != 0) {
                n9 /= std::abs(a);
            }
            if (n10 != 0) {
                n10 /= std::abs(a2);
            }
            app->game->trace(app->render->mapSprites[app->render->S_X + sprite] + n9 * 18, app->render->mapSprites[app->render->S_Y + sprite] + n10 * 18, viewX, viewY, this, 15791, 2);
            bool b = app->game->traceEntity == entity;
            int monsterField2 = app->combat->getMonsterField(this->def, 0);
            int monsterField3 = app->combat->getMonsterField(this->def, 1);
            if (!b) {
                if (Entity::CheckWeaponMask(monsterField2, 0x78002) != 0x0) {
                    monsterField2 = 0;
                }
                if (Entity::CheckWeaponMask(monsterField3, 0x78002) != 0x0) {
                    monsterField3 = 0;
                }
            }
            int n12;
            int n11 = n12 = -1;
            int worldDistToTileDist2 = app->combat->WorldDistToTileDist(entity->distFrom(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite]));
            if (monsterField2 == monsterField3) {
                monsterField3 = 0;
            }
            if (monsterField2 != 0) {
                int n13 = monsterField2 * 9;
                if (weapons[n13 + 2] <= worldDistToTileDist2 && weapons[n13 + 3] >= worldDistToTileDist2) {
                    n12 = n13;
                }
            }
            if (monsterField3 != 0) {
                int n14 = monsterField3 * 9;
                if (weapons[n14 + 2] <= worldDistToTileDist2 && weapons[n14 + 3] >= worldDistToTileDist2) {
                    n11 = n14;
                }
            }

            if (monsterField2 == 18 || monsterField3 == 18) {
                bool b2 = true;
                int n15 = app->render->mapSprites[app->render->S_X + sprite] >> 6;
                int n16 = app->render->mapSprites[app->render->S_Y + sprite] >> 6;

                do {
                    if (app->game->baseVisitedTiles[n16] & 1 << n15){
                        b2 = false;
                        break;
                    }
                    n15 += n9;
                    n16 += n10;
                } while (--worldDistToTileDist2 > 0);

                if (monsterField2 == 18 && !b2) {
                    n12 = -1;
                }
                if (monsterField3 == 18 && !b2) {
                    n11 = -1;
                }
            }

            int n17;
            if (n11 != -1 && n12 != -1) {
                n17 = (((int)app->nextByte() <= app->combat->getMonsterField(this->def, 2)) ? monsterField2 : monsterField3);
            }
            else if (n11 != -1) {
                n17 = monsterField3;
            }
            else {
                if (n12 == -1) {
                    return -1;
                }
                n17 = monsterField2;
            }
            return n17;
        }
    }
}

LerpSprite* Entity::aiInitLerp(int travelTime) {
    Applet* app = CAppContainer::getInstance()->app;

    int sprite = this->getSprite();
    LerpSprite* allocLerpSprite = app->game->allocLerpSprite(nullptr, sprite, true);
    allocLerpSprite->srcX = app->render->mapSprites[app->render->S_X + sprite];
    allocLerpSprite->srcY = app->render->mapSprites[app->render->S_Y + sprite];
    allocLerpSprite->srcZ = app->render->mapSprites[app->render->S_Z + sprite];
    allocLerpSprite->dstX = 32 + (this->monster->goalX << 6);
    allocLerpSprite->dstY = 32 + (this->monster->goalY << 6);
    allocLerpSprite->dstZ = 32 + app->render->getHeight(allocLerpSprite->dstX, allocLerpSprite->dstY);
    allocLerpSprite->srcScale = allocLerpSprite->dstScale = app->render->mapSprites[app->render->S_SCALEFACTOR + sprite];
    allocLerpSprite->startTime = app->gameTime;
    allocLerpSprite->travelTime = travelTime;
    allocLerpSprite->flags |= (Enums::LS_FLAG_ENT_NORELINK | Enums::LS_FLAG_ASYNC);
    this->monster->frameTime = app->time + travelTime;
    allocLerpSprite->calcDist();
    this->monster->goalFlags |= 0x1;
    return allocLerpSprite;
}

void Entity::aiFinishLerp() {
    this->monster->goalFlags &= ~1;
    if ((this->monster->flags & 0x1000) != 0x0) {
        this->monster->flags &= ~0x1000;
        this->info &= ~0x10000000;
    }
    else {
        this->aiReachedGoal_MOVE();
    }
}

bool Entity::checkLineOfSight(int n, int n2, int n3, int n4, int n5) {
    Applet* app = CAppContainer::getInstance()->app;

    int a = n3 - n;
    int a2 = n4 - n2;
    if (a != 0 && a2 != 0) {
        return false;
    }
    if (a != 0) {
        a /= std::abs(a);
    }
    if (a2 != 0) {
        a2 /= std::abs(a2);
    }
    while (n != n3 && n2 != n4) {
        n += a;
        n2 += a2;
        if (app->game->findMapEntity(n << 6, n2 << 6, n5)) {
            return false;
        }
    }
    return true;
}

bool Entity::calcPath(int n, int n2, int n3, int n4, int n5, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    int n6 = n3 - n;
    int n7 = n4 - n2;
    int closestPathDist = n6 * n6 + n7 * n7;
    uint8_t* visitOrder = app->game->visitOrder;
    int* visitDist = app->game->visitDist;
    app->game->visitedTiles[n2] |= 1 << n;
    bool checkLineOfSight = this->checkLineOfSight(n, n2, n3, n4, n5 | 0x100);
    if ((app->game->lineOfSight == 0 && !checkLineOfSight) || (app->game->lineOfSight == 1 && checkLineOfSight)) {
        if (b) {
            closestPathDist -= 30;
        }
        else {
            closestPathDist += 30;
        }
    }
    if (checkLineOfSight) {
        closestPathDist += app->game->lineOfSightWeight;
    }
    if (app->game->pathDepth > 0 && ((!b && closestPathDist < app->game->closestPathDist) || (b && closestPathDist > app->game->closestPathDist))) {
        app->game->closestPath = app->game->curPath;
        app->game->closestPathDepth = app->game->pathDepth;
        app->game->closestPathDist = closestPathDist;
    }
    if (n == n3 && n2 == n4) {
        app->game->closestPath = app->game->curPath;
        app->game->closestPathDepth = app->game->pathDepth;
        app->game->closestPathDist = closestPathDist;
        return true;
    }
    if (app->game->pathDepth == app->game->pathSearchDepth) {
        return false;
    }
    int n8 = 0;
    const int* viewStepValues = Canvas::viewStepValues;
    int n9 = 4;
    uint8_t b2 = (uint8_t)(app->nextByte() & 0x3);
    while (--n9 >= 0) {
        b2 = (uint8_t)(b2 + 1 & 0x3);
        int n10 = n + (viewStepValues[(b2 << 2) + 0] >> 6);
        int n11 = n2 + (viewStepValues[(b2 << 2) + 1] >> 6);
        if (n11 >= 0 && n11 < 32 && n10 >= 0) {
            if (n10 >= 32) {
                continue;
            }
            if (app->game->findMapEntity(n10 << 6, n11 << 6, 256) != nullptr) {
                continue;
            }
            if ((app->game->visitedTiles[n11] & 1 << n10) != 0) {
                continue;
            }
            visitOrder[n8] = b2;
            int n12 = n3 - n10;
            int n13 = n4 - n11;
            visitDist[n8] = n12 * n12 + n13 * n13;
            bool checkLineOfSight2 = this->checkLineOfSight(n10, n11, n3, n4, n5 | 0x100);
            if ((app->game->lineOfSight == 0 && !checkLineOfSight2) || (app->game->lineOfSight == 1 && checkLineOfSight2)) {
                if (b) {
                    visitDist[n8] -= 30;
                }
                else {
                    visitDist[n8] += 30;
                }
            }
            if (checkLineOfSight2) {
                visitDist[n8] += app->game->lineOfSightWeight;
            }
            ++n8;
        }
    }
    for (int i = 0; i < n8; ++i) {
        for (int j = 0; j < n8 - i - 1; ++j) {
            int n17 = visitDist[j] - visitDist[j + 1];
            if ((!b && n17 > 0) || (b && n17 < 0)) {
                int n18 = visitDist[j + 1];
                visitDist[j + 1] = visitDist[j];
                visitDist[j] = n18;
                uint8_t b3 = visitOrder[j + 1];
                visitOrder[j + 1] = visitOrder[j];
                visitOrder[j] = b3;
            }
        }
    }
    int n19 = 0;
    int n20 = 0;
    for (int k = 0; k < n8; ++k) {
        n19 |= (visitOrder[k] & 0x3) << n20;
        n20 += 2;
    }
    for (int l = 0; l < n8; ++l) {
        int n21 = n19 & 0x3;
        n19 >>= 2;
        int n22 = n + (viewStepValues[(n21 << 2) + 0] >> 6);
        int n23 = n2 + (viewStepValues[(n21 << 2) + 1] >> 6);
        app->game->trace((n << 6) + 32, (n2 << 6) + 32, (n22 << 6) + 32, (n23 << 6) + 32, app->game->skipEnt, n5, 16);
        if (app->game->findEnt != nullptr && app->game->traceEntity == app->game->findEnt) {
            app->game->closestPath = app->game->curPath;
            app->game->closestPathDepth = app->game->pathDepth;
            app->game->closestPathDist = closestPathDist;
            return true;
        }
        int interactClipMask = app->game->interactClipMask;
        if (app->game->traceEntity != nullptr) {
            interactClipMask = 1 << app->game->traceEntity->def->eType;
        }
        if (interactClipMask == 0 || (interactClipMask & app->game->interactClipMask) != 0x0) {
            ++app->game->pathDepth;
            app->game->curPath >>= 2;
            app->game->curPath &= 0x3FFFFFFFFFFFFFFFLL;
            app->game->curPath |= (int64_t)n21 << 62;
            if (this->calcPath(n22, n23, n3, n4, n5, b)) {
                return true;
            }
            --app->game->pathDepth;
            app->game->curPath <<= 2;
        }
    }
    return false;
}

bool Entity::aiGoal_MOVE() {
    Applet* app = CAppContainer::getInstance()->app;

    bool b = false;
    int sprite = this->getSprite();
    app->game->snapLerpSprites(sprite);
    int sX = (int)app->render->mapSprites[app->render->S_X + sprite];
    int sY = (int)app->render->mapSprites[app->render->S_Y + sprite];
    app->game->closestPath = 0LL;
    app->game->closestPathDepth = 0;
    app->game->closestPathDist = 999999999;
    app->game->curPath = 0LL;
    app->game->pathDepth = 0;
    app->game->pathSearchDepth = 8;
    app->game->findEnt = nullptr;
    app->game->skipEnt = this;
    app->game->lineOfSight = 2;
    app->game->lineOfSightWeight = 0;
    app->game->interactClipMask = 32;
    std::memcpy(app->game->visitedTiles, app->game->baseVisitedTiles, sizeof(app->game->visitedTiles));
    if (this->monster->goalType == 2 && this->monster->goalParam == 1) {
        app->game->findEnt = &app->game->entities[1];
        this->monster->goalX = app->game->destX >> 6;
        this->monster->goalY = app->game->destY >> 6;
        app->game->lineOfSightWeight = -4;
    }
    else if (this->monster->goalType == 5) {
        this->monster->goalX = app->game->destX >> 6;
        this->monster->goalY = app->game->destY >> 6;
        app->game->interactClipMask = 0;
        b = true;
        app->game->lineOfSight = 1;
        app->game->pathSearchDepth = this->monster->goalParam;
    }
    else if (this->monster->goalType == 4) {
        this->monster->goalX = app->game->destX >> 6;
        this->monster->goalY = app->game->destY >> 6;
        b = true;
        app->game->lineOfSight = 1;
    }
    else if (this->monster->goalType == 2) {
        app->game->findEnt = &app->game->entities[this->monster->goalParam];
        this->monster->goalX = app->game->findEnt->linkIndex % 32;
        this->monster->goalY = app->game->findEnt->linkIndex / 32;
    }
    if (b) {
        app->game->closestPathDist = 0;
    }
    int calcPath = this->calcPath(sX >> 6, sY >> 6, this->monster->goalX, this->monster->goalY, 15535, b) ? 1 : 0;
    if (calcPath == 0 && app->game->closestPathDist < 999999999) {
        calcPath = 1;
        app->game->curPath = app->game->closestPath;
        app->game->pathDepth = app->game->closestPathDepth;
    }
    if (calcPath != 0 && app->game->pathDepth > 0) {
        app->game->curPath >>= 64 - app->game->pathDepth * 2;
        this->info &= 0xEFFFFFFF;
        int dX = sX + Canvas::viewStepValues[(int)((app->game->curPath & 0x3LL) << 2) + 0];
        int dY = sY + Canvas::viewStepValues[(int)((app->game->curPath & 0x3LL) << 2) + 1];
        this->monster->goalX = dX >> 6;
        this->monster->goalY = dY >> 6;
        app->game->trace(sX, sY, dX, dY, this, app->game->interactClipMask, 25);
        if (app->game->numTraceEntities == 0) {
            app->game->unlinkEntity(this);
            app->game->linkEntity(this, dX >> 6, dY >> 6);
            if (!app->render->cullBoundingBox(std::min(sX, dX) - 16 << 4, std::min(sY, dY) - 16 << 4, std::max(sX, dX) + 16 << 4, std::max(sY, dY) + 16 << 4, true)) {
                this->info |= 0x10000000;
            }
            app->game->interpolatingMonsters = true;
            if ((this->def->eSubType == 14 && this->def->parm != 0) || this->def->eSubType == 13) {
                this->aiInitLerp(500);
            }
            else {
                this->aiInitLerp(275);
            }
        }
        else {
            this->monster->goalX = sX >> 6;
            this->monster->goalY = sY >> 6;
            if (app->game->traceEntity->def->eType == 5) {
                app->game->performDoorEvent(0, app->game->traceEntity, 2);
            }
        }
        return true;
    }
    return false;
}

void Entity::aiReachedGoal_MOVE() {
    Applet* app = CAppContainer::getInstance()->app;
    EntityMonster* monster = this->monster;
    EntityDef* def = this->def;
    this->info &= ~0x10000000;
    if (monster->goalType != 4 && monster->goalType != 5 && ((1 << def->eSubType & Enums::MOVE2ATTACK_MONSTERS) || ((1 << def->eSubType & Enums::EVADING_MONSTERS) && !(monster->flags & 0x1)))) {
        Entity* target = &app->game->entities[1];
        int aiWeaponForTarget = this->aiWeaponForTarget(target);
        if (aiWeaponForTarget != -1) {
            if (target == &app->game->entities[1]) {
                monster->target = nullptr;
            }
            else {
                monster->target = target;
            }
            monster->ce.weapon = aiWeaponForTarget;
            this->attack();
            return;
        }
    }
    if ((1 << def->eSubType & Enums::EVADING_MONSTERS) != 0x0) {
        monster->flags |= 0x1;
    }
    if ((monster->goalFlags & 0x10) != 0x0) {
        monster->goalFlags &= ~0x10;
        this->aiCalcSimpleGoal(false);
        if (monster->goalType == 1 || monster->goalType == 2) {
            if (!app->game->tileObstructsAttack(monster->goalX, monster->goalY)) {
                this->aiGoal_MOVE();
            }
            else {
                monster->resetGoal();
            }
        }
    }
}

int Entity::distFrom(int n, int n2) {
    int* calcPosition = this->calcPosition();
    return std::max((n - calcPosition[0]) * (n - calcPosition[0]), (n2 - calcPosition[1]) * (n2 - calcPosition[1]));
}

void Entity::attack() {
    Applet* app = CAppContainer::getInstance()->app;
    if ((this->monster->flags & 0x400) == 0x0) {
        this->monster->flags |= 0x400;
        this->monster->nextAttacker = app->game->combatMonsters;
        app->game->combatMonsters = this;
    }
}

void Entity::undoAttack() {
    Applet* app = CAppContainer::getInstance()->app;
    if ((this->monster->flags & 0x400) == 0x0) {
        return;
    }
    monster->flags &= ~0x400;
    int sprite = this->getSprite();
    if (this->monster->ce.weapon == 18) {
        app->render->mapSpriteInfo[sprite] &= 0xFFFF00FF;
    }
    this->monster->resetGoal();
    Entity* entity = app->game->combatMonsters;
    Entity* entity2 = nullptr;
    while (entity != nullptr && entity != this) {
        entity2 = entity;
        entity = entity->monster->nextAttacker;
    }
    if (entity2 != nullptr) {
        entity2->monster->nextAttacker = this->monster->nextAttacker;
    }
    else if (app->game->combatMonsters != nullptr) {
        app->game->combatMonsters = this->monster->nextAttacker;
    }
}

void Entity::trimCorpsePile(int n, int n2) {
    Applet* app = CAppContainer::getInstance()->app;
    Entity* entity = app->game->inactiveMonsters;
    if (entity != nullptr) {
        int n3 = 0;
        do {
            int sprite = entity->getSprite();
            if (app->render->mapSprites[app->render->S_X + sprite] == n && app->render->mapSprites[app->render->S_Y + sprite] == n2 && (entity->info & 0x1010000) != 0x0 && (app->render->mapSpriteInfo[sprite] & 0x10000) == 0x0 && ++n3 >= 3) {
                app->render->mapSpriteInfo[sprite] |= 0x10000;
                entity->info = ((entity->info & 0xFEFFFFFF) | 0x10000);
                app->game->unlinkEntity(entity);
            }
            entity = entity->monster->nextOnList;
        } while (entity != app->game->inactiveMonsters);
    }
}

void Entity::knockback(int n, int n2, int n3) {
    Applet* app = CAppContainer::getInstance()->app;
    int32_t* knockbackDelta = this->knockbackDelta;
    if (n3 == 0) {
        return;
    }
    int destX;
    int destY;
    int n4;
    if (this->def->eType == Enums::ET_PLAYER) {
        destX = app->game->destX;
        destY = app->game->destY;
        n4 = 13501;
    }
    else {
        int sprite = this->getSprite();
        destX = app->render->mapSprites[app->render->S_X + sprite];
        destY = app->render->mapSprites[app->render->S_Y + sprite];
        n4 = 15535;
    }
    knockbackDelta[0] = destX - n;
    knockbackDelta[1] = destY - n2;
    if (knockbackDelta[0] != 0) {
        knockbackDelta[0] /= std::abs(knockbackDelta[0]);
        app->canvas->knockbackStart = destX;
        app->canvas->knockbackWorldDist = std::abs(64 * knockbackDelta[0] * n3);
    }
    if (knockbackDelta[1] != 0) {
        knockbackDelta[1] /= std::abs(knockbackDelta[1]);
        app->canvas->knockbackStart = destY;
        app->canvas->knockbackWorldDist = std::abs(64 * knockbackDelta[1] * n3);
    }

    int farthestKnockbackDist = this->getFarthestKnockbackDist(destX, destY, destX + 64 * knockbackDelta[0] * n3, destY + 64 * knockbackDelta[1] * n3, this, n4, 16, n3);
    if (farthestKnockbackDist == 0 || (knockbackDelta[0] == 0 && knockbackDelta[1] == 0)) {
        return;
    }
    int goalX = destX + knockbackDelta[0] * farthestKnockbackDist * 64 >> 6;
    int goalY = destY + knockbackDelta[1] * farthestKnockbackDist * 64 >> 6;
    if (this->def->eType == Enums::ET_PLAYER) {
        if (this->def->eSubType != 1) {
            app->canvas->knockbackX = knockbackDelta[0];
            app->canvas->knockbackY = knockbackDelta[1];
            app->canvas->knockbackDist = farthestKnockbackDist;
        }
    }
    else {
        this->monster->goalType = 1;
        this->monster->goalX = goalX;
        this->monster->goalY = goalY;
        this->monster->flags |= 0x1000;
        LerpSprite* aiInitLerp = this->aiInitLerp(400);
        app->game->unlinkEntity(this);
        app->game->linkEntity(this, goalX, goalY);
        app->game->interpolatingMonsters = true;
        app->game->updateLerpSprite(aiInitLerp);
    }
}

int Entity::getFarthestKnockbackDist(int n, int n2, int n3, int n4, Entity* entity, int n5, int n6, int n7) {
    Applet* app = CAppContainer::getInstance()->app;
    int n8 = n7;
    app->game->trace(n, n2, n3, n4, entity, n5, n6);
    if (app->game->traceEntity != nullptr) {
        n8 = n8 * app->game->traceFracs[0] >> 14;
    }
    return n8;
}

int Entity::findRaiseTarget(int n, int n2, int n3) {
    Applet* app = CAppContainer::getInstance()->app;
    Entity* inactiveMonsters = app->game->inactiveMonsters;
    int* calcPosition = this->calcPosition();
    int n4 = calcPosition[0];
    int n5 = calcPosition[1];
    if (this->param != 0) {
        --this->param;
        return -1;
    }
    int n6 = 0;
    if (inactiveMonsters != nullptr) {
        do {
            Entity* nextOnList = inactiveMonsters->monster->nextOnList;
            int sprite = inactiveMonsters->getSprite();
            if ((inactiveMonsters->info & 0x10000) == 0x0 &&
                (inactiveMonsters->info & 0x8000000) == 0x0 &&
                (inactiveMonsters->info & 0x1000000) != 0x0 &&
                !inactiveMonsters->isBoss() &&
                (n2 == 0 || (inactiveMonsters->monster->flags & n2) != 0x0) &&
                (inactiveMonsters->monster->flags & n3) == 0x0 &&
                inactiveMonsters->distFrom(n4, n5) <= n &&
                app->game->findMapEntity(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], 15535) == nullptr &&
                (app->game->difficulty != 4 || 0x0 == (inactiveMonsters->monster->monsterEffects & 0x4)))
            {
                this->raiseTargets[n6++] = inactiveMonsters;
            }
            inactiveMonsters = nextOnList;
        } while (inactiveMonsters != app->game->inactiveMonsters && n6 != 4);
        if (n6 != 0) {
            Entity* entity = this->raiseTargets[app->nextInt() % n6];
            entity->info |= 0x8000000;
            this->raiseTarget(entity->getIndex());
            this->param = 4;
            return entity->getIndex();
        }
    }
    return -1;
}

void Entity::raiseTarget(int n) {
    Applet* app = CAppContainer::getInstance()->app;
    Entity* entity = &app->game->entities[n];
    int sprite = entity->getSprite();
    app->localization->resetTextArgs();
    Text* smallBuffer = app->localization->getSmallBuffer();
    app->localization->composeTextField(this->name, smallBuffer);
    app->localization->addTextArg(smallBuffer);
    smallBuffer->dispose();
    if (app->game->findMapEntity(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], 15535) != nullptr) {
        app->hud->addMessage((short)92);
        entity->info &= ~0x8000000;
        return;
    }
    entity->resurrect(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], app->render->mapSprites[app->render->S_Z + sprite]);
    app->game->activate(entity, false, false, true, true);
    Text* smallBuffer2 = app->localization->getSmallBuffer();
    app->localization->composeTextField(entity->name, smallBuffer2);
    app->localization->addTextArg(smallBuffer2);
    if (this->def->eType != 1) {
        app->hud->addMessage((short)93);
    }
    smallBuffer2->dispose();
}

void Entity::resurrect(int n, int n2, int n3) {
    Applet* app = CAppContainer::getInstance()->app;
    int sprite = this->getSprite();
    this->def = app->entityDefManager->find(2, this->def->eSubType, this->def->parm);
    this->name = (short)(this->def->name | 0x400);
    this->monster->clearEffects();
    app->render->mapSprites[app->render->S_X + sprite] = (short)n;
    app->render->mapSprites[app->render->S_Y + sprite] = (short)n2;
    app->render->mapSprites[app->render->S_Z + sprite] = (short)n3;
    app->render->mapSpriteInfo[sprite] &= 0xFFFC00FF;
    if ((app->nextInt() & 0x1) != 0x0) {
        app->render->mapSpriteInfo[sprite] |= 0x20000;
    }
    app->render->relinkSprite(sprite);
    this->info &= 0xF6FEFFFF;
    this->info |= 0x20000;
    CombatEntity* ce = &this->monster->ce;
    this->initspawn();
    ce->setStat(0, ce->getStat(1));
    this->monster->flags &= ~0x1000;
    app->game->unlinkEntity(this);
    app->game->linkEntity(this, n >> 6, n2 >> 6);
    app->canvas->updateFacingEntity = true;
    app->particleSystem->spawnParticles(1, -161512, sprite);
}

int* Entity::calcPosition() {
    Applet* app = CAppContainer::getInstance()->app;
    int x;
    int y;
    if (this->def->eType == Enums::ET_WORLD) {
        x = app->game->traceCollisionX;
        y = app->game->traceCollisionY;
    }
    else if (this->def->eType == Enums::ET_PLAYER) {
        x = app->canvas->destX;
        y = app->canvas->destY;
    }
    else if (this->def->eType == Enums::ET_MONSTER) {
        int sprite = this->getSprite();
        x = app->render->mapSprites[app->render->S_X + sprite];
        y = app->render->mapSprites[app->render->S_Y + sprite];
    }
    else {
        int sprite = this->getSprite();
        x = app->render->mapSprites[app->render->S_X + sprite];
        y = app->render->mapSprites[app->render->S_Y + sprite];
    }
    this->pos[0] = x;
    this->pos[1] = y;
    return this->pos;
}

bool Entity::isBoss() {
	return this->def->eSubType >= Enums::FIRSTBOSS && this->def->eSubType <= Enums::LASTBOSS && (this->def->eSubType != Enums::BOSS_MASTERMIND || this->def->parm != 0);
}

bool Entity::isHasteResistant() {
    return this->isBoss();
}

bool Entity::isDroppedEntity() {
    Applet* app = CAppContainer::getInstance()->app;
    short index = this->getIndex();
    return index >= app->game->firstDropIndex && index < app->game->firstDropIndex + 16;
}

bool Entity::isBinaryEntity(int* array) {
    Applet* app = CAppContainer::getInstance()->app;
    bool b = false;
    if (this->def == nullptr) {
        return false;
    }
    if (this->isDroppedEntity()) {
        return false;
    }
    switch (this->def->eType) {
        case Enums::ET_ITEM:
        case Enums::ET_ATTACK_INTERACTIVE:
        case Enums::ET_MONSTERBLOCK_ITEM: {
            if (this->def->eSubType == Enums::INTERACT_PICKUP) {
                b = false;
                break;
            }
            if ((this->info & 0x100000) != 0x0) {
                b = true;
                break;
            }
            b = false;
            break;
        }
        case Enums::ET_DOOR: {
            if (app->render->mapSprites[app->render->S_SCALEFACTOR + this->getSprite()] != 64) {
                b = true;
            }
            else {
                b = false;
            }
            if (this->def->eSubType == Enums::DOOR_LOCKED && nullptr != array) {
                array[1] |= 0x200000;
            }
            break;
        }
        case Enums::ET_NONOBSTRUCTING_SPRITEWALL: {
           int sprite = this->getSprite();
            if ((app->render->mapSpriteInfo[sprite] & 0xFF) == Enums::TILENUM_NONOBSTRUCTING_SPRITEWALL2) {
                if ((app->render->mapSpriteInfo[sprite] & 0x10000) != 0x0) {
                    b = true;
                }
                else {
                    b = false;
                }
            }
            break;
        }
        case Enums::ET_CORPSE: {
            if (this->def->eSubType == Enums::CORPSE_SKELETON) {
                b = (this->param != 0);
                break;
            }
            return false;
        }
        case Enums::ET_DECOR_NOCLIP: {
            if (this->def->eSubType == Enums::DECOR_WATER_SPOUT) {
                b = ((this->info & 0x400000) != 0x0);
                break;
            }
            return false;
        }
        default: {
            return false;
        }
    }
    if (nullptr != array) {
        array[0] = (b ? 1 : 0);
    }
    return true;
}

bool Entity::isNamedEntity(int* array) {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->def == nullptr || this->name == Localization::STRINGID((short)1, this->def->name) || this->def->eType == Enums::ET_CORPSE) {
        return false;
    }
    array[0] = this->name;
    if (array[0] != -1) {
        return true;
    }
    app->Error(25); // ERR_ISNAMEDENTITY
    return false;
}

void Entity::saveState(OutputStream* OS, int n) {
    Applet* app = CAppContainer::getInstance()->app;

    short* mapSprites = app->render->mapSprites;
    int* mapSpriteInfo = app->render->mapSpriteInfo;
    if ((n & 0x20000) != 0x0) {
        this->isNamedEntity(this->tempSaveBuf);
        OS->writeShort((int16_t)this->tempSaveBuf[0]);
    }
    if ((n & 0x80000) != 0x0) {
        return;
    }
    if (this->def->eType == Enums::ET_ATTACK_INTERACTIVE && this->def->eSubType == Enums::INTERACT_BARRICADE) {
        OS->writeShort(mapSprites[app->render->S_SCALEFACTOR + this->getSprite()]);
        return;
    }
    int sprite = this->getSprite();
    if (this->def->eType == Enums::ET_MONSTER && (n & 0x200000) != 0x0) {
        OS->writeByte(mapSprites[app->render->S_X + sprite] >> 3);
        OS->writeByte(mapSprites[app->render->S_Y + sprite] >> 3);
        OS->writeByte((mapSpriteInfo[sprite] & 0xFF00) >> 8);
        OS->writeShort(this->monster->flags);
        return;
    }
    OS->writeByte(this->info >> 16 & 0xFF);
    if ((this->info & 0x10000) == 0x0) {
        OS->writeByte((mapSpriteInfo[sprite] & 0xFF0000) >> 16);
        OS->writeByte((mapSpriteInfo[sprite] & 0xFF00) >> 8);
        if (this->isDroppedEntity() || (app->render->mapSpriteInfo[sprite] & 0xF000000) == 0x0) {
            OS->writeByte(mapSprites[app->render->S_X + sprite] >> 3);
            OS->writeByte(mapSprites[app->render->S_Y + sprite] >> 3);
        }
        if (this->isDroppedEntity()) {
            OS->writeInt(this->param);
        }
        if (!this->isDroppedEntity() && (app->render->mapSpriteInfo[sprite] & 0xF000000) != 0x0) {
            OS->writeShort(this->linkIndex);
        }
    }
    if (this->monster != nullptr) {
        OS->writeShort(this->monster->flags);
        if ((this->info & 0x10000) == 0x0) {
            if ((this->monster->flags & 0x200) != 0x0) {
                OS->writeByte(mapSprites[app->render->S_SCALEFACTOR + sprite]);
            }
            if ((n & 0x100000) == 0x0) {
                OS->writeShort(this->monster->monsterEffects);
                this->monster->ce.saveState(OS, false);
                this->monster->saveGoalState(OS);
            }
        }
    }
    else if (this->isDroppedEntity()) {
        OS->writeByte((uint8_t)(this->def->eType | this->def->eSubType << 4));
        OS->writeByte(this->def->parm);
        if ((n & 0x100000) != 0x0) {
            int v3;
            int v2;
            int v = v2 = (v3 = 0);
            if (this->lootSet != nullptr) {
                v2 = this->lootSet[0];
                v = this->lootSet[1];
                v3 = this->lootSet[2];
            }
            OS->writeInt(v2);
            OS->writeInt(v);
            OS->writeInt(v3);
        }
        if (this->def->eType == Enums::ET_DECOR_NOCLIP && this->def->eSubType == Enums::DECOR_DYNAMITE) {
            OS->writeByte((mapSpriteInfo[sprite] & 0xFF000000) >> 24);
        }
    }
    else {
        OS->writeShort(mapSprites[app->render->S_Z + sprite]);
    }
}

void Entity::loadState(InputStream* IS, int n) {
    Applet* app = CAppContainer::getInstance()->app;

    if ((n & 0x20000) != 0x0) {
        this->name = IS->readShort();
    }
    int sprite = this->getSprite();
    int n2 = app->render->mapSpriteInfo[sprite] & 0xFF;
    if ((app->render->mapSpriteInfo[sprite] & 0x400000) != 0x0) {
        n2 += 257;
    }
    if ((n & 0x40000) != 0x0) {
        app->render->mapSpriteInfo[sprite] &= 0xFFFEFFFF;
        if ((n & 0x80000) != 0x0 && (this->info & 0x100000) == 0x0) {
            app->game->linkEntity(this, app->render->mapSprites[app->render->S_X + sprite] >> 6, app->render->mapSprites[app->render->S_Y + sprite] >> 6);
        }
    }
    else {
        app->render->mapSpriteInfo[sprite] |= 0x10000;
        if ((this->info & 0x100000) != 0x0 && this->def->eType != Enums::ET_DOOR && (n2 < Enums::TILENUM_DUMMY_START || n2 > Enums::TILENUM_DUMMY_END)) {
            app->game->unlinkEntity(this);
        }
        else if (this->def->eType == Enums::ET_ATTACK_INTERACTIVE && this->def->eSubType != Enums::INTERACT_PICKUP) {
            ++app->game->destroyedObj;
        }
    }
    if ((n & 0x1000000) != 0x0) {
        this->info |= 0x2000000;
    }
    if (this->isBinaryEntity(nullptr)) {
        this->restoreBinaryState(n);
        if ((n & 0x80000) != 0x0) {
            return;
        }
    }
    if (this->def != nullptr && this->def->eType == Enums::ET_ATTACK_INTERACTIVE && this->def->eSubType == Enums::INTERACT_BARRICADE) {
        short short1 = IS->readShort();
        if (short1 != app->render->mapSprites[app->render->S_SCALEFACTOR + sprite]) {
            app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = short1;
            this->info |= 0x400000;
        }
        return;
    }
    if ((n & 0x100000) != 0x0) {
        if ((n & 0x40000) == 0x0) {
            this->info |= 0x10000;
            if ((this->info & 0x100000) != 0x0) {
                app->game->unlinkEntity(this);
            }
        }
        else if ((n & 0x80000) != 0x0) {
            app->Error(Enums::ERR_CLEANCORPSE);
        }
        app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x7000);
        if (!this->isDroppedEntity()) {
            this->def = app->entityDefManager->find(9, this->def->eSubType, this->def->parm);
        }
    }
    if (this->def != nullptr && this->def->eType == Enums::ET_NPC) {
        this->param = (((n & 0x200000) != 0x0) ? 1 : 0);
        this->param += (((n & 0x4000000) != 0x0) ? 1 : 0);
    }
    if (this->monster != nullptr) {
        if ((n & 0x800000) != 0x0) {
            this->monster->flags |= 0x10;
            this->info |= 0x400000;
        }
        if ((n & 0x400000) != 0x0) {
            this->monster->flags |= 0x80;
            this->info |= 0x400000;
        }
        if ((n & 0x8000000) != 0x0) {
            this->monster->flags |= 0x800;
            this->info |= 0x400000;
        }
    }
    if ((n & 0x80000) != 0x0) {
        return;
    }
    if ((this->info & 0x100000) != 0x0) {
        app->game->unlinkEntity(this);
    }
    int sprite2 = this->getSprite();
    if (this->def != nullptr && this->def->eType == Enums::ET_MONSTER && (n & 0x200000) != 0x0) {
        short n5 = (short)((IS->readByte() & 0xFF) << 3);
        short n6 = (short)((IS->readByte() & 0xFF) << 3);
        int n7 = (IS->readByte() & 0xFF) << 8;
        app->render->mapSprites[app->render->S_X + sprite2] = n5;
        app->render->mapSprites[app->render->S_Y + sprite2] = n6;
        app->render->mapSprites[app->render->S_Z + sprite2] = (short)(app->render->getHeight(n5, n6) + 32);
        app->render->mapSpriteInfo[sprite2] = ((app->render->mapSpriteInfo[sprite2] & 0xFFFF00FF) | n7);
        app->render->relinkSprite(sprite2);
        if ((this->info & 0x100000) != 0x0) {
            app->game->unlinkEntity(this);
        }
        if ((n & 0x40000) == 0x0) {
            app->game->deactivate(this);
        }
        else {
            app->game->linkEntity(this, n5 >> 6, n6 >> 6);
        }
        this->monster->flags = IS->readShort();
        if (this->monster->flags != 0 || n7 != 0) {
            this->info |= 0x400000;
        }
        return;
    }
    this->info = ((this->info & 0xFF00FFFF) | (IS->readByte() & 0xFF) << 16);
    if ((this->info & 0x10000) == 0x0) {
        int n8 = IS->readByte() & 0xFF;
        int n9 = IS->readByte() & 0xFF;
        app->render->mapSpriteInfo[sprite2] = ((app->render->mapSpriteInfo[sprite2] & 0xFF0000FF) | n9 << 8 | n8 << 16);
        if (this->isDroppedEntity() || (app->render->mapSpriteInfo[sprite2] & 0xF000000) == 0x0) {
            app->render->mapSprites[app->render->S_X + sprite2] = (short)(IS->readUnsignedByte() << 3);
            app->render->mapSprites[app->render->S_Y + sprite2] = (short)(IS->readUnsignedByte() << 3);
            if (this->monster != nullptr || this->isDroppedEntity()) {
                app->render->mapSprites[app->render->S_Z + sprite2] = (short)(app->render->getHeight(app->render->mapSprites[app->render->S_X + sprite2], app->render->mapSprites[app->render->S_Y + sprite2]) + 32);
            }
            app->render->relinkSprite(sprite2);
        }
        if (this->isDroppedEntity()) {
            this->param = IS->readInt();
        }
        if (!this->isDroppedEntity() && (app->render->mapSpriteInfo[sprite2] & 0xF000000) != 0x0) {
            this->linkIndex = IS->readShort();
        }
        else {
            this->linkIndex = (short)((app->render->mapSprites[app->render->S_X + sprite2] >> 6) + (app->render->mapSprites[app->render->S_Y + sprite2] >> 6) * 32);
        }
        if (((app->render->mapSpriteInfo[sprite2] & 0xF000000) == 0xC000000 || (app->render->mapSpriteInfo[sprite2] & 0xF000000) == 0x3000000) && (this->def->eType == Enums::ET_NONOBSTRUCTING_SPRITEWALL || this->def->eType == Enums::ET_SPRITEWALL)) {
            int n10 = this->linkIndex % 32;
            int n11 = this->linkIndex / 32;
            app->render->mapSprites[app->render->S_X + sprite2] = (short)((n10 << 6) + 32);
            app->render->mapSprites[app->render->S_Y + sprite2] = (short)((n11 << 6) + 32);
            app->render->relinkSprite(sprite2);
        }
        if (n9 != 0) {
            this->info |= 0x400000;
        }
    }
    if ((n & 0x2000000) != 0x0) {
        this->info |= 0x80000;
    }
    if (this->monster != nullptr) {
        this->monster->flags = IS->readShort();
        if ((this->info & 0x10000) == 0x0) {
            if ((this->monster->flags & 0x200) != 0x0) {
                app->render->mapSprites[app->render->S_SCALEFACTOR + sprite2] = (short)IS->readUnsignedByte();
            }
            if ((n & 0x100000) != 0x0) {
                this->info |= 0x1000000;
            }
            else {
                this->monster->monsterEffects = IS->readShort();
                this->monster->ce.loadState(IS, false);
                this->monster->loadGoalState(IS);
            }
        }
        if ((this->info & 0x40000) != 0x0) {
            this->info &= 0xFFFBFFFF;
            app->game->activate(this, false, false, false, true);
        }
        if ((this->info & 0x1000000) != 0x0) {
            this->def = app->entityDefManager->find(9, this->def->eSubType, this->def->parm);
        }
    }
    else if (this->isDroppedEntity()) {
        uint8_t byte1 = IS->readByte();
        uint8_t b = (uint8_t)(byte1 & 0xF);
        uint8_t b2 = (uint8_t)(byte1 >> 4 & 0xF);
        uint8_t byte2 = IS->readByte();
        this->def = app->entityDefManager->find(b, b2, byte2);
        if (this->name == -1) {
            this->name = (short)(this->def->name | 0x400);
        }
        short n12 = this->def->tileIndex;
        if ((n & 0x100000) != 0x0) {
            n12 = app->entityDefManager->find(2, b2, byte2)->tileIndex;
            int int1 = IS->readInt();
            int int2 = IS->readInt();
            int int3 = IS->readInt();
            if (this->param == 0) {
                this->populateDefaultLootSet();
                this->lootSet[0] = int1;
                this->lootSet[1] = int2;
                this->lootSet[2] = int3;
            }
            this->info |= 0x1000000;
        }
        app->render->mapSpriteInfo[sprite2] = ((app->render->mapSpriteInfo[sprite2] & 0xFFFFFF00) | n12);
        app->render->mapSprites[app->render->S_ENT + sprite2] = this->getIndex();
        if (this->def->eType == Enums::ET_DECOR_NOCLIP && this->def->eSubType == Enums::DECOR_DYNAMITE) {
            int n13 = (IS->readByte() & 0xFF) << 24;
            app->render->mapSpriteInfo[sprite2] |= n13;
            app->render->mapSprites[app->render->S_Z + sprite2] = (short)(app->render->getHeight(app->render->mapSprites[app->render->S_X + sprite2], app->render->mapSprites[app->render->S_Y + sprite2]) + (((app->render->mapSpriteInfo[sprite2] & 0xF000000) != 0x0) ? 32 : 31));
            app->render->mapSprites[app->render->S_SCALEFACTOR + sprite2] = 32;
            app->render->relinkSprite(sprite2);
        }
    }
    else {
        app->render->mapSprites[app->render->S_Z + sprite2] = IS->readShort();
        app->render->relinkSprite(sprite2);
    }
    if ((this->info & 0x100000) != 0x0) {
        if ((app->render->mapSpriteInfo[sprite2] & 0xF000000) != 0x0) {
            app->game->linkEntity(this, this->linkIndex % 32, this->linkIndex / 32);
        }
        else {
            app->game->linkEntity(this, app->render->mapSprites[app->render->S_X + sprite2] >> 6, app->render->mapSprites[app->render->S_Y + sprite2] >> 6);
        }
    }
}

int Entity::getSaveHandle(bool b) {
    Applet* app = CAppContainer::getInstance()->app;
    int* tempSaveBuf = this->tempSaveBuf;
    tempSaveBuf[tempSaveBuf[0] = 1] = this->getIndex();
    bool droppedEntity = this->isDroppedEntity();
    bool binaryEntity = this->isBinaryEntity(tempSaveBuf);
    if (((this->info & 0xFFFF) == 0x0 || this->def == nullptr) && !binaryEntity) {
        return -1;
    }
    if (droppedEntity && (this->info & 0x100000) == 0x0) {
        return -1;
    }
    if (droppedEntity && b && this->def->eType == Enums::ET_DECOR_NOCLIP && this->def->eSubType == Enums::DECOR_DYNAMITE) {
        return -1;
    }
    bool b2 = tempSaveBuf[0] != 0;
    int n = tempSaveBuf[1];
    if (binaryEntity && b2) {
        n |= 0x10000;
    }
    if (this->isNamedEntity(this->tempSaveBuf)) {
        n |= 0x20000;
    }
    if ((app->render->mapSpriteInfo[this->getSprite()] & 0x10000) == 0x0) {
        n |= 0x40000;
    }
    if ((this->info & 0x2000000) != 0x0) {
        n |= 0x1000000;
    }
    if ((this->info & 0x400000) == 0x0) {
        n |= 0x80000;
        if ((n & 0x20000) == 0x0 && this->def->eType == Enums::ET_DECOR && this->def->eSubType != Enums::DECOR_STATUE) {
            return -1;
        }
    }
    if ((this->info & 0x1010000) != 0x0) {
        n |= 0x100000;
    }
    if ((this->info & 0x80000) != 0x0) {
        n |= 0x2000000;
    }
    if ((this->info & 0x10000) != 0x0) {
        n = ((n & 0xFFFBFFFF) | 0x100000);
    }
    if (this->def->eType == Enums::ET_NPC && this->param != 0) {
        n |= 0x200000;
        if (this->param == 2) {
            n |= 0x4000000;
        }
    }
    if (this->monster != nullptr) {
        if ((this->monster->flags & 0x10) != 0x0) {
            n |= 0x800000;
        }
        if ((this->monster->flags & 0x80) != 0x0) {
            n |= 0x400000;
        }
        if ((this->monster->flags & 0x800) != 0x0) {
            n |= 0x8000000;
        }
    }
    if (b) {
        if (this->def->eType == Enums::ET_CORPSE && this->def->eSubType != Enums::CORPSE_SKELETON && !droppedEntity && 0x0 == (this->monster->flags & 0x80)) {
            n = ((n & 0xFFFBFFFF) | 0x80000);
        }
        else if (this->def->eType == Enums::ET_MONSTER) {
            n |= 0x200000;
        }
    }
    return n;
}

void Entity::restoreBinaryState(int n) {
    Applet* app = CAppContainer::getInstance()->app;
    bool b = (n & 0x10000) != 0x0;
    switch (this->def->eType) {
        case Enums::ET_ITEM:
        case Enums::ET_ATTACK_INTERACTIVE:
        case Enums::ET_MONSTERBLOCK_ITEM: {
            if (this->def->eSubType == Enums::INTERACT_PICKUP) {
                if (b) {
                    app->canvas->turnEntityIntoWaterSpout(this);
                    break;
                }
                break;
            }
            else {
                if (b) {
                    app->game->unlinkEntity(this);
                    app->game->linkEntity(this, this->linkIndex % 32, this->linkIndex / 32);
                }
                else {
                    app->game->unlinkEntity(this);
                }
                if (this->def->eSubType == Enums::INTERACT_BARRICADE || this->def->eSubType == Enums::INTERACT_CRATE) {
                    int sprite = this->getSprite();
                    if (b) {
                        app->game->unlinkEntity(this);
                        app->game->linkEntity(this, this->linkIndex % 32, this->linkIndex / 32);
                        app->render->relinkSprite(sprite);
                    }
                    else {
                        app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | ((this->def->eSubType == 2) ? 3 : 1) << 8);
                        app->render->relinkSprite(sprite);
                    }
                    break;
                }
                break;
            }
            break;
        }
        case Enums::ET_DOOR: {
            bool b2 = this->def->eSubType == Enums::DOOR_LOCKED;
            app->game->setLineLocked(this, false);
            if (b) {
                app->game->performDoorEvent(0, this, 0);
            }
            else {
                app->game->performDoorEvent(1, this, 0);
            }
            if ((n & 0x200000) != 0x0 || b2) {
                app->game->setLineLocked(this, (n & 0x200000) != 0x0);
            }
            break;
        }
        case Enums::ET_NONOBSTRUCTING_SPRITEWALL: {
            if (b) {
                app->game->performDoorEvent(0, this, 0);
                break;
            }
            app->game->performDoorEvent(1, this, 0);
            break;
        }
        case Enums::ET_CORPSE: {
            if (b) {
                ++this->param;
                this->lootSet = nullptr;
                break;
            }
            break;
        }
        default: {
            break;
        }
    }
}

short Entity::getIndex() {
    Applet* app = CAppContainer::getInstance()->app;
    for (short n = 0; n < app->game->numEntities; ++n) {
        if (this == &app->game->entities[n]) {
            return n;
        }
    }
    return -1;
}

void Entity::updateMonsterFX() {
    Applet* app = CAppContainer::getInstance()->app;
    if (nullptr != this->monster) {
        for (int i = 0; i < 5; ++i) {
            int n = 1 << i;
            int n2 = this->monster->monsterEffects;
            if ((n2 & n) != 0x0) {
                int n3 = 5 + i * 4;
                int n4 = n2 >> n3 & 0xF;
                if (this->def->eType != Enums::ET_CORPSE && (this->info & 0x20000) != 0x0) {
                    int n5 = 0;
                    if (n == 8) {
                        n5 = 4;
                        app->localization->resetTextArgs();
                        app->localization->addTextArg(n5);
                        app->hud->addMessage((short)0, (short)94);
                    }

                    if (n == 1) {
                        n5 = 10;
                        app->localization->resetTextArgs();
                        app->localization->addTextArg(n5);
                        app->hud->addMessage((short)0, (short)95);
                    }
                    else if (n5 > 0) {
                        app->localization->resetTextArgs();
                        app->localization->addTextArg(n5);
                        app->hud->addMessage((short)0, (short)71);
                    }
                    if (n5 > 0) {
                        this->pain(n5, nullptr);
                        n2 = this->monster->monsterEffects;
                        if (this->monster->ce.getStat(0) <= 0) {
                            this->died(true, nullptr);
                            n2 = ((this->monster->monsterEffects & 0xFFFF801F) | 0x220220);
                            n4 = 1;
                        }
                    }
                }
                int monsterEffects;
                if (n4 == 0) {
                    monsterEffects = (n2 & ~n);
                }
                else {
                    --n4;
                    monsterEffects = ((n2 & ~(15 << n3)) | n4 << n3);
                }
                this->monster->monsterEffects = monsterEffects;
            }
        }
    }
}

void Entity::populateDefaultLootSet() {
    if (this->lootSet) {
        delete this->lootSet;
        this->lootSet = nullptr;
    }

    this->lootSet = new int[3];
    this->lootSet[0] = 0;
    this->lootSet[1] = 0;
    this->lootSet[2] = 0;

    if (this->def->eType == Enums::ET_CORPSE) {
        if (this->def->eSubType != Enums::MONSTER_SENTRY_BOT) {
            this->lootSet[0] = 1089;
        }
    }
    else {
        switch (this->def->eSubType) {
            case Enums::MONSTER_ZOMBIE: {
                int n = this->getSprite() % 3 + 1;
                if (this->def->parm == 1) {
                    n = this->getSprite() % 5 + 3;
                }
                else if (this->def->parm == 2) {
                    n = this->getSprite() % 8 + 5;
                }
                this->lootSet[0] = (0x600 | n);
                break;
            }
            case Enums::MONSTER_CACODEMON: {
                this->lootSet[0] = (0x2100 | this->getSprite() % 5 + 3);
                break;
            }
            case Enums::MONSTER_MANCUBUS: {
                this->lootSet[0] = (0x2140 | this->getSprite() % 3 + 1);
                break;
            }
            case Enums::MONSTER_REVENANT: {
                this->lootSet[0] = (0x2140 | this->getSprite() % 3 + 3);
                break;
            }
            case Enums::MONSTER_SENTRY_BOT: {
                this->lootSet[0] = (0x2040 | this->getSprite() % 6 + 6);
                break;
            }
            default: {
                this->lootSet[0] = (0x6000 | this->findRandomJokeItem());
                break;
            }
        }
    }
}

int Entity::findRandomJokeItem() {
    Applet* app = CAppContainer::getInstance()->app;

    int sprite = this->getSprite();

    switch (app->canvas->loadMapID) {
        case 1: {
            switch (sprite % 5) {
                case 0: {
                    return 180;
                }
                case 1: {
                    return 181;
                }
                case 2: {
                    return 182;
                }
                case 3: {
                    return 183;
                }
                case 4: {
                    return 184;
                }
                default: {
                    goto LABEL_9;
                }
            }
            break;
        }
        case 2: {
            LABEL_9:
            switch (sprite % 5) {
                case 0: {
                    return 149;
                }
                case 1: {
                    return 150;
                }
                case 2: {
                    return 151;
                }
                case 3: {
                    return 152;
                }
                case 4: {
                    return 153;
                }
                default: {
                    goto LABEL_16;
                }
            }
            break;
        }
        case 3: {
            LABEL_16:
            switch (sprite % 5) {
                case 0: {
                    return 130;
                }
                case 1: {
                    return 131;
                }
                case 2: {
                    return 132;
                }
                case 3: {
                    return 133;
                }
                case 4: {
                    return 134;
                }
                default: {
                    goto LABEL_18;
                }
            }
            break;
        }
        case 4: {
            LABEL_18:
            switch (sprite % 5) {
                case 0: {
                    return 129;
                }
                case 1: {
                    return 130;
                }
                case 2: {
                    return 131;
                }
                case 3: {
                    return 132;
                }
                case 4: {
                    return 133;
                }
                default: {
                    goto LABEL_21;
                }
            }
            break;
        }
        case 5: {
            LABEL_21:
            switch (sprite % 5) {
                case 0: {
                    return 131;
                }
                case 1: {
                    return 132;
                }
                case 2: {
                    return 133;
                }
                case 3: {
                    return 134;
                }
                case 4: {
                    return 135;
                }
                default: {
                    goto LABEL_24;
                }
            }
            break;
        }
        case 6: {
            LABEL_24:
            switch (sprite % 5) {
                case 0: {
                    return 78;
                }
                case 1: {
                    return 79;
                }
                case 2: {
                    return 80;
                }
                case 3: {
                    return 81;
                }
                case 4: {
                    return 82;
                }
                default: {
                    goto LABEL_31;
                }
            }
            break;
        }
        case 7: {
            LABEL_31:
            switch (sprite % 5) {
                case 0: {
                    return 24;
                }
                case 1: {
                    return 25;
                }
                case 2: {
                    return 26;
                }
                case 3: {
                    return 27;
                }
                case 4: {
                    return 28;
                }
                default: {
                    goto LABEL_38;
                }
            }
            break;
        }
        case 8: {
            LABEL_38:
            switch (sprite % 5) {
                case 0: {
                    return 34;
                }
                case 1: {
                    return 35;
                }
                case 2: {
                    return 36;
                }
                case 3: {
                    return 37;
                }
                case 4: {
                    return 38;
                }
                default: {
                    goto LABEL_45;
                }
            }
            break;
        }
        case 9: {
            LABEL_45:
            switch (sprite % 5) {
                case 0: {
                    return 15;
                }
                case 1: {
                    return 16;
                }
                case 2: {
                    return 17;
                }
                case 3: {
                    return 18;
                }
                case 4: {
                    return 19;
                }
                default: {
                    goto LABEL_52;
                }
            }
            break;
        }
        case 10: {
            LABEL_52:
            switch (sprite % 5) {
                case 0: {
                    return 9;
                }
                case 1: {
                    return 10;
                }
                case 2: {
                    return 11;
                }
                case 3: {
                    return 12;
                }
                case 4: {
                    return 13;
                }
            }
            break;
        }
    }

    app->Error(117); // ERR_ENT_LOOTSET
    return 0;
}

void Entity::addToLootSet(int n, int n2, int n3) {
    if (this->lootSet != nullptr && n3 > 0) {
        for (int i = 0; i < 3; ++i) {
            if (this->lootSet[i] == 0) {
                this->lootSet[i] = (n << 12 | n2 << 6 | n3);
                return;
            }
        }
    }
}

bool Entity::hasEmptyLootSet() {
    return this->lootSet == nullptr || this->lootSet[0] == 0;
}
