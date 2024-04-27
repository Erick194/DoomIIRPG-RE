#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "JavaStream.h"
#include "Resource.h"
#include "MayaCamera.h"
#include "Canvas.h"
#include "Image.h"
#include "Render.h"
#include "Game.h"
#include "Text.h"
#include "GLES.h"
#include "TGLVert.h"
#include "TGLEdge.h"
#include "TinyGL.h"
#include "Player.h"
#include "Game.h"
#include "MenuSystem.h"
#include "Menus.h"
#include "Combat.h"
#include "Enums.h"
#include "Hud.h"
#include "Utils.h"
#include "Sound.h"
#include "Span.h"

Render::Render() {
	std::memset(this, 0, sizeof(Render));
}

Render::~Render() {
}

bool Render::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	printf("Render::startup\n");

	this->nodeIdxs = new short[Render::MAX_VISIBLE_NODES];
	this->_spanTrans = new SpanType[11];
	this->_spanTexture = new SpanType[11];
	this->mapFlags = new uint8_t[1024];
	this->staticFuncs = new int[12];
	this->customSprites = new int[Render::MAX_CUSTOM_SPRITES];
	this->dropSprites = new int[Render::MAX_DROP_SPRITES];
	this->splitSprites = new int[Render::MAX_SPLIT_SPRITES];
	this->temp = new int[3];
	this->renderMode = Render::RENDER_DEFAULT;

	this->_spanTrans[Render::RENDER_NONE].Span = new SpanMode;
	this->_spanTrans[Render::RENDER_NONE].Span->Normal = (SpanFunc)spanNoDraw;
	this->_spanTrans[Render::RENDER_NONE].Span->DT = (SpanFunc)spanNoDraw;
	this->_spanTrans[Render::RENDER_NONE].Span->DS = (SpanFunc)spanNoDraw;
	this->_spanTrans[Render::RENDER_NONE].Span->Stretch = (SpanFuncStretch)spanNoDrawStretch;
	this->_spanTrans[Render::RENDER_NORMAL].Span = new SpanMode;
	this->_spanTrans[Render::RENDER_NORMAL].Span->Normal = (SpanFunc)spanTransparent;
	this->_spanTrans[Render::RENDER_NORMAL].Span->DT = (SpanFunc)spanTransparentDT;
	this->_spanTrans[Render::RENDER_NORMAL].Span->DS = (SpanFunc)spanTransparentDS;
	this->_spanTrans[Render::RENDER_NORMAL].Span->Stretch = (SpanFuncStretch)spanTransparentStretch;
	this->_spanTrans[Render::RENDER_BLEND25].Span = this->_spanTrans[Render::RENDER_NORMAL].Span;
	this->_spanTrans[Render::RENDER_BLEND50].Span = new SpanMode;
	this->_spanTrans[Render::RENDER_BLEND50].Span->Normal = (SpanFunc)spanBlend50Transparent;
	this->_spanTrans[Render::RENDER_BLEND50].Span->DT = (SpanFunc)spanBlend50TransparentDT;
	this->_spanTrans[Render::RENDER_BLEND50].Span->DS = (SpanFunc)spanBlend50TransparentDS;
	this->_spanTrans[Render::RENDER_BLEND50].Span->Stretch = (SpanFuncStretch)spanBlend50TransparentStretch;
	this->_spanTrans[Render::RENDER_ADD].Span = new SpanMode;
	this->_spanTrans[Render::RENDER_ADD].Span->Normal = (SpanFunc)spanAddTransparent;
	this->_spanTrans[Render::RENDER_ADD].Span->DT = (SpanFunc)spanAddTransparentDT;
	this->_spanTrans[Render::RENDER_ADD].Span->DS = (SpanFunc)spanAddTransparentDS;
	this->_spanTrans[Render::RENDER_ADD].Span->Stretch = (SpanFuncStretch)spanAddTransparentStretch;
	this->_spanTrans[Render::RENDER_ADD75].Span = this->_spanTrans[Render::RENDER_ADD].Span;
	this->_spanTrans[Render::RENDER_ADD50].Span = this->_spanTrans[Render::RENDER_ADD].Span;
	this->_spanTrans[Render::RENDER_ADD25].Span = this->_spanTrans[Render::RENDER_ADD].Span;
	this->_spanTrans[Render::RENDER_SUB].Span = new SpanMode;
	this->_spanTrans[Render::RENDER_SUB].Span->Normal = (SpanFunc)spanSubTransparent;
	this->_spanTrans[Render::RENDER_SUB].Span->DT = (SpanFunc)spanSubTransparentDT;
	this->_spanTrans[Render::RENDER_SUB].Span->DS = (SpanFunc)spanSubTransparentDS;
	this->_spanTrans[Render::RENDER_SUB].Span->Stretch = (SpanFuncStretch)spanSubTransparentStretch;
	this->_spanTrans[Render::RENDER_PERF].Span = new SpanMode;
	this->_spanTrans[Render::RENDER_PERF].Span->Normal = (SpanFunc)spanPerfTexture;
	this->_spanTrans[Render::RENDER_PERF].Span->DT = (SpanFunc)spanPerfTexture;
	this->_spanTrans[Render::RENDER_PERF].Span->DS = (SpanFunc)spanPerfTexture;
	this->_spanTrans[Render::RENDER_PERF].Span->Stretch = (SpanFuncStretch)spanPerfTextureStretch;
	//--------------------
	this->_spanTexture[Render::RENDER_NONE].Span = this->_spanTrans[Render::RENDER_NONE].Span;
	this->_spanTexture[Render::RENDER_NORMAL].Span = new SpanMode;
	this->_spanTexture[Render::RENDER_NORMAL].Span->Normal = (SpanFunc)spanTexture;
	this->_spanTexture[Render::RENDER_NORMAL].Span->DT = (SpanFunc)spanTextureDT;
	this->_spanTexture[Render::RENDER_NORMAL].Span->DS = (SpanFunc)spanTextureDS;
	this->_spanTexture[Render::RENDER_NORMAL].Span->Stretch = (SpanFuncStretch)spanNoDrawStretch;
	this->_spanTexture[Render::RENDER_BLEND25].Span = new SpanMode;
	this->_spanTexture[Render::RENDER_BLEND25].Span->Normal = (SpanFunc)spanBlend25Texture;
	this->_spanTexture[Render::RENDER_BLEND25].Span->DT = (SpanFunc)spanBlend25TextureDT;
	this->_spanTexture[Render::RENDER_BLEND25].Span->DS = (SpanFunc)spanBlend25TextureDS;
	this->_spanTexture[Render::RENDER_BLEND25].Span->Stretch = (SpanFuncStretch)spanNoDrawStretch;
	this->_spanTexture[Render::RENDER_BLEND50].Span = this->_spanTexture[Render::RENDER_NORMAL].Span;
	this->_spanTexture[Render::RENDER_ADD].Span = new SpanMode;
	this->_spanTexture[Render::RENDER_ADD].Span->Normal = (SpanFunc)spanAddTexture;
	this->_spanTexture[Render::RENDER_ADD].Span->DT = (SpanFunc)spanAddTextureDT;
	this->_spanTexture[Render::RENDER_ADD].Span->DS = (SpanFunc)spanAddTextureDS;
	this->_spanTexture[Render::RENDER_ADD].Span->Stretch = (SpanFuncStretch)spanNoDrawStretch;
	this->_spanTexture[Render::RENDER_ADD75].Span = this->_spanTexture[Render::RENDER_ADD].Span;
	this->_spanTexture[Render::RENDER_ADD50].Span = this->_spanTexture[Render::RENDER_ADD].Span;
	this->_spanTexture[Render::RENDER_ADD25].Span = this->_spanTexture[Render::RENDER_ADD].Span;
	this->_spanTexture[Render::RENDER_SUB].Span = new SpanMode;
	this->_spanTexture[Render::RENDER_SUB].Span->Normal = (SpanFunc)spanSubTexture;
	this->_spanTexture[Render::RENDER_SUB].Span->DT = (SpanFunc)spanSubTextureDT;
	this->_spanTexture[Render::RENDER_SUB].Span->DS = (SpanFunc)spanSubTextureDS;
	this->_spanTexture[Render::RENDER_SUB].Span->Stretch = (SpanFuncStretch)spanNoDrawStretch;
	this->_spanTexture[Render::RENDER_PERF].Span = this->_spanTrans[Render::RENDER_PERF].Span;

	this->skipCull = false;
	this->skipBSP = false;
	this->skipLines = false;
	this->skipSprites = false;
	this->skipViewNudge = false;

	//-------------------------------
	this->numMapSprites = 0;
	if (this->mapSprites) {
		delete this->mapSprites;
	}
	this->mapSprites = nullptr;

	if (this->mapSpriteInfo) {
		delete this->mapSpriteInfo;
	}
	this->mapSpriteInfo = nullptr;

	//-------------------------------
	this->lastTileEvent = -1;
	this->numTileEvents = 0;
	if (this->tileEvents) {
		delete this->tileEvents;
	}
	this->tileEvents = nullptr;

	//-------------------------------
	this->mapByteCodeSize = 0;
	if (this->mapByteCode) {
		delete this->mapByteCode;
	}
	this->mapByteCode = nullptr;

	//-------------------------------
	this->currentFrameTime = 0;
	this->postProcessMode = 0;
	this->brightenPostProcess = false;
	this->brightenPostProcessBeginTime = 0;
	this->screenVScrollOffset = 0;
	this->useMastermindHack = false;
	this->useCaldexHack = false;
	this->delayedSpriteBuffer[0] = -1;
	this->delayedSpriteBuffer[1] = -1;

	this->screenWidth = app->canvas->viewRect[2];
	this->screenHeight = app->canvas->viewRect[3];
	if ((this->screenHeight & 0x1) != 0x0) {
		--this->screenHeight;
	}

	this->_gles = new gles;
	this->_gles->WindowInit();
	this->_gles->GLInit(this);
	this->imgPortal = nullptr;

	this->imgPortal = app->loadImage("portal_image2.bmp", true);

	// [GEC] New 
	this->fixWaterAnim1 = false;
	this->fixWaterAnim2 = false;
	this->fixWaterAnim3 = false;
	this->fixWaterAnim4 = false;

	return true;
}

void Render::shutdown() {
	this->unloadMap();
}

int Render::getNextEventIndex() {
	if (this->lastTileEvent == -1) {
		return -1;
	}

	int n = (this->lastTileEvent & 0xFFFF0000) >> 16;
	int n2 = (this->lastTileEvent & 0xFFFF) + 1;
	if (n2 < this->numTileEvents) {
		int n3 = n2 * 2;
		if ((this->tileEvents[n3] & 0x3FF) == n) {
			this->lastTileEvent = n2 | (n << 16);
			return n3;
		}
	}

	return this->lastTileEvent = -1;
}

int Render::findEventIndex(int n) {
	for (int i = 0; i < this->numTileEvents; ++i) {
		int n2 = i * 2;
		if ((this->tileEvents[n2] & 0x3FF) == n) {
			this->lastTileEvent = i | (n << 16);
			return n2;
		}
	}
	return this->lastTileEvent = -1;
}

void Render::unloadMap() {
	Applet* app = CAppContainer::getInstance()->app;

	this->startFogLerp(32752, 32752, 0);
	this->_gles->UnloadSkyMap();

	if (this->mediaBounds) { delete this->mediaBounds; }
	this->mediaBounds = nullptr;

	if (this->mediaMappings) { delete this->mediaMappings; }
	this->mediaMappings = nullptr;

	if (this->mediaDimensions) { delete this->mediaDimensions; }
	this->mediaDimensions = nullptr;

	if (this->mediaPalColors) { delete this->mediaPalColors; }
	this->mediaPalColors = nullptr;

	if (this->mediaTexelSizes) { delete this->mediaTexelSizes; }
	this->mediaTexelSizes = nullptr;

	if (this->mediaPalettesSizes) { delete this->mediaPalettesSizes; }
	this->mediaPalettesSizes = nullptr;

	if (this->mediaTexelSizes2) { delete this->mediaTexelSizes2; }
	this->mediaTexelSizes2 = nullptr;

	if (this->mediaTexels) {
		for (int i = 0; i < 1024; i++) {
			if (this->mediaTexels[i]) { delete this->mediaTexels[i]; }
			this->mediaTexels[i] = nullptr;
		}
		delete this->mediaTexels;
	}
	this->mediaTexels = nullptr;

	if (this->mediaPalettes) {
		for (int i = 0; i < 1024; i++) {
			for (int j = 0; j < 16; j++) {
				if (this->mediaPalettes[i][j]) { delete this->mediaPalettes[i][j]; }
				this->mediaPalettes[i][j] = nullptr;
			}
			delete this->mediaPalettes[i];
			this->mediaPalettes[i] = nullptr;
		}
		
		delete this->mediaPalettes;
	}
	this->mediaPalettes = nullptr;
	
	if (this->normals) { delete this->normals; }
	this->normals = nullptr;

	if (this->nodeNormalIdxs) { delete this->nodeNormalIdxs; }
	this->nodeNormalIdxs = nullptr;

	if (this->nodeOffsets) { delete this->nodeOffsets; }
	this->nodeOffsets = nullptr;

	if (this->nodeChildOffset1) { delete this->nodeChildOffset1; }
	this->nodeChildOffset1 = nullptr;

	if (this->nodeChildOffset2) { delete this->nodeChildOffset2; }
	this->nodeChildOffset2 = nullptr;

	if (this->nodeSprites) { delete this->nodeSprites; }
	this->nodeSprites = nullptr;

	if (this->nodeBounds) { delete this->nodeBounds; }
	this->nodeBounds = nullptr;

	if (this->nodePolys) { delete this->nodePolys; }
	this->nodePolys = nullptr;

	if (this->lineFlags) { delete this->lineFlags; }
	this->lineFlags = nullptr;

	if (this->lineXs) { delete this->lineXs; }
	this->lineXs = nullptr;

	if (this->lineYs) { delete this->lineYs; }
	this->lineYs = nullptr;

	if (this->heightMap) { delete this->heightMap; }
	this->heightMap = nullptr;

	this->numMapSprites = 0;
	if (this->mapSprites) { delete this->mapSprites; }
	this->mapSprites = nullptr;

	if (this->mapSpriteInfo) { delete this->mapSpriteInfo; }
	this->mapSpriteInfo = nullptr;

	this->numTileEvents = 0;
	this->mapByteCodeSize = 0;
	app->game->mapSecretsFound = 0;

	if (this->tileEvents) { delete this->tileEvents; }
	this->tileEvents = nullptr;

	if (this->mapByteCode) { delete this->mapByteCode; }
	this->mapByteCode = nullptr;

	if (this->skyMapTexels) { delete this->skyMapTexels; }
	this->skyMapTexels = nullptr;

	if (this->skyMapPalette) {
		for (int i = 0; i < 16; i++) {
			if(this->skyMapPalette[i]) { delete this->skyMapPalette[i]; }
			this->skyMapPalette[i] = nullptr;
		}
		delete this->skyMapPalette;
	}
	this->skyMapPalette = nullptr;

	for (int i = 0; i < 12; ++i) {
		this->staticFuncs[i] = -1;
	}

	if (app->canvas->loadMapStringID != -1) {
		app->localization->unloadText(app->canvas->loadMapStringID);
	}
	app->canvas->loadMapStringID = -1;

	for (int j = 0; j < Render::MAX_CUSTOM_SPRITES; ++j) {
		this->customSprites[j] = -1;
	}
	for (int k = 0; k < Render::MAX_DROP_SPRITES; ++k) {
		this->dropSprites[k] = -1;
	}
}

void Render::RegisterMedia(int n) {
	short mappingsBeg = this->mediaMappings[n];
	short mappingsEnd = this->mediaMappings[n + 1];
	for (; mappingsBeg < mappingsEnd; mappingsBeg++) {
		int palIndex = mappingsBeg;
		if ((this->mediaPalColors[palIndex] & Render::MEDIA_FLAG_REFERENCE) != 0x0) {
			palIndex = (this->mediaPalColors[palIndex] & 0x3FF);
		}
		this->mediaPalColors[palIndex] |= Render::MEDIA_PALETTE_REGISTERED;

		int texelIndex = mappingsBeg;
		if ((this->mediaTexelSizes[texelIndex] & Render::MEDIA_FLAG_REFERENCE) != 0x0) {
			texelIndex = (this->mediaTexelSizes[texelIndex] & 0x3FF);
		}
		this->mediaTexelSizes[texelIndex] |= Render::MEDIA_TEXELS_REGISTERED;
	}
}

void Render::FinalizeMedia() {
	Applet* app = CAppContainer::getInstance()->app;
	InputStream IS;

	app->canvas->updateLoadingBar(false);
	this->texelMemoryUsage = 0;
	this->paletteMemoryUsage = 0;

	int n = 0;
	int n2 = 0;
	for (int i = 0; i < 1024; ++i) {
		bool b = (this->mediaTexelSizes[i] & Render::MEDIA_TEXELS_REGISTERED) != 0x0;
		bool b2 = (this->mediaPalColors[i] & Render::MEDIA_PALETTE_REGISTERED) != 0x0;

		if (b) {
			int n3 = (this->mediaTexelSizes[i] & 0x3FFFFFFF) + 1;
			this->texelMemoryUsage += n3;
			this->mediaTexelSizes2[n2] = n3;
			this->mediaTexels[n2] = new uint8_t[n3];
			n2++;
		}
		if (b2) {
			int n4 = this->mediaPalColors[i] & 0x3FFFFFFF;
			this->paletteMemoryUsage += 4 * n4;
			this->mediaPalettesSizes[n] = n4;
			this->mediaPalettes[n][0] = new uint16_t[n4];
			n++;
		}
	}

	app->canvas->updateLoadingBar(false);

	if (!IS.loadFile(Resources::RES_NEWPALETTES_BIN_GZ, InputStream::LOADTYPE_RESOURCE)) {
		app->Error("getResource(%s) failed\n", Resources::RES_NEWPALETTES_BIN_GZ);
	}

	// [GEC]: Verifica los datos del las palletas y se corrigen datos si es necesario
	if (checkFileMD5Hash(IS.data, IS.fileSize, 0xFFE7C84C143EA906, 0xF93B1383F6B2510E)) {
		// WATER STREAM Palette
		IS.data[266852] = 0;
		IS.data[266853] = 0;
		IS.data[266854] = 0;
		IS.data[266855] = 0;
	}

	//app->checkPeakMemory("Loading Palettes");
	int n5 = 0;
	int n6 = 0;
	for (int j = 0; j < 1024; ++j) {
		bool b3 = (this->mediaPalColors[j] & Render::MEDIA_PALETTE_REGISTERED) != 0x0;
		bool b4 = (this->mediaPalColors[j] & Render::MEDIA_FLAG_REFERENCE) != 0x0;
		int n7 = this->mediaPalColors[j] & 0x3FFFFFFF;
		if (b3 && !b4) {
			app->resource->read(&IS, n7 * 2);
			for (int k = 0; k < n7; ++k) {
				this->mediaPalettes[n5][0][k] = app->resource->shiftUShort(); // j2me only -> upSamplePixel(app->resource->shiftUShort());
			}
			int n8 = (j | Render::MEDIA_FLAG_REFERENCE);
			for (int l = j + 1; l < 1024; ++l) {
				if (this->mediaPalColors[l] == n8) {
					this->mediaPalColors[l] = (0xC0000000 | n5);
				}
			}
			this->mediaPalColors[j] = (0x40000000 | n5);
			++n5;
			app->resource->readMarker(&IS, n6);
			n6 += (2 * n7) + 4;
		}
		else if (!b4) {
			app->resource->bufSkip(&IS, n7 * 2, true);
			app->resource->readMarker(&IS, n6);
			n6 += (2 * n7) + sizeof(uint32_t);
		}
		if ((j & 0x1E) == 0x1E) {
			app->canvas->updateLoadingBar(false);
		}
	}

	int n9 = 0;
	int n10 = -1;
	int n11 = 0;
	int n12 = 0;
	int m = 0;

	IS.close();
	app->canvas->updateLoadingBar(false);
	//app->checkPeakMemory("Loading Texels");

	for (int n13 = 0; n13 < 1024; ++n13) {
		bool b5 = (this->mediaTexelSizes[n13] & Render::MEDIA_TEXELS_REGISTERED) != 0x0;
		bool b6 = (this->mediaTexelSizes[n13] & Render::MEDIA_FLAG_REFERENCE) != 0x0;
		int n14 = (this->mediaTexelSizes[n13] & 0x3FFFFFFF) + 1;
		if (b5 && !b6) {
			if (m != n10) {
				IS.close();
				/*String str = "/tex0";
				if (m >= 10) {
					str = "/tex";
				}
				resourceAsStream2 = App.getResourceAsStream(str + m + ".bin");*/
				if (!IS.loadFile(Resources::RES_NEWTEXEL_FILE_ARRAY[m], InputStream::LOADTYPE_RESOURCE)) {
					app->Error("getResource(%s) failed\n", Resources::RES_NEWTEXEL_FILE_ARRAY[m]);
				}
				
				n10 = m;
				n11 = 0;
				//Canvas.updateLoadingBar(false);
			}

			if (n11 != n12) {
				app->resource->bufSkip(&IS, n12 - n11, true);
			}
			//printf("n14 %d\n", n14);
			app->resource->readByteArray(&IS, this->mediaTexels[n9], 0, n14);

			// [GEC]: Verifica los datos del sprite de agua animada
			{
				if (n13 == 814) {
					if (checkFileMD5Hash(this->mediaTexels[n9], n14, 0x2AFCC8EDC9EA8610, 0xEA5458CBEBF345EC)) {
						this->fixWaterAnim1 = true;
					}
				}
				else if (n13 == 815) {
					if (checkFileMD5Hash(this->mediaTexels[n9], n14, 0x14F8BC466131E9AB, 0xFBEC94B21422E569)) {
						this->fixWaterAnim2 = true;
					}
				}
				else if (n13 == 816) {
					if (checkFileMD5Hash(this->mediaTexels[n9], n14, 0xB784065E177D596E, 0x23729441F971FEE7)) {
						this->fixWaterAnim3 = true;
					}
				}
				else if (n13 == 817) {
					if (checkFileMD5Hash(this->mediaTexels[n9], n14, 0x8A53E5E7CD062F73, 0xFAA5B42239006DC8)) {
						this->fixWaterAnim4 = true;
					}
				}
			}

			int n15 = (n13 | Render::MEDIA_FLAG_REFERENCE);
			for (int n16 = n13 + 1; n16 < 1024; ++n16) {
				if (this->mediaTexelSizes[n16] == n15) {
					this->mediaTexelSizes[n16] = (0xC0000000 | n9);
				}
			}
			this->mediaTexelSizes[n13] = (0x40000000 | n9);
			//printf("n9 %d\n", n9);
			//printf("this->mediaTexelSizes[%d] %d\n", n13, this->mediaTexelSizes[n13]);
			++n9;
			app->resource->readMarker(&IS, n12);
			n12 = (n11 = n12 + (n14 + sizeof(uint32_t)));
		}
		else if (!b6) {
			n12 += n14 + sizeof(uint32_t);
		}

		if (n12 > 0x40000) {
			++m;
			n12 = 0;
		}
		if ((n13 & 0xF) == 0xF) {
			app->canvas->updateLoadingBar(false);
		}
	}

	IS.close();
	this->_gles->CreateAllActiveTextures();
	app->canvas->updateLoadingBar(false);
	IS.~InputStream();
}

bool Render::beginLoadMap(int mapNameID) {
	Applet* app = CAppContainer::getInstance()->app;
	InputStream IS;

	this->mapNameID = mapNameID;
	app->canvas->loadMapStringID = (short)(4 + (this->mapNameID - 1));
	app->localization->loadText(app->canvas->loadMapStringID);

	for (int i = 0; i < 1024; ++i) {
		this->mapFlags[i] = 0;
	}

	this->mapEntranceAutomap = -1;
	this->mapExitAutomap = -1;

	for (int i = 0; i < Render::MAX_LADDERS_PER_MAP; ++i) {
		this->mapLadders[i] = -1;
	}

	for (int i = 0; i < Render::MAX_KEEP_PITCH_LEVEL_TILES; ++i) {
		this->mapKeepPitchLevelTiles[i] = -1;
	}

	this->portalState = Render::PORTAL_DNE;
	this->previousPortalState = 0;
	this->portalScripted = false;
	this->mapNameField = (0xC00 | app->game->levelNames[this->mapNameID - 1]);

	this->mediaMappings = new short[Render::MEDIA_MAX_MAPPINGS];
	this->mediaDimensions = new uint8_t[Render::MEDIA_MAX_IMAGES];
	this->mediaBounds = new short[(Render::MEDIA_MAX_IMAGES * 4)];
	this->mediaPalColors = new int[Render::MEDIA_MAX_IMAGES];
	this->mediaPalettesSizes = new int[Render::MEDIA_MAX_IMAGES];
	this->mediaTexelSizes = new int[Render::MEDIA_MAX_IMAGES];
	this->mediaTexelSizes2 = new int[Render::MEDIA_MAX_IMAGES];
	this->mediaTexels = new uint8_t*[1024]();
	this->mediaPalettes = new uint16_t **[1024]();
	for (int i = 0; i < 1024; ++i) {
		this->mediaPalettes[i] = new uint16_t *[16]();
		for (int j = 0; j < 16; ++j) {
			this->mediaPalettes[i][j] = nullptr;
		}
	}

	app->canvas->updateLoadingBar(false);
	IS.loadResource(Resources::RES_NEWMAPPINGS_BIN_GZ);
	app->resource->readShortArray(&IS, this->mediaMappings, 0, Render::MEDIA_MAX_MAPPINGS);
	app->resource->readByteArray(&IS, (uint8_t*)this->mediaDimensions, 0, Render::MEDIA_MAX_IMAGES);
	app->resource->readShortArray(&IS, this->mediaBounds, 0, (Render::MEDIA_MAX_IMAGES * 4));
	app->resource->readIntArray(&IS, this->mediaPalColors, 0, Render::MEDIA_MAX_IMAGES);
	app->resource->readIntArray(&IS, this->mediaTexelSizes, 0, Render::MEDIA_MAX_IMAGES);
	IS.close();

	this->mediaMappings[Enums::TILENUM_SKY_BOX] = (gles::MAX_MEDIA-1); // Readjust the index so it doesn't interfere with the fade texture.

	app->canvas->updateLoadingBar(false);

	IS.loadResource(Resources::RES_MAP_FILE_ARRAY[(mapNameID - 1)]);
	app->resource->read(&IS, 42);
	if (app->resource->shiftUByte() != 3) {
		app->Error(68); // ERR_BADMAPVERSION
		return false;
	}

	this->mapCompileDate = app->resource->shiftInt();
	this->mapSpawnIndex = app->resource->shiftUShort();
	this->mapSpawnDir = app->resource->shiftUByte();
	this->mapFlagsBitmask = app->resource->shiftByte();
	app->game->totalSecrets = app->resource->shiftByte();
	app->game->totalLoot = app->resource->shiftUByte();
	this->numNodes = app->resource->shiftUShort();
	int dataSizePolys = app->resource->shiftUShort();
	this->numLines = app->resource->shiftUShort();
	this->numNormals = app->resource->shiftUShort();
	this->numNormalSprites = app->resource->shiftUShort();
	this->numZSprites = app->resource->shiftShort();
	this->numMapSprites = this->numNormalSprites + this->numZSprites;
	this->numSprites = this->numMapSprites + Render::MAX_CUSTOM_SPRITES + Render::MAX_DROP_SPRITES;
	this->numTileEvents = (int)app->resource->shiftShort();
	this->mapByteCodeSize = (int)app->resource->shiftShort();
	app->game->totalMayaCameras = app->resource->shiftByte();
	app->game->totalMayaCameraKeys = app->resource->shiftShort();
	app->canvas->updateLoadingBar(false);

	short totalMayaTweens = 0;
	for (int l = 0; l < 6; ++l) {
		app->game->ofsMayaTween[l] = totalMayaTweens;
		short shiftShort = app->resource->shiftShort();
		if (shiftShort != -1) {
			totalMayaTweens += shiftShort;
		}
	}
	app->game->totalMayaTweens = totalMayaTweens;

	// Load Media data
	app->resource->readMarker(&IS, 0xDEADBEEF);
	app->resource->read(&IS, 2);
	int mediaCount = app->resource->shiftUShort();
	app->resource->read(&IS, mediaCount * 2);
	for (int i = 0; i < mediaCount; i++) {
		this->RegisterMedia(app->resource->shiftUShort());
	}
	app->resource->readMarker(&IS, 0xDEADBEEF);
	IS.close();
	this->FinalizeMedia();

	//-----------------------------
	this->nodeNormalIdxs = new uint8_t[this->numNodes];
	this->nodeOffsets = new short[this->numNodes];
	this->nodeChildOffset1 = new short[this->numNodes];
	this->nodeChildOffset2 = new short[this->numNodes];
	this->nodeSprites = new short[this->numNodes];
	this->nodeBounds = new uint8_t[this->numNodes * 4];
	this->nodePolys = new uint8_t[dataSizePolys];
	this->lineFlags = new uint8_t[(this->numLines + 1) / 2];
	this->lineXs = new uint8_t[this->numLines * 2];
	this->lineYs = new uint8_t[this->numLines * 2];
	this->normals = new short[this->numNormals * 3];
	this->heightMap = new uint8_t[1024];

	for (int i = 0; i < this->numNodes; ++i) {
		this->nodeSprites[i] = -1;
	}

	this->mapSprites = new short[this->numSprites * 10];
	for (int i = 0; i < this->numSprites * 10; ++i) {
		this->mapSprites[i] = 0;
	}

	this->mapSpriteInfo = new int[this->numSprites * 2];
	for (int i = 0; i < this->numSprites * 2; ++i) {
		this->mapSprites[i] = 0;
	}

	this->S_X = this->numSprites * 0;
	this->S_Y = this->numSprites * 1;
	this->S_Z = this->numSprites * 2;
	this->S_RENDERMODE = this->numSprites * 3;
	this->S_NODE = this->numSprites * 4;
	this->S_NODENEXT = this->numSprites * 5;
	this->S_VIEWNEXT = this->numSprites * 6;
	this->S_ENT = this->numSprites * 7;
	this->S_SCALEFACTOR = this->numSprites * 8;
	this->SINFO_SORTZ = this->numSprites;


	this->tileEvents = new int[this->numTileEvents * 2];
	this->mapByteCode = new uint8_t[this->mapByteCodeSize];
	app->game->mayaCameras = new MayaCamera[app->game->totalMayaCameras];
	app->game->mayaCameraKeys = new short[app->game->totalMayaCameraKeys * 7];
	app->game->mayaCameraTweens = new int8_t[app->game->totalMayaTweens];
	app->game->mayaTweenIndices = new short[app->game->totalMayaCameraKeys * 6];
	app->game->setKeyOffsets();

	//app->checkPeakMemory("Allocated memory for the map");
	IS.loadResource(Resources::RES_MAP_FILE_ARRAY[(mapNameID - 1)]);
	app->canvas->updateLoadingBar(false);
	//app->checkPeakMemory(, "Reading in final map data");

	app->resource->read(&IS, 42);
	app->resource->readMarker(&IS, 0xDEADBEEF);
	app->resource->read(&IS, mediaCount * 2 + 2);
	app->resource->readMarker(&IS, 0xDEADBEEF);
	app->resource->readShortArray(&IS, this->normals, 0, this->numNormals * 3);
	app->resource->readMarker(&IS);
	app->resource->readShortArray(&IS, this->nodeOffsets, 0, this->numNodes);
	app->resource->readMarker(&IS);
	app->resource->readByteArray(&IS, this->nodeNormalIdxs, 0, this->numNodes);
	app->resource->readMarker(&IS);
	app->resource->readShortArray(&IS, this->nodeChildOffset1, 0, this->numNodes);
	app->resource->readShortArray(&IS, this->nodeChildOffset2, 0, this->numNodes);
	app->resource->readMarker(&IS);
	app->resource->readByteArray(&IS, this->nodeBounds, 0, this->numNodes * 4);
	app->resource->readMarker(&IS);
	app->canvas->updateLoadingBar(false);
	app->resource->readByteArray(&IS, this->nodePolys, 0, dataSizePolys);
	app->resource->readMarker(&IS);
	app->resource->readByteArray(&IS, this->lineFlags, 0, (this->numLines + 1) / 2);
	app->resource->readByteArray(&IS, this->lineXs, 0, this->numLines * 2);
	app->resource->readByteArray(&IS, this->lineYs, 0, this->numLines * 2);
	app->resource->readMarker(&IS);
	app->resource->readByteArray(&IS, this->heightMap, 0, 1024);
	app->resource->readMarker(&IS);
	app->canvas->updateLoadingBar(false);
	app->resource->readCoordArray(&IS, this->mapSprites, this->S_X, this->numMapSprites);
	app->resource->readCoordArray(&IS, this->mapSprites, this->S_Y, this->numMapSprites);
	app->canvas->updateLoadingBar(false);

	for (int i = 0; this->numMapSprites, i < this->numMapSprites; i++) {
		this->mapSprites[i + app->render->S_NODE] = -1;
		this->mapSprites[i + app->render->S_NODENEXT] = -1;
		this->mapSprites[i + app->render->S_VIEWNEXT] = -1;
		this->mapSprites[i + app->render->S_ENT] = -1;
		this->mapSprites[i + app->render->S_SCALEFACTOR] = 64;
		this->mapSprites[i + app->render->S_Z] = 32;
	}

	int n5 = 0;
	int numMapSprites = this->numMapSprites;
	while (numMapSprites > 0) {
		int n6 = (Resource::IO_SIZE > numMapSprites) ? numMapSprites : Resource::IO_SIZE;
		numMapSprites -= n6;
		app->resource->read(&IS, n6);
		while (--n6 >= 0) {
			this->mapSpriteInfo[n5++] = app->resource->shiftUByte();
		}
	}

	app->resource->readMarker(&IS);
	app->canvas->updateLoadingBar(false);

	int n7 = 0;
	int numMapSprites2 = this->numMapSprites;
	while (numMapSprites2 > 0) {
		int n8 = ((Resource::IO_SIZE / 2) > numMapSprites2) ? numMapSprites2 : (Resource::IO_SIZE / 2);
		numMapSprites2 -= n8;
		app->resource->read(&IS, n8 * 2);
		while (--n8 >= 0) {
			this->mapSpriteInfo[n7++] |= (app->resource->shiftUShort() & 0xFFFF) << 16;
		}
	}

	app->resource->readMarker(&IS);
	app->resource->readUByteArray(&IS, this->mapSprites, this->S_Z + this->numNormalSprites, this->numZSprites);
	app->resource->readMarker(&IS);

	int numNormalSprites = this->numNormalSprites;
	int numZSprites = this->numZSprites;
	while (numZSprites > 0) {
		int n10 = (Resource::IO_SIZE > numZSprites) ? numZSprites : Resource::IO_SIZE;
		numZSprites -= n10;
		app->resource->read(&IS, n10);
		while (--n10 >= 0) {
			this->mapSpriteInfo[numNormalSprites++] |= app->resource->shiftUByte() << 8;
		}
	}
	app->canvas->updateLoadingBar(false);
	app->resource->readMarker(&IS);
	app->resource->readUShortArray(&IS, this->staticFuncs, 0, 12);
	app->resource->readMarker(&IS);
	app->resource->readIntArray(&IS, this->tileEvents, 0, app->render->numTileEvents * 2);

	for (int i = 0; i < this->numTileEvents; i++) {
		int index = this->tileEvents[i << 1] & 0x3FF;
		this->mapFlags[index] |= 0x40;
	}

	app->resource->readMarker(&IS);
	app->resource->readByteArray(&IS, this->mapByteCode, 0, this->mapByteCodeSize);
	app->resource->readMarker(&IS);
	app->canvas->updateLoadingBar(false);
	app->game->loadMayaCameras(&IS);
	app->resource->readMarker(&IS);
	app->resource->read(&IS, 512);

	int cnt = 0;
	for (int i = 0; i < 512; i++) {
		short flags = app->resource->shiftUByte();
		this->mapFlags[cnt++] |= (uint8_t)(flags & 0xF);
		this->mapFlags[cnt++] |= (uint8_t)((flags >> 4) & 0xF);
	}
	app->resource->readMarker(&IS);
	IS.close();

	app->canvas->updateLoadingBar(false);
	this->postProcessSprites();

	int skyIndex = ((this->mapNameID - 1) / 5 % 2) * 2;
	int skyPal = app->resource->getNumTableShorts(skyIndex + 16);
	int skyTexel = app->resource->getNumTableBytes(skyIndex + 17);

	this->skyMapPalette = new uint16_t*[16];
	for (int i = 0; i < 16; i++) {
		this->skyMapPalette[i] = new uint16_t[skyPal];
	}
	this->skyMapTexels = new uint8_t[skyTexel];

	app->resource->beginTableLoading();
	app->resource->loadUShortTable(this->skyMapPalette[0], skyIndex + 16);
	app->resource->loadUByteTable(this->skyMapTexels, skyIndex + 17);
	app->resource->finishTableLoading();
	app->canvas->updateLoadingBar(false);

	for (int n19 = 0; n19 < 1024; n19++) {
		if (this->mediaPalettes[n19][0] != nullptr) {
			int length = this->mediaPalettesSizes[n19];
			for (int n20 = 1; n20 < 16; n20++) {
				this->paletteMemoryUsage += 4 * length;
				this->mediaPalettes[n19][n20] = new uint16_t[length];
			}
		}
	}

	app->canvas->changeMapStarted = false;
	this->destDizzy = 0;
	this->baseDizzy = 0;

	return true;
}

void Render::draw2DSprite(int tileNum, int frame, int x, int y, int flags, int renderMode, int renderFlags, int scaleFactor) {
	Applet* app = CAppContainer::getInstance()->app;
	TinyGL* tinyGL = app->tinyGL;

	TGLVert* vert1 = &tinyGL->cv[0];
	TGLVert* vert2 = &tinyGL->cv[1];
	TGLVert* vert3 = &tinyGL->cv[2];

	//RENDER_FLAG_SCALE_WEAPON
	if (renderFlags & Render::RENDER_FLAG_SCALE_WEAPON) {  // [GEC] Adjusted like this to match the scaleFactor on the GL version
		scaleFactor = (int)((float)scaleFactor * 1.35f);
	}

	this->setupTexture(tileNum, frame, renderMode, renderFlags);

	int v12 = (176 * scaleFactor) / 0x10000;

	vert1->x = x << 3;
	vert1->y = y + v12 << 3;
	vert1->z = 8192;
	vert1->s = 0;

	vert2->x = x + v12 << 3;
	vert2->y = y + v12 << 3;
	vert2->z = 8192;
	vert2->s = 0x800000;

	vert3->x = x << 3;
	vert3->y = y << 3;
	vert3->z = 8192;
	vert3->s = 0;

	if (vert1->x < app->tinyGL->viewportClampX1) {
		vert1->s += (app->tinyGL->viewportClampX1 - vert1->x) * ((vert2->s - vert1->s) / (vert2->x - vert1->x));
		vert1->x = app->tinyGL->viewportClampX1;
	}

	if (vert2->x > app->tinyGL->viewportClampX2) {
		vert2->s -= (vert2->x - app->tinyGL->viewportClampX2) * ((vert2->s - vert1->s) / (vert2->x - vert1->x));
		vert2->x = app->tinyGL->viewportClampX2;
	}

	this->setupPalette(app->tinyGL->getFogPalette(0x40000000), renderMode, renderFlags);

	tinyGL->mv[0].x = tinyGL->viewX - ((int)(5 * ((tinyGL->view[2] & 0xFFFFFFE0) + (8 * (tinyGL->view[2] >> 5)))) >> 8);
	tinyGL->mv[0].y = tinyGL->viewY - ((int)(5 * ((tinyGL->view[6] & 0xFFFFFFE0) + (8 * (tinyGL->view[6] >> 5)))) >> 8);
	tinyGL->mv[0].z = tinyGL->viewZ - ((int)(5 * ((tinyGL->view[10] & 0xFFFFFFE0) + (8 * (tinyGL->view[10] >> 5)))) >> 8);

	int v27 = ((x - tinyGL->viewportWidth / 2) << 15) / tinyGL->viewportWidth;
	int v28 = (((y + v12) - tinyGL->viewportHeight / 2) << 15) / tinyGL->viewportWidth;
	int v33 = (v12 << 15) / tinyGL->viewportWidth;

	int view0 = (tinyGL->view[0] >> 5);
	int view1 = (tinyGL->view[1] >> 5);
	int view4 = (tinyGL->view[4] >> 5);
	int view5 = (tinyGL->view[5] >> 5);
	int view8 = (tinyGL->view[8] >> 5);
	int view9 = (tinyGL->view[9] >> 5);

	tinyGL->mv[0].x += (((v28 * view1) + (v27 * view0)) >> 14);
	tinyGL->mv[0].y += (((v28 * view5) + (v27 * view4)) >> 14);
	tinyGL->mv[0].z += (((v28 * view9) + (v27 * view8)) >> 14);

	tinyGL->mv[1].x = tinyGL->mv[0].x + ((v33 * view0) >> 14);
	tinyGL->mv[1].y = tinyGL->mv[0].y + ((v33 * view4) >> 14);
	tinyGL->mv[1].z = tinyGL->mv[0].z + ((v33 * view8) >> 14);

	tinyGL->mv[2].x = tinyGL->mv[1].x - ((v33 * view1) >> 14);
	tinyGL->mv[2].y = tinyGL->mv[1].y - ((v33 * view5) >> 14);
	tinyGL->mv[2].z = tinyGL->mv[1].z - ((v33 * view9) >> 14);

	this->_gles->SetGLState();
	bool v37 = this->_gles->DrawWorldSpaceSpriteLine(&tinyGL->mv[0], &tinyGL->mv[1], &tinyGL->mv[2], flags ^ 0x20000);
	this->_gles->ResetGLState();
	if (!v37) {
		app->tinyGL->drawClippedSpriteLine(vert1, vert2, vert3, flags, false);
	}
}

void Render::renderSprite(int x, int y, int z, int tileNum, int frame, int flags, int renderMode, int scaleFactor, int renderFlags) {
	this->renderSprite(x, y, z, tileNum, frame, flags, renderMode, scaleFactor, renderFlags, -1);
}

void Render::renderSprite(int x, int y, int z, int tileNum, int frame, int flags, int renderMode, int scaleFactor, int renderFlags, int palIndex) {
	Applet* app = CAppContainer::getInstance()->app;
	int n10 = scaleFactor;

	if ((flags & 0x80000000) != 0x0) {
		scaleFactor = 65536;
	}
	if (scaleFactor == 0) {
		return;
	}

	// [GEC] Exclusivo para el VIOS ya que se ve mejor en su color multiplicado
	if ((tileNum >= Enums::TILENUM_BOSS_VIOS) && (tileNum <= Enums::TILENUM_BOSS_VIOS5)) {
		renderFlags |= Render::RENDER_FLAG_MULTYPLYSHIFT;
	}

	this->setupTexture(tileNum, frame, renderMode, renderFlags);
	int n11 = app->tinyGL->imageBounds[1] - app->tinyGL->imageBounds[0];
	int n12 = app->tinyGL->imageBounds[3] - app->tinyGL->imageBounds[2];
	int n13 = (app->tinyGL->imageBounds[0] << 10) / app->tinyGL->sWidth;
	int n14 = (n11 << 10) / app->tinyGL->sWidth;
	int n15 = ((app->tinyGL->tHeight - app->tinyGL->imageBounds[3]) << 10) / app->tinyGL->tHeight;
	int n16 = (n12 << 10) / app->tinyGL->tHeight;
	if ((flags & 0x10100000) != 0x0) {
		app->tinyGL->faceCull = TinyGL::CULL_NONE;
	}
	else {
		app->tinyGL->faceCull = TinyGL::CULL_CCW;
	}
	if ((flags & 0x2F000000) == 0x0) {
		z -= 512;
		int n17 = 0;
		int n18 = 4;
		int n19;
		int n20;
		if (0x0 != (flags & 0x400000) || app->tinyGL->textureBaseSize == app->tinyGL->sWidth * app->tinyGL->tHeight) {
			n19 = ((((n11 >> 2) << 4) + 7) * scaleFactor) / 0x10000;
			n20 = ((((n12 >> 1) << 4) + 7) * scaleFactor) / 0x10000;
		}
		else {
			n19 = (518 * scaleFactor) / 0x10000;
			n20 = (1036 * scaleFactor) / 0x10000;
			n15 = (n13 = 0);
			n16 = (n14 = 1024);
			n18 = 3;
			n17 = 2;
		}
		n17 += 10;
		x -= n17 * this->viewCos >> 16;
		y += n17 * this->viewSin >> 16;
		if (tileNum == Enums::TILENUM_OBJ_CRATE) {
			z -= 224;
		}
		for (int i = 0; i < n18; ++i) {
			int n21 = (i & 0x2) >> 1;
			int n22 = (i & 0x1) ^ n21 ^ 0x1;
			TGLVert* tglVert = &app->tinyGL->mv[i];
			tglVert->x = x << 4;
			tglVert->y = y << 4;
			tglVert->z = z - 84;
			tglVert->s = n13 + n22 * n14;
			tglVert->t = n15 + n21 * n16;
			app->tinyGL->viewMtxMove(tglVert, 0, (n22 * 2 - 1) * n19, n21 * n20);
		}

		TGLVert* transform3DVerts = app->tinyGL->transform3DVerts(app->tinyGL->mv, n18);
		if (0x0 != (flags & 0x400000) || app->tinyGL->textureBaseSize == app->tinyGL->sWidth * app->tinyGL->tHeight) {
			app->tinyGL->ClipQuad(&transform3DVerts[0], &transform3DVerts[1], &transform3DVerts[2], &transform3DVerts[3]);
		}
		else {
			TGLVert* tglVert2 = &transform3DVerts[0];
			if (tglVert2->w + tglVert2->z < 0) {
				return;
			}
			if (app->tinyGL->clipLine(transform3DVerts)) {
				app->tinyGL->projectVerts(transform3DVerts, n18);
				if (palIndex >= 0) {
					app->tinyGL->spanPalette = app->tinyGL->paletteBase[palIndex % 16];
					this->setupPalette(app->tinyGL->spanPalette, renderMode, renderFlags);
				}
				else {
					this->setupPalette(app->tinyGL->getFogPalette(transform3DVerts[0].z << 16), renderMode, renderFlags);
				}
				if (!this->_gles->DrawWorldSpaceSpriteLine(&app->tinyGL->mv[0], &app->tinyGL->mv[1], &app->tinyGL->mv[2], flags)) {
					app->tinyGL->drawClippedSpriteLine(&transform3DVerts[1], &transform3DVerts[0], &transform3DVerts[2], flags, true);
				}
			}
		}
	}
	else {

		if (tileNum >= Enums::TILENUM_TERMINAL_TARGET && tileNum <= Enums::TILENUM_TERMINAL_HACKING) {
			z -= 256;
		}

		if (tileNum >= Enums::TILENUM_CLOSED_PORTAL_EYE && tileNum <= Enums::TILENUM_EYE_PORTAL) {
			z -= 128;
		}

		int n23 = 0;
		if ((flags & 0x4000000) != 0x0) {
			n23 = 0;
		}
		else if ((flags & 0x1000000) != 0x0) {
			n23 = 2;
		}
		else if ((flags & 0x8000000) != 0x0) {
			n23 = 4;
		}
		else if ((flags & 0x2000000) != 0x0) {
			n23 = 6;
		}
		if ((app->tinyGL->textureBaseSize == app->tinyGL->sWidth * app->tinyGL->tHeight) ||
			(tileNum == Enums::TILENUM_CLOSED_PORTAL_EYE) || (tileNum == Enums::TILENUM_EYE_PORTAL) || (tileNum == Enums::TILENUM_PORTAL_SOCKET)) {
			int n24;
			int n25;
			if (app->tinyGL->tHeight == 256 && app->tinyGL->sWidth == 256) {
				n24 = 64;
				n25 = 32;
			}
			else {
				n24 = n12 >> 1;
				n25 = n11 >> 2;
			}
			int n26 = n24 * scaleFactor / 65536;
			int n27 = n25 * scaleFactor / 65536;
			if ((flags & 0x20000000) == 0x0) {
				z += app->tinyGL->tHeight - app->tinyGL->imageBounds[3] << 4;
				z -= 16 * (scaleFactor / 2048);
			}
			else {
				z -= 512;
			}
			const int* viewStepValues = Canvas::viewStepValues;
			int n28 = ((n23 + 4) & 0x7) << 1;
			int n29 = ((n23 + 2) & 0x7) << 1;
			int n30 = n27 << 4;
			int n31 = n26 << 4;
			x <<= 4;
			y <<= 4;
			/*if (tileNum == Enums::TILENUM_GLASS && frame == 0) { // J2ME
				if ((flags & 0x3000000) != 0x0) {
					n13 += std::abs(x - this->viewX >> 2);
					n15 -= std::abs(y - this->viewY >> 3);
				}
				else {
					n13 += std::abs(y - this->viewY >> 2);
					n15 -= std::abs(x - this->viewX >> 3);
				}
			}*/
			if ((flags & 0x20000) != 0x0) {
				n13 += n14;
				n14 = -n14;
			}
			if ((flags & 0x40000) != 0x0) {
				n15 += n16;
				n16 = -n16;
			}

			int n32 = n31;
			if ((flags & 0x80000000) != 0x0) {
				if (tileNum >= Enums::TILENUM_RED_DOOR_LOCKED && tileNum <= Enums::TILENUM_BLUE_DOOR_UNLOCKED) {
					int n33 = n16;
					n31 = (n10 * n31) / 65536;
					n16 = (n10 * n16) / 65536;
					n15 += n33 - n16 >> 1;
				}
				else {
					int n34 = n14;
					n30 = (n10 * n30) / 65536;
					n14 = (n10 * n14) / 65536;
					n13 += n34 - n14;
				}
			}

			if (tileNum >= Enums::TILENUM_RED_DOOR_LOCKED && tileNum <= Enums::TILENUM_BLUE_DOOR_UNLOCKED) { // Slip Door
				int n35 = n16 >> 1;
				int n36 = n31 >> 1;
				int n37 = n32 >> 1;
				for (int j = 0; j < 2; ++j) {
					for (int k = 0; k < 4; ++k) {
						int n38 = (k & 0x2) >> 1;
						int n39 = (k & 0x1) ^ n38 ^ 0x1;
						int n40 = (n39 * 2 - 1) * n30;
						int n41 = (n38 * 2 - 1) * n36;
						TGLVert* tglVert3 = &app->tinyGL->mv[k];
						tglVert3->x = x + (viewStepValues[n29 + 0] >> 6) * n40;
						tglVert3->y = y + (viewStepValues[n29 + 1] >> 6) * n40;
						tglVert3->z = z + n38 * n41 + j * (n37 - n36 << 1);
						tglVert3->s = n13 + n39 * n14;
						tglVert3->t = n15 + n38 * n35;
					}
					z += n36;
					n15 += n35;
					app->tinyGL->swapXY = true;
					app->tinyGL->drawModelVerts(app->tinyGL->mv, 4);
				}
			}
			else if ((flags & 0x20000000) == 0x0) { // Wall 
				for (int l = 0; l < 4; ++l) {
					int n42 = (l & 0x2) >> 1;
					int n43 = (l & 0x1) ^ n42 ^ 0x1;
					TGLVert* tglVert4 = &app->tinyGL->mv[l];
					int n44 = (n43 * 2 - 1) * n30;
					tglVert4->x = x + (viewStepValues[n29 + 0] >> 6) * n44;
					tglVert4->y = y + (viewStepValues[n29 + 1] >> 6) * n44;
					tglVert4->z = z + n42 * n31;
					tglVert4->s = n13 + n43 * n14;
					tglVert4->t = n15 + n42 * n16;
				}
				app->tinyGL->swapXY = true;
				app->tinyGL->drawModelVerts(app->tinyGL->mv, 4);
			}
			else { // Plane
				for (int n45 = 0; n45 < 4; n45++) {
					int n46 = (n45 & 0x2) >> 1;
					int n47 = (n45 & 0x1) ^ n46 ^ 0x1;
					TGLVert* tglVert5 = &app->tinyGL->mv[n45];
					int n48 = (n47 * 2 - 1) * n30;
					int n49 = (n46 * 2 - 1) * n31 >> 1;
					tglVert5->x = x + (viewStepValues[n29 + 0] >> 6) * n48 + (viewStepValues[n28 + 0] >> 6) * n49;
					tglVert5->y = y + (viewStepValues[n29 + 1] >> 6) * n48 + (viewStepValues[n28 + 1] >> 6) * n49;
					tglVert5->z = z;
					tglVert5->s = n13 + n47 * n14;
					tglVert5->t = n15 + n46 * n16;
					if (tileNum == Enums::TILENUM_FLAT_LAVA) {
						tglVert5->s += (app->time / 8 & 0x3FF);
						tglVert5->t += (app->time / 16 & 0x3FF);
					}
					else if (tileNum == Enums::TILENUM_FLAT_LAVA2) {
						tglVert5->t += (app->time / 2 & 0x3FF);
					}
				}
				app->tinyGL->swapXY = false;
				app->tinyGL->drawModelVerts(app->tinyGL->mv, 4);
			}
		}
	}
}

void Render::occludeSpriteLine(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	int n2 = this->mapSpriteInfo[n];
	if (n2 < 0) {
		return;
	}

	int x = this->mapSprites[this->S_X + n] << 4;
	int y = this->mapSprites[this->S_Y + n] << 4;

	app->tinyGL->mv[0].x = x;
	app->tinyGL->mv[1].x = x;
	app->tinyGL->mv[0].y = y;
	app->tinyGL->mv[1].y = y;
	app->tinyGL->mv[0].z = 0;
	app->tinyGL->mv[1].z = 0;

	int n6 = 0;
	if ((n2 & 0x4000000) != 0x0) {
		n6 = 4;
	}
	else if ((n2 & 0x1000000) != 0x0) {
		n6 = 6;
	}
	else if ((n2 & 0x8000000) != 0x0) {
		n6 = 4;
	}
	else if ((n2 & 0x2000000) != 0x0) {
		n6 = 6;
	}

	int n7 = (n6 + 2 & 0x7) << 1;

	app->tinyGL->mv[0].x += Canvas::viewStepValues[n7 + 0] >> 1 << 4;
	app->tinyGL->mv[1].x -= Canvas::viewStepValues[n7 + 0] >> 1 << 4;
	app->tinyGL->mv[0].y += Canvas::viewStepValues[n7 + 1] >> 1 << 4;
	app->tinyGL->mv[1].y -= Canvas::viewStepValues[n7 + 1] >> 1 << 4;

	TGLVert* transform2DVerts = app->tinyGL->transform2DVerts(app->tinyGL->mv, 2);
	if (app->tinyGL->clipLine(transform2DVerts)) {
		app->tinyGL->projectVerts(transform2DVerts, 2);
		if (transform2DVerts[0].x > transform2DVerts[1].x) {
			app->tinyGL->occludeClippedLine(&transform2DVerts[1], &transform2DVerts[0]);
		}
		else {
			app->tinyGL->occludeClippedLine(&transform2DVerts[0], &transform2DVerts[1]);
		}
	}
}

void Render::drawNodeLines(short n) {
	Applet* app = CAppContainer::getInstance()->app;

	short n2 = this->nodeChildOffset2[n];
	for (int n3 = (n2 & 0x3FF) + (n2 >> 10 & 0x3F), i = n2 & 0x3FF; i < n3; ++i) {
		int n4 = this->lineFlags[i >> 1] >> ((i & 0x1) << 2) & 0xF;
		if ((n4 & 0x7) == 0x0 || (n4 & 0x7) == 0x4) {
			TGLVert* tglVert1 = &app->tinyGL->mv[0];
			TGLVert* tglVert2 = &app->tinyGL->mv[1];
			tglVert1->x = (this->lineXs[(i << 1) + 0] & 0xFF) << 7;
			tglVert2->x = (this->lineXs[(i << 1) + 1] & 0xFF) << 7;
			tglVert1->y = (this->lineYs[(i << 1) + 0] & 0xFF) << 7;
			tglVert2->y = (this->lineYs[(i << 1) + 1] & 0xFF) << 7;
			tglVert1->z = 0;
			tglVert2->z = 0;
			TGLVert* transform2DVerts = app->tinyGL->transform2DVerts(app->tinyGL->mv, 2);
			if (app->tinyGL->clipLine(transform2DVerts)) {
				app->tinyGL->projectVerts(transform2DVerts, 2);
				if (app->tinyGL->occludeClippedLine(&transform2DVerts[0], &transform2DVerts[1]) && app->game->updateAutomap) {
					this->lineFlags[i >> 1] |= (uint8_t)(8 << ((i & 0x1) << 2));
				}
			}
		}
		else if ((n4 & 0x7) == 0x6) {
			this->lineFlags[i >> 1] |= (uint8_t)(8 << ((i & 0x1) << 2));
		}
	}
}

bool Render::cullBoundingBox(int n, int n2, bool b) {
	return this->cullBoundingBox((n & 0xFFFFFFC0) << 4, (n2 & 0xFFFFFFC0) << 4, (n | 0x3F) << 4, (n2 | 0x3F) << 4, b);
}

bool Render::cullBoundingBox(int n, int n2, int n3, int n4, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->skipCull) {
		return false;
	}
	if (this->viewX >= n - TinyGL::CULL_EXTRA && this->viewX <= n3 + TinyGL::CULL_EXTRA && this->viewY >= n2 - TinyGL::CULL_EXTRA && this->viewY <= n4 + TinyGL::CULL_EXTRA) {
		return false;
	}
	TGLVert* tglVert = &app->tinyGL->mv[0];
	TGLVert* tglVert2 = &app->tinyGL->mv[1];
	if (this->viewX < n) {
		if (this->viewY < n2) {
			tglVert->x = n3;
			tglVert->y = n2;
			tglVert2->x = n;
			tglVert2->y = n4;
		}
		else if (this->viewY < n4) {
			tglVert->x = n;
			tglVert->y = n2;
			tglVert2->x = n;
			tglVert2->y = n4;
		}
		else {
			tglVert->x = n;
			tglVert->y = n2;
			tglVert2->x = n3;
			tglVert2->y = n4;
		}
	}
	else if (this->viewX < n3) {
		if (this->viewY < n2) {
			tglVert->x = n3;
			tglVert->y = n2;
			tglVert2->x = n;
			tglVert2->y = n2;
		}
		else {
			if (this->viewY < n4) {
				return false;
			}
			tglVert->x = n;
			tglVert->y = n4;
			tglVert2->x = n3;
			tglVert2->y = n4;
		}
	}
	else if (this->viewY < n2) {
		tglVert->x = n3;
		tglVert->y = n4;
		tglVert2->x = n;
		tglVert2->y = n2;
	}
	else if (this->viewY < n4) {
		tglVert->x = n3;
		tglVert->y = n4;
		tglVert2->x = n3;
		tglVert2->y = n2;
	}
	else {
		tglVert->x = n;
		tglVert->y = n4;
		tglVert2->x = n3;
		tglVert2->y = n2;
	}
	tglVert->z = 0;
	tglVert2->z = 0;

	TGLVert* transform2DVerts = app->tinyGL->transform2DVerts(app->tinyGL->mv, 2);
	if (app->tinyGL->clipLine(transform2DVerts)) {
		app->tinyGL->projectVerts(transform2DVerts, 2);
		return !app->tinyGL->clippedLineVisCheck(&transform2DVerts[0], &transform2DVerts[1], b);
	}
	return true;
}

void Render::addSprite(short n) {
	Applet* app = CAppContainer::getInstance()->app;

	if ((this->mapSpriteInfo[n] & 0x10000) != 0x0) {
		return;
	}
	int n2 = this->mapSpriteInfo[n] & 0xFF;
	Entity* entity = nullptr;
	if (this->mapSprites[this->S_ENT + n] != -1) {
		entity = &app->game->entities[this->mapSprites[this->S_ENT + n]];
	}
	int n3;
	if ((this->mapSpriteInfo[n] & 0x10000000) != 0x0) {
		n3 = 0x7fffffff;
	}
	else {
		short n4 = this->mapSprites[this->S_X + n];
		short n5 = this->mapSprites[this->S_Y + n];
		int* mvp = app->tinyGL->mvp;
		n3 = (n4 * mvp[2] + n5 * mvp[6] + this->mapSprites[this->S_Z + n] * mvp[10] >> 14) + mvp[14];
		if ((this->mapSpriteInfo[n] & 0x400000) != 0x0) {
			n3 += 6;
		}
		else if (n2 == 240 || n2 == 246 || n2 == 245 || n2 == 247) {
			n3 = 0x80000000;
		}
		else if (0x0 != (this->mapSpriteInfo[n] & 0xF000000)) {
			n3 += 5;
		}
		else if (entity != nullptr && (entity->info & 0x1010000) != 0x0) {
			++n3;
		}
		else if (entity != nullptr && entity->def->eType == 2) {
			this->handleMonsterIdleSounds(entity);
			--n3;
		}
		else if ((n2 >= 240 && n2 <= 244) || n2 == 241 || n2 == 255) {
			n3 -= 3;
		}
		else if (n2 == 138 || n2 == 139 || n2 == 137) {
			n3 += 2;
		}
		else if (n2 == 152) {
			n3 += 5;
		}
		else if (n2 == 239) {
			n3 -= 3;
		}
	}
	this->mapSpriteInfo[this->SINFO_SORTZ + n] = n3;
	if (app->game->updateAutomap) {
		this->mapSpriteInfo[n] |= 0x200000;
	}
	if (this->viewSprites == -1) {
		this->mapSprites[this->S_VIEWNEXT + n] = -1;
		this->viewSprites = n;
	}
	else if (n3 >= this->mapSpriteInfo[this->SINFO_SORTZ + this->viewSprites]) {
		this->mapSprites[this->S_VIEWNEXT + n] = this->viewSprites;
		this->viewSprites = n;
	}
	else {
		short viewSprites;
		for (viewSprites = this->viewSprites; this->mapSprites[this->S_VIEWNEXT + viewSprites] != -1 && n3 < this->mapSpriteInfo[this->SINFO_SORTZ + this->mapSprites[this->S_VIEWNEXT + viewSprites]]; viewSprites = this->mapSprites[this->S_VIEWNEXT + viewSprites]) {}
		this->mapSprites[this->S_VIEWNEXT + n] = this->mapSprites[this->S_VIEWNEXT + viewSprites];
		this->mapSprites[this->S_VIEWNEXT + viewSprites] = n;
	}
}

void Render::addSplitSprite(int n, int n2) {
	for (int i = n; i < this->numVisibleNodes; ++i) {
		short n3 = this->nodeIdxs[i];
		int n4 = (this->nodeBounds[(n3 << 2) + 0] & 0xFF) << 3;
		int n5 = (this->nodeBounds[(n3 << 2) + 1] & 0xFF) << 3;
		int n6 = (this->nodeBounds[(n3 << 2) + 2] & 0xFF) << 3;
		int n7 = (this->nodeBounds[(n3 << 2) + 3] & 0xFF) << 3;
		short n8 = this->mapSprites[this->S_X + n2];
		short n9 = this->mapSprites[this->S_Y + n2];
		if (n4 < n8 + 8 && n6 > n8 - 8 && n5 < n9 + 8 && n7 > n9 - 8 && this->numSplitSprites < 8) {
			this->splitSprites[this->numSplitSprites++] = (n3 << 16 | n2);
			return;
		}
	}
}

void Render::addNodeSprites(short n) {
	if ((this->nodeOffsets[n] & 0xFFFF) == 0xFFFF) {
		for (short n2 = this->nodeSprites[n]; n2 != -1; n2 = this->mapSprites[this->S_NODENEXT + n2]) {
			this->addSprite(n2);
		}
		for (int i = 0; i < this->numSplitSprites; ++i) {
			if ((this->splitSprites[i] & 0xFFFF0000) >> 16 == n) {
				this->addSprite((short)(this->splitSprites[i] & 0xFFFF));
				this->splitSprites[i] = -1;
			}
		}
	}
}

int Render::nodeClassifyPoint(int n, int n2, int n3, int n4) {
	int n5 = (this->nodeNormalIdxs[n] & 0xFF) * 3;
	return (((n2 * this->normals[n5]) + (n3 * this->normals[n5 + 1]) + (n4 * this->normals[n5 + 2])) >> 14) + (this->nodeOffsets[n] & 0xFFFF);
}

void Render::drawNodeGeometry(short n) {
	Applet* app = CAppContainer::getInstance()->app;

	int iVar10;
	int offset;
	bool bVar15;
	uint32_t z, s, t;

	if ((this->nodeOffsets[n] & 0xFFFF) != 0xFFFF) {
		return;
	}

	offset = this->nodeChildOffset1[n] & 0xffff;
	int meshCount = this->nodePolys[offset++];

	for (int i = 0; i < meshCount; i++) {
		uint16_t uVar3 = this->nodePolys[offset + 4] | (this->nodePolys[offset + 5] << 8);
		offset += 6;
		uint32_t uVar7 = (uint32_t)(uVar3 >> 7);
		iVar10 = Render::RENDER_NORMAL;
		app->tinyGL->faceCull = TinyGL::CULL_CCW;

		if (uVar7 == Enums::TILENUM_HELL_HANDS) {
			iVar10 = Render::RENDER_BLEND50;
		}
		else if ((uVar7 == Enums::TILENUM_FADE) || (uVar7 == Enums::TILENUM_SCORCH_MARK)) {
			iVar10 = Render::RENDER_NORMAL;
			if (!this->_gles->isInit) {
				iVar10 = Render::RENDER_SUB; // [GEC] TinyGL Only like J2ME/BREW
			}
		}
		else if ((uVar7 == Enums::TILENUM_FLAT_LAVA) || (uVar7 == Enums::TILENUM_FLAT_LAVA2)) {
			iVar10 = Render::RENDER_NORMAL;
			app->tinyGL->faceCull = TinyGL::CULL_NONE;
		}

		this->setupTexture(uVar7, 0, iVar10, 0);
		if (uVar7 == Enums::TILENUM_FLAT_LAVA) {
			z = 2 * (this->sinTable[app->time / 2 & 0x3FF] - this->sinTable[256]) >> 14;
			s = (app->time / 16 & 0x3FF);
			t = (app->time / 32 & 0x3FF);
		}
		else if (uVar7 == Enums::TILENUM_FLAT_LAVA2) {
			z = 0;
			s = 0;
			t = (app->time / 4 & 0x3FF);
		}
		else {
			z = 0;
			s = 0;
			t = 0;
		}

		for (int j = 0; j < (int)(uVar3 & 127); j++) {
			int polyFlags = this->nodePolys[offset++];
			int numVerts = (polyFlags & Enums::POLY_FLAG_VERTS_MASK) + 2;
			app->tinyGL->swapXY = (polyFlags & Enums::POLY_FLAG_SWAPXY) ? true : false;
			for (int k = 0; k < numVerts; k++) {
				TGLVert* vert = &app->tinyGL->mv[k];
				vert->x = (((uint32_t)this->nodePolys[offset + 0] & 0xFF) << 7);
				vert->y = (((uint32_t)this->nodePolys[offset + 1] & 0xFF) << 7);
				vert->z = (((uint32_t)this->nodePolys[offset + 2] & 0xFF) << 7) + z;
				vert->s = (((int8_t)this->nodePolys[offset + 3]) << 6) + s;
				vert->t = (((int8_t)this->nodePolys[offset + 4]) << 6) + t;
				offset += 5;
			}
			if (numVerts == 2) {
				numVerts = 4;

				TGLVert* vert = &app->tinyGL->mv[2];
				std::memcpy(&app->tinyGL->mv[2], &app->tinyGL->mv[1], sizeof(TGLVert));
				std::memcpy(&app->tinyGL->mv[1], vert, sizeof(TGLVert));

				app->tinyGL->mv[2].x = app->tinyGL->mv[0].x;
				app->tinyGL->mv[2].y = app->tinyGL->mv[0].y;
				app->tinyGL->mv[2].z = app->tinyGL->mv[0].z;
				app->tinyGL->mv[2].s = app->tinyGL->mv[0].s;
				app->tinyGL->mv[2].t = app->tinyGL->mv[0].t;

				app->tinyGL->mv[3].x = app->tinyGL->mv[2].x;
				app->tinyGL->mv[3].y = app->tinyGL->mv[2].y;
				app->tinyGL->mv[3].z = app->tinyGL->mv[2].z;
				app->tinyGL->mv[3].s = app->tinyGL->mv[2].s;
				app->tinyGL->mv[3].t = app->tinyGL->mv[2].t;

				switch (polyFlags & Enums::POLY_FLAG_AXIS_MASK) {
					case Enums::POLY_FLAG_AXIS_X: {
						app->tinyGL->mv[1].x = app->tinyGL->mv[2].x;
						app->tinyGL->mv[3].x = app->tinyGL->mv[0].x;
						break;
					}
					case Enums::POLY_FLAG_AXIS_Y: {
						app->tinyGL->mv[1].y = app->tinyGL->mv[2].y;
						app->tinyGL->mv[3].y = app->tinyGL->mv[0].y;
						break;
					}
					case Enums::POLY_FLAG_AXIS_Z: {
						app->tinyGL->mv[1].z = app->tinyGL->mv[2].z;
						app->tinyGL->mv[3].z = app->tinyGL->mv[0].z;
						break;
					}
				}

				if ((polyFlags & Enums::POLY_FLAG_UV_DELTAX)) {
					app->tinyGL->mv[1].s = app->tinyGL->mv[2].s;
					app->tinyGL->mv[3].s = app->tinyGL->mv[0].s;
				}
				else {
					app->tinyGL->mv[1].t = app->tinyGL->mv[2].t;
					app->tinyGL->mv[3].t = app->tinyGL->mv[0].t;
				}
			}
			app->tinyGL->drawModelVerts(app->tinyGL->mv, numVerts);
		}
		app->tinyGL->span = nullptr;
	}
}

void Render::walkNode(short n) {
	//printf("walkNode %d\viewPitch", viewPitch);
	Applet* app = CAppContainer::getInstance()->app;

	if (this->cullBoundingBox((this->nodeBounds[(n << 2) + 0] & 0xFF) << 7, (this->nodeBounds[(n << 2) + 1] & 0xFF) << 7, (this->nodeBounds[(n << 2) + 2] & 0xFF) << 7, (this->nodeBounds[(n << 2) + 3] & 0xFF) << 7, true)) {
		return;
	}
	++this->nodeCount;
	if ((this->nodeOffsets[n] & 0xFFFF) == 0xFFFF) {
		if (this->numVisibleNodes < 256) {
			this->nodeIdxs[this->numVisibleNodes++] = n;
		}
		++this->leafCount;
		if (!this->skipLines) {
			this->drawNodeLines(n);
		}

		for (short n2 = this->nodeSprites[n]; n2 != -1; n2 = this->mapSprites[this->S_NODENEXT + n2]) {
			int n3 = this->mapSpriteInfo[n2];
			if ((n3 & 0x10000) == 0x0) {
				int n4 = this->mapSpriteInfo[n2] & 0xFF;
				Entity* entity = nullptr;
				if (this->mapSprites[this->S_ENT + n2] != -1) {
					entity = &app->game->entities[this->mapSprites[this->S_ENT + n2]];
				}
				if (entity != nullptr && (n3 & 0x400000) != 0x0) {
					n4 += 257;
					if (n4 < 450) {
						this->occludeSpriteLine(n2);
					}
				}
			}
		}
		return;
	}

	int numVisibleNodes = this->numVisibleNodes;
	if (nodeClassifyPoint(n, this->viewX, this->viewY, this->viewZ) >= 0) {
		walkNode(this->nodeChildOffset1[n]);
		walkNode(this->nodeChildOffset2[n]);
	}
	else {
		walkNode(this->nodeChildOffset2[n]);
		walkNode(this->nodeChildOffset1[n]);
	}
	for (short n5 = this->nodeSprites[n]; n5 != -1; n5 = this->mapSprites[this->S_NODENEXT + n5]) {
		this->addSplitSprite(numVisibleNodes, n5);
	}
}

int Render::dot(int n, int n2, int n3, int n4) {
	return (n * n3) + (n2 * n4);
}

int Render::CapsuleToCircleTrace(int* array, int n, int n2, int n3, int n4) {
	int n5 = array[2] - array[0];
	int n6 = array[3] - array[1];
	int n7 = n2 - array[0];
	int n8 = n3 - array[1];
	int n9 = n5 * n5 + n6 * n6;
	int n10 = n7 * n5 + n8 * n6;
	if (n9 == 0) {
		return 0;
	}
	if (n10 < 0) {
		n10 = 0;
	}
	if (n10 > n9) {
		n10 = n9;
	}
	int n11 = (int)(((int64_t)n10 << 16) / n9);
	int n12 = n2 - (array[0] + (n5 * n11 >> 16));
	int n13 = n3 - (array[1] + (n6 * n11 >> 16));
	if (n12 * n12 + n13 * n13 < n + n4) {
		return (n11 >> 2) - 1;
	}
	return 16384;
}

int Render::CapsuleToLineTrace(int* array, int n, int* array2) {
	int n2 = array[2] - array[0];
	int n3 = array[3] - array[1];
	int n4 = array2[2] - array2[0];
	int n5 = array2[3] - array2[1];
	int n6 = array[0] - array2[0];
	int n7 = array[1] - array2[1];
	int dot = this->dot(n2, n3, n2, n3);
	int dot2 = this->dot(n2, n3, n4, n5);
	int dot3 = this->dot(n4, n5, n4, n5);
	int dot4 = this->dot(n2, n3, n6, n7);
	int dot5 = this->dot(n4, n5, n6, n7);
	int64_t n9;
	int64_t n8;
	int64_t n10;
	int64_t n11;
	if ((n8 = (n9 = dot * (int64_t)dot3 - dot2 * (int64_t)dot2)) < 0LL) {
		n10 = 0LL;
		n9 = 1LL;
		n11 = dot5;
		n8 = dot3;
	}
	else {
		n10 = dot2 * (int64_t)dot5 - dot3 * (int64_t)dot4;
		n11 = dot * (int64_t)dot5 - dot2 * (int64_t)dot4;
		if (n10 < 0LL) {
			n10 = 0LL;
			n11 = dot5;
			n8 = dot3;
		}
		else if (n10 > n9) {
			n10 = n9;
			n11 = dot5 + dot2;
			n8 = dot3;
		}
	}
	if (n11 < 0LL) {
		n11 = 0LL;
		if (-dot4 < 0) {
			n10 = 0LL;
		}
		else if (-dot4 > dot) {
			n10 = n9;
		}
		else {
			n10 = -dot4;
			n9 = dot;
		}
	}
	else if (n11 > n8) {
		n11 = n8;
		if (-dot4 + dot2 < 0) {
			n10 = 0LL;
		}
		else if (-dot4 + dot2 > dot) {
			n10 = n9;
		}
		else {
			n10 = -dot4 + dot2;
			n9 = dot;
		}
	}
	int n12;
	if (n10 == 0LL) {
		n12 = 0;
	}
	else {
		n12 = (int)((n10 << 16) / n9);
	}
	int n13;
	if (n11 == 0LL) {
		n13 = 0;
	}
	else {
		n13 = (int)((n11 << 16) / n8);
	}
	int n14 = (n6 << 16) + n12 * n2 - n13 * n4 >> 16;
	int n15 = (n7 << 16) + n12 * n3 - n13 * n5 >> 16;
	if (this->dot(n14, n15, n14, n15) < n) {
		return (n12 >> 2) - 1;
	}
	return 16384;
}

int Render::traceWorld(int n, int* array, int n2, int* array2, int n3) {
	if (array2[0] > ((this->nodeBounds[(n << 2) + 2] & 0xFF) << 3)) {
		return 16384;
	}
	if (array2[2] < ((this->nodeBounds[(n << 2) + 0] & 0xFF) << 3)) {
		return 16384;
	}
	if (array2[1] > ((this->nodeBounds[(n << 2) + 3] & 0xFF) << 3)) {
		return 16384;
	}
	if (array2[3] < ((this->nodeBounds[(n << 2) + 1] & 0xFF) << 3)) {
		return 16384;
	}
	if ((this->nodeOffsets[n] & 0xFFFF) == 0xFFFF) {
		int n4 = 16384;
		short n5 = this->nodeChildOffset2[n];
		int n6 = (n5 & 0x3FF) + (n5 >> 10 & 0x3F);
		int i = n5 & 0x3FF;
		while (i < n6) {
			int* traceLine = this->traceLine;
			int n7 = this->lineFlags[i >> 1] >> ((i & 0x1) << 2) & 0xF & 0x7;
			traceLine[0] = (this->lineXs[(i << 1) + 0] & 0xFF) << 3;
			traceLine[2] = (this->lineXs[(i << 1) + 1] & 0xFF) << 3;
			traceLine[1] = (this->lineYs[(i << 1) + 0] & 0xFF) << 3;
			traceLine[3] = (this->lineYs[(i << 1) + 1] & 0xFF) << 3;
			++i;
			if (n7 != 4) {
				if (n7 == 6) {
					continue;
				}
				if (n7 == 5 && (n3 & 0x10) == 0x0 && (n3 & 0x800) == 0x0) {
					continue;
				}
				if (n7 == 7 && (traceLine[0] - array[0]) * (traceLine[3] - traceLine[1]) + (traceLine[1] - array[1]) * -(traceLine[2] - traceLine[0]) <= 0) {
					continue;
				}
				if (traceLine[0] > array2[2] && traceLine[2] > array2[2]) {
					continue;
				}
				if (traceLine[0] < array2[0] && traceLine[2] < array2[0]) {
					continue;
				}
				if (traceLine[1] > array2[3] && traceLine[3] > array2[3]) {
					continue;
				}
				if (traceLine[1] < array2[1] && traceLine[3] < array2[1]) {
					continue;
				}
				int capsuleToLineTrace = this->CapsuleToLineTrace(array, n2 * n2, traceLine);
				if (capsuleToLineTrace >= n4) {
					continue;
				}
				n4 = capsuleToLineTrace;
			}
		}
		return n4;
	}
	if (this->nodeClassifyPoint(n, array[0] << 4, array[1] << 4, this->viewZ) >= 0) {
		int traceWorld = this->traceWorld(this->nodeChildOffset1[n], array, n2, array2, n3);
		if (traceWorld == 16384) {
			return this->traceWorld(this->nodeChildOffset2[n], array, n2, array2, n3);
		}
		return traceWorld;
	}
	else {
		int traceWorld2 = traceWorld(this->nodeChildOffset2[n], array, n2, array2, n3);
		if (traceWorld2 == 16384) {
			return traceWorld(this->nodeChildOffset1[n], array, n2, array2, n3);
		}
		return traceWorld2;
	}
}


#include <iostream>
#include <array>
#include <string>
#include <vector>

// Subdivide Code From http://coliru.stacked-crooked.com/a/b01db8e00f50c872
using Position5 = std::array<float, 5>;
using Plane = std::array<Position5, 4>;

Position5& operator *= (Position5& pos, const float scale) {
	for (auto& x : pos) x *= scale;
	return pos;
}

Position5 operator * (const Position5& pos, const float scale) {
	Position5 result = pos;
	result *= scale;
	return result;
}

Position5& operator += (Position5& left, const Position5& right) {
	for (size_t i = 0; i < 5; i++)
		left[i] += right[i];

	return left;
}

Position5 operator + (const Position5& left, const Position5& right) {
	Position5 result = left;
	result += right;
	return result;
}

Position5& operator -= (Position5& left, const Position5& right) {
	for (size_t i = 0; i < 5; i++)
		left[i] -= right[i];

	return left;
}

Position5 operator - (const Position5& left, const Position5& right) {
	Position5 result = left;
	result -= right;
	return result;
}


/* vertices by index in quad:
	3---------2
	|         |
	|         |
	|         |
	0---------1
*/
std::vector<Plane> subdivide(const Plane& plane, size_t iterations = 1)
{
	if (iterations == 0) {
		return{ plane };
	}
	else {
		Position5 e1 = plane[1] - plane[0];
		Position5 e2 = plane[2] - plane[3];
		Position5 e3 = plane[3] - plane[0];
		Position5 e4 = plane[2] - plane[1];

		Position5 p1 = e1 * 0.5f + plane[0];
		Position5 p2 = e2 * 0.5f + plane[3];
		Position5 p3 = e3 * 0.5f + plane[0];
		Position5 p4 = e4 * 0.5f + plane[1];

		Position5 e5 = p2 - p1;
		Position5 p5 = e5 * 0.5f + p1;

		std::vector<Plane> result{
			{ plane[0], p1, p5, p3 },
			{ p1, plane[1], p4, p5 },
			{ p5, p4, plane[2], p2 },
			{ p3, p5, p2, plane[3] }
		};

		if (iterations == 1) {
			return result;
		}
		else {
			std::vector<Plane> result2;
			for (auto& x : result) {
				auto subplanes = subdivide(x, iterations - 1);
				result2.insert(result2.end(), subplanes.begin(), subplanes.end());
			}
			return result2;
		}
	}
}

bool Render::renderStreamSpriteGL(TGLVert* array, int n) {
	Applet* app = CAppContainer::getInstance()->app;
	if (app->render->_gles->isInit) {
		Plane p = { {
			{(float)array[0].x, (float)array[0].y, (float)array[0].z, (float)array[0].s, (float)array[0].t},
			{(float)array[1].x, (float)array[1].y, (float)array[1].z, (float)array[1].s, (float)array[1].t},
			{(float)array[2].x, (float)array[2].y, (float)array[2].z, (float)array[2].s, (float)array[2].t},
			{(float)array[3].x, (float)array[3].y, (float)array[3].z, (float)array[3].s, (float)array[3].t} } };

		auto subd = subdivide(p, 3);

		TGLVert quad[4];
		int quadCnt = 0;
		for (auto& plane : subd)
		{
			for (auto& pos : plane) {
				quad[quadCnt].x = (int)pos[0];
				quad[quadCnt].y = (int)pos[1];
				quad[quadCnt].z = (int)pos[2];
				quad[quadCnt].s = (int)pos[3];
				quad[quadCnt].t = (int)pos[4];
				if (++quadCnt >= 4) {
					app->render->_gles->DrawModelVerts(quad, 4);
					quadCnt = 0;
				}
			}
		}
		return true;
	}
	return false;
}

void Render::renderStreamSprite(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	TGLVert* mv = app->tinyGL->mv;
	int n2 = this->mapSpriteInfo[n];
	short n3 = this->mapSprites[this->S_ENT + n];
	int n4 = n2 & 0xFF;
	int n5 = (n2 & 0xFF00) >> 8;
	short n6 = this->mapSprites[this->S_RENDERMODE + n];
	int n7 = this->mapSprites[this->S_SCALEFACTOR + n] << 10;
	short n8 = this->mapSprites[this->S_X + n];
	short n9 = this->mapSprites[this->S_Y + n];
	int n10 = this->mapSprites[this->S_Z + n] << 4;
	int n11, n12, n13, n14;
	if (n3 == 1) {
		mv[0].x = mv[1].x = app->canvas->destX << 4;
		mv[0].y = mv[1].y = app->canvas->destY << 4;
		mv[0].z = mv[1].z = app->canvas->destZ << 4;
		mv[2].x = mv[3].x = n8 << 4;
		mv[2].y = mv[3].y = n9 << 4;
		mv[2].z = mv[3].z = n10;

		if (app->canvas->viewPitch <= 10) {
			if (app->canvas->viewPitch < 0) {
				n11 = 128;
				n12 = 224;
			}
			else {
				n11 = 176;
				n12 = 320;
			}
		}
		else {
			n11 = 224;
			n12 = 496;
		}

		n13 = -112;
		n14 = 0;
	}
	else {
		int sprite = app->game->entities[n3].getSprite();
		mv[0].x = mv[1].x = this->mapSprites[this->S_X + sprite] << 4;
		mv[0].y = mv[1].y = this->mapSprites[this->S_Y + sprite] << 4;
		mv[0].z = mv[1].z = this->mapSprites[this->S_Z + sprite] << 4;
		mv[2].x = mv[3].x = n8 << 4;
		mv[2].y = mv[3].y = n9 << 4;
		mv[2].z = mv[3].z = n10;

		n11 = 0;
		n12 = 0;
		n13 = 0;
		n14 = -320;
	}
	app->tinyGL->faceCull = TinyGL::CULL_NONE;
	this->setupTexture(n4, n5, n6, 0);
	int n27 = app->tinyGL->imageBounds[1] - app->tinyGL->imageBounds[0];
	int n28 = app->tinyGL->imageBounds[3] - app->tinyGL->imageBounds[2];
	int n29 = (app->tinyGL->imageBounds[0] << 10) / app->tinyGL->sWidth;
	int n30 = (n27 << 10) / app->tinyGL->sWidth;
	int n31 = (app->tinyGL->tHeight - app->tinyGL->imageBounds[3] << 10) / app->tinyGL->tHeight;
	int n32 = (n28 << 10) / app->tinyGL->tHeight * 3;
	int n33 = ((n27 / 2) << 4) * n7 / 65536;
	int n34= ((n27 / 2) << 4) * n7 / 65536;

	if (n33 < 0) {
		n34 += 3;
	}

	int n35 = 0;
	app->tinyGL->viewMtxMove(&mv[0], n12, n11 - (n34 / 4), (n35 / 4) + n13);
	app->tinyGL->viewMtxMove(&mv[1], n12, n11 + (n34 / 4), n13);
	app->tinyGL->viewMtxMove(&mv[2], 0,  (n33 * 2), (n35 * 2) + n14);
	app->tinyGL->viewMtxMove(&mv[3], 0, -(n33 * 2), n14);
	int n36 = n31 - (app->time * 3 & 0x3FF);
	mv[3].s = mv[0].s = n29;
	mv[1].t = mv[0].t = n36;
	mv[2].s = mv[1].s = n29 + n30;
	mv[3].t = mv[2].t = n36 + n32;
	app->tinyGL->swapXY = false;

	if (this->renderStreamSpriteGL(app->tinyGL->mv, 4) == false) { // [GEC]
		app->tinyGL->drawModelVerts(app->tinyGL->mv, 4);
	}
}

void Render::renderSpriteObject(int n) {
	Applet* app = CAppContainer::getInstance()->app;
	//printf("renderSpriteObject %d\viewPitch", viewPitch);
	unsigned int n2 = this->mapSpriteInfo[n];
	if ((n2 & 0x10000) != 0x0) {
		return;
	}

	int n3 = n2 & 0xFF;
	int x = this->mapSprites[this->S_X + n];
	int y = this->mapSprites[this->S_Y + n];
	int z = this->mapSprites[this->S_Z + n] << 4;
	int n7 = (n2 & 0xFF00) >> 8;
	short renderMode = this->mapSprites[this->S_RENDERMODE + n];
	int scaleFactor = this->mapSprites[this->S_SCALEFACTOR + n] << 10;
	int n10 = 0;

	if ((this->mapSpriteInfo[n] & 0x400000) != 0x0) {
		n3 += 257;
	}

	if (this->useMastermindHack) {
		if (n3 == 57) {
			if (this->delayedSpriteBuffer[0] == -1) {
				this->delayedSpriteBuffer[0] = n;
				return;
			}
			this->delayedSpriteBuffer[0] = -1;
		}
		else if ((n3 >= 239 && n3 <= 247) || (n3 >= 225 && n3 <= 231)) {
			if (this->delayedSpriteBuffer[1] == -1) {
				this->delayedSpriteBuffer[1] = n;
				return;
			}
			this->delayedSpriteBuffer[1] = -1;
		}
	}

	if (this->useCaldexHack && n3 == 69) {
		if (this->delayedSpriteBuffer[2] == -1) {
			this->delayedSpriteBuffer[2] = n;
			return;
		}
		this->delayedSpriteBuffer[2] = -1;
	}

	if ((n2 & 0x80000) != 0x0) {
		n7 = (n + app->time / 100) % n7;
	}

	if (n3 == Enums::TILENUM_WATER_STREAM) { // 240
		this->renderStreamSprite(n);
		return;
	}

	if (n3 == Enums::TILENUM_OBJ_FIRE && x == app->canvas->viewX && y == app->canvas->viewY) {
		x += (app->canvas->viewStepX >> 6) * 18;
		y += (app->canvas->viewStepY >> 6) * 18;
		z -= 512;
		if (this->mapSprites[this->S_SCALEFACTOR + n] != 64) {
			scaleFactor = 65536;
		}
	}

	Entity* entity;
	EntityMonster* monster;
	EntityDef* def;
	if (this->mapSprites[this->S_ENT + n] != -1) {
		entity = &app->game->entities[this->mapSprites[this->S_ENT + n]];
		monster = entity->monster;
		def = entity->def;
	}
	else {
		entity = nullptr;
		monster = nullptr;
		def = nullptr;
	}

	if (entity != nullptr) {
		if (monster != nullptr) {
			if (!this->disableRenderActivate && app->game->activePropogators == 0 && app->game->animatingEffects == 0 && !(entity->info & 0x1050000) && !(monster->flags & Enums::MFLAG_NOACTIVATE) && !app->player->noclip && !app->game->disableAI) {
				app->game->trace(app->canvas->viewX, app->canvas->viewY, app->canvas->viewZ, x, y, z >> 4, app->player->getPlayerEnt(), 5293, 2, true);
				if (app->game->traceEntity == entity) {
					app->game->activate(entity, true, true, true, false);
				}
			}
			int monsterEffects = monster->monsterEffects;
			int n11 = n7 & 0xF0;

			if (0x0 != (monsterEffects & 0x2)) {
				n10 |= 0x20; // RENDER_FLAG_BLUESHIFT
				renderMode = 0;
			}
			else if (0x0 != (monsterEffects & 0x8)) {
				n10 |= 0x80; // RENDER_FLAG_REDSHIFT
				renderMode = 0;
			}
			else if (0x0 != (monsterEffects & 0x1)) {
				n10 |= 0x40; // RENDER_FLAG_GREENSHIFT
				renderMode = 0;
			}
			if ((n10 & 0x4) == 0x0) { // RENDER_FLAG_GREYSHIFT
				if ((monster->flags & 0x1000) == 0x0 && (n11 == 96 || n11 == 144) && app->time > monster->frameTime) {
					monster->frameTime = 0;
					n7 = 0;
				}
				this->mapSpriteInfo[n] = ((n2 & 0xFFFF00FF) | n7 << 8);
			}
		}
		if (entity->def->eType == 10 && entity->def->eSubType == 2 && entity->param != 0) {
			if (app->time > entity->param) {
				++n7;
				entity->param = app->time + 200;
			}
			if (n7 > 3) {
				entity->param = 0;
				n7 = 3;
			}
			this->mapSpriteInfo[n] = ((n2 & 0xFFFF00FF) | n7 << 8);
		}
	}

	if ((n2 & 0x400000) != 0x0) {
		this->renderSprite(x, y, z, n3, n7, n2, renderMode, scaleFactor, n10);
	}
	else {
		if (monster != nullptr) {
			this->renderSpriteAnim(n, n7, x, y, z, n3, n2, renderMode, scaleFactor, n10);
			return;
		}
		if (n3 == Enums::TILENUM_EYE_PORTAL) {
			this->portalInView = true;
			if (this->checkPortalVisibility(x, y, z)) {
				n7 = 1;
			}
			else  {
				n7 = 0;
				int n12 = app->time / 1536 & 0x3;
				n2 = (n2 ^ (n12 & 0x1) << 17 ^ (n12 & 0x2) >> 1 << 18);
			}
			n2 &= 0xFFF7FFFF;
			if ((n2 & 0xF000000) == 0x0) {
				z += 288;
			}
		}
		else if (n3 == Enums::TILENUM_OBJ_TORCHIERE) {
			int zheight = ((10 * scaleFactor) / 65536) << 4;
			n2 ^= (n7 & 0x1) << 17;
			this->renderSprite(x, y, z + zheight, Enums::TILENUM_SFX_LIGHTGLOW1, 0, n2, Render::RENDER_ADD50, scaleFactor, n10);
		}
		else {
			if (n3 == Enums::TILENUM_WATER_SPOUT) {
				int n15 = app->time / 128;
				this->renderSprite(x, y, z, n3, (n15 & 0x1), n2, renderMode, scaleFactor, n10);
				return;
			}
			if (n3 == Enums::TILENUM_TREE_TOP) {
				int zheight = ((36 * scaleFactor) / 65536) << 4;
				this->renderSprite(x, y, z, Enums::TILENUM_TREE_TRUNK, n7, n2, renderMode, scaleFactor, n10);
				this->renderSprite(x, y, z + zheight, n3, n7, n2, renderMode, scaleFactor, n10);
				return;
			}
			if (def != nullptr && def->eType == Enums::ET_NPC) {
				this->renderSpriteAnim(n, n7, x, y, z, n3, n2, renderMode, scaleFactor, n10);
				if (entity->param != 0) {
					int n17 = 160;
					int n18 = 254;
					if (entity->param == 2) {
						n18 = 255;
					}
					this->renderSprite(x, y, z + n17, n18, 0, this->mapSpriteInfo[n] & 0xFFFDFFFF, 0, scaleFactor, n10);
				}
				return;
			}
			if (n3 == Enums::TILENUM_PRACTICE_TARGET) {
				int n19 = n2 & 0xFFFDFFFF;

				if (app->canvas->legShotTime) {
					if (app->canvas->legShotTime > app->time) {
						this->renderSprite(x, y, z, 149, 0, n19, renderMode, scaleFactor, n10);
					}
					else {
						app->canvas->legShotTime = 0;
					}
				}
				else {
					this->renderSprite(x, y, z, 149, 5, n19, renderMode, scaleFactor, n10);
				}

				if (app->canvas->bodyShotTime) {
					if (app->canvas->bodyShotTime > app->time) {
						this->renderSprite(x, y, z, 149, 2, n19, renderMode, scaleFactor, n10);
					}
					else {
						app->canvas->bodyShotTime = 0;
					}
				}
				else {
					this->renderSprite(x, y, z, 149, 6, n19, renderMode, scaleFactor, n10);
				}

				if (app->canvas->headShotTime) {
					if (app->canvas->headShotTime > app->time) {
						this->renderSprite(x, y, z, 149, 3, n19, renderMode, scaleFactor, n10);
					}
					else {
						app->canvas->headShotTime = 0;
					}
				}
				else {
					this->renderSprite(x, y, z, 149, 7, n19, renderMode, scaleFactor, n10);
				}

				this->renderSprite(x, y, z, n3, app->canvas->isZoomedIn ? 1 : 4, n19, renderMode, scaleFactor, n10);
				return;
			}
			if (n3 == Enums::TILENUM_OBJ_CORPSE || n3 == Enums::TILENUM_OBJ_OTHER_CORPSE || n3 == Enums::TILENUM_OBJ_SCIENTIST_CORPSE) {
				if (entity->param == 0 && app->canvas->state != Canvas::ST_CAMERA && !entity->hasEmptyLootSet()) {
					this->renderSprite(x, y, z, n3, n7, n2, 0, 18 * scaleFactor >> 4, 512);
				}
			}
			else if ((n3 == Enums::TILENUM_MONSTER_SENTRY_BOT || n3 == Enums::TILENUM_MONSTER_RED_SENTRY_BOT) && entity != nullptr && entity->isDroppedEntity()) {
				if (entity->param == 0 && app->canvas->state != Canvas::ST_CAMERA) {
					this->renderSprite(x, y, z, n3, n7, n2, 0, (18 * scaleFactor) >> 4, 512);
				}
			}
			else if (n3 == Enums::TILENUM_TREADMILL_SIDE || n3 == Enums::TILENUM_SENTINEL_SPIKES_DUMMY) {
				return;
			}
		}
		this->renderSprite(x, y, z, n3, n7, n2, renderMode, scaleFactor, n10);
	}
}

void Render::renderBSP() {
	Applet* app = CAppContainer::getInstance()->app;

	this->nodeCount = 0;
	this->leafCount = 0;
	this->lineCount = 0;
	this->lineRasterCount = 0;
	this->spriteCount = 0;
	this->spriteRasterCount = 0;
	this->lineTime = 0;
	this->dclTime = 0;
	this->bspTime = app->upTimeMs;
	this->numVisibleNodes = 0;
	this->numSplitSprites = 0;
	if (this->skipBSP == false) {
		this->walkNode(0);
	}

	this->bspTime = app->upTimeMs - this->bspTime;
	app->tinyGL->resetCounters();
	for (int i = this->numVisibleNodes - 1; i >= 0; --i) {
		this->drawNodeGeometry(this->nodeIdxs[i]);
		this->viewSprites = -1;
		this->addNodeSprites(this->nodeIdxs[i]);
		if (!this->skipSprites) {
			for (short viewSprites = this->viewSprites; viewSprites != -1; viewSprites = this->mapSprites[this->S_VIEWNEXT + viewSprites]) {
				++this->spriteCount;
				this->renderSpriteObject(viewSprites);
			}
			//printf("this->spriteCount %d\viewPitch", this->spriteCount);
		}
	}
	for (int j = 0; j <= 1; j++) {
		if (this->useMastermindHack && this->delayedSpriteBuffer[j] != -1) {
			this->renderSpriteObject(this->delayedSpriteBuffer[j]);
		}
	}
	if (this->useCaldexHack && this->delayedSpriteBuffer[2] != -1) {
		this->renderSpriteObject(this->delayedSpriteBuffer[2]);
	}
}

void Render::loadPlayerFog() {
	Applet* app = CAppContainer::getInstance()->app;
	this->fogLerpStart = 0;
	this->fogLerpTime = 0;
	app->tinyGL->fogMin = this->playerFogMin;
	app->tinyGL->fogRange = this->playerFogRange;
	this->buildFogTables(this->playerFogColor);
}

void Render::savePlayerFog() {
	Applet* app = CAppContainer::getInstance()->app;
	this->fogLerpStart = 0;
	this->playerFogMin = app->tinyGL->fogMin;
	this->playerFogRange = app->tinyGL->fogRange;
	this->playerFogColor = app->tinyGL->fogColor;
}

void Render::snapFogLerp() {
	Applet* app = CAppContainer::getInstance()->app;
	if (this->fogLerpTime != 0) {
		this->fogLerpStart = 0;
		this->fogLerpTime = 0;
		app->tinyGL->fogMin = this->destFogMin;
		app->tinyGL->fogRange = this->destFogRange;
	}
}

void Render::startFogLerp(int n, int n2, int fogLerpTime) {
	Applet* app = CAppContainer::getInstance()->app;

	this->baseFogMin = app->tinyGL->fogMin;
	this->baseFogRange = app->tinyGL->fogRange;
	this->destFogMin = n << 4;
	this->destFogRange = n2 - n << 4;
	if (this->destFogRange == 0) {
		this->destFogRange = 1;
	}
	if (fogLerpTime != 0) {
		this->fogLerpStart = app->time;
		this->fogLerpTime = fogLerpTime;
	}
	else {
		this->fogLerpStart = 0;
		this->fogLerpTime = 0;
		app->tinyGL->fogMin = this->destFogMin;
		app->tinyGL->fogRange = this->destFogRange;
	}
}

void Render::buildFogTable() {
	int fogTableFrac = this->fogTableFrac;
	int fogTableColor = this->fogTableColor;
	uint16_t* fogTableBase = this->fogTableBase;
	uint16_t* fogTableDest = this->fogTableDest;
	for (int i = 0; i < this->fogTableBaseSize; i++) {
		uint16_t dat = fogTableBase[i];
		fogTableDest[i] = (((fogTableFrac >> 2) * ((dat & 0x07e0) >> 6)) & 0x07e0 | (((fogTableFrac >> 3) * (dat & 0xF81F)) >> 5) & 0xF81F) + fogTableColor;
	}
}

void Render::buildFogTable(int n, int n2, int n3) {
	int n4 = (n3 & 0xFF000000) >> 24 & 0xFF;
	int n5 = this->mediaMappings[n] + n2;
	if ((this->mediaPalColors[n5] & 0x4000) != 0x0) {
		uint16_t** array = this->mediaPalettes[this->mediaPalColors[n5] & 0x3FFF];
		this->fogTableBase = array[1];
		this->fogTableBaseSize = this->mediaPalettesSizes[this->mediaPalColors[n5] & 0x3FFF]; // Pc port only
		for (int i = 1; i < 16; ++i) {
			int n6 = (i << 8) / 16 * n4 >> 8;
			this->fogTableColor = (((n3 & 0xFF00FF00) >> 8) * n6 & 0xFF00FF00) | ((n3 & 0xFF00FF) * n6 >> 8 & 0xFF00FF);
			this->fogTableColor = Render::upSamplePixel(this->fogTableColor);
			this->fogTableFrac = 256 - n6;
			this->fogTableDest = array[i];
			this->buildFogTable();
		}
	}
}

void Render::buildFogTables(int fogColor) {
	Applet* app = CAppContainer::getInstance()->app;

	app->tinyGL->fogColor = fogColor;

	if ((fogColor & 0xFF000000) == 0x0) {
		app->tinyGL->fogMin = 32752;
		app->tinyGL->fogRange = 1;
		return;
	}

	int n = (fogColor & 0xFF000000) >> 24 & 0xFF;
	for (int i = 1; i < 16; ++i) {
		int n2 = (i << 8) / 16 * n >> 8;
		this->fogTableColor = Render::upSamplePixel((((app->tinyGL->fogColor & 0xFF00FF00) >> 8) * n2 & 0xFF00FF00) | ((app->tinyGL->fogColor & 0xFF00FF) * n2 >> 8 & 0xFF00FF));
		this->fogTableFrac = 256 - n2;
		for (int j = 0; j < 1024; ++j) {
			if (this->mediaPalettes[j][0] != nullptr) {
				this->fogTableBase = this->mediaPalettes[j][0];
				this->fogTableDest = this->mediaPalettes[j][i];
				this->fogTableBaseSize = this->mediaPalettesSizes[j]; // Pc port only
				buildFogTable();
			}
		}
		this->fogTableBase = this->skyMapPalette[0];
		this->fogTableDest = this->skyMapPalette[i];
		this->fogTableBaseSize = 256; // Pc port only
		this->buildFogTable();
	}
	this->buildFogTable(234, 0, ((n * 180 >> 8 & 0xFF) << 24 | (app->tinyGL->fogColor & 0xFFFFFF)));
	this->buildFogTable(234, 0, 0xFF000000); // IOS
}

void Render::setupPalette(uint16_t* spanPalette, int renderMode, int renderFlags) {
	// Pendiente
	Applet* app = CAppContainer::getInstance()->app;

	bool isMultiply = (renderFlags & Render::RENDER_FLAG_MULTYPLYSHIFT) ? true : false; // [GEC]

	uint16_t* array = app->tinyGL->scratchPalette;
	app->tinyGL->spanPalette = spanPalette;
	if (renderFlags & 0x4) { // RENDER_FLAG_GREYSHIFT
		for (int i = 0; i < app->tinyGL->paletteBaseSize; ++i) {
			uint16_t n2 = spanPalette[i];
			//int n3 = ((n2 & 0xFF) + (n2 >> 8 & 0xFF) + (n2 >> 16 & 0xFF)) / 3;
			//array[i] = (n3 << 16 | n3 << 8 | n3);

			//__int64 v10 = 0x55555556 * (__int64)(LOBYTE(*(uint16_t*)&spanPalette[i]) + HIBYTE(*(uint16_t*)&spanPalette[i]));
			//array[i] = (uint16_t)((int)v10 << 8) | (uint16_t)v10;

			uint16_t grayColor = (((n2 & 0xf800) >> 10) + ((n2 >> 5) & 0x3f) + ((n2 & 0x1f) << 1)) / 3; //RGB
			array[i] = ((grayColor >> 1) << 11) | (grayColor << 5) | (grayColor >> 1);
		}
		app->tinyGL->paletteBase = &app->tinyGL->scratchPalette;
		app->tinyGL->spanPalette = array;
	}
	else if ((renderFlags & 0x1E8) != 0x0) { //(RENDER_FLAG_WHITESHIFT | RENDER_FLAG_BLUESHIFT | RENDER_FLAG_GREENSHIFT | RENDER_FLAG_REDSHIFT | RENDER_FLAG_BRIGHTREDSHIFT)
		int n4 = 0;
		switch (renderFlags & 0x1E8) {
			case 8: { // RENDER_FLAG_WHITESHIFT
					n4 = Render::RGB888ToRGB565(64, 64, 64); // 16904;
				break;
			}
			case 32: { // RENDER_FLAG_BLUESHIFT
				if (isMultiply) {
					n4 = Render::RGB888ToRGB565(255, 255, 0); // color inverted 0x0000FF
				}
				else {
					n4 = Render::RGB888ToRGB565(0, 0, 64); // 8;
				}
				break;
			}
			case 64: { // RENDER_FLAG_GREENSHIFT
				if (isMultiply) {
					n4 = Render::RGB888ToRGB565(255, 0, 255); // color inverted 0x00FF00
				}
				else {
					n4 = Render::RGB888ToRGB565(0, 64, 0); // 512;
				}
				break;
			}
			case 128: { // RENDER_FLAG_REDSHIFT
				if (isMultiply) {
					n4 = Render::RGB888ToRGB565(0, 255, 255); // color inverted 0xFF0000
				}
				else {
					n4 = Render::RGB888ToRGB565(64, 0, 0); // 16384;
				}
				break;
			}
			case 256: { // RENDER_FLAG_BRIGHTREDSHIFT
				n4 = Render::RGB888ToRGB565(128, 0, 0); // 32768; // Player Only
				break;
			}
		}
		for (int j = 0; j < app->tinyGL->paletteBaseSize; ++j) {
			uint32_t n5, n6, n7;
			if (isMultiply) {
				n5 = (spanPalette[j] | 0x10821) - (n4 & 0xFFFFF7DE);
				n6 = n5 & 0x10820;
				n7 = (n5 ^ n6) & n6 - (n6 >> 4);
			}
			else {
				n5 = (spanPalette[j] & 0xF7DE) + (n4 & 0xffff);
				n6 = (n5 & 0x10820);
				n7 = (uint16_t)n5 ^ (uint16_t)n6;
				if (n6 & 0x1f000) { n7 |= 0xF800; }
				if (n6 & 0xfc0) { n7 |= 0x7E0; }
				if (n6 & 0x3e) { n7 |= 0x1F; }
			}
			
			array[j] = n7;
		}
		app->tinyGL->paletteBase = &app->tinyGL->scratchPalette;
		app->tinyGL->spanPalette = array;
	}
	else if ((renderFlags & 0x200) != 0x0) { // RENDER_FLAG_PULSATE
		int n7 = app->time >> 2 & 0x1FF;
		if ((n7 & 0x100) != 0x0) {
			n7 = 511 - n7;
		}
		uint16_t n8 = (n7 >> 1) + 127;
		uint16_t rgb565 = Render::upSamplePixel(n8 << 16 | n8 << 8 | n8);
		for (int k = 0; k < app->tinyGL->paletteBaseSize; ++k) {
			array[k] = rgb565;
		}
		app->tinyGL->paletteBase = &app->tinyGL->scratchPalette;
		app->tinyGL->spanPalette = array;
	}
	else if (renderMode != 0) {
		app->tinyGL->paletteBase = &app->tinyGL->scratchPalette;
		app->tinyGL->spanPalette = array;
		int n9 = 0;
		int n10 = 0;
		switch (renderMode) {
			default: {
				n9 = 0xFFFFE79C;
				n10 = 1;
				break;
			}
			case 1: {
				n9 = 0xFFFFE79C;
				n10 = 2;
				break;
			}
			case 3: {
				n9 = 0xFFFFF7DE;
				n10 = 0;
				break;
			}
			case 5: {
				n9 = 0xFFFFE79C;
				n10 = 1;
				break;
			}
			case 6: {
				n9 = 0xFFFFC718;
				n10 = 2;
				break;
			}
			case 4: {
				for (int l = 0; l < app->tinyGL->paletteBaseSize; ++l) {
					array[l] = ((spanPalette[l] & 0xFFFFE79C) >> 1) + ((spanPalette[l] & 0xFFFFC718) >> 2);
				}
				return;
			}
		}
		for (int n11 = 0; n11 < app->tinyGL->paletteBaseSize; ++n11) {
			array[n11] = (spanPalette[n11] & n9) >> n10; 
		}
	}
}

int (*Render::getImageFrameBounds(int n, int n2, int n3, int n4))[4]{
	short n5 = this->mediaMappings[n];
	this->temp[0] = n2 + n5 << 2;
	this->temp[1] = n3 + n5 << 2;
	this->temp[2] = n4 + n5 << 2;
	for (int i = 0; i < 3; ++i) {
		this->imageFrameBounds[i][0] = (this->mediaBounds[this->temp[i] + 0] & 0xFF) * 64 / 176 - 32;
		this->imageFrameBounds[i][1] = (this->mediaBounds[this->temp[i] + 1] & 0xFF) * 64 / 176 - 32;
		this->imageFrameBounds[i][2] = (176 - (this->mediaBounds[this->temp[i] + 3] & 0xFF)) * 64 / 176 - 32;
		this->imageFrameBounds[i][3] = (176 - (this->mediaBounds[this->temp[i] + 2] & 0xFF)) * 64 / 176 - 32;
	}
	return this->imageFrameBounds;
}

uint16_t* Render::getPalette(int n, int n2, int n3) {
	return this->mediaPalettes[this->mediaPalColors[this->mediaMappings[n] + n2] & 0x3FFF][n3];
}

void Render::setupTexture(int n, int n2, int renderMode, int renderFlags) {
	Applet* app = CAppContainer::getInstance()->app;

	int n4 = this->mediaMappings[n] + n2;

	if ((app->canvas->state == Canvas::ST_AUTOMAP) || (this->renderMode & 0x10) == 0x0) {
		renderMode = 10;
	}
	else if ((this->renderMode & 0x20)) {
		renderMode = 9;
	}
	if (this->_gles->isInit) { // New
		this->_gles->SetupTexture(n, n2, renderMode, renderFlags);
	}

	if (n < 257 && n != 175 && n != 162 && n != 129 && n != 173 && n != 184) {
		app->tinyGL->span = &this->_spanTrans[renderMode];
	}
	else {
		app->tinyGL->span = &this->_spanTexture[renderMode];
	}

	int n5;
	int n6;
	if (n == Enums::TILENUM_SKY_BOX) {
		app->tinyGL->textureBase = this->skyMapTexels;
		app->tinyGL->paletteBase = this->skyMapPalette;
		app->tinyGL->textureBaseSize = 256 * 256; // new
		app->tinyGL->paletteBaseSize = 256; // new
		app->tinyGL->mediaID = -1; // new
		this->isSkyMap = true;
		n5 = 8;
		n6 = 8;

	}
	else {
		app->tinyGL->textureBase = this->mediaTexels[this->mediaTexelSizes[n4] & 0x3FFF];
		app->tinyGL->paletteBase = this->mediaPalettes[this->mediaPalColors[n4] & 0x3FFF];
		app->tinyGL->textureBaseSize = this->mediaTexelSizes2[this->mediaTexelSizes[n4] & 0x3FFF]; // [GEC] new
		app->tinyGL->paletteBaseSize = this->mediaPalettesSizes[this->mediaPalColors[n4] & 0x3FFF]; // [GEC] new
		app->tinyGL->mediaID = n4; // new

		// [GEC] new
		app->tinyGL->paletteTransparentMask = -1;
		for (int i = 0; i < app->tinyGL->paletteBaseSize; i++) {
			if(app->tinyGL->paletteBase[0][i] == 0xF81F) {
				app->tinyGL->paletteTransparentMask = i;
			}
		}

		uint8_t b = this->mediaDimensions[n4];
		n6 = (b >> 4 & 0xF);
		n5 = (b & 0xF);
		app->tinyGL->imageBounds[0] = (uint16_t)(this->mediaBounds[(n4 << 2) + 0] & 0xFFF);
		app->tinyGL->imageBounds[1] = (uint16_t)(this->mediaBounds[(n4 << 2) + 1] & 0xFFF);
		app->tinyGL->imageBounds[2] = (uint16_t)(this->mediaBounds[(n4 << 2) + 2] & 0xFFF);
		app->tinyGL->imageBounds[3] = (uint16_t)(this->mediaBounds[(n4 << 2) + 3] & 0xFFF);
		this->isSkyMap = false;
	}
	app->tinyGL->sWidth = 1 << n6;
	app->tinyGL->sShift = 26 - n6;
	app->tinyGL->sMask = app->tinyGL->sWidth - 1;
	app->tinyGL->tHeight = 1 << n5;
	app->tinyGL->tShift = 26 - (n5 + n6);
	app->tinyGL->tMask = (app->tinyGL->tHeight - 1) * app->tinyGL->sWidth;
}

void Render::drawSkyMap(int n2) {
	Applet* app = CAppContainer::getInstance()->app;

	uint8_t* p_skyMapTexels; // r4
	uint16_t* FogPalette; // r0
	__int16 v7; // r2
	__int16 v9; // r12
	int v11; // lr
	int v12; // r5
	int v13; // r12
	uint16_t* pixels; // r1
	int v15; // r3
	int v16; // r3

	p_skyMapTexels = this->skyMapTexels;
	app->tinyGL->paletteBase = this->skyMapPalette;
	FogPalette = app->tinyGL->getFogPalette(0x10000000);

	v7 = n2 + 255;
	if (n2 >= 0) {
		v7 = n2;
	}
	v9 = (v7 & 0xFF00) + (app->tinyGL->viewportY << 8) - 256;

	// Original BREW version
#if 0
	for (int i = app->tinyGL->viewportY; i < app->tinyGL->viewportY2; i++) {
		v11 = v9 & 0xFF00;
		v12 = app->tinyGL->viewportX2 - app->tinyGL->viewportX;
		v13 = n2;
		pixels = &app->tinyGL->pixels[app->tinyGL->viewportX + i * app->tinyGL->screenWidth];
		while (v12 >= 8) {
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
			v12 -= 8;
		}
		while (--v12 >= 0) {
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 & 255)]];
			v13++;
		}
		v9 = v11 + 256;
	}
#endif

	// [GEC] It is adjusted like this to be consistent with the IOS version
	for (int i = app->tinyGL->viewportY; i < app->tinyGL->viewportY2; i++) {
		v11 = v9 & 0xFF00;
		v12 = app->tinyGL->viewportX2 - app->tinyGL->viewportX;
		v13 = (n2 + 132) << 8;
		pixels = &app->tinyGL->pixels[app->tinyGL->viewportX + i * app->tinyGL->screenWidth];
		while (v12 >= 8) {
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
			v12 -= 8;
		}
		while (--v12 >= 0) {
			*pixels++ = FogPalette[p_skyMapTexels[v11 | (v13 >> 8 & 255)]];
			v13 += 128;
		}
		v9 = v11 + 256;
	}
}

void Render::render(int viewX, int viewY, int viewZ, int viewAngle, int viewPitch, int viewRoll, int viewFov) {
	Applet* app = CAppContainer::getInstance()->app;

	//printf("Render::render\n");

	// Original BREW version
	//this->skyMapX = ((1023 - viewAngle) / 4) << 3;
	//this->skyMapY = (((256 - this->screenHeight) / 2) << 3) + (((1023 - viewPitch) / 2) << 3);

	// [GEC] It is adjusted like this to be consistent with the IOS version
	this->skyMapX = ((1023 - (viewAngle & 0x3FF))) << 3;
	this->skyMapY = ((1023 - 0 /*viewPitch*/)) << 3;

	this->currentFrameTime = app->upTimeMs;

	int viewAspect = (viewFov << 14) / ((app->tinyGL->viewportWidth << 14) / app->tinyGL->viewportHeight);
	int n6 = 0;
	int max = std::max(app->player->statusEffects[33], app->player->statusEffects[34]);
	if (max != this->destDizzy) {
		this->baseDizzy = this->destDizzy;
		this->destDizzy = max;
		this->dizzyTime = app->time;
	}
	if (this->destDizzy != this->baseDizzy && this->dizzyTime + 2048 > app->time) {
		max = this->baseDizzy + ((this->destDizzy - this->baseDizzy) * ((app->time - this->dizzyTime << 16) / 2048) >> 16);
	}
	int n7 = (max << 8) / 30;
	if (app->canvas->state == Canvas::ST_CAMERA || app->menuSystem->menu == Menus::MENU_LEVEL_STATS) {
		n7 = 0;
	}
	if (app->game->scriptStateVars[6] != 0) {
		n6 = 8;
	}
	else if (n7 != 0) {
		n6 = 8 * n7 >> 8;
	}
	if (n6 != 0) {
		viewFov = viewFov - n6 + (n6 * this->sinTable[app->time / 2 & 0x3FF] >> 16);
		viewAspect = viewAspect - n6 + (n6 * this->sinTable[app->time / 2 + 256 & 0x3FF] >> 16);
	}
	if (app->time < this->rockViewTime + this->rockViewDur) {
		int n8 = this->sinTable[(app->time - this->rockViewTime << 16) / (this->rockViewDur << 8) + 256 & 0x3FF] >> 8;
		viewX += n8 * (this->rockViewX - viewX << 8) >> 16;
		viewY += n8 * (this->rockViewY - viewY << 8) >> 16;
		viewZ += n8 * (this->rockViewZ - viewZ << 8) >> 16;
	}
	if (n7 != 0) {
		int time = app->time;
		int n9 = (this->sinTable[time * 5 / 8 & 0x3FF] >> 8) * n7 >> 8;
		int n10 = (this->sinTable[time * 4 / 8 + 256 & 0x3FF] >> 8) * n7 >> 8;
		int n11 = (this->sinTable[time * 6 / 8 & 0x3FF] >> 8) * n7 >> 8;
		int n12 = (this->sinTable[time * 3 / 8 & 0x3FF] >> 8) * n7 >> 8;
		viewX += n9 * 12288 >> 16;
		viewY += n10 * 12288 >> 16;
		viewZ += n11 * 8192 >> 16;
		viewAngle += n12 * 2048 >> 16;
	}
	if (app->time < app->canvas->shakeTime) {
		int n13 = this->sinTable[viewAngle + 256 & 0x3FF];
		viewX += (app->canvas->shakeX << 4) * this->sinTable[viewAngle + 256 + 256 & 0x3FF] >> 16;
		viewY += (app->canvas->shakeX << 4) * -n13 >> 16;
		viewZ += app->canvas->shakeY << 4;
	}
	if (app->player->isFamiliar) {
		viewZ += (this->sinTable[app->time >> 2 & 0x3FF] >> 11) + 128;
	}
	this->viewSin = this->sinTable[viewAngle & 0x3FF];
	this->viewCos = this->sinTable[viewAngle + 256 & 0x3FF];
	int n14 = viewAngle - 256 & 0x3FF;
	this->viewRightStepX = this->sinTable[n14 + 256 & 0x3FF] >> 10;
	this->viewRightStepY = -this->sinTable[n14 & 0x3FF] >> 10;
	if (!this->skipViewNudge) {
		viewX -= 160 * this->viewCos >> 16;
		viewY += 160 * this->viewSin >> 16;
	}

	if (this->chatZoom) { // WolfRpg ?
		Entity* curTarget = app->combat->curTarget;
		if (curTarget && ((curTarget->def->eType == 2) || (curTarget->def->eType == 3))) {
			int sprite = curTarget->getSprite();
			viewX = (-(32 * this->viewCos) >> 12) + (app->render->mapSprites[this->S_X + sprite] << 4);
			viewY = ( (32 * this->viewSin) >> 12) + (app->render->mapSprites[this->S_Y + sprite] << 4);
			viewZ = (4 + app->render->mapSprites[sprite + this->S_Z + sprite]) << 4;
			viewPitch = 0;
		}
	}

	this->viewX = viewX;
	this->viewY = viewY;
	this->viewZ = viewZ;
	this->viewAngle = viewAngle;
	app->tinyGL->setView(this->viewX, this->viewY, this->viewZ, viewAngle, viewPitch, viewRoll, viewFov, viewAspect);
	if (this->fogLerpTime != 0) {
		if (app->time < this->fogLerpStart + this->fogLerpTime) {
			int n15 = (app->time - this->fogLerpStart << 12) / this->fogLerpTime;
			app->tinyGL->fogMin = this->baseFogMin + ((this->destFogMin - this->baseFogMin) * n15 >> 12);
			app->tinyGL->fogRange = this->baseFogRange + ((this->destFogRange - this->baseFogRange) * n15 >> 12);
			if (app->tinyGL->fogRange == 0) {
				app->tinyGL->fogRange = 1;
			}
		}
		else {
			this->fogLerpTime = 0;
			app->tinyGL->fogMin = this->destFogMin;
			app->tinyGL->fogRange = this->destFogRange;
		}
	}
	for (int i = 0; i < this->screenWidth; ++i) {
		app->tinyGL->columnScale[i] = TinyGL::COLUMN_SCALE_INIT;
	}

	//app->game->scriptStateVars[5] = 1; // IOS

	this->clearColorBuffer = app->upTimeMs;
	if (app->canvas->state != Canvas::ST_AUTOMAP) {
		if ((this->renderMode & 0x20) != 0x0) {
			app->tinyGL->clearColorBuffer(0xFFFF00FF);
		}
		else if (this->skyMapTexels != nullptr && app->game->scriptStateVars[5] != 0 && !(this->renderMode & 0x20)) {
			if (!this->_gles->DrawSkyMap()) {
				this->drawSkyMap((this->skyMapX >> 3) + ((this->skyMapY >> 3) << 8));
			}
		}
		else {
			int fogColor = 0;
			if (app->tinyGL->fogRange > 1) {
				fogColor = app->tinyGL->fogColor;
			}
			app->tinyGL->clearColorBuffer(fogColor);
		}
	}
	this->clearColorBuffer = app->upTimeMs - this->clearColorBuffer;
	app->tinyGL->unk04 = 0;

	this->portalInView = false;
	this->closestPortalDist = 0x7FFFFFFF;
	this->_gles->DrawSkyMap();
	this->renderBSP();
	if (!this->portalInView) {
		this->portalState = Render::PORTAL_DNE;
		this->previousPortalState = Render::PORTAL_DNE;
	}
	this->frameTime = app->upTimeMs - this->currentFrameTime;
	this->shotsFired = false;

	this->_gles->ResetGLState();
}

void Render::unlinkSprite(int n) {
	this->unlinkSprite(n, this->mapSprites[this->S_X + n], this->mapSprites[this->S_Y + n]);
}

void Render::unlinkSprite(int n, int n2, int n3) {
	if (this->mapSprites[this->S_NODE + n] != -1) {
		short n4 = this->mapSprites[this->S_NODE + n];
		if (this->nodeSprites[n4] == n) {
			this->nodeSprites[n4] = this->mapSprites[this->S_NODENEXT + n];
		}
		else {
			short n5;
			for (n5 = this->nodeSprites[n4]; n5 != -1 && this->mapSprites[this->S_NODENEXT + n5] != n; n5 = this->mapSprites[this->S_NODENEXT + n5]) {}
			if (n5 != -1) {
				this->mapSprites[this->S_NODENEXT + n5] = this->mapSprites[this->S_NODENEXT + n];
			}
		}
	}
}

void Render::relinkSprite(int n) {
	relinkSprite(n, this->mapSprites[this->S_X + n] << 4, this->mapSprites[this->S_Y + n] << 4, this->mapSprites[this->S_Z + n] << 4);
}

void Render::relinkSprite(int n, int n2, int n3, int n4) {
	if (this->mapSprites[this->S_NODE + n] != -1) {
		short n5 = this->mapSprites[this->S_NODE + n];
		if (this->nodeSprites[n5] == n) {
			this->nodeSprites[n5] = this->mapSprites[this->S_NODENEXT + n];
		}
		else {
			short n6;
			for (n6 = this->nodeSprites[n5]; n6 != -1 && this->mapSprites[this->S_NODENEXT + n6] != n; n6 = this->mapSprites[this->S_NODENEXT + n6]) {}
			if (n6 != -1) {
				this->mapSprites[this->S_NODENEXT + n6] = this->mapSprites[this->S_NODENEXT + n];
			}
		}
	}
	short nodeForPoint = this->getNodeForPoint(n2, n3, n4, this->mapSpriteInfo[n]);
	if ((this->mapSprites[this->S_NODE + n] = nodeForPoint) != -1) {
		this->mapSprites[this->S_NODENEXT + n] = this->nodeSprites[nodeForPoint];
		this->nodeSprites[nodeForPoint] = (short)n;
	}
}

short Render::getNodeForPoint(int n, int n2, int n3, int n4) {
	short n5 = 0;
	int i = this->nodeOffsets[n5] & 0xFFFF;
	bool b = (n4 & 0xF000000) != 0x0;
	int n6 = n4 & 0xFF;
	if ((n4 & 0x400000) != 0x0) {
		n6 += 256 + 1;
	}

	bool b2 = n6 == 240;
	while (i != 65535) {
		int nodeClassifyPoint = this->nodeClassifyPoint(n5, n, n2, n3);
		if (nodeClassifyPoint == 0 && b) {
			if ((n4 & 0x9000000) != 0x0) {
				n5 = this->nodeChildOffset1[n5];
			}
			else {
				n5 = this->nodeChildOffset2[n5];
			}
		}
		else {
			if (!b2 && nodeClassifyPoint > -128 && nodeClassifyPoint < 128) {
				return n5;
			}
			if (nodeClassifyPoint > 0) {
				n5 = this->nodeChildOffset1[n5];
			}
			else {
				n5 = this->nodeChildOffset2[n5];
			}
		}
		i = (this->nodeOffsets[n5] & 0xFFFF);
	}
	int n7 = (this->nodeBounds[(n5 << 2) + 0] & 0xFF) << 7;
	int n8 = (this->nodeBounds[(n5 << 2) + 1] & 0xFF) << 7;
	int n9 = (this->nodeBounds[(n5 << 2) + 2] & 0xFF) << 7;
	int n10 = (this->nodeBounds[(n5 << 2) + 3] & 0xFF) << 7;
	if (n < n7 || n2 < n8 || n > n9 || n2 > n10) {
		return -1;
	}
	return n5;
}

int Render::getHeight(int x, int y) {
	if (this->heightMap == nullptr) {
		return 0;
	}
	x &= 0x7FF;
	y &= 0x7FF;
	return this->heightMap[(y >> 6) * 32 + (x >> 6)] << 3;
}

bool Render::checkTileVisibilty(int n, int n2) {
	int n3 = n << 6;
	int n4 = n2 << 6;
	return !this->cullBoundingBox(n3 << 4, n4 << 4, n3 + 64 << 4, n4 + 64 << 4, true);
}

void Render::postProcessSprites() {
	int* mapSpriteInfo = this->mapSpriteInfo;
	short* mapSprites = this->mapSprites;
	int i;
	for (i = 0; i < this->numMapSprites; ++i) {
		mapSprites[this->S_Z + i] += (short)getHeight(mapSprites[this->S_X + i], mapSprites[this->S_Y + i]);
		if (i >= this->numNormalSprites) {
			mapSprites[this->S_Z + i] -= 32;
		}
		int n3 = (uint8_t)(mapSpriteInfo[i] & 0xFF);
		if ((mapSpriteInfo[i] & 0x400000) != 0x0) {
			n3 += 257;
		}
		if (n3 == 479) {
			mapSprites[this->S_RENDERMODE + i] = 0;
		}
		else if (n3 == 208 || n3 == 234 || n3 == 130 || n3 == 242) {
			mapSprites[this->S_RENDERMODE + i] = 3;
		}
		else if (n3 == 178) {
			mapSprites[this->S_RENDERMODE + i] = 3;
		}
		else if (n3 == 236) {
			mapSprites[this->S_RENDERMODE + i] = 3;
		}
		else if (n3 == 212) {
			mapSprites[this->S_RENDERMODE + i] = 7;
		}
		else if (n3 == 161) {
			mapSprites[this->S_RENDERMODE + i] = 2;
		}
		else if (n3 == 244) {
			mapSprites[this->S_RENDERMODE + i] = 4;
		}
		else {
			mapSprites[this->S_RENDERMODE + i] = 0;
		}
		this->relinkSprite(i);
	}
	for (int j = 0; j < 48; ++j) {
		this->customSprites[j] = i;
		mapSprites[this->S_NODE + i] = -1;
		mapSpriteInfo[i] = 0;
		mapSprites[this->S_NODENEXT + i] = -1;
		mapSprites[this->S_VIEWNEXT + i] = -1;
		mapSprites[this->S_ENT + i] = -1;
		mapSprites[this->S_RENDERMODE + i] = 0;
		mapSprites[this->S_SCALEFACTOR + i] = 64;
		mapSprites[this->S_X + i] = (mapSprites[this->S_Y + i] = 0);
		mapSprites[this->S_Z + i] = 32;
		++i;
	}
	this->firstDropSprite = i;
	for (int k = 0; k < 16; ++k) {
		this->dropSprites[k] = i;
		mapSprites[this->S_NODE + i] = -1;
		mapSpriteInfo[i] = 0;
		mapSprites[this->S_NODENEXT + i] = -1;
		mapSprites[this->S_VIEWNEXT + i] = -1;
		mapSprites[this->S_ENT + i] = -1;
		mapSprites[this->S_RENDERMODE + i] = 0;
		mapSprites[this->S_SCALEFACTOR + i] = 64;
		mapSprites[this->S_X + i] = (mapSprites[this->S_Y + i] = 0);
		mapSprites[this->S_Z + i] = 32;
		++i;
	}
}

bool Render::isFading() {
	return this->fadeFlags != 0;
}

int Render::getFadeFlags() {
	return this->fadeFlags;
}

void Render::startFade(int fadeDuration, int fadeFlags) {
	Applet* app = CAppContainer::getInstance()->app;
	this->fadeTime = app->upTimeMs;
	this->fadeDuration = fadeDuration;
	this->fadeFlags = fadeFlags;
	//printf("startFade\n");
}

void Render::endFade() {
	this->fadeFlags = 0;
	this->fadeTime = -1;
}

bool Render::fadeScene(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	int fadeRect[4] = {0, 0, 480, 320};

	int fadeDuration = app->upTimeMs - this->fadeTime;
	if (fadeDuration >= this->fadeDuration - 50) {
		fadeDuration = this->fadeDuration;
		if ((this->fadeFlags & Render::FADE_FLAG_CHANGEMAP) != 0x0) {
			this->fadeFlags &= Render::FADE_SPECIAL_FLAG_MASK;
			graphics->fade(fadeRect, 0, 0x00000000);
			app->canvas->loadMap(app->menuSystem->LEVEL_STATS_nextMap, false, false);
			return false;
		}
		if ((this->fadeFlags & Render::FADE_FLAG_SHOWSTATS) != 0x0) {
			this->fadeFlags &= Render::FADE_SPECIAL_FLAG_MASK;
			graphics->fade(fadeRect, 0, 0x00000000);
			app->canvas->saveState(51, (short)3, (short)196);
			return false;
		}
		if ((this->fadeFlags & Render::FADE_FLAG_EPILOGUE) != 0x0) {
			this->endFade();
			app->canvas->setState(Canvas::ST_EPILOGUE);
			return false;
		}
		if ((this->fadeFlags & Render::FADE_FLAG_FADEIN) != 0x0) {
			this->endFade();
			return true;
		}
		if ((this->fadeFlags & Render::FADE_FLAG_FADEOUT) != 0x0) {
			//this->fadeAll(255); // J2ME
			graphics->fade(fadeRect, 0, 0x00000000);
			return true;
		}
	}
	int alpha = 65280 * ((fadeDuration << 16) / (this->fadeDuration << 8)) >> 16;
	if ((this->fadeFlags & Render::FADE_FLAG_FADEOUT) != 0x0) {
		alpha = 256 - alpha;
	}
	graphics->fade(fadeRect, alpha, 0x00000000);

	//this->fadeAll(alpha);  // J2ME
	return true;
}

void Render::postProcessView(Graphics* graphics) {
	static float update = 0.f;
	static int y = 0;
	Applet* app = CAppContainer::getInstance()->app;

	//final int[] pixels = TinyGL.pixels;
	int screenVScrollOffset = 0;
	if (this->vScrollVelocity != 0) {
		int screenVScrollOffset2 = this->screenVScrollOffset;
		int n = -(5 * (app->time - this->lastScrollChangeTime)) / 30;
		if (this->vScrollVelocity + n < 0) {
			if (app->tinyGL->viewportHeight - screenVScrollOffset2 <= 15 || screenVScrollOffset2 <= 15) {
				screenVScrollOffset2 = 0;
				this->vScrollVelocity = 0;
			}
			else {
				this->vScrollVelocity = std::max(this->vScrollVelocity + n, -30);
			}
		}
		else {
			this->vScrollVelocity += n;
		}
		screenVScrollOffset = (screenVScrollOffset2 + this->vScrollVelocity + app->tinyGL->viewportHeight) % app->tinyGL->viewportHeight;
	}
	this->lastScrollChangeTime = app->time;
	this->screenVScrollOffset = screenVScrollOffset;
	int max = 32;
	int max2 = 0;
	if (this->brightenPostProcess) {
		if (this->brightenPPMaxReachedTime != 0 && app->time > this->brightenPPMaxReachedTime + 500 + 500) {
			app->hud->stopBrightenScreen();
		}
		else {
			int n2;
			if (this->brightenPPMaxReachedTime == 0) {
				n2 = (app->time - this->brightenPostProcessBeginTime >> 4 & 0x7F) + this->brightnessInitialBoost;
				if (n2 >= this->maxLocalBrightness) {
					this->brightenPPMaxReachedTime = app->time;
				}
			}
			else if (app->time < this->brightenPPMaxReachedTime + 500) {
				n2 = this->maxLocalBrightness;
			}
			else {
				n2 = std::min(this->brightenPPMaxReachedTime + 500 + 500 - app->time >> 4 & 0x7F, this->maxLocalBrightness);
			}
			max = std::max((n2 * n2 << 10) / 127 >> 5, 32);
		}
	}

	if (app->player->isFamiliar)
	{
		if (this->brightenPostProcess) {
			graphics->FMGL_fillRect(0, 0, 480, 320, 1.0, 1.0, 1.0, (float)max / 2226.0);
		}
		update += 0.05f;
		y = (y + 1) % 30;
		float alpha = sin(update) * 0.0500000007 + 0.349999994;
		if ((uint16_t)(app->player->familiarType - 1) > 1u) {
			graphics->FMGL_fillRect(0, 0, 480, 256/*320*/, 1.0, 0.0, 0.0, alpha);
		}
		else {
			graphics->FMGL_fillRect(0, 0, 480, 256/*320*/, 0.0, 1.0, 0.0, alpha);
		}
		for (int i = 0; i < 256 /*300*/; i += 10) {
			graphics->FMGL_fillRect( 0, i + (y / 3), 480, 5, 0.0, 0.0, 0.0, 0.15);
		}
	}

#if 0
	for (int i = 1; i < Render.screenHeight - 1; ++i) {
		final int n3 = ((i & 0x3) == 0x3) ? 1 : 0;
		for (int j = 1; j < Render.screenWidth - 1; ++j) {
			if (Render.brightenPostProcess && n3 == 0) {
				max2 = Math.max(max, 32);
			}
			final int n4 = pixels[i * Render.screenWidth + j];
			int min = 2 * (n4 >> 8 & 0xFF) + (n4 >> 16 & 0xFF) + (n4 & 0xFF) >> 2;
			if (Render.brightenPostProcess && n3 == 0) {
				int max3 = Math.max(1, min);
				if (max2 != 0) {
					max3 = max3 * max2 >> 5;
				}
				min = Math.min(255, max3);
			}
			switch (Render.postProcessMode) {
			case 1: {
				pixels[i * Render.screenWidth + j] = (0xFF000000 | min >> 1 << 16 | min >> n3 << 8 | min >> 1);
				break;
			}
			case 2: {
				pixels[i * Render.screenWidth + j] = (0xFF000000 | min >> n3 << 16 | min >> 1 << 8 | min >> 1);
				break;
			}
			}
		}
	}
#endif
}

int Render::convertToGrayscale(int color) {
	int n2 = (color & 0xFF) + ((color >> 16 & 0xFF) - (color & 0xFF) >> 1);
	int n3 = n2 + ((color >> 8 & 0xFF) - n2 >> 1);
	return 0xFF000000 | n3 >> 1 << 16 | n3 >> 1 << 8 | n3 >> 1;
}

bool Render::checkPortalVisibility(int x, int y, int z) {
	Applet* app = CAppContainer::getInstance()->app;
	int n4 = 64 * this->sinTable[this->viewAngle & 0x3FF] >> 12;
	int n5 = 64 * this->sinTable[this->viewAngle + 256 & 0x3FF] >> 12;
	if (app->player->isFamiliar) {
		this->portalState = Render::PORTAL_DNE;
		return false;
	}
	int n6 = ((app->canvas->viewX >> 6) - (x >> 6)) * ((app->canvas->viewX >> 6) - (x >> 6));
	int n7 = ((app->canvas->viewY >> 6) - (y >> 6)) * ((app->canvas->viewY >> 6) - (y >> 6));
	if (this->closestPortalDist <= n6 + n7) {
		return false;
	}
	this->closestPortalDist = n6 + n7;
	if (n6 + n7 > 15) {
		if (!this->portalScripted) {
			if (this->portalState != Render::PORTAL_DNE) {
				this->portalState = Render::PORTAL_SPINNING_DOWN;
			}
			return false;
		}
		return true;
	}
	else {
		app->game->trace(app->canvas->viewX, app->canvas->viewY, app->canvas->viewZ, x, y, z >> 4, app->player->getPlayerEnt(), 5293, 25, true);
		if (app->game->traceFracs[0] != 0x3FFF) {
			this->portalScripted = false;
			this->portalState = Render::PORTAL_DNE;
			return false;
		}
		x <<= 4;
		y <<= 4;
		TGLVert* tglVert = &app->tinyGL->mv[0];
		tglVert->x = x + n4;
		tglVert->y = y + n5;
		tglVert->z = z + 1056;
		TGLVert* tglVert2 = &app->tinyGL->mv[1];
		tglVert2->x = tglVert->x;
		tglVert2->y = tglVert->y;
		tglVert2->z = tglVert->z - (1056 * 2);
		TGLVert* tglVert3 = &app->tinyGL->mv[2];
		tglVert3->x = tglVert->x - (n4 << 1);
		tglVert3->y = tglVert->y - (n5 << 1);
		tglVert3->z = tglVert->z;
		TGLVert* transform3DVerts = app->tinyGL->transform3DVerts(app->tinyGL->mv, 3);
		int tz = transform3DVerts[0].z;
		if (transform3DVerts[0].w + transform3DVerts[0].z < 0 || transform3DVerts[1].w + transform3DVerts[1].z < 0 || transform3DVerts[2].w + transform3DVerts[2].z < 0) {
			this->portalScripted = false;
			this->portalState = Render::PORTAL_DNE;
			return false;
		}
		app->tinyGL->projectVerts(transform3DVerts, 3);
		this->portalCX = transform3DVerts[0].x + transform3DVerts[2].x >> 4;
		this->portalCY = transform3DVerts[0].y + transform3DVerts[1].y >> 4;
		this->portalScale = std::min(((transform3DVerts[1].x - transform3DVerts[2].x << 7) / 132) >> 1, 2048);

		if (app->render->_gles->isInit) { // [GEC] GL Version
			if (app->tinyGL->viewPitch != 0 && (tz < 3000)) {
				this->portalCY += 10;
			}
			this->portalCY += tz / 300;
		}

		if (this->portalScripted) {
			return this->portalState != Render::PORTAL_DNE;
		}
		if (this->portalState != Render::PORTAL_SPINNING) {
			this->portalState = Render::PORTAL_SPINNING_UP;
		}
		return true;
	}
}

void Render::renderPortal() {
	static float angle = 0.f;
	Applet* app = CAppContainer::getInstance()->app;

	
	if (this->portalState == Render::PORTAL_DNE) {
		this->previousPortalState = Render::PORTAL_DNE;
		return;
	}
	if (this->portalState == Render::PORTAL_SPINNING_UP) {
		if (this->previousPortalState != Render::PORTAL_SPINNING_UP) {
			this->portalSpinTime = app->time + 500;
			this->previousPortalState = Render::PORTAL_SPINNING_UP;
		}
	}
	else if (this->portalState == Render::PORTAL_SPINNING_DOWN && this->previousPortalState != Render::PORTAL_SPINNING_DOWN) {
		this->portalSpinTime = app->time + 500;
		this->previousPortalState = Render::PORTAL_SPINNING_DOWN;
	}
	if (this->portalState != Render::PORTAL_SPINNING) {
		int n2 = this->portalSpinTime - app->time;
		if (n2 < 0) {
			if (this->portalState == Render::PORTAL_SPINNING_DOWN) {
				this->portalState = Render::PORTAL_DNE;
				return;
			}
			if (this->portalState == Render::PORTAL_SPINNING_UP) {
				this->portalState = Render::PORTAL_SPINNING;
			}
		}
		else if (this->portalState == Render::PORTAL_SPINNING_UP) {
			this->portalScale = (this->portalScale * (500 - n2)) / 500;
		}
		else {
			this->portalScale = (this->portalScale * n2) / 500;
		}
	}

	if (app->render->_gles->isInit) { // [GEC] GL Version
		if (!this->imgPortal) {
			this->imgPortal = app->loadImage("portal_image.bmp", true);
		}

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
		glDisable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		angle += 1.0f;

		float pScale = (float)this->portalScale / 600.0f;
		float sinAng = sinf((float)((float)(angle * 5.0f) * 3.14f) / 360.0f);
		float scale = pScale * (float)((float)(sinAng * 0.1f) + 1.0f);

		this->_gles->DrawPortalTexture(this->imgPortal, 0, 0, imgPortal->width, imgPortal->height,
			(float)((float)this->portalCX + (float)((float)(scale * (float)imgPortal->width) * -0.5f)),
			(float)((float)this->portalCY + (float)((float)(scale * (float)imgPortal->height) * -0.5f)),
			scale,
			angle,
			0);

		this->_gles->DrawPortalTexture(this->imgPortal, 0, 0, imgPortal->width, imgPortal->height,
			(float)((float)this->portalCX + (float)((float)((float)((float)(pScale + pScale) - scale) * (float)imgPortal->width) * -0.5f)),
			(float)((float)this->portalCY + (float)((float)((float)((float)(pScale + pScale) - scale) * (float)imgPortal->height) * -0.5f)),
			(float)(pScale + pScale) - scale,
			-angle,
			1);
	}
	else { // TnyGL Version restored from J2ME/BREW
		int n = 10;
		int max = std::max(Render::portalMaxRadius * this->portalScale >> n, 1);
		int portalCX = this->portalCX;
		int portalCY = this->portalCY;
		int n3 = (Render::portalMaxRadius - 14) * this->portalScale >> n;
		int n4 = 11 * this->portalScale >> n;
		int n5 = 9 * this->portalScale >> n;
		int n6 = 6 * this->portalScale >> n;
		int max2 = std::max(((8 * this->portalScale) >> n), 1);
		int n7 = 100 * (16 / max2); // Old (8 / max2); // [GEC] Prevents it from spinning too fast / Evita que gire demasiado rapido
		int n8 = this->currentPortalMod >> 1;
		for (int i = portalCY - n3 - n8; i < portalCY + n3 + n8; ++i) {
			for (int j = portalCX - n3 - n8; j < portalCX + n3 + n8; ++j) {
				if ((portalCX - j) * (portalCX - j) + (portalCY - i) * (portalCY - i) <= (n3 + n8) * (n3 + n8)) {
					if (j >= 1 && j < screenWidth - 1 && i >= 1 && i < screenHeight - 1) {
						app->canvas->graphics.drawPixelPortal(app->canvas->viewRect, j, i, 0x6f000000);
					}
				}
			}
		}
		for (int k = -max - this->currentPortalMod; k < max + this->currentPortalMod; ++k) {
			if (k <= -n4 || k >= n4) {
				int n11 = (max >> 1) * this->sinTable[(k - max << 16) / max * 512 >> 16 & 0x3FF] >> 16;
				int n12 = (max - std::abs(k) << 16) / max;
				int n13;
				if (k < -max || k > max) {
					n13 = 0xAFB06E00;
				}
				else {
					n13 = ((160 * (65536 - n12) >> 16) + 15 << 24 | 0xB00000 | 55 * n12 >> 16 << 8);
				}
				for (int l = 0; l < 8; ++l) {
					int n14 = l * 512 / 8 - this->currentPortalTheta;
					int n15 = this->sinTable[n14 & 0x3FF];
					int n16 = this->sinTable[n14 + 256 & 0x3FF];
					int n17 = (k * n16 - n11 * n15 >> 16) + portalCX;
					int n18 = (k * n15 + n11 * n16 >> 16) + portalCY;
					int n19 = (n13 >> 24 & 0xFF) << 16;
					if (n17 >= 1 && n17 < screenWidth - 1 && n18 >= 1 && n18 < screenHeight - 1) {
						app->canvas->graphics.drawPixelPortal(app->canvas->viewRect, n17, n18, n13);
					}
					for (int n22 = 0; n22 < n6; ++n22) {
						int n23 = n17 + app->nextByte() % n5 - (n5 >> 1);
						int n24 = n18 + app->nextByte() % n5 - (n5 >> 1);
						if (n23 >= 1 && n23 < screenWidth - 1 && n24 >= 1 && n24 < screenHeight - 1) {
							app->canvas->graphics.drawPixelPortal(app->canvas->viewRect, n23, n24, n13);
						}
					}
				}
			}
		}
		if (app->time > this->nextPortalFrame) {
			this->currentPortalTheta = (this->currentPortalTheta + 20 & 0x3FF);
			if (this->portalModIncreasing) {
				if (this->currentPortalMod >= max2) {
					this->portalModIncreasing = false;
					this->currentPortalMod = max2 - 1;
				}
				else {
					++this->currentPortalMod;
				}
			}
			else if (this->currentPortalMod <= 0) {
				this->portalModIncreasing = true;
				++this->currentPortalMod;
			}
			else {
				--this->currentPortalMod;
			}
			this->nextPortalFrame = app->time + n7;
		}
	}
}

void Render::fillWeaponRect(int n, int n2, int n3, int n4, int n5) {
	Applet* app = CAppContainer::getInstance()->app;
	int n6 = n2 * this->screenWidth + n;
	int n7 = n6 + n3;
	int i = n6;
	int j = 0;
	uint16_t* pixels = app->tinyGL->pixels;
	while (j < n4) {
		while (i < n7) {
			pixels[i] = n5;
			++i;
		}
		n6 += this->screenWidth;
		n7 += this->screenWidth;
		i = n6;
		++j;
	}
}

void Render::drawRGB(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->hud->cockpitOverlayRaw) { // [GEC]
		return;
	}

	this->bltTime = app->upTimeMs;
	int viewportX = app->canvas->viewRect[0] + app->tinyGL->viewportX;
	int viewportY = app->canvas->viewRect[1] + app->tinyGL->viewportY;

	int viewportWidth = app->tinyGL->viewportWidth;
	int viewportHeight = app->tinyGL->viewportHeight;

	if (!app->render->_gles->isInit) { // [GEC] Adjusted like this to match the Y position on the GL version
		viewportY = 0;
		viewportHeight = 320;
	}

	graphics->setColor(0x00000000);
	graphics->drawRect(viewportX - 1, viewportY - 1, viewportWidth + 2 - 1, viewportHeight + 2 - 1);
	if (this->fadeFlags != 0) {
		this->fadeScene(graphics);
	}
	this->_gles->SwapBuffers();
	this->bltTime = app->upTimeMs - this->bltTime;
}

void Render::rockView(int rockViewDur, int x, int y, int z) {
	Applet* app = CAppContainer::getInstance()->app;
	this->rockViewDur = rockViewDur;
	this->rockViewTime = app->upTimeMs;
	this->rockViewX = x << 4;
	this->rockViewY = y << 4;
	this->rockViewZ = z << 4;
}

bool Render::isNPC(int n) {
	return n >= Enums::TILENUM_FIRST_NPC && n <= Enums::TILENUM_LAST_NPC;
}

bool Render::isImp(int n) {
	return n >= Enums::TILENUM_MONSTER_IMP && n <= Enums::TILENUM_MONSTER_IMP3;
}

bool Render::isSentinel(int n) {
	return n >= Enums::TILENUM_MONSTER_SENTINEL && n <= Enums::TILENUM_MONSTER_SENTINEL3;
}

bool Render::isPinky(int n) {
	return n >= Enums::TILENUM_MONSTER_PINKY && n <= Enums::TILENUM_MONSTER_PINKY3;
}

bool Render::isRevenant(int n) {
	return n >= Enums::TILENUM_MONSTER_REVENANT && n <= Enums::TILENUM_MONSTER_REVENANT3;
}

bool Render::isMancubus(int n) {
	return n >= Enums::TILENUM_MONSTER_MANCUBUS && n <= Enums::TILENUM_MONSTER_MANCUBUS3;
}

bool Render::isArchVile(int n) {
	return n >= Enums::TILENUM_MONSTER_ARCH_VILE && n <= Enums::TILENUM_MONSTER_ARCH_VILE3;
}

bool Render::isChainsawGoblin(int n) {
	return n >= Enums::TILENUM_MONSTER_SAW_GOBLIN && n <= Enums::TILENUM_MONSTER_SAW_GOBLIN3;
}

bool Render::isCacodemon(int n) {
	return n >= Enums::TILENUM_MONSTER_CACODEMON && n <= Enums::TILENUM_MONSTER_CACODEMON3;
}

bool Render::isLostSoul(int n) {
	return n >= Enums::TILENUM_MONSTER_LOST_SOUL && n <= Enums::TILENUM_MONSTER_LOST_SOUL3;
}

bool Render::isSentryBot(int n) {
	return n == Enums::TILENUM_MONSTER_SENTRY_BOT || n == Enums::TILENUM_MONSTER_RED_SENTRY_BOT;
}

bool Render::isZombie(int n) {
	return n >= Enums::TILENUM_MONSTER_ZOMBIE && n <= Enums::TILENUM_MONSTER_ZOMBIE3;
}

bool Render::hasGunFlare(int n) {
	return isMancubus(n) || isRevenant(n) || isSentryBot(n) || n == Enums::TILENUM_BOSS_CYBERDEMON || n == Enums::TILENUM_BOSS_MASTERMIND;
}

bool Render::isFloater(int n) {
	return isSentinel(n) || isLostSoul(n) || isCacodemon(n);
}

bool Render::isSpecialBoss(int n) {
	return n == Enums::TILENUM_BOSS_MASTERMIND || n == Enums::TILENUM_MONSTER_ARACHNOTRON || n == Enums::TILENUM_BOSS_PINKY || (n >= Enums::TILENUM_BOSS_VIOS && n <= Enums::TILENUM_BOSS_VIOS5);
}

void Render::renderFearEyes(Entity* entity, int frame, int x, int y, int z, int scaleFactor, bool flipH) {
	Applet* app = CAppContainer::getInstance()->app;

	int anim = frame & Enums::MANIM_MASK;
	uint8_t eSubType = entity->def->eSubType;
	frame &= Enums::MFRAME_MASK;

	if ((anim != 0 && anim != 32) || entity->monster == nullptr || (1 << eSubType & Enums::FEAR_IMMUNE_MONSTERS) != 0x0 || entity->monster->goalType != 4) {
		return;
	}

	int eyeZ = 16;
	int eyeL = -1;
	int eyeR = -1;
	if (anim == 0) {
		int n10 = app->time + entity->getSprite() * 1337;
		int n11;
		if (eSubType == Enums::MONSTER_CACODEMON || eSubType == Enums::MONSTER_SENTINEL || eSubType == Enums::MONSTER_LOST_SOUL) {
			n11 = (n10 / 512 & 0x1) << 4;
		}
		else {
			n11 = (n10 / 1024 & 0x1) * 26;
		}
		if ((entity->info & 0x20000000) != 0x0) {
			n11 = 0;
		}
		eyeZ += n11;
	}
	if (eSubType == Enums::MONSTER_LOST_SOUL) {
		eyeZ += 192;
	}
	else if (eSubType == Enums::MONSTER_REVENANT) {
		eyeZ += ((anim == 0) ? 12 : 10) << 4;
	}
	else if (anim == 0 && eSubType == Enums::MONSTER_ARCH_VILE) {
		eyeZ += 48;
	}
	switch (eSubType) {
	case Enums::MONSTER_ARCH_VILE: {
		eyeL = -3;
		eyeR = 3;
		eyeZ += 403;
		break;
	}
	case Enums::MONSTER_SAW_GOBLIN: {
		eyeL = -3;
		eyeR = 3;
		eyeZ += 388;
		break;
	}
	case Enums::MONSTER_IMP: {
		eyeL = -3; // old eyeL = -3;
		eyeR = 3;
		eyeZ += 294;
		break;
	}
	case Enums::MONSTER_LOST_SOUL: {
		eyeL = -4; // old eyeL = -6;
		eyeR = 4;
		eyeZ -= 192;
		break;
	}
	case Enums::MONSTER_MANCUBUS: {
		eyeL = -3;
		eyeR = 3;
		eyeZ += 280;
		break;
	}
	case Enums::MONSTER_PINKY: {
		eyeL = -5;
		eyeR = 5; // eyeR = 4;
		eyeZ += 35; // eyeZ += 45;
		break;
	}
	case Enums::MONSTER_CACODEMON: {
		eyeZ += 32;
		eyeL = 0;
		eyeR = -1;
		break;
	}
	case Enums::MONSTER_REVENANT: {
		if (flipH) { // Old line code, wrong -> if (app->canvas->loadMapID == 8) {
			eyeL = -5;
			eyeR = 2;
		}
		else {
			eyeL = -2;
			eyeR = 5;
		}
		eyeZ += 324;
		break;
	}
	case Enums::MONSTER_SENTINEL: {
		eyeL = -1;
		eyeR = 4;
		eyeZ += 274;
		break;
	}
	}
	if (flipH) {
		int tmp = eyeL;
		eyeL = eyeR;
		eyeR = tmp;
	}
	int scaleEyeL = eyeL * scaleFactor / 65536;
	int scaleEyeR = eyeR * scaleFactor / 65536;
	int scaleEyeZ = eyeZ * scaleFactor / 65536;
	this->renderSprite(x + (scaleEyeL * this->viewRightStepX >> 6), y + (scaleEyeL * this->viewRightStepY >> 6), z + scaleEyeZ, 251, 0, 0, 0, scaleFactor, 0);
	if (eSubType != Enums::MONSTER_CACODEMON) {
		this->renderSprite(x + (scaleEyeR * this->viewRightStepX >> 6), y + (scaleEyeR * this->viewRightStepY >> 6), z + scaleEyeZ, 251, 0, 0, 0, scaleFactor, 0);
	}
}

void Render::renderSpriteAnim(int n, int frame, int x, int y, int z, int tileNum, int flags, int renderMode, int scaleFactor, int renderFlags) {
	Applet* app = CAppContainer::getInstance()->app;

	Entity* entity = &app->game->entities[this->mapSprites[this->S_ENT + n]];

	if (tileNum == Enums::TILENUM_BOSS_CYBERDEMON && scaleFactor > 0x10000) {
		//printf("scaleFactor %x\viewPitch", scaleFactor);
		__int64 v16 = 7 * scaleFactor;
		__int64 v13 = (unsigned __int64)(v16 * (__int64)0x38E38E39) >> 32;
		scaleFactor = (int)((v13 >> 1) - (v16 >> 31));
		//printf("->scaleFactor %x\viewPitch", scaleFactor);
	}

	if (this->isFloater(tileNum)) {
		this->renderFloaterAnim(n, frame, x, y, z, tileNum, flags, renderMode, scaleFactor, renderFlags);
		this->renderFearEyes(entity, frame, x, y, z, scaleFactor, (flags & Enums::SPRITE_FLAG_FLIP_HORIZONTAL));
		return;
	}
	if (this->isSpecialBoss(tileNum)) {
		this->renderSpecialBossAnim(n, frame, x, y, z, tileNum, flags, renderMode, scaleFactor, renderFlags);
		this->renderFearEyes(entity, frame, x, y, z, scaleFactor, (flags & Enums::SPRITE_FLAG_FLIP_HORIZONTAL));
		return;
	}
	int anim = frame & Enums::MANIM_MASK;
	frame &= Enums::MFRAME_MASK;
	int n12 = 0;
	int n13;
	int min;
	if ((entity->monster != nullptr && (entity->monster->flags & 0x4000) != 0x0) || this->isNPC(tileNum)) {
		n13 = z;
		min = 0;
	}
	else {
		n13 = (this->getHeight(x, y) + 32) << 4;
		min = std::min(std::max(z - n13, 0), 256);
	}
	int n14 = scaleFactor * (256 - min) / 256;
	int n15 = 0;
	int n16 = 0;
	int n17 = 0;
	int n18 = 0;
	int n19 = 1;

	//anim = Enums::MANIM_SLAP;
	//frame = (app->time) / 1024 & 0x1;

	switch (anim) {
	case Enums::MANIM_IDLE_BACK: {
		n12 = 4;
	}
	case Enums::MANIM_IDLE: {
		int n20 = (app->time + n * 1337) / 1024 & 0x1;
		if ((entity->info & 0x20000000) != 0x0) {
			n20 = 0;
		}
		int n21 = n20 * 26;

		// Shadow
		this->renderSprite(x, y, n13, Enums::TILENUM_SHADOW, 0, flags, renderMode, n14, renderFlags);
		// Legs
		this->renderSprite(x, y, z, tileNum, n12, flags, renderMode, scaleFactor, renderFlags);

		if (this->isRevenant(tileNum) && anim == Enums::MANIM_IDLE) {
			n15 = -30;
		}

		if ((tileNum == Enums::TILENUM_NPC_RILEY_OCONNOR) || 
			(tileNum == Enums::TILENUM_NPC_CIVILIAN2) ||
			(tileNum == Enums::TILENUM_NPC_SCIENTIST)) {
			if (anim == Enums::MANIM_IDLE) {
				n15 = -18;
			}
		}

		if (this->isArchVile(tileNum) && anim == Enums::MANIM_IDLE) {
			n15 = -36;
		}

		if (tileNum == Enums::TILENUM_BOSS_CYBERDEMON && anim == Enums::MANIM_IDLE) { // [GEC]
			n16 = -15;
		}

		if (tileNum == Enums::TILENUM_BOSS_CYBERDEMON) {
			n15 = -30;
		}

		// Torso
		this->renderSprite(x + (n17 * this->viewRightStepX >> 6), y + (n17 * this->viewRightStepY >> 6), (z + n21) + n15, tileNum, n12 + 2, flags, renderMode, scaleFactor, renderFlags);
		
		// Head
		if ((tileNum == Enums::TILENUM_MONSTER_SENTRY_BOT) || (tileNum == Enums::TILENUM_MONSTER_RED_SENTRY_BOT)) {
			flags ^= (((app->time + n * 1337) / 2048) & 0x1) << 17; // Flip Head
			this->renderSprite(x + (n17 * this->viewRightStepX >> 6), y + (n17 * this->viewRightStepY >> 6), (z + n21) + n16, tileNum, n12 + 3, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		if (n19 != 0 && (!this->isPinky(tileNum) || anim != Enums::MANIM_IDLE_BACK)) {
			if (this->isRevenant(tileNum) && anim == Enums::MANIM_IDLE) {
				n16 = 140;
			}
			else if (this->isArchVile(tileNum) && anim == Enums::MANIM_IDLE) {
				n16 = 109;
			}
			else if ((tileNum == Enums::TILENUM_NPC_RILEY_OCONNOR) && anim == Enums::MANIM_IDLE) {
				n16 = -32;
			}
			else if (this->isRevenant(tileNum) && anim == Enums::MANIM_IDLE) {
				n16 = 8;
			}
			this->renderSprite(x + (n17 * this->viewRightStepX >> 6), y + (n17 * this->viewRightStepY >> 6), (z + n21) + n16, tileNum, n12 + 3, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		break;
	}
	case Enums::MANIM_WALK_BACK: {
		n12 = 4;
	}
	case Enums::MANIM_WALK_FRONT: {
		int n22 = ((frame & 0x2) >> 1 ^ 0x1) << 17;
		// Shadow
		this->renderSprite(x, y, n13, Enums::TILENUM_SHADOW, 0, flags, renderMode, n14, renderFlags);
		// Legs
		this->renderSprite(x, y, z, tileNum, n12 + (frame & 0x1), flags ^ n22, renderMode, scaleFactor, renderFlags);
		int n23 = frame & 0x1;
		if ((frame & 0x2) == 0x0) {
			n23 = -n23;
		}
		x += n23 * this->viewRightStepX >> 6;
		y += n23 * this->viewRightStepY >> 6;

		if (this->isArchVile(tileNum)) {
			n15 = -36;
		}
		// Torso
		this->renderSprite(x + (n17 * this->viewRightStepX >> 6), y + (n17 * this->viewRightStepY >> 6), z + n15 + ((frame & 0x1) << 4), tileNum, n12 + 2, ((this->isNPC(tileNum) && tileNum != Enums::TILENUM_NPC_SARGE) || this->isArchVile(tileNum)) ? n22 : flags, renderMode, scaleFactor, renderFlags);
		
		// Head
		if ((tileNum == Enums::TILENUM_MONSTER_SENTRY_BOT) || (tileNum == Enums::TILENUM_MONSTER_RED_SENTRY_BOT)) {
			flags ^= ((app->time + n * 1337) / 1024 & 0x1) << 17; // Flip Head
		}
		if (n19 != 0 && (!this->isPinky(tileNum) || anim != Enums::MANIM_WALK_BACK)) {
			if (this->isRevenant(tileNum) && anim == Enums::MANIM_WALK_FRONT) {
				n16 = 160;
			}
			else if (this->isArchVile(tileNum)) {
				n16 = 109;
			}
			this->renderSprite(x + (n18 * this->viewRightStepX >> 6), y + (n18 * this->viewRightStepY >> 6), z + n16 + ((frame & 0x1) << 4), tileNum, n12 + 3, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		break;
	}
	case Enums::MANIM_ATTACK1:
	case Enums::MANIM_ATTACK2: {
		int n24 = 0;
		int n25 = flags;
		if (this->isZombie(tileNum) && frame == 1) {
			n25 ^= 0x20000;
		}
		int n26;
		if (anim == Enums::MANIM_ATTACK1) {
			n26 = 8;
		}
		else {
			n26 = 10;
		}
		// Shadow
		this->renderSprite(x, y, n13, Enums::TILENUM_SHADOW, 0, flags, renderMode, n14, renderFlags);
		if (tileNum > Enums::TILENUM_LAST_MONSTER) {
			this->renderSprite(x, y, z, tileNum, n26, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		if (this->isPinky(tileNum)) {
			int n27 = n26;
			int n28 = 2;
			this->renderSprite(x, y, z, tileNum, n24, flags ^ 0x20000, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x + (n17 * this->viewRightStepX >> 6), y + ((n17 * this->viewRightStepY) >> 6), z, tileNum, n28, flags, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x + (n18 * this->viewRightStepX >> 6), y + ((n18 * this->viewRightStepY) >> 6), z, tileNum, n27 + (frame & 0x1), flags, renderMode, scaleFactor, renderFlags);
			break;
		}

		int n27;
		if (tileNum != Enums::TILENUM_BOSS_CYBERDEMON || n25 == 0x20000) {
			n27 = 0;
		}
		else {
			n27 = -3;
		}

		// Legs
		this->renderSprite(x + ((n27 * this->viewRightStepX) >> 6), y + ((n27 * this->viewRightStepY) >> 6), z, tileNum, n24, n25 ^ 0x20000, renderMode, scaleFactor, renderFlags);

		if (this->isArchVile(tileNum)) {
			if (frame == 0) {
				n15 = 288;
			}
			n19 = 0;
		}
		else if (this->isChainsawGoblin(tileNum)) {
			if (frame == 0) {
				n15 = 96;
			}
			else if (frame == 1) {
				n15 = -100;
			}
		}
		else if (this->isRevenant(tileNum) && (anim == Enums::MANIM_ATTACK2)) { //[GEC]
			if (frame == 0) {
				n15 = 130;
			}
		}

		if (frame == 1 && (!this->hasGunFlare(tileNum) || (this->isRevenant(tileNum) && anim == Enums::MANIM_ATTACK2))) {
			++n26;
		}
		// Torso
		this->renderSprite(x + (n17 * this->viewRightStepX >> 6), y + (n17 * this->viewRightStepY >> 6), z + n15, tileNum, n26, flags, renderMode, scaleFactor, renderFlags);
		
		bool b = (flags & 0x20000) != 0x0;

		// [GEC] No muestra las cabezas para los siguientes sprites, ya que el torzo contine la cabeza incluida
		// Mancubus
		// Revenant
		bool renderHead = true;
		{
			if (this->isMancubus(tileNum)) {
				renderHead = false;
			}
			if (this->isRevenant(tileNum) && (anim == Enums::MANIM_ATTACK2)) {
				renderHead = false;
			}
		}
		
		if (n19 != 0 && !this->isZombie(tileNum) && !this->isImp(tileNum) && renderHead) {
			int n29 = 3;
			if (this->isRevenant(tileNum)) {
				n18 = 2;
				n16 = 20; // [GEC]
			}
			else if (this->isMancubus(tileNum)) {
				n18 = 7;
				n16 = -112;
			}
			else if (this->isChainsawGoblin(tileNum)) {
#if 0 //J2ME
				if (frame == 0) {
					n16 = 96;
				}
				else {
					n16 = 16;
				}
#else
				n16 = 16;

				if (frame == 0) { //[GEC]
					n18 = 2;
					n16 = 17;
				}
#endif
			}
			if (b) {
				n18 = -n18;
			}


			// head
			this->renderSprite(x + (n18 * this->viewRightStepX >> 6), y + (n18 * this->viewRightStepY >> 6), z + n16, tileNum, n29, flags, renderMode, scaleFactor, renderFlags);
		}
		if (frame == 1 && this->hasGunFlare(tileNum) && (!this->isRevenant(tileNum) || anim != Enums::MANIM_ATTACK2)) {
			int n30 = 0;
			int n31 = 0;
			int n32 = 0;
			if (this->isMancubus(tileNum)) {
				int n33 = -24 * this->viewRightStepX >> 6;
				int n34 = -24 * this->viewRightStepY >> 6;
				int n35 = 160;
				if ((flags & 0x20000) != 0x0) {
					n33 = -n33;
					n34 = -n34;
				}
				// Flash
				this->renderSprite(x + n33, y + n34, z + n35, tileNum, n26 + 1, (app->player->totalMoves + app->combat->animLoopCount & 0x3) << 17, 4, scaleFactor / 3, renderFlags);
				n30 = (22 * this->viewRightStepX) >> 6;
				n31 = (22 * this->viewRightStepY) >> 6;
				n32 = 128;
			}
			else if (this->isRevenant(tileNum)) {
				int n36 = -9 * this->viewRightStepX >> 6;
				int n37 = -9 * this->viewRightStepY >> 6;
				int n38 = 736;
				if ((flags & 0x20000) != 0x0) {
					n36 = -n36;
					n37 = -n37;
				}
				// Flash
				this->renderSprite(x + n36, y + n37, z + n38, tileNum, n26 + 1, (app->player->totalMoves + app->combat->animLoopCount & 0x3) << 17, 4, scaleFactor / 3, renderFlags);
				n30 = 15 * this->viewRightStepX >> 6;
				n31 = 15 * this->viewRightStepY >> 6;
				n32 = 736;
			}
			else if (this->isSentryBot(tileNum)) {
				n32 = -64;
			}
			else if (tileNum == Enums::TILENUM_BOSS_CYBERDEMON) {
				n30 = 14 * this->viewRightStepX >> 6;
				n31 = 14 * this->viewRightStepY >> 6;
				n32 = 352;
			}
			if (b) {
				n30 = -n30;
				n31 = -n31;
			}
			x += n30;
			y += n31;
			z += n32;
			// Flash
			this->renderSprite(x, y, z, tileNum, n26 + 1, (app->player->totalMoves + app->combat->animLoopCount & 0x3) << 17, 4, scaleFactor / 3, renderFlags);
			break;
		}
		break;
	}
	case Enums::MANIM_PAIN:
	case Enums::MANIM_SLAP: {
		this->renderSprite(x, y, n13, Enums::TILENUM_SHADOW, 0, flags, renderMode, n14, renderFlags);
		this->renderSprite(x, y, z + n15, tileNum, 12, flags, renderMode, scaleFactor, renderFlags);
		break;
	}
	case Enums::MANIM_DEAD: {
		if (entity->monster != nullptr && (entity->monster->flags & 0x800) == 0x0 && app->canvas->state != 18 && !entity->hasEmptyLootSet()) {
			this->renderSprite(x, y, z, tileNum, 13, flags, 0, (18 * scaleFactor) >> 4, 512);
		}
		this->renderSprite(x, y, z, tileNum, 13, flags, renderMode, scaleFactor, renderFlags);
		break;
	}
	case Enums::MANIM_NPC_BACK_ACTION: {
		this->renderSprite(x, y, n13, Enums::TILENUM_SHADOW, 0, flags, renderMode, n14, renderFlags);
		this->renderSprite(x, y, z, tileNum, 8 + frame, flags, renderMode, scaleFactor, renderFlags);
		break;
	}
	case Enums::MANIM_NPC_TALK: {
		this->renderSprite(x, y, z, tileNum, frame, flags, renderMode, scaleFactor, renderFlags);
		break;
	}
	}
	this->renderFearEyes(entity, anim, x, y, z, scaleFactor, (flags & Enums::SPRITE_FLAG_FLIP_HORIZONTAL));
}

void Render::renderFloaterAnim(int n, int frame, int x, int y, int z, int tileNum, int flags, int renderMode, int scaleFactor, int renderFlags) {
	Applet* app = CAppContainer::getInstance()->app;

	int anim = frame & Enums::MANIM_MASK;
	int n12 = frame;
	frame = 0;
	bool lostSoul = this->isLostSoul(tileNum);
	if (lostSoul) {
		z += 192;
	}
	int n13 = 0;
	if (this->isSentinel(tileNum)) {
		frame = 2;
		switch (anim) {
		case Enums::MANIM_IDLE_BACK:
		case Enums::MANIM_WALK_BACK: {
			frame = 6;
		}
		case Enums::MANIM_IDLE:
		case Enums::MANIM_WALK_FRONT:
		case Enums::MANIM_DODGE: {
			int n14 = (app->time + n * 1337) / 512 & 0x1;
			int zPos = -11; // [GEC]
			this->renderSprite(x, y, z + (n14 << 4), tileNum, frame, flags, renderMode, scaleFactor, renderFlags); // Torzo
			this->renderSprite(x, y, z + (n14 << 4) + zPos, tileNum, ++frame, flags, renderMode, scaleFactor, renderFlags); // Head
			break;
		}
		case Enums::MANIM_ATTACK2: {
			n13 = 2;
		}
		case Enums::MANIM_ATTACK1: {
			this->renderSprite(x, y, z, tileNum, 8 + n13 + (n12 & 0x1), flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		case Enums::MANIM_PAIN: {
			this->renderSprite(x, y, z, tileNum, 12, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		case Enums::MANIM_DEAD: {
			Entity *entity = &app->game->entities[this->mapSprites[this->S_ENT + n]];
			if (entity->monster != nullptr && (entity->monster->flags & 0x800) == 0x0 && app->canvas->state != 18 && !entity->hasEmptyLootSet()) {
				this->renderSprite(x, y, z, tileNum, 13, flags, 0, 17 * scaleFactor >> 4, 512);
			}
			this->renderSprite(x, y, z, tileNum, 13, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		}
		return;
	}
	switch (anim) {
	case Enums::MANIM_IDLE_BACK:
	case Enums::MANIM_WALK_BACK: {
		frame = 4;
	}
	case Enums::MANIM_IDLE:
	case Enums::MANIM_WALK_FRONT: {
		int n15 = (app->time + n * 1337) / 512 & 0x1;
		if (n15 != 0 && lostSoul) {
			++frame;
		}
		this->renderSprite(x, y, z + (n15 << 4), tileNum, frame, flags, renderMode, scaleFactor, renderFlags);
		if (anim == 16 && !lostSoul) {
			this->renderSprite(x, y, z, tileNum, 7, flags & 0xFFFDFFFF, renderMode, scaleFactor, renderFlags);
			break;
		}
		break;
	}
	case Enums::MANIM_ATTACK2: {
		n13 = 2;
	}
	case Enums::MANIM_ATTACK1: {
		this->renderSprite(x, y, z, tileNum, 8 + n13 + (n12 & 0x1), flags, renderMode, scaleFactor, renderFlags);
		break;
	}
	case Enums::MANIM_PAIN: {
		this->renderSprite(x, y, z, tileNum, 12, flags, renderMode, scaleFactor, renderFlags);
		break;
	}
	case Enums::MANIM_DEAD: {
		this->renderSprite(x, y, z, tileNum, 13, flags, renderMode, scaleFactor, renderFlags);
		break;
	}
	}
}

void Render::renderSpecialBossAnim(int n, int frame, int x, int y, int z, int tileNum, int flags, int renderMode, int scaleFactor, int renderFlags) {
	Applet* app = CAppContainer::getInstance()->app;

	int anim = frame & Enums::MANIM_MASK;
	frame &= Enums::MFRAME_MASK;

	//anim = Enums::MANIM_IDLE;
	//frame = (app->time) / 1024 & 0x1;

	int n12 = 0;
	int n13 = (app->time + n * 1337) / 1024 & 0x1;
	int n14 = n13 * 26;
	Entity* entity = &app->game->entities[this->mapSprites[this->S_ENT + n]];
	int n15;
	int min;
	if (entity->monster != nullptr && (entity->monster->flags & 0x4000) != 0x0) {
		n15 = z;
		min = 32;
	}
	else {
		n15 = this->getHeight(x, y) + 32 << 4;
		min = std::min(std::max(32 - (z - n15), 0), 32);
	}
	int n16 = 65536 * min / 32;
	if (tileNum == Enums::TILENUM_BOSS_PINKY) {
		if (anim == Enums::MANIM_DEAD) {
			if (entity->monster != nullptr && (entity->monster->flags & 0x800) == 0x0 && app->canvas->state != 18 && !entity->hasEmptyLootSet()) {
				this->renderSprite(x, y, z, tileNum, 13, flags, 0, 17 * scaleFactor >> 4, 512);
			}
			this->renderSprite(x, y, z, tileNum, 13, flags, renderMode, scaleFactor, renderFlags);
			return;
		}
		int n17 = 3;
		if (anim == Enums::MANIM_ATTACK1 || anim == Enums::MANIM_ATTACK2) {
			n17 = 8;
		}
		else if (anim == Enums::MANIM_PAIN) {
			n17 = 12;
		}
		this->renderSprite(x, y, n15, 232, 0, flags, renderMode, n16, renderFlags);
		this->renderSprite(x, y, z, tileNum, 0 + n13, flags, renderMode, scaleFactor, renderFlags);
		this->renderSprite(x, y, z + n14 + 384, tileNum, 2, flags, renderMode, scaleFactor, renderFlags);
		this->renderSprite(x, y, z + n14 + 384, tileNum, n17, flags, renderMode, scaleFactor, renderFlags);
	}
	else if (tileNum >= Enums::TILENUM_BOSS_VIOS && tileNum <= Enums::TILENUM_BOSS_VIOS5) {
		renderMode = 3;
		if (app->game->angryVIOS) {
			renderFlags |= 0x80;
		}
		switch (anim) {
		case Enums::MANIM_IDLE:
		case Enums::MANIM_IDLE_BACK: {
			this->renderSprite(x, y, z, tileNum, 0, flags, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x, y, z + n14, tileNum, 2, flags, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x, y, z + n14, tileNum, 3, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		case Enums::MANIM_WALK_FRONT:
		case Enums::MANIM_WALK_BACK: {
			this->renderSprite(x, y, z, tileNum, 0 + (frame & 0x1), flags ^ (frame & 0x2) >> 1 << 17, renderMode, scaleFactor, renderFlags);
			int n18 = frame & 0x1;
			if ((frame & 0x2) == 0x0) {
				n18 = -n18;
			}
			x += n18 * this->viewRightStepX >> 6;
			y += n18 * this->viewRightStepY >> 6;
			this->renderSprite(x, y, z, tileNum, 2, flags, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x, y, z, tileNum, 3, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		case Enums::MANIM_ATTACK1:
		case Enums::MANIM_ATTACK2: {
			this->renderSprite(x, y, z, tileNum, 0, flags, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x, y, z, tileNum, 8 + (frame & 0x1), flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		case Enums::MANIM_PAIN: {
			renderMode = 5;
			this->renderSprite(x, y, z, tileNum, 0, flags, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x, y, z, tileNum, 2, flags, renderMode, scaleFactor, renderFlags);
			this->renderSprite(x, y, z, tileNum, 3, flags, renderMode, scaleFactor, renderFlags);
			break;
		}
		}
	}
	else if (tileNum == Enums::TILENUM_BOSS_MASTERMIND || tileNum == Enums::TILENUM_MONSTER_ARACHNOTRON) {
		bool b = tileNum == Enums::TILENUM_BOSS_MASTERMIND;
		int n19 = b ? -44 : -29;
		int n20 = 6;
		flags &= 0xFFFDFFFF;
		switch (anim) {
			case Enums::MANIM_IDLE:
			case Enums::MANIM_IDLE_BACK: {
				n14 = b ? n14 + 35 : (n14 / 2) + 50; // [GEC] ajusta el torzo para que se vea mejor
				this->renderSprite(x, y, n15, 232, 0, flags, renderMode, n16, renderFlags); // Shadow
				this->renderSprite(x, y, z + n14, tileNum, n12 + 3, flags, renderMode, scaleFactor, renderFlags); // Torzo
				break;
			}
			case Enums::MANIM_WALK_FRONT:
			case Enums::MANIM_WALK_BACK: {
				this->renderSprite(x, y, n15, 232, 0, flags, renderMode, n16, renderFlags); // Shadow
				int n21 = 0;
				if (frame == 0) {
					--n21;
				}
				else if (frame == 2) {
					++n21;
				}

				int zTorzo = b ? 35 : 50; // [GEC] ajusta el torzo para que se vea mejor
				// Torzo
				this->renderSprite(x + (n21 * this->viewRightStepX >> 6), y + (n21 * this->viewRightStepY >> 6), z + zTorzo, tileNum, n12 + 3, flags, renderMode, scaleFactor, renderFlags);
				
				// Legs
				switch (frame) {
					case 0: {
						this->renderSprite(x + (n19 * this->viewRightStepX >> 6), y + (n19 * this->viewRightStepY >> 6), z + (n20 - 1 << 4), tileNum, n12, flags, renderMode, scaleFactor, renderFlags);
						int n22 = -n19;
						flags ^= 0x20000;
						this->renderSprite(x + ((n22 - 1) * this->viewRightStepX >> 6), y + ((n22 - 1) * this->viewRightStepY >> 6), z + (n20 << 4), tileNum, n12 + 2, flags, renderMode, scaleFactor, renderFlags);
						return;
					}
					case 1: {
						this->renderSprite(x + ((n19 + 1) * this->viewRightStepX >> 6), y + ((n19 + 1) * this->viewRightStepY >> 6), z + (n20 - 1 << 4), tileNum, n12, flags, renderMode, scaleFactor, renderFlags);
						int n23 = -n19;
						flags ^= 0x20000;
						this->renderSprite(x + (n23 * this->viewRightStepX >> 6), y + (n23 * this->viewRightStepY >> 6), z + (n20 << 4), tileNum, n12 + 1, flags, renderMode, scaleFactor, renderFlags);
						return;
					}
					case 2: {
						this->renderSprite(x + ((n19 + 1) * this->viewRightStepX >> 6), y + ((n19 + 1) * this->viewRightStepY >> 6), z + (n20 << 4), tileNum, n12 + 1, flags, renderMode, scaleFactor, renderFlags);
						int n24 = -n19;
						flags ^= 0x20000;
						this->renderSprite(x + (n24 * this->viewRightStepX >> 6), y + (n24 * this->viewRightStepY >> 6), z + (n20 - 1 << 4), tileNum, n12, flags, renderMode, scaleFactor, renderFlags);
						return;
					}
					default: {
						break;
					}
				}
				break;
			}
			case Enums::MANIM_ATTACK1:
			case Enums::MANIM_ATTACK2: {
				int zTorzo = b ? 40 : 50; // [GEC] ajusta el torzo para que se vea mejor
				this->renderSprite(x, y, n15, 232, 0, flags, renderMode, n16, renderFlags); // Shadow
				this->renderSprite(x, y, z + zTorzo, tileNum, n12 + 3, flags, renderMode, scaleFactor, renderFlags); // Torzo
				this->renderSprite(x, y, z + zTorzo, tileNum, n12 + 8, flags, renderMode, scaleFactor, renderFlags); // Head Attack
				if (frame == 1 && b) { // Flash
					int n25;
					int n26;
					int pos = 0;// Old-> 2;
					zTorzo += 10;
					if ((flags & 0x20000) != 0x0) {
						n25 = x - (pos * this->viewRightStepX >> 6);
						n26 = y - (pos * this->viewRightStepY >> 6);
					}
					else {
						n25 = x + (pos * this->viewRightStepX >> 6);
						n26 = y + (pos * this->viewRightStepY >> 6);
					}
					this->renderSprite(n25, n26, z + 288 + zTorzo, tileNum, n12 + 9, (app->player->totalMoves + app->combat->animLoopCount & 0x3) << 17, 4, scaleFactor / 3, renderFlags);
					break;
				}
				break;
			}
			case Enums::MANIM_PAIN: {
				int zTorzo = b ? 70 : 100; // [GEC] ajusta el torzo para que se vea mejor
				this->renderSprite(x, y, n15, 232, 0, flags, renderMode, n16, renderFlags); // Shadow
				this->renderSprite(x, y, z + zTorzo, tileNum, n12 + 12, flags, renderMode, scaleFactor, renderFlags); // Torzo
				int zLegs = b ? 100 : 70; // [GEC] ajusta el torzo para que se vea mejor
				this->renderSprite(x + ((n19 + 3) * this->viewRightStepX >> 6), y + ((n19 + 3) * this->viewRightStepY >> 6), z + (n20 + 4 << 4) + zLegs, tileNum, n12 + 1, flags, renderMode, scaleFactor, renderFlags);
				int n27 = -n19;
				int pos = b ? 2 : 1; // [GEC] ajusta el torzo para que se vea mejor
				flags ^= 0x20000;
				this->renderSprite(x + ((n27 + pos/* - 1*/) * this->viewRightStepX >> 6), y + ((n27 + pos/* - 1*/) * this->viewRightStepY >> 6), z + (n20 + 3 << 4), tileNum, n12, flags, renderMode, scaleFactor, renderFlags);
				return;
			}
			case Enums::MANIM_DEAD: {
				if (entity->monster != nullptr && (entity->monster->flags & 0x800) == 0x0 && app->canvas->state != 18 && !entity->hasEmptyLootSet()) {
					this->renderSprite(x, y, z, tileNum, 13, flags, 0, 17 * scaleFactor >> 4, 512);
				}
				this->renderSprite(x, y, z, tileNum, 13, flags, renderMode, scaleFactor, renderFlags);
				return;
			}
		}
		// Legs
		this->renderSprite(x + (n19 * this->viewRightStepX >> 6), y + (n19 * this->viewRightStepY >> 6), z + (n20 << 4), tileNum, n12 + 2, flags, renderMode, scaleFactor, renderFlags);
		int n28 = -n19;
		flags ^= 0x20000;
		this->renderSprite(x + ((n28 - 1) * this->viewRightStepX >> 6), y + ((n28 - 1) * this->viewRightStepY >> 6), z + (n20 - 1 << 4), tileNum, n12, flags, renderMode, scaleFactor, renderFlags);
	}
}

void Render::handleMonsterIdleSounds(Entity* entity) {
	Applet* app = CAppContainer::getInstance()->app;

	if ((this->monsterIdleTime[entity->def->eSubType] != 0) && (this->monsterIdleTime[entity->def->eSubType] <= app->time)) {
		if (entity->distFrom(app->canvas->viewX, app->canvas->viewY) < app->combat->tileDistances[3]) {
			int MonsterSound = app->game->getMonsterSound(entity->def->eSubType, (char)entity->def->parm, Enums::MSOUND_IDLE);
			app->sound->playSound(MonsterSound, 0, 1, 0);
		}
		this->monsterIdleTime[entity->def->eSubType] = ((app->nextByte() % 10) * 1000) + app->time + 6000;
	}
}


//--------------------
void DrawBitmap(short* buffer, int buffW, int buffH, int x, int y, int w, int h)
{
	static GLuint textureName = -1;
	float vp[12]; // [sp+18h] [bp-68h] BYREF
	float st[8]; // [sp+48h] [bp-38h] BYREF

	PFNGLACTIVETEXTUREPROC glActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");

	vp[2] = 0.0;
	vp[5] = 0.0;
	vp[8] = 0.0;
	vp[11] = 0.0;
	vp[0] = (float)(x + w);
	vp[1] = (float)y;
	vp[6] = vp[0];
	vp[4] = (float)y;
	vp[3] = (float)x;
	vp[9] = (float)x;
	vp[7] = (float)(y + h);
	vp[10] = vp[7];
	memset(&st[1], 0, 12);
	st[6] = 0.0;
	st[0] = (float)w / (float)buffW;
	st[4] = st[0];
	st[5] = (float)h / (float)buffH;
	st[7] = st[5];
	if (textureName == -1) {
		glGenTextures(1, &textureName);
	}
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffW, buffH, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);
	glVertexPointer(3, GL_FLOAT, 0, vp);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::Render3dScene(void) {
	short* backBuffer = CAppContainer::getInstance()->GetBackBuffer();

	int cx, cy;
	int w = CAppContainer::getInstance()->sdlGL->vidWidth;
	int h = CAppContainer::getInstance()->sdlGL->vidHeight;
	SDL_GetWindowSize(CAppContainer::getInstance()->sdlGL->window, &cx, &cy);
	if (w != cx || h != cy) {
		w = cx; h = cy;
	}
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, Applet::IOS_WIDTH, Applet::IOS_HEIGHT, 0.0, -1.0, 1.0);
	//glRotatef(90.0, 0.0, 0.0, 1.0);
	//glTranslatef(0.0, -320.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//DrawBitmap(backBuffer, 480, 320, 0, 21, 480, 320); // <- Old
	DrawBitmap(backBuffer, 480, 320, 0, 0, 480, 320);
}

void Render::fixTexels(int offset, int i, int mediaID, int* rowHeight) { // [GEC] New
	if (mediaID == 814 && this->fixWaterAnim1) {
		if (offset == 5614 && (i & 1) == 0) { *rowHeight = 16; }
	}
	else if (mediaID == 815 && this->fixWaterAnim2) {
		if (offset == 7521 && (i & 1) == 0) { *rowHeight = 16; }
		if (offset == 7522 && (i & 1) == 1) { *rowHeight = 18; }
		if (offset == 7542 && (i & 1) == 1) { *rowHeight = 0; }
	}
	else if (mediaID == 816 && this->fixWaterAnim3) {
		if (offset == 7397 && (i & 1) == 0) { *rowHeight = 16; }
		if (offset == 7397 && (i & 1) == 1) { *rowHeight = 17; }
		if (offset == 7415 && (i & 1) == 0) { *rowHeight = 0; }
	}
	else if (mediaID == 817 && this->fixWaterAnim4) {
		if (offset == 6889 && (i & 1) == 1) { *rowHeight = 17; }
	}
}