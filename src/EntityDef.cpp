#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "EntityDef.h"
#include "JavaStream.h"
#include "Resource.h"

// -----------------------
// EntityDefManager Class
// -----------------------

EntityDefManager::EntityDefManager() {
	std::memset(this, 0, sizeof(EntityDefManager));
}

EntityDefManager::~EntityDefManager() {
}

bool EntityDefManager::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	InputStream IS;
	printf("EntityDefManager::startup\n");

	this->numDefs = 0;

	if (!IS.loadFile(Resources::RES_ENTITIES_BIN_GZ, InputStream::LOADTYPE_RESOURCE)) {
		app->Error("getResource(%s) failed\n", Resources::RES_ENTITIES_BIN_GZ);
	}

	app->resource->read(&IS, sizeof(short));
	this->numDefs = (int)app->resource->shiftShort();
	this->list = new EntityDef[this->numDefs];

	for (int i = 0; i < this->numDefs; i++) {
		app->resource->read(&IS, 8);
		this->list[i].tileIndex = (int16_t)app->resource->shiftShort();
		this->list[i].eType = (uint8_t)app->resource->shiftByte();
		this->list[i].eSubType = (uint8_t)app->resource->shiftByte();
		this->list[i].parm = (uint8_t)app->resource->shiftByte();
		this->list[i].name = (int16_t)app->resource->shiftUByte();
		this->list[i].longName = (int16_t)app->resource->shiftUByte();
		this->list[i].description = (int16_t)app->resource->shiftUByte();
	}

	IS.~InputStream();

	/*for (int i = 0; i < this->numDefs; i++) {
		printf("list[%d]------------------------\n", i);
		printf("tileIndex %d\n", this->list[i].tileIndex);
		printf("eType %d\n", this->list[i].eType);
		printf("eSubType %d\n", this->list[i].eSubType);
		printf("parm %d\n", this->list[i].parm);
		printf("name %d\n", this->list[i].name);
		printf("longName %d\n", this->list[i].longName);
		printf("description %d\n", this->list[i].description);
	}*/

	return true;
}


EntityDef* EntityDefManager::find(int eType, int eSubType) {
	return this->find(eType, eSubType, -1);
}

EntityDef* EntityDefManager::find(int eType, int eSubType, int parm) {
	for (int i = 0; i < this->numDefs; i++) {
		if (this->list[i].eType == eType && this->list[i].eSubType == eSubType && (parm == -1 || this->list[i].parm == parm)) {
			return &this->list[i];
		}
	}
	return nullptr;
}

EntityDef* EntityDefManager::lookup(int tileIndex) {
	for (int i = 0; i < this->numDefs; ++i) {
		if (this->list[i].tileIndex == tileIndex) {
			return &this->list[i];
		}
	}
	return nullptr;
}


// ----------------
// EntityDef Class
// ----------------

EntityDef::EntityDef() {
	std::memset(this, 0, sizeof(EntityDef));
}
