#include <stdexcept>

#include "EntityMonster.h"
#include "JavaStream.h"

EntityMonster::EntityMonster() {
	std::memset(this, 0, sizeof(EntityMonster));
}

void EntityMonster::clearEffects() {
	this->monsterEffects = 0;
}

void EntityMonster::reset() {
	this->prevOnList = nullptr;
	this->nextOnList = nullptr;
	this->target = nullptr;
	this->nextAttacker = nullptr;
	this->frameTime = 0;
	this->flags = 0;
	this->clearEffects();
	this->resetGoal();
}

void EntityMonster::saveGoalState(OutputStream* OS) {
	OS->writeInt(this->goalType | this->goalFlags << 4 | this->goalTurns << 8 |this->goalX << 12 | this->goalY << 17 | this->goalParam << 22);
}

void EntityMonster::loadGoalState(InputStream* IS) {
	int args = IS->readInt();
	this->goalType = (uint8_t)(args & 0xF);
	this->goalFlags = (uint8_t)(args >> 4 & 0xF);
	this->goalTurns = (uint8_t)(args >> 8 & 0xF);
	this->goalX = (args >> 12 & 0x1F);
	this->goalY = (args >> 17 & 0x1F);
	this->goalParam = (args >> 22 & 0x3FF);
}

void EntityMonster::resetGoal() {
	this->goalType = 0;
	this->goalFlags = 0;
	this->goalTurns = 0;
	this->goalY = 0;
	this->goalX = 0;
	this->goalParam = 0;
}

