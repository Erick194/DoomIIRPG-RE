#ifndef __CAPPCONTAINER_H__
#define __CAPPCONTAINER_H__
#include "App.h"

class ZipFile;
class SDLGL;
class Applet;

class CAppContainer
{
private:

public:
	Applet* app;
	SDLGL* sdlGL; // New
	ZipFile* zipFile; // New
	static CAppContainer* getInstance();
	static int m_cheatEntry;
	float MoveX, MoveY, MoveAng;

	// Constructor
	CAppContainer();
	// Destructor
	~CAppContainer();

	short* GetBackBuffer();
	void DoLoop(int time);
	void suspendOpenAL();
	void resumeOpenAL();
	void userPressed(float pressX, float pressY);
	void userMoved(float pressX, float pressY);
	void userReleased(float pressX, float pressY);
	void unHighlightButtons();
	uint32_t getTimeMS();
	void saveGame(int i, int * i2);
	void TestCheatEntry(float pressX, float pressY);
	bool testCheatCode(int code);
	void UpdateAccelerometer(float x, float y, float z, bool useMouse);
	void Construct(SDLGL* sdlGL, ZipFile* zipFile);

};

#endif