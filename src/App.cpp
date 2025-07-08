#include <stdexcept>
#include <cstdio>
#include <algorithm>

#include "SDLGL.h"
#include "ZipFile.h"

#include "CAppContainer.h"
#include "App.h"
#include "IDIB.h"
#include "Text.h"
#include "Resource.h"
#include "Render.h"
#include "TinyGL.h"
#include "Canvas.h"
#include "Combat.h"
#include "Game.h"
#include "MenuSystem.h"
#include "Player.h"
#include "Sound.h"
#include "Combat.h"
#include "Hud.h"
#include "EntityDef.h"
#include "ParticleSystem.h"
#include "HackingGame.h"
#include "SentryBotGame.h"
#include "VendingMachine.h"
#include "ComicBook.h"
#include "JavaStream.h"
#include "Image.h"
#include "Graphics.h"

Applet::Applet() {
	std::memset(this, 0, sizeof(Applet));
}

Applet::~Applet() {
}

bool Applet::startup() {
	printf("Applet::startup\n");

	this->closeApplet = false;
	this->fontType = 0;
	this->accelerationIndex = 0;
	this->field_0x290 = '\0';
	this->field_0x291 = '\0';

	// Iphone Only
	{
		for (int i = 0; i < 32; i++) {
			accelerationX[i] = 0.0f;
			accelerationY[i] = 0.0f;
			accelerationZ[i] = 0.0f;
		}
	}

	this->field_0x414 = 0;
	this->field_0x418 = 0;
	this->field_0x41c = 0;
	this->field_0x420 = 0;
	this->field_0x424 = 0;
	this->field_0x428 = 0;

	this->backBuffer = new IDIB;
	this->backBuffer->pBmp =  new uint8_t[480 * 320 *2];
	std::memset(this->backBuffer->pBmp, 0, 480 * 320 * 2);
	this->backBuffer->pRGB888 = nullptr;
	this->backBuffer->pRGB565 = nullptr;
	this->backBuffer->width = CAppContainer::getInstance()->sdlGL->vidWidth;
	this->backBuffer->height = CAppContainer::getInstance()->sdlGL->vidHeight;
	this->backBuffer->pitch = CAppContainer::getInstance()->sdlGL->vidWidth;

	printf("w: %d || h: %d\n", backBuffer->width, backBuffer->height);

	this->initLoadImages = false;
	this->time = 0;
	this->upTimeMs = 0;
	this->field_0x7c = 0;
	this->field_0x80 = 0;
	this->canvas = new Canvas;
	this->resource = new Resource;
	this->localization = new Localization;
	this->render = new Render;
	this->tinyGL = new TinyGL;
	this->game = new Game;
	this->menuSystem = new MenuSystem;
	this->player = new Player;
	this->sound = new Sound;
	this->combat = new Combat;
	this->hud = new Hud;
	this->entityDefManager = new EntityDefManager;
	this->particleSystem = new ParticleSystem;
	this->hackingGame = new HackingGame;
	this->sentryBotGame = new SentryBotGame;
	this->vendingMachine = new VendingMachine;
	this->comicBook = new ComicBook;

	Applet::loadConfig();
	//this->moreGames = getStartupVarBool("More_Games", false);
	//this->systemFont = getStartupVarBool("System_Font", false);
	int time = this->upTimeMs;
	this->gameTime = this->upTimeMs;
	this->startupMemory = Applet::MAXMEMORY;

	if (this->canvas->startup()) {
		this->testImg = Applet::loadImage("cockpit.bmp", true);

		this->canvas->loadMiniGameImages();
		if (this->localization->startup()) {
			if (this->render->startup()) {
				this->resource->initTableLoading();
				this->loadTables();

				if (this->tinyGL->startup(this->render->screenWidth, this->render->screenHeight)) {
					if (this->entityDefManager->startup()) {
						if (this->player->startup()) {
							if (this->menuSystem->startup()) {
								if (this->sound->startup()) {
									if (this->game->startup()) {
										if (this->particleSystem->startup()) {
											if (this->combat->startup()) {

												this->game->loadConfig();
												if (this->canvas->isFlipControls != false) {
													this->canvas->isFlipControls = false;
													this->canvas->flipControls();
												}

												this->canvas->setControlLayout();
												this->canvas->clearEvents(1);
												this->canvas->setState(Canvas::ST_LOGO);
												this->canvas->graphics.backBuffer = this->backBuffer;
												this->canvas->graphics.graphClipRect[0] = 0;
												this->canvas->graphics.graphClipRect[1] = 0;
												this->canvas->graphics.graphClipRect[2] = this->backBuffer->width;
												this->canvas->graphics.graphClipRect[3] = this->backBuffer->height;

												this->accelerationIndex = 0;
												this->field_0x290 = false;
												this->field_0x291 = '\0';
												//this->accelStart();
												printf("**** Startup took %i ms\n", this->upTimeMs - time);
												printf("**** Fragment size %i ms\n", 0);

												return true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	printf("error faltal:\n");
	return false;
}

void Applet::loadConfig() {

}

Image* Applet::createImage(InputStream* inputStream, bool isTransparentMask)
{
	Image* img;
	int Width, Height, offBeg, BitsPerPixel, ColorsUsed, rgb, pitch;
	img = (Image*)std::malloc(sizeof(Image));
	img->texture = -1;
	img->piDIB = (IDIB*)std::malloc(sizeof(IDIB));
	img->piDIB->pBmp = nullptr;
	img->piDIB->pRGB888 = nullptr;
	img->piDIB->pRGB565 = nullptr;

	// read header
    inputStream->offsetCursor(10);
	offBeg = inputStream->readInt();
    inputStream->offsetCursor(4);
	Width = inputStream->readInt();
	Height = inputStream->readInt();
    inputStream->offsetCursor(2);
	BitsPerPixel = inputStream->readShort();
    inputStream->offsetCursor(16);
	ColorsUsed = inputStream->readInt();
    inputStream->offsetCursor(4);

	//printf("offBeg %d\n", offBeg);
	//printf("Width %d\n", Width);
	//printf("Height %d\n", Height);
	//printf("BitsPerPixel %d\n", BitsPerPixel);
	//printf("ColorsUsed %d\n", ColorsUsed);

	// read data

	if (BitsPerPixel == 4 || BitsPerPixel == 8) {

		if (ColorsUsed == 0) {
			ColorsUsed = 1 << (BitsPerPixel & 0xff);
		}
		//printf("ColorsUsed %d\n", ColorsUsed);

		img->piDIB->cntRGB = ColorsUsed;
		img->piDIB->nColorScheme = 0;
		img->piDIB->pRGB888 = (uint32_t*)std::malloc(img->piDIB->cntRGB * sizeof(uint32_t));
		img->piDIB->pRGB565 = (uint16_t*)std::malloc(img->piDIB->cntRGB * sizeof(uint16_t));

		// read palette
		std::memcpy(img->piDIB->pRGB888, inputStream->getTop(), img->piDIB->cntRGB * sizeof(uint32_t));
		inputStream->offsetCursor(img->piDIB->cntRGB * sizeof(uint32_t));

		img->isTransparentMask = isTransparentMask;

		if (isTransparentMask) {
			for (uint32_t i = 0; i < img->piDIB->cntRGB; i++) {
				rgb = img->piDIB->pRGB888[i];
				if (rgb == 0xff00ff) {
					img->piDIB->ncTransparent = i;
				}
				else {
					if (((rgb >> 8 & 0xf800U) | (rgb >> 5 & 0x07e0U) | ((rgb >> 3) & 0x001f)) == 0) { // rgb888 to rgb565
						img->piDIB->pRGB888[i] = 8;
					}
				}
			}
		}

		for (uint32_t i = 0; i < img->piDIB->cntRGB; i++) {
			rgb = img->piDIB->pRGB888[i];
			img->piDIB->pRGB565[i] = (rgb >> 8 & 0xf800) | (rgb >> 5 & 0x07e0) | (rgb >> 3 & 0x001f); // rgb888 to rgb565
		}

		// read pixels
		img->width = img->piDIB->width;
		img->height = img->piDIB->height;
		img->depth = img->piDIB->depth;

		pitch = BitsPerPixel * Width;
		int _pitch = pitch;
		if ((pitch & 7) != 0) {
			//printf("pitch 7 %d\n", pitch);
			ColorsUsed = (uint32_t)((int)pitch >> 0x1f) >> 0x1d;
			_pitch = (pitch - ((pitch + ColorsUsed & 7) - ColorsUsed)) + 8;
		}

		if ((pitch & 0x1f) != 0) {
			//printf("pitch 31 %d\n", pitch);
			ColorsUsed = (uint32_t)((int)pitch >> 0x1f) >> 0x1b;
			pitch = (pitch - ((pitch + ColorsUsed & 0x1f) - ColorsUsed)) + 0x20;
		}
		ColorsUsed = pitch;
		if ((int)pitch < 0) {
			ColorsUsed = pitch + 7;
		}
		int iVar8 = (int)ColorsUsed >> 3;
		ColorsUsed = pitch + 7;
		if ((int)_pitch < 0) {
			ColorsUsed = _pitch + 7;
		}
		int sVar6 = Height * Width;
		if ((int)_pitch >= 0) {
			ColorsUsed = _pitch;
		}

		//printf("sVar6 %d\n", sVar6);
		img->piDIB->pBmp = (uint8_t*)std::malloc(sVar6 * sizeof(int16_t));
		img->piDIB->width = Width;
		img->piDIB->pitch = Width;
		img->piDIB->depth = BitsPerPixel;
		img->piDIB->height = Height;

		bool bVar7 = BitsPerPixel != 4;

		if (bVar7) {
			Width = 0;
		}
		img->height = img->piDIB->height;
		if (!bVar7) {
			Width = Height - 1;
		}
		img->depth = img->piDIB->depth;

		uint8_t *data = inputStream->getTop();
		if (bVar7) {
			for (; Width < Height; Width = Width + 1) {
				std::memcpy(img->piDIB->pBmp + img->piDIB->pitch * Width, data + iVar8 * (Height + (-1 - Width)),
					(int)ColorsUsed >> 3);
			}
		}
		else {
			for (int i = 0; i < Height; i++) {
				std::memcpy(img->piDIB->pBmp + (i * img->piDIB->pitch >> 1), data + iVar8 * Width, (int)ColorsUsed >> 3);
				Width = Width - 1;
			}
		}
		inputStream->offsetCursor(iVar8 * Height);

		if ((short)BitsPerPixel == 4) {
			uint8_t* pbVar4 = (uint8_t*)std::malloc(sVar6);
			uint8_t* _data = pbVar4;
			for (ColorsUsed = 0; ColorsUsed < sVar6 >> 1; ColorsUsed = ColorsUsed + 1) {
				uint8_t bVar1 = img->piDIB->pBmp[ColorsUsed];
				_data[0] = bVar1 >> 4;
				_data[1] = bVar1 & 0xf;
				_data += 2;
			}
			for (ColorsUsed = 0; ColorsUsed < sVar6; ColorsUsed = ColorsUsed + 1) {
				img->piDIB->pBmp[ColorsUsed] = pbVar4[ColorsUsed];
			}
			std::free(pbVar4);
		}
		img->width = img->piDIB->width;
		img->height = img->piDIB->height;
	}
	else {
		img->~Image();
		img = nullptr;

		Error("Expected image bpp 4 or 8. Found bpp %d", BitsPerPixel);
	}

	return img;
}

Image* Applet::loadImage(char* fileName, bool isTransparentMask) {
	InputStream iStream = InputStream();
	Image* img;

	if (iStream.loadResource(fileName) == 0) {
		Applet::Error("Failed to open image: %s", fileName);
	}

	img = this->createImage(&iStream, isTransparentMask);

	iStream.close();
	iStream.~InputStream();
	return img;
}

#include <stdarg.h> //va_list|va_start|va_end
void Applet::Error(const char* fmt, ...) {

	char errMsg[256];
	va_list ap;
	va_start(ap, fmt);
#ifdef _WIN32
	vsprintf_s(errMsg, sizeof(errMsg), fmt, ap);
#else
	vsnprintf(errMsg, sizeof(errMsg), fmt, ap);
#endif
	va_end(ap);

	SDL_SetWindowFullscreen(CAppContainer::getInstance()->sdlGL->window, 0);

	printf("%s", errMsg);

	const SDL_MessageBoxButtonData buttons[] = {
		{ /* .flags, .buttonid, .text */        0, 0, "Ok" },
	};
	const SDL_MessageBoxColorScheme colorScheme = {
		{ /* .colors (.r, .g, .b) */
			/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
			{ 255,   0,   0 },
			/* [SDL_MESSAGEBOX_COLOR_TEXT] */
			{   0, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
			{ 255, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
			{   0,   0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
			{ 255,   0, 255 }
		}
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_ERROR, /* .flags */
		NULL, /* .window */
		"Doom II RPG Error", /* .title */
		errMsg, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&colorScheme /* .colorScheme */
	};

	SDL_ShowMessageBox(&messageboxdata, NULL);

	CAppContainer::getInstance()->~CAppContainer();
	exit(0);
}

void Applet::Error(int id)
{
	std::printf("Error id: %i\n", id);
	this->Error("App Error");
	this->idError = id;
}

void Applet::beginImageLoading() {
}

void Applet::endImageLoading() {
}

void Applet::loadTables() {
	int count01, count02, count03, count04, count05, count06, count07;
	int count08, count09, count10, count11, count12, count13, count14;

	count01 = this->resource->getNumTableShorts(0); // count TBL_COMBAT_MONSTERATTACKS
	count02 = this->resource->getNumTableBytes(1);	// count TBL_COMBAT_WEAPONINFO
	count03 = this->resource->getNumTableBytes(2);	// count TBL_COMBAT_WEAPONDATA
	count04 = this->resource->getNumTableBytes(3);	// count TBL_COMBAT_MONSTERSTATS
	count05 = this->resource->getNumTableInts(4);	// count TBL_COMBAT_COMBATMASKS
	count06 = this->resource->getNumTableBytes(5);	// count TBL_CANVAS_KEYSNUMERIC
	count07 = this->resource->getNumTableBytes(6);	// count TBL_ENUMS_OSC_CYCLE
	count08 = this->resource->getNumTableShorts(7); // count TBL_GAME_LEVELNAMES
	count09 = this->resource->getNumTableBytes(8);	// count TBL_MONSTER_COLORS
	count10 = this->resource->getNumTableInts(9);	// count TBL_RENDER_SINETABLE
	count11 = this->resource->getNumTableShorts(10);// count TBL_ENERGY_DRINK_DATA
	count12 = this->resource->getNumTableBytes(11); // count TBL_MONSTER_WEAKNESS
	count13 = this->resource->getNumTableInts(12);	// count TBL_UNK
	count14 = this->resource->getNumTableBytes(13); // count TBL_MONSTER_SOUNDS

	this->combat->monsterAttacks = new short[count01];
	this->combat->wpinfo = new int8_t[count02];
	this->combat->weapons = new int8_t[count03];
	this->combat->monsterStats = new int8_t[count04];
	this->combat->tableCombatMasks = new int32_t[count05];
	this->canvas->keys_numeric = new int8_t[count06];
	this->canvas->OSC_CYCLE = new int8_t[count07];
	this->game->levelNames = new int16_t[count08];
	this->particleSystem->monsterColors = new uint8_t[count09];
	this->render->sinTable = new int32_t[count10];
	this->vendingMachine->energyDrinkData = new int16_t[count11];
	this->combat->monsterWeakness = new int8_t[count12];
	this->canvas->movieEffects = new int32_t[count13];
	this->game->monsterSounds = new uint8_t[count14];

	this->resource->beginTableLoading();
	this->resource->loadShortTable(this->combat->monsterAttacks, 0);
	this->resource->loadByteTable(this->combat->wpinfo, 1);
	this->resource->loadByteTable(this->combat->weapons, 2);
	this->resource->loadByteTable(this->combat->monsterStats, 3);
	this->resource->loadIntTable(this->combat->tableCombatMasks, 4);
	this->resource->loadByteTable(this->canvas->keys_numeric, 5);
	this->resource->loadByteTable(this->canvas->OSC_CYCLE, 6);
	this->game->levelNamesCount = this->resource->loadShortTable(this->game->levelNames, 7);
	this->resource->loadByteTable((int8_t*)this->particleSystem->monsterColors, 8);
	this->resource->loadIntTable(this->render->sinTable, 9);
	this->resource->loadShortTable(this->vendingMachine->energyDrinkData, 10);
	this->resource->loadByteTable(this->combat->monsterWeakness, 11);
	this->resource->loadIntTable(this->canvas->movieEffects, 12);
	this->resource->loadByteTable((int8_t*)this->game->monsterSounds, 13);
	this->resource->finishTableLoading();

	//for (int i = 0; i < count14; i++) {
	//	printf("data[%d] = %d\n", i, (uint8_t)this->game->monsterSounds[i]);
	//}
}

void Applet::loadRuntimeImages() {
	//printf("Applet::loadRuntimeImages\n");
	//printf("this->initLoadImages %d\n", this->initLoadImages);
	if (this->initLoadImages == false) {
		this->imageMemory = 1000000000;

		this->canvas->imgMapCursor = this->loadImage("Automap_Cursor.bmp", true);
		this->canvas->imgUIImages = this->loadImage("ui_images.bmp", true);
		this->hud->imgCockpitOverlay = this->loadImage("cockpit.bmp", true);
		this->hud->imgDamageVignette = this->loadImage("damage.bmp", true);
		this->hud->imgDamageVignetteBot = this->loadImage("damage_bot.bmp", true);
		this->canvas->imgDialogScroll = this->loadImage("DialogScroll.bmp", true);
		this->hud->imgActions = this->loadImage("Hud_Actions.bmp", true);
		this->hud->imgBottomBarIcons = this->loadImage("Hud_Fill.bmp", true);
		this->hud->imgHudFill = this->loadImage("Hud_Actions.bmp", true);
		
		this->hud->imgPlayerFrameNormal->~Image();
		this->hud->imgPlayerFrameNormal = nullptr;
		this->hud->imgPlayerFrameActive->~Image();
		this->hud->imgPlayerFrameActive = nullptr;

		this->hud->imgPlayerFrameNormal = this->loadImage("HUD_Player_frame_Normal.bmp", true);
		this->hud->imgPlayerFrameActive = this->loadImage("HUD_Player_frame_Active.bmp", true);

		this->hud->imgPlayerFaces->~Image();
		this->hud->imgPlayerFaces = nullptr;
		this->hud->imgPlayerActive->~Image();
		this->hud->imgPlayerActive = nullptr;

		if (this->player->characterChoice == 1) {
			this->hud->imgPlayerFaces = this->loadImage("Hud_Player.bmp", true);
			this->hud->imgPlayerActive = this->loadImage("HUD_Player_Active.bmp", true);
		}
		else if (this->player->characterChoice == 2) {
			this->hud->imgPlayerFaces = this->loadImage("Hud_PlayerDoom.bmp", true);
			this->hud->imgPlayerActive = this->loadImage("HUD_PlayerDoom_Active.bmp", true);
		}
		else if (this->player->characterChoice == 3) {
			this->hud->imgPlayerFaces = this->loadImage("Hud_PlayerScientist.bmp", true);
			this->hud->imgPlayerActive = this->loadImage("HUD_PlayerScientist_Active.bmp", true);
		}

		this->hud->imgSentryBotFace = this->loadImage("Hud_Sentry.bmp", true);
		this->hud->imgSentryBotActive = this->loadImage("HUD_sentry_active.bmp", true);

		this->hud->imgHudTest = this->loadImage("Hud_Test.bmp", true);
		this->hud->imgScope = this->loadImage("scope.bmp", true);

		this->canvas->updateLoadingBar(false);
		this->imageMemory = this->imageMemory + -1000000000;
	}
}

void Applet::freeRuntimeImages() {
	this->canvas->imgMapCursor->~Image();
	this->canvas->imgMapCursor = nullptr;
	this->canvas->imgUIImages->~Image();
	this->canvas->imgUIImages = nullptr;
	this->canvas->imgDialogScroll->~Image();
	this->canvas->imgDialogScroll = nullptr;
	this->hud->imgScope->~Image();
	this->hud->imgScope = nullptr;
	this->hud->imgDamageVignette->~Image();
	this->hud->imgDamageVignette = nullptr;
	this->hud->imgActions->~Image();
	this->hud->imgActions = nullptr;
	this->hud->imgBottomBarIcons->~Image();
	this->hud->imgBottomBarIcons = nullptr;
	this->hud->imgHudFill->~Image();
	this->hud->imgHudFill = nullptr;
	this->hud->imgPlayerFaces->~Image();
	this->hud->imgPlayerFaces = nullptr;
	this->hud->imgPlayerActive->~Image();
	this->hud->imgPlayerActive = nullptr;
	this->hud->imgDamageVignetteBot->~Image();
	this->hud->imgDamageVignetteBot = nullptr;
	this->hud->imgHudTest->~Image();
	this->hud->imgHudTest = nullptr;
	this->hud->imgSentryBotFace->~Image();
	this->hud->imgSentryBotFace = nullptr;
	this->hud->imgSentryBotActive->~Image();
	this->hud->imgSentryBotActive = nullptr;
	this->hud->imgCockpitOverlay->~Image();
	this->hud->imgCockpitOverlay = nullptr;
}

void Applet::setFont(int fontType) {
	if (fontType <= 3) {
		this->fontType = fontType;
	}
}

void Applet::shutdown() {
	this->closeApplet = true;
}

uint32_t Applet::nextInt() {
	return std::rand() & INT32_MAX;
}

uint32_t Applet::nextByte() {
	return this->nextInt() & UINT8_MAX;
}

void Applet::setFontRenderMode(int fontRenderMode) {
	this->canvas->fontRenderMode = fontRenderMode;
}


void Applet::AccelerometerUpdated(float x, float y, float z) {

	this->accelerationX[this->accelerationIndex] = x;
	this->accelerationY[this->accelerationIndex] = y;
	this->accelerationZ[this->accelerationIndex] = z;
	this->accelerationIndex = (this->accelerationIndex + 1) % 32;

	int v7 = (uint8_t)this->field_0x291;
	int v8 = v7 == 0;
	if (!v7) {
		v8 = this->accelerationIndex == 0;
	}
	if (v8) {
		this->field_0x291 = v7 + 1;
	}
	//this->comicBook->UpdateAccelerometer(x, y, z);
}

void Applet::StartAccelerometer() {
	this->accelerationIndex = 0;
	this->field_0x290 = false;
	this->field_0x291 = false;
}

void Applet::StopAccelerometer() {
	this->accelerationIndex = 0;
	this->field_0x290 = false;
	this->field_0x291 = false;
}

void Applet::CalcAccelerometerAngles() {
	bool v2; // zf
	float y; // s13
	int v5; // r2
	float x; // s14
	float z; // s12
	float v9; // s11
	float v10; // s14
	int zoomAngle; // r3
	int v14; // r3
	int zoomMaxAngle; // r3
	int zoomPitch; // r1

	v2 = this->field_0x291 == false;
	if (this->field_0x291)
	{
		v2 = !this->canvas->isZoomedIn;
	}
	if (!v2)
	{
		v5 = 0;
		x = 0.0;
		y = 0.0;
		z = 0.0;
		do{
			x += this->accelerationX[v5];
			y += this->accelerationY[v5];
			z += this->accelerationZ[v5];
		} while (++v5 < 32);

		this->field_0x414 = x * 0.03125;
		this->field_0x418 = y * 0.03125;
		this->field_0x41c = z * 0.03125;

		if (!this->field_0x290)
		{
			this->field_0x290 = true;
			this->field_0x420 = x * 0.03125;
			this->field_0x424 = y * 0.03125;
			this->field_0x428 = z * 0.03125;
			return;
		}
		this->canvas->zoomAngle = (int)(float)((float)(this->field_0x414 - this->field_0x420) * 420.0);

		zoomAngle = this->canvas->zoomAngle;
		if (zoomAngle >= -200)
		{
			if (zoomAngle <= 200)
				goto LABEL_13;
			v14 = 200;
		}
		else
		{
			v14 = -200;
		}
		this->canvas->zoomAngle = v14;
		this->canvas = this->canvas;
	LABEL_13:
		this->canvas->zoomPitch = (int)(float)((float)(this->field_0x418 - this->field_0x424) * 420.0);
		zoomMaxAngle = this->canvas->zoomMaxAngle;
		zoomPitch = this->canvas->zoomPitch;
		if (zoomPitch >= -zoomMaxAngle)
		{
			if (zoomPitch > zoomMaxAngle)
				this->canvas->zoomPitch = zoomMaxAngle;
		}
		else
		{
			this->canvas->zoomPitch = -zoomMaxAngle;
		}
	}
}
