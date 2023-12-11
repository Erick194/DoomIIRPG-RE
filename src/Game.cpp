#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Hud.h"
#include "Text.h"
#include "Game.h"
#include "Sound.h"
#include "Entity.h"
#include "Player.h"
#include "Combat.h"
#include "Canvas.h"
#include "Render.h"
#include "TinyGL.h"
#include "Resource.h"
#include "MayaCamera.h"
#include "LerpSprite.h"
#include "JavaStream.h"
#include "MenuSystem.h"
#include "ParticleSystem.h"
#include "Enums.h"
#include "Utils.h"
#include "SDLGL.h"
#include "Input.h"
#include "GLES.h"

Game::Game() {
	std::memset(this, 0, sizeof(Game));
}

Game::~Game() {
}

bool Game::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	printf("Game::startup\n");

	this->entities = new Entity[275];
	this->ofsMayaTween = new short[6];

	this->difficulty = 1;
	this->cinematicWeapon = -1;

	for (int i = 0; i < 20; i++) {
		this->scriptThreads[i].app = app;
	}

	for (int i = 0; i < 128; i++) {
		this->entityDeathFunctions[i] = -1;
	}

	this->activeMonsters = nullptr;
	this->combatMonsters = nullptr;
	this->inactiveMonsters = nullptr;
	this->mapSecretsFound = 0;
	this->totalSecrets = 0;
	this->keycode[0] = '\0';
	this->gotoTriggered = false;
	this->disableAI = false;
	this->skipMinigames = false;
	this->activeSprites = 0;
	this->activePropogators = 0;
	this->skipDialog = false;

	return true;
}

void Game::unlinkEntity(Entity* entity)
{
	Applet* app = CAppContainer::getInstance()->app;

	if (entity == &this->entities[0]) {
		app->Error(4); // ERR_BADUNLINKWORLD
		return;
	}

	if (entity == this->entityDb[entity->linkIndex]) {
		this->entityDb[entity->linkIndex] = entity->nextOnTile;
	}
	else if (entity->prevOnTile != nullptr) {
		entity->prevOnTile->nextOnTile = entity->nextOnTile;
	}

	if (entity->nextOnTile != nullptr) {
		entity->nextOnTile->prevOnTile = entity->prevOnTile;
	}

	entity->nextOnTile = nullptr;
	entity->prevOnTile = nullptr;
	entity->info &= 0xFFEFFFFF;
}

void Game::linkEntity(Entity* entity, int i, int i2) {
	Applet* app = CAppContainer::getInstance()->app;
	short linkIndex = (short)(i2 * 32 + i);

	if (entity == &this->entities[0]) {
		app->Error(3); // ERR_BADLINKWORLD
		return;
	}

	if (entity->nextOnTile != nullptr || entity->prevOnTile != nullptr || entity == this->entityDb[linkIndex]) {
		app->Error(26); // ERR_LINKLINKEDENTITY
		return;
	}

	Entity *nextOnTile = this->entityDb[linkIndex];
	if (nextOnTile != nullptr) {
		nextOnTile->prevOnTile = entity;
	}

	entity->nextOnTile = nextOnTile;
	entity->prevOnTile = nullptr;
	entity->linkIndex = linkIndex;
	this->entityDb[linkIndex] = entity;
	entity->info |= 0x100000;
}

void Game::unlinkWorldEntity(int i, int i2) {
	Applet* app = CAppContainer::getInstance()->app;

	short linkIndex = (short)(i2 * 32 + i);
	Entity* nextOnTile = this->entityDb[linkIndex];

	Entity* entity = &this->entities[0];
	this->baseVisitedTiles[i2] &= ~(1 << i);
	if (nextOnTile == entity) {
		this->entityDb[linkIndex] = nullptr;
		app->render->mapFlags[linkIndex] &= 0xFFFFFFFE;
		return;
	}

	Entity* nextEntity = nullptr;
	while (nextOnTile != nullptr) {
		if (nextOnTile == entity) {
			if (nextEntity != nullptr) {
				nextEntity->nextOnTile = nullptr;
			}
			app->render->mapFlags[linkIndex] &= 0xFFFFFFFE;
			return;
		}
		nextEntity = nextOnTile;
		nextOnTile = nextOnTile->nextOnTile;
	}

	app->Error(52); // ERR_UNLINKWORLD
}

void Game::linkWorldEntity(int i, int i2) {
	Applet* app = CAppContainer::getInstance()->app;

	short linkIndex = (short)(i2 * 32 + i);
	Entity* nextOnTile = this->entityDb[linkIndex];
	Entity* nextEntity = nullptr;

	Entity* entity = &this->entities[0];
	while (nextOnTile != nullptr) {
		if (nextOnTile->def->eType == 0) {
			app->Error(27); // ERR_LINKWORLD
			return;
		}
		nextEntity = nextOnTile;
		nextOnTile = nextOnTile->nextOnTile;
	}

	if (nextEntity == nullptr) {
		this->entityDb[linkIndex] = entity;
	}
	else {
		nextEntity->nextOnTile = entity;
	}

	entity->nextOnTile = nullptr;
	entity->prevOnTile = nullptr;
	entity->linkIndex = linkIndex;

	this->baseVisitedTiles[i2] |= 1 << i;
	app->render->mapFlags[linkIndex] |= 0x1;
}

void Game::removeEntity(Entity* entity) {
	Applet* app = CAppContainer::getInstance()->app;

	if ((entity->info & 0xFFFF) != 0x0) {
		app->render->mapSpriteInfo[entity->getSprite()] |= 0x10000;
	}
	if ((entity->info & 0x100000)) {
		this->unlinkEntity(entity);
	}
	app->player->facingEntity = nullptr;
}

void Game::trace(int n, int n2, int n3, int n4, Entity* entity, int n5, int n6) {
	this->trace(n, n2, -1, n3, n4, -1, entity, n5, n6, false);
}

void Game::trace(int n, int n2, int n3, int traceCollisionX, int traceCollisionY, int traceCollisionZ, Entity* entity, int n4, int n5, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	int* tracePoints = this->tracePoints;
	int* traceBoundingBox = this->traceBoundingBox;
	int* traceFracs = this->traceFracs;
	Entity** traceEntities = this->traceEntities;
	this->traceEntity = nullptr;
	this->numTraceEntities = 0;
	tracePoints[0] = n;
	tracePoints[1] = n2;
	tracePoints[2] = traceCollisionX;
	tracePoints[3] = traceCollisionY;
	traceBoundingBox[0] = std::max(std::min(n - n5, traceCollisionX - n5), 0);
	traceBoundingBox[1] = std::max(std::min(n2 - n5, traceCollisionY - n5), 0);
	traceBoundingBox[2] = std::min(std::max(n + n5, traceCollisionX + n5), 2047);
	traceBoundingBox[3] = std::min(std::max(n2 + n5, traceCollisionY + n5), 2047);
	for (int i = traceBoundingBox[0] >> 6; i < (traceBoundingBox[2] >> 6) + 1; ++i) {
		for (int j = traceBoundingBox[1] >> 6; j < (traceBoundingBox[3] >> 6) + 1; ++j) {
			Entity* nextOnTile = this->entityDb[i + 32 * j];
			if (nextOnTile != nullptr) {
				while (nextOnTile != nullptr) {
					if (nextOnTile != entity && 0x0 != (n4 & 1 << nextOnTile->def->eType)) {
						if (nextOnTile->def->eType != 0) {
							int sprite = nextOnTile->getSprite();
							int destX;
							int destY;
							int destZ;
							if (nextOnTile->monster != nullptr) {
								if ((nextOnTile->monster->goalFlags & 0x1) != 0x0) {
									destX = 32 + (nextOnTile->monster->goalX << 6);
									destY = 32 + (nextOnTile->monster->goalY << 6);
									destZ = app->render->mapSprites[app->render->S_Z + sprite];
								}
								else {
									destX = app->render->mapSprites[app->render->S_X + sprite];
									destY = app->render->mapSprites[app->render->S_Y + sprite];
									destZ = app->render->mapSprites[app->render->S_Z + sprite];
								}
							}
							else if (nextOnTile->def->eType == 1) {
								destX = app->canvas->destX;
								destY = app->canvas->destY;
								destZ = app->canvas->destZ;
							}
							else {
								destX = app->render->mapSprites[app->render->S_X + sprite];
								destY = app->render->mapSprites[app->render->S_Y + sprite];
								destZ = app->render->mapSprites[app->render->S_Z + sprite];
							}
							if (sprite != -1 && 0x0 != (app->render->mapSpriteInfo[sprite] & 0xF000000)) {
								int n6 = destX;
								int n7 = destY;
								if (0x0 != (app->render->mapSpriteInfo[sprite] & 0x3000000)) {
									destX -= 32;
									n6 += 32;
								}
								else {
									destY -= 32;
									n7 += 32;
								}
								app->render->traceLine[0] = destX;
								app->render->traceLine[1] = destY;
								app->render->traceLine[2] = n6;
								app->render->traceLine[3] = n7;
								int capsuleToLineTrace = app->render->CapsuleToLineTrace(tracePoints, n5 * n5, app->render->traceLine);
								if (capsuleToLineTrace < 16384) {
									traceFracs[this->numTraceEntities] = capsuleToLineTrace;
									traceEntities[this->numTraceEntities++] = nextOnTile;
								}
							}
							else {
								int n8 = 625;
								if (nextOnTile->def->eType == 8) {
									n8 = 256;
								}
								int capsuleToCircleTrace = app->render->CapsuleToCircleTrace(tracePoints, n5 * n5, destX, destY, n8);
								if (capsuleToCircleTrace < 16384) {
									if (n3 >= 0 && traceCollisionZ >= 0 && b) {
										int n9 = n3 + ((traceCollisionZ - n3) * capsuleToCircleTrace >> 14) - destZ;
										if (n9 > -(32 + n5) && n9 < 32 + n5) {
											traceFracs[this->numTraceEntities] = capsuleToCircleTrace;
											traceEntities[this->numTraceEntities++] = nextOnTile;
										}
									}
									else {
										traceFracs[this->numTraceEntities] = capsuleToCircleTrace;
										traceEntities[this->numTraceEntities++] = nextOnTile;
									}
								}
							}
						}
					}
					nextOnTile = nextOnTile->nextOnTile;
				}
			}
		}
	}
	if (0x0 != (n4 & 0x1)) {
		int traceWorld = app->render->traceWorld(0, tracePoints, n5, traceBoundingBox, n4);
		if (traceWorld < 16384) {
			traceFracs[this->numTraceEntities] = traceWorld;
			traceEntities[this->numTraceEntities++] = &this->entities[0];
			this->traceCollisionX = n + ((traceWorld * (traceCollisionX - n)) >> 14);
			this->traceCollisionY = n2 + ((traceWorld * (traceCollisionY - n2)) >> 14);
			this->traceCollisionZ = n3 + ((traceWorld * (traceCollisionZ - n3)) >> 14);
		}
		else {
			this->traceCollisionX = traceCollisionX;
			this->traceCollisionY = traceCollisionY;
			this->traceCollisionZ = traceCollisionZ;
		}
	}
	if (this->numTraceEntities > 0) {
		for (int k = 0; k < this->numTraceEntities - 1; ++k) {
			for (int l = 0; l < this->numTraceEntities - 1 - k; ++l) {
				if (traceFracs[l + 1] < traceFracs[l]) {
					int n10 = traceFracs[l];
					traceFracs[l] = traceFracs[l + 1];
					traceFracs[l + 1] = n10;
					Entity* entity2 = traceEntities[l];
					traceEntities[l] = traceEntities[l + 1];
					traceEntities[l + 1] = entity2;
				}
			}
		}
		this->traceEntity = traceEntities[0];
	}
}

void Game::loadMapEntities() {
	Applet* app = CAppContainer::getInstance()->app;

	this->interpolatingMonsters = false;
	this->monstersTurn = 0;
	for (int i = 0; i < 4; ++i) {
		this->openDoors[i] = nullptr;
	}
	this->watchLine = nullptr;
	app->player->setPickUpWeapon(15);
	for (int j = 0; j < 4; ++j) {
		this->placedBombs[j] = 0;
	}
	for (int k = 0; k < 275; ++k) {
		this->entities[k].reset();
	}
	for (int l = 0; l < 8; ++l) {
		this->levelVars[l] = 0;
	}
	for (int n = 0; n < 32; ++n) {
		this->baseVisitedTiles[n] = 0;
	}
	this->secretActive = false;
	this->numMonsters = 0;
	this->numLerpSprites = 0;
	for (int n2 = 0; n2 < 16; ++n2) {
		this->lerpSprites[n2].hSprite = 0;
	}
	this->numScriptThreads = 0;
	for (int n3 = 0; n3 < 20; ++n3) {
		this->scriptThreads[n3].reset();
	}
	this->numEntities = 0;
	Entity* entity = &this->entities[this->numEntities++];
	entity->def = app->entityDefManager->find(0, 0);
	entity->name = (short)(entity->def->name | 0x400);

	Entity* entity2 = &this->entities[this->numEntities++];
	entity2->def = app->entityDefManager->find(1, 0);
	entity2->name = (short)(entity2->def->name | 0x400);
	entity2->info = 0x20000;

	EntityDef* find = app->entityDefManager->find(12, 0);
	EntityDef* find2 = app->entityDefManager->find(13, 0);
	int n4 = 0;
	for (int n5 = 0; n5 < app->render->numMapSprites; ++n5) {
		int n6 = app->render->mapSpriteInfo[n5] & 0xFF;
		if ((app->render->mapSpriteInfo[n5] & 0x400000) != 0x0) {
			n6 += 257;
		}
		int n7 = app->render->mediaMappings[n6 + 1] - app->render->mediaMappings[n6];
		if (n6 == 234) {
			n7 = 4;
		}
		if (n6 == 156) {
			n7 = 2;
			app->render->mapSpriteInfo[n5] &= 0xFFFF00FF;
			app->render->mapSpriteInfo[n5] |= (n7 << 8 | 0x80200);
		}
		if (n6 == 236) {
			n7 = 3;
			app->render->mapSpriteInfo[n5] &= 0xFFFF00FF;
			app->render->mapSpriteInfo[n5] |= (n7 << 8 | 0x80300);
			app->render->mapSprites[app->render->S_RENDERMODE + n5] = 3;
		}
		if (n6 == 136 || n6 == 234 || n6 == 130) {
			app->render->mapSpriteInfo[n5] &= 0xFFFF00FF;
			app->render->mapSpriteInfo[n5] |= (n7 << 8 | 0x80000);
		}
		if ((app->render->mapSpriteInfo[n5] & 0x200000) != 0x0) {
			app->render->mapSpriteInfo[n5] &= 0xFFDFFFFF;
		}
		else {
			int n15 = app->render->mapSprites[app->render->S_X + n5];
			int n16 = app->render->mapSprites[app->render->S_Y + n5];
			EntityDef* lookup = app->entityDefManager->lookup(n6);
			if (n6 >= 1 && n6 <= 12) {
				app->render->mapSpriteInfo[n5] |= 0x200;
			}
			if ((app->render->mapSpriteInfo[n5] & 0xF000000) != 0x0 && ((n15 & 0x3F) == 0x0 || (n16 & 0x3F) == 0x0)) {
				if ((app->render->mapSpriteInfo[n5] & 0x4000000) != 0x0) {
					++n15;
				}
				else if ((app->render->mapSpriteInfo[n5] & 0x2000000) != 0x0) {
					++n16;
				}
				else if ((app->render->mapSpriteInfo[n5] & 0x1000000) != 0x0) {
					--n16;
				}
				else if ((app->render->mapSpriteInfo[n5] & 0x8000000) != 0x0) {
					--n15;
				}
			}
			if (lookup != nullptr) {
				if (this->numEntities == 275) {
					app->Error(35); // ERR_MAX_ENTITIES
					return;
				}
				Entity* entity3 = &this->entities[this->numEntities];
				entity3->info = (n5 + 1 & 0xFFFF);
				entity3->def = lookup;
				if (entity3->def->eType == 2) {
					if (this->numMonsters == 80) {
						app->Error(37); // ERR_MAX_MONSTERS
						return;
					}
					entity3->monster = &this->entityMonsters[this->numMonsters++];
					entity3->monster->reset();

					app->render->mapSpriteInfo[n5] &= 0xF0FFFFFF;
					if ((app->nextByte() & 0x1) == 0x0 && !entity3->isBoss()) {
						app->render->mapSpriteInfo[n5] |= 0x20000;
					}
					entity3->info |= 0x40000;
					this->deactivate(entity3);

					app->sound->cacheCombatSound(this->getMonsterSound(lookup->eSubType, lookup->parm, Enums::MSOUND_ATTACK1));
					app->sound->cacheCombatSound(this->getMonsterSound(lookup->eSubType, lookup->parm, Enums::MSOUND_ATTACK2));
				}
				else if (entity3->def->eType == 10 && entity3->def->eSubType != 3 && entity3->def->eSubType != 2) {
					this->numDestroyableObj++;
				}
				entity3->initspawn();
				app->render->mapSprites[app->render->S_ENT + n5] = this->numEntities++;
				if ((app->render->mapSpriteInfo[n5] & 0x10000) == 0x0) {
					this->linkEntity(entity3, n15 >> 6, n16 >> 6);
				}
				if (n6 >= 140 && n6 <= 143) {
					app->render->mapSpriteInfo[n5] |= 0x10000;
				}
				++n4;
			}
			else if ((app->render->mapSpriteInfo[n5] & 0x800000) != 0x0) {
				if (this->numEntities == 275) {
					app->Error(35); // ERR_MAX_ENTITIES
					return;
				}
				Entity* entity5 = &this->entities[this->numEntities];
				entity5->info = (n5 + 1 & 0xFFFF);
				if (n6 == 166 || n6 == 168) {
					entity5->def = find2;
				}
				else {
					entity5->def = find;
				}
				entity5->name = (short)(entity5->def->name | 0x400);

				app->render->mapSprites[app->render->S_ENT + n5] = this->numEntities++;
				if ((app->render->mapSpriteInfo[n5] & 0x10000) == 0x0) {
					this->linkEntity(entity5, n15 >> 6, n16 >> 6);
				}
				++n4;
			}
			if ((n5 & 0xF) == 0xF) {
				app->canvas->updateLoadingBar(false);
			}
		}
	}
	this->firstDropIndex = this->numEntities;
	for (int n23 = 0; n23 < 16; ++n23) {
		if (this->numEntities == 275) {
			app->Error(35); // ERR_MAX_ENTITIES
			return;
		}
		this->entities[this->numEntities++].info = (app->render->firstDropSprite + n23 + 1 & 0xFFFF);
		app->render->mapSpriteInfo[app->render->dropSprites[n23]] |= 0x10000;
		++n4;
	}
	for (int n25 = 0; n25 < 1024; ++n25) {
		if ((app->render->mapFlags[n25] & 0x1) != 0x0) {
			Entity* nextOnTile = this->entityDb[n25];
			bool b = true;
			while (nextOnTile != nullptr) {
				nextOnTile = nextOnTile->nextOnTile;
			}
			if (b) {
				this->linkWorldEntity(n25 % 32, n25 / 32);
			}
		}
	}
}

void Game::loadTableCamera(int i, int i2) {
	Applet* app = CAppContainer::getInstance()->app;

	this->cleanUpCamMemory();

	int cnt = app->resource->getNumTableBytes(i2);
	int16_t* array = new int16_t[app->resource->getNumTableShorts(i)];
	int8_t* array2 = new int8_t[cnt];

	app->resource->beginTableLoading();
	app->resource->loadShortTable(array, i);
	app->resource->loadByteTable(array2, i2);
	app->resource->finishTableLoading();

	this->mayaCameras = new MayaCamera[1];
	this->mayaCameras[0].isTableCam = true;
	this->totalMayaCameras = 1;

	int buffPos = 0;
	this->totalMayaCameraKeys = (int)array[buffPos++];
	mayaCameras[0].sampleRate = (int)array[buffPos++];
	this->posShift = 4 - (int)array[buffPos++];
	this->angleShift = 10 - (int)array[buffPos++];

	int size = this->totalMayaCameraKeys * 7;
	this->mayaCameraKeys = new int16_t[size];
	std::memcpy(this->mayaCameraKeys, &array[buffPos], size << 1);
	buffPos += size;

	int size2 = this->totalMayaCameraKeys * 6;
	this->mayaTweenIndices = new int16_t[size2];
	std::memcpy(this->mayaTweenIndices, &array[buffPos], size2 << 1);
	buffPos += size2;

	this->ofsMayaTween[0] = 0;
	short n8 = array[buffPos++];
	this->ofsMayaTween[1] = n8;
	short n9 = (short)(n8 + array[buffPos++]);
	this->ofsMayaTween[2] = n9;
	short n10 = (short)(n9 + array[buffPos++]);
	this->ofsMayaTween[3] = n10;
	short n11 = (short)(n10 + array[buffPos++]);
	this->ofsMayaTween[4] = n11;
	this->ofsMayaTween[5] = (short)(n11 + array[buffPos++]);

	this->mayaCameraTweens = new int8_t[cnt];
	std::memcpy(this->mayaCameraTweens, array2, cnt);

	this->mayaCameras[0].keyOffset = 0;
	this->mayaCameras[0].numKeys = this->totalMayaCameraKeys;
	this->mayaCameras[0].complete = false;
	this->mayaCameras[0].keyThreadResumeCount = this->totalMayaCameraKeys;
	this->setKeyOffsets();

	if (array) {
		delete[] array;
	}
	array = nullptr;

	if (array2) {
		delete[] array2;
	}
	array2 = nullptr;
}

void Game::setKeyOffsets() {
	this->OFS_MAYAKEY_X = 0;
	this->OFS_MAYAKEY_Y = this->totalMayaCameraKeys;
	this->OFS_MAYAKEY_Z = this->totalMayaCameraKeys * 2;
	this->OFS_MAYAKEY_PITCH = this->totalMayaCameraKeys * 3;
	this->OFS_MAYAKEY_YAW = this->totalMayaCameraKeys * 4;
	this->OFS_MAYAKEY_ROLL = this->totalMayaCameraKeys * 5;
	this->OFS_MAYAKEY_MS = this->totalMayaCameraKeys * 6;
}

void Game::loadMayaCameras(InputStream* IS) {
	Applet* app = CAppContainer::getInstance()->app;

	int keyOffset = 0;
	int n = 0;
	short array[6];
	int array2[6];

	for (int i = 0; i < 6; i++) {
		array[i] = 0;
		array2[i] = 0;
	}

	for (int i = 0; i < this->totalMayaCameras; i++) {
		app->resource->read(IS, 3);
		this->mayaCameras[i].numKeys = app->resource->shiftByte();
		this->mayaCameras[i].sampleRate = app->resource->shiftShort();
		app->resource->read(IS, this->mayaCameras[i].numKeys * 7 * sizeof(short));
		this->mayaCameras[i].keyOffset = keyOffset;

		for (int j = 0; j < 7; j++) {
			for (int k = 0; k < this->mayaCameras[i].numKeys; k++) {
				this->mayaCameraKeys[this->totalMayaCameraKeys * j + keyOffset + k] = app->resource->shiftShort();
			}
		}

		keyOffset += this->mayaCameras[i].numKeys;
		int n2 = this->mayaCameras[i].numKeys * 6 * sizeof(short);
		app->resource->read(IS, n2);
		for (int l = 0; l < n2 / 2; l++, n++) {
			short shiftShort = app->resource->shiftShort();
			if (shiftShort >= 0) {
				shiftShort += array[l % 6];
			}
			this->mayaTweenIndices[n] = shiftShort;
		}

		int n3 = 0;
		app->resource->read(IS, 6 * sizeof(short));
		for (int n4 = 0; n4 < 6; n4++) {
			array2[n4] = app->resource->shiftShort();
			n3 += array2[n4];
		}

		app->resource->readMarker(IS, 0xDEADBEEF);
		app->resource->read(IS, n3);
		for (int n5 = 0; n5 < 6; ++n5) {
			short* array3;
			int n7;
			for (int n6 = 0; n6 < array2[n5]; ++n6, array3 = array, n7 = n5, ++array3[n7]) {
				this->mayaCameraTweens[this->ofsMayaTween[n5] + array[n5]] = app->resource->shiftByte();
			}
		}
	}
}

void Game::unloadMapData() {
	Applet* app = CAppContainer::getInstance()->app;

	this->cinematicWeapon = -1;
	this->activeSprites = 0;
	this->animatingEffects = 0;
	this->activePropogators = 0;
	this->activeCameraView = false;
	this->activeCamera = nullptr;
	app->player->facingEntity = nullptr;
	app->combat->curTarget = nullptr;
	app->combat->curAttacker = nullptr;
	app->hud->lastTarget = nullptr;
	for (int i = 0; i < 48; ++i) {
		if ((this->gsprites[i].flags & 0x1)) {
			this->gsprite_destroy(&gsprites[i]);
		}
	}
	this->inactiveMonsters = nullptr;
	this->activeMonsters = nullptr;
	this->combatMonsters = nullptr;
	this->keycode[0] = '\0';

	for (int j = 0; j < 1024; ++j) {
		this->entityDb[j] = nullptr;
	}
	for (int k = 0; k < this->numEntities; ++k) {
		this->entities[k].reset();
	}
	for (int l = 0; l < 128; ++l) {
		this->entityDeathFunctions[l] = -1;
	}
	this->totalMayaCameraKeys = 0;
	this->totalMayaCameras = 0;
	this->cleanUpCamMemory();
	app->combat->lerpWpDown = false;
	app->combat->weaponDown = false;
	this->numDestroyableObj = 0;
	this->destroyedObj = 0;
	this->lootFound = 0;
	app->canvas->showingLoot = false;
	this->angryVIOS = false;
	app->sound->freeMonsterSounds();
}

bool Game::touchTile(int n, int n2, bool b) {
	bool b2 = false;
	Entity* nextOnTile;
	//printf("touchTile %d, %d\n", n, n2);
	for (Entity* mapEntity = this->findMapEntity(n, n2); mapEntity != nullptr; mapEntity = nextOnTile) {
		nextOnTile = mapEntity->nextOnTile;
		if (b || mapEntity->def->eType == Enums::ET_ENV_DAMAGE) {
			mapEntity->touched();
			b2 = true;
		}
	}
	return b2;
}

void Game::prepareMonsters() {
	Applet* app = CAppContainer::getInstance()->app;

	int n = app->canvas->loadMapID - 1;
	app->player->fillMonsterStats();
	if (!this->isLoaded && 0x0 != (app->player->completedLevels & 1 << n) && app->player->monsterStats[0] == app->player->monsterStats[1]) {
		int n2 = 0;
		int n3 = app->player->monsterStats[1];
		int n4 = n3 - app->player->monsterStats[0];
		for (int i = 0; i < this->numEntities; ++i) {
			Entity* entity = &this->entities[i];
			if (entity->monster != nullptr) {
				int sprite = entity->getSprite();
				EntityMonster* monster = entity->monster;
				if (0x0 != (entity->info & 0x1010000) && 0x0 == (monster->flags & 0x90) && n4 < n3 / 2 && n2 < 8 && app->nextByte() <= 100) {
					Render* render = app->render;
					if (this->findMapEntity(render->mapSprites[render->S_X + sprite], render->mapSprites[render->S_Y + sprite], 15535) == nullptr) {
						entity->resurrect(render->mapSprites[render->S_X + sprite], render->mapSprites[render->S_Y + sprite], 32);
						entity->info &= 0xFFBFFFFF;
						++n4;
						++n2;
					}
				}
			}
		}
	}
}

Entity* Game::findMapEntity(int n, int n2) {
	n >>= 6;
	n2 >>= 6;
	if (n < 0 || n >= 32 || n2 < 0 || n2 >= 32) {
		return nullptr;
	}
	return this->entityDb[n2 * 32 + n];
}

Entity* Game::findMapEntity(int n, int n2, int n3) {
	Applet* app = CAppContainer::getInstance()->app;

	if ((n3 & 0x2) != 0x0 && n >> 6 == app->canvas->destX >> 6 && n2 >> 6 == app->canvas->destY >> 6) {
		return &this->entities[1];
	}
	for (Entity* entity = this->findMapEntity(n, n2); entity != nullptr; entity = entity->nextOnTile) {
		if ((n3 & 1 << entity->def->eType) != 0x0) {
			return entity;
		}
	}
	return nullptr;
}

void Game::activate(Entity* entity) {
	this->activate(entity, true, false, true, false);
}

void Game::activate(Entity* entity, bool b, bool b2, bool b3, bool b4) {
	Applet* app = CAppContainer::getInstance()->app;

	EntityMonster* monster = entity->monster;
	if (((app->render->mapSpriteInfo[entity->getSprite()] & 0xFF00) >> 8 & 0xF0) == 16 && !app->render->shotsFired) {
		return;
	}
	if (b2 && entity->distFrom(app->canvas->viewX, app->canvas->viewY) > app->combat->tileDistances[3]) {
		return;
	}
	entity->info |= 0x400000;
	if (app->player->noclip) {
		return;
	}
	if ((entity->info & 0x40000) != 0x0) {
		return;
	}
	int sprite = entity->getSprite();
	app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x0);
	if (monster->nextOnList != nullptr) {
		if (entity == this->inactiveMonsters && monster->nextOnList == this->inactiveMonsters) {
			this->inactiveMonsters = nullptr;
		}
		else {
			if (entity == this->inactiveMonsters) {
				this->inactiveMonsters = monster->nextOnList;
			}
			monster->nextOnList->monster->prevOnList = monster->prevOnList;
			monster->prevOnList->monster->nextOnList = monster->nextOnList;
		}
	}
	if (this->activeMonsters == nullptr) {
		monster->nextOnList = entity;
		monster->prevOnList = entity;
		this->activeMonsters = entity;
	}
	else {
		monster->prevOnList = this->activeMonsters->monster->prevOnList;
		monster->nextOnList = this->activeMonsters;
		this->activeMonsters->monster->prevOnList->monster->nextOnList = entity;
		this->activeMonsters->monster->prevOnList = entity;
	}
	entity->info |= 0x40000;
	monster->flags &= ~Enums::MFLAG_NOACTIVATE;
	if (b && 0x0 != (monster->flags & 0x2)) {
		this->executeStaticFunc(9);
		monster->flags &= ~Enums::MFLAG_TRIGGERONACTIVATE;
	}
	if (app->canvas->state == Canvas::ST_PLAYING && b3) {
		int MonsterSound = this->getMonsterSound(entity->def->eSubType, (char)entity->def->parm, Enums::MSOUND_ALERT1);
		app->sound->playSound(MonsterSound, 0, 3, 0);
	}
}

void Game::killAll() {
	for (int i = 0; i < this->numEntities; ++i) {
		Entity* entity = &this->entities[i];
		if (entity->def != nullptr && entity->def->eType == 2) {
			if (!entity->isBoss()) {
				if ((entity->info & 0x20000) != 0x0) {
					if ((entity->monster->flags & 0x80) == 0x0) {
						entity->died(false, nullptr);
					}
				}
			}
		}
	}
}

void Game::deactivate(Entity* entity) {
	EntityMonster* monster = entity->monster;
	if ((entity->info & 0x40000) == 0x0) {
		return;
	}
	if (monster->nextOnList != nullptr) {
		if (entity == this->activeMonsters && monster->nextOnList == this->activeMonsters) {
			this->activeMonsters = nullptr;
		}
		else {
			if (entity == this->activeMonsters) {
				this->activeMonsters = monster->nextOnList;
			}
			monster->nextOnList->monster->prevOnList = monster->prevOnList;
			monster->prevOnList->monster->nextOnList = monster->nextOnList;
		}
	}
	if (this->inactiveMonsters == nullptr) {
		EntityMonster* entityMonster = monster;
		monster->nextOnList = entity;
		entityMonster->prevOnList = entity;
		this->inactiveMonsters = entity;
	}
	else {
		monster->prevOnList = this->inactiveMonsters->monster->prevOnList;
		monster->nextOnList = this->inactiveMonsters;
		this->inactiveMonsters->monster->prevOnList->monster->nextOnList = entity;
		this->inactiveMonsters->monster->prevOnList = entity;
	}
	entity->info &= ~0x40000;
}

void Game::UpdatePlayerVars() {
	Applet* app = CAppContainer::getInstance()->app;

	this->viewX = app->canvas->viewX;
	this->viewY = app->canvas->viewY;
	this->viewAngle = app->canvas->viewAngle;
	this->destX = app->canvas->destX;
	this->destY = app->canvas->destY;
	this->destAngle = app->canvas->destAngle;
	this->viewSin = app->canvas->viewSin;
	this->viewCos = app->canvas->viewCos;
	this->viewStepX = app->canvas->viewStepX;
	this->viewStepY = app->canvas->viewStepY;
	this->viewRightStepX = app->canvas->viewRightStepX;
	this->viewRightStepY = app->canvas->viewRightStepY;
}

void Game::monsterAI() {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->disableAI) {
		return;
	}
	if (this->monstersUpdated) {
		return;
	}
	this->monstersUpdated = true;
	this->combatMonsters = nullptr;
	if (this->activeMonsters != nullptr) {
		int i = 0;
		Entity* activeMonsters = this->activeMonsters;
		do {
			Entity* nextOnList = activeMonsters->monster->nextOnList;
			if (0x0 == (activeMonsters->monster->monsterEffects & 0x2) && (this->monstersTurn == 1 || (this->monstersTurn == 2 && activeMonsters->isHasteResistant()))) {
				activeMonsters->aiThink(false);
				if ((activeMonsters->monster->goalFlags & 0x1) != 0x0) {
					i = 1;
				}
			}
			activeMonsters = nextOnList;
		} while (activeMonsters != this->activeMonsters && this->activeMonsters != nullptr);
		while (i != 0) {
			i = 0;
			Entity* nextAttacker;
			for (Entity* combatMonsters = this->combatMonsters; combatMonsters != nullptr; combatMonsters = nextAttacker) {
				nextAttacker = combatMonsters->monster->nextAttacker;
				if (!combatMonsters->aiIsAttackValid()) {
					combatMonsters->undoAttack();
					combatMonsters->aiThink(false);
					if ((combatMonsters->monster->goalFlags & 0x1) != 0x0) {
						i = 1;
					}
				}
			}
		}
	}
}

void Game::monsterLerp() {
	Applet* app = CAppContainer::getInstance()->app;

	bool interpolatingMonsters = false;
	int n = 0;
	if (!this->interpolatingMonsters) {
		return;
	}
	if (this->activeMonsters != nullptr) {
		Entity* entity = this->activeMonsters;
		do {
			if ((entity->monster->goalFlags & 0x1) != 0x0) {
				interpolatingMonsters = true;
				if (n == 0 && app->render->checkTileVisibilty(entity->monster->goalX, entity->monster->goalY)) {
					n = 1;
				}
			}
			entity = entity->monster->nextOnList;
		} while (entity != this->activeMonsters);
	}
	if (interpolatingMonsters && n != 0) {
		app->canvas->invalidateRect();
	}
	this->interpolatingMonsters = interpolatingMonsters;
}

void Game::spawnPlayer() {
	Applet* app = CAppContainer::getInstance()->app;
	int n;
	int n2;
	int mapSpawnDir;
	if (this->spawnParam == 0) {
		if (app->canvas->loadType == 3) {
			n = 3;
			n2 = 15;
			mapSpawnDir = 6;
		}
		else {
			n = app->render->mapSpawnIndex % 32;
			n2 = app->render->mapSpawnIndex / 32;
			mapSpawnDir = app->render->mapSpawnDir;
		}
	}
	else {
		n = (this->spawnParam & 0x1F);
		n2 = (this->spawnParam >> 5 & 0x1F);
		mapSpawnDir = (this->spawnParam >> 10 & 0xFF);
		this->spawnParam = 0;
	}
	int destAngle = mapSpawnDir << 7 & 0x3FF;
	app->canvas->viewX = (app->canvas->destX = n * 64 + 32);
	app->canvas->viewY = (app->canvas->destY = n2 * 64 + 32);
	app->canvas->viewZ = (app->canvas->destZ = app->render->getHeight(app->canvas->viewX, app->canvas->viewY) + 36);
	app->canvas->viewAngle = (app->canvas->destAngle = destAngle);
	app->player->relink();
	this->lastTurnTime = app->time;
	app->canvas->invalidateRect();
}

void Game::eventFlagsForMovement(int n, int n2, int n3, int n4) {
	int n5 = n3 - n;
	int n6 = n4 - n2;
	this->eventFlags[0] = 2;
	this->eventFlags[1] = 1;
	this->eventFlags[0] |= this->eventFlagForDirection(n5, n6);
	this->eventFlags[1] |= this->eventFlagForDirection(-n5, -n6);
}

int Game::eventFlagForDirection(int n, int n2) {
	int n3;
	if (n > 0) {
		if (n2 < 0) {
			n3 = 32;
		}
		else if (n2 > 0) {
			n3 = 2048;
		}
		else {
			n3 = 16;
		}
	}
	else if (n < 0) {
		if (n2 < 0) {
			n3 = 128;
		}
		else if (n2 > 0) {
			n3 = 512;
		}
		else {
			n3 = 256;
		}
	}
	else if (n2 > 0) {
		n3 = 1024;
	}
	else {
		n3 = 64;
	}
	return n3;
}

void Game::givemap(int n, int n2, int n3, int n4) {
	Applet* app = CAppContainer::getInstance()->app;

	for (int i = 0; i < app->render->numLines; ++i) {
		app->render->lineFlags[i >> 1] |= (uint8_t)(8 << ((i & 0x1) << 2));
	}
	for (int numMapSprites = app->render->numMapSprites, j = 0; j < numMapSprites; ++j) {
		int n6 = app->render->mapSprites[app->render->S_X + j] >> 6;
		int n7 = app->render->mapSprites[app->render->S_Y + j] >> 6;
		if (n6 >= n && n6 < n + n3 && n7 >= n2 && n7 < n2 + n4) {
			app->render->mapSpriteInfo[j] |= 0x200000;
		}
	}
	for (int k = n2; k < n2 + n4; ++k) {
		for (int l = n; l < n + n3; ++l) {
			uint8_t b = app->render->mapFlags[k * 32 + l];
			if ((b & 0x1) == 0x0) {
				app->render->mapFlags[k * 32 + l] = (uint8_t)(b | 0x80);
			}
		}
	}
}

bool Game::performDoorEvent(int n, Entity* entity, int n2) {
	return this->performDoorEvent(n, entity, n2, false);
}

bool Game::performDoorEvent(int n, Entity* entity, int n2, bool b) {
	return this->performDoorEvent(n, nullptr, entity, n2, b);
}

bool Game::performDoorEvent(int n, ScriptThread* scriptThread, Entity* watchLine, int n2, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	int sprite = watchLine->getSprite();
	int n3 = app->render->mapSpriteInfo[sprite];
	int n4 = n3 & 0xFF;
	bool b2 = (watchLine->info & 0x100000) == 0x0;
	if ((n3 & 0x400000) != 0x0) {
		n4 += 257;
	}

	bool b3 = n4 >= 271 && n4 < 281;
	if (watchLine->def->eSubType == 1) {
		return false;
	}
	if (n == 0 && b2 && b3) {
		this->updatePlayerDoors(watchLine, true);
		return false;
	}
	if (n == 1 && !b2) {
		return false;
	}
	if (b2 && b3) {
		for (Entity* entity = this->findMapEntity(watchLine->linkIndex % 32 << 6, watchLine->linkIndex / 32 << 6); entity != nullptr; entity = entity->nextOnTile) {
			if (entity->def->eType == 2 && entity->def->eSubType != 17) {
				this->watchLine = watchLine;
				return false;
			}
		}
		if (this->watchLine == watchLine) {
			this->watchLine = nullptr;
		}
		this->linkEntity(watchLine, watchLine->linkIndex % 32, watchLine->linkIndex / 32);
	}
	LerpSprite* allocLerpSprite = this->allocLerpSprite(scriptThread, sprite, true);
	app->render->mapSpriteInfo[sprite] = (n3 & 0xFFFEFFFF);
	if (b) {
		this->secretActive = true;
		allocLerpSprite->flags |= Enums::LS_FLAG_SECRET_OPEN;
	}
	else {
		allocLerpSprite->flags |= Enums::LS_FLAG_DOOROPEN;
		app->render->mapSpriteInfo[sprite] |= 0x80000000;
	}

	allocLerpSprite->dstX = allocLerpSprite->srcX = app->render->mapSprites[app->render->S_X + sprite];
	allocLerpSprite->dstY = allocLerpSprite->srcY = app->render->mapSprites[app->render->S_Y + sprite];
	allocLerpSprite->dstZ = allocLerpSprite->srcZ = app->render->mapSprites[app->render->S_Z + sprite];
	allocLerpSprite->dstScale = allocLerpSprite->srcScale = app->render->mapSprites[app->render->S_SCALEFACTOR + sprite];
	
	int n10 = 32;
	if (n == 1) {
		n10 = -n10;
	}
	if (0x0 != (n3 & 0x3000000)) {
		if (b) {
			if ((n3 & 0x1000000) != 0x0) {
				allocLerpSprite->dstY += n10;
			}
			else {
				allocLerpSprite->dstY -= n10;
			}
		}
		else if ((watchLine->def->parm & 0x1) == 0x0) {
			allocLerpSprite->dstX += n10;
		}
	}
	else if (0x0 != (n3 & 0xC000000)) {
		if (b) {
			if ((n3 & 0x8000000) != 0x0) {
				allocLerpSprite->dstX += n10;
			}
			else {
				allocLerpSprite->dstX -= n10;
			}
		}
		else if ((watchLine->def->parm & 0x1) == 0x0) {
			allocLerpSprite->dstY += n10;
		}
	}
	allocLerpSprite->startTime = app->gameTime;
	allocLerpSprite->travelTime = 750;
	allocLerpSprite->flags |= (Enums::LS_FLAG_ENT_NORELINK | Enums::LS_FLAG_S_NORELINK);
	if (n == 1) {
		allocLerpSprite->flags &= ~(Enums::LS_FLAG_ENT_NORELINK | Enums::LS_FLAG_DOOROPEN);
		allocLerpSprite->flags |= Enums::LS_FLAG_DOORCLOSE;
		allocLerpSprite->dstScale = 64;
		if (!b && b3) {
			this->updatePlayerDoors(watchLine, false);
		}
		app->sound->playSound(1028, 0, 3, 0);
	}
	else if (n == 0 && !b) {
		if (b3) {
			this->updatePlayerDoors(watchLine, true);
		}
		allocLerpSprite->dstScale = 0;
		app->sound->playSound(1030, 0, 3, 0);
	}
	app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = (uint16_t)((uint8_t)allocLerpSprite->srcScale);
	app->render->mapSprites[app->render->S_X + sprite] = (short)allocLerpSprite->srcX;
	app->render->mapSprites[app->render->S_Y + sprite] = (short)allocLerpSprite->srcY;
	app->render->mapSprites[app->render->S_Z + sprite] = (short)allocLerpSprite->srcZ;
	if (b3 && !(allocLerpSprite->flags & Enums::LS_FLAG_DOORCLOSE)) {
		app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x100);
	}
	if (n2 == 0 || app->canvas->state == Canvas::ST_AUTOMAP || (n2 == 2 && app->render->cullBoundingBox(allocLerpSprite->srcX + allocLerpSprite->dstX >> 1, allocLerpSprite->srcY + allocLerpSprite->dstY >> 1, true))) {
		this->snapLerpSprites(sprite);
	}
	return true;
}

void Game::lerpSpriteAsDoor(int n, int n2, ScriptThread* scriptThread) {
	Applet* app = CAppContainer::getInstance()->app;

	LerpSprite* allocLerpSprite = this->allocLerpSprite(scriptThread, n, false);

	app->render->mapSpriteInfo[n] &= 0xFFFEFFFF;
	int n3 = app->render->mapSpriteInfo[n];
	int n4 = 64;
	if (n2 == 1) {
		n4 = -n4;
	}
	allocLerpSprite->flags |= Enums::LS_FLAG_DOOROPEN;
	app->render->mapSpriteInfo[n] |= 0x80000000;
	app->render->mapSprites[app->render->S_SCALEFACTOR + n] = 256;
	allocLerpSprite->dstX = allocLerpSprite->srcX = app->render->mapSprites[app->render->S_X + n];
	allocLerpSprite->dstY = allocLerpSprite->srcY = app->render->mapSprites[app->render->S_Y + n];
	allocLerpSprite->dstZ = allocLerpSprite->srcZ = app->render->mapSprites[app->render->S_Z + n];
	allocLerpSprite->dstScale = allocLerpSprite->srcScale = 64;
	if (0x0 != (n3 & 0x3000000)) {

		allocLerpSprite->dstX += n4;
	}
	else if (0x0 != (n3 & 0xC000000)) {
		allocLerpSprite->dstY += n4;
	}
	allocLerpSprite->startTime = app->gameTime;
	allocLerpSprite->travelTime = 750;
	allocLerpSprite->flags |= (Enums::LS_FLAG_ENT_NORELINK | Enums::LS_FLAG_S_NORELINK);
	if (n2 == 1) {
		allocLerpSprite->flags &= ~(Enums::LS_FLAG_ENT_NORELINK | Enums::LS_FLAG_DOOROPEN);
		allocLerpSprite->flags |= Enums::LS_FLAG_DOORCLOSE;
	}
	app->render->mapSprites[app->render->S_X + n] = (short)allocLerpSprite->srcX;
	app->render->mapSprites[app->render->S_Y + n] = (short)allocLerpSprite->srcY;
	app->render->mapSprites[app->render->S_Z + n] = (short)allocLerpSprite->srcZ;
}

void Game::updatePlayerDoors(Entity* entity, bool b) {
	bool b2 = false;
	int i;
	for (i = 0; i < 6; ++i) {
		if ((b && this->openDoors[i] == nullptr) || (!b && this->openDoors[i] == entity)) {
			b2 = true;
			break;
		}
	}
	if (b2) {
		if (b) {
			this->openDoors[i] = entity;
		}
		else {
			this->openDoors[i] = nullptr;
		}
	}
}

bool Game::CanCloseDoor(Entity* entity) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->player->isFamiliar) {
		return false;
	}
	int n = (entity->linkIndex % 32 << 6) + 32;
	int n2 = (entity->linkIndex / 32 << 6) + 32;
	int sprite = entity->getSprite();
	if (this->findMapEntity(n, n2, 6) != nullptr) {
		return false;
	}
	int n3 = 64;
	int n4 = 64;
	if ((app->render->mapSpriteInfo[sprite] & 0x3000000) == 0x0) {
		n4 = 0;
	}
	else {
		n3 = 0;
	}
	return ((this->findMapEntity(n + n3, n2 + n4, 6) == nullptr) && (findMapEntity(n - n3, n2 - n4, 6) == nullptr));
}

void Game::advanceTurn() {
	Applet* app = CAppContainer::getInstance()->app;
	this->queueAdvanceTurn = false;
	if (this->interpolatingMonsters) {
		app->Error(95); // ERR_NONSNAPPEDMONSTERS
	}
	bool b = false;
	if (app->player->statusEffects[2] > 0) {
		++app->player->statusEffects[20];
		if (app->player->statusEffects[20] % 2 == 0) {
			b = true;
		}
	}
	else {
		b = true;
	}
	app->canvas->pushedWall = false;
	app->player->advanceTurn();
	this->updateBombs();
	if (b) {
		this->monstersTurn = 1;
	}
	else {
		this->monstersTurn = 2;
	}
	this->monstersUpdated = false;
	this->lastTurnTime = app->time;
	if (b) {
		for (int i = 0; i < this->numEntities; ++i) {
			this->entities[i].updateMonsterFX();
		}
		app->canvas->updateFacingEntity = true;
	}
	for (int j = 0; j < 6; ++j) {
		Entity* openDoors = this->openDoors[j];
		if (openDoors != nullptr) {
			if (this->CanCloseDoor(openDoors)) {
				this->performDoorEvent(1, openDoors, 2);
			}
		}
	}
	this->executeStaticFunc(6);
	app->canvas->startRotation(true);
}

void Game::gsprite_clear(int n) {
	for (int i = 0; i < 48; ++i) {
		GameSprite* gameSprite = &this->gsprites[i];
		if ((gameSprite->flags & 0x1) != 0x0) {
			if ((gameSprite->flags & n) != 0x0) {
				this->gsprite_destroy(gameSprite);
			}
		}
	}
}

GameSprite* Game::gsprite_alloc(int n, int n2, int n3) {
	Applet* app = CAppContainer::getInstance()->app;

	int n4;
	for (n4 = 0; n4 < 48 && (this->gsprites[n4].flags & 0x1) != 0x0; ++n4) {}
	if (n4 == 48) {
		app->Error(34); // ERR_MAX_CUSTOMSPRITES
		return nullptr;
	}
	GameSprite* gameSprite = &this->gsprites[n4];
	gameSprite->sprite = app->render->customSprites[n4];
	for (int i = 0; i < 6; ++i) {
		gameSprite->pos[i] = 0;
	}
	gameSprite->vel[2] = 0;
	gameSprite->vel[1] = 0;
	gameSprite->vel[0] = 0;
	gameSprite->duration = 0;
	gameSprite->time = app->time;
	gameSprite->data = nullptr;
	app->render->mapSprites[app->render->S_ENT + gameSprite->sprite] = -1;
	app->render->mapSprites[app->render->S_RENDERMODE + gameSprite->sprite] = 0;
	app->render->mapSprites[app->render->S_SCALEFACTOR + gameSprite->sprite] = 64;
	gameSprite->flags = (n3 | 0x1);
	app->render->mapSpriteInfo[gameSprite->sprite] = (n | n2 << 8);
	if ((gameSprite->flags & 0x2)) {
		this->activePropogators++;
	}
	this->activeSprites++;
	app->canvas->invalidateRect();
	return gameSprite;
}

GameSprite* Game::gsprite_allocAnim(int n, int n2, int n3, int n4) {
	Applet* app = CAppContainer::getInstance()->app;

	GameSprite* gsprite_alloc = this->gsprite_alloc(n, 0, 66);
	gsprite_alloc->numAnimFrames = 4;
	gsprite_alloc->pos[3] = n2;
	gsprite_alloc->pos[0] = n2;
	app->render->mapSprites[app->render->S_X + gsprite_alloc->sprite] = n2; 
	gsprite_alloc->pos[4] = n3;
	gsprite_alloc->pos[1] = n3;
	app->render->mapSprites[app->render->S_Y + gsprite_alloc->sprite] = n3;
	gsprite_alloc->pos[5] = n4;
	gsprite_alloc->pos[2] = n4;
	app->render->mapSprites[app->render->S_Z + gsprite_alloc->sprite] = n4;
	gsprite_alloc->destScale = 64;
	gsprite_alloc->startScale = 64;
	gsprite_alloc->duration = 200 * gsprite_alloc->numAnimFrames;
	switch (n) {
	case 241: { // TILENUM_POOF
		app->render->mapSprites[app->render->S_RENDERMODE + gsprite_alloc->sprite] = 4;
		break;
	}
	case 234: { // TILENUM_ANIM_FIRE
		app->render->mapSprites[app->render->S_RENDERMODE + gsprite_alloc->sprite] = 3;
		app->render->mapSprites[app->render->S_SCALEFACTOR + gsprite_alloc->sprite] = 48;
		app->render->mapSprites[app->render->S_Z + gsprite_alloc->sprite] = (short)(app->render->getHeight(n2, n3) + 32);
		if ((app->nextInt() & 0x1) != 0x0) {
			app->render->mapSpriteInfo[gsprite_alloc->sprite] |= 0x20000;
			break;
		}
		app->render->mapSpriteInfo[gsprite_alloc->sprite] &= 0xFFFDFFFF;
		break;
	}
	case 242: { // TILENUM_FIRE_BALL
		app->render->mapSprites[app->render->S_RENDERMODE + gsprite_alloc->sprite] = 3;
		break;
	}
	case 208: { // TILENUM_FOG_GRAY
		gsprite_alloc->numAnimFrames = 1;
		gsprite_alloc->duration = 400;
		break;
	}
	case 245: // TILENUM_MONSTER_CLAW
	case 246: // TILENUM_MONSTER_BITE
	case 247: { // TILENUM_MONSTER_BLUNT_TRAUMA
		gsprite_alloc->duration = 200;
		gsprite_alloc->flags |= 0x1002;
		gsprite_alloc->numAnimFrames = 1;
		gsprite_alloc->pos[0] = -6;
		gsprite_alloc->pos[3] = -6;
		gsprite_alloc->pos[1] = 0;
		gsprite_alloc->pos[4] = 0;
		gsprite_alloc->vel[2] = 0;
		gsprite_alloc->vel[1] = 0;
		gsprite_alloc->vel[0] = 0;
		gsprite_alloc->pos[5] -= (short)app->canvas->viewZ;
		gsprite_alloc->pos[2] = gsprite_alloc->pos[5];
		app->render->mapSprites[app->render->S_X + gsprite_alloc->sprite] = (short)(app->canvas->viewX + app->canvas->viewStepX + (this->viewStepX >> 6) * gsprite_alloc->pos[0] + (this->viewRightStepX >> 6) * gsprite_alloc->pos[1]);
		app->render->mapSprites[app->render->S_Y + gsprite_alloc->sprite] = (short)(app->canvas->viewY + app->canvas->viewStepY + (this->viewStepY >> 6) * gsprite_alloc->pos[0] + (this->viewRightStepY >> 6) * gsprite_alloc->pos[1]);
		app->render->mapSprites[app->render->S_Z + gsprite_alloc->sprite] = (short)(app->canvas->viewZ + gsprite_alloc->pos[2]);
		break;
	}
	case 252: { // TILENUM_ACID_SPIT
		gsprite_alloc->numAnimFrames = 2;
		break;
	}
	case 170: { // TILENUM_SENTINEL_SPIKES
		gsprite_alloc->numAnimFrames = 1;
		gsprite_alloc->duration = 200;
		break;
	}
	}
	app->render->relinkSprite(gsprite_alloc->sprite);
	return gsprite_alloc;
}

void Game::gsprite_destroy(GameSprite* gameSprite) {
	Applet* app = CAppContainer::getInstance()->app;

	if ((gameSprite->flags & 0x2000) == 0x0) {
		app->render->mapSpriteInfo[gameSprite->sprite] |= 0x10000;
	}
	else if ((gameSprite->flags & 0x8000) == 0x0) {
		app->render->mapSpriteInfo[gameSprite->sprite] &= 0xFFFFFEFF;
		app->render->mapSprites[app->render->S_X + gameSprite->sprite] = gameSprite->pos[3];
		app->render->mapSprites[app->render->S_Y + gameSprite->sprite] = gameSprite->pos[4];
		if ((gameSprite->flags & 0x4000) == 0x0) {
			app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = gameSprite->pos[5];
		}
		else {
			app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = gameSprite->pos[2];
		}
		app->render->relinkSprite(gameSprite->sprite);
	}
	gameSprite->flags &= 0xFFFFFFFE;
	if ((gameSprite->flags & 0x8000) != 0x0) {
		if ((gameSprite->flags & 0x4000) != 0x0) {
			gameSprite->flags &= 0xFFFF3FFF;
			gameSprite->flags |= 0x1;
			app->render->mapSpriteInfo[gameSprite->sprite] &= 0xFFFEFFFF;
			gameSprite->pos[0] = app->render->mapSprites[app->render->S_X + gameSprite->sprite];
			gameSprite->pos[1] = app->render->mapSprites[app->render->S_Y + gameSprite->sprite];
			gameSprite->pos[2] = app->render->mapSprites[app->render->S_Z + gameSprite->sprite];
			gameSprite->pos[5] = (short)(app->render->getHeight(app->render->mapSprites[app->render->S_X + gameSprite->sprite], app->render->mapSprites[app->render->S_Y + gameSprite->sprite]) + 32);
			gameSprite->time = app->time;
			gameSprite->duration = 250;
			if (gameSprite->vel[0] > 0) {
				gameSprite->pos[3] -= 31;
				gameSprite->vel[0] = -31000 / gameSprite->duration;
			}
			else if (gameSprite->vel[0] < 0) {
				gameSprite->pos[3] += 31;
				gameSprite->vel[0] = 31000 / gameSprite->duration;
			}
			if (gameSprite->vel[1] > 0) {
				gameSprite->pos[4] -= 31;
				gameSprite->vel[1] = -31000 / gameSprite->duration;
			}
			else if (gameSprite->vel[1] < 0) {
				gameSprite->pos[4] += 31;
				gameSprite->vel[1] = 31000 / gameSprite->duration;
			}
			gameSprite->vel[2] = (gameSprite->pos[5] - gameSprite->pos[2]) * 1000 / gameSprite->duration;
			this->activeSprites++;
		}
	}
	else if ((gameSprite->flags & 0x800) != 0x0) {
		app->render->unlinkSprite(gameSprite->sprite);
	}
}

void Game::snapGameSprites() {
	do {
		this->gsprite_update(0x40000000);
	} while (this->activeSprites != 0);
}

void Game::gsprite_update(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->activeSprites == 0) {
		return;
	}

	bool b = false;
	this->activeSprites = 0;
	this->activePropogators = 0;
	for (int i = 0; i < 48; ++i) {
		GameSprite* gameSprite = &this->gsprites[i];
		if ((gameSprite->flags & 0x1) != 0x0) {
			int n2 = n - gameSprite->time;
			if (n2 < 0) {
				if (0x0 != (gameSprite->flags & 0x2)) {
					this->activePropogators++;
				}
				this->activeSprites++;
			}
			else if ((gameSprite->flags & 0x200) == 0x0 && n2 >= gameSprite->duration) {
				if ((gameSprite->flags & 0x8) == 0x0) {
					app->canvas->invalidateRect();
				}
				this->gsprite_destroy(gameSprite);
			}
			else {
				this->activeSprites++;
				b = true;
				if (0x0 != (gameSprite->flags & 0x40)) {
					app->render->mapSpriteInfo[gameSprite->sprite] = ((app->render->mapSpriteInfo[gameSprite->sprite] & 0xFFFF00FF) | n2 / 200 % gameSprite->numAnimFrames << 8);
				}
				if (0x0 != (gameSprite->flags & 0x1000)) {
					int n3 = app->render->mapSprites[app->render->S_X + gameSprite->sprite] >> 6;
					int n4 = app->render->mapSprites[app->render->S_Y + gameSprite->sprite] >> 6;
					short n5 = (short)(app->canvas->viewX + this->viewStepX);
					short n6 = (short)(app->canvas->viewY + this->viewStepY);
					if (0x0 != (gameSprite->flags & 0x2)) {
						if (n2 > gameSprite->duration) {
							app->render->mapSprites[app->render->S_X + gameSprite->sprite] = (short)(n5 + (this->viewStepX >> 6) * gameSprite->pos[3] + (this->viewRightStepX >> 6) * gameSprite->pos[4]);
							app->render->mapSprites[app->render->S_Y + gameSprite->sprite] = (short)(n6 + (this->viewStepY >> 6) * gameSprite->pos[3] + (this->viewRightStepY >> 6) * gameSprite->pos[4]);
							app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = (short)(app->canvas->viewZ + gameSprite->pos[5]);
							gameSprite->flags &= 0xFFFFFFFD;
						}
						else {
							int n7 = gameSprite->pos[0] + gameSprite->vel[0] * n2 / 1000;
							int n8 = gameSprite->pos[1] + gameSprite->vel[1] * n2 / 1000;
							app->render->mapSprites[app->render->S_X + gameSprite->sprite] = (short)(n5 + (this->viewStepX >> 6) * n7 + (this->viewRightStepX >> 6) * n8);
							app->render->mapSprites[app->render->S_Y + gameSprite->sprite] = (short)(n6 + (this->viewStepY >> 6) * n7 + (this->viewRightStepY >> 6) * n8);
							app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = (short)(app->canvas->viewZ + gameSprite->pos[2] + gameSprite->vel[2] * n2 / 1000);
						}
						this->activePropogators++;
					}
					else {
						app->render->mapSprites[app->render->S_X + gameSprite->sprite] = n5;
						app->render->mapSprites[app->render->S_Y + gameSprite->sprite] = n6;
						app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = (short)app->canvas->viewZ;
					}
					if (0x0 == (gameSprite->flags & 0x4) && (n3 != app->render->mapSprites[app->render->S_X + gameSprite->sprite] >> 6 || n4 != app->render->mapSprites[app->render->S_Y + gameSprite->sprite] >> 6)) {
						if (0x0 != (gameSprite->flags & 0x1000)) {
							app->render->relinkSprite(gameSprite->sprite, app->canvas->destX << 4, app->canvas->destY << 4, app->canvas->destZ << 4);
						}
						else {
							app->render->relinkSprite(gameSprite->sprite);
						}
					}
				}
				else if (0x0 != (gameSprite->flags & 0x2)) {
					this->activePropogators++;
					if (n2 >= gameSprite->duration) {
						app->render->mapSprites[app->render->S_X + gameSprite->sprite] = gameSprite->pos[3];
						app->render->mapSprites[app->render->S_Y + gameSprite->sprite] = gameSprite->pos[4];
						if ((gameSprite->flags & 0x4000) == 0x0) {
							app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = gameSprite->pos[5];
						}
						else if ((gameSprite->flags & 0x8000) == 0x0) {
							app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = gameSprite->pos[2];
						}
						gameSprite->flags &= 0xFFFFFFFD;
					}
					else {
						app->render->mapSprites[app->render->S_X + gameSprite->sprite] = (short)(gameSprite->pos[0] + gameSprite->vel[0] * n2 / 1000);
						app->render->mapSprites[app->render->S_Y + gameSprite->sprite] = (short)(gameSprite->pos[1] + gameSprite->vel[1] * n2 / 1000);
						if ((gameSprite->flags & 0x4000) == 0x0) {
							app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = (short)(gameSprite->pos[2] + gameSprite->vel[2] * n2 / 1000);
						}
						else {
							app->render->mapSprites[app->render->S_Z + gameSprite->sprite] = (short)(gameSprite->pos[2] + ((app->render->sinTable[(n2 << 16) / (gameSprite->duration << 8) << 1 & 0x3FF] >> 8) * (gameSprite->pos[5] - gameSprite->pos[2] << 8) >> 16));
						}
						if (0x0 == (gameSprite->flags & 0x4)) {
							app->render->relinkSprite(gameSprite->sprite);
						}
					}
				}
				if (0x0 != (gameSprite->flags & 0x400)) {
					if (n2 > gameSprite->duration) {
						app->render->mapSprites[app->render->S_SCALEFACTOR + gameSprite->sprite] = gameSprite->destScale;
						gameSprite->flags &= 0xFFFFFBFF;
					}
					else {
						app->render->mapSprites[app->render->S_SCALEFACTOR + gameSprite->sprite] = (uint8_t)(gameSprite->startScale + gameSprite->scaleStep * n2 / 1000);
					}
				}
			}
		}
	}
	if (b) {
		app->canvas->staleView = true;
		app->canvas->invalidateRect();
	}
}

void Game::saveWorldState(OutputStream* OS, bool b) {
	Applet* app = CAppContainer::getInstance()->app;
	const char* name;

	short* mapSprites = app->render->mapSprites;
	int* mapSpriteInfo = app->render->mapSpriteInfo;
	app->canvas->updateLoadingBar(false);
	OS->writeInt(app->render->mapCompileDate);
	int gameTime = app->gameTime;
	this->totalLevelTime += gameTime - this->curLevelTime;
	this->curLevelTime = gameTime;
	OS->writeInt(this->totalLevelTime);
	OS->writeByte(this->mapSecretsFound);
	app->render->snapFogLerp();
	OS->writeInt(app->tinyGL->fogColor);
	OS->writeInt(app->tinyGL->fogMin);
	OS->writeInt(app->tinyGL->fogRange);
	OS->writeInt(app->render->playerFogColor);
	OS->writeInt(app->render->playerFogMin);
	OS->writeInt(app->render->playerFogRange);
	if (app->player->ce->weapon == 14) {
		OS->writeByte((uint8_t)app->player->activeWeaponDef->tileIndex);
	}
	for (int i = 0; i < 4; ++i) {
		if (!b) {
			OS->writeShort(this->placedBombs[i]);
		}
		else {
			OS->writeShort(0);
		}
	}
	this->saveEntityStates(OS, b);
	app->canvas->updateLoadingBar(false);
	int numLines = app->render->numLines;
	int n = 0;
	int n2 = 0;
	for (int j = 0; j < numLines; ++j) {
		if (n2 == 8) {
			OS->writeByte((uint8_t)n);
			n2 = 0;
			n = 0;
		}
		if ((app->render->lineFlags[j >> 1] & 8 << ((j & 0x1) << 2)) != 0x0) {
			n |= 1 << n2;
		}
		++n2;
	}
	if (n2 != 0) {
		OS->writeByte((uint8_t)n);
	}
	int b2 = 0;
	for (int k = 0; k < 64; ++k) {
		if (this->entityDeathFunctions[k * 2] != -1) {
			b2 = (uint8_t)(b2 + 1);
		}
	}
	OS->writeByte(b2);
	for (int l = 0; l < 64; ++l) {
		if (this->entityDeathFunctions[l * 2] != -1) {
			OS->writeShort(this->entityDeathFunctions[l * 2 + 0]);
			OS->writeShort(this->entityDeathFunctions[l * 2 + 1]);
		}
	}
	int numMapSprites = app->render->numMapSprites;
	int n3 = 0;
	while (n3 < numMapSprites) {
		int v = 0;
		for (int n4 = 0; n4 < 8; ++n4, ++n3) {
			int n5 = 0;
			if (b && mapSprites[app->render->S_ENT + n3] != -1) {
				Entity* entity = &this->entities[mapSprites[app->render->S_ENT + n3]];
				EntityDef* def = entity->def;
				if (def->eType == 9 && def->eSubType != 17 && (entity->monster->flags & 0x80) == 0x0) {
					n5 = 1;
				}
			}
			if (n5 == 0 && 0x0 != (app->render->mapSpriteInfo[n3] & 0x10000)) {
				n5 = 1;
			}
			if (n5 != 0) {
				v |= 1 << n4;
			}
			++n4;
			if (0x0 != (mapSpriteInfo[n3] & 0x200000)) {
				v |= 1 << n4;
			}
		}
		OS->writeByte(v);
	}
	for (int n6 = 1024, n7 = 0; n7 < n6; n7 += 8) {
		int v2 = 0;
		for (int n8 = 0; n8 < 8; ++n8) {
			if (0x0 != (app->render->mapFlags[n7 + n8] & 0x80)) {
				v2 |= 1 << n8;
			}
		}
		OS->writeByte(v2);
	}
	short* scriptStateVars = this->scriptStateVars;
	for (int n9 = 0; n9 < 128; ++n9) {
		if (n9 != 15) {
			OS->writeShort(scriptStateVars[n9]);
		}
	}
	int numTileEvents = app->render->numTileEvents;
	int n10 = 0;
	int n11 = 0;
	for (int n12 = 0; n12 < numTileEvents; ++n12) {
		n11 = n12 % 32;
		n10 |= (app->render->tileEvents[n12 * 2 + 1] & 0x80000) >> 19 << n11;
		if (n11 == 31) {
			OS->writeInt(n10);
			n10 = 0;
		}
	}
	if (n11 != 31) {
		OS->writeInt(n10);
	}
	app->canvas->updateLoadingBar(false);
	OS->writeByte(this->numLerpSprites);
	int n13 = 0;
	for (int n14 = 0; n14 < 16; ++n14) {
		LerpSprite lerpSprite = this->lerpSprites[n14];
		if (lerpSprite.hSprite != 0) {
			lerpSprite.saveState(OS);
			++n13;
		}
	}
	OS->writeByte(this->numScriptThreads);
	for (int n15 = 0; n15 < 20; ++n15) {
		this->scriptThreads[n15].saveState(OS);
	}
	OS->writeInt(this->dropIndex);
	OS->writeInt(app->player->moves);
	OS->writeByte((uint8_t)app->player->numNotebookIndexes);
	OS->writeByte(app->player->questComplete);
	OS->writeByte(app->player->questFailed);
	for (int n16 = 0; n16 < 8; ++n16) {
		OS->writeShort(app->player->notebookIndexes[n16]);
		OS->writeShort(app->player->notebookPositions[n16]);
	}
	OS->writeByte(app->render->mapFlagsBitmask);
	OS->writeByte((uint8_t)(app->render->useMastermindHack ? 1 : 0));
	OS->writeByte((uint8_t)(app->render->useCaldexHack ? 1 : 0));
	if (this->watchLine == nullptr) {
		OS->writeShort(-1);
	}
	else {
		OS->writeShort(this->watchLine->getIndex());
	}
	int numMallocsForVIOS = this->numMallocsForVIOS;
	if (this->angryVIOS) {
		numMallocsForVIOS |= 0x40000000;
	}
	OS->writeInt(numMallocsForVIOS);
	OS->writeShort(this->lootFound);
	OS->writeShort(this->destroyedObj);
	app->canvas->updateLoadingBar(false);
}

void Game::loadWorldState() {
	Applet* app = CAppContainer::getInstance()->app;
	const char* name;
	InputStream IS;

	if (app->canvas->loadMapID > 10) {
		return;
	}

	if (this->activeLoadType == 1) {
		name = this->GetSaveFile(Game::FILE_NAME_FULLWORLD, 0);
	}
	else {
		name = this->GetSaveFile(Game::FILE_NAME_BRIEFWORLD, app->canvas->loadMapID - 1);
	}

	if (!IS.loadFile(name, LT_FILE) || (this->loadWorldState(&IS) == 0)) {
		this->totalLevelTime = 0;
	}
}

bool Game::loadWorldState(InputStream* IS) {
	Applet* app = CAppContainer::getInstance()->app;

	int mapCompileDate = IS->readInt();
	if (app->render->mapCompileDate == mapCompileDate) {
		this->totalLevelTime = IS->readInt();
		this->mapSecretsFound = IS->readByte();
		app->tinyGL->fogColor = IS->readInt();
		app->tinyGL->fogMin = IS->readInt();
		app->tinyGL->fogRange = IS->readInt();
		app->render->playerFogColor = IS->readInt();
		app->render->playerFogMin = IS->readInt();
		app->render->playerFogRange = IS->readInt();

		app->canvas->updateLoadingBar(false);
		app->render->buildFogTables(app->tinyGL->fogColor);

		app->canvas->updateLoadingBar(false);
		if (app->player->ce->weapon == 14) {
			app->player->setPickUpWeapon(IS->readByte() & 0xFF);
		}
		for (int i = 0; i < 4; ++i) {
			this->placedBombs[i] = IS->readShort();
		}
		this->loadEntityStates(IS);

		app->canvas->updateLoadingBar(false);
		int numLines = app->render->numLines;
		uint8_t byte1 = 0;
		int n = 8;
		for (int j = 0; j < numLines; ++j) {
			if (n == 8) {
				byte1 = IS->readByte();
				n = 0;
			}
			if ((byte1 & 1 << n) != 0x0) {
				app->render->lineFlags[j >> 1] |= (uint8_t)(8 << ((j & 0x1) << 2));
			}
			++n;
		}
		for (uint8_t byte2 = IS->readByte(), b = 0; b < byte2; ++b) {
			this->entityDeathFunctions[b * 2 + 0] = IS->readShort();
			this->entityDeathFunctions[b * 2 + 1] = IS->readShort();
		}

		app->canvas->updateLoadingBar(false);
		int numMapSprites = app->render->numMapSprites;
		int k = 0;
		while (k < numMapSprites) {
			uint8_t byte3 = IS->readByte();
			for (int l = 0; l < 8; ++l, ++k) {
				if ((byte3 & 1 << l) != 0x0) {
					app->render->mapSpriteInfo[k] |= 0x10000;
				}
				else {
					app->render->mapSpriteInfo[k] &= ~0x10000;
				}
				++l;
				if ((byte3 & 1 << l) != 0x0) {
					app->render->mapSpriteInfo[k] |= 0x200000;
				}
				else {
					app->render->mapSpriteInfo[k] &= ~0x200000;
				}
			}
		}

		app->canvas->updateLoadingBar(false);
		for (int n7 = 1024, n8 = 0; n8 < n7; n8 += 8) {
			uint8_t byte4 = IS->readByte();
			for (int n9 = 0; n9 < 8; ++n9) {
				if ((byte4 & 1 << n9) != 0x0) {
					app->render->mapFlags[n8 + n9] |= (uint8_t)0x80;
				}
			}
		}
		short* scriptStateVars = this->scriptStateVars;
		for (int n11 = 0; n11 < 128; ++n11) {
			if (n11 != 15) {
				scriptStateVars[n11] = IS->readShort();
			}
		}

		app->canvas->updateLoadingBar(false);
		int numTileEvents = app->render->numTileEvents;
		int n12 = IS->readInt();
		for (int n13 = 0; n13 < numTileEvents; ++n13) {
			int n14 = n13 % 32;
			app->render->tileEvents[n13 * 2 + 1] = ((app->render->tileEvents[n13 * 2 + 1] & 0xFFF7FFFF) | (n12 >> n14 & 0x1) << 19);
			if (n14 == 31 && n13 < numTileEvents - 1) {
				n12 = IS->readInt();
			}
		}

		this->numLerpSprites = IS->readByte();
		for (int n15 = 0; n15 < 16; ++n15) {
			LerpSprite* lerpSprite = &this->lerpSprites[n15];
			if (n15 < this->numLerpSprites) {
				lerpSprite->loadState(IS);
			}
			else {
				lerpSprite->hSprite = 0;
			}
		}

		this->numScriptThreads = IS->readByte();
		for (int n16 = 0; n16 < 20; ++n16) {
			ScriptThread* scriptThread = &this->scriptThreads[n16];
			scriptThread->loadState(IS);
			if (scriptThread->stackPtr > 0) {
				scriptThread->inuse = true;
			}
			else {
				scriptThread->inuse = false;
			}
		}

		this->updateLerpSprites();

		this->dropIndex = IS->readInt();
		app->player->moves = IS->readInt();
		app->player->numNotebookIndexes = IS->readByte();
		app->player->questComplete = IS->readByte();
		app->player->questFailed = IS->readByte();
		for (int n17 = 0; n17 < 8; ++n17) {
			app->player->notebookIndexes[n17] = IS->readShort();
			app->player->notebookPositions[n17] = IS->readShort();
		}
		app->render->mapFlagsBitmask = IS->readByte();
		app->render->useMastermindHack = (IS->readByte() == 1);
		app->render->useCaldexHack = (IS->readByte() == 1);
		short short1 = IS->readShort();
		if (short1 != -1) {
			this->watchLine = &this->entities[short1];
		}
		int numMallocsForVIOS = IS->readInt();
		this->numMallocsForVIOS = (numMallocsForVIOS & 0x3fffffff);
		this->angryVIOS = (numMallocsForVIOS & 0x40000000) ? true : false;
		this->lootFound = IS->readShort();
		this->destroyedObj = IS->readShort();
		app->hud->playerDestHealth = 0;
		app->hud->playerStartHealth = 0;
		app->hud->playerHealthChangeTime = 0;
		this->prepareMonsters();
		return true;
	}
	else
	{
		if (mapCompileDate) {
			this->spawnParam = 0;
		}
		this->totalLevelTime = 0;
		return false;
	}
}

void Game::saveConfig() {
	Applet* app = CAppContainer::getInstance()->app;
	SDLGL* sdlGL = CAppContainer::getInstance()->sdlGL;
	OutputStream OS;
	const char* name = this->GetSaveFile(Game::FILE_NAME_CONFIG, 0);
	
	if (OS.openFile(name, 1)) {
		OS.writeInt(11);
		OS.writeByte(this->difficulty);
		OS.writeBoolean(app->sound->allowSounds);
		OS.writeByte((uint8_t)app->sound->soundFxVolume);
		OS.writeByte((uint8_t)app->sound->musicVolume);
		OS.writeBoolean(app->canvas->vibrateEnabled);
		OS.writeBoolean(app->player->enableHelp);
		OS.writeInt(app->localization->defaultLanguage);
		OS.writeInt(app->player->currentLevelDeaths);
		OS.writeInt(app->player->totalDeaths);
		OS.writeInt(app->canvas->animFrames);
		OS.writeInt(app->canvas->m_controlLayout);
		OS.writeInt(app->canvas->m_controlAlpha);
		OS.writeBoolean(app->canvas->isFlipControls);
		OS.writeInt(app->player->helpBitmask);
		OS.writeInt(app->player->invHelpBitmask);
		OS.writeInt(app->player->ammoHelpBitmask);
		OS.writeInt(app->player->weaponHelpBitmask);
		OS.writeInt(app->player->armorHelpBitmask);
		for (int i = 0; i < 5; ++i) {
			OS.writeInt(app->menuSystem->indexes[i * 2]);
			OS.writeInt(app->menuSystem->indexes[i * 2 + 1]);
		}
		OS.writeBoolean(app->canvas->recentBriefSave);
		OS.writeBoolean(this->hasSeenIntro);
		for (int j = 0; j < 10; ++j) {
			OS.writeShort(this->numLevelLoads[j]);
		}
		OS.writeInt(this->totalPlayTime += (app->upTimeMs - this->lastSaveTime) / 1000);
		OS.writeInt(app->player->currentGrades);
		OS.writeInt(app->player->bestGrades);
		this->lastSaveTime = app->upTimeMs;

		// [GEC] Port Configurations
		OS.writeInt(sdlGL->windowMode);
		OS.writeBoolean(sdlGL->vSync);
		OS.writeInt(sdlGL->resolutionIndex);
		OS.writeInt(gVibrationIntensity);
		OS.writeInt(gDeadZone);
		OS.writeBoolean(_glesObj->isInit);

		for (int i = 0; i < KEY_MAPPIN_MAX; i++) {
			for (int j = 0; j < KEYBINDS_MAX; j++) {
				OS.writeInt(keyMapping[i].keyBinds[j]);
			}
		}

		OS.writeInt(0xDEADBEEF);
		OS.close();
	}
	else {
		app->Error("I/O Error in saveConfig"); // ERR_SAVECONFIG
	}
	OS.~OutputStream();
}

void Game::loadConfig() {
	Applet* app = CAppContainer::getInstance()->app;
	SDLGL* sdlGL = CAppContainer::getInstance()->sdlGL;
	InputStream IS;
	const char* name = this->GetSaveFile(Game::FILE_NAME_CONFIG, 0);

	if (IS.loadFile(name, LT_FILE)) {
		if (IS.readInt() == 11) {
			this->difficulty = IS.readByte();
			app->sound->allowSounds = IS.readBoolean();
			app->canvas->areSoundsAllowed = app->sound->allowSounds;
			app->sound->soundFxVolume = IS.readByte();
			app->sound->musicVolume = IS.readByte();
			app->canvas->vibrateEnabled = IS.readBoolean();
			app->player->enableHelp = IS.readBoolean();
			int l = IS.readInt();
			if (l != app->localization->defaultLanguage) {
				app->localization->setLanguage(l);
			}
			app->player->currentLevelDeaths = IS.readInt();
			app->player->totalDeaths = IS.readInt();
			int animFrames = IS.readInt();
			if (animFrames < 2) {
				animFrames = 2;
			}
			else if (animFrames > 64) {
				animFrames = 64;
			}
			app->canvas->setAnimFrames(animFrames);
			app->canvas->m_controlLayout = IS.readInt();
			if (app->canvas->m_controlLayout > 2) {
				app->canvas->m_controlLayout = 0;
			}
			app->canvas->m_controlAlpha = IS.readInt();
			app->canvas->isFlipControls = IS.readBoolean();
			app->player->helpBitmask = IS.readInt();
			app->player->invHelpBitmask = IS.readInt();
			app->player->ammoHelpBitmask = IS.readInt();
			app->player->weaponHelpBitmask = IS.readInt();
			app->player->armorHelpBitmask = IS.readInt();
			for (int i = 0; i < 5; ++i) {
				app->menuSystem->indexes[i * 2] = IS.readInt();
				app->menuSystem->indexes[i * 2 + 1] = IS.readInt();
			}
			app->canvas->recentBriefSave = IS.readBoolean();
			this->hasSeenIntro = IS.readBoolean();
			for (int j = 0; j < 10; ++j) {
				this->numLevelLoads[j] = IS.readShort();
			}
			this->totalPlayTime = IS.readInt();
			app->player->currentGrades = IS.readInt();
			app->player->bestGrades = IS.readInt();
			this->lastSaveTime = app->upTimeMs;

			// [GEC] Port Configurations
			sdlGL->windowMode = IS.readInt();
			sdlGL->vSync = IS.readBoolean();
			sdlGL->resolutionIndex = IS.readInt();
			gVibrationIntensity = IS.readInt();
			gDeadZone = IS.readInt();
			_glesObj->isInit = IS.readBoolean();

			for (int i = 0; i < KEY_MAPPIN_MAX; i++) {
				for (int j = 0; j < KEYBINDS_MAX; j++) {
					keyMapping[i].keyBinds[j] = IS.readInt();
				}
			}

			SDL_memcpy(keyMappingTemp, keyMapping, sizeof(keyMapping));

			if (IS.readInt() != 0xDEADBEEF) {
				app->Error("Failed marker check in loadConfig()");
			}
			IS.close();
			app->sound->updateVolume();
		}
		else {
			IS.close();
		}
	}
	IS.~InputStream();
}

void Game::saveState(int lastMapID, int loadMapID, int viewX, int viewY, int viewAngle, int viewPitch, int prevX, int prevY, int saveX, int saveY, int saveZ, int saveAngle, int savePitch, int saveType) {
	Applet* app = CAppContainer::getInstance()->app;
	const char* name;
	OutputStream OS;

	app->canvas->recentBriefSave = ((saveType & 0x20) != 0x0);
	bool briefSave = app->canvas->recentBriefSave;
	app->canvas->freeRuntimeData();
	app->canvas->updateLoadingBar(false);

	if ((saveType & 0x10) != 0x0) {
		app->player->levelGrade(true);
	}

	this->saveConfig();

	if ((saveType & 0x1) != 0x0) {
		app->canvas->updateLoadingBar(false);
		if (briefSave) {
			name = this->GetSaveFile(Game::FILE_NAME_BRIEFPLAYER, 0);
		}
		else {
			name = this->GetSaveFile(Game::FILE_NAME_FULLPLAYER, 0);
		}

		if (OS.openFile(name, 1)) {
			if (!this->savePlayerState(&OS, loadMapID, viewX, viewY, viewAngle, viewPitch, prevX, prevY, saveX, saveY, saveZ, saveAngle, savePitch)) {
				OS.~OutputStream();
				OS.close();
				return;
			}
			OS.close();
		}
		else {
			app->Error("saveState: failed to open player file");
		}
	}

	if ((saveType & 0x2) != 0x0) {
		app->canvas->updateLoadingBar(false);
		if (briefSave) {
			name = this->GetSaveFile(Game::FILE_NAME_BRIEFWORLD, (lastMapID - 1));
		}
		else {
			name = this->GetSaveFile(Game::FILE_NAME_FULLWORLD, 0);
		}
		if (OS.openFile(name, 1)) {
			this->saveWorldState(&OS, briefSave);
			OS.close();
		}
		else {
			app->Error("saveState: failed to open world file");
		}
	}

	app->canvas->loadRuntimeData();

	OS.~OutputStream();
}

void Game::saveLevelSnapshot() {
	Applet* app = CAppContainer::getInstance()->app;
	OutputStream OS;

	const char* name = this->GetSaveFile(Game::FILE_NAME_BRIEFPLAYER, 0);
	if (OS.openFile(name, 1))
	{
		app->canvas->updateLoadingBar(false);
		if (this->savePlayerState(
			&OS,
			app->canvas->loadMapID,
			app->canvas->viewX,
			app->canvas->viewY,
			app->canvas->viewAngle,
			app->canvas->viewPitch,
			app->canvas->viewX,
			app->canvas->viewY,
			app->canvas->viewX,
			app->canvas->viewY,
			app->canvas->viewZ,
			app->canvas->saveAngle,
			app->canvas->savePitch))
		{
			OS.close();
			const char* name = this->GetSaveFile(Game::FILE_NAME_BRIEFWORLD, app->canvas->loadMapID - 1);
			if (OS.openFile(name, 1)) {
				app->canvas->updateLoadingBar(false);
				this->saveWorldState(&OS, true);
				OS.close();
				app->canvas->updateLoadingBar(false);
			}
		}
	}
	OS.~OutputStream();
}

bool Game::savePlayerState(OutputStream* OS, int loadMapID, int viewX, int viewY, int viewAngle, int viewPitch, int prevX, int prevY, int saveX, int saveY, int saveZ, int saveAngle, int savePitch) {
	Applet* app = CAppContainer::getInstance()->app;
	OS->writeShort(loadMapID);
	OS->writeShort(viewX);
	OS->writeShort(viewY);
	OS->writeShort(viewAngle);
	OS->writeShort(viewPitch);
	OS->writeInt(prevX);
	OS->writeInt(prevY);
	OS->writeInt(saveX);
	OS->writeInt(saveY);
	OS->writeInt(saveZ);
	OS->writeShort(saveAngle);
	OS->writeShort(savePitch);
	return app->player->saveState(OS);
}

bool Game::loadPlayerState(InputStream* IS) {
	Applet* app = CAppContainer::getInstance()->app;
	this->saveStateMap = IS->readShort();
	app->canvas->viewX = (app->canvas->destX = IS->readShort());
	app->canvas->viewY = (app->canvas->destY = IS->readShort());
	app->canvas->viewAngle = (app->canvas->destAngle = IS->readShort());
	app->canvas->viewPitch = (app->canvas->destPitch = IS->readShort());
	app->canvas->viewZ = (app->canvas->destZ = 36);
	app->canvas->prevX = IS->readInt();
	app->canvas->prevY = IS->readInt();
	app->canvas->saveX = IS->readInt();
	app->canvas->saveY = IS->readInt();
	app->canvas->saveZ = IS->readInt();
	app->canvas->saveAngle = IS->readShort();
	app->canvas->savePitch = IS->readShort();
	this->spawnParam = 0;
	return app->player->loadState(IS);
}

bool Game::loadState(int activeLoadType) {
	Applet* app = CAppContainer::getInstance()->app;
	const char* name;
	InputStream IS;
	bool rtn = false;

	if (activeLoadType == 1) {
		name = this->GetSaveFile(Game::FILE_NAME_FULLPLAYER, 0);
	}
	else if (activeLoadType == 2) {
		app->player->currentLevelDeaths = 0;
		name = this->GetSaveFile(Game::FILE_NAME_BRIEFPLAYER, 0);
	}

	if (IS.loadFile(name, LT_FILE)) {
		if (this->loadPlayerState(&IS)) {
			this->activeLoadType = activeLoadType;
			if (app->canvas->viewX != 0 && app->canvas->viewY != 0) {
				this->spawnParam = ((app->canvas->viewX >> 6 & 0x1F) | (app->canvas->viewY >> 6 & 0x1F) << 5 | (app->canvas->viewAngle & 0x3FF) >> 7 << 10);
			}
			app->canvas->loadMap(this->saveStateMap, false, false);
			this->isSaved = false;
			this->isLoaded = true;
			rtn = true;
		}
	}
	else {
		app->Error("loadState: failed to open player file");
	}

	IS.~InputStream();
	return rtn;
}

bool Game::hasConfig() {
	InputStream IS;
	const char* name = this->GetSaveFile(Game::FILE_NAME_CONFIG, 0);
	if (IS.loadFile(name, LT_FILE)) {
		if (IS.readInt() == 11) {
			return true;
		}
	}
	IS.~InputStream();
	return false;
}

bool Game::hasElement(const char* name){
	if (name != nullptr) {
		char* namePath = this->getProfileSaveFileName(name);
		FILE* file = std::fopen(namePath, "rb");
		if (namePath) {delete[] namePath;}
		if (file != nullptr) {
			std::fclose(file);
			return true;
		}
	}
	return false;
}

bool Game::hasSavedState() {
	if (this->hasConfig()) {
		bool v3 = false;
		for (int i = 0; i < 10; i++)
		{
			if (this->hasElement(this->GetSaveFile(Game::FILE_NAME_BRIEFWORLD, i))) {
				v3 = true;
			}
		}
		if (this->hasElement(this->GetSaveFile(Game::FILE_NAME_BRIEFPLAYER, 0))) {
			if (this->hasElement(this->GetSaveFile(Game::FILE_NAME_FULLPLAYER, 0))){
				if (v3) {
					return true;
				}
			}
		}
	}

	return false;
}

void Game::removeState(bool b) {
	Applet* app = CAppContainer::getInstance()->app;
	char* namePath;

	if (b) {
		app->canvas->updateLoadingBar(false);
	}

	this->saveEmptyConfig();

	if (b) {
		app->canvas->updateLoadingBar(false);
	}

	namePath = this->getProfileSaveFileName(this->GetSaveFile(Game::FILE_NAME_FULLWORLD, 0));
	std::remove(namePath);
	if (namePath) { delete[] namePath; }

	if (b) {
		app->canvas->updateLoadingBar(false);
	}

	namePath = this->getProfileSaveFileName(this->GetSaveFile(Game::FILE_NAME_FULLPLAYER, 0));
	std::remove(namePath);
	if (namePath) { delete[] namePath; }

	if (b) {
		app->canvas->updateLoadingBar(false);
	}

	namePath = this->getProfileSaveFileName(this->GetSaveFile(Game::FILE_NAME_BRIEFPLAYER, 0));
	std::remove(namePath);
	if (namePath) { delete[] namePath; }

	if (b) {
		app->canvas->updateLoadingBar(false);
	}
	for (int i = 0; i < 10; i++) {
		namePath = this->getProfileSaveFileName(this->GetSaveFile(Game::FILE_NAME_BRIEFWORLD, i));
		std::remove(namePath);
		if (namePath) { delete[] namePath; }
		if (b) {
			app->canvas->updateLoadingBar(false);
		}
	}
}

void Game::saveEmptyConfig() {
	Applet* app = CAppContainer::getInstance()->app;
	SDLGL* sdlGL = CAppContainer::getInstance()->sdlGL;
	OutputStream OS;
	const char* name = this->GetSaveFile(Game::FILE_NAME_CONFIG, 0);

	if (OS.openFile(name, 1)) {
		OS.writeInt(11);
		OS.writeByte(1);
		OS.writeBoolean(app->sound->allowSounds);
		OS.writeByte((uint8_t)app->sound->soundFxVolume);
		OS.writeByte((uint8_t)app->sound->musicVolume);
		OS.writeBoolean(app->canvas->vibrateEnabled);
		OS.writeBoolean(app->player->enableHelp);
		OS.writeInt(app->localization->defaultLanguage);
		OS.writeInt(0);
		OS.writeInt(0);
		OS.writeInt(app->canvas->animFrames);
		OS.writeInt(app->canvas->m_controlLayout);
		OS.writeInt(app->canvas->m_controlAlpha);
		OS.writeBoolean(app->canvas->isFlipControls);
		OS.writeInt(0);
		OS.writeInt(0);
		OS.writeInt(0);
		OS.writeInt(0);
		OS.writeInt(0);
		for (int i = 0; i < 5; ++i) {
			OS.writeInt(0);
			OS.writeInt(0);
		}
		OS.writeBoolean(false);
		OS.writeBoolean(this->hasSeenIntro);

		for (int j = 0; j < 10; ++j) {
			OS.writeShort(this->numLevelLoads[j]);
		}

		OS.writeInt(this->totalPlayTime += (app->upTimeMs - this->lastSaveTime) / 1000);
		OS.writeInt(0);
		OS.writeInt(0);
		this->lastSaveTime = app->upTimeMs;

		// [GEC] Port Configurations
		OS.writeInt(sdlGL->windowMode);
		OS.writeBoolean(sdlGL->vSync);
		OS.writeInt(sdlGL->resolutionIndex);

		OS.writeInt(0xDEADBEEF);
		OS.close();
	}
	else {
		app->Error("I/O Error in saveConfig");
	}
	OS.~OutputStream();
}

bool Game::canSnapMonsters() {
	if (this->disableAI) {
		return true;
	}
	if (this->activeMonsters != nullptr) {
		Entity* entity = this->activeMonsters;
		while (0x0 == (entity->info & 0x10000000)) {
			entity = entity->monster->nextOnList;
			if (entity == this->activeMonsters) {
				return true;
			}
		}
		return false;
	}
	return true;
}

bool Game::snapMonsters(bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->animatingEffects != 0) {
		for (int i = 0; i < 16; ++i) {
			LerpSprite* lerpSprite = &this->lerpSprites[i];
			if (lerpSprite->hSprite != 0) {
				if ((lerpSprite->flags & Enums::LS_FLAG_ANIMATING_EFFECT) != 0x0) {
					short n = app->render->mapSprites[app->render->S_ENT + (lerpSprite->hSprite - 1)];
					if (n == -1) {
						return false;
					}
					if (this->entities[n].def->eType != 2) {
						return false;
					}
				}
			}
		}
	}
	bool b2 = b;
	if (this->monstersTurn == 0) {
		if (!b) {
			b2 = this->canSnapMonsters();
		}
		Entity* activeMonsters = this->activeMonsters;
		if (b2 && activeMonsters != nullptr) {
			do {
				Entity* nextOnList = activeMonsters->monster->nextOnList;
				this->snapLerpSprites(activeMonsters->getSprite());
				activeMonsters = nextOnList;
			} while (activeMonsters != this->activeMonsters);
			this->interpolatingMonsters = false;
		}
		else if (activeMonsters == nullptr) {
			this->interpolatingMonsters = false;
		}
		app->canvas->updateFacingEntity = true;
		app->canvas->invalidateRect();
		return b2;
	}
	this->monsterAI();
	if (!b) {
		b2 = this->canSnapMonsters();
	}
	if (b2 && this->activeMonsters != nullptr) {
		Entity* entity = this->activeMonsters;
		do {
			this->snapLerpSprites(entity->getSprite());
			entity = entity->monster->nextOnList;
		} while (entity != this->activeMonsters);
		app->canvas->updateFacingEntity = true;
		app->canvas->invalidateRect();
	}
	this->monsterLerp();
	if (this->combatMonsters != nullptr && !this->interpolatingMonsters) {
		app->combat->performAttack(this->combatMonsters, this->combatMonsters->monster->target, 0, 0, false);
		return false;
	}
	return b2;
}

void Game::endMonstersTurn() {
	Applet* app = CAppContainer::getInstance()->app;
	app->canvas->startRotation(true);
	this->monstersTurn = 0;
}

void Game::updateMonsters() {
	Applet* app = CAppContainer::getInstance()->app;

	this->monsterAI();
	this->monsterLerp();
	if (!this->interpolatingMonsters) {
		app->canvas->updateFacingEntity = true;
		if (this->combatMonsters != nullptr) {
			app->combat->performAttack(this->combatMonsters, this->combatMonsters->monster->target, 0, 0, false);
		}
		else {
			this->endMonstersTurn();
			if (!this->isCameraActive() && app->canvas->blockInputTime == 0) {
				app->canvas->drawPlayingSoftKeys();
			}
		}
	}
}

void Game::setLineLocked(Entity* entity, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	int sprite = entity->getSprite();
	int n = app->render->mapSpriteInfo[sprite] & 0xFF;
	int n2;
	if (b) {
		n2 = (n & 0xFFFFFFFE);
	}
	else {
		n2 = (n | 0x1);
	}
	app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFFFF00) | n2);
	n2 += 257;
	if (entity->def->name == (entity->name & 0x3FF)) {
		entity->def = app->entityDefManager->lookup(n2);
		entity->name = (short)(entity->def->name | 0x400);
	}
	else {
		entity->def = app->entityDefManager->lookup(n2);
	}
}

void Game::snapAllMovers() {
	while (this->numLerpSprites > 0) {
		this->snapGameSprites();
		this->snapLerpSprites(-1);
	}
}

void Game::skipCinematic() {
	Applet* app = CAppContainer::getInstance()->app;

	bool allowSounds = app->sound->allowSounds;
	app->sound->allowSounds = false;
	app->sound->soundStop();
	this->skippingCinematic = true;
	while (this->skippingCinematic) {
		this->snapGameSprites();
		this->snapLerpSprites(-1);
		if (this->activeCameraKey != -1) {
			this->activeCamera->Snap(this->activeCameraKey);
		}
		if (this->skippingCinematic && nullptr == this->activeCamera->keyThread) {
			this->activeCamera->cameraThread->attemptResume(app->gameTime + 0x40000000);
		}
	}
	app->hud->subTitleID = -1;
	app->hud->cinTitleID = -1;
	this->snapGameSprites();
	this->snapLerpSprites(-1);
	this->cinematicWeapon = -1;
	app->canvas->shakeTime = 0;
	if (!app->render->isFading() || (app->render->getFadeFlags() & 0xC) == 0x0) {
		app->render->startFade(500, 2);
	}
	app->canvas->updateFacingEntity = true;
	app->sound->allowSounds = allowSounds;
	app->canvas->shakeTime = 0;
	app->canvas->blockInputTime = 0;
	app->particleSystem->freeAllParticles();
	if (app->canvas->state == Canvas::ST_DIALOG && this->isCameraActive()) {
		this->activeCameraTime = 65536;
	}
	if (app->canvas->state == Canvas::ST_PLAYING) {
		app->canvas->drawPlayingSoftKeys();
	}
}

int Game::getNextBombIndex() {
	for (int i = 0; i < 4; ++i) {
		if (this->placedBombs[i] == 0) {
			return i;
		}
	}
	return -1;
}

void Game::updateBombs() {
	for (int i = 0; i < 4; ++i) {
		if (this->placedBombs[i] != 0) {
			Entity* entity = &this->entities[placedBombs[i]];
			int n = entity->param & 0xFF;
			if (--n < 0) {
				if ((entity->info & 0x100000) != 0x0) {
					entity->pain(1, nullptr);
				}
				this->placedBombs[i] = 0;
			}
			else {
				entity->param = ((entity->param & 0xFFFFFF00) | n);
			}
		}
	}
}

int Game::setDynamite(int x, int y, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	int nextBombIndex = this->getNextBombIndex();
	if (nextBombIndex == -1) {
		app->Error("Too many bombs placed");
	}
	int flags = 0;
	if (!b) {
		switch (this->destAngle & 0x3FF) {
		case Enums::ANGLE_EAST: {
			flags |= Enums::SPRITE_FLAG_WEST;
			x--;
			break;
		}
		case Enums::ANGLE_NORTH: {
			flags |= Enums::SPRITE_FLAG_SOUTH;
			y++;
			break;
		}
		case Enums::ANGLE_WEST: {
			flags |= Enums::SPRITE_FLAG_EAST;
			x++;
			break;
		}
		case Enums::ANGLE_SOUTH: {
			flags |= Enums::SPRITE_FLAG_NORTH;
			y--;
			break;
		}
		}
	}
	else {
		flags |= (Enums::SPRITE_FLAG_TWO_SIDED | Enums::SPRITE_FLAG_HIDDEN);
	}
	this->allocDynamite((short)x, (short)y, (short)(app->render->getHeight(x, y) + (b ? 31 : 32)), flags, nextBombIndex, 0);
	return nextBombIndex;
}

Entity* Game::getFreeDropEnt() {
	int dropIndex = this->dropIndex;
	int lastDropEntIndex = this->firstDropIndex + dropIndex;
	if ((this->entities[lastDropEntIndex].info & 0x100000) != 0x0) {
		for (dropIndex = (dropIndex + 1) % 16; dropIndex != this->dropIndex && (this->entities[lastDropEntIndex].info & 0x100000) != 0x0; dropIndex = (dropIndex + 1) % 16, lastDropEntIndex = this->firstDropIndex + dropIndex) {}
		if (this->dropIndex == dropIndex) {}
	}
	this->dropIndex = (dropIndex + 1) % 16;
	this->lastDropEntIndex = lastDropEntIndex;
	return &this->entities[lastDropEntIndex];
}

void Game::allocDynamite(int n, int n2, int n3, int n4, int n5, int n6) {
	Applet* app = CAppContainer::getInstance()->app;

	Entity* freeDropEnt = this->getFreeDropEnt();
	freeDropEnt->def = app->entityDefManager->find(14, 6);
	freeDropEnt->name = (short)(freeDropEnt->def->name | 0x400);
	freeDropEnt->param = (n6 << 8 | 0x3);
	if (0x0 != (freeDropEnt->info & 0x100000)) {
		this->unlinkEntity(freeDropEnt);
	}
	this->linkEntity(freeDropEnt, n >> 6, n2 >> 6);
	freeDropEnt->info |= 0x420000;
	int sprite = freeDropEnt->getSprite();
	app->render->mapSprites[app->render->S_X + sprite] = (short)n;
	app->render->mapSprites[app->render->S_Y + sprite] = (short)n2;
	app->render->mapSprites[app->render->S_Z + sprite] = (short)n3;
	app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = 32;
	app->render->mapSprites[app->render->S_RENDERMODE + sprite] = 0;
	app->render->mapSpriteInfo[sprite] = (n4 | freeDropEnt->def->tileIndex);
	app->render->relinkSprite(sprite);
	this->placedBombs[n5] = this->lastDropEntIndex;
}

Entity* Game::spawnDropItem(int n, int n2, int n3, EntityDef* def, int param, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	Entity* freeDropEnt = this->getFreeDropEnt();
	freeDropEnt->def = def;
	freeDropEnt->name = (short)(freeDropEnt->def->name | 0x400);
	if (n3 >= 1 && n3 < 14) {
		n3 |= 0x200;
	}
	if (0x0 != (freeDropEnt->info & 0x100000)) {
		this->unlinkEntity(freeDropEnt);
	}
	freeDropEnt->info &= 0xFFFF;
	freeDropEnt->info |= 0x400000;
	if (freeDropEnt->def->eType == 10) {
		freeDropEnt->info |= 0x20000;
	}
	int sprite = freeDropEnt->getSprite();
	int height = app->render->getHeight(n, n2);
	app->render->mapSpriteInfo[sprite] = n3;
	app->render->mapSprites[app->render->S_X + sprite] = (short)n;
	app->render->mapSprites[app->render->S_Y + sprite] = (short)n2;
	app->render->mapSprites[app->render->S_Z + sprite] = (short)(32 + height);
	app->render->mapSprites[app->render->S_ENT + sprite] = (short)this->lastDropEntIndex;
	app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = 64;
	freeDropEnt->param = param;
	app->render->relinkSprite(sprite);
	this->linkEntity(freeDropEnt, n >> 6, n2 >> 6);
	if (b) {
		this->throwDropItem(n, n2, height, freeDropEnt);
	}
	return freeDropEnt;
}

Entity* Game::spawnDropItem(int n, int n2, int n3, int n4, int n5, int n6, int n7, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	EntityDef* find = app->entityDefManager->find(n4, n5, n6);
	if (find != nullptr) {
		return spawnDropItem(n, n2, n3, find, n7, b);
	}
	app->Error("Cannot find a def for the spawnDropItem.");
	return nullptr;
}

void Game::spawnDropItem(Entity* entity) {
	Applet* app = CAppContainer::getInstance()->app;

	bool b = entity->lootSet != nullptr;
	bool b2;
	if (entity->monster == nullptr) {
		b2 = (b & entity->param == 0);
	}
	else {
		b2 = (b & (entity->monster->flags & 0x800) == 0x0);
	}
	if (b2) {
		int n = entity->lootSet[0];
		int n2 = n >> 12 & 0xF;
		if (n2 != 6) {
			int n3 = (n & 0xFC0) >> 6;
			int n4 = n & 0x3F;
			if (n4 > 0) {
				EntityDef* find = app->entityDefManager->find(6, n2, n3);
				if (find != nullptr) {
					if (find->tileIndex != 0) {
						int sprite = entity->getSprite();
						this->spawnDropItem(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], find->tileIndex, find, n4, false);
					}
				}
				else {
					app->Error("Cannot find a def for the dropItem.", 117); // ERR_ENT_LOOTSET
				}
			}
		}
	}
}

Entity* Game::spawnPlayerEntityCopy(int n, int n2) {
	Applet* app = CAppContainer::getInstance()->app;

	int n3 = 72;
	int n4 = 224;
	switch (app->player->characterChoice) {
	case 1: {
		n3 = 68;
		n4 = 224;
		break;
	}
	case 3: {
		n3 = 66;
		n4 = 225;
		break;
	}
	case 2: {
		n3 = 72;
		n4 = 226;
		break;
	}
	}
	Entity* freeDropEnt = this->getFreeDropEnt();
	freeDropEnt->def = app->entityDefManager->lookup(n3);
	freeDropEnt->name = (short)(n4 | 0x0);
	if (0x0 != (freeDropEnt->info & 0x100000)) {
		this->unlinkEntity(freeDropEnt);
	}
	freeDropEnt->info &= 0xFFFF;
	freeDropEnt->info |= 0x400000;
	int sprite = freeDropEnt->getSprite();
	int height = app->render->getHeight(n, n2);
	app->render->mapSpriteInfo[sprite] = n3;
	app->render->mapSprites[app->render->S_X + sprite] = (short)n;
	app->render->mapSprites[app->render->S_Y + sprite] = (short)n2;
	app->render->mapSprites[app->render->S_Z + sprite] = (short)(32 + height);
	app->render->mapSprites[app->render->S_ENT + sprite] = (short)this->lastDropEntIndex;
	app->render->mapSprites[app->render->S_RENDERMODE + sprite] = 0;
	app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = 64;
	freeDropEnt->param = 0;
	app->render->relinkSprite(sprite);
	this->linkEntity(freeDropEnt, n >> 6, n2 >> 6);
	return freeDropEnt;
}

Entity* Game::spawnSentryBotCorpse(int n, int n2, int n3, int n4, int n5) {
	Applet* app = CAppContainer::getInstance()->app;

	int n6 = 19;
	int n7 = 0;
	switch (n3) {
	case 3:
	case 4: {
		n6 = 19;
		n7 = 0;
		break;
	}
	case 5:
	case 6: {
		n6 = 18;
		n7 = 1;
		break;
	}
	}
	Entity* freeDropEnt = this->getFreeDropEnt();
	freeDropEnt->def = app->entityDefManager->find(9, 11, n7);
	freeDropEnt->name = (short)(freeDropEnt->def->name | 0x400);
	freeDropEnt->populateDefaultLootSet();
	freeDropEnt->param = 0;
	if (n3 == 3 || n3 == 5) {
		freeDropEnt->addToLootSet(2, 1, 10);
	}
	else {
		freeDropEnt->addToLootSet(0, 12, 1);
	}
	freeDropEnt->addToLootSet(0, 16, n4);
	freeDropEnt->addToLootSet(0, 13, n5);
	if (0x0 != (freeDropEnt->info & 0x100000)) {
		this->unlinkEntity(freeDropEnt);
	}
	freeDropEnt->info &= 0xFFFF;
	freeDropEnt->info |= 0x1420000;
	int sprite = freeDropEnt->getSprite();
	int height = app->render->getHeight(n, n2);
	app->render->mapSpriteInfo[sprite] = n6;
	app->render->mapSpriteInfo[sprite] |= 0xD00;
	app->render->mapSprites[app->render->S_X + sprite] = (short)n;
	app->render->mapSprites[app->render->S_Y + sprite] = (short)n2;
	app->render->mapSprites[app->render->S_Z + sprite] = (short)(32 + height);
	app->render->mapSprites[app->render->S_ENT + sprite] = (short)this->lastDropEntIndex;
	app->render->mapSprites[app->render->S_RENDERMODE + sprite] = 0;
	app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = 64;
	app->render->relinkSprite(sprite);
	this->linkEntity(freeDropEnt, n >> 6, n2 >> 6);
	return freeDropEnt;
}

void Game::throwDropItem(int dstX, int dstY, int n, Entity* entity) {
	Applet* app = CAppContainer::getInstance()->app;

	LerpSprite* allocLerpSprite = this->allocLerpSprite(nullptr, entity->getSprite(), entity->def->eType != 6);
	if (allocLerpSprite != nullptr) {
		allocLerpSprite->srcX = dstX;
		allocLerpSprite->srcY = dstY;
		allocLerpSprite->srcZ = (short)(32 + n);
		allocLerpSprite->dstX = dstX;
		allocLerpSprite->dstY = dstY;
		allocLerpSprite->dstZ = allocLerpSprite->srcZ;
		allocLerpSprite->height = 48;
		allocLerpSprite->flags |= (Enums::LS_FLAG_ASYNC | Enums::LS_FLAG_PARABOLA);
		allocLerpSprite->srcScale = allocLerpSprite->dstScale = app->render->mapSprites[app->render->S_SCALEFACTOR + entity->getSprite()];
		allocLerpSprite->startTime = app->gameTime;
		allocLerpSprite->travelTime = 850;
		int n3 = app->nextByte() & 0x7;
		int n4 = 8;
		while (--n4 >= 0) {
			dstX = allocLerpSprite->srcX + this->dropDirs[n3 << 1];
			dstY = allocLerpSprite->srcY + this->dropDirs[(n3 << 1) + 1];
			this->trace(allocLerpSprite->srcX, allocLerpSprite->srcY, dstX, dstY, entity, 15855, 25);
			if (this->traceEntity == nullptr && (this->baseVisitedTiles[dstY >> 6] & 1 << (dstX >> 6)) == 0x0) {
				allocLerpSprite->dstX = dstX;
				allocLerpSprite->dstY = dstY;
				allocLerpSprite->dstZ = (short)(app->render->getHeight(allocLerpSprite->dstX, allocLerpSprite->dstY) + 32);
				break;
			}
			n3 = (n3 + 1 & 0x7);
		}
	}
}

int Game::updateLerpSprite(LerpSprite* lerpSprite) {
	Applet* app = CAppContainer::getInstance()->app;

	int n = 0;
	int n2 = lerpSprite->hSprite - 1;
	int n3 = app->canvas->viewX >> 6;
	int n4 = app->canvas->viewY >> 6;
	int n5 = app->gameTime - lerpSprite->startTime;
	int n6 = app->render->mapSpriteInfo[n2];
	int n7 = n6 & 0xFF;
	if ((n6 & 0x400000) != 0x0) {
		n7 += 257;
	}
	if (n5 >= lerpSprite->travelTime) {
		this->freeLerpSprite(lerpSprite);
		return 3;
	}
	if (n5 < 0) {
		return 4;
	}
	int n8 = 0;
	if (lerpSprite->travelTime != 0) {
		n8 = (n5 << 16) / (lerpSprite->travelTime << 8);
	}
	app->render->mapSprites[app->render->S_X + n2] = (short)(lerpSprite->srcX + (n8 * (lerpSprite->dstX - lerpSprite->srcX << 8) >> 16));
	app->render->mapSprites[app->render->S_Y + n2] = (short)(lerpSprite->srcY + (n8 * (lerpSprite->dstY - lerpSprite->srcY << 8) >> 16));
	app->render->mapSprites[app->render->S_SCALEFACTOR + n2] = (uint8_t)(lerpSprite->srcScale + (n8 * (lerpSprite->dstScale - lerpSprite->srcScale << 8) >> 16));
	app->render->mapSprites[app->render->S_Z + n2] = (short)(lerpSprite->srcZ + (n8 * (lerpSprite->dstZ - lerpSprite->srcZ << 8) >> 16));
	if ((lerpSprite->flags & Enums::LS_FLAG_PARABOLA) != 0x0) {
		int n9 = n8 << 1;
		if ((lerpSprite->flags & Enums::LS_FLAG_TRUNC) != 0x0) {
			n9 = n9 * 6 / 8;
		}
		int n10 = app->render->sinTable[n9 & 0x3FF] >> 8;
		if (!(lerpSprite->flags & Enums::LS_FLAG_S_NORELINK)) {
			app->render->relinkSprite(n2, app->render->mapSprites[app->render->S_X + n2] << 4, app->render->mapSprites[app->render->S_Y + n2] << 4, app->render->mapSprites[app->render->S_Z + n2] << 4);
		}
		app->render->mapSprites[app->render->S_Z + n2] += (short)(n10 * (lerpSprite->height << 8) >> 16);
	}
	else if (!(lerpSprite->flags & Enums::LS_FLAG_S_NORELINK)) {
		app->render->relinkSprite(n2);
	}
	int n12 = app->render->mapSprites[app->render->S_X + n2] >> 6;
	int n13 = app->render->mapSprites[app->render->S_Y + n2] >> 6;
	if (app->render->mapSprites[app->render->S_ENT + n2] != -1 && !(app->render->mapSpriteInfo[n2] & 0x10000)) {
		Entity* entity = &this->entities[app->render->mapSprites[app->render->S_ENT + n2]];
		int n14 = entity->linkIndex % 32;
		int n15 = entity->linkIndex / 32;
		if (entity->def->eType == Enums::ET_NPC || entity->monster != nullptr) {
			int anim = ((app->render->mapSpriteInfo[n2] & 0xFF00) >> 8) & Enums::MANIM_MASK;
			int n17 = lerpSprite->dstX - lerpSprite->srcX;
			int n18 = lerpSprite->dstY - lerpSprite->srcY;

			if ((anim == Enums::MANIM_IDLE || anim == Enums::MANIM_WALK_FRONT || 
				(anim == Enums::MANIM_WALK_BACK && (lerpSprite->flags & Enums::LS_FLAG_AUTO_FACE) != 0x0)) && (n17 | n18) != 0x0) {
				int abs = std::abs((app->render->viewAngle & 0x3FF) - this->VecToDir(n17, n18, true));
				if (app->combat->WorldDistToTileDist(std::max((lerpSprite->dstX - lerpSprite->srcX) * (lerpSprite->dstX - lerpSprite->srcX), (lerpSprite->dstY - lerpSprite->srcY) * (lerpSprite->dstY - lerpSprite->srcY))) > 1 && abs < 256) {
					anim = Enums::MANIM_WALK_BACK;
					lerpSprite->flags |= Enums::LS_FLAG_AUTO_FACE;
				}
				else {
					anim = Enums::MANIM_WALK_FRONT;
				}
			}
			else if (anim == Enums::MANIM_IDLE_BACK) {
				anim = Enums::MANIM_WALK_BACK;
			}

			if (anim == Enums::MANIM_WALK_FRONT || anim == Enums::MANIM_WALK_BACK) {

				if (entity->def->eType == Enums::ET_MONSTER)
				{
					if (entity->def->eSubType == Enums::BOSS_CYBERDEMON || entity->def->eSubType == Enums::BOSS_MASTERMIND)
					{
						if ((1 + (n8 * lerpSprite->dist >> 12) & 0x3) >> 1 != ((((app->render->mapSpriteInfo[n2]) & 0xFF00) >> 8) & 3) >> 1) {
							if (entity->def->eSubType == Enums::BOSS_CYBERDEMON) {
								app->sound->playSound(1047, 0, true, 0);
							}
							else if (entity->def->eSubType == Enums::BOSS_MASTERMIND) {
								app->sound->playSound(1003, 0, true, 0);
							}
							else {
								app->sound->playSound((int8_t)(Enums::MSOUND_NONE), 0, true, 0);
							}
						}
					}
				}

				app->render->mapSpriteInfo[n2] = ((app->render->mapSpriteInfo[n2] & 0xFFFF00FF) | ((1 + (n8 * lerpSprite->dist >> 12) & 0x3) | anim) << 8);
			}
		}
		if (!(lerpSprite->flags & Enums::LS_FLAG_ENT_NORELINK) && (n12 != n14 || n13 != n15)) {
			this->unlinkEntity(entity);
			this->linkEntity(entity, app->render->mapSprites[app->render->S_X + n2] >> 6, app->render->mapSprites[app->render->S_Y + n2] >> 6);
			n |= 0x2;
		}
	}
	if (!app->canvas->staleView && ((n3 == n12 && n4 == n13) || app->render->checkTileVisibilty(n12, n13))) {
		n |= 0x4;
	}
	return n;
}

void Game::snapLerpSprites(int n) {
	if (this->numLerpSprites == 0) {
		return;
	}
	this->numCallThreads = 0;
	for (int i = 0; i < 16; ++i) {
		LerpSprite* lerpSprite = &this->lerpSprites[i];
		if (lerpSprite->hSprite != 0) {
			if (n == -1 || lerpSprite->hSprite == n + 1) {
				if (lerpSprite->thread != nullptr && !(lerpSprite->flags & Enums::LS_FLAG_ASYNC)) {
					int n2;
					for (n2 = 0; n2 < this->numCallThreads && this->callThreads[n2] != lerpSprite->thread; ++n2) {}
					if (n2 == this->numCallThreads) {
						this->callThreads[this->numCallThreads++] = lerpSprite->thread;
					}
				}
				lerpSprite->startTime = 0;
				lerpSprite->travelTime = 0;
				this->updateLerpSprite(lerpSprite);
			}
		}
	}
	for (int j = 0; j < this->numCallThreads; ++j) {
		this->callThreads[j]->run();
	}
}

void Game::updateLerpSprites() {
	Applet* app = CAppContainer::getInstance()->app;

	bool b = false;
	bool b2 = false;
	this->numCallThreads = 0;
	if (this->numLerpSprites > 0) {
		for (int i = 0; i < 16; ++i) {
			LerpSprite* lerpSprite = &this->lerpSprites[i];
			if (lerpSprite->hSprite != 0) {
				ScriptThread* thread = lerpSprite->thread;
				int flags = lerpSprite->flags;
				int updateLerpSprite = this->updateLerpSprite(lerpSprite);
				if (0x0 != (updateLerpSprite & 0x1) && !(flags & Enums::LS_FLAG_ASYNC) && nullptr != thread) {
					int n;
					for (n = 0; n < this->numCallThreads && this->callThreads[n] != thread; ++n) {}
					if (n == this->numCallThreads) {
						this->callThreads[this->numCallThreads++] = thread;
					}
				}
				b2 = (b2 || (updateLerpSprite & 0x2) != 0x0);
				b = (b || (updateLerpSprite & 0x4) != 0x0);
			}
		}
		for (int j = 0; j < this->numCallThreads; ++j) {
			if (this->callThreads[j]->inuse) {
				this->callThreads[j]->run();
			}
		}
		if (b2) {
			app->canvas->updateFacingEntity = true;
		}
		if (b) {
			app->canvas->invalidateRect();
		}
	}
}

LerpSprite* Game::allocLerpSprite(ScriptThread* thread, int n, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	LerpSprite* lerpSprite = nullptr;
	LerpSprite* lerpSprite2 = nullptr;
	for (int i = 0; i < 16; ++i) {
		LerpSprite* lerpSprite3 = &this->lerpSprites[i];
		if (lerpSprite3->hSprite == n + 1) {
			lerpSprite = lerpSprite3;
			break;
		}
		if (lerpSprite2 == nullptr && lerpSprite3->hSprite == 0) {
			lerpSprite2 = lerpSprite3;
		}
	}
	if (lerpSprite2 != nullptr && lerpSprite == nullptr) {
		lerpSprite = lerpSprite2;
		lerpSprite->flags = 0;
		lerpSprite->hSprite = n + 1;
		lerpSprite->thread = thread;
		this->numLerpSprites++;
	}
	else {
		if ((lerpSprite->flags & Enums::LS_FLAG_ANIMATING_EFFECT) != 0x0) {
			this->animatingEffects--;
		}
		int n2 = lerpSprite->hSprite - 1;
		Entity* entity = nullptr;
		if (-1 != app->render->mapSprites[app->render->S_ENT + n2]) {
			entity = &this->entities[app->render->mapSprites[app->render->S_ENT + n2]];
		}
		if (entity != nullptr && entity->monster != nullptr) {
			int n3 = (app->render->mapSpriteInfo[n2] & 0xFF00) >> 8 & 0xF0;
			if (n3 == 32 || (lerpSprite->flags & Enums::LS_FLAG_AUTO_FACE) != 0x0) {
				n3 = 0;
			}
			else if (n3 == 48) {
				n3 = 16;
			}
			app->render->mapSpriteInfo[n2] = ((app->render->mapSpriteInfo[n2] & 0xFFFF00FF) | n3 << 8);
		}
	}
	if (lerpSprite == nullptr) {
		app->Error(36); // ERR_MAX_LERPSPRITES
	}
	if (thread == nullptr) {
		lerpSprite->flags |= Enums::LS_FLAG_ASYNC;
	}
	if (b) {
		lerpSprite->flags |= Enums::LS_FLAG_ANIMATING_EFFECT;
		this->animatingEffects++;
	}
	return lerpSprite;
}

void Game::freeLerpSprite(LerpSprite* lerpSprite) {
	Applet* app = CAppContainer::getInstance()->app;
	Entity* entity = nullptr;
	int sprite = lerpSprite->hSprite - 1;
	int n2 = app->render->mapSpriteInfo[sprite] & 0xFF;
	if ((app->render->mapSpriteInfo[sprite] & 0x400000) != 0x0) {
		n2 += 257;
	}
	app->render->mapSprites[app->render->S_X + sprite] = (short)lerpSprite->dstX;
	app->render->mapSprites[app->render->S_Y + sprite] = (short)lerpSprite->dstY;
	if (!(lerpSprite->flags & Enums::LS_FLAG_TRUNC)) {
		app->render->mapSprites[app->render->S_Z + sprite] = (short)lerpSprite->dstZ;
	}
	app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = (short)lerpSprite->dstScale;
	if (!(lerpSprite->flags & Enums::LS_FLAG_S_NORELINK)) {
		app->render->relinkSprite(sprite);
	}
	if (app->render->mapSprites[app->render->S_ENT + sprite] != -1) {
		entity = &this->entities[app->render->mapSprites[app->render->S_ENT + sprite]];
	}
	int n3 = lerpSprite->dstX >> 6;
	int n4 = lerpSprite->dstY >> 6;
	if (nullptr != entity && 0x0 == (app->render->mapSpriteInfo[sprite] & 0x10000)) {
		if (entity->def->eType == Enums::ET_NPC || entity->def->eType == Enums::ET_MONSTER) {
			if (entity->monster != nullptr) {
				entity->monster->frameTime = 0;
			}
			int n5 = (app->render->mapSpriteInfo[sprite] & 0xFF00) >> 8 & 0xF0;
			if ((n5 == 16 || n5 == 48) && !(lerpSprite->flags & Enums::LS_FLAG_AUTO_FACE)) {
				app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x1000);
			}
			else {
				app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 0x0);
			}
		}
		int n6 = entity->linkIndex % 32;
		int n7 = entity->linkIndex / 32;
		if (!(lerpSprite->flags & Enums::LS_FLAG_ENT_NORELINK) && ((entity->info & 0x100000) == 0x0 || n3 != n6 || n4 != n7)) {
			this->unlinkEntity(entity);
			this->linkEntity(entity, app->render->mapSprites[app->render->S_X + sprite] >> 6, app->render->mapSprites[app->render->S_Y + sprite] >> 6);
		}
	}
	if ((lerpSprite->flags & Enums::LS_FLAG_DOORCLOSE) != 0x0) {
		app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = 64;
		app->render->mapSpriteInfo[sprite] &= 0xFFFF00FF;
		app->canvas->updateFacingEntity = true;
		app->canvas->automapDrawn = false;
		app->render->mapSpriteInfo[sprite] &= 0x7fffffff;
	}
	else if ((lerpSprite->flags & Enums::LS_FLAG_DOOROPEN) || (lerpSprite->flags & Enums::LS_FLAG_SECRET_HIDE)) {
		app->canvas->updateFacingEntity = true;
		this->secretActive = false;
		app->canvas->automapDrawn = false;
		this->unlinkEntity(entity);
		if (entity->def->eType != Enums::ET_DOOR) {
			app->render->mapSpriteInfo[sprite] |= Enums::SPRITE_FLAG_HIDDEN;
		}
	}
	else {
		if ((lerpSprite->flags & Enums::LS_MASK_CHICKEN_BOUNCE) == Enums::LS_MASK_CHICKEN_BOUNCE) {
			int sx = lerpSprite->srcX - lerpSprite->dstX;
			int sy = lerpSprite->srcY - lerpSprite->dstY;
			lerpSprite->dstX = lerpSprite->srcX = app->render->mapSprites[app->render->S_X + sprite];
			lerpSprite->dstY = lerpSprite->srcY = app->render->mapSprites[app->render->S_Y + sprite];
			lerpSprite->srcZ = app->render->mapSprites[app->render->S_Z + sprite];
			lerpSprite->flags &= ~Enums::LS_FLAG_TRUNC;
			if (sx != 0) {
				sx /= std::abs(sx);
			}
			if (sy != 0) {
				sy /= std::abs(sy);
			}
			int dx;
			int dy;
			if ((lerpSprite->dstX & 0x3F) == 32 && (lerpSprite->dstY & 0x3F) == 32) {
				dx = sx * 64;
				dy = sy * 64;
			}
			else {
				dx = sx * 31;
				dy = sy * 31;
			}
			lerpSprite->dstX = (lerpSprite->dstX + dx & 0xFFFFFFC0) + 32;
			lerpSprite->dstY = (lerpSprite->dstY + dy & 0xFFFFFFC0) + 32;
			lerpSprite->dstZ = app->render->getHeight(lerpSprite->dstX, lerpSprite->dstY) + 32;
			lerpSprite->height >>= 1;
			lerpSprite->startTime = app->gameTime - 100;
			lerpSprite->travelTime = 300;
			this->updateLerpSprite(lerpSprite);
			return;
		}

		if (lerpSprite->flags & Enums::LS_FLAG_SECRET_OPEN) {
			lerpSprite->flags &= ~Enums::LS_FLAG_SECRET_OPEN;
			lerpSprite->flags |= Enums::LS_FLAG_SECRET_HIDE;
			lerpSprite->srcX = lerpSprite->dstX;
			lerpSprite->srcY = lerpSprite->dstY;
			app->render->mapSpriteInfo[sprite] |= Enums::SPRITE_FLAG_DOORLERP;
			app->render->mapSprites[app->render->S_SCALEFACTOR + sprite] = (uint8_t)lerpSprite->dstScale;
			lerpSprite->dstScale = 0;
			lerpSprite->startTime = app->gameTime;
			switch (app->render->mapSpriteInfo[sprite] & Enums::SPRITE_FLAGS_ORIENTED) {
				case Enums::SPRITE_FLAG_EAST: {
					lerpSprite->dstY += 32;
					break;
				}
				case Enums::SPRITE_FLAG_NORTH: {
					lerpSprite->dstX += 32;
					break;
				}
				case Enums::SPRITE_FLAG_WEST: {
					lerpSprite->dstY -= 32;
					break;
				}
				case Enums::SPRITE_FLAG_SOUTH: {
					lerpSprite->dstX -= 32;
					break;
				}
			}
			lerpSprite->travelTime = 750;
			app->canvas->automapDrawn = false;
			return;
		}
	}
	if ((lerpSprite->flags & Enums::LS_FLAG_CHICKEN_KICK) != 0x0) {
		app->render->mapSpriteInfo[sprite] &= 0xFFFF00FF;
	}
	if ((lerpSprite->flags & Enums::LS_FLAG_ANIMATING_EFFECT) != 0x0) {
		lerpSprite->flags &= ~Enums::LS_FLAG_ANIMATING_EFFECT;
		this->animatingEffects--;
	}
	lerpSprite->hSprite = 0;
	this->numLerpSprites--;
	if (entity != nullptr) {
		uint8_t eSubType = entity->def->eSubType;
		if (entity->monster != nullptr) {
			if ((entity->monster->goalFlags & 0x1) != 0x0) {
				entity->aiFinishLerp();
			}
			if (app->player->isFamiliar && entity->distFrom(app->canvas->saveX, app->canvas->saveY) <= app->combat->tileDistances[0]) {
				int*calcPosition = entity->calcPosition();
				if (calcPosition[0] - app->canvas->saveX == 0 ^ calcPosition[1] - app->canvas->saveY == 0) {
					if (app->canvas->state == Canvas::ST_CAMERA) {
						app->player->unsetFamiliarOnceOutOfCinematic = true;
					}
					else {
						app->player->forceFamiliarReturnDueToMonster();
					}
				}
			}
		}
		if (entity->def->eType == 2) {
			Entity* mapEntity = this->findMapEntity(lerpSprite->dstX, lerpSprite->dstY, 256);
			if (mapEntity != nullptr) {
				if (mapEntity->def->eSubType == 1 && entity->def->eType == 2) {
					if (eSubType != 2) {
						entity->monster->monsterEffects |= 0x60008;
						entity->monster->monsterEffects &= 0xFFFFE1FD;
					}
				}
				else {
					entity->pain(5, mapEntity);
				}
			}
		}
	}
}

void Game::runScriptThreads(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->numScriptThreads == 0) {
		return;
	}
	for (int i = 0; i < 20; ++i) {
		ScriptThread* scriptThread = &this->scriptThreads[i];
		if (scriptThread->inuse) {
			if (scriptThread->state == 2) {
				if (this->isInputBlockedByScript() && app->canvas->state == Canvas::ST_AUTOMAP) {
					app->canvas->setState(Canvas::ST_PLAYING);
				}
				if (app->canvas->state == Canvas::ST_PLAYING || app->canvas->state == Canvas::ST_CAMERA) {
					scriptThread->attemptResume(n);
				}
			}
			if (scriptThread->state != 2 || scriptThread->stackPtr == 0) {
				this->freeScriptThread(scriptThread);
			}
		}
	}
}

ScriptThread* Game::allocScriptThread() {
	Applet* app = CAppContainer::getInstance()->app;

	for (int i = 0; i < 20; ++i) {
		ScriptThread* scriptThread = &this->scriptThreads[i];
		if (!scriptThread->inuse) {
			scriptThread->init();
			scriptThread->inuse = true;
			this->numScriptThreads++;
			return scriptThread;
		}
	}
	app->Error(40); // ERR_MAX_SCRIPTTHREADS
	return nullptr;
}

void Game::freeScriptThread(ScriptThread* scriptThread) {
	Applet* app = CAppContainer::getInstance()->app;

	if (!scriptThread->inuse) {
		app->Error(102); // ERR_SCRIPTTHREAD_FREE
		return;
	}
	scriptThread->reset();
	this->numScriptThreads--;
}

bool Game::isCameraActive() {
	return this->activeCameraView && this->activeCamera != nullptr;
}

int Game::executeTile(int n, int n2, int n3) {
	return this->executeTile(n, n2, n3, false);
}

bool Game::doesScriptExist(int n, int n2, int n3) {
	Applet* app = CAppContainer::getInstance()->app;

	if (n < 0 || n >= 32 || n2 < 0 || n2 >= 32) {
		return false;
	}
	int n4 = n2 * 32 + n;
	if ((app->render->mapFlags[n4] & 0x40) != 0x0) {
		for (int i = app->render->findEventIndex(n4); i != -1; i = app->render->getNextEventIndex()) {
			int n5 = app->render->tileEvents[i + 1];
			int n6 = n5 & n3;
			if ((n5 & 0x80000) == 0x0 && (n6 & 0xF) != 0x0 && (n6 & 0xFF0) != 0x0 && (((n5 & 0x7000) == 0x0 && (n3 & 0x7000) == 0x0) || (n6 & 0x7000) != 0x0)) {
				return true;
			}
		}
	}
	return false;
}

int Game::executeTile(int n, int n2, int n3, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	ScriptThread* allocScriptThread = this->allocScriptThread();
	int n4;
	if (app->canvas->state == Canvas::ST_DIALOG) {
		n4 = allocScriptThread->queueTile(n, n2, n3, b);
		allocScriptThread->unpauseTime = 0;
	}
	else {
		n4 = allocScriptThread->executeTile(n, n2, n3, b);
	}
	if (n4 != 2) {
		this->freeScriptThread(allocScriptThread);
	}
	return n4;
}

int Game::executeStaticFunc(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	if (n >= 12 || app->render->staticFuncs[n] == 65535) {
		return 0;
	}
	ScriptThread* allocScriptThread = this->allocScriptThread();
	allocScriptThread->alloc(app->render->staticFuncs[n]);
	return allocScriptThread->run();
}

int Game::queueStaticFunc(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	if (n >= 12 || app->render->staticFuncs[n] == 65535) {
		return 0;
	}
	ScriptThread* allocScriptThread = this->allocScriptThread();
	allocScriptThread->alloc(app->render->staticFuncs[n]);
	allocScriptThread->flags |= 0x2;
	allocScriptThread->unpauseTime = 0;
	app->canvas->blockInputTime = app->gameTime + 1;
	return 2;
}

int Game::getSaveVersion() {
	return 11;
}

void Game::loadEntityStates(InputStream* IS) {
	Applet* app = CAppContainer::getInstance()->app;
	for (short short1 = IS->readShort(), n = 0; n < short1; ++n) {
		app->resource->readMarker(IS);
		int index = IS->readInt();
		this->entities[index & 0xFFFF].loadState(IS, index);
	}
}
void Game::saveEntityStates(OutputStream* OS, bool b) {
	Applet* app = CAppContainer::getInstance()->app;
	int indices[Game::MAX_ENTITIES];

	int16_t stateCount = 0;
	for (int i = 0; i < this->numEntities; i++) {
		int saveHandle = this->entities[i].getSaveHandle(b);
		if (saveHandle != -1) {
			indices[stateCount++] = saveHandle;
		}
	}
	OS->writeShort(stateCount);
	for (int i = 0; i < stateCount; i++) {
		Entity* entity = &this->entities[indices[i] & 0xFFFF];
		app->resource->writeMarker(OS);
		OS->writeInt(indices[i]);
		entity->saveState(OS, indices[i]);
	}
}

bool Game::tileObstructsAttack(int n, int n2) {
	Applet* app = CAppContainer::getInstance()->app;

	int n3 = app->canvas->destX >> 6;
	int n4 = app->canvas->destY >> 6;
	if (n != n3 && n2 != n4) {
		return false;
	}
	for (Entity* entity = this->combatMonsters; entity != nullptr; entity = entity->monster->nextAttacker) {
		int* calcPosition = entity->calcPosition();
		int n5 = calcPosition[0] >> 6;
		int n6 = calcPosition[1] >> 6;
		if (n5 == n || n6 == n2) {
			if (n5 != n3) {
				int n7 = 0;
				int n8 = 0;
				if (n5 < n3) {
					n7 = n5;
					n8 = n3;
				}
				else if (n5 > n3) {
					n7 = n3;
					n8 = n5;
				}
				if (n > n7 && n < n8) {
					return true;
				}
			}
			if (n6 != n4) {
				int n9 = 0;
				int n10 = 0;
				if (n6 < n4) {
					n9 = n6;
					n10 = n4;
				}
				else if (n6 > n4) {
					n9 = n4;
					n10 = n6;
				}
				if (n2 > n9 && n2 < n10) {
					return true;
				}
			}
		}
	}
	return false;
}

bool Game::isInputBlockedByScript() {
	if (this->numScriptThreads > 0) {
		for (int i = 0; i < 20; ++i) {
			ScriptThread* scriptThread = &this->scriptThreads[i];
			if (scriptThread->inuse) {
				if ((scriptThread->type & 0x10000) != 0x0) {
					return true;
				}
			}
		}
	}
	return false;
}

void Game::updateScriptVars() {
	Applet* app = CAppContainer::getInstance()->app;

	this->scriptStateVars[0] = (short)app->player->statusEffects[33];
	this->scriptStateVars[1] = (short)app->player->ce->getStat(0);
	this->scriptStateVars[2] = (short)(app->canvas->viewX >> 6);
	this->scriptStateVars[3] = (short)(app->canvas->viewY >> 6);
	this->scriptStateVars[14] = app->player->characterChoice;
	this->scriptStateVars[16] = (short)(app->player->isFamiliar ? 1 : 0);
	this->scriptStateVars[8] = app->player->inventory[24];
}

void Game::awardSecret(bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	app->hud->addMessage((short)119);
	this->mapSecretsFound++;

	if (b) {
		app->sound->playSound(1103, 0, 3, 0);
	}
	else {
		app->sound->playSound(1020, 0, 3, 0);
	}

	if (this->mapSecretsFound == this->totalSecrets && (app->player->foundSecretsLevels & 1 << app->canvas->loadMapID - 1) == 0x0) {
		app->player->showAchievementMessage(1);
		app->player->foundSecretsLevels |= 1 << app->canvas->loadMapID - 1;
	}
	else {
		app->player->addXP(5);
	}
}

void Game::addEntityDeathFunc(Entity* entity, int n) {
	Applet* app = CAppContainer::getInstance()->app;

	int i;
	for (i = 0; i < 64; ++i) {
		if (this->entityDeathFunctions[i * 2] == -1) {
			this->entityDeathFunctions[i * 2 + 0] = (short)entity->getSprite();
			this->entityDeathFunctions[i * 2 + 1] = (short)n;
			break;
		}
	}
	if (i == 64) {
		app->Error("Too many entity death functions set");
	}
	entity->info |= 0x2000000;
}

void Game::removeEntityFunc(Entity* entity) {
	int sprite;
	int n;
	for (sprite = entity->getSprite(), n = 0; n < 64 && this->entityDeathFunctions[n * 2] != sprite; ++n) {}
	if (n != 64) {
		this->entityDeathFunctions[n * 2 + 0] = -1;
		this->entityDeathFunctions[n * 2 + 1] = -1;
	}
	entity->info &= 0xFDFFFFFF;
}

void Game::executeEntityFunc(Entity* entity, bool throwAwayLoot) {
	int sprite;
	int n;
	for (sprite = entity->getSprite(), n = 0; n < 64 && this->entityDeathFunctions[n * 2] != sprite; ++n) {}
	if (n != 64) {
		ScriptThread* allocScriptThread = this->allocScriptThread();
		allocScriptThread->throwAwayLoot = throwAwayLoot;
		allocScriptThread->alloc(this->entityDeathFunctions[n * 2 + 1]);
		allocScriptThread->run();
		this->entityDeathFunctions[n * 2 + 1] = (this->entityDeathFunctions[n * 2 + 0] = -1);
	}
}

void Game::foundLoot(int n, int n2) {
	Applet* app = CAppContainer::getInstance()->app;
	this->foundLoot(app->render->mapSprites[app->render->S_X + n], app->render->mapSprites[app->render->S_Y + n], app->render->mapSprites[app->render->S_Z + n], n2);
}

void Game::foundLoot(int n, int n2, int n3, int n4) {
	this->lootFound += (short)n4;
}

void Game::destroyedObject(int n) {
	this->destroyedObj++;
}

void Game::raiseCorpses() {
	Applet* app = CAppContainer::getInstance()->app;

	Entity* inactiveMonsters = this->inactiveMonsters;
	int n = 0;
	if (inactiveMonsters != nullptr) {
		do {
			Entity* nextOnList = inactiveMonsters->monster->nextOnList;
			int sprite = inactiveMonsters->getSprite();
			if ((inactiveMonsters->info & 0x10000) == 0x0 && (inactiveMonsters->info & 0x8000000) == 0x0 && (inactiveMonsters->info & 0x1000000) != 0x0 && !inactiveMonsters->isBoss() && (inactiveMonsters->monster->flags & 0x40) == 0x0 && this->findMapEntity(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], 15535) == nullptr && (this->difficulty != 4 || 0x0 == (inactiveMonsters->monster->monsterEffects & 0x4))) {
				this->gridEntities[n++] = inactiveMonsters;
				if (n == 9) {
					break;
				}
			}
			inactiveMonsters = nextOnList;
		} while (inactiveMonsters != this->inactiveMonsters && n != 4);
		for (int i = 0; i < n; ++i) {
			Entity* entity = this->gridEntities[i];
			entity->info |= 0x8000000;
			int sprite = entity->getSprite();
			entity->resurrect(app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], app->render->mapSprites[app->render->S_Z + sprite]);
			activate(entity, false, false, true, true);
			GameSprite* gameSprite = gsprite_allocAnim(241, app->render->mapSprites[app->render->S_X + sprite], app->render->mapSprites[app->render->S_Y + sprite], app->render->mapSprites[app->render->S_Z + sprite] - 20);
			gameSprite->flags |= 0x400;
			gameSprite->startScale = 64;
			gameSprite->destScale = 96;
			gameSprite->scaleStep = 40;
			int* calcPosition = entity->calcPosition();
			app->particleSystem->spawnParticles(1, -3355444, calcPosition[0], calcPosition[1], app->render->getHeight(calcPosition[0], calcPosition[1]) + 48);
		}
	}
}

bool Game::isInFront(int n, int n2) {
	Applet* app = CAppContainer::getInstance()->app;

	int n3 = 256 - (app->canvas->viewAngle & 0x3FF);
	int n4 = (n << 6) + 32 - app->canvas->viewX;
	int n5 = (n2 << 6) + 32 - app->canvas->viewY;
	if (n4 == 0 && n5 == 0) {
		return true;
	}
	int n6 = this->VecToDir(n4, n5, true) + n3 & 0x3FF;
	return n6 >= 128 && n6 <= 384;
}

int Game::VecToDir(int n, int n2, bool b) {
	int n3 = -1;
	int n4 = b ? 7 : 0;
	if (n <= -32) {
		n3 = 4;
	}
	else if (n >= 32) {
		n3 = 0;
	}
	if (n2 >= 32) {
		if (n3 == 4) {
			n3 = 5;
		}
		else if (n3 == 0) {
			n3 = 7;
		}
		else {
			n3 = 6;
		}
	}
	else if (n2 <= -32) {
		if (n3 == 4) {
			n3 = 3;
		}
		else if (n3 == 0) {
			n3 = 1;
		}
		else {
			n3 = 2;
		}
	}
	return n3 << n4;
}

void Game::NormalizeVec(int n, int n2, int* array) {
	int n3 = (int)this->FixedSqrt(n * n + n2 * n2);
	array[0] = (n << 12) / n3;
	array[1] = (n2 << 12) / n3;
}

int64_t Game::FixedSqrt(int64_t n) {
	if (n == 0LL) {
		return 0LL;
	}
	n <<= 8;
	int64_t n2 = 256LL;
	int64_t n3 = n2 + n / n2 >> 1;
	int64_t n4 = n3 + n / n3 >> 1;
	int64_t n5 = n4 + n / n4 >> 1;
	int64_t n6 = n5 + n / n5 >> 1;
	for (int i = 0; i < 12; ++i) {
		int64_t n7 = n6 + n / n6 >> 1;
		if (n7 == n6) {
			break;
		}
		n6 = n7;
	}
	return n6;
}

void Game::setMonsterClip(int n, int n2) {
	this->baseVisitedTiles[n2] |= 1 << n;
}

void Game::unsetMonsterClip(int n, int n2) {
	this->baseVisitedTiles[n2] &= ~(1 << n);
}

bool Game::monsterClipExists(int n, int n2) {
	return (this->baseVisitedTiles[n2] >> n & 0x1) == 0x1;
}

void Game::cleanUpCamMemory() {
	if (this->mayaCameras) {
		delete[] this->mayaCameras;
	}
	this->mayaCameras = nullptr;

	if (this->mayaCameraKeys) {
		delete[] this->mayaCameraKeys;
	}
	this->mayaCameraKeys = nullptr;

	if (this->mayaCameraTweens) {
		delete[] this->mayaCameraTweens;
	}
	this->mayaCameraTweens = nullptr;

	if (this->mayaTweenIndices) {
		delete[] this->mayaTweenIndices;
	}
	this->mayaTweenIndices = nullptr;
}

const char* Game::GetSaveFile(int i, int i2) {
	const char* name;
	switch (i)
	{
		case 0: // SDFWORLD
			name = "FWORLD";
			break;
		case 1: // SDCONFIG
			name = "CONFIGFILENAME";
			break;
		case 2: // SDBPLAYER
			name = "BRIEFPLAYER";
			break;
		case 3: // SDFPLAYER
			name = "FULLPLAYER";
			break;
		case 4: // SDBWORLD
			switch (i2)
			{
				case 0:
					name = "SB_1";
					break;
				case 1:
					name = "SB_2";
					break;
				case 2:
					name = "SB_3";
					break;
				case 3:
					name = "SB_4";
					break;
				case 4:
					name = "SB_5";
					break;
				case 5:
					name = "SB_6";
					break;
				case 6:
					name = "SB_7";
					break;
				case 7:
					name = "SB_8";
					break;
				case 8:
					name = "SB_9";
					break;
				case 9: // IOS Missing File
					name = "SB_10";
					break;
				default:
					name = nullptr;
					break;
			}
			break;
		default:
			name = nullptr;
			break;
	}

	return name;
}

char* Game::getProfileSaveFileName(const char* name) {
	if (name != nullptr)
	{
		int len1 = std::strlen(dir);
		int len2 = std::strlen(name);
		char* namePath = new char[len1 + len2 + 200];
		std::strcpy(namePath, dir);
		std::strcat(namePath, "/");
		std::strcat(namePath, name);
		return namePath;
	}
	else
	{
		puts("getProfileSaveFileName2: ERROR -> filename is NULL! ");
		return nullptr;
	}
}


int Game::getMonsterSound(int eSubType, int param, int soundType) {
	Applet* app = CAppContainer::getInstance()->app;
	int monsterSoundResId = 0;
	int rand = 0;

	if (soundType == Enums::MSOUND_ALERT1) {
		rand = app->nextByte() % 3;
	}
	
	if ((eSubType == Enums::MONSTER_PINKY) && (param == 0)) {
		switch (soundType) {
			case Enums::MSOUND_ALERT1:
			case Enums::MSOUND_ALERT2:
			case Enums::MSOUND_ALERT3:
				monsterSoundResId = 1085; // "Pinky_small_sight.wav"
				break;
			case Enums::MSOUND_ATTACK1:
			case Enums::MSOUND_ATTACK2:
				monsterSoundResId = 1082; // "Pinky_small_attack.wav"
				break;
			case Enums::MSOUND_IDLE:
				monsterSoundResId = 1026; // "demon_small_active.wav",
				break;
			case Enums::MSOUND_PAIN:
				monsterSoundResId = 1084; // "Pinky_small_pain.wav"
				break;
			case Enums::MSOUND_DEATH:
				monsterSoundResId = 1083; // "Pinky_small_death.wav",
				break;
		}
	}
	else
	{
		if ((eSubType == Enums::BOSS_MASTERMIND) && (param == 1)) {
			switch (soundType)
			{
				case Enums::MSOUND_ALERT1:
				case Enums::MSOUND_ALERT2:
				case Enums::MSOUND_ALERT3:
					monsterSoundResId = 1116; // "SpiderMastermind_sight.wav"
					break;
				case Enums::MSOUND_ATTACK1:
				case Enums::MSOUND_ATTACK2:
					goto getMonsterSound;
					break;
				case Enums::MSOUND_IDLE:
					monsterSoundResId = 1024; // "demon_active.wav"
					break;
				case Enums::MSOUND_PAIN:
					monsterSoundResId = 1025; // "demon_pain.wav"
					break;
				case Enums::MSOUND_DEATH:
					monsterSoundResId = 1115; // "SpiderMastermind_death.wav"
					break;
			}
		}
		else {
		getMonsterSound:
			monsterSoundResId = this->monsterSounds[(eSubType * Enums::MSOUND_TYPES) + soundType + rand];
			if (this->monsterSounds[(eSubType * Enums::MSOUND_TYPES) + soundType + rand] != Enums::MSOUND_NONE) {
				monsterSoundResId += 1000;
			}
		}
	}

	if ((eSubType == Enums::MONSTER_IMP) && (soundType == Enums::MSOUND_DEATH)) {
		monsterSoundResId = (std::rand() % 2) + 1050; // "Imp_death1.wav", "Imp_death2.wav"
	}

	if ((eSubType == Enums::MONSTER_ZOMBIE) && (soundType == Enums::MSOUND_DEATH)) {
		monsterSoundResId = (std::rand() % 3) + 1139; // "zombie_death1.wav", "zombie_death2.wav", "zombie_death3.wav" 
	}

	return monsterSoundResId;
}
