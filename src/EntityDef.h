#ifndef __ENTITYDEF_H__
#define __ENTITYDEF_H__

class EntityDef;

// -----------------------
// EntityDefManager Class
// -----------------------

class EntityDefManager
{
private:
	EntityDef* list;
	int numDefs;
public:

	// Constructor
	EntityDefManager();
	// Destructor
	~EntityDefManager();

	bool startup();
	EntityDef* find(int eType, int eSubType);
	EntityDef* find(int eType, int eSubType, int parm);
	EntityDef* lookup(int tileIndex);
};

// ----------------
// EntityDef Class
// ----------------

class EntityDef
{
private:

public:
	int16_t tileIndex;
	int16_t name;
	int16_t longName;
	int16_t description;
	uint8_t eType;
	uint8_t eSubType;
	uint8_t parm;
	uint8_t touchMe;

	// Constructor
	EntityDef();
};

#endif