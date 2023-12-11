#ifndef __ENTITYMONSTER_H__
#define __ENTITYMONSTER_H__

#include "CombatEntity.h"
class OutputStream;
class InputStream;
class CombatEntity;
class Entity;

class EntityMonster
{
private:

public:
	int touchMe;
	CombatEntity ce;
	Entity* nextOnList;
	Entity* prevOnList;
	Entity* nextAttacker;
	Entity* target;
	int frameTime;
	short flags;
	int monsterEffects;
	uint8_t goalType;
	uint8_t goalFlags;
	uint8_t goalTurns;
	int goalX;
	int goalY;
	int goalParam;

	// Constructor
	EntityMonster();

	void clearEffects();
	void reset();
	void saveGoalState(OutputStream* OS);
	void loadGoalState(InputStream* IS);
	void resetGoal();
};

#endif