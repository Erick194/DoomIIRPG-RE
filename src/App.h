#ifndef __APP_H__
#define __APP_H__

#include "IDIB.h"

class SDLGL;
class ZipFile;
class IDIB;
class Image;

class Localization;
class Resource;
class Render;
class TinyGL;
class Canvas;
class Game;
class MenuSystem;
class Player;
class Sound;
class Combat;
class Hud;
class EntityDefManager;
class ParticleSystem;
class HackingGame;
class SentryBotGame;
class VendingMachine;
class ComicBook;
class InputStream;

class Applet
{
private:

public:
	// Defines
	static constexpr int MAXMEMORY = 1000000000;
	static constexpr int IOS_WIDTH = 480;
	static constexpr int IOS_HEIGHT = 320;

	static constexpr int FONT_HEIGHT[4] = { 16, 16, 18, 25 };
	static constexpr int FONT_WIDTH[4] = { 12, 12, 13, 22 };
	static constexpr int CHAR_SPACING[4] = { 11, 11, 12, 22 };

	//------------------
	IDIB* backBuffer;
	int upTimeMs;
	int lastTime;
	int time;
	int gameTime;
	int startupMemory;
	int imageMemory;
	char* peakMemoryDesc;
	int peakMemoryUsage;
	int idError;
	bool initLoadImages;

	int sysAdvTime;
	int osTime[8];
	int codeTime[8];
	int field_0x26c;
	int field_0x270;
	int field_0x278;
	int field_0x27c;
	int fontType;
	int accelerationIndex;
	bool field_0x290;
	bool field_0x291;
	int field_0x7c;
	int field_0x80;

	// Iphone Only
	float accelerationX[32];
	float accelerationY[32];
	float accelerationZ[32];

	float field_0x414;
	float field_0x418;
	float field_0x41c;
	float field_0x420;
	float field_0x424;
	float field_0x428;
	bool closeApplet;

	//-------------------------
	Localization* localization;
	Resource* resource;
	Render* render;
	TinyGL* tinyGL;
	Canvas* canvas;
	Game* game;
	MenuSystem* menuSystem;
	Player* player;
	Sound* sound;
	Combat* combat;
	Hud* hud;
	EntityDefManager* entityDefManager;
	ParticleSystem* particleSystem;
	HackingGame* hackingGame;
	SentryBotGame* sentryBotGame;
	VendingMachine* vendingMachine;
	ComicBook* comicBook;
	//-------------------------
	Image* testImg;
	int seed;

	// Constructor
	Applet();
	// Destructor
	~Applet();

	bool startup();
	void loadConfig();

	Image* createImage(InputStream* inputStream, bool isTransparentMask);
	Image* loadImage(char* fileName, bool isTransparentMask);

	void beginImageLoading();
	void endImageLoading();

	void Error(const char* fmt, ...);
	void Error(int id);
	void loadTables();

	void loadRuntimeImages();
	void freeRuntimeImages();
	void setFont(int fontType);
	void shutdown();
	uint32_t nextInt();
	uint32_t nextByte();
	void setFontRenderMode(int fontRenderMode);

	void Applet::AccelerometerUpdated(float x, float y, float z);
	void Applet::StartAccelerometer();
	void Applet::StopAccelerometer();
	void Applet::CalcAccelerometerAngles();
};


#endif