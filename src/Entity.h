#ifndef __ENTITY_H__
#define __ENTITY_H__

class EntityDef;
class EntityMonster;
class LerpSprite;
class OutputStream;
class InputStream;

class Entity
{
private:

public:
	int touchMe;
	EntityDef* def;
	EntityMonster* monster;
	Entity* nextOnTile;
	Entity* prevOnTile;
	short linkIndex;
	short name;
	int info;
	int param;
	int* lootSet;
	int32_t knockbackDelta[2];
	Entity* raiseTargets[4];
	int pos[2];
	int tempSaveBuf[2];

	// Constructor
	Entity();
	// Destructor
	~Entity();

	bool startup();
	void reset();
	void initspawn();
	int getSprite();
	bool touched();
	bool touchedItem();
	bool pain(int n, Entity* entity);
	void checkMonsterDeath(bool b, bool b2);
	void died(bool b, Entity* entity);
	bool deathByExplosion(Entity* entity);
	void aiCalcSimpleGoal(bool b);
	bool aiCalcArchVileGoal();
	void aiMoveToGoal();
	void aiChooseNewGoal(bool b);
	bool aiIsValidGoal();
	bool aiIsAttackValid();
	void aiThink(bool b);
	static bool Entity::CheckWeaponMask(char n1, int n2) {
		return (1 << n1 & n2);
	}
	int aiWeaponForTarget(Entity* entity);
	LerpSprite* aiInitLerp(int travelTime);
	void aiFinishLerp();
	bool checkLineOfSight(int n, int n2, int n3, int n4, int n5);
	bool calcPath(int n, int n2, int n3, int n4, int n5, bool b);
	bool aiGoal_MOVE();
	void aiReachedGoal_MOVE();
	int distFrom(int n, int n2);
	void attack();
	void undoAttack();
	void trimCorpsePile(int n, int n2);
	void knockback(int n, int n2, int n3);
	int getFarthestKnockbackDist(int n, int n2, int n3, int n4, Entity* entity, int n5, int n6, int n7);
	int findRaiseTarget(int n, int n2, int n3);
	void raiseTarget(int n);
	void resurrect(int n, int n2, int n3);
	int* calcPosition();
	bool isBoss();
	bool isHasteResistant();
	bool isDroppedEntity();
	bool isBinaryEntity(int* array);
	bool isNamedEntity(int* array);
	void Entity::saveState(OutputStream* OS, int n);
	void Entity::loadState(InputStream* IS, int n);
	int getSaveHandle(bool b);
	void restoreBinaryState(int n);
	short getIndex();
	void updateMonsterFX();
	void populateDefaultLootSet();
	int findRandomJokeItem();
	void addToLootSet(int n, int n2, int n3);
	bool hasEmptyLootSet();
	
	

	
	
};

#endif