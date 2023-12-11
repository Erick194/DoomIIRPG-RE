#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Player.h"
#include "Combat.h"
#include "Canvas.h"
#include "Render.h"
#include "Game.h"
#include "Text.h"
#include "Hud.h"
#include "Enums.h"
#include "TinyGL.h"
#include "JavaStream.h"
#include "Sound.h"

Player::Player() {
	std::memset(this, 0, sizeof(Player));
}

Player::~Player() {
}

bool Player::startup() {
	printf("Player::startup\n");
	this->isFamiliar = false;
	this->noclip = false;
	this->god = false;
	this->helpBitmask = 0;
	this->invHelpBitmask = 0;
	this->weaponHelpBitmask = 0;
	this->armorHelpBitmask = 0;
	this->currentLevelDeaths = 0;
	this->totalDeaths = 0;
	this->currentGrades = 0;
	this->bestGrades = 0;
	this->enableHelp = true;
	this->baseCe = new CombatEntity();
	this->ce = new CombatEntity();
	this->reset();

	return true;
}

bool Player::modifyCollision(Entity* entity) {
	Applet* app = CAppContainer::getInstance()->app;
	return nullptr != entity && entity->def->eType == Enums::ET_SPRITEWALL && (app->render->mapSpriteInfo[(entity->info & 0xFFFF) - 1] & 0xFF) == 131;
}

void Player::advanceTurn() {
	Applet* app = CAppContainer::getInstance()->app;

    this->moves++;
    this->totalMoves++;
    this->updateStatusEffects();
    bool b = false;

    if (this->buffs[3] > 0) {
        this->addHealth(this->buffs[18], false);
    }

    if (this->statusEffects[53] > 0) {
        this->addHealth(-this->statusEffects[35]);
        b = true;
    }

    if (this->statusEffects[13] > 0) {
        this->addHealth(-3);
        Text* text = app->hud->getMessageBuffer(0);
        app->localization->composeText(0, 82, text);
        text->append(' ');
        app->localization->resetTextArgs();
        app->localization->addTextArg(3);
        app->localization->composeText(0, 71, text);
        app->hud->finishMessageBuffer();
        b = true;
    }

    if (this->inCombat && this->totalMoves - this->lastCombatTurn >= 4) {
        this->inCombat = false;
    }

    if (this->statusEffects[15] > 0) {
        ++this->counters[3];
    }

    this->turnTime = app->time;
    if (b && app->canvas->state == Canvas::ST_AUTOMAP) {
        app->canvas->setState(Canvas::ST_PLAYING);
    }
}

void Player::levelInit() {
    Applet* app = CAppContainer::getInstance()->app;
    this->moves = 0;
    this->numNotebookIndexes = 0;
    this->questComplete = 0;
    this->questFailed = 0;
    this->turnTime = app->time;
    this->inCombat = false;
    if (this->ce->getStat(Enums::STAT_HEALTH) == 0) {
        this->ce->setStat(Enums::STAT_HEALTH, 1);
    }
}

void Player::fillMonsterStats() {
    Applet* app = CAppContainer::getInstance()->app;

    int n = 0;
    int n2 = 0;
    for (int i = 0; i < app->game->numEntities; ++i) {
        Entity* entity = &app->game->entities[i];
        if (entity->monster != nullptr) {
            if ((entity->monster->flags & 0x80) == 0x0) {
                ++n;
                if ((app->game->entities[i].info & 0x1010000) != 0x0) {
                    ++n2;
                }
            }
        }
    }

    this->monsterStats[0] = n2;
    this->monsterStats[1] = n;
}

void Player::readyWeapon() {
    CAppContainer::getInstance()->app->canvas->readyWeaponSound = 2;
}

void Player::selectWeapon(int i) {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->isFamiliar) {
        return;
    }
    if (i != 14) {
        this->weapons &= 0xFFFFBFFF;
        this->ammo[8] = 0;
    }
    if (app->canvas->isZoomedIn) {
        app->canvas->zoomOut();
    }
    if ((this->weapons & ~this->disabledWeapons & 1 << i) == 0x0) {
        this->ce->weapon = i;
        this->selectNextWeapon();
    }
    if (this->ce->weapon != i) {
        app->canvas->invalidateRect();
        this->prevWeapon = this->ce->weapon;
    }
    this->ce->weapon = i;
    if (app->canvas->state != Canvas::ST_DIALOG && app->canvas->state != Canvas::ST_CAMERA) {
        app->canvas->drawPlayingSoftKeys();
    }
    if (i != this->prevWeapon) {
        this->readyWeapon();
    }
    this->activeWeaponDef = app->entityDefManager->find(6, 1, i);
    app->canvas->updateFacingEntity = true;
    app->hud->repaintFlags |= 0x4;
}

void Player::selectPrevWeapon() {
    Applet* app = CAppContainer::getInstance()->app;

    int weapon = this->ce->weapon;
    int n = this->weapons & ~this->disabledWeapons;
    for (int i = weapon - 1; i >= 0; --i) {
        if ((n & 1 << i) != 0x0) {
            int n2 = i * 9;
            if (app->combat->weapons[n2 + 5] == 0 || this->ammo[app->combat->weapons[n2 + 4]] != 0 || i == 2) {
                this->selectWeapon(i);
                return;
            }
        }
    }
    if (this->ce->weapon == weapon && weapon != 14) {
        for (int j = 14; j > weapon; --j) {
            if ((n & 1 << j) != 0x0) {
                int n3 = j * 9;
                if (app->combat->weapons[n3 + 5] == 0 || this->ammo[app->combat->weapons[n3 + 4]] != 0 || j == 2) {
                    this->selectWeapon(j);
                    return;
                }
            }
        }
    }
}

void Player::selectNextWeapon() {
    Applet* app = CAppContainer::getInstance()->app;

    int weapon = this->ce->weapon;
    int n = this->weapons & ~this->disabledWeapons;
    for (int i = weapon + 1; i < 15; ++i) {
        if ((n & 1 << i) != 0x0) {
            int n2 = i * 9;
            if (app->combat->weapons[n2 + 5] == 0 || this->ammo[app->combat->weapons[n2 + 4]] != 0 || i == 2) {
                this->selectWeapon(i);
                return;
            }
        }
    }
    if (this->ce->weapon == weapon && weapon != 0) {
        for (int j = 0; j < weapon; ++j) {
            if ((n & 1 << j) != 0x0) {
                int n3 = j * 9;
                if (app->combat->weapons[n3 + 5] == 0 || this->ammo[app->combat->weapons[n3 + 4]] != 0 || j == 2) {
                    this->selectWeapon(j);
                    return;
                }
            }
        }
    }
}

int Player::getHealth() {
    return this->isFamiliar ? this->ammo[7] : this->ce->getStat(0);
}

int Player::modifyStat(int n, int n2) {
    int n3;
    int n4;
    if (n == 0) {
        n3 = this->getHealth();
        this->addHealth(n2);
        if (n2 < 0) {
            this->painEvent(nullptr, false);
        }
        n4 = this->getHealth();
    }
    else {
        n3 = this->baseCe->getStat(n);
        int n5 = (n == 7) ? 200 : 99;
        if (n3 + n2 > n5) {
            n2 = n5 - n3;
        }
        if (n2 != 0) {
            this->baseCe->setStat(n, n3 + n2);
        }
        this->updateStats();
        n4 = this->baseCe->getStat(n);
    }
    return n4 - n3;
}

bool Player::requireStat(int n,int n2) {
    return this->ce->getStat(n) >= n2;
}

bool Player::requireItem(int n, int n2, int n3, int n4) {
    int n5 = 1 << n2;
    if (n != 1) {
        return n == 0 && this->inventory[n2 - 0] >= n3 && this->inventory[n2 - 0] <= n4;
    }
    if (n4 != 0) {
        return (this->weapons & n5) != 0x0;
    }
    return (this->weapons & n5) == 0x0;
}

void Player::addXP(int xp) {
    Applet* app = CAppContainer::getInstance()->app;

    app->localization->resetTextArgs();
    app->localization->addTextArg(xp);
    if (xp < 0) {
        app->hud->addMessage((short)102);
    }
    else {
        app->hud->addMessage((short)103);
    }
    this->currentXP += xp;
    this->xpGained += xp;
    while (this->currentXP >= this->nextLevelXP) {
        this->addLevel();
    }
    this->counters[5] += xp;
}

void Player::addLevel() {
    Applet* app = CAppContainer::getInstance()->app;
    Text* textBuff;
    int stat;

    this->level++;
    this->nextLevelXP = this->calcLevelXP(this->level);

    textBuff = app->localization->getLargeBuffer();
    textBuff->setLength(0);
    app->localization->resetTextArgs();
    app->localization->addTextArg(this->level);
    app->localization->composeText((short)0, (short)104, textBuff);

    int n = 10;
    stat = this->baseCe->getStat(1);
    if (stat + n > 999) {
        n = 999 - stat;
    }
    if (n != 0) {
        this->baseCe->setStat(Enums::STAT_MAX_HEALTH, stat + n);
        app->localization->resetTextArgs();
        app->localization->addTextArg(n);
        app->localization->composeText((short)0, (short)105, textBuff);
    }

    this->baseCe->getStat(Enums::STAT_DEFENSE);
    stat = this->modifyStat(Enums::STAT_DEFENSE, 1);
    if (stat != 0) {
        app->localization->resetTextArgs();
        app->localization->addTextArg(stat);
        app->localization->composeText((short)0, (short)106, textBuff);
    }

    this->baseCe->getStat(Enums::STAT_STRENGTH);
    stat = this->modifyStat(Enums::STAT_STRENGTH, 2);
    if (stat != 0) {
        app->localization->resetTextArgs();
        app->localization->addTextArg(stat);
        app->localization->composeText((short)0, (short)107, textBuff);
    }

    this->baseCe->getStat(Enums::STAT_ACCURACY);
    stat = this->modifyStat(Enums::STAT_ACCURACY, 1);
    if (stat != 0) {
        app->localization->resetTextArgs();
        app->localization->addTextArg(stat);
        app->localization->composeText((short)0, (short)108, textBuff);
    }

    this->baseCe->getStat(Enums::STAT_AGILITY);
    stat = this->modifyStat(Enums::STAT_AGILITY, 3);
    if (stat != 0) {
        app->localization->resetTextArgs();
        app->localization->addTextArg(stat);
        app->localization->composeText((short)0, (short)109, textBuff);
    }
    app->localization->composeText((short)0, (short)110, textBuff);

    this->ce->setStat(Enums::STAT_HEALTH, this->ce->getStat(Enums::STAT_MAX_HEALTH));
    /*if ((this->weapons & 0x78) != 0x0) {
        this->ammo[7] = 100;
    }*/
    app->hud->repaintFlags |= 0x4;

    bool dispose = true;
    if (app->canvas->state != Canvas::ST_MENU) {
        app->sound->soundStop();
        app->sound->playSound(1070, 0, 5, false);
        dispose = (app->canvas->enqueueHelpDialog(textBuff, 0) ? false : true);
    }

    if (dispose) {
        textBuff->dispose();
    }
}

int Player::calcLevelXP(int n) {
    return 500 * n + 100 * ((n - 1) * (n - 1) * (n - 1) + (n - 1));
}

int Player::calcScore() {
    Applet* app = CAppContainer::getInstance()->app;
    int n = 0;
    bool b = true;
    int i;
    for (i = 0; i <= 8; ++i) {
        if ((this->killedMonstersLevels & 1 << i) != 0x0) {
            n += 1000;
        }
        else {
            b = false;
        }
    }
    if (b) {
        n += 1000;
    }
    if (this->totalDeaths == 0) {
        n += 1000;
    }
    else if (this->totalDeaths < 10) {
        n += (5 - this->totalDeaths) * 50;
    }
    else {
        n -= 250;
    }
    int n2 = (this->totalTime + (app->gameTime - this->playTime)) / 60000;
    if (n2 < 120) {
        n += (120 - n2) * 15;
    }
    if (this->totalMoves < 5000) {
        n += (5000 - this->totalMoves) / 2;
    }
    bool b2 = true;
    while (i <= 8) {
        if ((this->foundSecretsLevels & 1 << i) != 0x0) {
            n += 1000;
        }
        else {
            b2 = false;
        }
        ++i;
    }
    if (b2) {
        n += 1000;
    }
    return n;
}

bool Player::addHealth(int i) {
    return this->addHealth(i, true);
}

bool Player::addHealth(int i, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    app->hud->repaintFlags |= 0x4;
    int stat;
    int stat2;
    if (this->isFamiliar) {
        stat = this->ammo[7];
        stat2 = 100;
    }
    else {
        stat = this->ce->getStat(Enums::STAT_HEALTH);
        stat2 = this->ce->getStat(Enums::STAT_MAX_HEALTH);
    }
    if (i > 0) {
        if (stat == stat2) {
            return false;
        }
    }
    else if (this->god) {
        return false;
    }
    if (this->isFamiliar) {
        this->ammo[7] = (short)std::max(0, this->ammo[7] + i);
    }
    else {
        app->hud->playerStartHealth = stat;
        this->ce->addStat(Enums::STAT_HEALTH, i);
    }
    int n2 = this->isFamiliar ? this->ammo[7] : this->ce->getStat(Enums::STAT_HEALTH);
    if (b && n2 > stat) {
        app->localization->resetTextArgs();
        app->localization->addTextArg(n2 - stat);
        app->hud->addMessage((short)111);
    }
    return true;
}

void Player::setStatsAccordingToCharacterChoice() {
    int n = 0;
    switch (this->characterChoice) {
    case 1: {
        this->baseCe->setStat(Enums::STAT_DEFENSE, 8);
        this->baseCe->setStat(Enums::STAT_STRENGTH, 9);
        this->baseCe->setStat(Enums::STAT_ACCURACY, 97);
        this->baseCe->setStat(Enums::STAT_AGILITY, 12);
        this->baseCe->setStat(Enums::STAT_IQ, 110);
        n = 30;
        break;
    }
    case 2: {
        this->baseCe->setStat(Enums::STAT_DEFENSE, 12);
        this->baseCe->setStat(Enums::STAT_STRENGTH, 14);
        this->baseCe->setStat(Enums::STAT_ACCURACY, 92);
        this->baseCe->setStat(Enums::STAT_AGILITY, 6);
        this->baseCe->setStat(Enums::STAT_IQ, 100);
        n = 10;
        break;
    }
    case 3: {
        this->baseCe->setStat(Enums::STAT_DEFENSE, 8);
        this->baseCe->setStat(Enums::STAT_STRENGTH, 8);
        this->baseCe->setStat(Enums::STAT_ACCURACY, 87);
        this->baseCe->setStat(Enums::STAT_AGILITY, 6);
        this->baseCe->setStat(Enums::STAT_IQ, 150);
        n = 80;
        break;
    }
    }
    this->give(0, 24, n, true);
}

void Player::reset() {
    Applet* app = CAppContainer::getInstance()->app;

    app->hud->msgCount = 0;
    this->numNotebookIndexes = 0;
    this->resetCounters();
    this->level = 1;
    this->currentXP = 0;
    this->nextLevelXP = this->calcLevelXP(this->level);
    this->facingEntity = nullptr;
    this->noclip = false;
    this->questComplete = 0;
    this->questFailed = 0;
    this->isFamiliar = false;
    this->setFamiliarType((short)0);
    this->attemptingToSelfDestructFamiliar = false;
    this->inTargetPractice = false;
    this->targetPracticeScore = 0;
    this->hackedVendingMachines = 0;
    this->botReturnedDueToMonster = false;
    this->unsetFamiliarOnceOutOfCinematic = false;
    this->vendingMachineHackTriesLeft1 = 0;
    this->vendingMachineHackTriesLeft2 = 0;
    for (int i = 0; i < 9; ++i) {
        this->vendingMachineHackTriesLeft1 += 4;
        this->vendingMachineHackTriesLeft2 += 4;
        if (i < 8) {
            this->vendingMachineHackTriesLeft1 <<= 3;
            this->vendingMachineHackTriesLeft2 <<= 3;
        }
    }
    this->chainsawStrengthBonusCount = 0;
    this->lastSkipCode = 0;
    this->playerEntityCopyIndex = -1;
    app->canvas->viewX = (app->canvas->destX = (app->canvas->saveX = (app->canvas->prevX = 0)));
    app->canvas->viewY = (app->canvas->destY = (app->canvas->saveY = (app->canvas->prevY = 0)));
    app->canvas->viewZ = (app->canvas->destZ = 36);
    app->canvas->viewAngle = (app->canvas->destAngle = (app->canvas->saveAngle = 0));
    app->canvas->viewPitch = (app->canvas->destPitch = (app->canvas->savePitch = 0));
    this->inCombat = false;
    for (int j = 0; j < 9; ++j) {
        this->ammo[j] = 0;
    }
    for (int k = 0; k < 26; ++k) {
        this->inventory[k] = 0;
    }
    this->give(0, 18, 1, true);
    this->numbuffs = 0;
    for (int l = 0; l < 15; ++l) {
        this->buffs[15 + l] = (this->buffs[0 + l] = 0);
    }
    this->numStatusEffects = 0;
    for (int n = 0; n < 18; ++n) {
        this->statusEffects[0 + n] = 0;
        this->statusEffects[18 + n] = 0;
        this->statusEffects[36 + n] = 0;
    }
    this->numbuffsCopy = 0;
    for (int n6 = 0; n6 < 15; ++n6) {
        this->buffsCopy[0 + n6] = 0;
        this->buffsCopy[15 + n6] = 0;
    }
    this->numStatusEffectsCopy = 0;
    for (int n7 = 0; n7 < 18; ++n7) {
        this->statusEffectsCopy[0 + n7] = 0;
        this->statusEffectsCopy[18 + n7] = 0;
        this->statusEffectsCopy[36 + n7] = 0;
    }
    this->weapons = 0;
    this->foundSecretsLevels = 0;
    this->killedMonstersLevels = 0;
    this->baseCe->setStat(Enums::STAT_MAX_HEALTH, 100);
    this->setStatsAccordingToCharacterChoice();
    if (app->game->difficulty == 2) {
        this->baseCe->setStat(Enums::STAT_DEFENSE, 0);
    }
    this->updateStats();
    this->ce->setStat(Enums::STAT_HEALTH, 100);
    this->baseCe->setStat(Enums::STAT_ARMOR, 0);
    this->ce->setStat(Enums::STAT_ARMOR, 0);
    this->totalTime = 0;
    this->totalMoves = 0;
    this->completedLevels = 0;
    this->highestMap = 1;
    this->gameCompleted = false;
    this->gamePlayedMask = 0;
}

int Player::calcDamageDir(int x1, int y1, int angle, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    angle &= Enums::ANGLE_FULL;
    if (dx == 0 && dy == 0) {
        return Enums::DIR_NORTHWEST;
    }
    int ang;
    if (dx > 0) {
        if (dy < 0) {
            ang = Enums::ANGLE_NORTHEAST;
        }
        else if (dy > 0) {
            ang = Enums::ANGLE_SOUTHEAST;
        }
        else {
            ang = Enums::ANGLE_EAST;
        }
    }
    else if (dx < 0) {
        if (dy < 0) {
            ang = Enums::ANGLE_NORTHWEST;
        }
        else if (dy > 0) {
            ang = Enums::ANGLE_SOUTHWEST;
        }
        else {
            ang = Enums::ANGLE_WEST;
        }
    }
    else if (dy > 0) {
        ang = Enums::ANGLE_SOUTH;
    }
    else {
        ang = Enums::ANGLE_NORTH;
    }

    int dAng = (ang - angle) & Enums::ANGLE_FULL;
    if (dAng > Enums::ANGLE_PI) {
        dAng = -(Enums::ANGLE_2PI - dAng);
    }
    int dir = (dAng / Enums::ANGLE_PI_FOURTH) + Enums::DIR_NORTHWEST;
    if (dir < 0) {
        dir = Enums::DIR_SOUTHEAST;
    }
    return dir;
}

void Player::painEvent(Entity* entity, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    if (entity == nullptr) {
        app->hud->damageDir = 3;
    }

    if (this->isFamiliar) {
        app->sound->playSound(1111, 0, 3, false);
    }
    else {
        if (this->characterChoice == 1) {
            app->sound->playSound(1094, 0, 3, false);
        }
        else {
            app->sound->playSound(1093, 0, 3, false);
        }
    }

    app->hud->damageCount = 1;
    if (!b) {
        app->canvas->startShake(500, 2, 150);
    }
    else { // [GEC] Damage From Monster
        app->canvas->startShake(0, 0, 150);
    }
}

void Player::pain(int n, Entity* entity, bool b) {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->god) {
        return;
    }
    if (b) {
        app->localization->resetTextArgs();
        if (entity != nullptr) {
            Text* smallBuffer = app->localization->getSmallBuffer();
            app->localization->composeText((short)1, (short)(entity->def->name & 0x3FF), smallBuffer);
            app->localization->addTextArg(smallBuffer);
            app->localization->addTextArg(n);
            app->hud->addMessage((short)0, (short)112);
            smallBuffer->dispose();
        }
        else {
            app->localization->addTextArg(n);
            app->hud->addMessage((short)0, (short)71);
        }
    }
    if (n == 0) {
        return;
    }
    if (this->isFamiliar) {
        this->addHealth(-n);
    }
    else {
        int stat = this->ce->getStat(Enums::STAT_HEALTH);
        if (n >= stat && this->noDeathFlag) {
            n = stat - 1;
        }
        int n2 = (stat << 16) / (this->ce->getStat(Enums::STAT_MAX_HEALTH) << 8);
        int n3 = (stat - n << 16) / (this->ce->getStat(Enums::STAT_MAX_HEALTH) << 8);
        if (n3 > 0) {
            if (n2 > 26 && n3 <= 26) {
                app->hud->addMessage((short)113, 3);
            }
            else if (n2 > 78 && n3 <= 78) {
                app->hud->addMessage((short)114, 3);
            }
            else if (n2 > 128 && n3 <= 128 && (this->helpBitmask & 0xC00) == 0x0) {
                if (this->inventory[17] != 0 || this->inventory[16] != 0) {
                    this->showHelp((short)10, true);
                }
                else {
                    this->showHelp((short)11, true);
                }
            }
        }
        this->addHealth(-n);
    }
    if (app->canvas->state == Canvas::ST_AUTOMAP) {
        app->canvas->setState(Canvas::ST_PLAYING);
    }
    if (this->ce->getStat(Enums::STAT_HEALTH) <= 0) {
        this->died();
    }
}

void Player::died() {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->canvas->state == Canvas::ST_DYING) {
        return;
    }
    this->currentLevelDeaths++;
    this->totalDeaths++;

    if (this->characterChoice == 1) {
        app->sound->playSound(1092, 0, 3, false);
    }
    else if(this->characterChoice == 2) {
        app->sound->playSound(1091, 0, 3, false);
    }else if (this->characterChoice == 3) {
        app->sound->playSound(1090, 0, 3, false);
    }

    app->canvas->startShake(350, 5, 500);
    this->ce->setStat(Enums::STAT_HEALTH, 0);
    app->canvas->setState(Canvas::ST_DYING);
    app->game->combatMonsters = nullptr;
}

void Player::familiarDying(bool familiarSelfDestructed) {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->canvas->state == Canvas::ST_BOT_DYING) {
        return;
    }

    if (familiarSelfDestructed) {
        app->combat->curAttacker = nullptr;
    }
    else {
        app->sound->playSound(1109, 0, 3, false);
        app->canvas->startShake(350, 5, 500);
    }

    app->canvas->familiarSelfDestructed = familiarSelfDestructed;
    app->canvas->setState(Canvas::ST_BOT_DYING);
}

bool Player::fireWeapon(Entity* entity, int n, int n2) {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->ce->weapon == Enums::WP_SOUL_CUBE && entity->monster == nullptr) {
        return false;
    }

    if (app->combat->weaponDown || (this->disabledWeapons != 0 && (this->weapons & 1 << this->ce->weapon) == 0x0)) {
        return false;
    }

    if (app->combat->lerpingWeapon) {
        if (app->combat->lerpWpDown) {
            return false;
        }
        app->combat->lerpingWeapon = false;
        app->combat->weaponDown = false;
    }

    if (entity->monster != nullptr) {
        entity->monster->flags &= 0xfff7;
    }

    if (this->ce->weapon == Enums::WP_CHAINSAW && (entity->def->eType == Enums::ET_CORPSE || (entity->def->eType == Enums::ET_ATTACK_INTERACTIVE && (entity->def->eSubType == Enums::INTERACT_BARRICADE || entity->def->eSubType == Enums::INTERACT_FURNITURE)))) {
        this->usedChainsaw(false);
    }

    int weapon = this->ce->weapon * Combat::WEAPON_MAX_FIELDS;
    if (app->combat->weapons[weapon + Combat::WEAPON_FIELD_AMMOTYPE] != 0) {
        short n4 = this->ammo[app->combat->weapons[weapon + Combat::WEAPON_FIELD_AMMOTYPE]];
        if (app->combat->weapons[weapon + Combat::WEAPON_FIELD_AMMOUSAGE] > 0 && (n4 - app->combat->weapons[weapon + Combat::WEAPON_FIELD_AMMOUSAGE]) < 0) {
            if (this->ce->weapon == Enums::WP_SOUL_CUBE) {
                app->hud->addMessage((short)117, 3);
            }
            else if (n4 == 0) {
                app->hud->addMessage((short)115, 3);
            }
            else {
                app->hud->addMessage((short)116, 3);
            }
            return false;
        }
    }
    app->combat->performAttack(nullptr, entity, n, n2, false);
    return true;
}

bool Player::useItem(int n) {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->inventory[n] == 0 && n != 22) {
        return false;
    }
    bool b = true;
    bool b2 = true;
    switch (n) {
    case 16: {
        if (!this->addHealth(20)) {
            return false;
        }
        break;
    }
    case 17: {
        if (!this->addHealth(80)) {
            return false;
        }
        break;
    }
    case 11: {
        if (!this->addArmor(50)) {
            return false;
        }
        break;
    }
    case 12: {
        if (!this->addArmor(10)) {
            return false;
        }
        break;
    }
    case 7: {
        if (!this->addStatusEffect(6, 25, 31)) {
            return false;
        }
        this->translateStatusEffects();
        break;
    }
    case 6: {
        if (!this->addStatusEffect(4, 25, 31)) {
            return false;
        }
        this->translateStatusEffects();
        break;
    }
    case 1: {
        if (!this->addStatusEffect(7, 5, 31)) {
            return false;
        }
        this->translateStatusEffects();
        break;
    }
    case 3: {
        bool b3 = false;
        if (this->addStatusEffect(8, 25, 31)) {
            b3 = true;
        }
        if (this->addStatusEffect(5, 20, 31)) {
            b3 = true;
        }
        if (b3) {
            this->translateStatusEffects();
            break;
        }
        return false;
    }
    case 2: {
        if (!this->addStatusEffect(9, 0, 11)) {
            return false;
        }
        this->removeStatusEffect(13);
        this->translateStatusEffects();
        break;
    }
    case 9: {
        if (!this->addStatusEffect(8, 100, 31)) {
            return false;
        }
        this->translateStatusEffects();
        break;
    }
    case 8: {
        if (this->statusEffects[2] != 0) {
            return false;
        }
        b = false;
        this->addStatusEffect(2, 0, 21);
        this->translateStatusEffects();
        break;
    }
    case 0: {
        if (!this->addStatusEffect(10, 20, 31)) {
            return false;
        }
        this->translateStatusEffects();
        this->addHealth(20);
        break;
    }
    case 5: {
        if (!this->addStatusEffect(3, 5, 31)) {
            return false;
        }
        this->translateStatusEffects();
        break;
    }
    case 10: {
        bool b4 = false;
        if (this->addStatusEffect(6, 25, 31)) {
            b4 = true;
        }
        if (this->addStatusEffect(8, 25, 31)) {
            b4 = true;
        }
        if (this->addStatusEffect(4, 25, 31)) {
            b4 = true;
        }
        if (this->addStatusEffect(7, 5, 31)) {
            b4 = true;
        }
        if (b4) {
            this->translateStatusEffects();
            break;
        }
        return false;
    }
    case 4: {
        if (!this->addStatusEffect(12, 10, 11)) {
            return false;
        }
        this->translateStatusEffects();
        break;
    }
    case 22: {
        bool b5 = this->addHealth(25) | this->addStatusEffect(11, 100, 3);
        b2 = false;
        if (b5) {
            this->ammo[3] -= 25;
            this->translateStatusEffects();
            break;
        }
        return false;
    }
    case 23: {
        return true;
    }
    default: {
        return false;
    }
    }
    app->sound->playSound(1124, 0, 3, false);
    if (b2) {
        --this->inventory[n];
    }
    if (b) {
        app->game->advanceTurn();
    }
    return true;
}

bool Player::give(int n, int n2, int n3) {
    return this->give(n, n2, n3, false);
}

bool Player::give(int n, int n2, int n3, bool b) {
    return this->give(n, n2, n3, b, false);
}

bool Player::give(int n, int n2, int n3, bool b, bool b2) {
    Applet* app = CAppContainer::getInstance()->app;
    if (n3 == 0) {
        return false;
    }
    int n4 = 1 << (n2 & 0xffU);
    switch (n) {
    case 1: {
        if (this->weaponIsASentryBot((n2 & 0xffU)) && n3 > 0) {
            if (this->isFamiliar) {
                return false;
            }
            this->give(2, 7, 100, true);
            this->weapons &= ~(8|16|32|64);
        }
        bool b3 = (this->weapons & n4) == 0x0;
        if (n3 < 0) {
            if (!this->isFamiliar || b2) {
                this->weapons &= ~n4;
                if (n2 == this->ce->weapon) {
                    this->selectNextWeapon();
                }
            }
            return true;
        }
        if (this->isFamiliar && !b2) {
            this->weaponsCopy |= n4;
        }
        else {
            this->weapons |= n4;
        }
        if (!b) {
            this->showWeaponHelp(n2, false);
        }
        if (b3 && !this->isFamiliar) {
            this->selectWeapon(n2);
        }
        break;
    }
    case 0: {
        short* array = (this->isFamiliar && !b2) ? this->inventoryCopy : this->inventory;
        int n5 = n3 + array[n2 - 0];
        if (n2 == 24) {
            if (n5 > 9999) {
                n5 = 9999;
            }
        }
        else if (n5 > 999) {
            n5 = 999;
        }
        if (n5 < 0) {
            return false;
        }
        if (n2 == 13 && (!this->isFamiliar || b2)) {
            this->give(2, 3, n5 * 20, true, true);
        }
        else {
            array[n2 - 0] = (short)n5;
        }
        if (!b) {
            this->showInvHelp(n2 - 0, false);
            break;
        }
        break;
    }
    case 2: {
        short* array2 = (this->isFamiliar && !b2 && n2 != 6) ? this->ammoCopy : this->ammo;
        int n6 = n3 + array2[n2];
        if (n6 > 100) {
            n6 = 100;
        }
        if (n2 == 6 && n6 > 5) {
            n6 = 5;
        }
        if (n6 < 0) {
            return false;
        }
        array2[n2] = (short)n6;
        if (!b) {
            this->showAmmoHelp(n2, false);
        }
        app->hud->repaintFlags |= 0x4;
        break;
    }
    case 3: {
        this->addHealth(n3);
        break;
    }
    default: {
        return false;
    }
    }
    return true;
}

void Player::giveAmmoWeapon(int n, bool b) {
    this->weapons |= 1 << (n & 0xffU);
    this->selectWeapon(n);
    if (!b) {
        this->showWeaponHelp(n, false);
    }
}

void Player::updateQuests(short n, int n2) {
    Applet* app = CAppContainer::getInstance()->app;

    if (n2 == 0) {
        if (this->numNotebookIndexes == 8) {
            //app->Error(Enums::ERR_MAX_NOTEBOOKINDEXES); // J2ME
            return;
        }
        this->questComplete &= (uint8_t)~(1 << this->numNotebookIndexes);
        this->questFailed &= (uint8_t)~(1 << this->numNotebookIndexes);
        this->notebookIndexes[this->numNotebookIndexes++] = n;
    }
    else {
        for (int i = 0; i < this->numNotebookIndexes; ++i) {
            if (n == this->notebookIndexes[i]) {
                if (n2 == 1) {
                    this->questComplete |= (uint8_t)(1 << i);
                }
                else if (n2 == 2) {
                    this->questFailed |= (uint8_t)(1 << i);
                }
                return;
            }
        }
        if (n2 == 1) {
            this->questComplete |= (uint8_t)(1 << this->numNotebookIndexes);
            this->questFailed &= (uint8_t)~(1 << this->numNotebookIndexes);
        }
        else if (n2 == 2) {
            this->questComplete &= (uint8_t)~(1 << this->numNotebookIndexes);
            this->questFailed |= (uint8_t)(1 << this->numNotebookIndexes);
        }
        this->notebookPositions[this->numNotebookIndexes] = 0;
        this->notebookIndexes[this->numNotebookIndexes++] = n;
    }
}

void Player::setQuestTile(int n, int n2, int n3) {
    for (int i = 0; i < this->numNotebookIndexes; ++i) {
        if (n == this->notebookIndexes[i]) {
            this->notebookPositions[i] = (short)(n2 << 5 | n3);
            return;
        }
    }
}

bool Player::isQuestDone(int n) {
    return (this->questComplete & 1 << n) != 0x0;
}

bool Player::isQuestFailed(int n) {
    return (this->questFailed & 1 << n) != 0x0;
}

void Player::formatTime(int n, Text* text) {
    text->setLength(0);
    int n2 = n / 1000;
    int n3 = n2 / 60;
    int n4 = n3 / 60;
    int n5 = n3 % 60;
    text->append(n4);
    text->append(":");
    if (n5 < 10) {
        text->append("0");
    }
    text->append(n5);
    text->append(":");
    int n6 = n2 - n3 * 60;
    if (n6 < 10) {
        text->append("0");
    }
    text->append(n6);
}

void Player::showInvHelp(int n, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    if (!this->enableHelp && !b) {
        return;
    }
    int n2 = n - 0;
    if ((this->invHelpBitmask & 1 << n2) != 0x0) {
        return;
    }
    this->invHelpBitmask |= 1 << n2;
    app->canvas->enqueueHelpDialog(app->entityDefManager->find(6, 0, n));
}

void Player::showAmmoHelp(int n, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    if (!this->enableHelp && !b) {
        return;
    }
    int n2 = 1 << n;
    if ((this->ammoHelpBitmask & n2) != 0x0) {
        return;
    }
    this->ammoHelpBitmask |= n2;
    app->canvas->enqueueHelpDialog(app->entityDefManager->find(6, 2, n));
}

bool Player::showHelp(short n, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->game->isCameraActive()) {
        return false;
    }

    if (!this->enableHelp && !b) {
        return false;
    }
    if ((this->helpBitmask & 1 << n) != 0x0 && !b) {
        return false;
    }
    this->helpBitmask |= 1 << n;
    app->canvas->enqueueHelpDialog(n);
    if (n != 5 && (app->canvas->state == Canvas::ST_AUTOMAP || app->canvas->state == Canvas::ST_PLAYING)) {
        app->canvas->dequeueHelpDialog();
    }
    return true;
}

void Player::showWeaponHelp(int n, bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    if (!this->enableHelp && !b) {
        return;
    }
    if ((this->weaponHelpBitmask & 1 << n) != 0x0) {
        return;
    }
    this->weaponHelpBitmask |= 1 << n;
    app->canvas->enqueueHelpDialog(app->entityDefManager->find(6, 1, n));
}

void Player::drawBuffs(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->numbuffs == 0 || app->canvas->state == Canvas::ST_DIALOG) {
        return;
    }
    int n = app->canvas->viewRect[0] + app->canvas->viewRect[2];
    int n2 = n - 32;
    int n3 = app->canvas->viewRect[1] + 2;
    int n4 = 0;
    bool b = false;
    int numbuffs = this->numbuffs;
    if (numbuffs > 6) {
        numbuffs = 6;
        b = true;
    }
    int n5 = numbuffs * 31 + 6;
    for (int n6 = 0; n6 < 15 && n4 < 6; ++n6) {
        if (this->buffs[0 + n6] > 0 && (1 << n6 & Enums::BUFF_AMT_NOT_DRAWN) == 0x0) {
            if (this->buffs[15 + n6] > 99 || this->buffs[15 + n6] < -99) {
                n2 = n - 73;
                break;
            }
            if (this->buffs[15 + n6] > 9 || this->buffs[15 + n6] < -9) {
                n2 = n - 65;
            }
            else if (n2 == n - 32) {
                n2 = n - 57;
            }
        }
    }
    int n7 = n - n2 + 4;
    if (b) {
        n5 += 5;
    }
    graphics->setColor(0);
    graphics->fillRect(n2 - 5, n3 - 2, n7, n5);
    graphics->setColor(0xFFAAAAAA);
    graphics->drawRect(n2 - 5, n3 - 2, n7, n5);
    int n8 = n - 36;
    for (int n9 = 0; n9 < 15 && n4 < 6; ++n9) {
        if (this->buffs[0 + n9] != 0) {
            ++n4;
            this->drawStatusEffectIcon(graphics, n9, this->buffs[15 + n9], this->buffs[0 + n9], n8, n3);
            n3 += 31;
        }
    }
    if (b) {
        Text* smallBuffer = app->localization->getSmallBuffer();
        smallBuffer->setLength(0);
        smallBuffer->append('\x85');
        graphics->drawString(smallBuffer, n2 + n7 / 2 - 4, n3 - 4, 1);
        smallBuffer->dispose();
    }
}

void Player::setCharacterChoice(short i) {
    Applet* app = CAppContainer::getInstance()->app;
    this->characterChoice = i;
    app->game->scriptStateVars[14] = i;
}

bool Player::loadState(InputStream* IS) {
    Applet* app = CAppContainer::getInstance()->app;
    this->baseCe->loadState(IS, true);
    this->ce->loadState(IS, true);
    this->setCharacterChoice(IS->readShort());
    this->weapons = IS->readInt();
    this->weaponsCopy = IS->readInt();
    this->level = (IS->readByte() & 0xFF);
    this->currentXP = IS->readInt();
    this->nextLevelXP = this->calcLevelXP(this->level);
    this->totalTime = IS->readInt();
    this->totalMoves = IS->readInt();
    this->completedLevels = IS->readInt();
    this->killedMonstersLevels = IS->readInt();
    this->foundSecretsLevels = IS->readInt();
    this->disabledWeapons = IS->readInt();
    this->prevWeapon = IS->readByte();
    this->gamePlayedMask = IS->readInt();
    this->lastCombatTurn = IS->readInt();
    this->inCombat = IS->readBoolean();
    this->highestMap = IS->readShort();
    this->isFamiliar = IS->readBoolean();
    this->setFamiliarType(IS->readShort());
    this->playerEntityCopyIndex = IS->readInt();
    this->hackedVendingMachines = IS->readInt();
    this->vendingMachineHackTriesLeft1 = IS->readInt();
    this->vendingMachineHackTriesLeft2 = IS->readInt();
    this->chainsawStrengthBonusCount = IS->readInt();
    this->lastSkipCode = IS->readByte();

    for (int i = 0; i < 9; ++i) {
        this->ammo[i] = IS->readShort();
    }
    for (int j = 0; j < 9; ++j) {
        this->ammoCopy[j] = IS->readShort();
    }
    for (int k = 0; k < 26; ++k) {
        this->inventory[k] = IS->readShort();
    }
    for (int l = 0; l < 26; ++l) {
        this->inventoryCopy[l] = IS->readShort();
    }
    this->numStatusEffects = IS->readByte();
    if (this->numStatusEffects == 0) {
        for (int n3 = 0; n3 < 18; ++n3) {
            this->statusEffects[36 + n3] = 0;
            this->statusEffects[0 + n3] = 0;
            this->statusEffects[18 + n3] = 0;
        }
    }
    else {
        for (int n4 = 0; n4 < 18; ++n4) {
            this->statusEffects[36 + n4] = IS->readShort();
            this->statusEffects[0 + n4] = IS->readShort();
            this->statusEffects[18 + n4] = IS->readShort();
        }
    }
    this->numStatusEffectsCopy = (this->isFamiliar ? IS->readByte() : 0);
    if (this->numStatusEffectsCopy == 0) {
        for (int n5 = 0; n5 < 18; ++n5) {
            this->statusEffectsCopy[36 + n5] = 0;
            this->statusEffectsCopy[0 + n5] = 0;
            this->statusEffectsCopy[18 + n5] = 0;
        }
    }
    else {
        for (int n6 = 0; n6 < 18; ++n6) {
            this->statusEffectsCopy[36 + n6] = IS->readShort();
            this->statusEffectsCopy[0 + n6] = IS->readShort();
            this->statusEffectsCopy[18 + n6] = IS->readShort();
        }
    }
    for (int n7 = 0; n7 < 8; ++n7) {
        this->counters[n7] = IS->readInt();
    }
    this->gameCompleted = IS->readBoolean();
    this->translateStatusEffects();
    this->updateStats();
    return true;
}

bool Player::saveState(OutputStream* OS) {
    Applet* app = CAppContainer::getInstance()->app;
    this->baseCe->saveState(OS, true);
    this->ce->saveState(OS, true);
    OS->writeShort(this->characterChoice);
    OS->writeInt(this->weapons);
    OS->writeInt(this->weaponsCopy);
    OS->writeByte(this->level);
    OS->writeInt(this->currentXP);
    int gameTime = app->gameTime;
    this->totalTime += gameTime - this->playTime;
    this->playTime = gameTime;
    OS->writeInt(this->totalTime);
    OS->writeInt(this->totalMoves);
    OS->writeInt(this->completedLevels);
    OS->writeInt(this->killedMonstersLevels);
    OS->writeInt(this->foundSecretsLevels);
    OS->writeInt(this->disabledWeapons);
    OS->writeByte(this->prevWeapon);
    OS->writeInt(this->gamePlayedMask);
    OS->writeInt(this->lastCombatTurn);
    OS->writeBoolean(this->inCombat);
    OS->writeShort(this->highestMap);
    OS->writeBoolean(this->isFamiliar);
    OS->writeShort(this->familiarType);
    OS->writeInt(this->playerEntityCopyIndex);
    OS->writeInt(this->hackedVendingMachines);
    OS->writeInt(this->vendingMachineHackTriesLeft1);
    OS->writeInt(this->vendingMachineHackTriesLeft2);
    OS->writeInt(this->chainsawStrengthBonusCount);
    OS->writeByte(this->lastSkipCode);
    for (int i = 0; i < 9; ++i) {
        OS->writeShort(this->ammo[i]);
    }
    for (int j = 0; j < 9; ++j) {
        OS->writeShort(this->ammoCopy[j]);
    }
    for (int k = 0; k < 26; ++k) {
        OS->writeShort(this->inventory[k]);
    }
    for (int l = 0; l < 26; ++l) {
        OS->writeShort(this->inventoryCopy[l]);
    }
    OS->writeByte(this->numStatusEffects);
    if (this->numStatusEffects != 0) {
        for (int n3 = 0; n3 < 18; ++n3) {
            OS->writeShort(this->statusEffects[36 + n3]);
            OS->writeShort(this->statusEffects[0 + n3]);
            OS->writeShort(this->statusEffects[18 + n3]);
        }
    }
    if (this->isFamiliar) {
        OS->writeByte(this->numStatusEffectsCopy);
        if (this->numStatusEffectsCopy != 0) {
            for (int n4 = 0; n4 < 18; ++n4) {
                OS->writeShort(this->statusEffectsCopy[36 + n4]);
                OS->writeShort(this->statusEffectsCopy[0 + n4]);
                OS->writeShort(this->statusEffectsCopy[18 + n4]);
            }
        }
    }
    for (int n5 = 0; n5 < 8; ++n5) {
        OS->writeInt(this->counters[n5]);
    }
    OS->writeBoolean(this->gameCompleted);
    return true;
}

void Player::unpause(int n) {
    if (n <= 0) {
        return;
    }
}

void Player::relink() {
    this->unlink();
    this->link();
}

void Player::unlink() {
    Applet* app = CAppContainer::getInstance()->app;

    Entity* playerEnt = getPlayerEnt();
    if ((playerEnt->info & 0x100000) != 0x0) {
        app->game->unlinkEntity(playerEnt);
    }
}

void Player::link() {
    Applet* app = CAppContainer::getInstance()->app;

    Entity* playerEnt = getPlayerEnt();
    if (app->canvas->destX >= 0 && app->canvas->destX <= 2047 && app->canvas->destY >= 0 && app->canvas->destY <= 2047) {
        app->game->linkEntity(playerEnt, app->canvas->destX >> 6, app->canvas->destY >> 6);
    }
}

void Player::updateStats() {
    this->ce->setStat(Enums::STAT_MAX_HEALTH, this->baseCe->getStat(Enums::STAT_MAX_HEALTH) + this->buffs[25]);
    this->ce->setStat(Enums::STAT_HEALTH, this->ce->getStat(Enums::STAT_HEALTH));
    this->ce->setStat(Enums::STAT_STRENGTH, this->baseCe->getStat(Enums::STAT_STRENGTH) + this->buffs[20]);
    this->ce->setStat(Enums::STAT_ACCURACY, this->baseCe->getStat(Enums::STAT_ACCURACY) + this->buffs[22] - this->buffs[28]);
    this->ce->setStat(Enums::STAT_DEFENSE, this->baseCe->getStat(Enums::STAT_DEFENSE) + this->buffs[19]);
    this->ce->setStat(Enums::STAT_AGILITY, this->baseCe->getStat(Enums::STAT_AGILITY) + this->buffs[21]);
    this->ce->setStat(Enums::STAT_IQ, this->baseCe->getStat(Enums::STAT_IQ));
}

void Player::updateStatusEffects() {
    if (this->numStatusEffects == 0) {
        return;
    }
    for (int i = 0; i < 18; ++i) {
        if (this->statusEffects[0 + i] != 0) {
            if (this->statusEffects[0 + i] <= 5 && this->statusEffects[0 + i] == 1) {
                this->removeStatusEffect(i);
            }
            else if (this->statusEffects[0 + i] != 0) {
                --this->statusEffects[0 + i];
            }
        }
    }
    this->translateStatusEffects();
}

void Player::translateStatusEffects() {
    for (int i = 0; i < 15; i++) {
        this->buffs[0 + i] = (this->buffs[15 + i] = 0);
    }

    this->numbuffs = 0;
    for (int j = 0; j < 18; j++) {
        int n = this->statusEffects[0 + j];
        int n2 = this->statusEffects[18 + j];
        if (n != 0) {
            if (n > 0) {
                switch (j) {
                case 14: {
                    this->buffs[20] -= (short)n2;
                    this->buffs[5] = (short)n;
                    this->buffs[19] -= (short)n2;
                    this->buffs[4] = (short)n;
                    this->buffs[29] -= (short)(this->statusEffects[36 + j] * 4);
                    this->buffs[14] = (short)n;
                    break;
                }
                case 15:
                case 17: {
                    this->buffs[20] += (short)n2;
                    this->buffs[5] = (short)n;
                    this->buffs[19] += (short)n2;
                    this->buffs[4] = (short)n;
                    this->buffs[22] -= (short)(n2 + (n2 / 2));
                    this->buffs[7] = (short)n;
                    break;
                }
                case 16: {
                    this->buffs[22] -= (short)n2;
                    this->buffs[7] = (short)n;
                    break;
                }
                default: {
                    this->buffs[15 + j] += (short)n2;
                    this->buffs[0 + j] = (short)n;
                    break;
                }
                }
            }
        }
    }

    for (int k = 0; k < 15; k++) {
        if (this->buffs[0 + k] > 0) {
            if ((1 << k & Enums::BUFF_NO_AMOUNT) == 0x0 && this->buffs[15 + k] == 0) {
                this->buffs[0 + k] = 0;
            }
            else {
                this->numbuffs++;
            }
        }
    }
    this->updateStats();
}

void Player::removeStatusEffect(int n) {
    Applet* app = CAppContainer::getInstance()->app;

    if (n == 18) {
        this->numStatusEffects = 0;
        for (int i = 0; i < 18; i++) {
            this->statusEffects[0 + i] = 0;
            this->statusEffects[18 + i] = 0;
            this->statusEffects[36 + i] = 0;
        }
    }
    else {
        if (this->statusEffects[36 + n] == 0) {
            return;
        }

        if (n == 17) {
            app->render->startFogLerp(1, 0, 2000);
        }

        this->statusEffects[18 + n] = 0;
        this->statusEffects[36 + n] = 0;
        this->statusEffects[0 + n] = 0;
        this->numStatusEffects--;
    }
    this->translateStatusEffects();
}

bool Player::addStatusEffect(int n, int n2, int n3) {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->isFamiliar) {
        return false;
    }
    if (n == 13) {
        if (this->buffs[9] > 0) {
            return false;
        }
        app->sound->playSound(1034, 0, 3, false);
    }
    int n4 = this->statusEffects[36 + n] + 1;
    if (n4 > 3 || ((n == 12 || n == 9) && n4 > 1)) {
        if (n == 14) {
            this->statusEffects[0 + n] = n3;
        }
        return false;
    }
    if (n4 == 1) {
        this->numStatusEffects++;
        if (n == 17) {
            app->tinyGL->fogMin = 0;
            if (app->tinyGL->fogRange > 0) {
                app->tinyGL->fogRange = -1;
            }
            app->render->startFogLerp(1024, 0, 2000);
        }
    }
    this->statusEffects[18 + n] += n2;
    this->statusEffects[0 + n] = n3;
    this->statusEffects[36 + n] = n4;
    return true;
}

void Player::drawStatusEffectIcon(Graphics* graphics, int n, int n2, int n3, int n4, int n5) {
    Applet* app = CAppContainer::getInstance()->app;

    Text* smallBuffer = app->localization->getSmallBuffer();
    smallBuffer->setLength(0);
    if (n == 8) {
        smallBuffer->append('%');
        smallBuffer->append(n2);
    }
    else if ((1 << n & 0x3A07) == 0x0) {
        if (n2 >= 0) {
            smallBuffer->append('+');
            smallBuffer->append(n2);
        }
        else {
            smallBuffer->append(n2);
        }
    }
    graphics->drawString(smallBuffer, n4, n5 + 2, 24);
    graphics->drawBuffIcon(n, n4 + 3, n5 + 1, 0);
    if (app->time - this->turnTime < 600) {
        smallBuffer->setLength(0);
        smallBuffer->append(n3);
        graphics->drawString(smallBuffer, n4 + 17, n5 + 2, 17);
        app->canvas->forcePump = true;
    }
    smallBuffer->dispose();
}

void Player::resetCounters() {
    for (int i = 0; i < 8; ++i) {
        this->counters[i] = 0;
    }
}

Entity* Player::getPlayerEnt() {
    return &CAppContainer::getInstance()->app->game->entities[1];
}

void Player::setPickUpWeapon(int n) {
    Applet* app = CAppContainer::getInstance()->app;

    EntityDef* lookup = nullptr;
    EntityDef* find = app->entityDefManager->find(6, 1, 14);
    if (n != 15) {
        lookup = app->entityDefManager->lookup(n);
    }
    if (lookup != nullptr) {
        find->tileIndex = lookup->tileIndex;
        find->name = lookup->name;
        find->longName = lookup->longName;
        find->description = lookup->description;
    }
    else {
        find->tileIndex = 15;
        find->name = 159;
        find->longName = 159;
        find->description = 159;
    }
}

void Player::giveAll() {
    Applet* app = CAppContainer::getInstance()->app;

    if (!this->isFamiliar) {
        if (this->hasASentryBot()) {
            for (int i = 0; i <= 3; ++i) {
                int n = 3 + i;
                if ((this->weapons & 1 << n) != 0x0) {
                    this->weapons &= ~(1 << n);
                    this->weapons |= 1 << 3 + (i + 1) % 4;
                    break;
                }
            }
        }
        else {
            this->weapons |= 0x8;
        }
        this->weapons |= (0x3FFF & 0xFFFFFFF7 & 0xFFFFFFEF & 0xFFFFFFDF & 0xFFFFFFBF);
        this->selectPrevWeapon();
        this->selectNextWeapon();
    }
    for (int j = 0; j < 9; ++j) {
        if (j != 8) {
            this->give(2, j, 100, true);
        }
    }
    for (int k = 0; k < 26; ++k) {
        if (k != 24) {
            this->give(0, (uint8_t)k, 999, true);
            app->canvas->numHelpMessages = 0;
        }
    }
    this->give(0, 24, 9999, true);
    this->ce->setStat(Enums::STAT_HEALTH, this->ce->getStat(Enums::STAT_MAX_HEALTH));
}

void Player::equipForLevel(int highestMap) {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->isFamiliar) {
        this->familiarReturnsToPlayer(false);
    }
    int viewX = app->canvas->viewX;
    int viewY = app->canvas->viewY;
    int viewAngle = app->canvas->viewAngle;
    this->reset();
    app->canvas->viewX = (app->canvas->destX = (app->canvas->saveX = (app->canvas->prevX = viewX)));
    app->canvas->viewY = (app->canvas->destY = (app->canvas->saveY = (app->canvas->prevY = viewY)));
    app->canvas->viewZ = (app->canvas->destZ = app->render->getHeight(app->canvas->viewX, app->canvas->viewY) + 36);
    app->canvas->viewAngle = (app->canvas->destAngle = (app->canvas->saveAngle = viewAngle));
    app->canvas->viewPitch = (app->canvas->destPitch = 0);
    this->highestMap = highestMap;
    bool enableHelp = this->enableHelp;
    this->enableHelp = false;
    this->weapons = 0;
    app->game->numMallocsForVIOS = 0;
    switch (highestMap) {
    case 2: {
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->addArmor(33);
        this->give(0, 17, 23);
        this->give(0, 11, 4);
        this->give(0, 12, 3);
        this->give(2, 1, 100);
        this->give(0, 24, 4);
        this->addXP(638);
        this->modifyStat(3, 4);
        this->modifyStat(4, 2);
        this->modifyStat(5, 11);
        this->modifyStat(7, 4);
        break;
    }
    case 3: {
        this->give(1, 2, 1);
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->give(1, 3, 100);
        this->addArmor(4);
        this->give(0, 17, 34);
        this->give(0, 11, 12);
        this->give(0, 12, 6);
        this->give(2, 1, 100);
        this->give(2, 3, 90);
        this->give(0, 24, 39);
        this->addXP(1519);
        this->modifyStat(3, 13);
        this->modifyStat(4, 5);
        this->modifyStat(5, 10);
        this->modifyStat(6, 6);
        this->modifyStat(7, 10);
        app->game->numMallocsForVIOS = 1;
        break;
    }
    case 4: {
        this->give(1, 7, 1);
        this->give(1, 2, 1);
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->give(1, 4, 100);
        this->addArmor(4);
        this->give(0, 17, 44);
        this->give(0, 11, 5);
        this->give(0, 12, 26);
        this->give(2, 1, 73);
        this->give(2, 3, 30);
        this->give(2, 2, 24);
        this->give(0, 24, 59);
        this->addXP(2622);
        this->modifyStat(3, 14);
        this->modifyStat(4, 10);
        this->modifyStat(5, 9);
        this->modifyStat(6, 14);
        this->modifyStat(7, 20);
        app->game->numMallocsForVIOS = 1;
        break;
    }
    case 5: {
        this->give(1, 9, 1);
        this->give(1, 8, 1);
        this->give(1, 7, 1);
        this->give(1, 2, 1);
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->give(1, 5, 100);
        this->addArmor(34);
        this->give(0, 17, 79);
        this->give(0, 16, 31);
        this->give(0, 11, 9);
        this->give(0, 12, 40);
        this->give(2, 1, 84);
        this->give(2, 3, 85);
        this->give(2, 2, 88);
        this->give(2, 4, 55);
        this->give(0, 24, 73);
        this->addXP(3594);
        this->modifyStat(3, 14);
        this->modifyStat(4, 12);
        this->modifyStat(5, 9);
        this->modifyStat(6, 18);
        this->modifyStat(7, 28);
        app->game->numMallocsForVIOS = 2;
        break;
    }
    case 6: {
        this->give(1, 10, 1);
        this->give(1, 9, 1);
        this->give(1, 8, 1);
        this->give(1, 7, 1);
        this->give(1, 2, 1);
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->give(1, 5, 100);
        this->addArmor(24);
        this->give(0, 17, 116);
        this->give(0, 16, 54);
        this->give(0, 11, 22);
        this->give(0, 12, 2);
        this->give(2, 1, 88);
        this->give(2, 3, 60);
        this->give(2, 2, 100);
        this->give(2, 4, 100);
        this->give(2, 5, 20);
        this->give(0, 24, 101);
        this->addXP(4749);
        this->modifyStat(3, 24);
        this->modifyStat(4, 13);
        this->modifyStat(5, 9);
        this->modifyStat(6, 18);
        this->modifyStat(7, 34);
        app->game->numMallocsForVIOS = 3;
        break;
    }
    case 7: {
        this->give(1, 11, 1);
        this->give(1, 10, 1);
        this->give(1, 9, 1);
        this->give(1, 8, 1);
        this->give(1, 7, 1);
        this->give(1, 2, 1);
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->give(1, 5, 100);
        this->addArmor(44);
        this->give(0, 17, 132);
        this->give(0, 16, 72);
        this->give(0, 11, 23);
        this->give(0, 12, 12);
        this->give(2, 1, 70);
        this->give(2, 3, 40);
        this->give(2, 2, 66);
        this->give(2, 4, 60);
        this->give(2, 5, 63);
        this->give(0, 24, 168);
        this->addXP(5971);
        this->modifyStat(3, 25);
        this->modifyStat(4, 14);
        this->modifyStat(5, 8);
        this->modifyStat(6, 18);
        this->modifyStat(7, 40);
        app->game->numMallocsForVIOS = 3;
        break;
    }
    case 8: {
        this->give(1, 12, 1);
        this->give(1, 11, 1);
        this->give(1, 10, 1);
        this->give(1, 9, 1);
        this->give(1, 8, 1);
        this->give(1, 7, 1);
        this->give(1, 2, 1);
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->give(1, 5, 100);
        this->addArmor(38);
        this->give(0, 17, 146);
        this->give(0, 16, 94);
        this->give(0, 11, 27);
        this->give(0, 12, 20);
        this->give(2, 1, 89);
        this->give(2, 3, 25);
        this->give(2, 2, 94);
        this->give(2, 4, 52);
        this->give(2, 5, 100);
        this->give(0, 24, 168);
        this->addXP(7198);
        this->modifyStat(3, 27);
        this->modifyStat(4, 16);
        this->modifyStat(5, 8);
        this->modifyStat(6, 22);
        this->modifyStat(7, 46);
        app->game->numMallocsForVIOS = 4;
        break;
    }
    case 9:
    case 10: {
        this->give(1, 13, 1);
        this->give(1, 12, 1);
        this->give(1, 11, 1);
        this->give(1, 10, 1);
        this->give(1, 9, 1);
        this->give(1, 8, 1);
        this->give(1, 7, 1);
        this->give(1, 2, 1);
        this->give(1, 1, 1);
        this->give(1, 0, 1);
        this->give(1, 5, 100);
        this->addArmor(27);
        this->give(0, 17, 160);
        this->give(0, 16, 90);
        this->give(0, 11, 23);
        this->give(0, 12, 28);
        this->give(2, 1, 29);
        this->give(2, 3, 8);
        this->give(2, 2, 44);
        this->give(2, 4, 55);
        this->give(2, 5, 90);
        this->give(2, 6, 5);
        this->give(0, 24, 174);
        this->addXP(8281);
        this->modifyStat(3, 27);
        this->modifyStat(4, 17);
        this->modifyStat(5, 8);
        this->modifyStat(6, 22);
        this->modifyStat(7, 46);
        app->game->numMallocsForVIOS = 4;
        break;
    }
    }
    this->give(0, 18, 1);
    this->enableHelp = enableHelp;
    this->selectNextWeapon();
    app->canvas->updateFacingEntity = true;
}

bool Player::addArmor(int n) {
    Applet* app = CAppContainer::getInstance()->app;

    int stat = this->ce->getStat(2);
    if (stat >= 200 && n > 0) {
        return false;
    }
    if (n < 0 && this->god) {
        return false;
    }
    app->hud->repaintFlags |= 0x4;
    this->ce->setStat(Enums::STAT_ARMOR, std::max(0, std::min(200, stat + n)));
    return true;
}

int Player::distFrom(Entity* entity) {
    Applet* app = CAppContainer::getInstance()->app;
    return entity->distFrom(app->canvas->destX, app->canvas->destY);
}

void Player::showAchievementMessage(int n) {
    Applet* app = CAppContainer::getInstance()->app;

    Text* smallBuffer = app->localization->getSmallBuffer();
    Text* smallBuffer2 = app->localization->getSmallBuffer();
    app->localization->resetTextArgs();
    switch (n) {
    case 0: {
        app->localization->composeText(136, smallBuffer);
        break;
    }
    case 1: {
        app->localization->composeText(137, smallBuffer);
        break;
    }
    case 2: {
        app->localization->composeText(138, smallBuffer);
        break;
    }
    case 3: {
        app->localization->composeText(139, smallBuffer);
        break;
    }
    }
    app->localization->addTextArg(smallBuffer);
    smallBuffer->dispose();
    app->localization->composeText((short)0, (short)135, smallBuffer2);
    if (!app->canvas->enqueueHelpDialog(smallBuffer2, 3)) {
        smallBuffer2->dispose();
    }
    int n2 = 10;
    if (n == 2) {
        n2 = this->calcLevelXP(this->level) - calcLevelXP(this->level - 1) >> 4;
    }
    this->addXP(n2);
    app->sound->playSound(1020, 0, 3, false);
}

short Player::gradeToString(int n) {
    short n2 = 44;
    switch (n) {
        case 6: {
            n2 = 38;
            break;
        }
        case 5: {
            n2 = 39;
            break;
        }
        case 4: {
            n2 = 40;
            break;
        }
        case 3: {
            n2 = 41;
            break;
        }
        case 2: {
            n2 = 42;
            break;
        }
        case 1: {
            n2 = 43;
            break;
        }
    }
    return n2;
}

int Player::levelGrade(bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    int currentGrade = this->getCurrentGrade(app->canvas->loadMapID);
    if (currentGrade != 0) {
        return currentGrade;
    }
    this->fillMonsterStats();
    if (this->monsterStats[1] == 0 || app->game->totalSecrets == 0) {
        return 0;
    }
    int max = std::max(((6 * ((this->monsterStats[0] << 8) / this->monsterStats[1]) >> 8) + (6 * ((app->game->mapSecretsFound << 8) / app->game->totalSecrets) >> 8) >> 1) - this->currentLevelDeaths, 1);
    if (b) {
        if (max > this->getBestGrade(app->canvas->loadMapID)) {
            this->setBestGrade(app->canvas->loadMapID, max);
        }
        this->setCurrentGrade(app->canvas->loadMapID, max);
    }
    return max;
}

int Player::finalCurrentGrade() {
    int n = 0;
    for (int i = 1; i <= 9; ++i) {
        n += this->getCurrentGrade(i);
    }
    return n / 9;
}

int Player::finalBestGrade() {
    int n = 0;
    for (int i = 1; i <= 9; ++i) {
        n += this->getBestGrade(i);
    }
    return n / 9;
}

int Player::getCurrentGrade(int n) {
    return (this->currentGrades >> (3 * (n - 1))) & 0x7;
}

void Player::setCurrentGrade(int n, int n2) {
    this->currentGrades |= n2 << (3 * (n - 1));
}

int Player::getBestGrade(int n) {
    return (this->bestGrades >> (3 * (n - 1))) & 0x7;
}

void Player::setBestGrade(int n, int n2) {
    this->bestGrades &= ~(7 << (3 * (n - 1)));
    this->bestGrades |= n2 << (3 * (n - 1));
}

bool Player::hasPurifyEffect() {
    return this->statusEffects[1] != 0;
}

void Player::setFamiliar(short familiarType) {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->canvas->state == Canvas::ST_AUTOMAP) {
        return;
    }
    app->game->scriptStateVars[16] = 1;
    app->canvas->saveX = app->canvas->viewX;
    app->canvas->saveY = app->canvas->viewY;
    app->canvas->saveZ = app->canvas->viewZ;
    app->canvas->saveAngle = app->canvas->viewAngle;
    app->canvas->savePitch = app->canvas->viewPitch;
    this->isFamiliar = true;
    this->setFamiliarType(familiarType);
    app->game->updateScriptVars();
    if (!app->canvas->attemptMove(app->canvas->viewX + app->canvas->viewStepX, app->canvas->viewY + app->canvas->viewStepY)) {
        app->game->scriptStateVars[16] = 0;
        this->isFamiliar = false;
        this->setFamiliarType((short)0);
        app->game->updateScriptVars();
        app->hud->addMessage((short)0, (short)190, 3);
        return;
    }
    app->render->savePlayerFog();
    app->render->startFade(400, 2);
    this->playerEntityCopyIndex = app->game->spawnPlayerEntityCopy((short)app->canvas->saveX, (short)app->canvas->saveY)->getIndex();
    this->weaponsCopy = 0;
    for (int i = 0; i < 9; ++i) {
        this->ammoCopy[i] = 0;
    }
    for (int j = 0; j < 26; ++j) {
        this->inventoryCopy[j] = 0;
    }
    this->clearOutFamiliarsStatusEffects();
    this->swapStatusEffects();
    bool b;
    if (this->familiarType == 1 || this->familiarType == 3) {
        b = this->showHelp((short)12, false);
    }
    else {
        b = this->showHelp((short)13, false);
    }
    if (!b) {
        app->canvas->drawPlayingSoftKeys();
    }
    app->sound->playSound(1108, 0, 3, false);
}

short Player::unsetFamiliar(bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    app->hud->stopBrightenScreen();
    app->hud->stopScreenSmack();
    int saveAngle = app->canvas->saveAngle;
    if (b) {
        if (app->canvas->saveX > app->canvas->viewX) {
            saveAngle = Enums::ANGLE_WEST;
        }
        else if (app->canvas->saveX < app->canvas->viewX) {
            saveAngle = Enums::ANGLE_EAST;
        }
        else if (app->canvas->saveY > app->canvas->viewY) {
            saveAngle = Enums::ANGLE_NORTH;
        }
        else if (app->canvas->saveY < app->canvas->viewY) {
            saveAngle = Enums::ANGLE_SOUTH;
        }
    }
    app->canvas->destAngle = (app->canvas->viewAngle = saveAngle);
    app->canvas->destPitch = (app->canvas->viewPitch = app->canvas->savePitch);
    if (!b) {
        app->canvas->destX = app->canvas->saveX;
        app->canvas->destY = app->canvas->saveY;
        app->canvas->destZ = app->canvas->saveZ;
        app->canvas->viewX = app->canvas->destX;
        app->canvas->viewY = app->canvas->destY;
        app->canvas->viewZ = app->canvas->destZ;
    }
    this->swapStatusEffects();
    if (!b) {
        app->render->loadPlayerFog();
    }
    app->render->startFade(400, 2);
    app->game->removeEntity(&app->game->entities[this->playerEntityCopyIndex]);
    this->playerEntityCopyIndex = -1;
    this->unlink();
    app->canvas->finishRotation(true);
    this->relink();
    this->isFamiliar = false;
    short familiarType = this->familiarType;
    this->setFamiliarType((short)0);
    app->game->updateScriptVars();
    app->canvas->drawPlayingSoftKeys();
    return familiarType;
}

void Player::clearOutFamiliarsStatusEffects() {
    this->numbuffsCopy = 0;
    for (int i = 0; i < 15; ++i) {
        this->buffsCopy[15 + i] = (this->buffsCopy[0 + i] = 0);
    }
    this->numStatusEffectsCopy = 0;
    for (int j = 0; j < 18; ++j) {
        this->statusEffectsCopy[0 + j] = 0;
        this->statusEffectsCopy[36 + j] = 0;
        this->statusEffectsCopy[18 + j] = 0;
    }
}

void Player::swapStatusEffects() {
    int tempStatusEffects[54];
    short tempBuffs[30];

    int numbuffsCopy = this->numbuffsCopy;
    this->numbuffsCopy = this->numbuffs;
    this->numbuffs = numbuffsCopy;
    std::memcpy(tempBuffs, this->buffs, sizeof(tempBuffs));
    std::memcpy(this->buffs, this->buffsCopy, sizeof(tempBuffs));
    std::memcpy(this->buffsCopy, tempBuffs, sizeof(tempBuffs));

    int numStatusEffectsCopy = this->numStatusEffectsCopy;
    this->numStatusEffectsCopy = this->numStatusEffects;
    this->numStatusEffects = numStatusEffectsCopy;
    std::memcpy(tempStatusEffects, this->statusEffects, sizeof(tempStatusEffects));
    std::memcpy(this->statusEffects, this->statusEffectsCopy, sizeof(tempStatusEffects));
    std::memcpy(this->statusEffectsCopy, tempStatusEffects, sizeof(tempStatusEffects));
}

void Player::familiarDied() {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->combat->curAttacker != nullptr) {
        int sprite = app->combat->curAttacker->getSprite();
        if (app->combat->curAttacker->def->eType == 2) {
            app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x0);
        }
        app->localization->resetTextArgs();
        if (app->combat->accumRoundDamage > 0) {
            app->localization->addTextArg((short)1, (short)(app->combat->curAttacker->def->name & 0x3FF));
            app->localization->addTextArg(app->combat->accumRoundDamage);
            app->hud->addMessage((short)0, (short)112);
        }
        app->canvas->shakeTime = 0;
        app->hud->damageDir = 0;
        app->hud->damageTime = 0;
        app->combat->curAttacker->monster->flags |= 0x400;
        app->game->gsprite_clear(64);
        app->canvas->invalidateRect();
    }
    int viewX = app->canvas->viewX;
    int viewY = app->canvas->viewY;
    short unsetFamiliar = this->unsetFamiliar(false);
    app->hud->addMessage((short)0, (short)192, 2);
    if (unsetFamiliar == 2 || unsetFamiliar == 4) {
        this->explodeFamiliar(viewX >> 6, viewY >> 6, unsetFamiliar);
    }
    int n = 0;
    switch (unsetFamiliar) {
    case 2: {
        n = 4;
        break;
    }
    case 3: {
        n = 5;
        break;
    }
    case 4: {
        n = 6;
        break;
    }
    default: {
        n = 3;
        break;
    }
    }
    if (this->noFamiliarRemains) {
        this->noFamiliarRemains = false;
    }
    else {
        this->handleBotRemains(viewX, viewY, n);
    }
    this->give(1, n, -1, true);
}

void Player::explodeFamiliar(int n, int n2, int n3) {
    Applet* app = CAppContainer::getInstance()->app;

    app->combat->attackerWeaponId = (n3 == 2) ? 4 : 6;
    int n4 = app->combat->attackerWeaponId * 9;

    int n5 = app->combat->weapons[n4 + 0] & 0xFF;
    int n6 = app->combat->weapons[n4 + 1] & 0xFF;
    if (n5 != n6) {
        n5 += app->nextByte() % (n6 - n5);
    }
    app->combat->radiusHurtEntities(n, n2, 0, n5, this->getPlayerEnt(), nullptr);
}

void Player::familiarReturnsToPlayer(bool b) {
    Applet* app = CAppContainer::getInstance()->app;
    app->sound->playSound(1112, 0, 3, false);

    this->unsetFamiliar(b);
    this->tookBotsInventory = this->stealFamiliarsInventory();
}

bool Player::stealFamiliarsInventory() {
    int weaponsCopy = this->weaponsCopy;
    short* inventoryCopy = this->inventoryCopy;
    short* ammoCopy = this->ammoCopy;

    bool b = false;
    for (int i = 0; i < 26; ++i) {
        if (inventoryCopy[i] != 0) {
            this->give(0, i, inventoryCopy[i], true);
            b = true;
        }
    }
    for (int j = 0; j < 9; ++j) {
        if (ammoCopy[j] != 0) {
            this->give(2, j, ammoCopy[j], true);
            b = true;
        }
    }
    for (int k = 0; k < 15; ++k) {
        if ((1 << k & weaponsCopy) != 0x0) {
            this->give(1, k, 1, true);
            b = true;
        }
    }
    return b;
}

void Player::handleBotRemains(int n, int n2, int n3) {
    Applet* app = CAppContainer::getInstance()->app;
    int weaponsCopy = this->weaponsCopy;
    short* inventoryCopy = this->inventoryCopy;
    short* ammoCopy = this->ammoCopy;
    short n4 = 0;
    short n5 = 0;
    for (int i = 0; i < 26; ++i) {
        if (inventoryCopy[i] != 0) {
            if (i == 24 || (i >= 0 && i < 11)) {
                short n6 = inventoryCopy[i];
                this->give(0, i, n6, true, true);
                EntityDef* find = app->entityDefManager->find(6, 0, i);
                Text* messageBuffer = app->hud->getMessageBuffer();
                app->localization->resetTextArgs();
                app->localization->addTextArg(n6);
                Text* smallBuffer = app->localization->getSmallBuffer();
                app->localization->composeText((short)1, find->longName, smallBuffer);
                app->localization->addTextArg(smallBuffer);
                app->localization->composeText((short)0, (short)86, messageBuffer);
                smallBuffer->dispose();
                app->hud->finishMessageBuffer();
            }
            else {
                switch (i) {
                case 13: {
                    n5 += inventoryCopy[i];
                    break;
                }
                case 16: {
                    n4 += inventoryCopy[i];
                    break;
                }
                case 11:
                case 12:
                case 17:
                case 19:
                case 20: {
                    EntityDef* find = app->entityDefManager->find(6, 0, i);
                    app->game->spawnDropItem(n, n2, find->tileIndex, find, inventoryCopy[i], true);
                    break;
                }
                }
            }
        }
    }
    for (int j = 1; j < 9; ++j) {
        if (ammoCopy[j] != 0) {
            switch (j) {
            case 1:
            case 2:
            case 4:
            case 5: {
                EntityDef* find = app->entityDefManager->find(6, 2, j);
                app->game->spawnDropItem(n, n2, find->tileIndex, find, ammoCopy[j], true);
                break;
            }
            }
        }
    }
    for (int k = 0; k < 15; ++k) {
        if ((1 << k & weaponsCopy) != 0x0) {
            int weaponTileNum = app->combat->getWeaponTileNum(k);
            app->game->spawnDropItem(n, n2, weaponTileNum, app->entityDefManager->lookup(weaponTileNum), 1, true);
        }
    }
    app->game->spawnSentryBotCorpse(n, n2, n3, n4, n5);
}

void Player::forceFamiliarReturnDueToMonster() {
    this->familiarReturnsToPlayer(false);
    this->botReturnedDueToMonster = true;
}

void Player::attemptToDeploySentryBot() {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->game->activeMonsters != nullptr) {
        Entity* activeMonsters = app->game->activeMonsters;
        do {
            Entity* nextOnList = activeMonsters->monster->nextOnList;
            if (activeMonsters->distFrom(app->canvas->destX, app->canvas->destY) <= app->combat->tileDistances[0] && activeMonsters->aiIsAttackValid()) {
                app->hud->addMessage((short)0, (short)219, 3);
                return;
            }
            activeMonsters = nextOnList;
        } while (activeMonsters != app->game->activeMonsters && app->game->activeMonsters != nullptr);
    }
    if ((app->render->mapFlags[(app->canvas->viewY + app->canvas->viewStepY >> 6) * 32 + (app->canvas->viewX + app->canvas->viewStepX >> 6)] & 0x10) != 0x0) {
        app->hud->addMessage((short)0, (short)217, 3);
        return;
    }
    short familiar = 0;
    switch (this->ce->weapon) {
    case 4: {
        familiar = 2;
        break;
    }
    case 5: {
        familiar = 3;
        break;
    }
    case 6: {
        familiar = 4;
        break;
    }
    default: {
        familiar = 1;
        break;
    }
    }
    this->setFamiliar(familiar);
}

void Player::attemptToDiscardFamiliar(int n) {
    Applet* app = CAppContainer::getInstance()->app;

    if ((app->render->mapFlags[(app->canvas->viewY >> 6) * 32 + (app->canvas->viewX >> 6)] & 0x20) != 0x0) {
        app->hud->addMessage((short)0, (short)221, 3);
    }
    else {
        app->game->spawnDropItem(app->canvas->viewX, app->canvas->viewY, app->combat->getWeaponTileNum(n), 6, 1, n, this->ammo[7], false);
        this->give(1, n, -1);
    }
}

void Player::startSelfDestructDialog() {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->canvas->state != Canvas::ST_AUTOMAP) {
        this->attemptingToSelfDestructFamiliar = true;
        Text* smallBuffer = app->localization->getSmallBuffer();
        app->localization->composeText((short)0, (short)194, smallBuffer);
        app->canvas->startDialog(nullptr, smallBuffer, 12, 1, false);
        smallBuffer->dispose();
    }
}

bool Player::vendingMachineIsHacked(int n) {
    return (this->hackedVendingMachines & 1 << n) == 1 << n;
}

void Player::setVendingMachineHack(int n) {
    this->hackedVendingMachines |= 1 << n;
}

int Player::getVendingMachineTriesLeft(int max) {
    max = std::max(std::min(max, 18), 1);
    int n;
    if (max > 9) {
        n = this->vendingMachineHackTriesLeft2;
        max -= 9;
    }
    else {
        n = this->vendingMachineHackTriesLeft1;
    }
    return (n >> (3 * (max - 1))) & 0x7;
}

void Player::removeOneVendingMachineTry(int max) {
    max = std::max(std::min(max, 18), 1);
    if (this->getVendingMachineTriesLeft(max) > 0) {
        if (max > 9) {
            max -= 9;
            this->vendingMachineHackTriesLeft2 -= 1 << (3 * (max - 1));
        }
        else {
            this->vendingMachineHackTriesLeft1 -= 1 << (3 * (max - 1));
        }
    }
}

bool Player::weaponIsASentryBot(int n) {
    return (1 << n & Enums::WP_SENTRY_BOT_MASK) != 0x0;
}

bool Player::hasASentryBot() {
    bool b = (this->weapons & 0x8) != 0x0 || (this->weapons & 0x20) != 0x0;
    bool b2 = (this->weapons & 0x10) != 0x0 || (this->weapons & 0x40) != 0x0;
    return b || b2;
}

void Player::setFamiliarType(short familiarType) {
    this->familiarType = familiarType;
    this->calcViewMode();
}

void Player::calcViewMode() {
    Applet* app = CAppContainer::getInstance()->app;

    switch (this->familiarType) {
        case 0: {
            app->render->postProcessMode = 0;
            break;
        }
        case 1:
        case 2: {
            app->render->postProcessMode = 1;
            break;
        }
        case 3:
        case 4: {
            app->render->postProcessMode = 2;
            break;
        }
    }
}

void Player::enterTargetPractice(int n, int n2, int n3, ScriptThread* targetPracticeThread) {
    Applet* app = CAppContainer::getInstance()->app;

    this->inTargetPractice = true;
    app->canvas->targetPracticeThread = targetPracticeThread;
    app->canvas->saveX = app->canvas->viewX;
    app->canvas->saveY = app->canvas->viewY;
    app->canvas->saveZ = app->canvas->viewZ;
    app->canvas->saveAngle = app->canvas->viewAngle;
    int viewAngle;
    if (n3 == 4) {
        viewAngle = Enums::ANGLE_WEST;
    }
    else if (n3 == 0) {
        viewAngle = Enums::ANGLE_EAST;
    }
    else if (n3 == 2) {
        viewAngle = Enums::ANGLE_NORTH;
    }
    else {
        viewAngle = Enums::ANGLE_SOUTH;
    }
    app->canvas->destAngle = (app->canvas->viewAngle = viewAngle);
    app->canvas->destX = (app->canvas->viewX = (n << 6) + 32);
    app->canvas->destY = (app->canvas->viewY = (n2 << 6) + 32);
    app->canvas->destZ = (app->canvas->viewZ = app->render->getHeight(app->canvas->viewX, app->canvas->viewY) + 36);
    this->showHelp((short)17, false);
    this->stripInventoryForTargetPractice();
    this->targetPracticeScore = 0;
    app->render->startFade(750, 2);
}

void Player::assessTargetPracticeShot(Entity* entity) {
    Applet* app = CAppContainer::getInstance()->app;

    int sprite = entity->getSprite();
    int n = app->canvas->zoomCollisionX - app->render->mapSprites[app->render->S_X + sprite];
    int n2 = app->canvas->zoomCollisionY - app->render->mapSprites[app->render->S_Y + sprite];
    int n3 = app->canvas->zoomCollisionZ - app->render->mapSprites[app->render->S_Z + sprite];
    int n4 = app->render->mapSpriteInfo[sprite] >> 8 & 0xF0;
    int(*imageFrameBounds)[4] = app->render->getImageFrameBounds(entity->def->tileIndex, 3, 2, 0);
    int n5 = -1;
    for (int i = 0; i < 3; ++i) {
        if (n > imageFrameBounds[i][0] && n < imageFrameBounds[i][1] && n2 > imageFrameBounds[i][0] && n2 < imageFrameBounds[i][1] && n3 > imageFrameBounds[i][2] && n3 < imageFrameBounds[i][3]) {
            n5 = i;
            break;
        }
    }
    if (n5 != -1) {
        if (n5 == 0 || n4 == 16) {
            app->hud->addMessage((short)230, 4);
            this->targetPracticeScore += 30;
            app->canvas->headShotTime = app->time + 500;
            app->canvas->bodyShotTime = 0;
            app->canvas->legShotTime = 0;
        }
        else if (n5 == 1) {
            app->hud->addMessage((short)231, 4);
            this->targetPracticeScore += 20;
            app->canvas->headShotTime = 0;
            app->canvas->bodyShotTime = app->time + 500;
            app->canvas->legShotTime = 0;
        }
        else {
            app->hud->addMessage((short)232, 4);
            this->targetPracticeScore += 10;
            app->canvas->headShotTime = 0;
            app->canvas->bodyShotTime = 0;
            app->canvas->legShotTime = app->time + 500;
        }
    }
    else {
        app->hud->addMessage((short)68, 4);
        app->canvas->headShotTime = 0;
        app->canvas->bodyShotTime = 0;
        app->canvas->legShotTime = 0;
    }
}

void Player::exitTargetPractice() {
    Applet* app = CAppContainer::getInstance()->app;

    this->inTargetPractice = false;
    app->canvas->destAngle = (app->canvas->viewAngle = app->canvas->saveAngle);
    app->canvas->destX = (app->canvas->viewX = app->canvas->saveX);
    app->canvas->destY = (app->canvas->viewY = app->canvas->saveY);
    app->canvas->destZ = (app->canvas->viewZ = app->canvas->saveZ);
    app->canvas->finishRotation(false);
    this->restoreInventory();
    app->render->startFade(750, 2);
    int n = 240;
    app->localization->resetTextArgs();
    app->localization->addTextArg(this->targetPracticeScore);

    if (this->targetPracticeScore > n / 2) {
        int modifyStat = this->modifyStat(5, 1);
        if (modifyStat > 0) {
            app->localization->addTextArg(modifyStat);
            app->hud->addMessage((short)233, 3);
        }
        else {
            app->hud->addMessage((short)235, 3);
        }
    }
    else {
        app->hud->addMessage((short)234, 3);
    }

    if (app->canvas->targetPracticeThread != nullptr) {
        app->canvas->setState(Canvas::ST_PLAYING);
        app->canvas->targetPracticeThread->run();
        app->canvas->targetPracticeThread = nullptr;
    }
}

void Player::usedChainsaw(bool b) {
    Applet* app = CAppContainer::getInstance()->app;

    this->chainsawStrengthBonusCount++;
    if (b && (app->combat->crFlags & 0x2) != 0x0) {
        this->chainsawStrengthBonusCount++;
    }
    if (this->chainsawStrengthBonusCount >= 30) {
        this->chainsawStrengthBonusCount %= 30;
        int modifyStat = this->modifyStat(4, 2);
        if (modifyStat != 0) {
            app->localization->resetTextArgs();
            app->localization->addTextArg(modifyStat);
            app->hud->addMessage((short)241, 3);
        }
    }
    app->canvas->startShake(666, 1, 0);
}

bool Player::hasANanoDrink() {
    for (int i = 0; i < 11; ++i) {
        if (this->inventory[i] > 0) {
            return true;
        }
    }
    return false;
}

void Player::stripInventoryForViosBattle() {
    this->weaponsCopy = 0;
    this->ammoCopy[3] = this->ammo[3];
    this->ammo[3] = 0;
    if ((this->weapons & 0x4) != 0x0) {
        this->give(1, 2, -1);
        this->weaponsCopy |= 0x4;
    }
    if ((this->weapons & 0x2000) != 0x0) {
        this->give(1, 13, -1);
        this->weaponsCopy |= 0x2000;
    }
    if ((this->weapons & 0x8) != 0x0) {
        this->give(1, 3, -1);
        this->weaponsCopy |= 0x8;
    }
    else if ((this->weapons & 0x10) != 0x0) {
        this->give(1, 4, -1);
        this->weaponsCopy |= 0x10;
    }
    else if ((this->weapons & 0x20) != 0x0) {
        this->give(1, 5, -1);
        this->weaponsCopy |= 0x20;
    }
    else if ((this->weapons & 0x40) != 0x0) {
        this->give(1, 6, -1);
        this->weaponsCopy |= 0x40;
    }
}

void Player::stripInventoryForTargetPractice() {
    this->currentWeaponCopy = this->ce->weapon;
    std::memcpy(this->ammoCopy, this->ammo, sizeof(this->ammo));

    this->weaponsCopy = (this->weapons & -1);
    for (int i = 0; i < 9; ++i) {
        this->ammo[i] = 0;
    }
    this->weapons = 0;
    this->give(1, 9, 1, true);
    this->give(2, 1, 8, true);
}

void Player::restoreInventory() {
    Applet* app = CAppContainer::getInstance()->app;

    if ((this->weapons & 0x1) != 0x0) {
        this->currentWeaponCopy = this->ce->weapon;
        this->ammo[3] = this->ammoCopy[3];
        if ((this->weaponsCopy & 0x4) != 0x0) {
            this->give(1, 2, 1, true);
        }
        if ((this->weaponsCopy & 0x2000) != 0x0) {
            this->give(1, 13, 1, true);
        }
        if ((this->weaponsCopy & 0x8) != 0x0) {
            short n = this->ammo[7];
            this->give(1, 3, 1, true);
            this->ammo[7] = n;
        }
        else if ((this->weaponsCopy & 0x10) != 0x0) {
            short n2 = this->ammo[7];
            this->give(1, 4, 1, true);
            this->ammo[7] = n2;
        }
        else if ((this->weaponsCopy & 0x20) != 0x0) {
            short n3 = this->ammo[7];
            this->give(1, 5, 1, true);
            this->ammo[7] = n3;
        }
        else if ((this->weaponsCopy & 0x40) != 0x0) {
            short n4 = this->ammo[7];
            this->give(1, 6, 1, true);
            this->ammo[7] = n4;
        }
        this->forceRemoveFromScopeZoom();
        this->selectWeapon(this->currentWeaponCopy);
        app->game->angryVIOS = false;
    }
    else {
        bool b = (this->weaponsCopy & 0x200) != 0x0;
        for (int i = 0; i < 9; ++i) {
            this->ammo[i] = 0;
            this->give(2, i, this->ammoCopy[i], true);
        }
        for (int j = 0; j < 15; ++j) {
            if ((1 << j & this->weaponsCopy) != 0x0) {
                this->give(1, j, 1, true);
            }
        }
        this->forceRemoveFromScopeZoom();
        if (!b) {
            this->give(1, 9, -1);
        }
        this->selectWeapon(this->currentWeaponCopy);
    }
}

void Player::forceRemoveFromScopeZoom() {
    Applet* app = CAppContainer::getInstance()->app;

    if (app->canvas->isZoomedIn) {
        app->canvas->zoomTurn = 0;
        app->canvas->handleZoomEvents(-6, 15, true);
    }
}
