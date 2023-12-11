#include <stdexcept>
#include <assert.h>

#include "SDLGL.h"
#include "App.h"
#include "Image.h"
#include "CAppContainer.h"
#include "Canvas.h"
#include "Graphics.h"
#include "MayaCamera.h"
#include "Game.h"
#include "GLES.h"
#include "TinyGL.h"
#include "Hud.h"
#include "Render.h"
#include "Combat.h"
#include "Player.h"
#include "MenuSystem.h"
#include "HackingGame.h"
#include "SentryBotGame.h"
#include "VendingMachine.h"
#include "ParticleSystem.h"
#include "Text.h"
#include "Button.h"
#include "Sound.h"
#include "Resource.h"
#include "Enums.h"
#include "Utils.h"
#include "Menus.h"
#include "Input.h"

Canvas::Canvas() {
	std::memset(this, 0, sizeof(Canvas));
}

Canvas::~Canvas() {
}

bool Canvas::isLoaded;

bool Canvas::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	int viewWidth, viewHeight;
	fmButton* button;

	printf("Canvas::startup\n");

	this->displayRect[0] = 0;
	this->displayRect[1] = 0;
	this->displayRect[2] = app->backBuffer->width;
	this->displayRect[3] = app->backBuffer->height;

	//printf("this->displayRect[0] %d\n", this->displayRect[0]);
	//printf("this->displayRect[1] %d\n", this->displayRect[1]);
	//printf("this->displayRect[2] %d\n", this->displayRect[2]);
	//printf("this->displayRect[3] %d\n", this->displayRect[3]);

	this->graphics.setGraphics();
	this->specialLootIcon = -1;
	this->pacLogoTime = -1;
	this->vibrateEnabled = true;
	this->loadMapStringID = -1;
	this->startupMap = 1;
	this->loadType = 0;
	this->saveType = 0;
	this->st_count = 0;
	this->knockbackDist = 0;
	this->numHelpMessages = 0;
	this->destZ = 36;
	this->viewZ = 36;
	this->screenRect[2] = 0;
	this->screenRect[3] = 0;
	this->dialogThread = nullptr;
	this->ignoreFrameInput = false;
	this->blockInputTime = 0;
	this->showLocation = false;
	this->lastPacifierUpdate = 0;
	this->numEvents = 0;
	this->dialogItem = nullptr;
	this->dialogViewLines = 0;
	this->lastMapID = 0;
	this->loadMapID = 0;
	this->automapDrawn = false;

	this->displayRect[2] &= 0xFFFFFFFE;
	this->screenRect[2] = this->displayRect[2];
	this->screenRect[3] = this->displayRect[3];

	this->dialogMaxChars = (this->displayRect[2] - 2) / 9;
	this->scrollMaxChars = (this->displayRect[2] - 2) / 9;
	this->dialogWithBarMaxChars		= (this->displayRect[2] - 9) / 9;
	this->scrollWithBarMaxChars		= (this->displayRect[2] - 9) / 9;
	this->menuScrollWithBarMaxChars = (this->displayRect[2] - 9) / 9;
	this->ingameScrollWithBarMaxChars = (this->displayRect[2] - 34) / 9;
	this->menuHelpMaxChars = (this->displayRect[2] - 32) / 9;
	this->subtitleMaxChars = this->displayRect[2] / 9;

	if (app->hud->startup()) {

		int n2 = this->screenRect[3] - 35 - 35;
		if (this->displayRect[3] >= 128) {
			this->displaySoftKeys = true;
			this->softKeyY = this->displayRect[3] - 0;
			if (this->displayRect[3] - this->screenRect[3] < 0) {
				n2 -= 0 - (this->displayRect[3] - this->screenRect[3]);
			}
		}
		else {
			this->softKeyY = this->displayRect[3];
		}

		int n3 = n2 & 0xFFFFFFFE;
		this->screenRect[3] = n3 + 35 + 35;
		this->screenRect[0] = (this->displayRect[2] - this->screenRect[2]) / 2;
		if (this->displaySoftKeys) {
			this->screenRect[1] = (this->softKeyY - this->screenRect[3]) / 2;
		}
		else {
			this->screenRect[1] = (this->displayRect[3] - this->screenRect[3]) / 2;
		}

		this->SCR_CX = this->screenRect[2] / 2;
		this->SCR_CY = this->screenRect[3] / 2;
		this->viewRect[0] = this->screenRect[0];
		this->viewRect[1] = 20;//this->screenRect[1] + 35;
		this->viewRect[2] = this->screenRect[2];
		this->viewRect[3] = n3;

		// custom
		viewWidth = 0;
		viewHeight = 0;
		if (viewWidth != 0 && viewHeight != 0) {
			this->viewRect[0] += ((this->viewRect[2] - viewWidth) >> 1);
			this->viewRect[1] += ((this->viewRect[3] - viewHeight) >> 1);
			this->viewRect[2] = viewWidth;
			this->viewRect[3] = viewHeight;
		}

		this->hudRect[0] = this->displayRect[0];
		this->hudRect[1] = this->screenRect[1];
		this->hudRect[2] = this->displayRect[2];
		this->hudRect[3] = this->screenRect[3];

		this->menuRect[0] = this->displayRect[0];
		this->menuRect[1] = this->displayRect[1];
		this->menuRect[2] = this->displayRect[2];
		this->menuRect[3] = this->screenRect[3];

		this->CAMERAVIEW_BAR_HEIGHT = 20;
		
		this->cinRect[0] = this->viewRect[0];
		this->cinRect[1] = 42;
		this->cinRect[2] = this->viewRect[2];
		this->cinRect[3] = this->viewRect[3];

		if (this->screenRect[1] + this->screenRect[3] == this->softKeyY + -1) {
			this->softKeyY = this->screenRect[1] + this->screenRect[3];
		}

		this->setAnimFrames(10);

		this->startupMap = 1;
		this->skipIntro = false;
		this->tellAFriend = false;

		app->beginImageLoading();
		this->imgDialogScroll = app->loadImage("DialogScroll.bmp", true);
		this->imgFabricBG = app->loadImage("FabricBG.bmp", true);
		this->imgFont = app->loadImage("Font.bmp", true);
		this->imgEndOfLevelStatsBG = app->loadImage("endOfLevelStatsBG.bmp", true);
		this->imgGameHelpBG = app->loadImage("gameHelpBG.bmp", true);
		this->imgIcons_Buffs = app->loadImage("Icons_Buffs.bmp", true);
		this->imgInventoryBG = app->loadImage("inventoryBG.bmp", true);
		this->imgLoadingFire = app->loadImage("loadingFire.bmp", true);
		this->imgFont_16p_Light = app->loadImage("Font_16p_Light.bmp", true);
		this->imgFont_16p_Dark = app->loadImage("Font_16p_Dark.bmp", true);
		this->imgFont_18p_Light = app->loadImage("Font_18p_Light.bmp", true);
		this->imgWarFont = app->loadImage("WarFont.bmp", true);
		this->fontRenderMode = 0;
		app->endImageLoading();

		this->lootSource = -1;
		this->m_controlLayout = 2;
		this->isFlipControls = false;
		this->m_controlMode = 1;
		this->lastBacklightRefresh = 0;
		this->vibrateTime = 0;
		this->areSoundsAllowed = false;
		this->m_controlAlpha = 50;
		this->m_controlGraphic = 0;

		char* arrowsFiles[] = {
			"arrow-up.bmp",
			"greenArrow_up.bmp",
			"arrow-up_pressed.bmp",
			"greenArrow_up-pressed.bmp",
			"arrow-down.bmp",
			"greenArrow_down.bmp",
			"arrow-down_pressed.bmp",
			"greenArrow_down-pressed.bmp",
			"arrow-left.bmp",
			"greenArrow_left.bmp",
			"arrow-left_pressed.bmp",
			"greenArrow_left-pressed.bmp",
			"arrow-right.bmp",
			"greenArrow_right.bmp",
			"arrow-right_pressed.bmp",
			"greenArrow_right-pressed.bmp"
		};

		char** files = arrowsFiles;
		Image **imgArrows = this->imgArrows;
		for (int i = 0; i < 2; i++) {
			imgArrows[0] = app->loadImage(files[0], true);
			imgArrows[2] = app->loadImage(files[2], true);
			imgArrows[4] = app->loadImage(files[4], true);
			imgArrows[6] = app->loadImage(files[6], true);
			imgArrows[8] = app->loadImage(files[8], true);
			imgArrows[10] = app->loadImage(files[10], true);
			imgArrows[12] = app->loadImage(files[12], true);
			imgArrows[14] = app->loadImage(files[14], true);
			imgArrows++;
			files++;
		}

		this->imgDpad_default = app->loadImage("dpad_default.bmp", true);
		this->imgDpad_up_press = app->loadImage("dpad_up-press.bmp", true);
		this->imgDpad_down_press = app->loadImage("dpad_down-press.bmp", true);
		this->imgDpad_left_press = app->loadImage("dpad_left-press.bmp", true);
		this->imgDpad_right_press = app->loadImage("dpad_right-press.bmp", true);
		this->imgPageUP_Icon = app->loadImage("pageUP_Icon.bmp", true);
		this->imgPageDOWN_Icon = app->loadImage("pageDOWN_Icon.bmp", true);
		this->imgPageOK_Icon = app->loadImage("pageOK_Icon.bmp", true);
		this->imgSniperScope_Dial = app->loadImage("SniperScope_Dial.bmp", true);
		this->imgSniperScope_Knob = app->loadImage("SniperScope_Knob.bmp", true);

		this->touched = false;

		// Setup Sniper Scope Dial Scroll Button
		{
			this->m_sniperScopeDialScrollButton = new fmScrollButton(400, 67, this->imgSniperScope_Dial->width, this->imgSniperScope_Dial->height, true, 1113);
			this->m_sniperScopeDialScrollButton->SetScrollBox(0, 0, 1, 1, 16);
			this->m_sniperScopeDialScrollButton->field_0x0_ = 1;
		}

		// Setup Sniper Scope Buttons
		{
			this->m_sniperScopeButtons = new fmButtonContainer();
			button = new fmButton(6, 122, 20, 236, 236, -1);
			this->m_sniperScopeButtons->AddButton(button);
		}

		// Setup Control Buttons
		{
			fmButtonContainer** m_controlButtons = this->m_controlButtons;
			for (int i = 0; i < 2; i++) {
				m_controlButtons[0] = new fmButtonContainer();
				m_controlButtons[2] = new fmButtonContainer();
				m_controlButtons[4] = new fmButtonContainer();

				if (i == 1) {
					int v53 = 5;
					int v49 = 117;
					while (1)
					{
						button = new fmButton(5, v53, v53 + 116, 13, v49, -1);
						//button->drawTouchArea = true; // Test
						m_controlButtons[2]->AddButton(button);
						button = new fmButton(7, 140 - v53, v53 + 116, 13, v49, -1);
						//button->drawTouchArea = true; // Test
						m_controlButtons[2]->AddButton(button);
						button = new fmButton(3, v53 + 13, v53 + 103, v49, 13, -1);
						//button->drawTouchArea = true; // Test
						m_controlButtons[0]->AddButton(button);
						button = new fmButton(9, v53 + 13, 243 - v53, v49, 13, -1);
						//button->drawTouchArea = true; // Test
						m_controlButtons[0]->AddButton(button);
						v49 -= 26;
						if (v53 == 44)
							break;
						v53 += 13;
					}
					button = new fmButton(6, 153, 25, 322, 226, -1);
					m_controlButtons[2]->AddButton(button);
					this->m_swipeArea[1] = new fmSwipeArea(0, 0, this->screenRect[2], this->screenRect[3] - 64, 50, 50);
				}
				else {
					button = new fmButton(3, 42, 31, 70, 70, -1);
					button->SetImage(&this->imgArrows[0], 0, true);
					button->SetHighlightImage(&this->imgArrows[2], 0, true);
					button->normalRenderMode = 13;
					button->highlightRenderMode = 13;
					m_controlButtons[0]->AddButton(button);
					button = new fmButton(9, 42, 181, 70, 70, -1);
					button->SetImage(&this->imgArrows[4], 0, true);
					button->SetHighlightImage(&this->imgArrows[6], 0, true);
					button->normalRenderMode = 13;
					button->highlightRenderMode = 13;
					m_controlButtons[0]->AddButton(button);
					button = new fmButton(5, 5, 106, 70, 70, -1);
					button->SetImage(&this->imgArrows[8], 0, true);
					button->SetHighlightImage(&this->imgArrows[10], 0, true);
					button->normalRenderMode = 13;
					button->highlightRenderMode = 13;
					m_controlButtons[2]->AddButton(button);
					button = new fmButton(7, 80, 106, 70, 70, -1);
					button->SetImage(&this->imgArrows[12], 0, true);
					button->SetHighlightImage(&this->imgArrows[14], 0, true);
					button->normalRenderMode = 13;
					button->highlightRenderMode = 13;
					m_controlButtons[2]->AddButton(button);
					button = new fmButton(6, 155, 25, 320, 226, -1);
					m_controlButtons[2]->AddButton(button);
					button = new fmButton(6, 475, 172, 0, 79, -1);
					m_controlButtons[2]->AddButton(button);
					this->m_swipeArea[0] = new fmSwipeArea(0, 0, this->screenRect[2], this->screenRect[3] - 64, 50, 50);
				}

				m_controlButtons++;
			}
		}

		// Setup Character Buttons
		{
			this->m_characterButtons = new fmButtonContainer();
			for (int i = 0; i < 5; i++) {
				button = new fmButton(i, 0, 0, 0, 0, 1027);
				this->m_characterButtons->AddButton(button);
			}
		}

		// Setup Dialog Buttons
		{
			this->m_dialogButtons = new fmButtonContainer();
			for (int i = 0; i < 5; i++) {
				button = new fmButton(i, 0, 0, 0, 0, 1027);
				this->m_dialogButtons->AddButton(button);
			}
			button = new fmButton(5, 390, 20, 90, 90, 1027);
			button->SetImage(this->imgPageUP_Icon, true);
			button->SetHighlightImage(this->imgPageUP_Icon, true);
			button->normalRenderMode = 12;
			button->highlightRenderMode = 0;
			this->m_dialogButtons->AddButton(button);
			button = new fmButton(6, 390, 110, 90, 90, 1027);
			button->SetImage(this->imgPageDOWN_Icon, true);
			button->SetHighlightImage(this->imgPageDOWN_Icon, true);
			button->normalRenderMode = 12;
			button->highlightRenderMode = 0;
			this->m_dialogButtons->AddButton(button);
			button = new fmButton(7, 390, 110, 90, 90, 1027);
			button->SetImage(this->imgPageOK_Icon, true);
			button->SetHighlightImage(this->imgPageOK_Icon, true);
			button->normalRenderMode = 12;
			button->highlightRenderMode = 0;
			this->m_dialogButtons->AddButton(button);
			button = new fmButton(8, 0, 0, 0, 0, 1027);
			this->m_dialogButtons->AddButton(button);
		}

		// Setup SoftKey Buttons
		{
			this->m_softKeyButtons = new fmButtonContainer();
			button = new fmButton(19, 0, 250, 100, 70, 1027);
			this->m_softKeyButtons->AddButton(button);
			button = new fmButton(20, 380, 250, 100, 70, 1027);
			this->m_softKeyButtons->AddButton(button);
		}

		// Setup Mixing Buttons
		{
			this->m_mixingButtons = new fmButtonContainer();
		}

		// Setup Story Buttons
		{
			this->m_storyButtons = new fmButtonContainer();
			button = new fmButton(0, 0, 280, 60, 40, 1027); // Old -> (0, 0, 250, 100, 70, 1027);
			this->m_storyButtons->AddButton(button);
			button = new fmButton(1, 380, 280, 100, 40, 1027); // Old -> (1, 320, 250, 100, 70, 1027);
			this->m_storyButtons->AddButton(button);
			button = new fmButton(2, 420, 0, 60, 40, 1027);// Old -> (2, 380, 0, 100, 70, 1027);
			this->m_storyButtons->AddButton(button);
		}

		// Setup TreadMill Buttons
		{
			this->imgBootL = app->loadImage("bootL.bmp", true);
			this->imgBootR = app->loadImage("bootR.bmp", true);
			this->m_treadmillButtons = new fmButtonContainer();
			int w = imgBootL->width;
			int h = imgBootL->height;
			int x = 240 - (2 * w);
			int y = 160 - (h / 2);
			button = new fmButton(0, x, y, w, h, 1027);
			button->SetImage(this->imgBootL, true);
			button->SetHighlightImage(this->imgBootL, true);
			button->normalRenderMode = 12;
			button->highlightRenderMode = 0;
			this->m_treadmillButtons->AddButton(button);
			button = new fmButton(1, x + 3 * w, y, w, h, 1027);
			button->SetImage(this->imgBootR, true);
			button->SetHighlightImage(this->imgBootR, true);
			button->normalRenderMode = 12;
			button->highlightRenderMode = 0;
			this->m_treadmillButtons->AddButton(button);
		}

		return true;
	}

	return false;
}

void Canvas::flushGraphics() {
	this->graphics.resetScreenSpace();
	this->backPaint(&this->graphics);
}

void Canvas::backPaint(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	graphics->clearClipRect();

	if (this->repaintFlags & Canvas::REPAINT_CLEAR) {
		this->repaintFlags &= ~Canvas::REPAINT_CLEAR;
		graphics->eraseRgn(this->displayRect);
	}

	if (this->repaintFlags & Canvas::REPAINT_VIEW3D) {
		if (app->render->_gles->isInit) {
			this->repaintFlags &= ~Canvas::REPAINT_VIEW3D;
			if (app->render->isFading()) {
				app->render->fadeScene(graphics);
			}
		}
		else {
			//this->repaintFlags &= ~Canvas::REPAINT_VIEW3D;
			//app->render->Render3dScene();
			app->render->drawRGB(graphics);
		}
	}

	if (this->repaintFlags & Canvas::REPAINT_PARTICLES) {
		this->repaintFlags &= ~Canvas::REPAINT_PARTICLES;
		app->particleSystem->renderSystems(graphics);
	}

	if (this->state == Canvas::ST_COMBAT || this->state == Canvas::ST_PLAYING) {
		this->m_swipeArea[this->m_controlMode]->Render(graphics);
	}

	if (((this->repaintFlags & Canvas::REPAINT_HUD) != 0) && (this->state != Canvas::ST_MENU)) { // REPAINT_HUD
		//this->repaintFlags &= ~Canvas::REPAINT_HUD; java, brew only
		app->hud->draw(graphics);
	}

	if (app->player->inTargetPractice) {
		this->drawTargetPracticeScore(graphics);
	}

	if (this->state == Canvas::ST_INTRO_MOVIE) {
		this->playIntroMovie(graphics);
	}
	else if (this->state == Canvas::ST_CHARACTER_SELECTION) {
		this->drawCharacterSelection(graphics);
	}
	else if (this->state == Canvas::ST_INTRO) {
		this->drawStory(graphics);
	}
	else if (this->state == Canvas::ST_EPILOGUE) {
		this->drawScrollingText(graphics);
	}
	else if (this->state == Canvas::ST_CREDITS) {
		this->drawCredits(graphics);
	}
	else if (this->state == Canvas::ST_TRAVELMAP) {
		this->drawTravelMap(graphics);
	}
	else if (this->state == Canvas::ST_AUTOMAP) {
		this->drawAutomap(graphics, !this->automapDrawn);
		this->m_softKeyButtons->Render(graphics);
		app->hud->drawArrowControls(graphics);
		this->automapDrawn = true;
	}
	else if (this->state == Canvas::ST_DIALOG) {
		this->dialogState(graphics);
	}
	else if (this->state == Canvas::ST_MINI_GAME) {
		switch (this->stateVars[0]) {
			case 2: {
				this->repaintFlags &= ~Canvas::REPAINT_HUD;
				app->hackingGame->updateGame(graphics);
				break;
			}
			case 0: {
				app->sentryBotGame->updateGame(graphics);
				break;
			}
			case 4: {
				app->vendingMachine->updateGame(graphics);
				break;
			}
		}
	}
	else if (this->state == Canvas::ST_ERROR) {
		//this->errorState(graphics);
	}
	else if (this->state == Canvas::ST_LOOTING) {
		this->drawLootingMenu(graphics);
	}
	else if (this->state == Canvas::ST_TREADMILL) {
		this->drawTreadmillReadout(graphics);
	}

	/*if ((this->repaintFlags & Canvas::REPAINT_SOFTKEYS) != 0x0) { // REPAINT_SOFTKEYS
		this->repaintFlags &= ~Canvas::REPAINT_SOFTKEYS;
		this->drawSoftKeys(graphics);
	}*/

	if (this->repaintFlags & Canvas::REPAINT_MENU) {
		this->repaintFlags &= ~Canvas::REPAINT_MENU;
		app->menuSystem->paint(graphics);
	}

	if (this->repaintFlags & Canvas::REPAINT_STARTUP_LOGO) {
		this->repaintFlags &= ~Canvas::REPAINT_STARTUP_LOGO;
		graphics->fillRect(0, 0, this->displayRect[2], this->displayRect[3], -0x1000000);
		graphics->drawImage(this->imgStartupLogo, this->displayRect[2] / 2, this->displayRect[3] / 2, 3, 0, 0);
	}

	//printf("this->repaintFlags %d\n", this->repaintFlags);
	if (this->repaintFlags & Canvas::REPAINT_LOADING_BAR) { // REPAINT_LOADING_BAR
		this->repaintFlags &= ~Canvas::REPAINT_LOADING_BAR;
		this->drawLoadingBar(graphics);
	}

	if (this->fadeFlags && (app->time < this->fadeTime + this->fadeDuration)) {
		int alpha = ((app->time - this->fadeTime) << 8) / this->fadeDuration;

		if ((this->fadeFlags & Canvas::FADE_FLAG_FADEOUT) != 0) {
			alpha = 256 - alpha;
		}

		graphics->fade(this->fadeRect, alpha, this->fadeColor);
	}
	else {
		this->fadeFlags = Canvas::FADE_FLAG_NONE;
	}

	if (this->state == Canvas::ST_BENCHMARK) {
		if (this->st_enabled) {
			int n = this->viewRect[1];
			this->debugTime = app->upTimeMs;
			Text* largeBuffer = app->localization->getLargeBuffer();
			largeBuffer->setLength(0);
			largeBuffer->append("Rndr ms: ");
			largeBuffer->append(this->st_fields[0] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[0] * 100 / this->st_count - this->st_fields[0] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("Bsp ms: ");
			largeBuffer->append(this->st_fields[1] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[1] * 100 / this->st_count - this->st_fields[1] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("Hud ms: ");
			largeBuffer->append(this->st_fields[2] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[2] * 100 / this->st_count - this->st_fields[2] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			int n2 = this->st_fields[4] + this->st_fields[5];
			largeBuffer->setLength(0);
			largeBuffer->append("Blit ms: ");
			largeBuffer->append(n2 / this->st_count)->append('.');
			largeBuffer->append(n2 * 100 / this->st_count - n2 / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("Paus ms: ");
			largeBuffer->append(this->st_fields[6] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[6] * 100 / this->st_count - this->st_fields[6] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("Dbg ms: ");
			largeBuffer->append(this->st_fields[9] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[9] * 100 / this->st_count - this->st_fields[9] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("Loop ms: ");
			largeBuffer->append(this->st_fields[7] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[7] * 100 / this->st_count - this->st_fields[7] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("Key ms: ");
			largeBuffer->append(this->st_fields[11] - this->st_fields[10]);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("State ms: ");
			largeBuffer->append(this->st_fields[12] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[12] * 100 / this->st_count - this->st_fields[12] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append("Totl ms: ");
			largeBuffer->append(this->st_fields[8] / this->st_count)->append('.');
			largeBuffer->append(this->st_fields[8] * 100 / this->st_count - this->st_fields[8] / this->st_count * 100);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			largeBuffer->setLength(0);
			largeBuffer->append(this->st_count);
			graphics->drawString(largeBuffer, this->viewRect[0], n, 0);
			n += 16;
			this->debugTime = app->upTimeMs - this->debugTime;
			largeBuffer->dispose();
		}
	}
	else if (this->state == Canvas::ST_CAMERA || this->state == Canvas::ST_PLAYING || this->state == Canvas::ST_COMBAT || this->state == Canvas::ST_INTER_CAMERA) {
		int n3 = this->viewRect[1];
		if (this->showSpeeds) {
			int lastRenderTime = this->afterRender - this->beforeRender;
			if (this->lastFrameTime == app->time) {
				this->afterRender = (this->beforeRender = 0);
				this->lastRenderTime = lastRenderTime;
			}
			int n4 = app->time - this->totalFrameTime;
			this->totalFrameTime = app->time;
			Text* largeBuffer2 = app->localization->getLargeBuffer();
			largeBuffer2->setLength(0);
			largeBuffer2->append("ms: ");
			largeBuffer2->append(this->lastRenderTime)->append('/');
			largeBuffer2->append(app->render->clearColorBuffer)->append('/');
			largeBuffer2->append(app->render->bltTime)->append('/');
			largeBuffer2->append(n4);
			graphics->drawString(largeBuffer2, this->viewRect[0], n3, 0);
			n3 += 16;
			largeBuffer2->setLength(0);
			largeBuffer2->append("li: ");
			largeBuffer2->append(app->render->lineRasterCount)->append('/');
			largeBuffer2->append(app->render->lineCount);
			graphics->drawString(largeBuffer2, this->viewRect[0], n3, 0);
			n3 += Applet::FONT_HEIGHT[app->fontType];
			largeBuffer2->setLength(0);
			largeBuffer2->append("sp: ");
			largeBuffer2->append(app->render->spriteRasterCount)->append('/');
			largeBuffer2->append(app->render->spriteCount)->append('/');
			largeBuffer2->append(app->render->numMapSprites);
			graphics->drawString(largeBuffer2, this->viewRect[0], n3, 0);
			n3 += Applet::FONT_HEIGHT[app->fontType];
			if (app->render->renderMode == 63) {
				largeBuffer2->setLength(0);
				largeBuffer2->append("cnt: ");
				largeBuffer2->append(app->tinyGL->spanCalls)->append('/');
				largeBuffer2->append(app->tinyGL->spanPixels);
				graphics->drawString(largeBuffer2, this->viewRect[0], n3, 0);
				n3 += Applet::FONT_HEIGHT[app->fontType];
				largeBuffer2->setLength(0);
				largeBuffer2->append("tris: ");
				largeBuffer2->append(app->tinyGL->countBackFace)->append('/');
				largeBuffer2->append(app->tinyGL->countDrawn);
				graphics->drawString(largeBuffer2, this->viewRect[0], n3, 0);
				n3 += Applet::FONT_HEIGHT[app->fontType];
			}
			largeBuffer2->setLength(0);
			largeBuffer2->append("OSTime: ");
			int v30 = 0;
			for (int i = 0; i < 8; i++) {
				v30 += app->osTime[i];
			}
			largeBuffer2->append(v30 / 8);
			graphics->drawString(largeBuffer2, this->viewRect[0], n3, 0);
			n3 += Applet::FONT_HEIGHT[app->fontType];
			largeBuffer2->setLength(0);
			largeBuffer2->append("Code: ");
			int v31 = 0;
			for (int i = 0; i < 8; i++) {
				v31 += app->codeTime[i];
			}
			largeBuffer2->append(v31 / 8);
			graphics->drawString(largeBuffer2, this->viewRect[0], n3, 0);
			largeBuffer2->dispose();
			n3 += Applet::FONT_HEIGHT[app->fontType];
		}

		if (this->showLocation) {
			Text* smallBuffer = app->localization->getSmallBuffer();
			smallBuffer->setLength(0);
			smallBuffer->append(this->viewX >> 6);
			smallBuffer->append(' ');
			smallBuffer->append(this->viewY >> 6);
			smallBuffer->append(' ');
			int angle = this->viewAngle & 0x3FF;
			if (angle == Enums::ANGLE_NORTH) {
				smallBuffer->append('N');
			}
			else if (angle == Enums::ANGLE_EAST) {
				smallBuffer->append('E');
			}
			else if (angle == Enums::ANGLE_SOUTH) {
				smallBuffer->append('S');
			}
			else if (angle == Enums::ANGLE_WEST) {
				smallBuffer->append('W');
			}
			else if (angle == Enums::ANGLE_NORTHEAST) {
				smallBuffer->append("NE");
			}
			else if (angle == Enums::ANGLE_NORTHWEST) {
				smallBuffer->append("NW");
			}
			else if (angle == Enums::ANGLE_SOUTHEAST) {
				smallBuffer->append("SE");
			}
			else if (angle == Enums::ANGLE_SOUTHWEST) {
				smallBuffer->append("SW");
			}
			graphics->drawString(smallBuffer, this->viewRect[0], n3, 0);
			smallBuffer->dispose();
		}
	}

	graphics->resetScreenSpace();
	if (this->showFreeHeap) {
		graphics->setColor(0xFF000000);
		graphics->fillRect(this->viewRect[0], this->viewRect[1] + this->viewRect[3] - 18, this->viewRect[2], 16);
		switch (this->updateChar) {
		case '*': {
			this->updateChar = '+';
			break;
		}
		case '+': {
			this->updateChar = '%';
			break;
		}
		case '%': {
			this->updateChar = '#';
			break;
		}
		case '#': {
			this->updateChar = '*';
			break;
		}
		}
		Text *largeBuffer3 = app->localization->getLargeBuffer();
		if (largeBuffer3 != nullptr) {
			largeBuffer3->append(this->updateChar);
			largeBuffer3->append(" ");
			largeBuffer3->append(1000000000/*App.getFreeMemory()*/);
			graphics->drawString(largeBuffer3, this->SCR_CX, this->viewRect[1] + this->viewRect[3] - 16, 17);
			largeBuffer3->setLength(0);
			for (int i = 0; i < 10; ++i) {
				largeBuffer3->append(app->game->numLevelLoads[i]);
				largeBuffer3->append('|');
			}
			graphics->drawString(largeBuffer3, this->viewRect[0], this->viewRect[1], 4);
			largeBuffer3->setLength(0);
			int t = app->game->totalPlayTime + (app->upTimeMs - app->game->lastSaveTime) / 1000; //[GEC] hace que el tiempo avance
			int n6 = t / 3600;
			int n7 = (t - n6 * 3600) / 60;
			int n8 = (t - n6 * 3600) - n7 * 60;
			largeBuffer3->append(n6);
			largeBuffer3->append(':');
			if (n7 < 10) {
				largeBuffer3->append(0);
			}
			largeBuffer3->append(n7);
			largeBuffer3->append(':');
			if (n8 < 10) {
				largeBuffer3->append(0);
			}
			largeBuffer3->append(n8);
			graphics->drawString(largeBuffer3, this->viewRect[0] + this->viewRect[2] - 3, this->viewRect[1], 8);
			largeBuffer3->dispose();
		}
	}
}

void Canvas::run() {
	Applet* app = CAppContainer::getInstance()->app;
	app->CalcAccelerometerAngles();

	int upTimeMs = app->upTimeMs;
	app->lastTime = app->time;
	app->time = upTimeMs;

	if (this->st_enabled != false) {
		this->st_count = this->st_count + 1;
		this->st_fields[0] = this->st_fields[0] + app->render->frameTime;
		this->st_fields[1] = this->st_fields[1] + app->render->bspTime;
		this->st_fields[2] = this->st_fields[2] + app->hud->drawTime;
		this->st_fields[4] = this->st_fields[4] + app->render->bltTime;
		this->st_fields[5] = (this->pauseTime - this->flushTime) + this->st_fields[5];
		this->st_fields[6] = (this->loopEnd - this->pauseTime) + this->st_fields[6];
		this->st_fields[8] = (app->time - app->lastTime) + this->st_fields[8];
		this->st_fields[3] = this->st_fields[3] + app->combat->renderTime;
		this->st_fields[9] = this->st_fields[9] + this->debugTime;
		this->st_fields[7] = (this->loopEnd - this->loopStart) + this->st_fields[7];
		upTimeMs = app->upTimeMs;
	}
	this->loopStart = upTimeMs;

	if (!app->game->pauseGameTime && this->state != Canvas::ST_MENU) {
		app->gameTime += app->time - app->lastTime;
	}

	if (this->vibrateTime && this->vibrateTime < app->time) {
		this->vibrateTime = 0;
	}

	if (this->state != Canvas::ST_DIALOG && this->state != Canvas::ST_MENU && app->game->isInputBlockedByScript()) {
		this->clearEvents(1);
	}

	this->runInputEvents();

	// [GEC]
	if (this->repaintFlags & Canvas::REPAINT_VIEW3D) { // REPAINT_VIEW3D
		if (!app->render->_gles->isInit) {
			app->render->Render3dScene();
			app->tinyGL->applyClearColorBuffer();
		}
	}

	if ((this->state != Canvas::ST_MENU) || (app->menuSystem->menu != Menus::MENU_ENABLE_SOUNDS)) {
		app->game->numTraceEntities = 0;
		app->game->UpdatePlayerVars();
		app->game->gsprite_update(app->time);
		app->game->runScriptThreads(app->gameTime);
	}

	//printf("lastTime %d\n", app->lastTime);
	//printf("time %d\n", app->time);
	//printf("this->loopStart %d\n", this->loopStart);
	int time = app->upTimeMs;
	app->game->updateAutomap = false;
	//printf("this->state %d\n", this->state);

	if (this->state == Canvas::ST_PLAYING) {

		if (this->m_controlButton) {
			this->handleEvent(this->m_controlButton->buttonID);
		}

		app->game->updateAutomap = true;
		if (this->numHelpMessages == 0 && app->game->queueAdvanceTurn) {
			app->game->snapMonsters(true);
			app->game->advanceTurn();
		}

		this->playingState();
		if (this->state == Canvas::ST_PLAYING) {
			this->repaintFlags |= Canvas::REPAINT_HUD;
			app->hud->repaintFlags |= 0x2f;
		}
	}
	else if (this->state == Canvas::ST_INTER_CAMERA) {
		this->repaintFlags |= Canvas::REPAINT_HUD;
		app->hud->repaintFlags |= 0x2B;
	}
	else if (this->state != Canvas::ST_CREDITS && this->state != Canvas::ST_TRAVELMAP && this->state != Canvas::ST_MIXING) {
		if (this->state != Canvas::ST_MINI_GAME) {
			if (this->state == Canvas::ST_COMBAT) {
				app->game->updateAutomap = true;
				this->combatState();
			}
			else if (this->state == Canvas::ST_INTRO_MOVIE) {
				if ((app->game->hasSeenIntro && this->numEvents != 0) || this->scrollingTextDone) {
					this->dialogBuffer->dispose();
					this->dialogBuffer = nullptr;
					app->game->hasSeenIntro = true;
					app->game->saveConfig();
					this->backToMain(false);
				}
			}
			else if (this->state == Canvas::ST_EPILOGUE) {
				if (this->scrollingTextDone) {
					this->disposeEpilogue();
				}
			}
			else if (this->state != Canvas::ST_CHARACTER_SELECTION) {
				if (this->state == Canvas::ST_INTRO) {
					if (this->storyPage >= this->storyTotalPages) {
						this->disposeIntro();
					}
				}
				else if (this->state == Canvas::ST_SAVING) {
					if ((this->saveType & 0x2) != 0x0 || (this->saveType & 0x1) != 0x0) {
						if ((this->saveType & 0x8) != 0x0) {
							if (app->game->spawnParam != 0) {
								int n11 = 32 + ((app->game->spawnParam & 0x1F) << 6);
								int n12 = 32 + ((app->game->spawnParam >> 5 & 0x1F) << 6);
								app->game->saveState(this->lastMapID, this->loadMapID, n11, n12, (app->game->spawnParam >> 10 & 0xFF) << 7, 0, n11, n12, n11, n12, 36, 0, 0, this->saveType);
							}
							else {
								app->game->saveState(this->lastMapID, this->loadMapID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, this->saveType);
							}
						}
						else if ((this->saveType & 0x10) != 0x0) {
							int n13 = 32 + ((app->game->spawnParam & 0x1F) << 6);
							int n14 = 32 + ((app->game->spawnParam >> 5 & 0x1F) << 6);
							app->game->saveState(this->loadMapID, app->menuSystem->LEVEL_STATS_nextMap, n13, n14, (app->game->spawnParam >> 10 & 0xFF) << 7, 0, n13, n14, n13, n14, 36, 0, 0, this->saveType);
						}
						else {
							app->game->saveState(this->loadMapID, this->loadMapID, this->destX, this->destY, this->destAngle, this->viewPitch, this->prevX, this->prevY, this->saveX, this->saveY, this->saveZ, this->saveAngle, this->savePitch, this->saveType);
						}
						app->hud->addMessage((short)0, (short)38);
					}
					else {
						app->Error(48); // ERR_SAVESTATE
					}

					if ((this->saveType & 0x4) != 0x0) {
						this->backToMain(false);
					}
					else if ((this->saveType & 0x40) != 0x0) {
						app->shutdown();
					}
					else if ((this->saveType & 0x8) != 0x0) {
						this->setState(Canvas::ST_TRAVELMAP);
					}
					else if ((this->saveType & 0x10) != 0x0) {
						app->sound->playSound(1069, 1u, 3, saveType & 8);
						app->menuSystem->setMenu(Menus::MENU_LEVEL_STATS);
					}
					else if ((this->saveType & 0x100) != 0x0) {
						app->menuSystem->setMenu(Menus::MENU_END_FINALQUIT);
					}
					else {
						if ((this->saveType & 0x80) != 0x0) {
							app->menuSystem->returnToGame();
						}
						this->setState(Canvas::ST_PLAYING);
					}
					this->saveType = 0;
					this->clearEvents(1);
				}
				else if (this->state == Canvas::ST_LOADING) {
					if (this->loadType == 0) {
						if (!this->loadMedia()) {
							this->flushGraphics();
							return;
						}
					}
					else {
						app->game->loadState(this->loadType);
						app->hud->addMessage((short)0, (short)39);
						this->loadType = 0;
					}
				}
				else if (this->state == Canvas::ST_MENU) {
					this->menuState();
				}
				else if (this->state == Canvas::ST_DIALOG) {
					app->game->updateLerpSprites();
					this->updateView();
					this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
					app->hud->repaintFlags |= 0x2B;
					//app->hud->repaintFlags &= 0xFFFFFFBF;
				}
				else if (this->state == Canvas::ST_AUTOMAP) {
					if (this->m_controlButton) {
						if (app->gameTime > this->m_controlButtonTime) {
							this->m_controlButtonTime = app->gameTime + 250;
							this->handleEvent(this->m_controlButton->buttonID);
						}
					}
					app->game->updateAutomap = true;
					this->automapState();
				}
				else if (this->state == Canvas::ST_DYING) {
					this->dyingState();
				}
				else if (this->state == Canvas::ST_LOOTING) {
					this->lootingState();
				}
				else if (this->state == Canvas::ST_TREADMILL) {
					this->treadmillState();
				}
				else if (this->state == Canvas::ST_BOT_DYING) {
					this->familiarDyingState();
				}
				else if (this->state == Canvas::ST_CAMERA) {
					if (app->game->activeCameraKey != -1) {
						app->game->activeCamera->Update(app->game->activeCameraKey, app->gameTime - app->game->activeCameraTime);
					}
					app->game->updateLerpSprites();
					this->updateView();
					if (this->state == Canvas::ST_CAMERA && app->gameTime > app->game->cinUnpauseTime && this->softKeyRightID == -1) {
						this->clearLeftSoftKey();
						this->setRightSoftKey((short)0, (short)40);
					}
				}
				else if (this->state == Canvas::ST_LOGO) {
					this->logoState();
				}
				else if (this->state == Canvas::ST_BENCHMARK) {
					this->renderOnlyState();
				}
				else {
					app->Error(51); // ERR_STATE
				}
			}
		}
	}

	if (this->state == Canvas::ST_SAVING || this->state == Canvas::ST_LOADING) {
		this->repaintFlags &= ~Canvas::REPAINT_VIEW3D;
	}
	this->st_fields[12] = app->upTimeMs - time;

	this->flushTime = app->upTimeMs;
	this->graphics.resetScreenSpace();
	this->backPaint(&this->graphics);
	if (this->keyPressedTime != 0) {
		this->lastKeyPressedTime = app->upTimeMs - this->keyPressedTime;
		this->keyPressedTime = 0;
	}
	this->pauseTime = app->upTimeMs;
	this->loopEnd = app->upTimeMs;
	this->stateChanged = false;

	if (this->sysSoundDelayTime > 0 && this->sysSoundDelayTime < app->upTimeMs - this->sysSoundTime) {
		app->sound->playSound((app->nextByte() & 0xF) + 1000, 0, 3, 0);
		this->sysSoundTime = app->upTimeMs;
	}
}

void Canvas::clearEvents(int ignoreFrameInput) {
	this->numEvents = 0;
	this->keyDown = false;
	this->keyDownCausedMove = false;
	this->ignoreFrameInput = ignoreFrameInput;
}

void Canvas::loadRuntimeData() {
	Applet* app = CAppContainer::getInstance()->app;

	app->loadRuntimeImages();
	//app->checkPeakMemory("after loadRuntimeData");
}

void Canvas::freeRuntimeData() {
	Applet* app = CAppContainer::getInstance()->app;
	app->freeRuntimeImages();
}

void Canvas::startShake(int i, int i2, int i3) {
	Applet* app = CAppContainer::getInstance()->app;
	SDLGL* sdlGL = CAppContainer::getInstance()->sdlGL;

	if (app->game->skippingCinematic) {
		return;
	}

	if (i2 != 0) {
		this->shakeTime = app->time + i;
		this->shakeIntensity = 2 * i2;
		this->staleTime += 1;
	}

	if (i3 != 0 && this->vibrateEnabled) {
		controllerVibrate(i3); // [GEC]
		this->vibrateTime = i3 + app->upTimeMs;
	}
}

void Canvas::setState(int state) {
	Applet* app = CAppContainer::getInstance()->app;

	this->stateChanged = true;
	for (int i = 0; i < 9; ++i) {
		this->stateVars[i] = 0;
	}
	this->m_controlButtonIsTouched = false;
	this->m_controlButton = 0;

	if (this->state == Canvas::ST_AUTOMAP) {
		app->player->unpause(app->time - this->automapTime);
	}
	else if (this->state == Canvas::ST_MENU) {
		app->player->unpause(app->time - app->menuSystem->startTime);
		app->menuSystem->clearStack();
	}
	else if (this->state == Canvas::ST_CAMERA) {
		app->render->disableRenderActivate = false;
		app->game->skippingCinematic = false;
	}
	else if (this->state == Canvas::ST_COMBAT && state != Canvas::ST_COMBAT && app->combat->stage != Canvas::ST_MENU) {
		app->combat->cleanUpAttack();
	}
	else if (this->state == Canvas::ST_CREDITS) {
		this->dialogBuffer->dispose();
		this->dialogBuffer = nullptr;
	}
	else if (this->state == Canvas::ST_INTER_CAMERA) {
	}


	this->oldState = this->state;
	this->state = state;

	//printf("state %d\n", state);
	if (state == Canvas::ST_COMBAT) {
		app->hud->repaintFlags = 47;
		this->repaintFlags |= Canvas::REPAINT_HUD;
		this->clearSoftKeys();
		this->combatDone = false;
	}
	else if (state == Canvas::ST_TRAVELMAP) {
		this->initTravelMap();
	}
	else if (state == Canvas::ST_PLAYING) {
		app->hud->repaintFlags = 0x2f;
		this->repaintFlags |= Canvas::REPAINT_HUD;
		app->game->lastTurnTime = app->time;
		if (app->game->monstersTurn == 0 || this->oldState == Canvas::ST_CAMERA) {
			this->drawPlayingSoftKeys();
		}
		if (this->oldState == Canvas::ST_COMBAT && app->combat->curTarget != nullptr && app->combat->curTarget->def->eType == 3) {
			app->game->executeStaticFunc(7);
		}
		this->updateFacingEntity = true;
		if (this->oldState != Canvas::ST_COMBAT && this->oldState != Canvas::ST_DIALOG) {
			this->invalidateRect();
		}
	}
	else if (state == Canvas::ST_INTER_CAMERA) {
		app->hud->repaintFlags = 43;
		this->repaintFlags |= Canvas::REPAINT_HUD;
	}
	else if (state == Canvas::ST_DIALOG) {
		if (app->canvas->isZoomedIn) {
			this->isZoomedIn = 0;
			app->StopAccelerometer();
			this->destAngle = this->viewAngle = (this->viewAngle + this->zoomAngle + 127) & 0xFFFFFF00;
			app->tinyGL->resetViewPort();
			this->drawPlayingSoftKeys();
		}
		if (app->game->isCameraActive()) {
			app->game->activeCameraTime = app->gameTime - app->game->activeCameraTime;
		}
		app->hud->repaintFlags = 47;
		this->repaintFlags |= Canvas::REPAINT_HUD;
		app->tinyGL->resetViewPort();
		//this->clearLeftSoftKey();
		/*this->setRightSoftKey((short)0, (short)40);
		int n2 = 0;
		if (this->dialogStyle == 2 || this->dialogStyle == 16 || this->dialogStyle == 9 || (this->dialogStyle == 4 && this->dialogItem != nullptr)) {
			n2 = 1;
		}
		if (this->numDialogLines - n2 > this->dialogViewLines) {
			this->setLeftSoftKey((short)0, (short)125);
		}
		else {
			//this->clearLeftSoftKey();
		}*/
		this->clearSoftKeys();
		this->clearEvents(1);
	}
	else if (state == Canvas::ST_DYING) {
		app->hud->repaintFlags = 47;
		this->repaintFlags |= Canvas::REPAINT_HUD;
		if (this->isZoomedIn) {
			this->isZoomedIn = false;
			app->StopAccelerometer();
			this->viewAngle += this->zoomAngle;
			int n3 = 255;
			this->destAngle = (this->viewAngle = (this->viewAngle + (n3 >> 1) & ~n3));
			app->tinyGL->resetViewPort();
			this->drawPlayingSoftKeys();
		}
		this->clearSoftKeys();
		this->deathTime = app->time;
		this->destPitch = 64;
		this->numHelpMessages = 0;
	}
	else if (state == Canvas::ST_BOT_DYING) {
		app->hud->repaintFlags = 47;
		this->repaintFlags |= Canvas::REPAINT_HUD;
		this->clearSoftKeys();
		this->familiarDeathTime = app->time;
		this->selfDestructScreenShakeStarted = false;
		app->hud->brightenScreen(100, 0);
		if (!this->familiarSelfDestructed) {
			app->hud->smackScreen(100);
		}
		this->destPitch = 64;
	}
	else if (state == Canvas::ST_LOOTING) {
		this->clearSoftKeys();
		this->lootingTime = app->time;
		this->crouchingForLoot = true;
		this->lootingCachedPitch = this->destPitch;
		this->field_0xac5_ = false;
	}
	else if (state == Canvas::ST_TREADMILL) {
		this->clearSoftKeys();
		//this->setRightSoftKey((short)0, (short)29); // J2ME
		app->combat->shiftWeapon(true);
		app->hud->repaintFlags |= 0x20;
		this->repaintFlags |= Canvas::REPAINT_HUD;
		this->treadmillNumSteps = 0;
		this->treadmillLastStep = 1;
		this->treadmillLastStepTime = app->time;
		this->treadmillReturnCode = 0;
	}
	else if (state == Canvas::ST_EPILOGUE) {
		app->player->levelGrade(true);
		bool soundEnabled = app->sound->allowSounds;
		app->sound->allowSounds = app->canvas->areSoundsAllowed;
		app->sound->allowSounds = soundEnabled;
		this->clearSoftKeys();
		this->loadEpilogueText();
		this->stateVars[0] = 1;
	}
	else if (state == Canvas::ST_CREDITS) {
		app->localization->loadText(2);
		this->initScrollingText(2, 0, false, 16, 5, 500);
		app->localization->unloadText(2);
	}
	else if (state == Canvas::ST_CHARACTER_SELECTION) {
		this->clearSoftKeys();
		this->setupCharacterSelection();
		this->stateVars[0] = 1;
		this->stateVars[1] = 0;
		this->stateVars[2] = 1;
		this->stateVars[8] = 0; // [GEC]
	}
	else if (state == Canvas::ST_INTRO) {
		this->clearSoftKeys();
		this->loadPrologueText();
		this->stateVars[0] = 1;
	}
	else if (state == Canvas::ST_INTRO_MOVIE) {
		this->initScrollingText((short)0, (short)133, 0, 32, 1, 800);
		this->stateVars[0] = 1;
	}
	else if (state == Canvas::ST_LOADING || state == Canvas::ST_SAVING) {
		this->repaintFlags &= ~Canvas::REPAINT_HUD;
		this->pacifierX = this->SCR_CX - 66;
		this->updateLoadingBar(false);
	}
	else if (state == Canvas::ST_AUTOMAP) {
		this->automapDrawn = false;
		this->automapTime = app->time;
	}
	else if (state == Canvas::ST_MENU) {
		app->menuSystem->startTime = app->time;
		/*if (Canvas.oldState == Canvas::ST_PLAYING) {
			Sound.playSound(3);
		}
		else if (Canvas.oldState == Canvas::ST_MIXING) {
			MenuSystem.goBackToStation = true;
		}*/

		if (this->oldState != Canvas::ST_MENU) {
			this->clearEvents(1);
		}
		app->beginImageLoading();
		app->endImageLoading();
	}
	else if (state == Canvas::ST_CAMERA) {
		app->hud->msgCount = 0;
		app->hud->subTitleID = -1;
		app->hud->cinTitleID = -1;
		app->render->disableRenderActivate = true;
		this->repaintFlags |= Canvas::REPAINT_HUD; // j2me 0x11;
		app->hud->repaintFlags = 24;
		this->clearSoftKeys();
		app->tinyGL->setViewport(this->cinRect[0], this->cinRect[1], this->cinRect[2], this->cinRect[3]);
	}
}

void Canvas::setAnimFrames(int animFrames) {
	this->animFrames = animFrames;
	this->animPos = (64 + this->animFrames - 1) / this->animFrames;
	this->animAngle = (256 + this->animFrames - 1) / this->animFrames;
}

void Canvas::checkFacingEntity() {
	Applet* app = CAppContainer::getInstance()->app;
	if (!this->updateFacingEntity) {
		return;
	}
	int destX = this->destX;
	int destY = this->destY;
	int destZ = this->destZ;
	int n = 21741;
	int *view = app->tinyGL->view;
	app->game->trace(destX + (-view[2] * 28 >> 14), destY + (-view[6] * 28 >> 14), destZ + (-view[10] * 28 >> 14), destX + (6 * -view[2] >> 8), destY + (6 * -view[6] >> 8), destZ + (6 * -view[10] >> 8), nullptr, n, 2, this->isZoomedIn);
	Entity* traceEntity = app->game->traceEntity;
	if (traceEntity != nullptr && (traceEntity->def->eType == 6 || traceEntity->def->eType == 11 || traceEntity->def->eType == 12 || traceEntity->def->eType == 10 || traceEntity->def->eType == 14)) {
		int i = 0;
		while (i < app->game->numTraceEntities) {
			Entity* entity = app->game->traceEntities[i];
			short linkIndex = entity->linkIndex;
			if (entity->def->eType == 2) {
				if (traceEntity->def->eType != 12) {
					traceEntity = entity;
					break;
				}
				break;
			}
			else {
				if (entity->def->eType == 5 || entity->def->eType == 4) {
					break;
				}
				if (entity->def->eType == 0) {
					break;
				}
				if (entity->def->eType == 12 && (app->render->mapFlags[linkIndex] & 0x2) != 0x0) {
					break;
				}
				if (entity->def->eType == 7) {
					if (traceEntity->def->eType == 12) {
						traceEntity = entity;
						break;
					}
					break;
				}
				else {
					if (entity->def->eType == 14) {
						if (traceEntity->def->eSubType != 6) {
							traceEntity = entity;
							break;
						}
					}
					else if (entity->def->eType == 10 && (entity->def->eSubType == 1 || entity->def->eSubType == 2 || entity->def->eSubType == 3) && traceEntity != nullptr && traceEntity->def->eType != 6) {
						traceEntity = entity;
						break;
					}
					++i;
				}
			}
		}
	}
	app->player->facingEntity = traceEntity;
	if (app->player->facingEntity != nullptr) {
		Entity* facingEntity = app->player->facingEntity;
		int dist = facingEntity->distFrom(this->viewX, this->viewY);
		if (facingEntity->def->eType != 2 && dist > app->combat->tileDistances[2]) {
			app->player->facingEntity = nullptr;
		}
		else if (facingEntity->def->eType == 3 && dist <= app->combat->tileDistances[0]) {
			app->player->showHelp((short)0, false);
		}
		else if (dist <= app->combat->tileDistances[0]) {
			if (facingEntity->def->eType == 10) {
				if (facingEntity->def->eSubType == 1) {
					app->player->showHelp((short)2, false);
				}
				else if (facingEntity->def->eSubType == 2) {
					app->player->showHelp((short)3, false);
				}
				else if (facingEntity->def->eSubType == 3) {
					app->player->showHelp((short)9, false);
				}
			}
			else if (facingEntity->def->eType == 5) {
				if (facingEntity->def->eSubType == 1) {
					app->player->showHelp((short)1, false);
				}
				app->player->showHelp((short)7, false);
			}
			else if (facingEntity->def->eType == 6 && facingEntity->def->eSubType == 3) {
				app->player->showHelp((short)4, false);
			}
			else if (facingEntity->def->tileIndex == 158) {
				app->player->showHelp((short)18, false);
			}
		}
	}
	Entity* traceEntity2 = app->game->traceEntity;
	int n2 = 4141;
	if (traceEntity2 != nullptr && (1 << traceEntity2->def->eType & n2) == 0x0) {
		for (int j = 1; j < app->game->numTraceEntities; ++j) {
			Entity* entity2 = app->game->traceEntities[j];
			if ((1 << entity2->def->eType & n2) != 0x0) {
				traceEntity2 = entity2;
				break;
			}
		}
	}
	if (traceEntity2 != nullptr) {
		int dist2 = traceEntity2->distFrom(this->viewX, this->viewY);
		if (dist2 <= app->combat->tileDistances[0] && traceEntity2->def->eType == 0 && app->combat->weaponDown) {
			app->combat->shiftWeapon(true);
		}
		else if ((this->state == Canvas::ST_PLAYING || this->state == Canvas::ST_DIALOG) && ((0x2 & 1 << app->player->ce->weapon) == 0x0 || dist2 <= app->combat->tileDistances[0])) {
			if (traceEntity2->def->eType == 3) {
				app->combat->shiftWeapon(true);
			}
			else if (app->combat->weaponDown) {
				app->combat->shiftWeapon(false);
			}
		}
		else if (this->state == Canvas::ST_DIALOG && this->oldState != Canvas::ST_INTER_CAMERA) {
			app->combat->shiftWeapon(true);
		}
		else {
			app->combat->shiftWeapon(false);
		}
	}
	else {
		app->combat->shiftWeapon(false);
	}
	this->updateFacingEntity = false;
}

void Canvas::finishMovement() {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->gotoThread != nullptr && this->viewAngle == this->destAngle) {
		this->gotoThread->run();
		this->gotoThread = nullptr;
	}
	app->game->executeTile(this->destX >> 6, this->destY >> 6, this->flagForFacingDir(8), true);
	app->game->executeTile(this->destX >> 6, this->destY >> 6, app->game->eventFlags[1], true);
	app->game->touchTile(this->destX, this->destY, true);
	if (this->knockbackDist > 0) {
		--this->knockbackDist;
		this->destZ += 12;
		if (this->knockbackDist == 0) {
			this->destZ = 36 + app->render->getHeight(this->destX, this->destY);
		}
	}
	else if (this->gotoThread == nullptr && this->state == Canvas::ST_PLAYING && app->game->monstersTurn == 0) {
		//if (this->state != Canvas::ST_AUTOMAP) {
			this->updateFacingEntity = true;
		//}
		this->uncoverAutomap();
		app->game->advanceTurn();
	}
	else if (this->state == Canvas::ST_AUTOMAP) {
		this->uncoverAutomap();
		app->game->advanceTurn();
		if (app->game->animatingEffects != 0) {
			this->setState(Canvas::ST_PLAYING);
		}
		else {
			app->game->snapMonsters(true);
			app->game->snapLerpSprites(-1);
		}
	}
}

int Canvas::flagForWeapon(int i) {
	Applet* app = CAppContainer::getInstance()->app;
	bool weaponIsASentryBot = app->player->weaponIsASentryBot(i);
	i = 1 << i;
	if (weaponIsASentryBot && (!app->player->isFamiliar || (app->player->familiarType != 1 && app->player->familiarType != 3))) {
		return 0;
	}
	if ((i & 0x2) != 0x0) {
		return 4096;
	}
	if ((i & 0x800) != 0x0) {
		return 16384;
	}
	return 8192;
}

int Canvas::flagForFacingDir(int i) {
	int destAngle = this->destAngle;
	if (i == 4) {
		destAngle += 512;
	}
	if (i == 8 || i == 4) {
		return i | 1 << ((destAngle & 0x3FF) >> 7) + 4;
	}
	return 0;
}

void Canvas::startRotation(bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	int8_t b2 = Canvas::viewStepValues[((this->destAngle & 0x3FF) >> 7 << 1) + 0];
	int8_t b3 = Canvas::viewStepValues[((this->destAngle & 0x3FF) >> 7 << 1) + 1];
	int n = 384;
	app->game->trace(this->destX, this->destY, this->destX + (b2 * n >> 6), this->destY + (b3 * n >> 6), nullptr, 4133, 2);
	Entity* traceEntity = app->game->traceEntity;
	int n2 = app->game->traceFracs[0] * n >> 14;
	int n3;
	int n4;
	if (traceEntity != nullptr && (traceEntity->def->eType == 0 || traceEntity->def->eType == 12) && n2 <= 36) {
		n3 = this->destZ;
		n4 = (b ? 0 : 1);
	}
	else {
		bool b4 = !this->pitchIsControlled(this->destX >> 6, this->destY >> 6, app->game->VecToDir(b2 * 32, b3 * 32, false));
		if (traceEntity != nullptr && traceEntity->def->eType == 2) {
			if (b4) {
				int* calcPosition = traceEntity->calcPosition();
				n3 = app->render->getHeight(calcPosition[0], calcPosition[1]) + 36;
			}
			else {
				n3 = this->destZ;
			}
			n4 = 1;
		}
		else {
			n2 = 64;
			if (b4) {
				n3 = app->render->getHeight(this->destX + b2, this->destY + b3) + 36;
			}
			else {
				n3 = this->destZ;
			}
			n4 = (b ? 0 : 1);
		}
		n4 = 1;
	}
	if (n4 == 0) {
		return;
	}
	if (n2 == 0) {
		this->destPitch = 0;
	}
	else {
		this->destPitch = ((n3 - this->destZ) << 7) / n2;
	}
	if (this->destPitch < -64) {
		this->destPitch = -64;
	}
	else if (this->destPitch > 64) {
		this->destPitch = 64;
	}
	this->pitchStep = std::abs((this->destPitch - this->viewPitch) / this->animFrames);
}

void Canvas::finishRotation(bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	this->viewSin = app->render->sinTable[this->destAngle & 0x3FF];
	this->viewCos = app->render->sinTable[this->destAngle + 256 & 0x3FF];
	this->viewStepX = Canvas::viewStepValues[(((this->destAngle & 0x3FF) >> 7) << 1) + 0];
	this->viewStepY = Canvas::viewStepValues[(((this->destAngle & 0x3FF) >> 7) << 1) + 1];
	int n = this->destAngle - 256 & 0x3FF;
	this->viewRightStepX = Canvas::viewStepValues[((n >> 7) << 1) + 0];
	this->viewRightStepY = Canvas::viewStepValues[((n >> 7) << 1) + 1];
	if (b && app->hud->msgCount > 0 && (app->hud->messageFlags[0] & 0x2) != 0x0) {
		app->hud->msgTime = 0;
	}
	if (this->gotoThread != nullptr && this->viewX == this->destX && this->viewY == this->destY) {
		ScriptThread* gotoThread = this->gotoThread;
		this->gotoThread = nullptr;
		gotoThread->run();
	}
	if (this->state == Canvas::ST_COMBAT) {
		this->updateFacingEntity = true;
	}
	else {
		app->game->executeTile(this->destX >> 6, this->destY >> 6, this->flagForFacingDir(8), true);
		this->updateFacingEntity = true;
	}
}

/*int KEY_ARROWUP = -1;
int KEY_ARROWDOWN = -2;
int KEY_ARROWLEFT = -3;
int KEY_ARROWRIGHT = -4;
int KEY_OK = -5;
int KEY_CLR = -8;
int KEY_LEFTSOFT = -6;
int KEY_RIGHTSOFT = -7;
int KEY_BACK = -8;*/

#include "Input.h"

#define MOVEFORWARD	1
#define MOVEBACK	2
#define TURNLEFT	3
#define TURNRIGHT	4
#define MENUOPEN	5
#define SELECT		6
#define AUTOMAP		7
//8
#define MOVELEFT		9
#define MOVERIGHT		10
#define PREVWEAPON		11
#define NEXTWEAPON		12
//13
#define PASSTURN		14
//15
#define MENU_UP			16
#define MENU_DOWN		17
#define MENU_PAGE_UP	18
#define MENU_PAGE_DOWN	19
#define MENU_SELECT		20
#define MENU_OPEN		21

#define NUM_CODES 36
int keys_codeActions[NUM_CODES] = {
	AVK_CLR,		Enums::ACTION_BACK,
	AVK_SOFT2,		Enums::ACTION_AUTOMAP,
	AVK_SOFT1,		Enums::ACTION_MENU,
	// New items Only Port
	AVK_STAR,		Enums::ACTION_PREVWEAPON,
	AVK_POUND,		Enums::ACTION_AUTOMAP,
	AVK_NEXTWEAPON, Enums::ACTION_NEXTWEAPON,
	AVK_PREVWEAPON, Enums::ACTION_PREVWEAPON,
	AVK_AUTOMAP,	Enums::ACTION_AUTOMAP,
	AVK_UP,			Enums::ACTION_UP,
	AVK_DOWN,		Enums::ACTION_DOWN,
	AVK_LEFT,		Enums::ACTION_LEFT,
	AVK_RIGHT,		Enums::ACTION_RIGHT,
	AVK_MOVELEFT,	Enums::ACTION_STRAFELEFT,
	AVK_MOVERIGHT,	Enums::ACTION_STRAFERIGHT,
	AVK_SELECT,		Enums::ACTION_FIRE,
	AVK_MENUOPEN,	Enums::ACTION_MENU,
	AVK_PASSTURN,	Enums::ACTION_PASSTURN,
	AVK_BOTDISCARD,	Enums::ACTION_BOT_DISCARD
};

int Canvas::getKeyAction(int i) {
	Applet* app = CAppContainer::getInstance()->app;
	int iVar1;

	//printf("getKeyAction i %d\n", i);

	if (this->state == Canvas::ST_MENU) {
		if (i & AVK_MENU_UP) {
			return Enums::ACTION_UP;
		}
		if (i & AVK_MENU_DOWN) {
			return Enums::ACTION_DOWN;
		}
		if (i & AVK_MENU_PAGE_UP) {
			return Enums::ACTION_LEFT;
		}
		if (i & AVK_MENU_PAGE_DOWN) {
			return Enums::ACTION_RIGHT;
		}
		if (i & AVK_MENU_SELECT) {
			return Enums::ACTION_FIRE;
		}
		if (i & AVK_ITEMS_INFO) {
			return Enums::ACTION_MENU_ITEM_INFO;
		}
	}

	if (i & AVK_MENU_OPEN) {
		return Enums::ACTION_MENU;
	}

	if (!app->player->isFamiliar) {
		if (i & AVK_ITEMS_INFO) {
			return Enums::ACTION_ITEMS;
		}

		if (i & AVK_DRINKS) {
			return Enums::ACTION_ITEMS_DRINKS;
		}

		if (i & AVK_PDA) {
			return Enums::ACTION_QUESTLOG;
		}
	}

	i &= ~(AVK_MENU_UP | AVK_MENU_DOWN | AVK_MENU_PAGE_UP | AVK_MENU_PAGE_DOWN | AVK_MENU_SELECT | AVK_MENU_OPEN | AVK_ITEMS_INFO | AVK_DRINKS | AVK_PDA);

	for (int j = 0; j < (NUM_CODES / 2); j++)
	{
		if (keys_codeActions[(j * 2) + 0] == i) {
			//printf("rtn %d\n", keys_codeActions[(j * 2) + 1]);
			return keys_codeActions[(j * 2) + 1];
		}
	}

	if (i - 1U < 10) { // KEY_1 to KEY_9 ... KEY_0
		return this->keys_numeric[i - 1U];
	}

	if (i == 12) { // KEY_STAR
		return Enums::ACTION_PREVWEAPON;
	}
	if (i == 11) { // KEY_POUND
		return Enums::ACTION_AUTOMAP;
	}
	if (i == 14) { // KEY_ARROWLEFT
		return Enums::ACTION_LEFT;
	}
	if (i == 15) { // KEY_ARROWRIGHT
		return Enums::ACTION_RIGHT;
	}
	if (i == 16) { // KEY_ARROWUP
		return Enums::ACTION_UP;
	}
	if (i == 17) { // KEY_ARROWDOWN
		return Enums::ACTION_DOWN;
	}
	if (i == 13) { // KEY_OK
		return Enums::ACTION_FIRE;
	}

	iVar1 = (i ^ i >> 0x1f) - (i >> 0x1f);
	if (iVar1 == 19) { // KEY_LEFTSOFT
		return Enums::ACTION_MENU;
	}
	if (iVar1 == 20) { // KEY_RIGHTSOFT
		return Enums::ACTION_AUTOMAP; // ACTION_AUTOMAP
	}
	if (iVar1 == 18) { // KEY_CLR, KEY_BACK
		return Enums::ACTION_BACK;
	}

	return Enums::ACTION_NONE;
}

bool Canvas::attemptMove(int n, int n2) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->renderOnly) {
		this->destX = n;
		this->destY = n2;
		return true;
	}

	if (app->player->isFamiliar && (app->render->mapFlags[(n2 >> 6) * 32 + (n >> 6)] & 0x10) != 0x0 && (n != this->saveX || n2 != this->saveY)) {
		app->hud->addMessage((short)0, (short)222, 3);
		return false;
	}
	int n3 = app->player->noclip ? 0 : 13501;
	app->game->eventFlagsForMovement(this->viewX, this->viewY, n, n2);
	this->abortMove = false;
	app->game->executeTile(this->viewX >> 6, this->viewY >> 6, app->game->eventFlags[0], true);
	bool b = false;
	if (!this->abortMove) {
		app->game->trace(this->viewX, this->viewY, n, n2, app->player->getPlayerEnt(), n3, 16);
		if (app->game->traceEntity == nullptr || (app->player->isFamiliar && n == this->saveX && n2 == this->saveY && app->game->traceEntity == &app->game->entities[app->player->playerEntityCopyIndex])) {
			if (app->player->isFamiliar && n == this->saveX && n2 == this->saveY && app->game->traceEntity == &app->game->entities[app->player->playerEntityCopyIndex]) {
				app->player->familiarReturnsToPlayer(true);
			}
			if (app->hud->msgCount > 0 && (app->hud->messageFlags[0] & 0x2) != 0x0) {
				app->hud->msgTime = 0;
			}
			this->automapDrawn = false;
			this->destX = n;
			this->destY = n2;
			this->destZ = 36 + app->render->getHeight(this->destX, this->destY);
			this->zStep = (std::abs(this->destZ - this->viewZ) + this->animFrames - 1) / this->animFrames;
			this->prevX = this->viewX;
			this->prevY = this->viewY;
			this->startRotation(false);
			app->player->relink();
			b = true;
		}
		else if (this->knockbackDist == 0 && this->state == Canvas::ST_AUTOMAP) {
			app->game->advanceTurn();
		}
	}
	else if (this->knockbackDist != 0) {
		this->knockbackDist = 0;
	}
	return b;
}

void Canvas::loadState(int loadType, short n, short n2) {
	Applet* app = CAppContainer::getInstance()->app;
	this->loadType = loadType;
	app->game->saveConfig();
	this->setLoadingBarText(n, n2);
	this->setState(Canvas::ST_LOADING);
}

void Canvas::saveState(int saveType, short n, short n2) {
	this->saveType = saveType;
	this->setLoadingBarText(n, n2);
	this->setState(Canvas::ST_SAVING);
}

void Canvas::loadMap(int loadMapID, bool b, bool tm_NewGame) {
	Applet* app = CAppContainer::getInstance()->app;

	if (loadMapID > 0 && loadMapID < 11) {
		bool b2 = false;
		int n = loadMapID - 1;
		short n2 = app->game->numLevelLoads[n];
		app->game->numLevelLoads[n] = (short)(n2 + 1);
		if ((b2 ? 1 : 0) == n2) {
			app->player->currentLevelDeaths = 0;
		}
	}
	this->lastMapID = this->loadMapID;
	this->loadMapID = loadMapID;
	app->sound->soundStop();
	this->TM_NewGame = tm_NewGame;
	if (!b && app->game->activeLoadType == 0 && this->lastMapID >= 1 && this->lastMapID <= 10) {
		this->saveState(43, (short)3, (short)196);
	}
	else {
		this->setLoadingBarText((short)3, app->game->levelNames[this->loadMapID - 1]);
		this->setState(Canvas::ST_TRAVELMAP);
	}
}

void Canvas::loadPrologueText() {
	Applet* app = CAppContainer::getInstance()->app;
	Text* text;

	this->storyPage = 0;
	this->storyTotalPages = 0;
	app->localization->resetTextArgs();

	short n = 0;
	switch (app->player->characterChoice) {
	case 1: {
		n = 218;
		break;
	}
	case 2: {
		n = 219;
		break;
	}
	default: {
		n = 220;
		break;
	}
	}

	app->localization->addTextArg(3, n);
	text = app->localization->getLargeBuffer();
	app->localization->loadText(2);
	app->localization->composeText(2, 12, text);
	app->localization->unloadText(2);

	int n2 = (this->displayRect[2] - 30) / 10;
	int n3 = (this->displayRect[3] - 40) / 21;
	text->wrapText(n2);

	this->storyIndexes[this->storyTotalPages++] = 0;

	int n4 = 0;
	int first = 0;
	while ((first = text->findFirstOf('|', first)) != -1) {
		++first;
		if (++n4 % n3 == 0) {
			this->storyIndexes[this->storyTotalPages++] = first;
		}
	}

	this->dialogBuffer = text;
	this->storyIndexes[this->storyTotalPages] = text->length();
	this->storyX = this->displayRect[0] + 15;
	this->storyY = this->displayRect[1] + 20;
}

void Canvas::loadEpilogueText() {
	Applet* app = CAppContainer::getInstance()->app;
	this->imgProlog = app->loadImage("prolog.bmp", true);
	this->initScrollingText((short)0, (short)134, false, 32, 1, 1000);
}

void Canvas::setupCharacterSelection() {
	Applet* app = CAppContainer::getInstance()->app;

	app->beginImageLoading();
	this->imgCharacter_select_stat_bar = app->loadImage("Character_select_stat_bar.bmp", true);
	this->imgCharacter_select_stat_header = app->loadImage("Character_select_stat_header.bmp", true);
	this->imgTopBarFill = app->loadImage("Character_select_top_bar.bmp", true);
	this->imgCharacter_upperbar = app->loadImage("character_upperbar.bmp", true);
	this->imgCharacterSelectionAssets = app->loadImage("charSelect.bmp", true);
	this->imgCharSelectionBG = app->loadImage("charSelectionBG.bmp", true);
	this->imgMajorMugs = app->loadImage("Hud_Player.bmp", true);
	this->imgSargeMugs = app->loadImage("Hud_PlayerDoom.bmp", true);
	this->imgScientistMugs = app->loadImage("Hud_PlayerScientist.bmp", true);
	this->imgMajor_legs = app->loadImage("Major_legs.bmp", true);
	this->imgMajor_torso = app->loadImage("Major_torso.bmp", true);
	this->imgRiley_legs = app->loadImage("Riley_legs.bmp", true);
	this->imgRiley_torso = app->loadImage("Riley_torso.bmp", true);
	this->imgSarge_legs = app->loadImage("Sarge_legs.bmp", true);
	this->imgSarge_torso = app->loadImage("Sarge_torso.bmp", true);
	app->endImageLoading();
}

void Canvas::disposeIntro() {
	this->dialogBuffer->dispose();
	this->dialogBuffer = NULL;
	this->loadMap(this->startupMap, false, true);
}

void Canvas::disposeEpilogue() {
	Applet* app = CAppContainer::getInstance()->app;
	this->dialogBuffer->dispose();
	this->dialogBuffer = nullptr;
	app->sound->soundStop();
	app->menuSystem->setMenu(Menus::MENU_LEVEL_STATS);
	app->sound->playSound(1069,'\x01',3,false);
}

void Canvas::loadMiniGameImages() {
	Applet* app = CAppContainer::getInstance()->app;

	app->beginImageLoading();
	app->hackingGame->imgEnergyCore = app->loadImage("hackerBG.bmp", true);
	app->hackingGame->imgGameColors = app->loadImage("blockGameColors.bmp", true);
	app->hackingGame->imgHelpScreenAssets = app->loadImage("gameHelpBG.bmp", true);
	app->sentryBotGame->imgMatrixSkip_BG = app->loadImage("matrixSkip_BG.bmp", true);
	app->sentryBotGame->imgGameAssets = app->loadImage("sentryGame.bmp", true);
	app->sentryBotGame->imgButton = app->loadImage("matrixSkip_grid_pressed.bmp", true);
	app->sentryBotGame->imgButton2 = app->loadImage("matrixSkip_grid_pressed_2.bmp", true);
	app->sentryBotGame->imgButton3 = app->loadImage("matrixSkip_grid_pressed_3.bmp", true);
	app->vendingMachine->imgVendingGame = app->loadImage("vendingGame.bmp", true);
	app->vendingMachine->imgVending_BG = app->loadImage("vending_BG.bmp", true);
	app->vendingMachine->imgVendingBG = app->loadImage("vendingBG.bmp", true);
	app->vendingMachine->imgSubmitButton = app->loadImage("vending_submit.bmp", true);
	app->vendingMachine->imgVending_submit_pressed = app->loadImage("vending_submit_pressed.bmp", true);
	app->vendingMachine->imgVending_arrow_up = app->loadImage("vending_arrow_up.bmp", true);
	app->vendingMachine->imgVending_arrow_up_pressed = app->loadImage("vending_arrow_up_pressed.bmp", true);
	app->vendingMachine->imgVending_arrow_down = app->loadImage("vending_arrow_down.bmp", true);
	app->vendingMachine->imgVending_arrow_down_pressed = app->loadImage("vending_arrow_down_pressed.bmp", true);
	app->vendingMachine->imgVending_button_small = app->loadImage("vending_button_small.bmp", true);
	app->endImageLoading();

	app->vendingMachine->imgHelpScreenAssets = app->sentryBotGame->imgHelpScreenAssets = app->hackingGame->imgHelpScreenAssets;


	// [GEC] Fix the image
	fixImage(app->hackingGame->imgGameColors);
	fixImage(app->vendingMachine->imgVending_arrow_down);
}

void Canvas::drawScroll(Graphics* graphics, int n, int n2, int n3, int n4) { // J2ME
	int width = this->imgDialogScroll->width;
	int height = this->imgDialogScroll->height;
	graphics->drawRegion(this->imgDialogScroll, 0, 0, width, height, n, n2 + n4 - height, 0, 0, 0);
	graphics->drawRegion(this->imgDialogScroll, 0, 0, width, height, n + n3 - width, n2 + n4 - height, 0, 2, 0);
	graphics->drawRegion(this->imgDialogScroll, 0, 0, width, height, n, n2, 0, 1, 0);
	graphics->drawRegion(this->imgDialogScroll, 0, 0, width, height, n + n3 - width, n2, 0, 3, 0);
	graphics->fillRegion(this->imgDialogScroll, width - 14, 0, 14, height, n + width, n2, n3 - 2 * width, height, 3);
	graphics->fillRegion(this->imgDialogScroll, width - 14, 0, 14, height, n + width, n2 + (n4 - height), n3 - 2 * width, height, 0);
	graphics->fillRegion(this->imgDialogScroll, 0, 0, width, 6, n, n2 + height, width, n4 - 2 * height, 0);
	graphics->fillRegion(this->imgDialogScroll, 0, 0, width, 6, n + n3 - width, n2 + height, width, n4 - 2 * height, 3);
	graphics->fillRegion(this->imgDialogScroll, width - 6, 0, 6, 6, n + width, n2 + height, n3 - 2 * width, n4 - 2 * height, 0);
}

void Canvas::initScrollingText(short i, short i2, bool dehyphenate, int spacingHeight, int numLines, int textMSLine) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->dialogBuffer == nullptr) {
		this->dialogBuffer = app->localization->getLargeBuffer();
	}
	else {
		this->dialogBuffer->setLength(0);
	}

	app->localization->composeText(i, i2, this->dialogBuffer);

	if (dehyphenate) {
		this->dialogBuffer->dehyphenate();
	}

	this->dialogBuffer->wrapText((this->displayRect[2] - 8) / Applet::CHAR_SPACING[app->fontType]);
	this->scrollingTextSpacing = spacingHeight;
	this->scrollingTextStart = -1;
	this->scrollingTextLines = (this->dialogBuffer->getNumLines() + numLines);
	this->scrollingTextMSLine = textMSLine;
	this->scrollingTextDone = false;
	this->scrollingTextFontHeight = Applet::FONT_HEIGHT[app->fontType] + 2;
	this->scrollingTextSpacingHeight = spacingHeight * ((316 - (Applet::FONT_HEIGHT[app->fontType] * 2)) / spacingHeight);
}

void Canvas::drawCredits(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	Text* textBuff;

	this->drawScrollingText(graphics);
	if (this->scrollingTextDone != false) {
		textBuff = app->localization->getSmallBuffer();
		textBuff->setLength(0);
		app->localization->composeText(0, 43, textBuff);
		textBuff->dehyphenate();
		graphics->drawString(textBuff, this->SCR_CX - 24, this->screenRect[3] - 32, 2);
		textBuff->dispose();
	}
}

void Canvas::drawScrollingText(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	int n = this->scrollingTextMSLine * ((this->scrollingTextSpacing << 16) >> 4) >> 16;
	if (this->scrollingTextStart == -1) {
		this->scrollingTextStart = app->gameTime;
		int n2 = this->screenRect[3] / this->scrollingTextSpacing;
		if (this->state == Canvas::ST_CREDITS) {
			this->scrollingTextEnd = n * this->scrollingTextLines;
		}
		else {
			this->scrollingTextEnd = n * (this->scrollingTextLines + (n2 - 2));
		}
	}

	int gameTime = app->gameTime;
	int scrollingTextStart = this->scrollingTextStart;
	if (gameTime - scrollingTextStart > this->scrollingTextEnd) {
		scrollingTextStart = gameTime - this->scrollingTextEnd;
		this->scrollingTextDone = true;
	}

	graphics->eraseRgn(this->displayRect);

	if (this->state == Canvas::ST_EPILOGUE || this->state == Canvas::ST_INTRO_MOVIE) {
		int rect[4];
		rect[0] = this->displayRect[0];
		rect[1] = this->displayRect[1];
		rect[2] = this->displayRect[2] - rect[0];
		rect[3] = this->displayRect[3]; // missing line code?
		graphics->clipRect(0, (this->displayRect[3] - 220) / 2, this->displayRect[2], 220);
		graphics->drawRegion(this->imgProlog, 0, 65, 480, 307, 0, 0, 0, 0, 0);
		graphics->fade(rect, 192, 0);
		graphics->setScreenSpace(0, (this->displayRect[3] - 220) / 2, this->displayRect[2], 220);
	}

	graphics->drawString(this->dialogBuffer, this->SCR_CX, this->cinRect[3] - ((((gameTime - scrollingTextStart) << 8) / n) * (this->scrollingTextSpacing << 8) >> 16), this->scrollingTextSpacing, 1, 0, -1);
	graphics->resetScreenSpace();
}

void Canvas::handleDialogEvents(int key) {
	Applet* app = CAppContainer::getInstance()->app;

	int action = this->getKeyAction(key);
	if (action == Enums::ACTION_FIRE) {
		if (this->dialogTypeLineIdx < this->dialogViewLines && this->dialogTypeLineIdx < this->numDialogLines - this->currentDialogLine) {
			this->dialogTypeLineIdx = this->dialogViewLines;
		}
		else if (this->currentDialogLine < this->numDialogLines - this->dialogViewLines) {
			this->dialogLineStartTime = app->time;
			this->dialogTypeLineIdx = 0;
			this->currentDialogLine += this->dialogViewLines;
			if (((this->dialogFlags & 0x4) != 0x0 || (this->dialogFlags & 0x1) != 0x0) && this->currentDialogLine + this->dialogViewLines > this->numDialogLines) {
				this->currentDialogLine = this->numDialogLines - this->dialogViewLines;
			}
		}
		else {
			this->closeDialog(false);
		}
	}
	else if (action == Enums::ACTION_UP) {
		if (this->currentDialogLine >= this->numDialogLines - this->dialogViewLines && 0x0 != (this->dialogFlags & 0x2)) {
			if (app->game->scriptStateVars[4] == 0) {
				this->currentDialogLine--;
				if (this->currentDialogLine < 0) {
					this->currentDialogLine = 0;
				}
			}
			else {
				app->game->scriptStateVars[4]--;
			}
		}
		else {
			this->currentDialogLine--;
			if (this->currentDialogLine < 0) {
				this->currentDialogLine = 0;
			}
		}
	}
	else if (action == Enums::ACTION_DOWN) {
		if (this->currentDialogLine >= this->numDialogLines - this->dialogViewLines && 0x0 != (this->dialogFlags & 0x2)) {
			if (app->game->scriptStateVars[4] < 1) {
				app->game->scriptStateVars[4]++;
			}
		}
		else {
			this->currentDialogLine++;
			if (this->currentDialogLine > this->numDialogLines - this->dialogViewLines) {
				this->currentDialogLine = this->numDialogLines - this->dialogViewLines;
				if (0x0 == (this->dialogFlags & 0x2)) {
					if (this->currentDialogLine < 0) {
						this->currentDialogLine = 0;
					}
				}
			}
			else {
				this->dialogLineStartTime = app->time;
				this->dialogTypeLineIdx = this->dialogViewLines - 1;
			}
		}
	}
	else if ((action == Enums::ACTION_LEFT || action == Enums::ACTION_RIGHT) && (this->dialogFlags & 0x5) != 0x0 && this->currentDialogLine >= this->numDialogLines - this->dialogViewLines) {
		app->game->scriptStateVars[4] ^= 0x1;
	}
	else if (action == Enums::ACTION_PASSTURN || action == Enums::ACTION_AUTOMAP) {
		this->closeDialog(true);
	}
	else if (action == Enums::ACTION_MENU || action == Enums::ACTION_LEFT) {
		this->currentDialogLine -= this->dialogViewLines;
		if (this->currentDialogLine < 0) {
			this->currentDialogLine = 0;
		}
	}
	else if (action == Enums::ACTION_RIGHT) {
		this->currentDialogLine += this->dialogViewLines;
		if (this->currentDialogLine > this->numDialogLines - this->dialogViewLines) {
			this->currentDialogLine = std::max(this->numDialogLines - this->dialogViewLines, 0);
		}
	}
	if (this->state == Canvas::ST_PLAYING && app->game->monstersTurn == 0) {
		this->dequeueHelpDialog();
	}
}

bool Canvas::handlePlayingEvents(int key, int action) {
	Applet* app = CAppContainer::getInstance()->app;
	//printf("handlePlayingEvents %d, %d\n", key, action);

	bool b = false;
	if (!this->isZoomedIn && (this->viewX != this->destX || this->viewY != this->destY || this->viewAngle != this->destAngle)) {
		return false;
	}

	if (this->knockbackDist != 0 || this->changeMapStarted) {
		return false;
	}

	if (this->renderOnly) {
		this->viewX = this->destX;
		this->viewY = this->destY;
		this->viewZ = this->destZ;
		this->viewAngle = this->destAngle;
		this->viewAngle = this->destPitch;
		this->viewSin = app->render->sinTable[this->destAngle & 0x3FF];
		this->viewCos = app->render->sinTable[this->destAngle + 256 & 0x3FF];
		this->viewStepX = this->viewCos * 64 >> 16;
		this->viewStepY = -this->viewSin * 64 >> 16;
		this->invalidateRect();
	}
	else {
		if (!this->isZoomedIn) {
			if (this->viewX != this->destX || this->viewY != this->destY) {
				b = true;
				this->viewX = this->destX;
				this->viewY = this->destY;
				this->viewZ = this->destZ;
				this->finishMovement();
				this->invalidateRect();
			}
			else if (this->viewAngle != this->destAngle) {
				b = true;
				this->viewAngle = this->destAngle;
				this->viewAngle = this->destPitch;
				this->finishRotation(true);
				this->invalidateRect();
			}
		}
		if (this->blockInputTime != 0) {
			return true;
		}

		if (app->hud->isInWeaponSelect != 0) { // [GEC]
			return true;
		}

		bool b2 = this->state == Canvas::ST_AUTOMAP;
		if (action != Enums::ACTION_PREVWEAPON && action != Enums::ACTION_NEXTWEAPON && action != Enums::ACTION_LEFT && action != Enums::ACTION_RIGHT && (app->game->activePropogators != 0 || !app->game->snapMonsters(b2) || app->game->animatingEffects != 0)) {
			return true;
		}
	}

	if (action == Enums::ACTION_BOT_DISCARD) { // [GEC]
		if (app->player->isFamiliar && this->state != Canvas::ST_AUTOMAP) {
			app->player->familiarReturnsToPlayer(false);
		}
		else if (app->player->weaponIsASentryBot(app->player->ce->weapon) && this->state != Canvas::ST_AUTOMAP) {
			app->player->attemptToDiscardFamiliar(app->player->ce->weapon);
		}
	}

	if (action == Enums::ACTION_AUTOMAP) {
		if (!app->player->inTargetPractice) {
			// OLD -> Original
			/*if (app->player->isFamiliar && this->state != Canvas::ST_AUTOMAP) {
				app->player->familiarReturnsToPlayer(false);
			}
			else if (app->player->weaponIsASentryBot(app->player->ce->weapon) && this->state != Canvas::ST_AUTOMAP) {
				app->player->attemptToDiscardFamiliar(app->player->ce->weapon);
			}
			else */{
				this->setState((this->state != Canvas::ST_AUTOMAP) ? Canvas::ST_AUTOMAP : Canvas::ST_PLAYING);
			}
		}
	}
	else if (action == Enums::ACTION_UP) {
		this->attemptMove(this->viewX + this->viewStepX, this->viewY + this->viewStepY);
	}
	else if (action == Enums::ACTION_DOWN) {
		this->attemptMove(this->viewX - this->viewStepX, this->viewY - this->viewStepY);
	}
	else if (action == Enums::ACTION_STRAFELEFT) {
		this->attemptMove(this->viewX + this->viewStepY, this->viewY - this->viewStepX);
	}
	else if (action == Enums::ACTION_STRAFERIGHT) {
		this->attemptMove(this->viewX - this->viewStepY, this->viewY + this->viewStepX);
	}
	else if (action == Enums::ACTION_LEFT || action == Enums::ACTION_RIGHT) {
		int n3 = 256;
		app->hud->damageTime = 0;
		if (action == Enums::ACTION_RIGHT) {
			n3 = -256;
		}
		this->destAngle += n3;
		this->startRotation(false);
		this->automapDrawn = false;
	}
	else if (action == Enums::ACTION_PREVWEAPON || action == Enums::ACTION_NEXTWEAPON) {
		int weapon = app->player->ce->weapon;
		if (weapon == 14) {
			return true;
		}
		if (action == Enums::ACTION_PREVWEAPON) {
			app->player->selectPrevWeapon();
		}
		else {
			app->player->selectNextWeapon();
		}
		if (weapon != app->player->ce->weapon) {
			app->hud->addMessage((short)1, (short)app->player->activeWeaponDef->longName, 1);
		}
		app->player->helpBitmask |= 0x100;
	}
	else if (action == Enums::ACTION_BACK) {
		if (this->state == Canvas::ST_AUTOMAP) {
			this->setState(Canvas::ST_PLAYING);
		}
		else if (app->player->inTargetPractice) {
			app->player->exitTargetPractice();
		}
		else {
			app->hud->msgCount = 0;
			app->menuSystem->setMenu(Menus::MENU_INGAME);
		}
	}
	else if (action == Enums::ACTION_MENU) {
		if (app->player->inTargetPractice) {
			app->player->exitTargetPractice();
		}
		else {
			app->hud->msgCount = 0;
			app->menuSystem->setMenu(Menus::MENU_INGAME);
		}
	}
	else if (action == Enums::ACTION_ITEMS) {
		if (app->player->inTargetPractice) {
			app->player->exitTargetPractice();
		}
		else {
			app->hud->msgCount = 0;
			app->menuSystem->setMenu(Menus::MENU_ITEMS);
			app->menuSystem->oldMenu = Menus::MENU_ITEMS; // [GEC]
		}
	}
	else if (action == Enums::ACTION_ITEMS_DRINKS) {
		if (app->player->inTargetPractice) {
			app->player->exitTargetPractice();
		}
		else {
			app->hud->msgCount = 0;
			app->menuSystem->setMenu(Menus::MENU_ITEMS_DRINKS);
			app->menuSystem->oldMenu = Menus::MENU_ITEMS_DRINKS; // [GEC]
		}
	}
	else if (action == Enums::ACTION_QUESTLOG) {
		if (app->player->inTargetPractice) {
			app->player->exitTargetPractice();
		}
		else {
			app->hud->msgCount = 0;
			app->menuSystem->setMenu(Menus::MENU_INGAME_QUESTLOG);
			app->menuSystem->oldMenu = Menus::MENU_INGAME_QUESTLOG; // [GEC]
		}
	}
	else if (action == Enums::ACTION_FIRE) {
		if (app->player->facingEntity != nullptr && app->player->facingEntity->def->eType == 10) {
			this->lootSource = app->player->facingEntity->name;
		}
		else {
			this->lootSource = -1;
		}

		int weapon2 = app->player->ce->weapon;
		
		int n4 = 16384;
		int n5 = 13997;

		if (Entity::CheckWeaponMask(weapon2, 2) != 0x0) {
			//n5 |= 0x2000; // J2ME only?
		}
		if (weapon2 == 2) {
			n5 |= 0x4100;
		}
		int n6 = 0;
		int n7 = 6;
		if (Entity::CheckWeaponMask(weapon2, 2) != 0x0) {
			n7 = 1;
			n5 |= 0x10;
		}

		Entity* entity = nullptr;
		Entity* entity2 = nullptr;

		int n8 = this->viewX + (n7 * -app->tinyGL->view[2] >> 8);
		int n9 = this->viewY + (n7 * -app->tinyGL->view[6] >> 8);
		int n10 = this->viewZ + (n7 * -app->tinyGL->view[10] >> 8);
		app->game->trace(this->viewX, this->viewY, this->viewZ, n8, n9, n10, nullptr, n5, 2, this->isZoomedIn);

		int i = 0;
		while (i < app->game->numTraceEntities) {
			Entity* entity3 = app->game->traceEntities[i];
			int n11 = app->game->traceFracs[i];
			int dist = entity3->distFrom(this->viewX, this->viewY);
			uint8_t eType = entity3->def->eType;
			if (eType == 0 || eType == 12 || eType == 4) {
				if (entity == nullptr) {
					entity = entity3;
					n4 = n11;
					break;
				}
				break;
			}
			else {
				if (eType == 10) {
					if ((1 << entity3->def->eSubType & 0x1) == 0x0 || app->player->ce->weapon == 1) {
						if (n6 == 0) {
							entity = entity3;
							n4 = n11;
							break;
						}
						break;
					}
				}
				else if (eType == 13) {
					if (Entity::CheckWeaponMask(weapon2, 2) != 0x0) {
						entity2 = entity3;
					}
				}
				else if (eType == 3) {
					if (dist >= 8192) {
						if (n6 == 0) {
							entity = entity3;
							n4 = n11;
							break;
						}
						break;
					}
				}
				else {
					if (eType == 2) {
						entity = entity3;
						n4 = n11;
						n6 = 0;
						break;
					}
					if (eType == 5) {
						if (entity == nullptr) {
							entity = entity3;
							n4 = n11;
							n6 = 0;
							break;
						}
						break;
					}
					else if (eType == 9) {
						if (dist == app->combat->tileDistances[0]) {
							if (weapon2 == 1) {
#if 0 // J2ME
								if (entity != nullptr && entity->def->eType == 9) {
									if (entity->linkIndex < entity3->linkIndex) {
										if (entity3->monster == nullptr && entity3->def->eSubType != 11) {
											if (entity3->param == 0 && entity3->lootSet != nullptr) {
												entity = entity3;
												n4 = n11;
												n6 = 1;
											}
											else {
												app->hud->addMessage((short)0, (short)250, 2);
											}
										}
										else {
											entity = entity3;
											n4 = n11;
										}
									}
								}
								else if (entity3->monster == nullptr && entity3->def->eSubType != 11) {
									if (entity3->param == 0 && entity3->lootSet != nullptr) {
										entity = entity3;
										n4 = n11;
										n6 = 1;
									}
									else {
										app->hud->addMessage((short)0, (short)250, 2);
									}
								}
								else {
									entity = entity3;
									n4 = n11;
								}
#else
								if (((entity == nullptr) || (entity->def->eType != 9))
									|| (entity->linkIndex < entity3->linkIndex)) {
									entity = entity3;
									n4 = n11;
								}
#endif
							}
							else if (!this->isZoomedIn) {
								if (entity3->monster == nullptr) {
									if (entity3->param == 0 && entity3->lootSet != nullptr) {
										entity = entity3;
										n4 = n11;
										n6 = 1;
									}
								}
								else if ((entity3->monster->flags & 0x800) == 0x0 && entity3->lootSet != nullptr) {
									entity = entity3;
									n4 = n11;
									n6 = 1;
								}
							}
						}
					}
					else if (eType == 8) {
						if (entity3->def->eSubType == 1 && weapon2 == 2 && app->player->ammo[3] >= 2) {
							entity = entity3;
							n4 = n11;
							n6 = 0;
							break;
						}
					}
					else if (eType == 7) {
						if ((app->render->mapSpriteInfo[entity3->getSprite()] & 0xFF) == 0x95) {
							entity = entity3;
							n4 = n11;
							break;
						}
					}
					else if (eType == 14) {
						if (entity3->def->eSubType == 7 && dist == app->combat->tileDistances[0]) {
							entity = entity3;
							n4 = n11;
							break;
						}
					}
					else if (eType != 7 && eType != 6 && entity == nullptr) {
						entity = entity3;
						n4 = n11;
					}
				}
				++i;
			}
		}

		int dist2 = app->combat->tileDistances[9];
		if (entity != nullptr) {
			dist2 = entity->distFrom(this->viewX, this->viewY);
		}
		if (n6 != 0) {
			this->setState(Canvas::ST_LOOTING);
			this->poolLoot(entity->calcPosition());
			return true;
		}
		int n12 = weapon2 * 9;
		if (entity != nullptr && entity->def->eType == 10 && (1 << entity->def->eSubType & 0x1) == 0x0 && app->combat->WorldDistToTileDist(dist2) > app->combat->weapons[n12 + 3]) {
			entity = nullptr;
		}
		if (entity2 != nullptr && (entity == nullptr || (1 << weapon2 & 0x0) != 0x0 || (entity->def->eType != 2 && entity->def->eType != 9))) {
			entity = entity2;
		}

		if (entity != nullptr && entity->def->eType == 10 && entity->def->eSubType == 2) {
			if (dist2 <= app->combat->tileDistances[0]) {
				entity->param = app->upTimeMs + 200;
				app->game->unlinkEntity(entity);
			}
			else if (weapon2 == 11) {}
		}

		int flagForFacingDir = this->flagForFacingDir(4);
		int n13 = this->destX + this->viewStepX >> 6;
		int n14 = this->destY + this->viewStepY >> 6;
		if (app->game->executeTile(n13, n14, flagForFacingDir, true)) {
			if (!app->game->skipAdvanceTurn && this->state == Canvas::ST_PLAYING) {
				app->game->touchTile(this->destX, this->destY, false);
				app->game->snapMonsters(true);
				app->game->advanceTurn();
			}
		}
		else if (entity != nullptr && entity->def->eType == 10 && entity->def->eSubType == 3 && dist2 <= app->combat->tileDistances[0] && app->player->ammo[8] == 0) {
			if (!app->player->isFamiliar) {
				if (app->player->ce->weapon == 2 && app->player->ammo[3] < 100) {
					app->hud->addMessage((short)248);
					app->player->ammo[3] = 100;
					app->player->showHelp((short)14, false);
					app->sound->playSound(1046, 0, 3, 0);
				}
				else if (app->player->ce->getStat(4) < 11) {
					app->hud->addMessage((short)238, 3);
				}
				else {
					app->player->currentWeaponCopy = app->player->ce->weapon;
					app->player->setPickUpWeapon(entity->def->tileIndex);
					app->player->give(2, 8, 1, true);
					app->player->giveAmmoWeapon(14, true);
					this->turnEntityIntoWaterSpout(entity);
					int* calcPosition = entity->calcPosition();
					if (this->shouldFakeCombat(calcPosition[0] >> 6, calcPosition[1] >> 6, flagForFacingDir) && app->combat->explodeThread != nullptr) {
						app->combat->explodeThread->run();
						app->combat->explodeThread = nullptr;
					}
					app->sound->playSound(1134, 0, 3, 0);
				}
				return true;
			}
		}
		else {
			if (entity != nullptr && entity->def->eType == 14 && entity->def->eSubType == 7 && dist2 <= app->combat->tileDistances[0] && dist2 > 0 && app->player->ce->weapon == 2) {
				if (app->player->ammo[3] < 100) {
					app->hud->addMessage((short)248);
					app->player->showHelp((short)14, false);
					app->sound->playSound(1046, 0, 3, 0);
				}
				else {
					app->hud->addMessage((short)249);
				}
				app->player->ammo[3] = 100;
				return true;
			}
			if (entity != nullptr && entity->def->eType == 5 && dist2 <= app->combat->tileDistances[0] && weapon2 != 14) {
				if (!app->player->isFamiliar) {
					if (entity->def->eSubType == 1) {
						app->hud->addMessage((short)44, 2);
					}
					else {
						app->game->performDoorEvent(0, entity, 1);
						app->game->advanceTurn();
					}
				}
				else {
					app->sound->playSound(1111, 0, 3, 0);
					app->hud->addMessage((short)193, 2);
				}
				return true;
			}
			if (entity != nullptr && entity->def->eType == 12 && dist2 <= app->combat->tileDistances[0] && (app->render->mapFlags[n14 * 32 + n13] & 0x4) != 0x0) {
				if (app->game->performDoorEvent(0, entity, 1, true)) {
					app->game->awardSecret(true);
				}
				return true;
			}
			if (entity != nullptr && (entity->def->eType == 0 || entity->def->eType == 12) && dist2 <= app->combat->tileDistances[0]) {
				if (this->isZoomedIn) {
					return true;
				}

				if (app->player->isFamiliar) {
					app->sound->playSound(1111, 0, 3, 0);
				}
				else if (app->player->characterChoice == 1) {
					app->sound->playSound(1073, 0, 3, 0);
				}
				else if (app->player->characterChoice >= 1 && app->player->characterChoice <= 3) {
					app->sound->playSound(1072, 0, 3, 0);
				}

				app->combat->shiftWeapon(true);
				this->pushedWall = true;
				this->pushedTime = app->gameTime + 500;
				app->render->rockView(1000, this->viewX + (this->viewStepX >> 6) * 2, this->viewY + (this->viewStepY >> 6) * 2, this->viewZ);
				return true;
			}
			else {
				if (app->player->isFamiliar && (app->player->familiarType == 2 || app->player->familiarType == 4)) {
					app->player->startSelfDestructDialog();
					return true;
				}
				if (!app->player->isFamiliar && app->player->weaponIsASentryBot(weapon2)) {
					app->player->attemptToDeploySentryBot();
					return true;
				}
				if (entity != nullptr && entity->def->eType == Enums::ET_ENV_DAMAGE) {
					this->shouldFakeCombat(app->game->traceCollisionX >> 6, app->game->traceCollisionY >> 6, flagForFacingDir);
					app->player->fireWeapon(entity, app->game->traceCollisionX, app->game->traceCollisionY);
					app->game->removeEntity(entity);
					app->player->addXP(5);
				}
				else {
					if (!this->isZoomedIn && Entity::CheckWeaponMask(weapon2, 512) != 0x0 && app->player->ammo[app->combat->weapons[n12 + 4]] > 0) {
						this->initZoom();
						return true;
					}
					if (entity != nullptr && entity->def->eType != 0) {
						int* calcPosition2 = entity->calcPosition();
						bool shouldFakeCombat = this->shouldFakeCombat(calcPosition2[0] >> 6, calcPosition2[1] >> 6, flagForFacingDir);
						int n15 = weapon2 * 9;
						if (shouldFakeCombat || (app->combat->weapons[n15 + 3] == 1 && app->combat->weapons[n15 + 2] == 1 && dist2 > app->combat->tileDistances[0])) {
							app->game->traceCollisionX = calcPosition2[0];
							app->game->traceCollisionY = calcPosition2[1];
							app->player->fireWeapon(&app->game->entities[0], calcPosition2[0], calcPosition2[1]);
						}
						else {
							if (this->isZoomedIn) {
								this->zoomCollisionX = this->viewX + (n4 * (n8 - this->viewX) >> 14);
								this->zoomCollisionY = this->viewY + (n4 * (n9 - this->viewY) >> 14);
								this->zoomCollisionZ = this->viewZ + (n4 * (n10 - this->viewZ) >> 14);
							}
							app->player->fireWeapon(entity, calcPosition2[0], calcPosition2[1]);
							if (app->player->inTargetPractice) {
								if (app->player->ammo[1] == 0) {
									app->player->exitTargetPractice();
								}
								else {
									app->player->assessTargetPracticeShot(entity);
								}
							}
						}
					}
					else {
						this->shouldFakeCombat(app->game->traceCollisionX >> 6, app->game->traceCollisionY >> 6, flagForFacingDir);
						app->player->fireWeapon(&app->game->entities[0], app->game->traceCollisionX, app->game->traceCollisionY);
						if (app->player->inTargetPractice) {
							if (app->player->ammo[1] == 0) {
								app->player->exitTargetPractice();
							}
							else {
								app->hud->addMessage((short)68);
							}
						}
					}
				}
			}
		}
	}
	else if (action == Enums::ACTION_PASSTURN) {
		app->hud->addMessage((short)45);
		app->game->touchTile(this->destX, this->destY, false);
		app->game->advanceTurn();
		this->invalidateRect();
	}

	return this->endOfHandlePlayingEvent(action, b);
}

bool Canvas::handleCinematicInput(int action) { // J2ME
	Applet* app = CAppContainer::getInstance()->app;
	if (action == Enums::ACTION_FIRE) {
		app->game->executeTile(this->destX >> 6, this->destY >> 6, app->game->eventFlags[1], true);
	}
	return false;
}

bool Canvas::shouldFakeCombat(int n, int n2, int n3) {
	Applet* app = CAppContainer::getInstance()->app;
	bool b = false;
	int n4 = app->player->ce->weapon * 9;
	if (app->combat->weapons[n4 + 4] != 0) {
		short n5 = app->player->ammo[app->combat->weapons[n4 + 4]];
		if (app->combat->weapons[n4 + 5] > 0 && n5 - app->combat->weapons[n4 + 5] < 0) {
			return false;
		}
	}
	n3 |= this->flagForWeapon(app->player->ce->weapon);
	if (app->game->doesScriptExist(n, n2, n3)) {
		(app->combat->explodeThread = app->game->allocScriptThread())->queueTile(n, n2, n3);
		b = true;
	}
	return b;
}

bool Canvas::endOfHandlePlayingEvent(int action, bool b) {
	Applet* app = CAppContainer::getInstance()->app;
	if ((action == Enums::ACTION_STRAFELEFT || action == Enums::ACTION_STRAFERIGHT || action == Enums::ACTION_LEFT || action == Enums::ACTION_RIGHT) && (this->viewX != this->destX || this->viewY != this->destY || this->viewAngle != this->destAngle)) {
		app->player->facingEntity = nullptr;
	}
	return true;
}

bool Canvas::handleEvent(int key) {
	Applet* app = CAppContainer::getInstance()->app;

	int state = this->state;
	int keyAction = this->getKeyAction(key);
	int fadeFlags = app->render->getFadeFlags();

	
	//printf("handleEvent key: %d keyAction: %d\n", key, keyAction);
	//printf("this->state %d\n", state);

	if (key == 26)
		return true;

	if (key == 18) {
		switch (state)
		{
		case Canvas::ST_LOGO:
			app->shutdown();
			break;
		case Canvas::ST_MENU:
			if (app->menuSystem->menu == Menus::MENU_ENABLE_SOUNDS) {
				app->shutdown();
			}
			break;
		case Canvas::ST_INTRO_MOVIE:
			this->exitIntroMovie(true);
			app->shutdown();
			break;
		}
		return true;
	}

	if (this->state == Canvas::ST_MENU && app->menuSystem->changeValues) {
		if (app->menuSystem->changeSfxVolume) { // [GEC]
			if (keyAction == Enums::ACTION_RIGHT) {
				app->sound->volumeUp(10);
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
			else if (keyAction == Enums::ACTION_LEFT) {
				app->sound->volumeDown(10);
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
		}
		else if (app->menuSystem->changeMusicVolume) { // [GEC]
			if (keyAction == Enums::ACTION_RIGHT) {
				app->sound->musicVolumeUp(10);
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
			else if (keyAction == Enums::ACTION_LEFT) {
				app->sound->musicVolumeDown(10);
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
		}
		else if (app->menuSystem->changeButtonsAlpha) { // [GEC]
			if (keyAction == Enums::ACTION_RIGHT) {
				this->m_controlAlpha += 10;
				if (this->m_controlAlpha > 100) {
					this->m_controlAlpha = 100;
				}
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
			else if (keyAction == Enums::ACTION_LEFT) {
				this->m_controlAlpha -= 10;
				if (this->m_controlAlpha < 0) {
					this->m_controlAlpha = 0;
				}
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
		}
		else if (app->menuSystem->changeVibrationIntensity) { // [GEC]
			if (keyAction == Enums::ACTION_RIGHT) {
				gVibrationIntensity += 10;
				if (gVibrationIntensity > 100) {
					gVibrationIntensity = 100;
				}
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
			else if (keyAction == Enums::ACTION_LEFT) {
				gVibrationIntensity -= 10;
				if (gVibrationIntensity < 0) {
					gVibrationIntensity = 0;
				}
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
		}
		else if (app->menuSystem->changeDeadzone) { // [GEC]
			if (keyAction == Enums::ACTION_RIGHT) {
				gDeadZone += 5;
				if (gDeadZone > 100) {
					gDeadZone = 100;
				}
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
			else if (keyAction == Enums::ACTION_LEFT) {
				gDeadZone -= 5;
				if (gDeadZone < 0) {
					gDeadZone = 0;
				}
				app->menuSystem->soundClick();
				app->menuSystem->refresh();
				return true;
			}
		}
	}

#if 0 // IOS
	if (app->sound->allowSounds) {
		bool refresh = false;
		if (this->state == Canvas::ST_MENU && app->menuSystem->changeSfxVolume) {
			refresh = true;
		}
		if (key == 27) {
			app->sound->volumeUp(10);
			if (refresh) {
				app->menuSystem->refresh();
			}
			return true;
		}
		if (key == 28) {
			app->sound->volumeDown(10);
			if (refresh) {
				app->menuSystem->refresh();
			}
			return true;
		}
	}
#endif

	if (this->st_enabled) {
		this->st_enabled = false;
		this->renderOnly = false;
	}

	if (fadeFlags != 0 && (fadeFlags & 0x10) != 0x0) {
		return true;
	}

	if (state == Canvas::ST_ERROR) {
		if (keyAction == Enums::ACTION_FIRE || key == 18) {
			app->shutdown();
		}
	}
	else if (state == Canvas::ST_MENU) {
		app->menuSystem->handleMenuEvents(key, keyAction);
	}
	else if (state == Canvas::ST_CHARACTER_SELECTION) {
		this->handleCharacterSelectionInput(key, keyAction);
	}
	else if (state == Canvas::ST_INTRO) {
		this->handleStoryInput(key, keyAction);
	}
	else if (state == Canvas::ST_INTRO_MOVIE) {
		app->game->skipMovie = (keyAction == Enums::ACTION_FIRE);
	}
	else if (state == Canvas::ST_EPILOGUE) {
		if (key == 18) {
			this->disposeEpilogue();
		}
	}
	else if (state == Canvas::ST_DIALOG) {
		this->handleDialogEvents(key);
	}
	else if (state == Canvas::ST_MINI_GAME) { // [GEC] Restored from J2ME/BREW
		switch (this->stateVars[0]) {
			case 2: {
				app->hackingGame->handleInput(keyAction);
				break;
			}
			case 0: {
				app->sentryBotGame->handleInput(keyAction);
				break;
			}
			case 4: {
				app->vendingMachine->handleInput(keyAction);
				break;
			}
		}
	}
	else if (state == Canvas::ST_BENCHMARK || state == Canvas::ST_BENCHMARKDONE) { // [GEC] Restored from J2ME/BREW
		this->setState(Canvas::ST_PLAYING);
		this->setAnimFrames(this->animFrames);
	}
	else if (state == Canvas::ST_AUTOMAP) {
		this->automapDrawn = false;
		return handlePlayingEvents(key, keyAction);
	}
	else if (state == Canvas::ST_PLAYING) {
		if (this->isZoomedIn) {
			return this->handleZoomEvents(key, keyAction);
		}
		return this->handlePlayingEvents(key, keyAction);
	}
	else if (state == Canvas::ST_COMBAT) {
		if (app->combat->curAttacker != nullptr && !this->isZoomedIn) {
			if (keyAction == Enums::ACTION_RIGHT) {
				this->destAngle -= 256;
			}
			else if (keyAction == Enums::ACTION_LEFT) {
				this->destAngle += 256;
			}
			this->startRotation(false);
		}
		if (this->combatDone && !app->game->interpolatingMonsters) {
			this->setState(Canvas::ST_PLAYING);
			if (app->combat->curAttacker == nullptr) {
				app->game->advanceTurn();
				if (state == Canvas::ST_PLAYING) {
					if (this->isZoomedIn) {
						return handleZoomEvents(key, keyAction);
					}
					return this->handlePlayingEvents(key, keyAction);
				}
			}
		}
	}
	else if (state == Canvas::ST_CREDITS) {
		if (this->endingGame) {
			if ((keyAction == Enums::ACTION_FIRE && this->scrollingTextDone) || key == 18) {
				this->endingGame = false;
				app->sound->soundStop();
				app->menuSystem->imgMainBG->~Image();
				app->menuSystem->imgLogo->~Image();
				app->menuSystem->imgMainBG = app->loadImage(Resources::RES_LOGO_BMP_GZ, true);
				app->menuSystem->imgLogo = app->loadImage(Resources::RES_LOGO2_BMP_GZ, true);
				app->menuSystem->background = app->menuSystem->imgMainBG;
				app->menuSystem->setMenu(Menus::MENU_END_);
			}
		}
		else if (keyAction == Enums::ACTION_BACK || keyAction == Enums::ACTION_FIRE) {
			if (this->loadMapID == 0) {
				int n2 = 3;
				if (app->game->hasSavedState()) {
					++n2;
				}
				app->menuSystem->pushMenu(3, n2, 0, 0, 0);
				app->menuSystem->setMenu(Menus::MENU_MAIN_HELP);
			}
			else {
				app->sound->soundStop();
				app->menuSystem->pushMenu(29, 7, 0, 0, 0);
				app->menuSystem->setMenu(Menus::MENU_INGAME_HELP);
			}
		}
	}
	else if (state == Canvas::ST_CAMERA) {
		if (!this->changeMapStarted && app->gameTime > app->game->cinUnpauseTime && (keyAction == Enums::ACTION_PASSTURN || keyAction == Enums::ACTION_AUTOMAP || keyAction == Enums::ACTION_FIRE || key == 18)) {
			app->game->skipCinematic();
		}
	}
	else if (state == Canvas::ST_DYING) {
		if (this->stateVars[0] > 0 && (keyAction == Enums::ACTION_FIRE || key == 18)) {
			app->menuSystem->setMenu(Menus::MENU_INGAME_DEAD);
		}
	}
	else if (state == Canvas::ST_MIXING) {
		// No implementado
	}
	else if (state == Canvas::ST_TRAVELMAP) {
		this->handleTravelMapInput(key, keyAction);
	}
	else if (state == Canvas::ST_LOOTING) {
		this->handleLootingEvents(keyAction);
	}
	else if (state == Canvas::ST_TREADMILL) {
		this->handleTreadmillEvents(keyAction);
	}
	else {
		return false;
	}

	return true;
}

void Canvas::runInputEvents() {
	Applet* app = CAppContainer::getInstance()->app;

	while (this->blockInputTime == 0) {
		if (this->ignoreFrameInput > 0) {
			this->numEvents = 0;
			this->ignoreFrameInput--;
			return;
		}
		if (this->numEvents == 0) {
			return;
		}

		
		int ev = this->events[0];
		//printf("this->events[0] %d\n", ev);
		this->st_fields[11] = app->upTimeMs;
		if (!this->handleEvent(ev)) {
			return;
		}

		if (this->numEvents > 0) {
			this->numEvents--;
		}
	}

	if (app->gameTime > this->blockInputTime) {
		if (this->state == Canvas::ST_PLAYING) {
			this->drawPlayingSoftKeys();
		}
		this->blockInputTime = 0;
	}
	this->clearEvents(1);
}

bool Canvas::loadMedia() {
	Applet* app = CAppContainer::getInstance()->app;
	printf("Canvas::loadMedia\n");

	//printf("Canvas::isLoaded %d\n", Canvas::isLoaded);
	if (Canvas::isLoaded == false) {
		this->updateLoadingBar(Canvas::isLoaded);
		this->drawLoadingBar(&this->graphics);
		Canvas::isLoaded = true;
		
		return false;
	}
	Canvas::isLoaded = false;

	bool allowSounds = app->sound->allowSounds;
	app->canvas->inInitMap = true;
	app->sound->allowSounds = false;
	this->mediaLoading = true;
	bool displaySoftKeys = this->displaySoftKeys;
	this->updateLoadingBar(false);
	this->unloadMedia();
	this->displaySoftKeys = false;
	this->isZoomedIn = false;
	app->StopAccelerometer();
	app->tinyGL->resetViewPort();

	for (int i = 0; i < 128; ++i) {
		if (i != 15) {
			app->game->scriptStateVars[i] = 0;
		}
	}

	if (!app->render->beginLoadMap(this->loadMapID)) {
		return false;
	}

	if (this->loadMapID <= 10 && this->loadMapID > app->player->highestMap) {
		app->player->highestMap = this->loadMapID;
	}

	if (app->game->isSaved) {
		this->setLoadingBarText((short)0, (short)38);
	}
	else if (app->game->isLoaded) {
		this->setLoadingBarText((short)0, (short)39);
	}
	else if (this->loadType != 3) {
		this->setLoadingBarText((short)3, (short)(app->render->mapNameField & 0x3FF));
	}

	this->updateLoadingBar(false);
	app->render->mapMemoryUsage -= 1000000000;
	//app->checkPeakMemory("after map loaded");
	app->game->loadMapEntities();
	app->hud->msgCount = 0;

	this->updateLoadingBar(false);
	app->player->playTime = app->gameTime;
	app->game->curLevelTime = app->gameTime;
	this->clearEvents(1);
	app->particleSystem->freeAllParticles();
	this->displaySoftKeys = displaySoftKeys;
	app->player->levelInit();

	this->updateLoadingBar(false);
	app->game->loadWorldState();

	this->updateLoadingBar(false);
	app->game->spawnPlayer();
	this->knockbackDist = 0;
	if (!app->game->isLoaded && this->loadType != 3) {
		app->game->saveLevelSnapshot();
	}

	this->updateLoadingBar(false);
	//printf("app->game->isLoaded %d\n", app->game->isLoaded);
	//printf("this->loadType %d\n", this->loadType);
	if (app->game->isLoaded || app->game->hasSavedState() || this->loadType == 3) {
		this->loadRuntimeData();
		//app->checkPeakMemory("after loadRuntimeData");
	}
	else {
		app->game->saveState(this->loadMapID, this->loadMapID, this->destX, this->destY, this->destAngle, this->destPitch, this->destX, this->destY, this->saveX, this->saveY, this->saveZ, this->saveAngle, this->savePitch, 3);
	}

	this->updateLoadingBar(false);
	app->player->selectWeapon(app->player->ce->weapon);
	app->game->scriptStateVars[12] = app->game->difficulty;
	app->game->executeStaticFunc(0);

	this->updateLoadingBar(false);
	if (app->player->gameCompleted) {
		app->game->executeStaticFunc(1);
	}

	if (!app->game->isLoaded) {
		this->prevX = this->destX;
		this->prevY = this->destY;
		app->game->executeTile(this->viewX >> 6, this->viewY >> 6, 4081, 1);
		this->finishRotation(false);
		this->dequeueHelpDialog(true);
	}
	this->finishRotation(false);

	app->game->endMonstersTurn();
	this->uncoverAutomap();
	this->updateLoadingBar(false);
	app->game->isSaved = (app->game->isLoaded = false);
	app->game->activeLoadType = 0;
	this->dequeueHelpDialog(true);
	if (this->state == 0) {
		this->setState(Canvas::ST_PLAYING);
	}
	app->game->pauseGameTime = false;
	app->lastTime = (app->time = app->upTimeMs);
	this->blockInputTime = app->gameTime + 200;
	app->sound->allowSounds = allowSounds;
	this->inInitMap = false;
	this->mediaLoading = false;
	this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, 0, 290);

	if (this->state != Canvas::ST_CAMERA) {
		this->repaintFlags |= Canvas::REPAINT_HUD;
	}
	this->repaintFlags |= Canvas::REPAINT_SOFTKEYS;

	if (this->state == Canvas::ST_PLAYING) {
		this->drawPlayingSoftKeys();
	}

	for (int i = 0; i < 18; i++) {
		app->render->monsterIdleTime[i] = ((app->nextByte() % 10) * 1000) + app->time + 12000;
	}

	if (this->state == Canvas::ST_LOADING && !this->loadType) {
		this->setState(Canvas::ST_PLAYING);
	}

	return true;
}

void Canvas::combatState() {
	Applet* app = CAppContainer::getInstance()->app;

	app->game->monsterLerp();
	app->game->updateLerpSprites();
	if (this->combatDone) {
		if (!app->game->interpolatingMonsters) {
			if (app->combat->curAttacker == nullptr) {
				app->game->advanceTurn();
			}
			if (!app->game->isCameraActive()) {
				this->setState(Canvas::ST_PLAYING);
			}
			else {
				this->setState(Canvas::ST_CAMERA);
				app->game->activeCamera->cameraThread->run();
			}
		}
	}
	else if (app->combat->runFrame() == 0) {
		if (this->state == Canvas::ST_DYING || this->state == Canvas::ST_BOT_DYING) {
			while (app->game->combatMonsters != nullptr) {
				app->game->combatMonsters->undoAttack();
			}
			return;
		}
		if (app->combat->curAttacker == nullptr) {
			app->game->touchTile(this->destX, this->destY, false);
			this->combatDone = true;
		}
		else if (this->knockbackDist == 0) {
			Entity* curAttacker = app->combat->curAttacker;
			if ((curAttacker->monster->goalFlags & 0x8) != 0x0) {
				curAttacker->monster->resetGoal();
				curAttacker->monster->goalType = 5;
				curAttacker->monster->goalParam = 1;
				curAttacker->aiThink(false);
			}
			Entity* nextAttacker;
			Entity* nextAttacker2;
			for (nextAttacker = curAttacker->monster->nextAttacker; nextAttacker != nullptr && nextAttacker->monster->target == nullptr && !nextAttacker->aiIsAttackValid(); nextAttacker = nextAttacker2) {
				nextAttacker2 = nextAttacker->monster->nextAttacker;
				nextAttacker->undoAttack();
			}
			if (nextAttacker != nullptr) {
				app->combat->performAttack(nextAttacker, nextAttacker->monster->target, 0, 0, false);
			}
			else {
				app->game->combatMonsters = nullptr;
				if (app->game->interpolatingMonsters) {
					this->setState(Canvas::ST_PLAYING);
				}
				else {
					app->game->endMonstersTurn();
					this->drawPlayingSoftKeys();
					this->combatDone = true;
				}
			}
		}
	}
	this->updateView();
	this->repaintFlags |= Canvas::REPAINT_PARTICLES;
	app->hud->repaintFlags |= 0x2B; // J2ME 0x6B
	if (!app->game->isCameraActive()) {
		this->repaintFlags |= Canvas::REPAINT_HUD;
	}
}

void Canvas::dialogState(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->dialogBuffer != nullptr && this->dialogBuffer->length() == 0) {
		return;
	}

	this->m_dialogButtons->GetButton(1)->drawButton = false;
	this->m_dialogButtons->GetButton(2)->drawButton = false;
	this->m_dialogButtons->GetButton(3)->drawButton = false;
	this->m_dialogButtons->GetButton(4)->drawButton = false;
	this->m_dialogButtons->GetButton(5)->drawButton = false;
	this->m_dialogButtons->GetButton(6)->drawButton = false;
	this->m_dialogButtons->GetButton(7)->drawButton = false;

	int* dialogRect = this->dialogRect;
	dialogRect[0] = -this->screenRect[0];
	dialogRect[2] = this->hudRect[2];
	dialogRect[3] = this->dialogViewLines * 16 + 8;
	dialogRect[1] = 320 - dialogRect[3] - 1;//Canvas.screenRect[3] - dialogRect[3] - 1;
	this->dialogTypeLineIdx = this->numDialogLines;
	int n = dialogRect[0] + 1;
	int n2 = 0xFF000000;
	int color = 0xFFFFFFFF;
	int color2 = 0xFF666666;
	switch (this->dialogStyle) {
		case 3: {
			n = -this->screenRect[0] + 1;
			dialogRect[1] = 320 - dialogRect[3] - 10;//Canvas.hudRect[3] - dialogRect[3] - 10;
			//this->drawScroll(graphics, dialogRect[0], dialogRect[1] - 10, this->hudRect[2], dialogRect[3] + 20);
			graphics->fillRect(dialogRect[0], dialogRect[1] - 10, this->hudRect[2], this->dialogRect[3] + 20, 12800);
			graphics->setColor(color);
			graphics->drawRect(dialogRect[0], dialogRect[1] - 10, dialogRect[2] - 1, dialogRect[3] + 19);
			break;
		}
		case 16: {
			color2 = 0xFF000066;
			break;
		}
		case 4: {
			if ((this->dialogFlags & 0x1) != 0x0) {
				n2 = 0xFFB18A01;
				break;
			}
			n2 = 0xFF005A00;
			break;
		}
		case 11: {
			n2 = 0xFF800000;
			if ((this->dialogFlags & 0x2) != 0x0) {
				dialogRect[1] = this->hudRect[1] + 20;
				break;
			}
			break;
		}
		case 5: {
			n2 = 0xFF800000;
			if ((this->dialogFlags & 0x2) != 0x0) {
				dialogRect[1] = this->hudRect[1] + 20;
				break;
			}
			break;
		}
		case 8: {
			dialogRect[1] -= 64;
			n2 = Canvas::PLAYER_DLG_COLOR;
			break;
		}
		case 14: {
			n2 = 0xFF002864;
			dialogRect[1] -= 20;
		}
		case 1:
		case 6: {
			n2 = 0xFF002864;
			break;
		}
		case 9: {
			n2 = 0xFF000000;
			color2 = 0xFF000000;
			break;
		}
		case 10: {
			n2 = 0xFF2E0854;
			dialogRect[1] = this->hudRect[1] + 20;
			break;
		}
		case 12: {
			n2 = 0xFFB18A01;
			break;
		}
		case 13: {
			n2 = 0xFFB18A01;
			break;
		}
		case 15: {
			n2 = 0xFFFF9600;
			break;
		}
	}

	if ((this->dialogFlags & 4) != 0 || (this->dialogFlags & 1) != 0) {
		this->m_dialogButtons->GetButton(3)->drawButton = true;
		this->m_dialogButtons->GetButton(4)->drawButton = true;
	}
	else
	{
		this->m_dialogButtons->GetButton(3)->drawButton = false;
		assert(m_dialogButtons->GetButton(4));
		//__symbol_stub4::___assert_rtn("dialogState","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/Canvas.cpp", 0x1499,"m_dialogButtons->GetButton(4)");
		
		this->m_dialogButtons->GetButton(4)->drawButton = false;
	}

	int currentDialogLine = 0;
	if (this->dialogStyle == 2 || this->dialogStyle == 16 || this->dialogStyle == 9) {
		currentDialogLine = 1;
		graphics->setColor(n2);
		graphics->fillRect(dialogRect[0], dialogRect[1], dialogRect[2], dialogRect[3]);
		graphics->setColor(color2);
		graphics->fillRect(dialogRect[0], dialogRect[1] - 18, dialogRect[2], 18);
		graphics->setColor(color);
		graphics->drawRect(dialogRect[0], dialogRect[1] - 18, dialogRect[2] - 1, 18);
		graphics->drawRect(dialogRect[0], dialogRect[1], dialogRect[2] - 1, dialogRect[3]);

		this->m_dialogButtons->GetButton(8)->drawButton = true;
		fmButton* Button = this->m_dialogButtons->GetButton(8);
		int fontHeight = Applet::FONT_HEIGHT[app->fontType];
		Button->SetTouchArea(*dialogRect, dialogRect[1] - fontHeight - 2, dialogRect[2], fontHeight + dialogRect[3] + 2);

		if (this->specialLootIcon != -1) {
			graphics->drawRegion(app->hud->imgActions, 0, 18 * this->specialLootIcon, 18, 18, dialogRect[0], dialogRect[1] - 17, 0, 0, 0);
			graphics->drawRegion(app->hud->imgActions, 0, 18 * this->specialLootIcon, 18, 18, dialogRect[2] - 18, dialogRect[1] - 17, 0, 0, 0);
		}
		if (this->dialogStyle == 9) {
			this->graphics.currentCharColor = 2;
		}
		graphics->drawString(this->dialogBuffer, this->SCR_CX, dialogRect[1] - 16, 1, this->dialogIndexes[0], this->dialogIndexes[1]);
	}
	else if (this->dialogStyle == 4) {
		graphics->setColor(n2);
		graphics->fillRect(dialogRect[0], dialogRect[1], dialogRect[2], dialogRect[3]);
		graphics->setColor(color);
		graphics->drawRect(dialogRect[0], dialogRect[1], dialogRect[2] - 1, dialogRect[3]);
		if (this->dialogItem != nullptr) {
			currentDialogLine = 1;
			graphics->setColor(n2);
			graphics->fillRect(dialogRect[0], dialogRect[1] - 12, dialogRect[2], 12);
			graphics->setColor(color);
			graphics->drawRect(dialogRect[0], dialogRect[1] - 12, dialogRect[2] - 1, 12);
			graphics->drawString(this->dialogBuffer, dialogRect[0] + (dialogRect[2] + 10 - 2 >> 1), dialogRect[1] - 5, 3, this->dialogIndexes[0], this->dialogIndexes[1]);
			this->m_dialogButtons->GetButton(8)->drawButton = true;
			fmButton* Button = this->m_dialogButtons->GetButton(8);
			Button->SetTouchArea(*dialogRect, dialogRect[1] - 12, dialogRect[2], dialogRect[3] + 12);
		}
		else {
			this->m_dialogButtons->GetButton(8)->drawButton = true;
			fmButton* Button = this->m_dialogButtons->GetButton(8);
			Button->SetTouchArea(*dialogRect, dialogRect[1], dialogRect[2], dialogRect[3]);
		}
	}
	else if (this->dialogStyle != 3) {
		graphics->setColor(n2);
		graphics->fillRect(dialogRect[0], dialogRect[1], dialogRect[2], dialogRect[3]);
		graphics->setColor(color);
		graphics->drawRect(dialogRect[0], dialogRect[1], dialogRect[2] - 1, dialogRect[3]);
		if (this->dialogStyle == 8) {
			int n6;
			int n5 = n6 = dialogRect[1] + 1;
			int n7 = n6 + (dialogRect[3] - 1);
			while (++n5 < n7) {
				int n8 = 96 + ((256 - (n5 - n6 << 8) / (n7 - n6)) * 160 >> 8);
				graphics->setColor((((n2 & 0xFF00FF00) >> 8) * n8 & 0xFF00FF00) | ((n2 & 0xFF00FF) * n8 >> 8 & 0xFF00FF) & 0xDE);
				graphics->drawLine(dialogRect[0] + 1, n5, dialogRect[0] + (dialogRect[2] - 2), n5);
			}
			graphics->drawRegion(this->imgUIImages, 30, 0, 15, 9, this->SCR_CX + 10, dialogRect[1] + dialogRect[3], 0, 0, 0);
			int width = app->hud->imgPortraitsSM->width;
			int n9 = app->hud->imgPortraitsSM->height / 3;
			int n10 = 0;
			switch (app->player->characterChoice) {
			case 1: {
				n10 = 0;
				break;
			}
			case 2: {
				n10 = 1;
				break;
			}
			case 3: {
				n10 = 2;
				break;
			}
			}
			graphics->drawRegion(app->hud->imgPortraitsSM, 0, n9 * n10, width, n9, dialogRect[0] + 2, dialogRect[1] + 3, 0, 0, 0);
			n += app->hud->imgPortraitsSM->width;
			n += 2;
		}
		else if (this->dialogStyle == 5) {
			if ((this->dialogFlags & 0x2) != 0x0) {
				graphics->drawRegion(this->imgUIImages, 0, 12, 10, 6, this->SCR_CX - 64, dialogRect[1] + dialogRect[3] + 6, 36, 0, 0);
			}
			else {
				graphics->drawRegion(this->imgUIImages, 0, 0, 10, 6, this->SCR_CX - 64, dialogRect[1] + 1, 36, 0, 0);
			}
		}
		else if (this->dialogStyle == 1) {
			graphics->drawRegion(this->imgUIImages, 10, 0, 10, 6, this->SCR_CX - 64, dialogRect[1] + 1, 36, 0, 0);
		}
		else if (this->dialogStyle == 10) {
			graphics->drawRegion(this->imgUIImages, 20, 6, 10, 6, this->SCR_CX + 10, dialogRect[1] + dialogRect[3], 0, 0, 0);
		}
		else if (this->dialogStyle == 14) {
			graphics->drawRegion(this->imgUIImages, 45, 0, 15, 9, this->SCR_CX + 10, dialogRect[1] + dialogRect[3], 0, 0, 0);
		}
	}
	if (this->currentDialogLine < currentDialogLine) {
		this->currentDialogLine = currentDialogLine;
	}
	int n11 = dialogRect[1] + 2;
	for (int n12 = 0; n12 < this->dialogViewLines && this->currentDialogLine + n12 < this->numDialogLines; ++n12) {
		short n13 = this->dialogIndexes[(this->currentDialogLine + n12) * 2];
		short n14 = this->dialogIndexes[(this->currentDialogLine + n12) * 2 + 1];
		int n15 = 0;
		if (n12 == this->dialogTypeLineIdx) {
			n15 = (app->time - this->dialogLineStartTime) / 25;
			if (n15 >= n14) {
				n15 = n14;
				++this->dialogTypeLineIdx;
				this->dialogLineStartTime = app->time;
			}
		}
		else if (n12 < this->dialogTypeLineIdx) {
			n15 = n14;
		}
		if (this->dialogStyle == 9) {
			this->graphics.currentCharColor = 2;
		}
		graphics->drawString(this->dialogBuffer, n, n11, 0, n13, n15);
		n11 += 16;
	}
	int8_t b = this->OSC_CYCLE[app->time / 200 % 4];
	short n16 = app->game->scriptStateVars[4];
	if ((this->dialogFlags & 0x2) != 0x0) {
		int y = this->screenRect[3] - 214;
		int x = this->screenRect[0] + 3;

		Text* smallBuffer = app->localization->getSmallBuffer();
		Text* smallBuffer2 = app->localization->getSmallBuffer();
		if ((this->dialogFlags & 0x8) != 0x0) {
			app->localization->composeText((short)0, (short)214, smallBuffer); // VIOS_MALLOC
			app->localization->composeText((short)0, (short)215, smallBuffer2); // VIOS_DELETE
		}
		else {
			app->localization->composeText((short)0, (short)141, smallBuffer); // YES_LABEL
			app->localization->composeText((short)0, (short)140, smallBuffer2); // NO_LABEL
		}
		smallBuffer->dehyphenate();
		smallBuffer2->dehyphenate();

		int strX = x + 4; // Old x + 8
		int strY = y + 9; // Old y + 1
		int strH = 32;
		int strW = std::max(smallBuffer->getStringWidth(), smallBuffer2->getStringWidth());
		strW += 20;// strW += 18

		//------------------------------------------------------------------------
		this->m_dialogButtons->GetButton(0)->drawButton = true;
		int hColor = (this->m_dialogButtons->GetButton(0)->highlighted) ? 0xFF8A8A8A : 0xFF4A4A4A;
		graphics->fillRect(x, y, strW, strH, hColor);
		graphics->drawRect(x, y, strW, strH, -1);
		graphics->drawString(smallBuffer, strX, strY, 4);
		this->m_dialogButtons->GetButton(0)->SetTouchArea(x, y, strW, (strH - 5));

		if (n16 == 0) { // J2ME/BREW
			graphics->drawCursor(strX + strW + b - 8, strY, 0x18, false);
		}

		//------------------------------------------------------------------------
		strY = y + 73; // Old y + 71
		this->m_dialogButtons->GetButton(1)->drawButton = true;
		hColor = (this->m_dialogButtons->GetButton(1)->highlighted) ? 0xFF8A8A8A : 0xFF4A4A4A;
		graphics->fillRect(x, y + 64, strW, strH, hColor);
		graphics->drawRect(x, y + 64, strW, strH, -1);
		graphics->drawString(smallBuffer2, strX, strY, 4);
		this->m_dialogButtons->GetButton(1)->SetTouchArea(x, y + 64, strW, (strH - 5));

		if (n16 == 1) {// J2ME/BREW
			graphics->drawCursor(strX + strW + b - 8, strY, 0x18, false);
		}
		//------------------------------------------------------------------------
		smallBuffer->dispose();
		smallBuffer2->dispose();
	}
	else if (this->dialogFlags != 0 && this->currentDialogLine >= this->numDialogLines - this->dialogViewLines) {
		short n19 = 30;
		short n20 = 31;
		Text* smallBuffer3 = app->localization->getSmallBuffer();
		if ((this->dialogFlags & 0x1) != 0x0) {
			n19 = 140;
			n20 = 141;
		}
		if ((this->dialogFlags & 0x4) != 0x0 || (this->dialogFlags & 0x1) != 0x0) {
			int n21 = this->dialogRect[1] + (16 * this->dialogViewLines) - 14;

			int v75 = Applet::FONT_HEIGHT[app->fontType] + 2;
			int v113 = Applet::FONT_HEIGHT[app->fontType];

			//------------------------------------------------------------------------
			smallBuffer3->setLength(0);
			app->localization->composeText((short)0, n19, smallBuffer3);
			smallBuffer3->dehyphenate();
			this->m_dialogButtons->GetButton(3)->drawButton = true;
			int hColor = (this->m_dialogButtons->GetButton(3)->highlighted) ? ((n2 + 0x333333) | 0xFF000000) : n2;
			graphics->fillRect(96, n21, 96, v75, hColor);
			graphics->drawRect(96, n21, 96, v75, -1);
			graphics->drawString(smallBuffer3, 144, n21 + (v75 >> 1) + 2, 3);
			this->m_dialogButtons->GetButton(3)->SetTouchArea(96, n21 - 40, 96, v113 + 42);

			if (n16 == 0) { // J2ME/BREW
				graphics->drawCursor((128 - (smallBuffer3->getStringWidth() >> 1)) + b, n21 + (v113 >> 1) - 5, 0);
			}
			//------------------------------------------------------------------------
			smallBuffer3->setLength(0);
			app->localization->composeText((short)0, n20, smallBuffer3);
			smallBuffer3->dehyphenate();
			this->m_dialogButtons->GetButton(4)->drawButton = true;
			hColor = (this->m_dialogButtons->GetButton(4)->highlighted) ? ((n2 + 0x333333) | 0xFF000000) : n2;
			graphics->fillRect(288, n21, 96, v75, hColor);
			graphics->drawRect(288, n21, 96, v75, -1);
			graphics->drawString(smallBuffer3, 336, n21 + (v75 >> 1) + 2, 3);
			this->m_dialogButtons->GetButton(4)->SetTouchArea(288, n21 - 40, 96, v113 + 42);

			if (n16 == 1) { // J2ME/BREW
				graphics->drawCursor((320 - (smallBuffer3->getStringWidth() >> 1)) + b, n21 + (v113 >> 1) - 5, 4);
			}
		}
		smallBuffer3->dispose();
	}

	if (this->numDialogLines <= this->dialogViewLines) {
		if (!(this->dialogFlags & 2) && !(this->dialogFlags & 4) && !(this->dialogFlags & 1)) {
			this->m_dialogButtons->GetButton(7)->drawButton = true;
			this->m_dialogButtons->GetButton(7)->Render(graphics);
		}
	}
	else {
		int numDialogLines;
		if (this->currentDialogLine + this->dialogViewLines > this->numDialogLines) {
			numDialogLines = this->numDialogLines;
		}
		else {
			numDialogLines = this->currentDialogLine + this->dialogViewLines;
		}

		this->drawScrollBar(graphics, dialogRect[0] + dialogRect[2] - 1, dialogRect[1] + 2, dialogRect[3] - 4, this->currentDialogLine - currentDialogLine, numDialogLines - currentDialogLine, this->numDialogLines - currentDialogLine, this->dialogViewLines);

		if (this->numDialogLines - currentDialogLine > this->dialogViewLines) {
			if (this->currentDialogLine > 1) {
				this->m_dialogButtons->GetButton(5)->drawButton = true;
				this->m_dialogButtons->GetButton(5)->Render(graphics);
			}
			if (this->currentDialogLine < this->numDialogLines - this->dialogViewLines) {
				this->m_dialogButtons->GetButton(6)->drawButton = true;
				this->m_dialogButtons->GetButton(6)->Render(graphics);
			}
			else {
				this->m_dialogButtons->GetButton(7)->drawButton = true;
				this->m_dialogButtons->GetButton(7)->Render(graphics);
			}
		}
		else {
			this->m_dialogButtons->GetButton(7)->drawButton = true;
			this->m_dialogButtons->GetButton(7)->Render(graphics);
		}
	}

	this->m_dialogButtons->GetButton(8)->drawButton = true;
	this->m_dialogButtons->GetButton(8)->Render(graphics);
	this->clearLeftSoftKey();

#if 0
	if (this->dialogFlags > this->dialogViewLines) {
		int numDialogLines;
		if (this->currentDialogLine + this->dialogViewLines > this->numDialogLines) {
			numDialogLines = this->numDialogLines;
		}
		else {
			numDialogLines = this->currentDialogLine + this->dialogViewLines;
		}
		if (this->dialogStyle == 3) {
			this->drawScrollBar(graphics, dialogRect[0] + dialogRect[2] - 1, dialogRect[1] - 8, dialogRect[3] + 16, this->currentDialogLine - currentDialogLine, numDialogLines - currentDialogLine, this->numDialogLines - currentDialogLine, this->dialogViewLines);
		}
		else {
			this->drawScrollBar(graphics, dialogRect[0] + dialogRect[2] - 1, dialogRect[1] + 2, dialogRect[3] - 4, this->currentDialogLine - currentDialogLine, numDialogLines - currentDialogLine, this->numDialogLines - currentDialogLine, this->dialogViewLines);
		}
	}
	if (this->currentDialogLine > currentDialogLine) {
		this->setLeftSoftKey((short)0, (short)125);
	}
	else {
		//this->clearLeftSoftKey();
	}
#endif
}

void Canvas::automapState() {
	Applet* app = CAppContainer::getInstance()->app;
	app->game->updateLerpSprites();
	if (!this->automapDrawn && app->game->animatingEffects == 0) {
		this->updateView();
		this->repaintFlags &= ~Canvas::REPAINT_VIEW3D;
		if (this->state != Canvas::ST_AUTOMAP) {
			this->updateView();
		}
	}
	if (this->state == Canvas::ST_AUTOMAP || this->state == Canvas::ST_PLAYING) {
		this->drawPlayingSoftKeys();
	}
}

void Canvas::renderOnlyState() {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->st_enabled) {
		this->viewAngle = (this->viewAngle + this->animAngle & 0x3FF);
		this->viewPitch = 0;
	}
	else {
		if (this->viewX == this->destX && this->viewY == this->destY && this->viewAngle == this->destAngle) {
			return;
		}
		if (this->viewX < this->destX) {
			this->viewX += this->animPos;
			if (this->viewX > this->destX) {
				this->viewX = this->destX;
			}
		}
		else if (this->viewX > this->destX) {
			this->viewX -= this->animPos;
			if (this->viewX < this->destX) {
				this->viewX = this->destX;
			}
		}
		if (this->viewY < this->destY) {
			this->viewY += this->animPos;
			if (this->viewY >this->destY) {
				this->viewY =this->destY;
			}
		}
		else if (this->viewY > this->destY) {
			this->viewY -= this->animPos;
			if (this->viewY < this->destY) {
				this->viewY = this->destY;
			}
		}
		if (this->viewZ < this->destZ) {
			++this->viewZ;
		}
		else if (this->viewZ > this->destZ) {
			--this->viewZ;
		}
		if (this->viewAngle < this->destAngle) {
			this->viewAngle += this->animAngle;
			if (this->viewAngle > this->destAngle) {
				this->viewAngle = this->destAngle;
			}
		}
		else if (this->viewAngle > this->destAngle) {
			this->viewAngle -= this->animAngle;
			if (this->viewAngle < this->destAngle) {
				this->viewAngle = this->destAngle;
			}
		}
		if (this->viewPitch < this->destPitch) {
			this->viewPitch += this->pitchStep;
			if (this->viewPitch > this->destPitch) {
				this->viewPitch = this->destPitch;
			}
		}
		else if (this->viewPitch > this->destPitch) {
			this->viewPitch -= this->pitchStep;
			if (this->viewPitch <this->destPitch) {
				this->viewPitch =this->destPitch;
			}
		}
	}
	this->lastFrameTime = app->time;
	app->render->render((this->viewX << 4) + 8, (this->viewY << 4) + 8, (this->viewZ << 4) + 8, this->viewAngle, 0, 0, 290);
	app->combat->drawWeapon(0, 0);
	this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_VIEW3D);
}

void Canvas::playingState() {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->pushedWall && this->pushedTime <= app->gameTime) {
		app->combat->shiftWeapon(false);
		this->pushedWall = false;
	}
	if (app->player->ce->getStat(0) <= 0) {
		app->player->died();
		return;
	}
	if (app->player->isFamiliar && app->player->ammo[7] <= 0) {
		app->player->ammo[7] = 0;
		app->player->familiarDying(false);
	}
	if (app->hud->isShiftingCenterMsg()) {
		this->staleView = true;
	}
	if (this->knockbackDist == 0 && app->game->activePropogators == 0 && app->game->animatingEffects == 0 && app->game->monstersTurn != 0 && this->numHelpMessages == 0) {
		app->game->updateMonsters();
	}
	app->game->updateLerpSprites();
	this->updateView();
	if (this->state == Canvas::ST_LOADING || this->state == Canvas::ST_SAVING) {
		return;
	}
	if (this->state != Canvas::ST_PLAYING && this->state != Canvas::ST_INTER_CAMERA) {
		return;
	}
	this->repaintFlags |= Canvas::REPAINT_PARTICLES;
	if (!app->game->isCameraActive() || this->state == Canvas::ST_INTER_CAMERA) {
		this->repaintFlags |= Canvas::REPAINT_HUD;
		app->hud->repaintFlags |= 0x2B; // J2ME 0x6B
		app->hud->update();
	}
	if (this->state == Canvas::ST_INTER_CAMERA || (!app->game->isCameraActive() && this->state == Canvas::ST_PLAYING) || this->state == Canvas::ST_AUTOMAP) {
		this->dequeueHelpDialog();
	}
}

void Canvas::menuState() {
	Applet* app = CAppContainer::getInstance()->app;

	short n = -1;
	int menu = app->menuSystem->menu;
	if ((app->menuSystem->items[app->menuSystem->selectedIndex].flags & 0x20)) {
		n = 49;
	}
	else {
		if (menu == Menus::MENU_END_RANKING || menu == Menus::MENU_LEVEL_STATS) {
			n = 43;
		}
		else if (app->menuSystem->type != 5) {
			if (menu != Menus::MENU_SHOWDETAILS) {
				if (menu == Menus::MENU_VENDING_MACHINE_DETAILS || menu == Menus::MENU_VENDING_MACHINE_CONFIRM) {
					n = 202;
				}
				else if (menu != Menus::MENU_VENDING_MACHINE_CANT_BUY) {
					if (app->menuSystem->items[app->menuSystem->selectedIndex].action != 0) {
						n = 121;
					}
				}
			}
		}
	}

	if (menu != Menus::MENU_MAIN_MORE_GAMES) {
		this->clearSoftKeys();
		if (app->menuSystem->getStackCount() != 0 || menu == Menus::MENU_MAIN_MINIGAME) {
			if (app->menuSystem->peekMenu() != 25) {
				this->setLeftSoftKey((short)3, (short)80);
			}
		}
		else if (menu == Menus::MENU_INGAME || menu == Menus::MENU_INGAME_KICKING || menu == Menus::MENU_INGAME_SNIPER) {
			this->setLeftSoftKey((short)0, (short)30);
		}
		else if (menu == Menus::MENU_VENDING_MACHINE) {
			this->setLeftSoftKey((short)3, (short)80);
		}
		if (n != -1) {
			if (!app->menuSystem->changeValues) { // Old changeSfxVolume
				this->setRightSoftKey((short)0, n);
			}
		}
		else if (app->menuSystem->menu == Menus::MENU_SHOWDETAILS) {
			this->setRightSoftKey((short)0, (short)40);
		}
	}

	this->repaintFlags |= Canvas::REPAINT_MENU;
}

void Canvas::dyingState() {
	Applet* app = CAppContainer::getInstance()->app;
	app->hud->repaintFlags = 32;
	if (app->time < this->deathTime + 750) {
		int n = (750 - (app->time - this->deathTime) << 16) / 750;
		this->viewZ = app->render->getHeight(this->destX, this->destY) + 18 + (20 * n >> 16);
		this->viewPitch = 96 + (-96 * n >> 16);
		int n2 = 16 + (-16 * n >> 16);
		this->updateView();
		this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, n2, 290);
		this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
	}
	else if (app->time < this->deathTime + 2750) {
		if (!app->render->isFading()) {
			app->render->startFade(2000, 1);
		}
		this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, 16, 290);
		this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
	}
	else {
		app->render->baseDizzy = (app->render->destDizzy = 0);
		app->menuSystem->setMenu(Menus::MENU_INGAME_DEAD);
	}
}

void Canvas::familiarDyingState() {
	Applet* app = CAppContainer::getInstance()->app;
	app->hud->repaintFlags = 36;
	// app->hud->repaintFlags |= 0x40; // J2ME
	if (this->familiarSelfDestructed) {
		this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, this->viewRoll, 290);
		this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
		if (app->time >= this->familiarDeathTime + 1500) {
			app->sound->playSound(1032, 0, 3, 0);
			this->setState(Canvas::ST_PLAYING);
			app->player->familiarDied();
		}
		else if (!this->selfDestructScreenShakeStarted && app->time >= this->familiarDeathTime + 750) {
			this->selfDestructScreenShakeStarted = true;
			this->startShake(this->familiarDeathTime + 1500 - app->time, 5, 500);
		}
	}
	else if (app->time < this->familiarDeathTime + 750) {
		int n = (750 - (app->time - this->familiarDeathTime) << 16) / 750;
		this->viewZ = app->render->getHeight(this->destX, this->destY) + 18 + (20 * n >> 16);
		this->viewPitch = 96 + (-96 * n >> 16);
		int n2 = 16 + (-16 * n >> 16);
		this->updateView();
		this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, n2, 290);
		this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
	}
	else if (app->time < this->familiarDeathTime + 1500) {
		if (!app->render->isFading()) {
			app->render->startFade(750, 1);
		}
		this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, 16, 290);
		this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
	}
	else {
		this->setState(Canvas::ST_PLAYING);
		app->player->familiarDied();
	}
}

void Canvas::logoState() {
	Applet* app = CAppContainer::getInstance()->app;

	if (!app->sound->soundsLoaded) {
		app->sound->cacheSounds();
	}

	if (this->pacLogoTime <= 120) {
		this->pacLogoTime++;
		if (this->imgStartupLogo == nullptr) {
			this->imgStartupLogo = app->loadImage("l2.bmp", true);
		}
		this->repaintFlags |= Canvas::REPAINT_STARTUP_LOGO;
	}
	else {
		this->imgStartupLogo->~Image();
		this->imgStartupLogo = nullptr;

		this->setState(Canvas::ST_INTRO_MOVIE);
		this->numEvents = 0;
		this->keyDown = false;
		this->keyDownCausedMove = false;
		this->ignoreFrameInput = 1;
	}
}

void Canvas::drawScrollBar(Graphics* graphics, int i, int i2, int i3, int i4, int i5, int i6, int i7)
{
	bool v9; // cc
	int v12; // r1
	Canvas* v14; // [sp+20h] [bp-24h]
	int v15; // [sp+24h] [bp-20h]
	int v16; // [sp+28h] [bp-1Ch]

	v14 = this;
	v9 = i7 < 0;
	if (i7)
		v9 = i7 < i6;
	if (v9)
	{
		v12 = i6 - i7;
		if (i6 - i7 < i4)
			v12 = i4;
		v15 = 3 * i3 / (4 * ((i7 + i6 - 1) / i7));
		v16 = ((i4 << 16) / (v12 << 8) * ((i3 - v15 - 14) << 8)) >> 16;
		if (i6 == i5)
			v16 = i3 - 3 * i3 / (4 * ((i7 + i6 - 1) / i7)) - 14;
		graphics->drawRegion(this->imgUIImages, 60, 0, 7, 7, i, i2, 24, 0, 0);
		graphics->drawRegion(v14->imgUIImages, 60, 7, 7, 7, i, i3 + i2, 40, 0, 0);
		graphics->setColor(-5002605);
		graphics->fillRect(i - 7, i2 + 7, 7, i3 - 14);
		graphics->setColor(-1585235);
		graphics->fillRect(i - 7, v16 + 7 + i2, 7, v15);
		graphics->setColor(-16777216);
		graphics->drawRect(i - 7, v16 + 7 + i2, 6, v15 - 1);
		graphics->drawRect(i - 7, i2, 6, i3 - 1);
	}
}

void Canvas::uncoverAutomap() {
	Applet* app = CAppContainer::getInstance()->app;
	if (!app->game->updateAutomap) {
		return;
	}
	int n = this->destX >> 6;
	int n2 = this->destY >> 6;
	if (n < 0 || n >= 32 || n2 < 0 || n2 >= 32) {
		return;
	}
	for (int i = n2 - 1; i <= n2 + 1; ++i) {
		if (i >= 0) {
			if (i < 31) {
				for (int j = n - 1; j <= n + 1; ++j) {
					if (j >= 0) {
						if (j < 31) {
							uint8_t b = app->render->mapFlags[i * 32 + j];
							if ((j == n && i == n2) || (b & 0x2) == 0x0) {
								app->render->mapFlags[i * 32 + j] |= (uint8_t)128;
							}
						}
					}
				}
			}
		}
	}
}

void Canvas::drawAutomap(Graphics* graphics, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	graphics->drawRegion(this->imgGameHelpBG, 0, 0, 480, 320, 0, 0, 0, 0, 0);

	
	int n3 = 8;
	int n6 = 0x400000 / (n3 << 8);
	int n4 = 112;
	int n5 = 32;

	int n7 = 0;
	for (int i = 0; i < 32; ++i) {
		for (int j = 0; j < 32; ++j) {
			uint8_t b2 = app->render->mapFlags[n7 + j];
			if ((b2 & 0x8) == 0x0) {
				if (app->render->mapEntranceAutomap == (i * 32) + j) {
					graphics->setColor(0xFF00FF00);
					graphics->fillRect(n4 + (n3 * j) + this->screenRect[0], (n5 - 1) + (n3 * i) + this->screenRect[1], n3, n3);
				}
				else if ((b2 & 0x80) != 0x0) {
					if (app->render->mapExitAutomap == (i * 32) + j) {
						graphics->setColor(0xFFFF0000);
						graphics->fillRect(n4 + (n3 * j) + this->screenRect[0], (n5 - 1) + (n3 * i) + this->screenRect[1], n3, n3);
					}
					else if ((b2 & 0x1) == 0x0) {
						graphics->setColor(0xFF90B9E7);
						graphics->fillRect(n4 + (n3 * j) + this->screenRect[0], (n5 - 1) + (n3 * i) + this->screenRect[1] + 1, n3, n3);
						graphics->setColor(0xFF2A3657);
					}
				}
			}

		}
		n7 += 32;
	}

	for (int k = 0; k < Render::MAX_LADDERS_PER_MAP; ++k) {
		if (app->render->mapLadders[k] >= 0 && app->render->mapLadders[k] < 1024 && 
			(app->render->mapFlags[app->render->mapLadders[k]] & 0x80) != 0x0 && 
			(app->render->mapFlags[app->render->mapLadders[k]] & 0x8) == 0x0) {
			int n8 = app->render->mapLadders[k] % 32;
			int n9 = app->render->mapLadders[k] / 32;
			graphics->setColor(0xFFFFFF00);
			graphics->fillRect(n4 + (n3 * n8), n5 + (n3 * n9), n3, n3);
			graphics->setColor(0xFF000000);
			for (int l = 0; l < 8; l += 2) {
				graphics->drawLine(n4 + n3 * n8, n5 + n3 * n9 + l, n4 + n3 * n8 + n3, n5 + n3 * n9 + l);
			}
			graphics->drawLine(n4 + n3 * n8, n5 + n3 * n9, n4 + n3 * n8, n5 + n3 * n9 + n3);
			graphics->drawLine(n4 + n3 * n8 + n3, n5 + n3 * n9, n4 + n3 * n8 + n3, n5 + n3 * n9 + n3);
		}
	}

	graphics->setColor(0xFF2A3657);
	for (int n10 = 0; n10 < app->render->numLines; ++n10) {
		int n11 = app->render->lineFlags[n10 >> 1] >> ((n10 & 0x1) << 2) & 0xF;
		if ((n11 & 0x8) != 0x0) {
			int n12 = n11 & 0x7;
			if (n12 == 6 || n12 == 0) {
				graphics->drawLine(n4 + app->render->lineXs[n10 << 1], n5 + app->render->lineYs[n10 << 1], n4 + app->render->lineXs[(n10 << 1) + 1], n5 + app->render->lineYs[(n10 << 1) + 1]);
			}
		}
	}

	int n13 = 0;
	for (int n14 = 0; n14 < 32; ++n14) {
		for (int n15 = 0; n15 < 32; ++n15) {
			uint8_t b3 = app->render->mapFlags[n13 + n15];
			for (Entity* nextOnTile = app->game->entityDb[n13 + n15]; nextOnTile != nullptr; nextOnTile = nextOnTile->nextOnTile) {
				if (nextOnTile != &app->game->entities[1]) {
					if (nextOnTile != &app->game->entities[0]) {
						if ((b3 & 0x8) == 0x0) {
							int sprite = nextOnTile->getSprite();
							int n16 = app->render->mapSpriteInfo[sprite];
							int n17 = (n16 & 0xFF00) >> 8;
							if (0x0 != (n16 & 0x200000)) {
								if (0x0 == (n16 & 0x10000)) {
									int color = 0;
									bool b4 = false;
									bool b5 = false;
									int n18 = 2;
									if (nextOnTile->def->eType == 5) {
										short tileIndex = nextOnTile->def->tileIndex;
										if (tileIndex == 271 || tileIndex == 272) {
											color = 0xFFFF8400;
										}
										else if (tileIndex == 273 || tileIndex == 274) {
											color = 0xFF00C0FF;
										}
										else {
											color = 0xFF3D68E3;
										}
										b5 = true;
									}
									else if ((n16 & 0x400000) != 0x0) {
										color = 0xFF2A3657;
										b5 = true;
									}
									else if (nextOnTile->def->eType == 13) {
										color = 0xFF8D8068;
										b5 = true;
									}
									else if (nextOnTile->def->eType == 3) {
										b4 = true;
										color = 0xFF0000FF;
										n18 = 128;
									}
									else if (nextOnTile->def->eType == 10 && n17 == 0) {
										color = 0xFF8000FF;
									}
									else if (nextOnTile->def->eType == 2) {
										color = 0xFFFF8000;
										n18 = 128;
									}
									else if (nextOnTile->def->eType == 7) {
										color = 0xFF8D8068;
										if (nextOnTile->def->tileIndex == 173 || nextOnTile->def->tileIndex == 180) {
											color = 0;
										}
									}
									else if (app->player->god && nextOnTile->def->eType == 6 && nextOnTile->def->eSubType != 3) {
										color = 0xFF00FFEA;
									}
									if (color != 0 && ((b3 & n18) != 0x0 || (n18 & 0x80) == 0x0)) {
										graphics->setColor(color);
										if ((n16 & 0xF000000) != 0x0) {
											int n20;
											int n19 = n20 = app->render->mapSprites[app->render->S_X + sprite];
											int n22;
											int n21 = n22 = app->render->mapSprites[app->render->S_Y + sprite];
											if ((n16 & 0x3000000) != 0x0) {
												n20 -= 32;
												n19 += 32;
											}
											else {
												n22 -= 32;
												n21 += 32;
											}
											int n23 = ((n20 << 16) / n6) + 1 + 128 >> 8;
											int n24 = ((n19 << 16) / n6) + 128 >> 8;
											int n25 = ((n22 << 16) / n6) + 128 >> 8;
											int n26 = ((n21 << 16) / n6) + 128 >> 8;
											if (b5) {
												graphics->drawLine(n4 + n23, n5 + n25, n4 + n24, n5 + n26);
											}
											else {
												graphics->fillRect(n4 + (n23 + n24 >> 1) - (n3 / 4), n5 + (n25 + n26 >> 1) - (n3 / 4), n3 / 2, n3 / 2);
											}
										}
										else if (b4) {
											graphics->fillRect(n4 + (n3 * n15) + (n3 / 4), n5 + (n3 * n14) + (n3 / 4), (n3 / 2) + 2, (n3 / 2) + 2);
										}
										else {
											graphics->fillRect(n4 + (n3 * n15) + (n3 / 4) + 1, n5 + (n3 * n14) + (n3 / 4) + 1, (n3 / 2), (n3 / 2));
										}
									}
								}
							}
						}
					}
				}
			}
		}
		n13 += 32;
	}

#if 0
	int n27 = 0;
	for (int n28 = 0; n28 < 32; ++n28) {
		for (int n29 = 0; n29 < 32; ++n29) {
			if ((app->render->mapFlags[n27 + n29] & 0x8) != 0x0) {
				graphics->setColor(0xFF526F8B);
				graphics->fillRect(n4 + 8 * n29 + this->screenRect[0], n5 + 8 * n28 + this->screenRect[1] + 1, 8, 8);
			}
		}
		n27 += 32;
	}
#endif

	for (int n30 = 0; n30 < app->player->numNotebookIndexes; ++n30) {
		if (!app->player->isQuestDone(n30)) {
			if (!app->player->isQuestFailed(n30)) {
				int n31 = app->player->notebookPositions[n30] >> 5 & 0x1F;
				int n32 = app->player->notebookPositions[n30] & 0x1F;
				if (n31 + n32 != 0 && (0x80 & app->render->mapFlags[n32 * 32 + n31]) != 0x0) {
					graphics->setColor(((app->time / 1024 & 0x1) == 0x0) ? 0xFFFF0000 : 0xFF00FF00);
					Entity* mapEntity = app->game->findMapEntity(n31 << 6, n32 << 6, 32);
					if (nullptr != mapEntity) {
						int sprite2 = mapEntity->getSprite();
						int n33 = app->render->mapSpriteInfo[sprite2];
						int n35;
						int n34 = n35 = app->render->mapSprites[app->render->S_X + sprite2];
						int n37;
						int n36 = n37 = app->render->mapSprites[app->render->S_Y + sprite2];
						if ((n33 & 0x3000000) != 0x0) {
							n35 -= 32;
							n34 += 32;
						}
						else {
							n37 -= 32;
							n36 += 32;
						}
						graphics->drawLine(n4 + ((n35 << 16) / n6 + 1 + 128 >> 8), n5 + ((n37 << 16) / n6 + 128 >> 8), n4 + ((n34 << 16) / n6 + 128 >> 8), n5 + ((n36 << 16) / n6 + 128 >> 8));
					}
					else {
						graphics->fillRect(n4 + n3 * n31 + n3 / 4, n5 + n3 * n32 + n3 / 4, n3 / 2 + 2, n3 / 2 + 2);
					}
				}
			}
		}
	}

	// Test Only
#if 0
	for (int node = 0; node < app->render->numNodes; node++) {
		int x1 = n4 + (app->render->nodeBounds[(node << 2) + 0] & 0xFF) << 3;
		int y1 = n5 + (app->render->nodeBounds[(node << 2) + 1] & 0xFF) << 3;
		int x2 = n4 + (app->render->nodeBounds[(node << 2) + 2] & 0xFF) << 3;
		int y2 = n5 + (app->render->nodeBounds[(node << 2) + 3] & 0xFF) << 3;

		graphics->setColor(0xff00ff00);
		graphics->drawRect( (x1 / 8), (y1 / 8), ((x2 - x1 + 1) / 8), ((y2 - y1 + 1) / 8));
	}
#endif

	if (app->time > this->automapBlinkTime) {
		this->automapBlinkTime = app->time + 333;
		this->automapBlinkState = !this->automapBlinkState;
	}

	int n38 = 0;
	switch (this->destAngle & 0x3FF) {
	case Enums::ANGLE_NORTH: {
		n38 = 0;
		break;
	}
	case Enums::ANGLE_SOUTH: {
		n38 = 1;
		break;
	}
	case Enums::ANGLE_EAST: {
		n38 = 2;
		break;
	}
	case Enums::ANGLE_WEST: {
		n38 = 3;
		break;
	}
	}
	int n39 = n38 * 4;
	if (this->automapBlinkState) {
		n39 += 16;
	}
	int n40 = n4 + ((n3 * (this->viewX - 32)) / 64) + (n3 / 2);
	int n41 = n5 + ((n3 * (this->viewY - 32)) / 64) + (n3 / 2);
	if (n41 < this->screenRect[1] + this->screenRect[3]) {
		graphics->drawRegion(this->imgMapCursor, 0, n39, 4, 4, n40, n41, 3, 0, 0);
	}

	Text *LargeBuffer = app->localization->getLargeBuffer();

	LargeBuffer->setLength(0);
	app->localization->composeText(this->softKeyLeftID, LargeBuffer);
	LargeBuffer->dehyphenate();
	app->setFontRenderMode(2);
	if (this->m_softKeyButtons->GetButton(19)->highlighted) {
		app->setFontRenderMode(0);
	}
	graphics->drawString(LargeBuffer, 15, 310, 36); // old 20, 316, 36
	app->setFontRenderMode(0);

	LargeBuffer->setLength(0);
	app->localization->composeText(this->softKeyRightID, LargeBuffer);
	LargeBuffer->dehyphenate();
	app->setFontRenderMode(2);
	if (this->m_softKeyButtons->GetButton(20)->highlighted) {
		app->setFontRenderMode(0);
	}
	graphics->drawString(LargeBuffer, 465, 310, 40);// old 470, 316, 40
	app->setFontRenderMode(0);
	LargeBuffer->dispose();
}

void Canvas::closeDialog(bool skipDialog) {
	Applet* app = CAppContainer::getInstance()->app;

	this->dialogClosing = true;
	this->specialLootIcon = -1;
	this->showingLoot = false;
	app->player->unpause(app->time - this->dialogStartTime);
	this->dialogBuffer->dispose();
	this->dialogBuffer = nullptr;
	if (this->numHelpMessages == 0 && (this->dialogStyle == 3 || this->dialogStyle == 4 || (this->dialogStyle == 2 && this->dialogType == 1) || (this->dialogStyle == 12 && app->game->scriptStateVars[4] == 1 && !skipDialog))) {
		app->game->queueAdvanceTurn = true;
	}
	if (this->dialogStyle == 11 && (this->dialogFlags & 0x2) != 0x0) {
		if (app->game->scriptStateVars[4] == 0) {
			++app->game->numMallocsForVIOS;
		}
		else {
			app->game->angryVIOS = true;
		}
	}
	if (this->oldState == Canvas::ST_INTER_CAMERA) {
		app->game->activeCameraTime = app->gameTime - app->game->activeCameraTime;
		this->setState(Canvas::ST_INTER_CAMERA);
	}
	else if (app->game->isCameraActive()) {
		app->game->activeCameraTime = app->gameTime - app->game->activeCameraTime;
		this->setState(Canvas::ST_CAMERA);
	}
	else if (this->oldState == Canvas::ST_COMBAT && !this->combatDone) {
		this->setState(Canvas::ST_COMBAT);
	}
	else {
		this->setState(Canvas::ST_PLAYING);
	}
	if (this->dialogResumeMenu) {
		this->setState(Canvas::ST_MENU);
	}
	this->dialogClosing = false;
	if (this->dialogResumeScriptAfterClosed) {
		app->game->skipDialog = skipDialog;
		this->dialogThread->run();
		app->game->skipDialog = false;
	}
	if (this->dialogStyle == 12 && app->player->attemptingToSelfDestructFamiliar) {
		app->player->attemptingToSelfDestructFamiliar = false;
		if (app->game->scriptStateVars[4] == 1 && !skipDialog) {
			app->player->familiarDying(true);
		}
	}
	else if (this->dialogStyle == 13 && this->repairingArmor) {
		if (app->game->scriptStateVars[4] == 1 && !skipDialog) {
			if (app->player->inventory[12] >= 5) {
				app->player->give(0, 12, -5);
				app->player->give(0, 11, 1);
				app->sound->playSound(1054, 0, 3, 0);
				int modifyStat = app->player->modifyStat(3, 1);
				if (modifyStat > 0) {
					app->localization->resetTextArgs();
					app->localization->addTextArg(modifyStat);
					app->hud->addMessage((short)0, (short)212, 3);
				}
				else {
					app->hud->addMessage((short)0, (short)213, 3);
				}
			}
			else {
				if (app->player->characterChoice == 1) {
					app->sound->playSound(1073, 0, 3, 0);
				}
				else if (app->player->characterChoice >= 1 && app->player->characterChoice <= 3) {
					app->sound->playSound(1072, 0, 3, 0);
				}
				app->hud->addMessage((short)0, (short)210, 3);
			}
		}
		this->endArmorRepair();
	}
	this->repaintFlags |= Canvas::REPAINT_VIEW3D;
}

void Canvas::prepareDialog(Text* text, int dialogStyle, int dialogFlags) {
	Applet* app = CAppContainer::getInstance()->app;
	int i = 0;
	int n = 0;
	Text* smallBuffer = app->localization->getSmallBuffer();
	if (dialogStyle == 3) {
		this->dialogViewLines = 4;
	}
	else if (dialogStyle == 8) {
		this->dialogViewLines = 3;
	}
	else if (dialogStyle == 2) {
		this->dialogViewLines = 3;
	}
	else {
		this->dialogViewLines = 4;
	}
	if (dialogStyle == 1 || (dialogStyle == 5 && ((dialogFlags & 0x2) != 0x0 || (dialogFlags & 0x4) != 0x0))) {
		this->updateFacingEntity = true;
		Entity* facingEntity = app->player->facingEntity;
		if (facingEntity != nullptr && facingEntity->def != nullptr && (facingEntity->def->eType == 2 || facingEntity->def->eType == 3)) {
			app->combat->curTarget = facingEntity;
			int sprite = facingEntity->getSprite();
			if (facingEntity->def->eType == 2) {
				app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFF00FF) | 96 << 8);
			}
			app->game->scriptStateVars[4] = 0;
		}
	}
	if (this->dialogBuffer == nullptr) {
		this->dialogBuffer = app->localization->getLargeBuffer();
	}
	else {
		this->dialogBuffer->setLength(0);
	}
	if ((dialogFlags & 0x4) != 0x0 || (dialogFlags & 0x1) != 0x0) {
		if (dialogStyle == 12) {
			app->game->scriptStateVars[4] = 0;
		}
		else {
			app->game->scriptStateVars[4] = 1;
		}
		smallBuffer->setLength(0);
		app->localization->composeText((short)0, (short)50, smallBuffer);
		text->append(smallBuffer);
	}
	if (dialogStyle == 8) {
		int n2 = this->dialogMaxChars;
		//int n3 = Hud.imgPortraitsSM.getWidth() / 9 + 1;
		smallBuffer->setLength(0);
		smallBuffer->append("   ");
		int n3 = smallBuffer->length();
		for (int j = 0; j < 2; ++j) {
			Text* dialogBuffer = this->dialogBuffer;
			int length;
			for (int k = 0; k < text->length(); k += dialogBuffer->wrapText(length, n2 - n3, 1, '|')) {
				length = dialogBuffer->length();
				dialogBuffer->append(text, k);
			}
			if (j == 0) {
				if (dialogBuffer->getNumLines() <= 3) {
					break;
				}
				dialogBuffer->setLength(0);
				n2 = this->dialogWithBarMaxChars;
			}
		}
	}
	else if (dialogStyle == 3) {
		this->dialogBuffer->append(text);
		this->dialogBuffer->wrapText(this->scrollMaxChars);
		if (this->dialogBuffer->getNumLines() > this->dialogViewLines) {
			this->dialogBuffer->setLength(0);
			this->dialogBuffer->append(text);
			this->dialogBuffer->wrapText(this->scrollWithBarMaxChars);
		}
	}
	else {
		this->dialogBuffer->append(text);
		this->dialogBuffer->wrapText(this->dialogMaxChars);
		int numLines = this->dialogBuffer->getNumLines();
		if (dialogStyle == 2 || dialogStyle == 16 || dialogStyle == 9) {
			--numLines;
		}
		if (numLines > this->dialogViewLines) {
			this->dialogBuffer->setLength(0);
			this->dialogBuffer->append(text);
			this->dialogBuffer->wrapText(this->dialogWithBarMaxChars);
		}
	}
	int length2 = this->dialogBuffer->length();
	this->numDialogLines = 0;
	while (i < length2) {
		if (this->dialogBuffer->charAt(i) == '|') {
			this->dialogIndexes[this->numDialogLines * 2] = (short)n;
			this->dialogIndexes[this->numDialogLines * 2 + 1] = (short)(i - n);
			this->numDialogLines++;
			n = i + 1;
		}
		++i;
	}
	this->dialogIndexes[this->numDialogLines * 2] = (short)n;
	this->dialogIndexes[this->numDialogLines * 2 + 1] = (short)(length2 - n);
	this->numDialogLines++;
	this->currentDialogLine = 0;
	this->dialogLineStartTime = app->time;
	this->dialogTypeLineIdx = 0;
	this->dialogStartTime = app->time;
	this->dialogItem = nullptr;
	this->dialogFlags = dialogFlags;
	this->dialogStyle = dialogStyle;
	smallBuffer->dispose();

	if (dialogStyle == 2) {
		app->sound->playSound(1027, 0, 3, 0);
	}
}

void Canvas::startDialog(ScriptThread* scriptThread, short n, int n2, int n3) {
	this->startDialog(scriptThread, (short)0, n, n2, n3, false);
}

void Canvas::startDialog(ScriptThread* scriptThread, short n, short n2, int n3, int n4, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	Text* largeBuffer = app->localization->getLargeBuffer();
	app->localization->composeText(n, n2, largeBuffer);
	this->startDialog(scriptThread, largeBuffer, n3, n4, b);
	largeBuffer->dispose();
}

void Canvas::startDialog(ScriptThread* scriptThread, Text* text, int n, int n2) {
	this->startDialog(scriptThread, text, n, n2, false);
}

void Canvas::startDialog(ScriptThread* dialogThread, Text* text, int n, int n2, bool dialogResumeScriptAfterClosed) {
	this->dialogResumeScriptAfterClosed = dialogResumeScriptAfterClosed;
	this->dialogResumeMenu = false;
	this->dialogThread = dialogThread;
	this->readyWeaponSound = 0;
	this->prepareDialog(text, n, n2);
	this->setState(Canvas::ST_DIALOG);
}

void Canvas::renderScene(int viewX, int viewY, int viewZ, int viewAngle, int viewPitch, int viewRoll, int viewFov) {
	Applet* app = CAppContainer::getInstance()->app;

	{ // J2ME
		//this->staleView = true;
		//if (!this->staleView && (this->staleTime == 0 || app->time < this->staleTime)) {
			//return;
		//}
	}

	this->staleView = false;
	this->staleTime = 0;
	this->lastFrameTime = app->time;
	this->beforeRender = app->upTimeMs;
	app->render->render((viewX << 4) + 8, (viewY << 4) + 8, (viewZ << 4) + 8, viewAngle, viewPitch, viewRoll, viewFov);
	this->afterRender = app->upTimeMs;
	++this->renderSceneCount;
	app->render->renderPortal();
	if (!this->isZoomedIn) {
		app->combat->drawWeapon(this->shakeX, this->shakeY);
	}
	if (app->render->postProcessMode != 0) {
		app->render->postProcessView(&this->graphics);
	}

	this->repaintFlags |= Canvas::REPAINT_VIEW3D;
}

void Canvas::startSpeedTest(bool b) {
	this->renderOnly = true;
	this->st_enabled = true;
	this->st_count = 1;
	for (int i = 0; i < Canvas::SPD_NUM_FIELDS; ++i) {
		this->st_fields[i] = 0;
	}
	if (!b) {
		this->animAngle = 4;
		this->destAngle = this->viewAngle;
		this->setState(Canvas::ST_BENCHMARK);
	}
}

void Canvas::backToMain(bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	this->loadMapID = 0;
	app->freeRuntimeImages();
	app->player->reset();
	app->game->unloadMapData();
	app->render->unloadMap();
	app->render->endFade();

	app->menuSystem->imgMainBG->~Image();
	app->menuSystem->imgMainBG = app->loadImage("logo.bmp", true);

	if (b) {
		this->clearEvents(1);
		if (this->skipIntro) {
			app->player->reset();
			app->render->unloadMap();
			app->game->unloadMapData();
			this->loadMap(this->startupMap, true, false);
		}
		else {
			if (app->localization->selectLanguage) {
				app->menuSystem->clearStack();
				app->menuSystem->setMenu(Menus::MENU_SELECT_LANGUAGE);
			}
			else {
				app->menuSystem->setMenu(Menus::MENU_MAIN);
			}
		}
	}
	else {
		app->sound->playSound(1071, 1, 3, false);
		app->menuSystem->setMenu(Menus::MENU_MAIN);
	}
}

void Canvas::drawPlayingSoftKeys() {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->player->inTargetPractice) {
		this->setLeftSoftKey((short)0, (short)30);
		this->clearRightSoftKey();
	}
	else if (this->isZoomedIn) {
		this->setSoftKeys((short)0, (short)52, (short)0, (short)55);
	}
	else if (this->state == Canvas::ST_AUTOMAP) {
		this->setSoftKeys((short)0, (short)52, (short)0, (short)53);
	}
	else if (app->player->isFamiliar) {
		this->setSoftKeys((short)0, (short)52, (short)0, (short)216);
	}
	else if (app->player->weaponIsASentryBot(app->player->ce->weapon)) {
		this->setSoftKeys((short)0, (short)52, (short)0, (short)220);
	}
	else {
		this->setSoftKeys((short)0, (short)52, (short)0, (short)55);
	}
}

void Canvas::changeStoryPage(int i) {
	if (i < 0 && this->storyPage == 0) {
		if (this->state == Canvas::ST_INTRO) {
			this->setState(Canvas::ST_CHARACTER_SELECTION);
			this->dialogBuffer->dispose();
			this->dialogBuffer = nullptr;
		}
	}
	else {
		this->storyPage += i;
	}
}

void Canvas::drawStory(Graphics* graphics)
{
	Applet* app = CAppContainer::getInstance()->app;

	Text* this_00;
	Text* this_01;
	short i2;
	Text* text;

	if (this->storyPage < this->storyTotalPages) {
		graphics->drawImage(app->hackingGame->imgHelpScreenAssets, 0, 0, 0, 0, 0);

		this_00 = app->localization->getLargeBuffer();
		if ((this->state != Canvas::ST_EPILOGUE) || (0 < this->storyPage)) {
			this_00->setLength(0);
			app->localization->composeText(3, 80, this_00); // "Back"
			this_00->dehyphenate();
			app->setFontRenderMode(2);
			if (this->m_storyButtons->GetButton(0)->highlighted != false) {
				app->setFontRenderMode(0);
			}
			graphics->drawString(this_00, 17, 310, 36); // Old -> 2, 319, 36
			app->setFontRenderMode(0);
		}

		this_00->setLength(0);
		this_01 = app->localization->getSmallBuffer();
		if (this->storyPage < this->storyTotalPages + -1) {
			app->localization->composeText(0, 56, this_00); // more
			i2 = 40; // Skip
			text = this_01;
			this->m_storyButtons->GetButton(1)->touchAreaDrawing.x = 420; // [GEC], ajusta la posicion X de la caja de toque
			this->m_storyButtons->GetButton(1)->touchAreaDrawing.w = 60; // [GEC], ajusta el ancho de la caja de toque
		}
		else {
			i2 = 43; // Continue
			text = this_00;
			this->m_storyButtons->GetButton(1)->touchAreaDrawing.x = 380; // [GEC], ajusta la posicion X de la caja de toque
			this->m_storyButtons->GetButton(1)->touchAreaDrawing.w = 100; // [GEC], ajusta el ancho de la caja de toque
		}

		app->localization->composeText(0, i2, text);
		this_00->dehyphenate();
		this_01->dehyphenate();
		app->setFontRenderMode(2);
		if (this->m_storyButtons->GetButton(1)->highlighted != false) {
			app->setFontRenderMode(0);
		}
		graphics->drawString(this_00, 463, 310, 40); // Old -> 478, 319, 40);
		app->setFontRenderMode(0);

		app->setFontRenderMode(2);
		if (this->m_storyButtons->GetButton(2)->highlighted != false) {
			app->setFontRenderMode(0);
		}
		graphics->drawString(this_01, 463, 10, 8); // Old -> 478, 1, 8);

		app->setFontRenderMode(0);
		this_00->dispose();
		this_01->dispose();

		graphics->drawString(this->dialogBuffer, this->storyX, this->storyY, 21, 0, this->storyIndexes[0],
			this->storyIndexes[1] - this->storyIndexes[0]);
	}
}

int Canvas::getCharacterConstantByOrder(int i) {
	switch (i) {
	case 0: {
		return 1;
	}
	case 1: {
		return 3;
	}
	case 2: {
		return 2;
	}
	}
	return 0;
}

void Canvas::drawCharacterSelection(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	fmButton* button;
	Text* textBuff;
	Image* img;
	int textID;


	graphics->clipRect(0, 0, this->screenRect[2], this->screenRect[3]);
	graphics->drawImage(this->imgCharSelectionBG, 0, 0, 0, 0, 0);
	graphics->drawImage(this->imgTopBarFill, this->SCR_CX - this->imgTopBarFill->width / 2, 0, 0, 0, 0);

	textBuff = app->localization->getSmallBuffer();
	textBuff->setLength(0);

	switch (this->stateVars[0]) {
		case 1: {
			app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::CHARACTER_SELECT_MAJOR_NAME, textBuff);
			break;
		}
		case 3: {
			app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::CHARACTER_SELECT_SCIENTIST_NAME, textBuff);
			break;
		}
		case 2: {
			app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::CHARACTER_SELECT_SERGEANT_NAME, textBuff);
			break;
		}
	}

	textBuff->dehyphenate();
	graphics->drawString(textBuff, this->SCR_CX, 3, 1);

	int j = 152;
	for (int i = 0; i < ((this->stateVars[1] == 0) ? 3 : 1); i++) {
		bool b = this->getCharacterConstantByOrder(i) == this->stateVars[0] || this->stateVars[1] != 0;

		//iVar2 = j + -0xf;
		graphics->drawRegion(this->imgCharacterSelectionAssets, 0, 0, 31, 83, j - 15, 55, 0, 0, 0);
		graphics->drawRegion(this->imgCharacterSelectionAssets, 0, 0, 31, 83, j + 16, 55, 0, 4, 0);

		button = this->m_characterButtons->GetButton(i);
		if (button->highlighted)
		{
			graphics->fillRect(j - 15, 55, 62, 83, 0x2896ff);
			graphics->drawRegion(this->imgCharacterSelectionAssets, 31, 0, 27, 60, j - 11, 64, 0, 0, 0);
			graphics->drawRegion(this->imgCharacterSelectionAssets, 31, 0, 27, 60, j + 16, 64, 0, 4, 0);
		}

		button->SetTouchArea(j - 15, 55, 62, 83);

		if (i < 2 && this->stateVars[1] == 0) {
			for (int x = (j + 46); x < (j + 64); x += 6) {
				graphics->drawRegion(this->imgCharacterSelectionAssets, 51, 60, 6, 19, x, 81, 0, 0, 0);
			}
		}

		if (b != 0) {
			graphics->drawRegion(this->imgCharacterSelectionAssets, 31, 60, 20, 32, j - 3, 161, 0, 0, 0);
			graphics->drawRegion(this->imgCharacterSelectionAssets, 31, 60, 19, 32, j + 17, 161, 0, 4, 0);
		}

		switch ((this->stateVars[1] == 0) ? this->getCharacterConstantByOrder(i) : this->stateVars[0]) {
			case 1: {
				img = this->imgMajorMugs;
				textID = MenuStrings::CHARACTER_SELECT_MAJOR;
				this->graphics.currentCharColor = 5;
				break;
			}
			case 3: {
				img = this->imgScientistMugs;
				textID = MenuStrings::CHARACTER_SELECT_SCIENTIST;
				this->graphics.currentCharColor = 5;
				break;
			}
			case 2: {
				img = this->imgSargeMugs;
				textID = MenuStrings::CHARACTER_SELECT_SERGEANT;
				this->graphics.currentCharColor = 5;
				break;
			}
		}

		graphics->drawRegion(img, 0, 0, 0x20, 0x20, j, 0x4b, 0, 0, 0);
		textBuff->setLength(0);
		app->localization->composeText(Strings::FILE_MENUSTRINGS, textID, textBuff);
		textBuff->dehyphenate();
		graphics->drawString(textBuff, j + (img->width / 2), 142, 1);

		j += 75;
	}

	this->drawCharacterSelectionAvatar(this->stateVars[0], -10, 0x8c, graphics);
	this->drawCharacterSelectionStats(this->stateVars[0], textBuff, 0x16a, 0x73, graphics);

	textBuff->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::CHARACTER_SELECT_CONFIRM, textBuff);
	textBuff->wrapText(0x18, '\n');
	graphics->drawString(textBuff, this->SCR_CX, this->screenRect[3] - 85, 1);

	textBuff->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::BACK_ITEM, textBuff);
	button = this->m_characterButtons->GetButton(4);
	graphics->fillRect(this->SCR_CX - 85, this->screenRect[3] - 60, 70, 30, button->highlighted ? 0x2896ff : 0x646464);
	graphics->drawRect(this->SCR_CX - 85, this->screenRect[3] - 60, 70, 30);
	button->SetTouchArea(this->SCR_CX - 85, this->screenRect[3] - 60, 70, 30);
	graphics->drawString(textBuff, this->SCR_CX - 50, this->screenRect[3] - 50, 1);

	int8_t b = this->OSC_CYCLE[app->time / 100 % 4];

	// [GEC]
	if (this->stateVars[2] == 0 && this->stateVars[8] == 1) { // J2ME/BREW
		graphics->drawCursor((this->SCR_CX - 50 - 6) - (textBuff->getStringWidth() / 2) + b, this->screenRect[3] - 50, 0x18, true);
	}

	textBuff->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::YES_LABEL, textBuff);
	button = this->m_characterButtons->GetButton(3);
	graphics->fillRect(this->SCR_CX + 15, this->screenRect[3] - 60, 70, 30, button->highlighted ? 0x2896ff : 0x646464);
	graphics->drawRect(this->SCR_CX + 15, this->screenRect[3] - 60, 70, 30);
	button->SetTouchArea(this->SCR_CX + 15, this->screenRect[3] - 60, 70, 30);
	graphics->drawString(textBuff, this->SCR_CX + 50, this->screenRect[3] - 50, 1);

	// [GEC]
	if (this->stateVars[2] == 1 && this->stateVars[8] == 1) { // J2ME/BREW
		graphics->drawCursor((this->SCR_CX + 50 - 6) - (textBuff->getStringWidth() / 2) + b, this->screenRect[3] - 50, 0x18, true);
	}

	textBuff->dispose();
}

void Canvas::drawCharacterSelectionAvatar(int i, int x, int y, Graphics* graphics)
{
	Applet* app = CAppContainer::getInstance()->app;
	Image* troso, * legs;

	switch (i) {
	case 1: {
		legs = this->imgMajor_legs;
		troso = this->imgMajor_torso;
		break;
	}
	case 3: {
		legs = this->imgRiley_legs;
		troso = this->imgRiley_torso;
		break;
	}
	case 2: {
		legs = this->imgSarge_legs;
		troso = this->imgSarge_torso;
		break;
	}
	}

	graphics->drawImage(legs, x, y, 0, 0, 0);
	graphics->drawImage(troso, x, y - (app->time / 1000 & 1U), 0, 0, 0);
}

void Canvas::drawCharacterSelectionStats(int i, Text* text, int x, int y, Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	int defense;
	int strength;
	int accuracy;
	int agility;
	int iq;

	switch (i) {
	case 1: {
		defense = 8;
		strength = 9;
		accuracy = 97;
		agility = 12;
		iq = 110;
		break;
	}
	case 3: {
		defense = 8;
		strength = 8;
		accuracy = 87;
		agility = 6;
		iq = 150;
		break;
	}
	case 2: {
		defense = 12;
		strength = 14;
		accuracy = 92;
		agility = 6;
		iq = 100;
		break;
	}
	}

	if (app->game->difficulty == 2) {
		defense = 0;
	}

	graphics->drawImage(this->imgCharacter_select_stat_header, x - 2, y - 4, 0, 0, 0);
	graphics->drawImage(this->imgCharacter_select_stat_bar, x - 2, y + 14, 0, 0, 0);

	int yFix = -2; // [GEC] ajusta el texto

	text->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::DEFENSE_LABEL, text);
	text->dehyphenate();
	text->append(defense);
	this->graphics.currentCharColor = 5;
	graphics->drawString(text, x, y + 18 + yFix, 20);
	graphics->drawImage(this->imgCharacter_select_stat_bar, x - 2, y + 34, 0, 0, 0);

	text->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::STRENGTH_LABEL, text);
	text->dehyphenate();
	text->append(strength);
	this->graphics.currentCharColor = 5;
	graphics->drawString(text, x, y + 38 + yFix, 20);
	graphics->drawImage(this->imgCharacter_select_stat_bar, x - 2, y + 54, 0, 0, 0);

	text->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::ACCURACY_LABEL, text);
	text->dehyphenate();
	text->append(accuracy);
	this->graphics.currentCharColor = 5;
	graphics->drawString(text, x, y + 58 + yFix, 20);
	graphics->drawImage(this->imgCharacter_select_stat_bar, x - 2, y + 74, 0, 0, 0);

	text->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::AGILITY_LABEL, text);
	text->dehyphenate();
	text->append(agility);
	this->graphics.currentCharColor = 5;
	graphics->drawString(text, x, y + 78 + yFix, 20);
	graphics->drawImage(this->imgCharacter_select_stat_bar, x - 2, y + 94, 0, 0, 0);

	text->setLength(0);
	app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::IQ_LABEL, text);
	text->dehyphenate();
	text->append(iq);
	this->graphics.currentCharColor = 5;
	graphics->drawString(text, x, y + 98 + yFix, 20);
}

void Canvas::dequeueHelpDialog() {
	this->dequeueHelpDialog(false);
}

void Canvas::dequeueHelpDialog(bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->numHelpMessages == 0) {
		return;
	}
	if (this->state == Canvas::ST_DIALOG || this->dialogClosing) {
		return;
	}
	if (!b && this->state != Canvas::ST_PLAYING && this->state != Canvas::ST_INTER_CAMERA && app->game->monstersTurn == 0) {
		return;
	}
	if (app->game->secretActive) {
		return;
	}

	int n = 2;
	int n2 = 0;
	Text* largeBuffer = app->localization->getLargeBuffer();
	this->dialogType = this->helpMessageTypes[0];
	void* object = this->helpMessageObjs[0];
	short n3 = this->helpMessageThreads[0];
	if (this->dialogType == 1) {
		EntityDef* entityDef = (EntityDef*)object;
		uint8_t eSubType = entityDef->eSubType;
		short n4 = -1;
		if (eSubType == 0) {
			if (entityDef->parm >= 0 && entityDef->parm < 11) {
				n4 = 32;
			}
			else if ((entityDef->parm >= 16 && entityDef->parm < 18) || (entityDef->parm >= 11 && entityDef->parm < 13)) {
				n4 = 35;
			}
			else if (entityDef->parm == 18) {
				n4 = 34;
			}
		}
		else if (eSubType == 1) {
			n4 = 36;
		}
		else if (eSubType != 2) {
			app->Error(0); // ERR_DEQUEUEHELP
			return;
		}
		if (entityDef != nullptr) {
			app->localization->composeText((short)1, entityDef->longName, largeBuffer);
			largeBuffer->append("|");
			app->localization->composeText((short)1, entityDef->description, largeBuffer);
			if (n4 != -1) {
				largeBuffer->append(" ");
				app->localization->composeText((short)0, n4, largeBuffer);
			}
		}
	}
	else if (this->dialogType == 2) {
		int n5 = this->helpMessageInts[0];
		app->localization->composeText((short)(n5 >> 16), (short)(n5 & 0xFFFF), largeBuffer);
	}
	else if (this->dialogType == 3) {
		largeBuffer->dispose();
		largeBuffer = (Text*)object;
	}
	else {
		largeBuffer->dispose();
		largeBuffer = (Text*)object;
	}
	for (int i = 0; i < 15; ++i) {
		this->helpMessageTypes[i] = this->helpMessageTypes[i + 1];
		this->helpMessageInts[i] = this->helpMessageInts[i + 1];
		this->helpMessageObjs[i] = this->helpMessageObjs[i + 1];
		this->helpMessageThreads[i] = this->helpMessageThreads[i + 1];
	}
	this->helpMessageObjs[15] = object;
	this->numHelpMessages--;
	if (app->player->enableHelp) {
		if (n3 == -1) {
			this->startDialog(nullptr, largeBuffer, n, n2, false);
		}
		else {
			this->startDialog(&app->game->scriptThreads[n3], largeBuffer, n, n2, true);
		}
	}
	largeBuffer->dispose();
}

void Canvas::enqueueHelpDialog(short n) {
	this->enqueueHelpDialog((short)0, n, (uint8_t)(-1));
}

bool Canvas::enqueueHelpDialog(short n, short n2, uint8_t b) {
	Applet* app = CAppContainer::getInstance()->app;

	if (!app->player->enableHelp || this->state == Canvas::ST_DYING) {
		return false;
	}
	if (this->numHelpMessages == 16) {
		app->Error(41); // ERR_MAXHELP
		return false;
	}
	this->helpMessageTypes[this->numHelpMessages] = 2;
	this->helpMessageInts[this->numHelpMessages] = (n << 16 | n2);
	this->helpMessageObjs[this->numHelpMessages] = nullptr;
	this->helpMessageThreads[this->numHelpMessages] = b;
	this->numHelpMessages++;
	if (this->state == Canvas::ST_PLAYING) {
		this->dequeueHelpDialog();
	}
	return true;
}

bool Canvas::enqueueHelpDialog(Text* text) {
	return this->enqueueHelpDialog(text, 0);
}

bool Canvas::enqueueHelpDialog(Text* text, int n) {
	Applet* app = CAppContainer::getInstance()->app;

	if (!app->player->enableHelp || this->state == Canvas::ST_DYING) {
		return false;
	}
	if (this->numHelpMessages == 16) {
		app->Error(41); // ERR_MAXHELP
		return false;
	}
	this->helpMessageTypes[this->numHelpMessages] = n;
	this->helpMessageObjs[this->numHelpMessages] = text;
	this->helpMessageThreads[this->numHelpMessages] = -1;
	this->numHelpMessages++;
	if (this->state == Canvas::ST_PLAYING) {
		this->dequeueHelpDialog();
	}
	return true;
}

void Canvas::enqueueHelpDialog(EntityDef* entityDef) {
	Applet* app = CAppContainer::getInstance()->app;

	if (!app->player->enableHelp || this->state == Canvas::ST_DYING) {
		return;
	}
	if (this->numHelpMessages == 16) {
		app->Error(41); // ERR_MAXHELP
		return;
	}
	this->helpMessageTypes[this->numHelpMessages] = 1;
	this->helpMessageObjs[this->numHelpMessages] = entityDef;
	this->helpMessageThreads[this->numHelpMessages] = -1;
	this->numHelpMessages++;
	if (this->state == Canvas::ST_PLAYING) {
		this->dequeueHelpDialog();
	}
}

void Canvas::updateView() {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->time < this->shakeTime) {
		this->shakeX = app->nextByte() % (this->shakeIntensity * 2) - this->shakeIntensity;
		this->shakeY = app->nextByte() % (this->shakeIntensity * 2) - this->shakeIntensity;
		this->staleView = true;
	}
	else if (this->shakeX != 0 || this->shakeY != 0) {
		this->staleView = true;
		this->shakeX = (this->shakeY = 0);
	}

	if (app->game->isCameraActive() && this->state != Canvas::ST_INTER_CAMERA) {
		app->game->activeCamera->Render();
		this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
		app->hud->repaintFlags &= 0x18;
		return;
	}

	if (this->knockbackDist > 0 && this->viewX == this->destX && this->viewY == this->destY) {
		this->attemptMove(this->viewX + this->knockbackX * 64, this->viewY + this->knockbackY * 64);
	}

	bool b = this->viewX == this->destX && this->viewY == this->destY;
	bool b2 = this->viewAngle == this->destAngle;
	int animPos = this->animPos;
	int animAngle = this->animAngle;

	if (app->player->statusEffects[2] > 0 || this->knockbackDist > 0) {
		animPos += animPos / 2;
		animAngle += animAngle / 2;
	}

	if (this->viewX != this->destX || this->viewY != this->destY || this->viewZ != this->destZ || this->viewAngle != this->destAngle) {
		this->invalidateRect();
	}

	if (this->viewX < this->destX) {
		this->viewX += animPos;
		if (this->viewX > this->destX) {
			this->viewX = this->destX;
		}
	}
	else if (this->viewX > this->destX) {
		this->viewX -= animPos;
		if (this->viewX < this->destX) {
			this->viewX = this->destX;
		}
	}

	if (this->viewY < this->destY) {
		this->viewY += animPos;
		if (this->viewY > this->destY) {
			this->viewY = this->destY;
		}
	}
	else if (this->viewY > this->destY) {
		this->viewY -= animPos;
		if (this->viewY < this->destY) {
			this->viewY = this->destY;
		}
	}

	if (this->viewZ < this->destZ) {
		this->viewZ += this->zStep;
		if (this->viewZ > this->destZ) {
			this->viewZ = this->destZ;
		}
	}
	else if (this->viewZ > this->destZ) {
		this->viewZ -= this->zStep;
		if (this->viewZ < this->destZ) {
			this->viewZ = this->destZ;
		}
	}

	if (this->viewAngle < this->destAngle) {
		this->viewAngle += animAngle;
		if (this->viewAngle > this->destAngle) {
			this->viewAngle = this->destAngle;
		}
	}
	else if (this->viewAngle > this->destAngle) {
		this->viewAngle -= animAngle;
		if (this->viewAngle < this->destAngle) {
			this->viewAngle = this->destAngle;
		}
	}

	if (this->viewPitch < this->destPitch) {
		this->viewPitch += this->pitchStep;
		if (this->viewPitch > this->destPitch) {
			this->updateFacingEntity = true;
			this->viewPitch = this->destPitch;
		}
	}
	else if (this->viewPitch > this->destPitch) {
		this->viewPitch -= this->pitchStep;
		if (this->viewPitch < this->destPitch) {
			this->updateFacingEntity = true;
			this->viewPitch = this->destPitch;
		}
	}

	int viewZ = this->viewZ;
	if (this->knockbackDist != 0) {
		int n = this->viewX;
		if (this->knockbackX == 0) {
			n = this->viewY;
		}
		viewZ = this->viewZ + (10 * app->render->sinTable[(std::abs(n - this->knockbackStart) << 9) / this->knockbackWorldDist & 0x3FF] >> 16);
	}

	if (this->state == Canvas::ST_AUTOMAP) {
		this->viewX = this->destX;
		this->viewY = this->destY;
		this->viewZ = this->destZ;
		this->viewAngle = this->destAngle;
		this->viewPitch = this->destPitch;
	}

	if (this->state == Canvas::ST_COMBAT) {
		app->game->gsprite_update(app->time);
	}
	if (app->game->gotoTriggered) {
		app->game->gotoTriggered = false;
		int flagForFacingDir = this->flagForFacingDir(8);
		app->game->eventFlagsForMovement(-1, -1, -1, -1);
		app->game->executeTile(this->destX >> 6, this->destY >> 6, app->game->eventFlags[1], true);
		app->game->executeTile(this->destX >> 6, this->destY >> 6, flagForFacingDir, true);
	}
	else if (!b && this->viewX == this->destX && this->viewY == this->destY) {
		this->finishMovement();
	}

	if (!b2 && this->viewAngle == this->destAngle) {
		this->finishRotation(true);
	}

	if (app->game->isCameraActive() && this->state != Canvas::ST_INTER_CAMERA) {
		app->game->activeCamera->Update(app->game->activeCameraKey, app->gameTime - app->game->activeCameraTime);
		app->game->activeCamera->Render();
		this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_PARTICLES);
		return;
	}

	if (this->isZoomedIn) {
		int n2 = this->zoomAccuracy * app->render->sinTable[(app->time - this->zoomStateTime) / 2 & 0x3FF] >> 24;
		int n3 = this->zoomAccuracy * app->render->sinTable[(app->time - this->zoomStateTime) / 3 & 0x3FF] >> 24;
		int zoomFOV = this->zoomFOV;
		int n4;
		if (app->time < this->zoomTime) {
			n4 = this->zoomDestFOV + (this->zoomFOV - this->zoomDestFOV) * (this->zoomTime - app->time) / 360;
		}
		else {
			this->zoomTime = 0;
			n4 = (this->zoomFOV = this->zoomDestFOV);
		}
		int n5 = this->zoomPitch + this->viewPitch;
		if (app->combat->curAttacker == nullptr && this->state == Canvas::ST_COMBAT && app->combat->nextStage == 1) {
			n5 += app->render->sinTable[512 * ((app->gameTime - app->combat->animStartTime << 16) / app->combat->animTime) >> 16 & 0x3FF] * 28 >> 16;
		}
		this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle + this->zoomAngle + n2, n5 + n3, this->viewRoll, n4);
		this->updateFacingEntity = true;
	}
	else if (this->loadMapID != 0) {
		this->renderScene(this->viewX, this->viewY, viewZ, this->viewAngle, this->viewPitch, this->viewRoll, 290);
	}
}

void Canvas::clearSoftKeys() {
	this->softKeyLeftID = -1;
	this->softKeyRightID = -1;
	this->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
}

void Canvas::clearLeftSoftKey() {
	this->softKeyLeftID = -1;
	this->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
}

void Canvas::clearRightSoftKey() {
	this->softKeyRightID = -1;
	this->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
}

void Canvas::setLeftSoftKey(short i, short i2) {
	if (!this->displaySoftKeys) {
		return;
	}
	this->softKeyLeftID = Localization::STRINGID(i, i2);
	this->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
}

void Canvas::setRightSoftKey(short i, short i2) {
	if (!this->displaySoftKeys) {
		return;
	}
	this->softKeyRightID = Localization::STRINGID(i, i2);
	this->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
}

void Canvas::setSoftKeys(short n, short n2, short n3, short n4) {
	if (!this->displaySoftKeys) {
		return;
	}
	this->softKeyLeftID = Localization::STRINGID(n, n2);
	this->softKeyRightID = Localization::STRINGID(n3, n4);
	this->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
	this->checkHudEvents();
}

void Canvas::checkHudEvents() {
	Applet* app = CAppContainer::getInstance()->app;
	//app->hud->hudEventsAvailable = (this->displaySoftKeys && (this->softKeyLeftID == 52 || this->softKeyRightID == 52)); // J2ME Only
}

void Canvas::drawSoftKeys(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	// J2ME Only
}

void Canvas::setLoadingBarText(short loadingStringID, short loadingStringType) {
	this->loadingStringID = loadingStringID;
	this->loadingStringType = loadingStringType;
	this->loadingFlags |= 0x3;
}

void Canvas::updateLoadingBar(bool b) {
	Applet* app = CAppContainer::getInstance()->app;
	int uVar2;

	if (b == false) {
		if (app->upTimeMs - this->lastPacifierUpdate < 0x96) {
			return;
		}
		this->lastPacifierUpdate = app->upTimeMs;
	}
	uVar2 = this->loadingFlags;
	this->loadingFlags = uVar2 | 3;
	if ((this->loadingStringID == -1) || (this->loadingStringType == -1)) {
		this->setLoadingBarText((short)0, (short)57);
	}
	this->loadingFlags = uVar2 | 3;
	this->repaintFlags |= Canvas::REPAINT_LOADING_BAR;
	return;
}

void Canvas::drawLoadingBar(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	Text* text;
	int iVar1;
	int iVar2;
	int iVar3;
	int uVar4;
	int iVar5;
	int iVar6;

	if ((this->loadingFlags & 3) != 0) {
		iVar1 = this->SCR_CX;
		iVar6 = this->SCR_CY;
		if ((this->loadingFlags & 1) != 0) {
			text = app->localization->getSmallBuffer();
			this->loadingFlags &= 0xfffffffe;
			graphics->eraseRgn(this->displayRect);
			graphics->fillRegion(this->imgFabricBG, iVar1 + -0x4b, iVar6 + -0x1d, 0x96, 0x3a);
			graphics->setColor(0xffffffff);
			graphics->drawRect(iVar1 + -0x4b, iVar6 + -0x1d, 0x96, 0x3a);
			app->localization->composeText(this->loadingStringID, this->loadingStringType, text);
			text->dehyphenate();
			graphics->drawString(text, this->SCR_CX, this->SCR_CY + -0x16, 0x11);
			text->setLength(0);
			app->localization->composeText(0, 58, text);
			text->dehyphenate();
			graphics->drawString(text, this->SCR_CX, this->SCR_CY + -6, 0x11);
			text->dispose();
			iVar1 = this->SCR_CX;
			iVar6 = this->SCR_CY;
		}
		this->loadingFlags &= 0xfffffffd;
		iVar2 = iVar1 + 0x3e;
		iVar5 = this->pacifierX + 10;
		this->pacifierX = iVar5;
		iVar3 = iVar2;
		if (iVar2 <= iVar5) {
			iVar3 = iVar1 + -0x42;
		}
		if ((iVar2 <= iVar5) || (iVar3 = iVar1 + -0x42, iVar5 < iVar3)) {
			this->pacifierX = iVar3;
		}
		graphics->setColor(-0x1000000);
		graphics->fillRect(iVar1 + -0x43, iVar6 + 0xb, 0x86, 0xc);
		graphics->setColor(-1);
		graphics->drawRect(iVar1 + -0x43, iVar6 + 0xb, 0x86, 0xc);
		iVar3 = this->pacifierX;
		iVar1 = (iVar1 + 0x43) - iVar3;
		if (0x19 < iVar1) {
			iVar1 = 0x1a;
		}
		iVar2 = ((iVar3 / 10) / 6) * 8;
		uVar4 = (iVar3 / 10) % 6;
		if (uVar4 < 3) {
			iVar2 = 6;
		}
		if (2 < uVar4) {
			iVar2 = 0;
		}
		graphics->drawRegion(this->imgLoadingFire, 0, 0, iVar1, 9, iVar3, iVar6 + 0xd, 0, iVar2, 0);
	}
}

void Canvas::unloadMedia() {
	Applet* app = CAppContainer::getInstance()->app;
	this->freeRuntimeData();
	app->game->unloadMapData();
	app->render->unloadMap();
}

void Canvas::invalidateRect() {
	this->staleView = true;
}

int Canvas::getRecentLoadType() {
	if (this->recentBriefSave) {
		return 2;
	}
	return 1;
}

void Canvas::initZoom() {
	Applet* app = CAppContainer::getInstance()->app;
	this->zoomTime = 0;
	this->zoomCurFOVPercent = 0;
	this->zoomFOV = this->zoomDestFOV = 190;
	this->zoomAngle = 0;
	this->zoomPitch = 0;
	this->zoomTurn = 0;
	this->viewPitch = this->destPitch = 0;
	this->zoomStateTime = app->time;
	this->isZoomedIn = true;
	app->StartAccelerometer();
	this->m_sniperScopeDialScrollButton->Update(0, 320);
	this->zoomAccuracy = 2560 * std::max(0, std::min((256 - app->player->ce->getStatPercent(Enums::STAT_ACCURACY) << 8) / 26, 256)) >> 8;
	this->zoomMinFOVPercent = 256;
	this->zoomMaxAngle = 64 - (this->zoomAccuracy >> 8);
	app->render->startFade(500, 2);
	this->drawPlayingSoftKeys();

	CAppContainer::getInstance()->sdlGL->centerMouse(0, -22); // [GEC]
}

void Canvas::zoomOut() {
	Applet* app = CAppContainer::getInstance()->app;
	this->isZoomedIn = false;
	this->viewAngle += this->zoomAngle;
	int n = 255;
	this->destAngle = this->viewAngle = ((this->viewAngle + (n >> 1)) & ~n);
	app->render->startFade(500, 2);
	this->finishRotation(true);
	app->tinyGL->resetViewPort();
	this->drawPlayingSoftKeys();
}

bool Canvas::handleZoomEvents(int key, int action) {
	return this->handleZoomEvents(key, action, false);
}

bool Canvas::handleZoomEvents(int key, int action, bool b) {
	Applet* app = CAppContainer::getInstance()->app;
	if (!b && ((this->zoomTime != 0) || app->game->activePropogators != 0 || app->game->animatingEffects != 0 || !app->game->snapMonsters(false))) {
		return true;
	}
	int n3 = 5 + ((20 * (256 - this->zoomCurFOVPercent)) >> 8);
	if (action == Enums::ACTION_MENU || action == Enums::ACTION_BACK) {
		this->zoomOut();
		return true;
	}
	if (action == Enums::ACTION_AUTOMAP) {
		if (!app->player->inTargetPractice) {
			app->hud->msgCount = 0;
			app->menuSystem->setMenu(Menus::MENU_INGAME);
			return true;
		}
	}
	else if (action == Enums::ACTION_RIGHT) {
		this->zoomAngle -= n3;
		this->updateFacingEntity = true;
		++this->zoomTurn;
		CAppContainer::getInstance()->sdlGL->centerMouse(0, -22); // [GEC]
		CAppContainer::getInstance()->app->StopAccelerometer(); // [GEC]
	}
	else if (action == Enums::ACTION_LEFT) {
		this->zoomAngle += n3;
		this->updateFacingEntity = true;
		++this->zoomTurn;
		CAppContainer::getInstance()->sdlGL->centerMouse(0, -22); // [GEC]
		CAppContainer::getInstance()->app->StopAccelerometer(); // [GEC]
	}
	else if (action == Enums::ACTION_DOWN) {
		this->zoomPitch -= n3;
		++this->zoomTurn;
		CAppContainer::getInstance()->sdlGL->centerMouse(0, -22); // [GEC]
		CAppContainer::getInstance()->app->StopAccelerometer(); // [GEC]
	}
	else if (action == Enums::ACTION_UP) {
		this->zoomPitch += n3;
		++this->zoomTurn;
		CAppContainer::getInstance()->sdlGL->centerMouse(0, -22); // [GEC]
		CAppContainer::getInstance()->app->StopAccelerometer(); // [GEC]
	}
	else if (action == Enums::ACTION_PASSTURN) {
		app->hud->addMessage((short)45);
		app->game->touchTile(this->destX, this->destY, false);
		app->game->advanceTurn();
		this->invalidateRect();
		this->zoomTurn = 0;
	}
	else if (action == Enums::ACTION_NEXTWEAPON) {
		if (this->zoomCurFOVPercent < this->zoomMinFOVPercent) {
			this->zoomCurFOVPercent += 64;
			this->zoomCurFOVPercent = std::min(this->zoomCurFOVPercent, this->zoomMinFOVPercent); // [GEC]
			++this->zoomTurn;
			this->zoomDestFOV = 190 + ((-55 * this->zoomCurFOVPercent) >> 7);
			this->zoomDestFOV = std::max(this->zoomDestFOV, 102);
			this->zoomTime = app->time + 360;

			// [GEC] update scroll bar
			{
				float maxScroll = (float)((this->m_sniperScopeDialScrollButton->barRect).h - this->m_sniperScopeDialScrollButton->field_0x4c_);
				float curFOV = (float)((float)this->zoomCurFOVPercent / (float)this->zoomMinFOVPercent);
				this->m_sniperScopeDialScrollButton->field_0x48_ = (int)(maxScroll - (maxScroll * curFOV));
				app->sound->playSound(1113, 0, 5, false);
			}

		}
	}
	else if (action == Enums::ACTION_PREVWEAPON) {
		if (this->zoomCurFOVPercent > 0) {
			this->zoomCurFOVPercent -= 64;
			this->zoomCurFOVPercent = std::max(this->zoomCurFOVPercent, 0); // [GEC]
			++this->zoomTurn;
			this->zoomDestFOV = 190 + ((-55 * this->zoomCurFOVPercent) >> 7);
			this->zoomTime = app->time + 360;

			// [GEC] update scroll bar
			{
				float maxScroll = (float)((this->m_sniperScopeDialScrollButton->barRect).h - this->m_sniperScopeDialScrollButton->field_0x4c_);
				float curFOV = (float)((float)this->zoomCurFOVPercent / (float)this->zoomMinFOVPercent);
				this->m_sniperScopeDialScrollButton->field_0x48_ = (int)(maxScroll - (maxScroll * curFOV));
				app->sound->playSound(1113, 0, 5, false);
			}
		}
		++this->zoomTurn;
	}
	else if (action == Enums::ACTION_FIRE) {
		this->zoomTurn = 0;
		return handlePlayingEvents(key, action);
	}

	if (this->zoomPitch < -this->zoomMaxAngle) {
		this->zoomPitch = -this->zoomMaxAngle;
	}
	else if (this->zoomPitch > this->zoomMaxAngle) {
		this->zoomPitch = this->zoomMaxAngle;
	}
	if ((this->zoomTurn & 0x7) == 0x7) {
		app->game->advanceTurn();
	}
	return true;
}

void Canvas::handleCharacterSelectionInput(int key, int action) {
	Applet* app = CAppContainer::getInstance()->app;
	//printf("handleCharacterSelectionInput key %d, action %d\n", key, action);
	if (this->stateVars[1]) {
		if (this->stateVars[1] == 1) {

			if ((action == Enums::ACTION_LEFT) || (action == Enums::ACTION_RIGHT)) {
				this->stateVars[2] = this->stateVars[2] != 1;
			}
			else if (action == Enums::ACTION_FIRE) {
				if (this->stateVars[2] == 1) {
					app->player->setCharacterChoice(this->stateVars[0]);
					app->player->reset();
					app->canvas->setState(Canvas::ST_INTRO);
					this->disposeCharacterSelection();
				}
				else {
					this->stateVars[1] = 0;
					this->stateVars[2] = 1;
				}
			}
			
		}
	}
	else {
		for (int i = 0; i < 3; i++) { // Characters
			if (this->m_characterButtons->GetButton(i)->highlighted) {
				if (i == 0) {
					this->stateVars[0] = 1;
				}
				else if (i == 1) {
					this->stateVars[0] = 3;
				}
				else {
					this->stateVars[0] = 2;
				}
			}
		}

		if (this->m_characterButtons->GetButton(3)->highlighted) { // Yes
			app->player->setCharacterChoice(this->stateVars[0]);
			app->player->reset();
			app->canvas->setState(Canvas::ST_INTRO);
			this->disposeCharacterSelection();
		}

		if (this->m_characterButtons->GetButton(4)->highlighted) { // Back
			app->canvas->backToMain(false);
		}

		this->m_characterButtons->HighlightButton(0, 0, false);

		if (!this->touched) {
			if (this->stateVars[8] == 0) { // [GEC]
				if (action == Enums::ACTION_RIGHT) {
					for (int i = 0; i < 3; ++i) {
						if (this->stateVars[0] == this->getCharacterConstantByOrder(i)) {
							this->stateVars[0] = this->getCharacterConstantByOrder((i + 1) % 3);
							app->menuSystem->soundClick();
							break;
						}
					}
				}
				else if (action == Enums::ACTION_LEFT) {
					for (int j = 0; j < 3; ++j) {
						if (this->stateVars[0] == this->getCharacterConstantByOrder(j)) {
							this->stateVars[0] = this->getCharacterConstantByOrder((j + 2) % 3);
							app->menuSystem->soundClick();
							break;
						}
					}
				}
				else if (action == Enums::ACTION_FIRE) {
					this->stateVars[8] = 1; // [GEC]
					app->sound->playSound(1086, 0, 5, false);
				}
				else if (action == Enums::ACTION_MENU) {
					this->disposeCharacterSelection();
					this->backToMain(false);
				}
			}
			else if (this->stateVars[8] == 1) { // [GEC]
				if ((action == Enums::ACTION_LEFT) || (action == Enums::ACTION_RIGHT)) {
					this->stateVars[2] = this->stateVars[2] != 1;
					app->menuSystem->soundClick();
				}
				else if (action == Enums::ACTION_FIRE) {
					if (this->stateVars[2] == 1) {
						app->player->setCharacterChoice(this->stateVars[0]);
						app->player->reset();
						app->canvas->setState(Canvas::ST_INTRO);
						this->disposeCharacterSelection();
					}
					else {
						this->disposeCharacterSelection();
						this->backToMain(false);
					}
					app->sound->playSound(1086, 0, 5, false);
				}
				else if (action == Enums::ACTION_MENU) {
					this->stateVars[1] = 0;
					this->stateVars[2] = 1;
					this->stateVars[8] = 0; // [GEC]
					app->sound->playSound(1122, 0, 5, false);
				}
			}
		}
	}
}

void Canvas::handleStoryInput(int key, int action) {
	Applet* app = CAppContainer::getInstance()->app;

	if (action == Enums::ACTION_LEFT || action == Enums::ACTION_RIGHT) {
		if (this->stateVars[0] != 2) {
			this->stateVars[0] ^= 1;
		}
	}
	else if (action == Enums::ACTION_UP) {
		if (this->stateVars[0] != 2 && this->storyPage < this->storyTotalPages - 1) {
			this->stateVars[0] = 2;
		}
	}
	else if (action == Enums::ACTION_DOWN) {
		if (this->stateVars[0] == 2) {
			this->stateVars[0] = 1;
		}
	}
	else if (action == Enums::ACTION_FIRE) {
		switch (this->stateVars[0]) {
		case 0: {
			this->changeStoryPage(-1);
			if (this->state == Canvas::ST_CHARACTER_SELECTION) {
				this->stateVars[0] = app->player->characterChoice;
				break;
			}
			break;
		}
		case 1: {
			this->changeStoryPage(1);
			break;
		}
		case 2: {
			this->storyPage = this->storyTotalPages;
			break;
		}
		}
	}
	else if (action == Enums::ACTION_AUTOMAP) {
		this->changeStoryPage(1);
		this->stateVars[0] = 1;
	}
	else if (action == Enums::ACTION_BACK || action == Enums::ACTION_MENU) {
		this->changeStoryPage(-1);
		if (this->state == Canvas::ST_CHARACTER_SELECTION) {
			this->stateVars[0] = app->player->characterChoice;
		}
		else {
			this->stateVars[0] = 0;
		}
	}
}

void Canvas::lootingState() {
	Applet* app = CAppContainer::getInstance()->app;
	app->hud->repaintFlags |= 0x22;
	this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_VIEW3D);
	//app->hud->repaintFlags &= 0xFFFFFFBF; // J2ME
	int height = app->render->getHeight(this->destX, this->destY);
	int height2 = app->render->getHeight(this->destX + this->viewStepX, this->destY + this->viewStepY);
	if (app->time < this->lootingTime + 500) {
		int n = (500 - (app->time - this->lootingTime) << 16) / 500;
		int n2 = 65536 - n;
		if (this->crouchingForLoot) {
			int n3 = (height > height2) ? height : (height * n + height2 * n2 >> 16);
			this->viewX = this->destX + (48 + (-48 * n >> 16)) * (this->viewStepX >> 6);
			this->viewY = this->destY + (48 + (-48 * n >> 16)) * (this->viewStepY >> 6);
			this->viewZ = n3 + 26 + (10 * n >> 16);
			this->viewPitch = std::max(-(64 - (64 * n >> 16)) + this->lootingCachedPitch, -64);
		}
		else {
			int n4 = (height > height2) ? height : (height * n2 + height2 * n >> 16);
			this->viewX = this->destX + (48 * n >> 16) * (this->viewStepX >> 6);
			this->viewY = this->destY + (48 * n >> 16) * (this->viewStepY >> 6);
			this->viewZ = n4 + 36 + (-10 * n >> 16);
			this->viewPitch = std::max(-(64 * n >> 16) + this->lootingCachedPitch, -64);
		}
		this->updateView();
	}
	else {
		if (!this->field_0xac5_) {
			this->field_0xac5_ = true;
			app->sound->playSound(1055, 0, 3, 0);
		}
		if (this->crouchingForLoot) {
			this->viewX = this->destX + 48 * (this->viewStepX >> 6);
			this->viewY = this->destY + 48 * (this->viewStepY >> 6);
			this->viewZ = std::max(height, height2) + 26;
			this->viewPitch = std::max(-64 + this->lootingCachedPitch, -64);
			this->updateView();
		}
		else {
			this->viewX = this->destX;
			this->viewY = this->destY;
			this->viewZ = height + 36;
			this->viewPitch = this->lootingCachedPitch;
			this->updateView();
			this->setState(Canvas::ST_PLAYING);
			app->game->advanceTurn();
		}
	}
}

void Canvas::handleLootingEvents(int action) {
	Applet* app = CAppContainer::getInstance()->app;
	if (this->crouchingForLoot && app->time > this->lootingTime + 500) {
		int max = std::max(this->numPoolItems + ((this->lootPoolCredits != 0) ? 1 : 0) - 3, 0);
		if (action == Enums::ACTION_FIRE) {
			if (this->lootLineNum >= max) {
				this->lootingTime = app->time;
				this->crouchingForLoot = false;
				this->giveLootPool();
			}
			else {
				this->lootLineNum = std::min(this->lootLineNum + 3, max);
			}
		}
		else if (action == Enums::ACTION_PASSTURN || action == Enums::ACTION_BACK) {
			this->lootingTime = app->time;
			this->crouchingForLoot = false;
			this->giveLootPool();
		}
		else if (action == Enums::ACTION_DOWN) {
			this->lootLineNum = std::min(this->lootLineNum + 1, max);
		}
		else if (action == Enums::ACTION_UP) {
			this->lootLineNum = std::max(this->lootLineNum - 1, 0);
		}
		else if (action == Enums::ACTION_LEFT) {
			this->lootLineNum = 0;
		}
		else if (action == Enums::ACTION_RIGHT) {
			this->lootLineNum = max;
		}
	}
}

void Canvas::drawLootingMenu(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	if (this->crouchingForLoot && app->time > this->lootingTime + 500) {
		int* dialogRect = this->dialogRect;
		dialogRect[0] = this->viewRect[0] + 0;
		dialogRect[1] = this->viewRect[1] + 0 + 16;
		dialogRect[2] = this->viewRect[2] - dialogRect[0] - 0 - 1;
		dialogRect[3] = 48;
		graphics->setColor(0xFF660000);
		graphics->fillRect(dialogRect[0], dialogRect[1], dialogRect[2], dialogRect[3]);
		graphics->setColor(0xFF000000);
		graphics->fillRect(dialogRect[0], dialogRect[1] - 18, dialogRect[2], 18);
		graphics->setColor(0xFFFFFFFF);
		graphics->drawRect(dialogRect[0], dialogRect[1] - 18, dialogRect[2], 18);
		graphics->drawRect(dialogRect[0], dialogRect[1], dialogRect[2], dialogRect[3]);
		Text* smallBuffer = app->localization->getSmallBuffer();
		app->localization->composeText((short)0, (short)227, smallBuffer);
		smallBuffer->dehyphenate();
		graphics->drawString(smallBuffer, this->SCR_CX, dialogRect[1] - 16, 1);
		smallBuffer->dispose();
		for (int i = 0; i < 3; ++i) {
			graphics->drawString(this->lootText, dialogRect[0] + 5, dialogRect[1] + 1 + i * 16, 20, this->lootPoolIndices[2 * (i + this->lootLineNum)], this->lootPoolIndices[2 * (i + this->lootLineNum) + 1]);
		}
		int n = this->numPoolItems + ((this->lootPoolCredits != 0) ? 1 : 0);
		this->drawScrollBar(graphics, dialogRect[0] + dialogRect[2], dialogRect[1] + 1, dialogRect[3] - 1, this->lootLineNum, (this->lootLineNum + 3 > n) ? n : (this->lootLineNum + 3), n, 3);
	}
}

void Canvas::poolLoot(int* array) {
	Applet* app = CAppContainer::getInstance()->app;
	Entity* entity = app->game->findMapEntity(array[0], array[1], 512);
	this->lootText = app->localization->getLargeBuffer();
	this->lootText->setLength(0);
	this->numPoolItems = 0;
	this->numLootItems = 0;
	this->lootLineNum = 0;
	this->lootPoolCredits = 0;
	while (entity != nullptr) {
		if (entity->def->eType == 9) {
			if (entity->monster == nullptr) {
				if (entity->param != 0) {
					entity = entity->nextOnTile;
					continue;
				}
				++entity->param;
			}
			else {
				if ((entity->monster->flags & 0x800) != 0x0) {
					entity = entity->nextOnTile;
					continue;
				}
				entity->monster->flags |= 0x800;
			}
			entity->info |= 0x400000;
			for (int i = 0; i < 3; ++i) {
				if (entity->lootSet[i] == 0) {
					break;
				}
				bool b = true;
				int n = entity->lootSet[i] >> 12 & 0xF;
				if (n == 6) {
					int n2 = entity->lootSet[i] & 0xFFF;
					for (int j = 0; j < this->numPoolItems; ++j) {
						if ((entity->lootSet[j] >> 12 & 0xF) == 0x6 && n2 == (this->lootPool[j] & 0xFFF)) {
							b = false;
							break;
						}
					}
				}
				else {
					int n3 = entity->lootSet[i] & 0x3F;
					++this->numLootItems;
					int n4 = (entity->lootSet[i] & 0xFC0) >> 6;
					if (n == 0) {
						if (n4 == 24) {
							this->lootPoolCredits += n3;
							continue;
						}
						if (n4 == 25) {
							this->lootPoolCredits += n3 * 100;
							continue;
						}
					}
					int n5 = entity->lootSet[i] >> 6;
					for (int k = 0; k < this->numPoolItems; ++k) {
						if (n5 == this->lootPool[k] >> 6) {
							b = false;
							this->lootPool[k] = ((this->lootPool[k] & 0xFFFFFFC0) | (n3 + (this->lootPool[k] & 0x3F) & 0x3F));
							break;
						}
					}
				}
				if (b) {
					this->lootPool[this->numPoolItems++] = entity->lootSet[i];
				}
			}
		}
		entity = entity->nextOnTile;
	}
	for (int l = 0; l < this->numPoolItems; ++l) {
		int n6 = this->lootPool[l];
		int n7 = n6 >> 12 & 0xF;
		if (n7 == 6) {
			short n8 = (short)(n6 & 0xFFF);
			this->lootText->append('\x88');
			app->localization->composeText(this->loadMapStringID, n8, this->lootText);
			this->lootText->append("|");
		}
		else {
			int n9 = (n6 & 0xFC0) >> 6;
			int n10 = n6 & 0x3F;
			app->localization->resetTextArgs();
			app->localization->addTextArg('\x88');
			EntityDef* find = app->entityDefManager->find(6, n7, n9);
			if (n7 == 1) {
				app->localization->addTextArg((short)1, find->longName);
				app->localization->composeText((short)0, (short)91, this->lootText);
			}
			else {
				app->localization->addTextArg(n10);
				app->localization->addTextArg((short)1, find->longName);
				app->localization->composeText((short)0, (short)90, this->lootText);
			}
		}
	}
	if (this->lootPoolCredits != 0) {
		app->localization->resetTextArgs();
		app->localization->addTextArg('\x88');
		app->localization->addTextArg(this->lootPoolCredits);
		app->localization->addTextArg((short)1, (short)157);
		app->localization->composeText((short)0, (short)90, this->lootText);
	}
	if (this->numPoolItems == 0 && this->lootPoolCredits == 0) {
		app->localization->composeText((short)0, (short)228, this->lootText);
	}
	this->lootText->dehyphenate();
	for (int n13 = 0; n13 < 18; ++n13) {
		this->lootPoolIndices[n13] = (short)0;
	}
	int length = this->lootText->length();
	int n11 = 0;
	int n12 = 0;
	for (int n13 = 0; n13 < length; ++n13) {
		if (this->lootText->charAt(n13) == '|') {
			this->lootPoolIndices[n12 * 2] = (short)n11;
			this->lootPoolIndices[n12 * 2 + 1] = (short)(n13 - n11);
			++n12;
			n11 = n13 + 1;
		}
	}
	this->lootPoolIndices[n12 * 2] = (short)n11;
	this->lootPoolIndices[n12 * 2 + 1] = (short)(length - n11);
}

void Canvas::giveLootPool() {
	Applet* app = CAppContainer::getInstance()->app;
	for (int i = 0; i < this->numPoolItems; ++i) {
		int n = this->lootPool[i];
		int n2 = n >> 12 & 0xF;
		if (n2 != 6) {
			int n3 = (n & 0xFC0) >> 6;
			app->player->give(n2, n3, n & 0x3F, false);
			if (n2 == 1) {
				int n4 = n3 * 9;
				uint8_t a = app->combat->weapons[n4 + 5];
				if (a != 0) {
					app->player->give(2, app->combat->weapons[n4 + 4], std::max((int)a, 10), false);
				}
			}
		}
	}
	if (this->lootPoolCredits != 0) {
		app->player->give(0, 24, this->lootPoolCredits, false);
		this->lootPoolCredits = 0;
	}
	app->game->foundLoot(this->viewX + this->viewStepX, this->viewY + this->viewStepY, this->viewZ, this->numLootItems);
	this->numPoolItems = 0;
	this->numLootItems = 0;
	this->lootText->dispose();
}

bool Canvas::handleTreadmillEvents(int action) {
	Applet* app = CAppContainer::getInstance()->app;
	if (this->treadmillReturnCode != 0) {
		return true;
	}
	if (this->treadmillLastStepTime + 300 > app->time) {
		if (this->numEvents == 4) {
			app->hud->addMessage((short)247, 3);
		}
		return false;
	}
	if (action == Enums::ACTION_DOWN) {
		this->treadmillReturnCode = 2;
		return true;
	}
	if (action == Enums::ACTION_STRAFELEFT || action == Enums::ACTION_STRAFERIGHT) {
		if (action == this->treadmillLastStep) {
			this->treadmillReturnCode = 3;
		}
		else {
			this->treadmillLastStep = action;
			if (++this->treadmillNumSteps * 2 >= 100) {
				this->treadmillReturnCode = 1;
			}
		}
		this->treadmillLastStepTime = app->time;
		return true;
	}
	if (action == Enums::ACTION_AUTOMAP) {
		this->treadmillReturnCode = 2;
		return true;
	}
	return true;
}

void Canvas::treadmillState() {
	Applet* app = CAppContainer::getInstance()->app;
	app->hud->repaintFlags |= 0x22;
	this->repaintFlags |= (Canvas::REPAINT_HUD | Canvas::REPAINT_VIEW3D);
	app->hud->repaintFlags &= 0xFFFFFFBF;
	bool b = false;
	if (this->treadmillReturnCode != 0) {
		if (this->treadmillReturnCode == 1 && app->time > this->treadmillLastStepTime + 300) {
			app->localization->resetTextArgs();
			app->localization->addTextArg(2);
			if (app->player->modifyStat(Enums::STAT_AGILITY, 2) == 0) {
				app->hud->addMessage((short)244, 3);
			}
			else {
				app->hud->addMessage((short)243, 3);
			}
			b = true;
		}
		else if (this->treadmillReturnCode == 3) {
			if (this->treadmillFall()) {
				app->hud->addMessage((short)245, 3);
				b = true;
			}
		}
		else if (this->treadmillReturnCode == 2) {
			this->attemptMove(this->viewX - this->viewStepX, this->viewY - this->viewStepY);
			app->hud->addMessage((short)246, 3);
			b = true;
		}
	}
	if (b) {
		app->combat->shiftWeapon(false);
		this->setState(Canvas::ST_PLAYING);
		return;
	}
	if (this->treadmillLastStep == 1) {
		this->updateView();
		return;
	}
	if (this->treadmillReturnCode == 0 && app->time > 1500 + this->treadmillLastStepTime) {
		this->treadmillReturnCode = 3;
		this->treadmillLastStepTime = app->time;
		return;
	}
	if (app->time <= this->treadmillLastStepTime + 300) {
		bool b2 = this->treadmillLastStep == 9;
		bool b3 = this->treadmillLastStepTime + 150 > app->time;
		bool b4 = b2 ^ !b3;
		int n = 4 + (-4 * ((std::abs(150 - (app->time - this->treadmillLastStepTime)) << 16) / 150) >> 16);
		if (b4 == b3) {
			n = -n;
		}
		int n2 = n * n;
		this->viewX = this->destX + n * (this->viewRightStepX >> 6);
		this->viewY = this->destY + n * (this->viewRightStepY >> 6);
		this->viewZ = 36 + n2 + app->render->getHeight(this->destX, this->destY);
		this->invalidateRect();
	}

	this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, this->viewRoll, 290);
}

bool Canvas::treadmillFall() {
	Applet* app = CAppContainer::getInstance()->app;
	if (app->time > this->treadmillLastStepTime + 1000 + 500 + 500) {
		this->viewX = this->destX;
		this->viewY = this->destY;
		this->viewZ = this->destZ;
		this->viewPitch = this->destPitch;
		return true;
	}
	int n = (this->viewStepX >> 6) * 32;
	int n2 = (this->viewStepY >> 6) * 32;
	if (this->treadmillLastStepTime + 1000 > app->time) {
		int n3 = 1000 - (app->time - this->treadmillLastStepTime);
		int n4 = (n3 << 16) / 1000;
		this->viewX = this->destX + (-n + (n * n4 >> 16));
		this->viewY = this->destY + (-n2 + (n2 * n4 >> 16));
		this->viewZ = 36;
		if (n3 < 250) {
			this->viewZ -= 12 - (12 * n4 >> 16);
		}
		if (n3 > 500) {
			this->viewPitch = 128 - (128 * n4 >> 16);
		}
		else {
			this->viewPitch = 128 * n4 >> 16;
		}
	}
	else if (this->treadmillLastStepTime + 1000 + 500 > app->time) {
		this->viewX = this->destX - n;
		this->viewY = this->destY - n2;
		this->viewZ = 24;
		this->viewPitch = 0;
	}
	else {
		int n5 = 500 - (app->time - 1000 - 500 - this->treadmillLastStepTime);
		int n6 = (n5 << 16) / 500;
		this->viewX = this->destX - (n * n6 >> 16);
		this->viewY = this->destY - (n2 * n6 >> 16);
		this->viewZ = 36 - (12 * n6 >> 16);
		if (n5 > 250) {
			this->viewPitch = -128 + (128 * n6 >> 16);
		}
		else {
			this->viewPitch = -(128 * n6 >> 16);
		}
	}
	this->viewZ += app->render->getHeight(this->destX, this->destY);
	this->invalidateRect();
	this->renderScene(this->viewX, this->viewY, this->viewZ, this->viewAngle, this->viewPitch, this->viewRoll, 290);
	return false;
}

void Canvas::drawTreadmillReadout(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	Text* smallBuffer = app->localization->getSmallBuffer();
	app->localization->resetTextArgs();
	app->localization->addTextArg(this->treadmillNumSteps * 2);
	app->localization->composeText((short)0, (short)242, smallBuffer);
	app->hud->drawImportantMessage(graphics, smallBuffer, 0xFF666666);
	smallBuffer->dispose();
	this->m_treadmillButtons->Render(graphics);
}

void Canvas::drawTargetPracticeScore(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	if (app->hud->msgCount != 0 && (app->hud->messageFlags[0] & 0x4) != 0x0) {
		return;
	}
	Text* smallBuffer = app->localization->getSmallBuffer();
	app->localization->resetTextArgs();
	app->localization->composeText((short)0, (short)229, smallBuffer);
	smallBuffer->append(app->player->targetPracticeScore);
	app->hud->drawImportantMessage(graphics, smallBuffer, 0xFF7F0000);
	smallBuffer->dispose();
}

void Canvas::drawTravelMap(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->stateVars[5] == 1) {
		this->drawStarFieldPage(graphics);
		this->staleView = true;
		return;
	}

	graphics->drawImage(this->imgTravelBG, this->SCR_CX, (this->displayRect[3] - this->imgTravelBG->height) / 2, 17, 0, 0);

	if (this->xDiff > 1 || this->yDiff > 1) {
		graphics->fillRegion(this->imgFabricBG, 0, 0, this->displayRect[2], this->yDiff);
		graphics->fillRegion(this->imgFabricBG, 0, this->yDiff, this->xDiff, this->displayRect[3] - this->yDiff);
		graphics->fillRegion(this->imgFabricBG, this->xDiff, this->yDiff + this->mapHeight, this->mapWidth, this->yDiff);
		graphics->fillRegion(this->imgFabricBG, this->xDiff + this->mapWidth, this->yDiff, this->xDiff, this->displayRect[3] - this->yDiff);
		graphics->drawRect(this->xDiff + -1, this->yDiff + -1, this->mapWidth + 1, this->mapHeight + 1, 0xFF000000);
		graphics->clipRect(this->xDiff, this->yDiff, this->imgTravelBG->width, this->imgTravelBG->height);
	}

	int time = app->upTimeMs - this->stateVars[0];
	this->drawGridLines(graphics, time + this->totalTMTimeInPastAnimations);

	bool levelSamePlanet = this->newLevelSamePlanet();
	if (this->stateVars[1] != 1) {
		if (time > (levelSamePlanet ? 1500 : 700)) {
			this->totalTMTimeInPastAnimations += time;
			this->stateVars[0] += time;
			this->stateVars[1] = 1;
			time = 0;
			if (levelSamePlanet) {
				this->stateVars[2] = 1;
				this->stateVars[3] = 1;
			}
		}
		else if (levelSamePlanet) {
			this->drawAppropriateCloseup(graphics, this->TM_LastLevelId, false);
			Text* smallBuffer = app->localization->getSmallBuffer();
			smallBuffer->setLength(0);
			app->localization->composeText((short)3, app->game->levelNames[this->TM_LastLevelId - 1], smallBuffer);
			smallBuffer->dehyphenate();
			this->drawLocatorBoxAndName(graphics, (time & 0x200) == 0x0, this->TM_LastLevelId, smallBuffer);
			smallBuffer->dispose();
		}
	}

	if (this->stateVars[1] == 1) {
		if (this->stateVars[2] != 1) {
			if (this->drawDottedLine(graphics, time)) {
				this->totalTMTimeInPastAnimations += time;
				this->stateVars[0] += time;
				this->stateVars[2] = 1;
			}
		}
		else if (this->stateVars[2] == 1) {
			if (this->stateVars[3] != 1) {
				this->drawDottedLine(graphics);
				if (time > 500 || levelSamePlanet) {
					this->totalTMTimeInPastAnimations += time;
					this->stateVars[0] += time;
					this->stateVars[3] = 1;
				}
			}
			else if (this->stateVars[3] == 1) {
				this->drawAppropriateCloseup(graphics, this->TM_LoadLevelId, this->stateVars[8] == 1);
				if (this->stateVars[6] == 1) {
					Text* smallBuffer2 = app->localization->getSmallBuffer();
					smallBuffer2->setLength(0);
					app->localization->composeText((short)3, app->game->levelNames[this->TM_LoadLevelId - 1], smallBuffer2);
					smallBuffer2->dehyphenate();

					drawLocatorBoxAndName(graphics, (time & 0x200) == 0x0, this->TM_LoadLevelId, smallBuffer2);
					smallBuffer2->setLength(0);
					app->localization->composeText((short)0, (short)96, smallBuffer2);
					smallBuffer2->wrapText(24);
					smallBuffer2->dehyphenate();
					graphics->drawString(smallBuffer2, this->SCR_CX, this->displayRect[3] - 21, 17);
					smallBuffer2->dispose();
				}
				else if (this->stateVars[4] != 1) {
					if (this->drawLocatorLines(graphics, time, levelSamePlanet, this->stateVars[8] == 1)) {
						this->totalTMTimeInPastAnimations += time;
						this->stateVars[0] += time;
						this->stateVars[4] = 1;
					}
				}
				else {
					this->drawLocatorLines(graphics, -1, levelSamePlanet, this->stateVars[8] == 1);
					if (time > 800) {
						this->totalTMTimeInPastAnimations += time;
						this->stateVars[0] += time;
						if (this->stateVars[8] == 0) {
							this->stateVars[6] = 1;
						}
						else {
							this->stateVars[4] = 0;
							this->stateVars[8] = 0;
							int n7 = 123;
							int n8 = 150;
							short n9 = (short)(2 * (this->TM_LoadLevelId - 1));
							this->targetX = Canvas::CROSS_HAIR_CORDS[n9] + this->xDiff + n7;
							this->targetY = Canvas::CROSS_HAIR_CORDS[n9 + 1] + this->yDiff + n8;
						}
					}
				}
			}
		}
	}

	this->staleView = true;
}

bool Canvas::newLevelSamePlanet() {
	return this->TM_LastLevelId != this->TM_LoadLevelId && ((onMoon(this->TM_LastLevelId) && onMoon(this->TM_LoadLevelId)) || (onEarth(this->TM_LastLevelId) && onEarth(this->TM_LoadLevelId)) || (inHell(this->TM_LastLevelId) && inHell(this->TM_LoadLevelId)));
}

void Canvas::drawAppropriateCloseup(Graphics* graphics, int n, bool b) {
	if (this->onMoon(n)) {
		graphics->drawImage(this->imgTierCloseUp, Canvas::moonCoords[0], Canvas::moonCoords[1], 0, 0, 0);
	}
	else if (this->onEarth(n)) {
		if (b) {
			graphics->drawImage(this->imgEarthCloseUp, Canvas::earthCoords[0], Canvas::earthCoords[1], 0, 0, 0);
		}
		else {
			graphics->drawImage(this->imgTierCloseUp, Canvas::earthCoords[2], Canvas::earthCoords[3], 0, 0, 0);
		}
	}
	else {
		graphics->drawImage(this->imgTierCloseUp, Canvas::hellCoords[0], Canvas::hellCoords[1], 0, 0, 0);
	}
}

bool Canvas::drawDottedLine(Graphics* graphics, int n) {
	int n2 = n / 22;
	bool b;
	if (this->TM_LastLevelId == 0 && onMoon(this->TM_LoadLevelId)) {
		b = this->drawMarsToMoonLinePlusSpaceShip(graphics, n2);
	}
	else if (this->onMoon(this->TM_LastLevelId) && this->onEarth(this->TM_LoadLevelId)) {
		b = this->drawMoonToEarthLine(graphics, n2, false);
	}
	else if (this->onEarth(this->TM_LastLevelId) && this->onMoon(this->TM_LoadLevelId)) {
		b = this->drawMoonToEarthLine(graphics, n2, true);
	}
	else if (this->onEarth(this->TM_LastLevelId) && inHell(this->TM_LoadLevelId)) {
		b = this->drawEarthToHellLine(graphics, n2, false);
	}
	else {
		b = (!inHell(this->TM_LastLevelId) || !onEarth(this->TM_LoadLevelId) || this->drawEarthToHellLine(graphics, n2, true));
	}
	return b;
}

bool Canvas::drawDottedLine(Graphics* graphics) {
	return drawDottedLine(graphics, 22 * std::max(this->screenRect[2], this->screenRect[3]));
}

bool Canvas::drawMarsToMoonLinePlusSpaceShip(Graphics* graphics, int n) {
	Applet* app = CAppContainer::getInstance()->app;

	bool b = false;
	int width = this->imgTravelPath->width;
	int n2 = n;
	if (n2 > width) {
		b = true;
		n2 = width;
	}
	if (n > width / 3) {
		Text* smallBuffer = app->localization->getSmallBuffer();
		smallBuffer->setLength(0);
		app->localization->composeText((short)3, (short)165, smallBuffer);
		smallBuffer->dehyphenate();
		graphics->drawString(smallBuffer, Canvas::moonNameCoords[0], Canvas::moonNameCoords[1], 4);
		smallBuffer->dispose();
	}
	graphics->drawRegion(this->imgTravelPath, width - n2, 0, n2, this->imgTravelPath->height, Canvas::moonPathCoords[0] + width - n2, Canvas::moonPathCoords[1], 0, 0, 0);
	int n3 = width - n2;
	graphics->drawImage(this->imgSpaceShip, Canvas::moonPathCoords[0] + n3 - this->imgSpaceShip->width, this->yCoordOfSpaceShip(n3) + Canvas::moonPathCoords[1] - (this->imgSpaceShip->height >> 1), 0, 0, 0);
	return b;
}

int Canvas::yCoordOfSpaceShip(int n) {
	int iVar1;
	iVar1 = (n * 11) / 15;
	return ((-851 * (iVar1 * iVar1) / 110880 + 6115 * iVar1 / 22176 + 40) * 15) / 11;
}

bool Canvas::drawMoonToEarthLine(Graphics* graphics, int n, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	bool b2 = false;
	int height = this->imgTravelPath->height;
	int n2 = n;
	if (n2 > height) {
		b2 = true;
		n2 = height;
	}
	int n3 = b ? 0 : (height - n2);
	int width = this->imgTravelPath->width;
	if (n > height / 2) {
		Text* smallBuffer = app->localization->getSmallBuffer();
		smallBuffer->setLength(0);
		if (!b) {
			app->localization->composeText((short)3, (short)164, smallBuffer);
			smallBuffer->dehyphenate();
			graphics->drawString(smallBuffer, Canvas::earthNameCoords[0], Canvas::earthNameCoords[1], 4);
		}
		else {
			app->localization->composeText((short)3, (short)165, smallBuffer);
			smallBuffer->dehyphenate();
			graphics->drawString(smallBuffer, Canvas::moonNameCoords[2], Canvas::moonNameCoords[3], 4);
		}
		smallBuffer->dispose();
	}
	graphics->drawRegion(this->imgTravelPath, 0, n3, width, n2, Canvas::earthPathCoords[0], Canvas::earthPathCoords[1] + n3, 0, 0, 0);
	return b2;
}

bool Canvas::drawEarthToHellLine(Graphics* graphics, int n, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	bool b2 = false;
	int height = this->imgTravelPath->height;
	int n2 = n;
	if (n2 > height) {
		b2 = true;
		n2 = height;
	}
	int n3 = b ? 0 : (height - n2);
	int width = this->imgTravelPath->width;
	if (n > 2 * height / 3) {
		Text* smallBuffer = app->localization->getSmallBuffer();
		smallBuffer->setLength(0);
		if (!b) {
			app->localization->composeText((short)3, (short)166, smallBuffer);
			smallBuffer->dehyphenate();
			graphics->drawString(smallBuffer, Canvas::hellNameCoords[0], Canvas::hellNameCoords[1], 4);
		}
		else {
			app->localization->composeText((short)3, (short)164, smallBuffer);
			smallBuffer->dehyphenate();
			graphics->drawString(smallBuffer, Canvas::earthNameCoords[2], Canvas::earthNameCoords[3], 4);
		}
		smallBuffer->dispose();
	}
	graphics->drawRegion(this->imgTravelPath, 0, n3, width, n2, Canvas::hellPathCoords[0], Canvas::hellPathCoords[1] + n3, 0, 0, 0);
	return b2;
}

void Canvas::drawLocatorBoxAndName(Graphics* graphics, bool b, int n, Text* text) {
	int n2;
	int n3;
	if (this->onMoon(n)) {
		n2 = Canvas::moonCoords[0];
		n3 = Canvas::moonCoords[1];
	}
	else if (this->onEarth(n)) {
		n2 = Canvas::earthCoords[0];
		n3 = Canvas::earthCoords[1];
	}
	else {
		n2 = Canvas::hellCoords[0];
		n3 = Canvas::hellCoords[1];
	}
	short n4 = (short)(2 * (n - 1));
	int n5 = Canvas::CROSS_HAIR_CORDS[n4] + this->xDiff + n2;
	int n6 = Canvas::CROSS_HAIR_CORDS[n4 + 1] + this->yDiff + n3;
	if (b) {
		graphics->drawImage(this->imgMagGlass, n5, n6, 3, 0, 0);
	}

	int x = Canvas::LOCATOR_BOX_CORDS[n4] + this->xDiff + n2;
	int y = Canvas::LOCATOR_BOX_CORDS[n4 + 1] + this->yDiff + n3 + (this->imgNameHighlight->height >> 1);
	graphics->drawImage(this->imgNameHighlight, x, y, 6, 0, 0);
	this->graphics.currentCharColor = 3;
	graphics->drawString(text, x + (this->imgNameHighlight->width >> 1), y, 3);
}

void Canvas::drawGridLines(Graphics* graphics, int i) {
	int iVar1;

	for (iVar1 = (i / 200) % 44 + this->displayRect[0]; iVar1 < this->displayRect[2]; iVar1 += 44) {
		graphics->drawImage(this->imgMapVertGridLines, iVar1, this->displayRect[1], 0x14, 0, 2);
	}
	for (iVar1 = this->displayRect[1] + 5; iVar1 < this->displayRect[3]; iVar1 += 44) {
		graphics->drawImage(this->imgMapHorzGridLines, this->displayRect[0], iVar1, 20, 0, 2);
		graphics->drawImage(this->imgMapHorzGridLines, 240, iVar1, 20, 0, 2);
	}
}

bool Canvas::onMoon(int n) {
	return n >= 1 && n <= 3;
}

bool Canvas::onEarth(int n) {
	return n >= 4 && n <= 6;
}

bool Canvas::inHell(int n) {
	return n >= 7;
}

void Canvas::handleTravelMapInput(int key, int action) {
#if 0 // IOS
	if ((this->stateVars[6] == 1) || (key == 18) || (action == Enums::ACTION_FIRE)) {
		this->finishTravelMapAndLoadLevel();
	}
	else {
		this->stateVars[6] = 1;
	}
#else // J2ME/BREW
	Applet* app = CAppContainer::getInstance()->app;
	bool hasSavedState = app->game->hasSavedState();

	if (action == Enums::ACTION_MENU) { // [GEC] skip all
		this->finishTravelMapAndLoadLevel();
		return;
	}

	if (action == Enums::ACTION_FIRE) {
		if (this->stateVars[6] == 1) {
#if 0 // [GEC] no esta disponible en ningina de las versiones del juego
			if (this->TM_LastLevelId == 0 && onMoon(this->TM_LoadLevelId)) {
				this->stateVars[5] = 1; // Draw start field
			}
			else {
				this->finishTravelMapAndLoadLevel();
			}
#else
			this->finishTravelMapAndLoadLevel();
#endif
		}
		else if (hasSavedState) {
			if (this->stateVars[2] == 1) {
				if (this->stateVars[8] == 0) {
					this->stateVars[3] = 1;
					this->stateVars[4] = 1;
					this->stateVars[6] = 1;
				}
				else {
					int n3 = app->upTimeMs - this->stateVars[0];
					this->totalTMTimeInPastAnimations += n3;
					this->stateVars[0] += n3;
					if (this->stateVars[4] == 1) {
						this->stateVars[4] = 0;
						this->stateVars[8] = 0;
						int n5 = this->earthCoords[0];
						int n6 = this->earthCoords[1];
						short n7 = (short)(2 * (this->TM_LoadLevelId - 1));
						this->targetX = this->CROSS_HAIR_CORDS[n7] + this->xDiff + n5;
						this->targetY = this->CROSS_HAIR_CORDS[n7 + 1] + this->yDiff + n6;
					}
					else {
						this->stateVars[3] = 1;
						this->stateVars[4] = 1;
					}
				}
			}
			else {
				this->totalTMTimeInPastAnimations += app->upTimeMs - this->stateVars[0];
				this->stateVars[0] = app->upTimeMs;
				this->stateVars[1] = 1;
				this->stateVars[2] = 1;
				if (this->newLevelSamePlanet()) {
					this->stateVars[3] = 1;
				}
			}
		}
	}
	else if (action == Enums::ACTION_AUTOMAP && this->stateVars[5] == 1) {
		this->finishTravelMapAndLoadLevel();
	}
#endif
}
void Canvas::finishTravelMapAndLoadLevel() {
	this->clearSoftKeys();
	this->disposeTravelMap();
	this->setLoadingBarText((short)0, (short)41);
	this->setState(Canvas::ST_LOADING);
}

bool Canvas::drawLocatorLines(Graphics* graphics, int n, bool b, bool b2) {
	int targetX;
	int targetY;
	int targetX2;
	int targetY2;
	if (n >= 0) {
		int n2 = n / (b ? 15 : 7);
		if (b) {
			int n3;
			int n4;
			if (this->onMoon(this->TM_LastLevelId)) {
				n3 = Canvas::moonCoords[0];
				n4 = Canvas::moonCoords[1];
			}
			else if (this->onEarth(this->TM_LastLevelId)) {
				if (b2) {
					n3 = Canvas::earthCoords[2];
					n4 = Canvas::earthCoords[3];
				}
				else {
					n3 = Canvas::earthCoords[0];
					n4 = Canvas::earthCoords[1];
				}
			}
			else {
				n3 = Canvas::hellCoords[0];
				n4 = Canvas::hellCoords[1];
			}
			short n5 = (short)(2 * (this->TM_LastLevelId - 1));
			targetX = Canvas::CROSS_HAIR_CORDS[n5] + this->xDiff + n3;
			targetY = Canvas::CROSS_HAIR_CORDS[n5 + 1] + this->yDiff + n4;
		}
		else {
			targetX = this->displayRect[2] - this->xDiff;
			targetY = this->displayRect[3] - this->yDiff;
		}
		int n6 = (this->targetX > targetX) ? 1 : ((this->targetX == targetX) ? 0 : -1);
		int n7 = (this->targetY > targetY) ? 1 : ((this->targetY == targetY) ? 0 : -1);
		targetX2 = targetX + n6 * n2;
		targetY2 = targetY + n7 * n2;
	}
	else {
		targetX2 = (targetX = this->targetX);
		targetY2 = (targetY = this->targetY);
	}
	bool b3 = false;
	bool b4 = false;
	if ((this->targetX - targetX) * (this->targetX - targetX2) < 0 || this->targetX == targetX2) {
		targetX2 = this->targetX;
		b3 = true;
	}
	if ((this->targetY - targetY) * (this->targetY - targetY2) < 0 || this->targetY == targetY2) {
		targetY2 = this->targetY;
		b4 = true;
	}
	graphics->drawLine(targetX2 + 1, this->displayRect[1], targetX2 + 1, this->displayRect[3], 0xFF000000);
	graphics->drawLine(this->displayRect[0], targetY2 + 1, this->displayRect[2], targetY2 + 1, 0xFF000000);
	graphics->drawLine(targetX2, this->displayRect[1], targetX2, this->displayRect[3], 0xFFBDFD80);
	graphics->drawLine(this->displayRect[0], targetY2, this->displayRect[2], targetY2, 0xFFBDFD80);
	return b3 && b4;
}

void Canvas::initTravelMap() {
	Applet* app = CAppContainer::getInstance()->app;

	this->TM_LoadLevelId = (short)std::max(1, std::min(this->loadMapID, 9));
	this->TM_LastLevelId = ((getRecentLoadType() == 1 && !this->TM_NewGame) ? this->TM_LoadLevelId : ((short)std::max(0, std::min(this->lastMapID, 9))));

	if (this->TM_LastLevelId == 0 && this->TM_LoadLevelId > 1) {
		this->TM_LastLevelId = (short)(this->TM_LoadLevelId - 1);
	}
	short n;
	if (this->TM_LoadLevelId > this->TM_LastLevelId) {
		n = 0;
	}
	else if (this->TM_LoadLevelId == this->TM_LastLevelId) {
		n = 1;
	}
	else {
		n = 2;
	}
	app->game->scriptStateVars[15] = n;

	app->beginImageLoading();
	this->imgNameHighlight = app->loadImage("highlight.bmp", true);
	this->imgMagGlass = app->loadImage("magnifyingGlass.bmp", true);
	this->imgSpaceShip = app->loadImage("spaceShip.bmp", true);

	bool b = false;
	if (onMoon(this->TM_LoadLevelId)) {
		this->imgTierCloseUp = app->loadImage("TM_Levels1.bmp", true);
	}
	else if (onEarth(this->TM_LoadLevelId)) {
		if (onMoon(this->TM_LastLevelId) || inHell(this->TM_LastLevelId)) {
			this->imgEarthCloseUp = app->loadImage("TM_Levels2.bmp", true);
			b = true;
		}
		this->imgTierCloseUp = app->loadImage("TM_Levels4.bmp", true);
	}
	else {
		this->imgTierCloseUp = app->loadImage("TM_Levels3.bmp", true);
	}

	if (onMoon(this->TM_LoadLevelId) && this->TM_LastLevelId == 0) {
		this->imgTravelPath = app->loadImage("toMoon.bmp", true);
	}
	else if ((onEarth(this->TM_LoadLevelId) && onMoon(this->TM_LastLevelId)) || (onMoon(this->TM_LoadLevelId) && onEarth(this->TM_LastLevelId))) {
		this->imgTravelPath = app->loadImage("toEarth.bmp", true);
	}
	else if ((inHell(this->TM_LoadLevelId) && onEarth(this->TM_LastLevelId)) || (onEarth(this->TM_LoadLevelId) && inHell(this->TM_LastLevelId))) {
		this->imgTravelPath = app->loadImage("toHell.bmp", true);
	}

	this->imgTravelBG = app->loadImage("TravelMap.bmp", true);
	this->imgMapHorzGridLines = app->loadImage("travelMapHorzGrid.bmp", true);
	this->imgMapVertGridLines = app->loadImage("travelMapVertGrid.bmp", true);
	app->endImageLoading();

	this->totalTMTimeInPastAnimations = 0;
	this->mapWidth = this->imgTravelBG->width;
	this->mapHeight = this->imgTravelBG->height;
	this->xDiff = std::max(0, (this->displayRect[2] - this->imgTravelBG->width) / 2);
	this->yDiff = std::max(0, (this->displayRect[3] - this->imgTravelBG->height) / 2);

	int n2;
	int n3;
	if (onMoon(this->TM_LoadLevelId)) {
		n2 = 137;
		n3 = 216;
	}
	else if (onEarth(this->TM_LoadLevelId)) {
		if (b) {
			n2 = 123;
			n3 = 150;
		}
		else {
			n2 = 123;
			n3 = 150;
		}
	}
	else {
		n2 = 150;
		n3 = 11;
	}

	if (!b) {
		short n4 = (short)(2 * (this->TM_LoadLevelId - 1));
		this->targetX = Canvas::CROSS_HAIR_CORDS[n4] + this->xDiff + n2;
		this->targetY = Canvas::CROSS_HAIR_CORDS[n4 + 1] + this->yDiff + n3;
	}
	else {
		this->targetX = Canvas::UAC_BUILDING_LOCATION_ON_EARTH[0] + this->xDiff + n2;
		this->targetY = Canvas::UAC_BUILDING_LOCATION_ON_EARTH[1] + this->yDiff + n3;
	}

	this->stateVars[0] = app->upTimeMs;;
	if (b) {
		this->stateVars[8] = 1;
	}

	// Unused
	this->imgStarField = app->loadImage("cockpit.bmp", true);
	this->_field_0xf24 = 480;
	this->_field_0xf20 = this->imgStarField->height;
	this->_field_0xf28 = 0;
	this->_field_0xf2c = 1;
}

void Canvas::disposeTravelMap() {
	this->imgNameHighlight->~Image();
	this->imgNameHighlight = nullptr;
	this->imgMagGlass->~Image();
	this->imgMagGlass = nullptr;
	this->imgTravelBG->~Image();
	this->imgTravelBG = nullptr;
	this->imgTravelPath->~Image();
	this->imgTravelPath = nullptr;
	this->imgSpaceShip->~Image();
	this->imgSpaceShip = nullptr;
	this->imgTierCloseUp->~Image();
	this->imgTierCloseUp = nullptr;
	this->imgEarthCloseUp->~Image();
	this->imgEarthCloseUp = nullptr;
	this->imgMapHorzGridLines->~Image();
	this->imgMapHorzGridLines = nullptr;
	this->imgMapVertGridLines->~Image();
	this->imgMapVertGridLines = nullptr;

	// Unused
	this->imgStarField->~Image();
	this->imgStarField = nullptr;
}

void Canvas::drawStarFieldPage(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->upTimeMs - this->stateVars[0] > 5250) {
		this->_field_0xf2c = 2u;
		this->finishTravelMapAndLoadLevel();
	}
	else {
		int yPos = this->screenRect[3] - this->screenRect[1] - this->imgStarField->height;
		graphics->fillRect(0, 0, this->menuRect[2], this->menuRect[3], 0xFF000000);
		this->drawStarField(graphics, 1, yPos / 2);
		graphics->drawImage(this->imgStarField, 1, yPos / 2, 0, 0, 0);
		graphics->drawImage(this->imgStarField, this->screenRect[2] - 1, yPos / 2, 24, 4, 0);
		app->canvas->softKeyRightID = -1;
		app->canvas->softKeyLeftID = -1;
		app->canvas->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
		if (app->canvas->displaySoftKeys) {
			app->canvas->softKeyRightID = 40;
			app->canvas->repaintFlags |= Canvas::REPAINT_SOFTKEYS;
		}
	}
}

void Canvas::drawStarField(Graphics* graphics, int x, int y) {
	Applet* app = CAppContainer::getInstance()->app;
	int upTimeMs; // r2
	int result; // r0
	int field_0xf20; // r2
	unsigned int v8; // r2
	unsigned int v9; // r11
	int v10; // r6
	int v11; // r0
	signed int v12; // r5
	int v13; // r6
	int v14; // r3
	int v15; // r0
	int i; // r4
	int v17; // r10
	int v18; // r8
	int v23; // [sp+18h] [bp-2Ch]
	int v24; // [sp+1Ch] [bp-28h]
	int v25; // [sp+20h] [bp-24h]
	int v26; // [sp+24h] [bp-20h]
	int v27; // [sp+28h] [bp-1Ch]

	graphics->fillRect(x, y, this->_field_0xf24, this->_field_0xf20, 0xFF000000);
	if (app->upTimeMs - this->stateVars[7] > 59) {
		this->stateVars[7] = app->upTimeMs;
		this->runStarFieldFrame();
	}

	v23 = this->_field_0xf24 / 2;
	field_0xf20 = this->_field_0xf20;
	v26 = y;
	v25 = 0;
	v24 = field_0xf20 / 2;
	v27 = field_0xf20 / -2;
LABEL_20:
	if (v25 < field_0xf20 - this->_field_0xf28)
	{
		v17 = x;
		v18 = -v23;
		for (i = 0; ; ++i)
		{
			if (i >= this->_field_0xf24)
			{
				++v25;
				++v26;
				++v27;
				field_0xf20 = this->_field_0xf20;
				goto LABEL_20;
			}
			v8 = app->tinyGL->pixels[i + this->_field_0xf24 * v25];
			v9 = v8 >> 10;
			if ((v8 & 0x3FF) != 0)
				break;
		LABEL_17:
			++v17;
			++v18;
		}
		v10 = (int)(abs(v18) << 10) / v23;
		v11 = (int)(abs(v27) << 10) / v24;
		v12 = v10 + v11 + ((unsigned int)(v10 + v11) >> 31);
		v13 = (v10 + v11) / 2;
		graphics->setColor(65793 * ((v13 << 7 >> 10) + 128) - 0x1000000);
		if (v9 == 1)
		{
			v15 = 8 * v13;
		}
		else
		{
			if (!v9 || v9 != 2)
			{
				v14 = v12 >> 11;
			LABEL_9:
				if (v14 <= 0 || v14 == 1)
				{
					graphics->fillRect(v17, v26, 1, 1);
				}
				else
				{
					graphics->fillCircle(v17, v26, v14);
				}
				goto LABEL_17;
			}
			v15 = 10 * v13;
		}
		v14 = v15 >> 10;
		goto LABEL_9;
	}
}

void Canvas::runStarFieldFrame() {
	Applet* app = CAppContainer::getInstance()->app;

	int v1; // r11
	int v2; // r3
	TinyGL* tinyGL; // r5
	unsigned int v4; // r3
	unsigned int v5; // r3
	int v6; // r6
	signed int v7; // r0
	int* v8; // r10
	int v9; // r0
	int v10; // r4
	int v11; // r8
	unsigned int v12; // r3
	TinyGL* v13; // r5
	unsigned int v14; // r3
	unsigned int v15; // r3
	int v16; // r6
	signed int v17; // r0
	int* v18; // r10
	int v19; // r0
	int v20; // r4
	int v21; // r8
	int v22; // r11
	unsigned int v23; // r3
	int v24; // r6
	int v25; // r10
	int v26; // r4
	int v27; // r5
	int v28; // r0
	int result; // r0
	int v30; // [sp+0h] [bp-78h]
	int v31; // [sp+4h] [bp-74h]
	int v32; // [sp+8h] [bp-70h]
	int v33; // [sp+Ch] [bp-6Ch]
	int v34; // [sp+10h] [bp-68h]
	int v35; // [sp+14h] [bp-64h]
	int v38; // [sp+20h] [bp-58h]
	int v39; // [sp+24h] [bp-54h]
	int v40; // [sp+28h] [bp-50h]
	int v41; // [sp+30h] [bp-48h]
	int i; // [sp+34h] [bp-44h]
	int v43; // [sp+3Ch] [bp-3Ch]
	unsigned short* v44; // [sp+44h] [bp-34h]
	unsigned short* pixels; // [sp+48h] [bp-30h]
	int v46; // [sp+4Ch] [bp-2Ch]
	int v47; // [sp+54h] [bp-24h]
	int v48; // [sp+58h] [bp-20h]
	int v49; // [sp+5Ch] [bp-1Ch]

	v1 = this->_field_0xf24;
	v38 = v1 / 2;
	v2 = this->_field_0xf20;
	v39 = v2 / 2;
	for (i = 0; i < v2 / 2; ++i)
	{
		v10 = 0;
		v48 = v39 - i;
		v12 = abs(v39 - i);
		v35 = 60 * v12;
		v34 = 20 * v12;
		v11 = -v38;
		while (v10 < v1)
		{
			tinyGL = app->tinyGL;
			pixels = tinyGL->pixels;
			v4 = pixels[v10 + v1 * i];
			v47 = v4 & 0x3FF;
			if ((v4 & 0x3FF) != 0)
			{
				v5 = v4 >> 10;
				if (v5 == 1)
				{
					v6 = v34;
					v7 = 20 * abs(v11);
				}
				else if (v5)
				{
					v6 = v35;
					v7 = 60 * abs(v11);
				}
				else
				{
					v6 = 300;
					v7 = 300;
				}
				v31 = this->_field_0xf2c;
				v8 = app->render->sinTable;
				v46 = (v8[(v47 + 256) & 0x3FF] * (v7 / v31) / v38) >> 16;
				v9 = (v8[v47] * (v6 / v31) / v39) >> 16;
				if (v48 + v9 >= -v39 && v39 > v48 + v9 && -v38 <= v11 + v46 && v38 > v11 + v46)
				{
					pixels[v10 + v1 * (i - v9) + v46] = v47 | ((short)v5 << 10);
					v1 = this->_field_0xf24;
					tinyGL = app->tinyGL;
				}
				tinyGL->pixels[v10 + v1 * i] = 0;
				v1 = this->_field_0xf24;
			}
			++v10;
			++v11;
		}
		v2 = this->_field_0xf20;
	}
	v49 = v2 - 1;
	v43 = v39 - (v2 - 1);
	while (v49 >= v2 / 2)
	{
		v20 = 0;
		v23 = abs(v43);
		v21 = -v38;
		v33 = 60 * v23;
		v32 = 20 * v23;
		while (v20 < v1)
		{
			v13 = app->tinyGL;
			v44 = v13->pixels;
			v14 = v44[v20 + v1 * v49];
			v40 = v14 & 0x3FF;
			if ((v14 & 0x3FF) != 0)
			{
				v15 = v14 >> 10;
				if (v15 == 1)
				{
					v16 = v32;
					v17 = 20 * abs(v21);
				}
				else if (v15)
				{
					v16 = v33;
					v17 = 60 * abs(v21);
				}
				else
				{
					v16 = 300;
					v17 = 300;
				}
				v30 = this->_field_0xf2c;
				v18 = app->render->sinTable;
				v41 = (v18[(v40 + 256) & 0x3FF] * (v17 / v30) / v38) >> 16;
				v19 = (v18[v40] * (v16 / v30) / v39) >> 16;
				if (v43 + v19 >= -v39 && v39 > v43 + v19 && v21 + v41 >= -v38 && v38 > v21 + v41)
				{
					v44[v20 + v1 * (v49 - v19) + v41] = v40 | ((short)v15 << 10);
					v1 = this->_field_0xf24;
					v13 = app->tinyGL;
				}
				v13->pixels[v20 + v1 * v49] = 0;
				v1 = this->_field_0xf24;
			}
			++v20;
			++v21;
		}
		--v49;
		++v43;
		v2 = this->_field_0xf20;
	}
	v22 = 0;
	while (1)
	{
		result = 4 / this->_field_0xf2c;
		if (v22 >= result)
			break;
		++v22;
		v24 = app->nextInt() % 1023;
		v25 = v24 + 1;
		v26 = app->nextInt() % 3;
		v27 = app->nextInt() % 15 + 1;
		if ((unsigned int)(v24 - 256) <= 0x1FE)
			v27 = -v27;
		v28 = app->nextInt() % 15 + 1;
		if (v25 >= 512)
			v28 = -v28;
		app->tinyGL->pixels[v27 + v38 + this->_field_0xf24 * (v39 - v28)] = v25 | ((short)v26 << 10);
	}
}

void Canvas::playIntroMovie(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->canvas->skipIntro != false) {
		this->backToMain(true);
		return;
	}

	if (app->game->hasSeenIntro && app->game->skipMovie) {
		this->exitIntroMovie(false);
		return;
	}

	if (this->stateVars[1] == 0) {  // load table camera
		this->stateVars[1] = app->gameTime;
		app->game->loadTableCamera(14, 15);
		this->numEvents = 0;
		this->keyDown = false;
		this->keyDownCausedMove = false;
		this->ignoreFrameInput = 1;
		this->imgProlog = app->loadImage("prolog.bmp", true);
	}

	if (app->game) {
		if (app->game->mayaCameras)
		{
			if (this->stateVars[3] == 0) { // init movie prolog
				app->sound->playSound(1068, 1, 6, false);
				if (this->stateVars[1] < app->gameTime) {
					app->game->activeCameraKey = -1;
					this->stateVars[3] = 0;
					this->stateVars[1] = app->gameTime;
					this->stateVars[2] = 0;
					this->stateVars[0] = 0;
					this->fadeRect = this->displayRect;
					this->fadeFlags = Canvas::FADE_FLAG_FADEIN;
					this->fadeColor = 0;
					this->fadeTime = app->time;
					this->fadeDuration = 1500;
					this->stateVars[3] = 1;
				}
			}
			else if (this->stateVars[3] == 1) { // draw movie prolog
				if (this->displayRect[3] > 220) {
					graphics->clipRect(0, (this->displayRect[3] - 220) / 2, this->displayRect[2], 220);
				}

				this->stateVars[0] = app->gameTime - app->game->activeCameraTime;
				this->stateVars[2] = app->gameTime - this->stateVars[1];

				app->game->mayaCameras->Update(app->game->activeCameraKey, this->stateVars[0]);

				int uVar3 = app->game->posShift;
				int texW = this->displayRect[2];
				int texH = this->displayRect[3];

				int posX = app->game->mayaCameras->x >> (uVar3 & 0xff);
				int posY = app->game->mayaCameras->y >> (uVar3 & 0xff);

				int texX = posX - this->SCR_CX; 
				if (texX < 0) {
					texX = 0;
				}

				int texY = posY - this->SCR_CY;
				if (texY < 0) {
					texY = 0;
				}

				posY = this->SCR_CY - posY;
				if (posY < 0) {
					posY = 0;
				}

				if (texX + texW > this->imgProlog->width) {
					texW = this->imgProlog->width - texX;
				}

				if (texY + texH > this->imgProlog->height) {
					texH = this->imgProlog->height - texY;
				}

				graphics->drawRegion(this->imgProlog, texX, texY, texW, texH, 0, posY, 0, 0, 0);
				if (app->game->mayaCameras->complete) {
					this->stateVars[3] += 1;
				}
			}
			else if (this->stateVars[3] == 2) { // init Scrolling Text
				this->initScrollingText(0, 133, false, 32, 1, 800);
				this->drawScrollingText(graphics);
				this->stateVars[3] += 1;
			}
			else if (this->stateVars[3] == 3) { // draw Scrolling Text
				this->drawScrollingText(graphics);
				if (this->scrollingTextDone) {
					this->exitIntroMovie(false);
				}
			}

			this->staleView = true;
			return;
		}
	}
}

void Canvas::exitIntroMovie(bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	app->game->cleanUpCamMemory();

	this->imgProlog->~Image();
	this->imgProlog = nullptr;

	if (this->dialogBuffer) {
		this->dialogBuffer->dispose();
		this->dialogBuffer = nullptr;
	}

	app->sound->soundStop();
	if (b == false) {
		app->game->hasSeenIntro = true;
		app->game->saveConfig();
		this->backToMain(false);
	}
}

void Canvas::setMenuDimentions(int x, int y, int w, int h)
{
	this->menuRect[0] = x;
	this->menuRect[1] = y;
	this->menuRect[2] = w;
	this->menuRect[3] = h;
}

void Canvas::setBlendSpecialAlpha(float alpha) {

	if (alpha < 0.0f) {
		alpha = 0.0f;
	}
	else if (alpha > 1.0f) {
		alpha = 1.0f;
	}

	this->blendSpecialAlpha = alpha;
}

void Canvas::touchStart(int pressX, int pressY) {
	Applet* app = CAppContainer::getInstance()->app;
	this->touched = false;

	if (this->state == Canvas::ST_MENU) {
		app->menuSystem->handleUserTouch(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_LOOTING) {
		this->handleEvent(6);
	}
	else if ((this->state == Canvas::ST_PLAYING) || (this->state == Canvas::ST_COMBAT)) {
		if (!this->isZoomedIn) {
			app->hud->handleUserTouch(pressX, pressY, true);

			if (!app->hud->isInWeaponSelect) {
				this->m_controlButtons[this->m_controlMode + 0]->HighlightButton(pressX, pressY, true);
				this->m_controlButtons[this->m_controlMode + 2]->HighlightButton(pressX, pressY, true);
				this->m_controlButtons[this->m_controlMode + 4]->HighlightButton(pressX, pressY, true);

				this->m_controlButton = nullptr;
				fmButton* button;
				if (!this->isZoomedIn && (
					(button = this->m_controlButtons[this->m_controlMode + 0]->GetTouchedButton(pressX, pressY)) ||
					(button = this->m_controlButtons[this->m_controlMode + 2]->GetTouchedButton(pressX, pressY)) ||
					(button = this->m_controlButtons[this->m_controlMode + 4]->GetTouchedButton(pressX, pressY))) &&
					(button->buttonID != 6)) {
					this->m_controlButton = button;
					this->m_controlButtonIsTouched = true;
				}
				else if (this->m_swipeArea[this->m_controlMode]->rect.ContainsPoint(pressX, pressY))
				{
					this->m_swipeArea[this->m_controlMode]->touched = true;
					this->m_swipeArea[this->m_controlMode]->begX = pressX;
					this->m_swipeArea[this->m_controlMode]->begY = pressY;
					this->m_swipeArea[this->m_controlMode]->curX = -1;
					this->m_swipeArea[this->m_controlMode]->curY = -1;
					return;
				}
				this->m_swipeArea[this->m_controlMode]->touched = false;
			}
		}
		else {
			if (this->m_sniperScopeDialScrollButton->field_0x0_ &&
				this->m_sniperScopeDialScrollButton->barRect.ContainsPoint(pressX, pressY)) {
				this->m_sniperScopeDialScrollButton->SetTouchOffset(pressX, pressY);
				this->m_sniperScopeDialScrollButton->field_0x14_ = 1;
			}
			else {
				app->hud->handleUserTouch(pressX, pressY, true);
				if (!app->hud->isInWeaponSelect) {
					this->m_sniperScopeButtons->HighlightButton(pressX, pressY, true);
				}
			}
		}
	}
	else if (this->state == Canvas::ST_AUTOMAP) {
		//puts("touch in automap!");
		this->m_softKeyButtons->HighlightButton(pressX, pressY, true);
		this->m_controlButtons[this->m_controlMode + 0]->HighlightButton(pressX, pressY, true);
		this->m_controlButtons[this->m_controlMode + 2]->HighlightButton(pressX, pressY, true);
		this->m_controlButton = nullptr;
		fmButton* button;
		if (((button = this->m_controlButtons[this->m_controlMode + 0]->GetTouchedButton(pressX, pressY)) ||
			(button = this->m_controlButtons[this->m_controlMode + 2]->GetTouchedButton(pressX, pressY))) &&
			(button->buttonID != 6)) {
			this->m_controlButton = button;
			this->m_controlButtonIsTouched = true;
		}
	}
	else if (this->state == Canvas::ST_MIXING) {
		this->m_mixingButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_INTRO) {
		this->m_storyButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_DIALOG) {
		this->m_dialogButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_TREADMILL) {
		this->m_treadmillButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_CHARACTER_SELECTION) {
		this->m_characterButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_MINI_GAME) {
		if (app->canvas->stateVars[0] == 2) {
			app->hackingGame->touchStart(pressX, pressY);
		}
		else if (app->canvas->stateVars[0] == 4) {
			app->vendingMachine->touchStart(pressX, pressY);
		}
		else if (app->canvas->stateVars[0] == 0) {
			app->sentryBotGame->touchStart(pressX, pressY);
		}
	}
	else if (this->state == Canvas::ST_CAMERA) { // [GEC ]Port: New
		this->m_softKeyButtons->HighlightButton(pressX, pressY, true);
	}
}

void Canvas::touchMove(int pressX, int pressY) {
	Applet* app = CAppContainer::getInstance()->app;
	fmSwipeArea::SwipeDir swDir;

	//this->touched = true; // Old

	// [GEC] Evita falsos toques en la pantalla
	const int begMouseX = (int)(gBegMouseX * Applet::IOS_WIDTH);
	const int begMouseY = (int)(gBegMouseY * Applet::IOS_HEIGHT);
	if (!pointInRectangle(pressX, pressY, begMouseX - 3, begMouseY - 3, 6, 6)) {
		this->touched = true;
	}

	if (this->state == Canvas::ST_MENU) {
		app->menuSystem->handleUserMoved(pressX, pressY);
	}
	else if ((this->state == Canvas::ST_PLAYING) || (this->state == Canvas::ST_COMBAT)) {

		if (this->isZoomedIn) {
			if (this->m_sniperScopeDialScrollButton->field_0x14_) {
				this->m_sniperScopeDialScrollButton->Update(pressX, pressY);
				int field_0x44 = this->m_sniperScopeDialScrollButton->field_0x44_;
				if (!this->m_sniperScopeDialScrollButton->field_0x0_ || (field_0x44 <= 2)) {
					field_0x44 = 3;
				}
				this->zoomDestFOV = 110 * field_0x44 / 15 + 80;

				// [GEC] update zoomCurFOVPercent
				{
					float maxScroll = (float)((this->m_sniperScopeDialScrollButton->barRect).h - this->m_sniperScopeDialScrollButton->field_0x4c_);
					float yScroll = (float)((float)this->m_sniperScopeDialScrollButton->field_0x48_ / maxScroll);
					this->zoomCurFOVPercent = this->zoomMinFOVPercent - (int)((float)this->zoomMinFOVPercent * yScroll);
				}
			}
			else {
				if (this->m_sniperScopeDialScrollButton->field_0x0_
					&& this->m_sniperScopeDialScrollButton->barRect.ContainsPoint(pressX, pressY))
				{
					this->m_sniperScopeDialScrollButton->SetTouchOffset(pressX, pressY);
					this->m_sniperScopeDialScrollButton->field_0x14_ = 1;
					return;
				}

				app->hud->handleUserMoved(pressX, pressY);
				if (!app->hud->isInWeaponSelect) {
					this->m_sniperScopeButtons->HighlightButton(pressX, pressY, true);
				}
			}
		}
		else {
			app->hud->handleUserMoved(pressX, pressY);
			if (!app->hud->isInWeaponSelect) {
				if (this->m_swipeArea[this->m_controlMode]->touched) {
					if (this->m_swipeArea[this->m_controlMode]->UpdateSwipe(pressX, pressY, &swDir)) {
						this->touched = true;
						this->touchSwipe(swDir);
						return;
					}
				}
				else {
					this->m_controlButtons[this->m_controlMode + 0]->HighlightButton(pressX, pressY, true);
					this->m_controlButtons[this->m_controlMode + 2]->HighlightButton(pressX, pressY, true);
					this->m_controlButtons[this->m_controlMode + 4]->HighlightButton(pressX, pressY, true);

					if (this->m_controlButton && !this->m_controlButton->highlighted) {
						this->m_controlButton = nullptr;
					}
					this->m_controlButton = nullptr;

					fmButton* button;
					if (!this->isZoomedIn && (
						(button = this->m_controlButtons[this->m_controlMode + 0]->GetTouchedButton(pressX, pressY)) ||
						(button = this->m_controlButtons[this->m_controlMode + 2]->GetTouchedButton(pressX, pressY)) ||
						(button = this->m_controlButtons[this->m_controlMode + 4]->GetTouchedButton(pressX, pressY))) &&
						(button->buttonID != 6)) {
						this->m_controlButton = button;
						this->m_controlButtonIsTouched = true;
					}
				}
			}
		}
	}
	else if (this->state == Canvas::ST_AUTOMAP) {
		this->m_softKeyButtons->HighlightButton(pressX, pressY, 1);
		this->m_controlButtons[this->m_controlMode + 0]->HighlightButton(pressX, pressY, true);
		this->m_controlButtons[this->m_controlMode + 2]->HighlightButton(pressX, pressY, true);

		if (this->m_controlButton && !this->m_controlButton->highlighted) {
			this->m_controlButton = nullptr;
		}
		this->m_controlButton = nullptr;

		fmButton* button;
		if (((button = this->m_controlButtons[this->m_controlMode + 0]->GetTouchedButton(pressX, pressY)) ||
			(button = this->m_controlButtons[this->m_controlMode + 2]->GetTouchedButton(pressX, pressY))) &&
			(button->buttonID != 6)) {
			this->m_controlButton = button;
			this->m_controlButtonIsTouched = true;
		}
	}
	else if (this->state == Canvas::ST_MIXING) {
		this->m_mixingButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_INTRO) {
		this->m_storyButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_DIALOG) {
		this->m_dialogButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_TREADMILL) {
		this->m_treadmillButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_CHARACTER_SELECTION) {
		this->m_characterButtons->HighlightButton(pressX, pressY, true);
	}
	else if (this->state == Canvas::ST_MINI_GAME) {
		if (app->canvas->stateVars[0] == 2) {
			app->hackingGame->touchMove(pressX, pressY);
		}
		else if (app->canvas->stateVars[0] == 4) {
			app->vendingMachine->touchMove(pressX, pressY);
		}
		else if (app->canvas->stateVars[0] == 0) {
			app->sentryBotGame->touchMove(pressX, pressY);
		}
	}
	else if (this->state == Canvas::ST_CAMERA) { // [GEC]: New
		this->m_softKeyButtons->HighlightButton(pressX, pressY, true);
	}
}

void Canvas::touchEnd(int pressX, int pressY) {
	Applet* app = CAppContainer::getInstance()->app;

	short sVar1;
	int iVar3;
	int state;
	int uVar5;

	state = this->state;
	//printf("state %d\n", state);
	if (this->state == Canvas::ST_MENU) {
		app->menuSystem->handleUserTouch(pressX, pressY, false);
		return;
	}

	if (this->state == Canvas::ST_INTRO) {
		state = this->m_storyButtons->GetTouchedButtonID(pressX, pressY);
		if (state != 1) {
			if (state == 2) {
				if (this->storyPage >= this->storyTotalPages - 1) {
					return;
				}
			}
			else if (state != 0) {
				return;
			}
		}
		this->stateVars[0] = state;
	LAB_00022b00:
		state = this->numEvents;
		if (state == 4) {
			return;
		}
		iVar3 = 6;
	}
	else {
		if (this->state != Canvas::ST_DIALOG) {
			if (this->state == Canvas::ST_AUTOMAP) {
				this->m_controlButton = nullptr;
				state = this->m_softKeyButtons->GetTouchedButtonID(pressX, pressY);
				this->m_controlButtonTime = app->gameTime + -1;
			}
			else {
				if (this->state == Canvas::ST_COMBAT || this->state == Canvas::ST_PLAYING) {
					state = this->touchToKey_Play(pressX, pressY);
				}
				else {
					if (this->state == Canvas::ST_CAMERA) {
						if (app->canvas->softKeyRightID != -1) { // [GEC]: New
							this->m_softKeyButtons->HighlightButton(pressX, pressY, true);
						}

						if ((app->canvas->softKeyRightID != -1) &&
							(state = this->m_softKeyButtons->GetTouchedButtonID(pressX, pressY),
								state == 20)) {
						LAB_00022c60:
							
							state = 6;
							goto LAB_00022c64;
						}
					}
					else {
						if (this->state == Canvas::ST_MINI_GAME) {
							state = app->canvas->stateVars[0];
							if (state == 2) {
								app->hackingGame->touchEnd(pressX, pressY);
							}
							else {
								if (state == 4) {
									app->vendingMachine->touchEnd(pressX, pressY);
								}
								else {
									if (state == 0) {
										app->sentryBotGame->touchEnd(pressX, pressY);
									}
								}
							}
						}
						else {
							if (this->state == Canvas::ST_CHARACTER_SELECTION) { // [GEC]
								state = AVK_PASSTURN;
								goto LAB_00022c64;
							}
							if (this->state != Canvas::ST_TREADMILL) goto LAB_00022c60;
							state = this->m_treadmillButtons->GetTouchedButtonID(pressX, pressY);
							if (state == 0) {
								state = this->numEvents;
								if (state != 4) {
									iVar3 = 2;
								LAB_00022c48:
									this->events[state] = iVar3;
									this->numEvents = state + 1;
									this->keyPressedTime = app->upTimeMs;
								}
							}
							else {
								if (state == 1) {
									state = this->numEvents;
									if (state != 4) {
										iVar3 = 4;
										goto LAB_00022c48;
									}
								}
							}
						}
					}
					state = -1;
				}
			}
		LAB_00022c64:
			this->m_controlButtonIsTouched = false;
			if (state == -1) {
				return;
			}
			iVar3 = this->numEvents;
			if (iVar3 == 4) {
				return;
			}
			this->events[iVar3] = state;
			this->numEvents = iVar3 + 1;
			state = app->upTimeMs;
			goto LAB_00022ca4;
		}
		state = this->m_dialogButtons->GetTouchedButtonID(pressX, pressY);
		if (state - 7U < 2) {
			state = 6;
		LAB_00022944:
			if (this->currentDialogLine < this->numDialogLines - this->dialogViewLines) goto LAB_00022980;
			uVar5 = this->dialogFlags;
			if ((uVar5 & 2) != 0) {
				return;
			}
			if ((uVar5 & 4) != 0) {
				return;
			}
			if ((uVar5 & 1) != 0) {
				return;
			}
		LAB_00022af8:
			//pCVar4 = *Applet;
			goto LAB_00022b00;
		}
		if (state == 6) goto LAB_00022944;
	LAB_00022980:
		if ((1 < state - 5U) || (this->numDialogLines <= this->dialogViewLines)) {
			uVar5 = this->dialogFlags;
			if ((uVar5 & 2) == 0) {
				if (uVar5 == 0) {
					return;
				}
				if (this->currentDialogLine < this->numDialogLines - this->dialogViewLines) {
					return;
				}
				if (((uVar5 & 4) == 0) && ((uVar5 & 1) == 0)) {
					return;
				}
				if (1 < state - 3U) {
					return;
				}
				if (state == 3) {
					app->game->scriptStateVars[4] = 0;
				}
				else {
					app->game->scriptStateVars[4] = 1;
				}
			}
			else {
				iVar3 = this->dialogStyle;
				sVar1 = (short)state;
				if (iVar3 == 11) {
					if (state == 0) {
						app->game->scriptStateVars[4] = sVar1;
					}
					else {
						if (state != 1) {
							return;
						}
						app->game->scriptStateVars[4] = sVar1;
					}
					iVar3 = this->numEvents;
					if (iVar3 != 4) {
						this->events[iVar3] = 6;
						this->numEvents = iVar3 + 1;
						this->keyPressedTime = app->upTimeMs;
					}
					iVar3 = this->dialogStyle;
				}
				if (iVar3 != 10) {
					return;
				}
				if (state == 0) {
					app->game->scriptStateVars[4] = sVar1;
				}
				else {
					if (state != 1) {
						return;
					}
					app->game->scriptStateVars[4] = sVar1;
				}
			}
			goto LAB_00022af8;
		}
		if (state != 5) goto LAB_00022af8;
		state = this->numEvents;
		if (state == 4) {
			return;
		}
		iVar3 = 19;
	}
	this->events[state] = iVar3;
	this->numEvents = state + 1;
	state = app->upTimeMs;
LAB_00022ca4:
	this->keyPressedTime = state;
	return;
}

void Canvas::touchEndUnhighlight() {

	if (this->state == Canvas::ST_MIXING) {
		this->m_mixingButtons->HighlightButton(0, 0, false);
	}
	else if (this->state == Canvas::ST_INTRO) {
		this->m_storyButtons->HighlightButton(0, 0, false);
	}

	if (this->state == Canvas::ST_PLAYING || this->state == Canvas::ST_COMBAT || this->state == Canvas::ST_AUTOMAP || this->state == Canvas::ST_DIALOG || this->state == Canvas::ST_CAMERA) {
		this->m_controlButtons[this->m_controlMode]->HighlightButton(0, 0, false);
		this->m_controlButtons[this->m_controlMode + 2]->HighlightButton(0, 0, false);
		this->m_controlButtons[this->m_controlMode + 4]->HighlightButton(0, 0, false);
		this->m_sniperScopeButtons->HighlightButton(0, 0, false);
		this->m_softKeyButtons->HighlightButton(0, 0, false);
		this->m_dialogButtons->HighlightButton(0, 0, false);
		this->m_characterButtons->HighlightButton(0, 0, false);
	}
	else if (this->state == Canvas::ST_TREADMILL) {
		this->m_treadmillButtons->HighlightButton(0, 0, false);
	}
}

void Canvas::initMiniGameHelpScreen() {
	this->miniGameHelpScrollPosition = 0;
}

void Canvas::drawMiniGameHelpScreen(Graphics* graphics, int i, int i2, Image* image) {
	graphics->drawImage(image, 0, 0, 0, 0, 0);
	this->drawMiniGameHelpText(graphics, i, i2);
}

void Canvas::drawMiniGameHelpText(Graphics* graphics, int i, int i2) {
	Applet* app = CAppContainer::getInstance()->app;

	int x;
	Text* textBuff1;
	int iVar1;
	Text* textBuff2;
	int iVar2;
	Text* textBuff3;
	int iVar3;

	textBuff1 = app->localization->getSmallBuffer();
	x = (this->screenRect[2] - this->screenRect[0]) / 2;
	textBuff1->setLength(0);
	app->localization->composeText(i, textBuff1);
	textBuff1->dehyphenate();
	graphics->drawString(textBuff1, x, this->screenRect[1] + 0x19, 3);

	textBuff1->setLength(0);
	app->localization->composeText(i2, textBuff1);
	textBuff1->wrapText(0x31, '\n');
	iVar1 = textBuff1->getNumLines();
	this->helpTextNumberOfLines = iVar1;

	textBuff2 = app->localization->getSmallBuffer();
	textBuff2->setLength(0);
	for (iVar1 = 0; iVar1 < this->miniGameHelpScrollPosition; iVar1 = iVar1 + 1) {
		iVar3 = textBuff1->findFirstOf('\n');
		textBuff3 = textBuff1;
		if (iVar3 != -1) {
			iVar2 = textBuff1->length();
			textBuff1->substring(textBuff2, iVar3 + 1, (iVar2 - iVar3) + -1);
			textBuff1->setLength(0);
			textBuff3 = textBuff2;
			textBuff2 = textBuff1;
		}
		textBuff1 = textBuff3;
	}
	for (iVar1 = this->miniGameHelpScrollPosition + 0x10; iVar1 < this->helpTextNumberOfLines;
		iVar1 = iVar1 + 1) {
		iVar3 = textBuff1->findLastOf('\n');
		textBuff3 = textBuff1;
		if (iVar3 != -1) {
			textBuff1->substring(textBuff2, 0, iVar3);
			textBuff1->setLength(0);
			textBuff3 = textBuff2;
			textBuff2 = textBuff1;
		}
		textBuff1 = textBuff3;
	}
	textBuff2->dispose();

	graphics->drawString(textBuff1, this->screenRect[0] + 0x12, this->screenRect[1] + 0x2d, 0x14);
	iVar3 = this->helpTextNumberOfLines;
	iVar1 = this->miniGameHelpScrollPosition + 0x10;
	if (iVar3 < iVar1) {
		iVar1 = iVar3;
	}
	this->drawScrollBar(graphics, this->screenRect[2] + -0x12, this->screenRect[1] + 0x2d, 0x100,
		this->miniGameHelpScrollPosition, iVar1, iVar3, 0x10);
	textBuff1->setLength(0);
	app->localization->composeText(0, 0x60, textBuff1);
	textBuff1->dehyphenate();
	graphics->drawString(textBuff1, x, this->screenRect[3] + -0x14, 3);
	textBuff1->dispose();
}

void Canvas::handleMiniGameHelpScreenScroll(int i) {
	int iVar1;
	int iVar2;

	if (i < 0) {
		iVar1 = i + this->miniGameHelpScrollPosition;
		if (iVar1 < 0) {
			iVar1 = 0;
		}
		this->miniGameHelpScrollPosition = iVar1;
		return;
	}
	if (0 < i) {
		iVar1 = this->helpTextNumberOfLines + -0x10;
		if (iVar1 < 0) {
			iVar1 = 0;
		}
		iVar2 = i + this->miniGameHelpScrollPosition;
		if (iVar1 < iVar2) {
			this->miniGameHelpScrollPosition = iVar1;
		}
		else {
			this->miniGameHelpScrollPosition = iVar2;
		}
		return;
	}
	return;
}

void Canvas::disposeCharacterSelection() {
	this->imgCharacter_select_stat_bar->~Image();
	this->imgCharacter_select_stat_bar = nullptr;
	this->imgCharacter_select_stat_header->~Image();
	this->imgCharacter_select_stat_header = nullptr;
	this->imgTopBarFill->~Image();
	this->imgTopBarFill = nullptr;
	this->imgCharacter_upperbar->~Image();
	this->imgCharacter_upperbar = nullptr;
	this->imgCharacterSelectionAssets->~Image();
	this->imgCharacterSelectionAssets = nullptr;
	this->imgCharSelectionBG->~Image();
	this->imgCharSelectionBG = nullptr;
	this->imgMajorMugs->~Image();
	this->imgMajorMugs = nullptr;
	this->imgSargeMugs->~Image();
	this->imgSargeMugs = nullptr;
	this->imgScientistMugs->~Image();
	this->imgScientistMugs = nullptr;
	this->imgMajor_legs->~Image();
	this->imgMajor_legs = nullptr;
	this->imgMajor_torso->~Image();
	this->imgMajor_torso = nullptr;
	this->imgRiley_legs->~Image();
	this->imgRiley_legs = nullptr;
	this->imgRiley_torso->~Image();
	this->imgRiley_torso = nullptr;
	this->imgSarge_legs->~Image();
	this->imgSarge_legs = nullptr;
	this->imgSarge_torso->~Image();
	this->imgSarge_torso = nullptr;
}

bool Canvas::pitchIsControlled(int n, int n2, int n3) {

	Applet* app = CAppContainer::getInstance()->app;

	bool b = false;
	for (int i = 0; i < Render::MAX_KEEP_PITCH_LEVEL_TILES; ++i) {
		if (app->render->mapKeepPitchLevelTiles[i] != -1) {
			short n4 = app->render->mapKeepPitchLevelTiles[i];
			if ((n2 << 5) + n == (n4 & 0x3FF) && n3 == (n4 & 0xFFFFFC00) >> 10) {
				b = true;
				break;
			}
		}
	}
	return b;
}

int Canvas::touchToKey_Play(int pressX, int pressY) {
	Applet* app = CAppContainer::getInstance()->app;
	int result;

	this->m_controlButton = 0;
	if (this->m_sniperScopeDialScrollButton->field_0x14_)
	{
		this->m_sniperScopeDialScrollButton->field_0x14_ = 0;
		return -1;
	}
	this->m_swipeArea[this->m_controlMode]->touched = false;
	if (((pressY > 256)) || (app->hud->isInWeaponSelect))
	{
		app->hud->handleUserTouch(pressX, pressY, false);
		return -1;
	}
	if (this->touched) {
		return -1;
	}
	if (this->isZoomedIn) {
		return this->m_sniperScopeButtons->GetTouchedButtonID(pressX, pressY);
	}
	if (this->m_controlButtonIsTouched) {
		return -1;
	}

	result = this->m_controlButtons[this->m_controlMode + 0]->GetTouchedButtonID(pressX, pressY);
	if (result == -1)
	{
		result = this->m_controlButtons[this->m_controlMode + 2]->GetTouchedButtonID(pressX, pressY);
		if (result == -1) {
			result = this->m_controlButtons[this->m_controlMode + 4]->GetTouchedButtonID(pressX, pressY);
		}
	}

	if (result != 6) {
		return -1;
	}

	return result;
}

bool Canvas::startArmorRepair(ScriptThread* armorRepairThread) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->player->showHelp((short)15, false)) {
		return false;
	}
	if (app->player->inventory[12] >= 5) {
		this->armorRepairThread = armorRepairThread;
		this->repairingArmor = true;
		Text* smallBuffer = app->localization->getSmallBuffer();
		app->localization->composeText((short)0, (short)211, smallBuffer);
		this->startDialog(nullptr, smallBuffer, 13, 1, false);
		smallBuffer->dispose();
		return true;
	}

	if (app->player->characterChoice == 1) {
		app->sound->playSound(1073, 0, 3, 0);
	}
	else if (app->player->characterChoice >= 1 && app->player->characterChoice <= 3){
		app->sound->playSound(1072, 0, 3, 0);
	}

	app->hud->addMessage((short)0, (short)210, 3);
	return false;
}

void Canvas::endArmorRepair() {
	if (this->armorRepairThread != nullptr) {
		this->setState(Canvas::ST_PLAYING);
		this->armorRepairThread->run();
		this->armorRepairThread = nullptr;
		this->repairingArmor = false;
	}
}

void Canvas::drawTouchSoftkeyBar(Graphics* graphics, bool highlighted_Left, bool highlighted_Right) {
	Applet* app = CAppContainer::getInstance()->app;
	Text* SmallBuffer = app->localization->getSmallBuffer();

	graphics->drawImage(app->menuSystem->imgGameMenuPanelbottom,
		0, 320 - app->menuSystem->imgGameMenuPanelbottom->height, 0, 0, 0);
	if (app->player->isFamiliar) {
		graphics->drawImage(app->menuSystem->imgGameMenuPanelBottomSentrybot,
			0, 320 - app->menuSystem->imgGameMenuPanelBottomSentrybot->height, 0, 0, 0);
	}

	if (!highlighted_Left || (highlighted_Left && this->softKeyLeftID == -1)) {
		graphics->drawImage(app->hud->imgSwitchLeftNormal, 9, 268, 0, 0, 0);
	}
	else {
		graphics->drawImage(app->hud->imgSwitchLeftActive, 9, 268, 0, 0, 0);
	}

	if (this->softKeyLeftID != -1) {
		SmallBuffer->setLength(0);
		app->localization->composeText(this->softKeyLeftID, SmallBuffer);
		SmallBuffer->dehyphenate();
		graphics->drawString(SmallBuffer, 2, 320, 36);
	}

	if (!highlighted_Right || (highlighted_Right && this->softKeyRightID == -1)) {
		graphics->drawImage(app->hud->imgSwitchRightNormal, 438, 268, 0, 0, 0);
	}
	else {
		graphics->drawImage(app->hud->imgSwitchRightActive, 438, 268, 0, 0, 0);
	}

	if (this->softKeyRightID != -1) {
		SmallBuffer->setLength(0);
		app->localization->composeText(this->softKeyRightID, SmallBuffer);
		SmallBuffer->dehyphenate();
		graphics->drawString(SmallBuffer, 478, 320, 40);
	}
	SmallBuffer->dispose();
}

void Canvas::touchSwipe(int swDir) {
	Applet* app = CAppContainer::getInstance()->app;
	int iVar1;
	int iVar2;

	if (this->state == Canvas::ST_PLAYING || this->state == Canvas::ST_COMBAT) {
#if 0 // IOS
		switch (swDir) {
			case fmSwipeArea::SwipeDir::Left:
				iVar1 = 2;
				break;
			case fmSwipeArea::SwipeDir::Right:
				iVar1 = 4;
				break;
			case fmSwipeArea::SwipeDir::Down:
				iVar1 = AVK_RIGHT;
				break;
			case fmSwipeArea::SwipeDir::Up:
				iVar1 = AVK_LEFT;
				break;
			default:
				return;
		}

		if (this->isZoomedIn != false) {
			bool bVar4 = iVar1 == 2;
			if (bVar4) {
				iVar1 = 5;
			}
			if ((!bVar4) && (iVar1 == 4)) {
				iVar1 = 7;
			}
		}
#else
		switch (swDir) {
			case fmSwipeArea::SwipeDir::Left:
				iVar1 = AVK_MOVELEFT;
				break;
			case fmSwipeArea::SwipeDir::Right:
				iVar1 = AVK_MOVERIGHT;
				break;
			case fmSwipeArea::SwipeDir::Down:
				iVar1 = AVK_DOWN;
				break;
			case fmSwipeArea::SwipeDir::Up:
				iVar1 = AVK_UP;
				break;
			default:
				return;
		}
#endif

		if (this->numEvents != 4) {
			this->events[this->numEvents++] = iVar1;
			this->keyPressedTime = app->upTimeMs;
		}
	}
}

void Canvas::turnEntityIntoWaterSpout(Entity* entity) {
	Applet* app = CAppContainer::getInstance()->app;
	int sprite = entity->getSprite();
	entity->def = app->entityDefManager->lookup(Enums::TILENUM_WATER_SPOUT);
	entity->name = (short)(entity->def->name | 0x400);
	app->render->mapSpriteInfo[sprite] = ((app->render->mapSpriteInfo[sprite] & 0xFFFFFF00) | Enums::TILENUM_WATER_SPOUT);
	entity->info |= 0x400000;
}


void Canvas::flipControls() {
	int v1; // r10
	fmButtonContainer** v2; // r6
	int x; // r8
	fmButton* Button; // r5
	fmButton* v5; // r0
	bool v6; // zf
	fmButton* v7; // r4
	fmButton* v8; // r4
	fmButton* v9; // r0
	bool v10; // zf
	fmButton* v11; // r4
	fmButton* v12; // r0
	bool v13; // zf
	fmButton* v14; // r5
	int v15; // r8

	v1 = 0;
	v2 = &this->m_controlButtons[4];
	this->isFlipControls ^= 1u;
	do
	{
		v2[-4]->FlipButtons();
		v2[-2]->FlipButtons();
		v2[0]->FlipButtons();
		while (1)
		{
			Button = v2[-2]->GetButton(5);
			v5 = v2[-2]->GetButton(7);
			v6 = Button == 0;
			if (Button)
				v6 = v5 == 0;
			v7 = v5;
			if (v6)
				break;
			x = Button->touchArea.x;
			Button->SetTouchArea(v5->touchArea.x, Button->touchArea.y, Button->touchArea.w, Button->touchArea.h);
			v7->SetTouchArea(x, v7->touchArea.y, v7->touchArea.w, v7->touchArea.h);
			Button->buttonID = -2;
			v7->buttonID = -3;
		}
		while (1)
		{
			v8 = v2[-2]->GetButton(-2);
			v9 = v2[-2]->GetButton(-3);
			v10 = v8 == 0;
			if (v8)
				v10 = v9 == 0;
			if (v10)
				break;
			v8->buttonID = 5;
			v9->buttonID = 7;
		}
		v11 = v2[0]->GetButton(2);
		v12 = v2[0]->GetButton(4);
		v13 = v11 == 0;
		if (v11)
			v13 = v12 == 0;
		v14 = v12;
		if (!v13)
		{
			v15 = v11->touchArea.x;
			v11->SetTouchArea(v12->touchArea.x, v11->touchArea.y, v11->touchArea.w, v11->touchArea.h);
			v14->SetTouchArea(v15, v14->touchArea.y, v14->touchArea.w, v14->touchArea.h);
		}
		++v1;
		++v2;
	} while (v1 != 2);
}

void Canvas::setControlLayout() {
	if (this->m_controlLayout == 2) {
		this->m_controlGraphic = 0;
		this->m_controlMode = 1;
	}
	else {
		this->m_controlGraphic = this->m_controlLayout;
		this->m_controlMode = 0;
	}

	for (int i = 0; i < 2; i++) {
		this->m_controlButtons[i + 0]->SetGraphic(this->m_controlGraphic);
		this->m_controlButtons[i + 2]->SetGraphic(this->m_controlGraphic);
	}
}

void Canvas::evaluateMiniGameResults(int n) {
	Applet* app = CAppContainer::getInstance()->app;
	if (n == 1) {
		int modifyStat = app->player->modifyStat(7, 2);
		app->localization->resetTextArgs();
		if (modifyStat > 0) {
			app->localization->addTextArg(modifyStat);
			app->hud->addMessage((short)236, 3);
		}
		else {
			app->hud->addMessage((short)237, 3);
		}
	}
}
void Canvas::addEvents(int event) { // [GEC]
	if (this->numEvents < Canvas::MAX_EVENTS) {
		//printf("numEvents %d Code %d\n", this->numEvents, event);
		this->events[0] = event;
		this->numEvents = 1;
	}
}