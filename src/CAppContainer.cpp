#include <stdexcept>

#include "SDLGL.h"
#include "ZipFile.h"
#include "App.h"
#include "Canvas.h"
#include "Game.h"
#include "CAppContainer.h"
#include "Utils.h"
#include "MenuSystem.h"
#include "Player.h"
#include "Sound.h"
#include "Menus.h"

static CAppContainer _mContainer;
int CAppContainer::m_cheatEntry = 0;

CAppContainer::CAppContainer() {
	this->MoveX = 0.0;
	this->MoveY = 0.0;
	this->MoveAng = 0.0;
}

CAppContainer::~CAppContainer() {

	if (this->app) {

		if (this->app->sound) {
			this->app->sound->~Sound();
		}
		delete this->app;
		this->app = nullptr;
	}

	if (this->sdlGL) {
		this->sdlGL->~SDLGL();
		this->sdlGL = nullptr;
	}

	if (this->zipFile) {
		this->zipFile->closeZipFile();
		this->zipFile = nullptr;
	}
}

CAppContainer* CAppContainer::getInstance() {
	return &_mContainer;
}

short* CAppContainer::GetBackBuffer()
{
	return (short*)this->app->backBuffer->pBmp;
}

void CAppContainer::DoLoop(int time) {
	this->app->sound->startFrame();
	this->app->canvas->staleView = true;
	this->app->canvas->run();
	this->app->sound->endFrame();
	this->app->upTimeMs += time;
}

void CAppContainer::suspendOpenAL() {

}

void CAppContainer::resumeOpenAL() {

}

void CAppContainer::userPressed(float pressX, float pressY) {
	int x = (int)(pressX * Applet::IOS_WIDTH);
	int y = (int)(pressY * Applet::IOS_HEIGHT);
	//printf("userPressed [x %d, y %d]\n", x, y);
	this->app->canvas->touchStart(x, /*Applet::IOS_HEIGHT - */y);
}

void CAppContainer::userMoved(float pressX, float pressY) {
	int x = (int)(pressX * Applet::IOS_WIDTH);
	int y = (int)(pressY * Applet::IOS_HEIGHT);
	//printf("userMoved [x %d, y %d]\n", x, y);
	this->app->canvas->touchMove(x, /*Applet::IOS_HEIGHT - */y);
}

void CAppContainer::userReleased(float pressX, float pressY) {
	int x = (int)(pressX * Applet::IOS_WIDTH);
	int y = (int)(pressY * Applet::IOS_HEIGHT);
	//printf("userReleased [x %d, y %d]\n", x, y);
	this->app->canvas->touchEnd(x, /*Applet::IOS_HEIGHT -*/ y);
}

void CAppContainer::unHighlightButtons() {
	this->app->canvas->touchEndUnhighlight();
}

uint32_t CAppContainer::getTimeMS() {
	return SDL_GetTicks();
}

void CAppContainer::saveGame(int i, int* i2) {

}

void CAppContainer::TestCheatEntry(float pressX, float pressY) {
	int x = (int)(pressX * Applet::IOS_WIDTH);
	int y = (int)(pressY * Applet::IOS_HEIGHT);

	if (pointInRectangle(x, y, 0, 0, 60, 60)) {
		m_cheatEntry *= 10;
		m_cheatEntry += 1;
	}
	else if (pointInRectangle(x, y, (Applet::IOS_WIDTH - 60), 0, 60, 60)) {
		m_cheatEntry *= 10;
		m_cheatEntry += 2;
	}
	else if (pointInRectangle(x, y, 0, (Applet::IOS_HEIGHT - 60), 60, 60)) {
		m_cheatEntry *= 10;
		m_cheatEntry += 3;
	}
	else if (pointInRectangle(x, y, (Applet::IOS_WIDTH - 60), (Applet::IOS_HEIGHT - 60), 60, 60)) {
		m_cheatEntry *= 10;
		m_cheatEntry += 4;
	}
	else {
		m_cheatEntry = 0;
	}

	if (m_cheatEntry > 100000) {
		m_cheatEntry %= 1000000;
	}

	if (this->testCheatCode(m_cheatEntry)) {
		m_cheatEntry = 0;
	}
}

bool CAppContainer::testCheatCode(int code) {
	switch (code) {
		case 123434: {
			this->app->menuSystem->scrollIndex = 0;
			this->app->menuSystem->selectedIndex = 0;
			this->app->menuSystem->gotoMenu(Menus::MENU_DEBUG);
			break;
		}
		case 123414: {
			this->app->canvas->loadMap(this->app->canvas->loadMapID, true, false);
			break;
		}
		case 123424: {
			this->app->player->giveAll();
			break;
		}
		case 123432: {
			if (this->app->menuSystem->menu >= Menus::MENU_INGAME) {
				this->app->canvas->startSpeedTest(0);
			}
			break;
		}
		default: {
			return false;
		}
	}
	return true;
}

void CAppContainer::UpdateAccelerometer(float x, float y, float z, bool useMouse) {
	if (this->app) {
		if (useMouse) {
			int aX = (int)(x * Applet::IOS_WIDTH);
			int aY = (int)(y * Applet::IOS_HEIGHT);
			float axisX = -AxisHit(aX, aY, 0, 0, 480, 320, true, 1.0f);
			float axisY = -AxisHit(aX, aY, 0, 0, 480, 320, false, 1.0f);
			this->app->AccelerometerUpdated(axisX, axisY, z);
		}
		else {
			this->app->AccelerometerUpdated(x, y, z);
		}
	}
}

void CAppContainer::Construct(SDLGL* sdlGL, ZipFile* zipFile) {
	printf("CAppContainer::Construct\n");
	this->sdlGL = sdlGL; // New
	this->zipFile = zipFile; // New

	this->app = new Applet();
	this->app->startup();
	this->app->game->hasSeenIntro = true;
}
