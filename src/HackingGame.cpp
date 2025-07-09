#include <stdexcept>
#include <algorithm>

#include "CAppContainer.h"
#include "App.h"
#include "Text.h"
#include "Game.h"
#include "Button.h"
#include "Canvas.h"
#include "Player.h"
#include "HackingGame.h"
#include "CombatEntity.h"
#include "MenuStrings.h"
#include "MenuSystem.h"
#include "Sound.h"
#include "Enums.h"
#include "Utils.h"
#include "Menus.h"

int HackingGame::touchedColumn = -1;

HackingGame::HackingGame() {
	std::memset(this, 0, sizeof(HackingGame));
}

HackingGame::~HackingGame() {
}

void HackingGame::playFromMainMenu() {
	Applet* app = CAppContainer::getInstance()->app;

	int rnd = app->nextInt() % 3 + 3;
	this->initGame(nullptr, rnd, (app->nextInt() % ((rnd * 10) - 15)) + 5);
}

void HackingGame::setupGlobalData() {
	Applet* app = CAppContainer::getInstance()->app;
	Canvas* canvas = app->canvas;

	this->confirmationCursor = 0;
	this->stateVars = canvas->stateVars;
	this->CORE_WIDTH = 35;
	this->CORE_HEIGHT = 100;
	canvas->setState(Canvas::ST_MINI_GAME);
	canvas->stateVars[0] = 2;
	canvas->stateVars[1] = 0;
	canvas->stateVars[2] = 0;
	canvas->initMiniGameHelpScreen();
	canvas->clearSoftKeys();
	this->selected = false;
}

void HackingGame::initGame(ScriptThread* scriptThread, int i) {
	this->initGame(scriptThread, i, 30);
}

void HackingGame::initGame(ScriptThread* scriptThread, int i, int i2) {
	Applet* app = CAppContainer::getInstance()->app;

	this->columnCount = std::max(3, std::min(i, 6));
	this->callingThread = scriptThread;
	this->gamePlayedFromMainMenu = (this->callingThread == nullptr);

	int n2 = this->gamePlayedFromMainMenu ? 0 : app->player->ce->getIQPercent();
	this->fillGameBoardRandomly(this->gameBoard, 5, this->columnCount, 3, (10000 * i2 + i2 * (100 - n2) * (100 - n2)) / 20000);
	this->setupGlobalData();
	this->turnsLeft = (short)i2;
	this->selectedRow = -1;
	for (int j = 0; j < this->columnCount; j++) {
		for (int k = 0; k < 5; k++) {
			if (this->gameBoard[k][j] != 0) {
				this->selectedRow = (short)k;
				this->selectedColumn = (short)j;
				break;
			}
		}
		if (this->selectedRow != -1) {
			break;
		}
	}

	this->touched = false;
	this->currentPress_x = 0;
	this->currentPress_y = 0;
	this->oldPress_x = 0;
	this->oldPress_y = 0;

	if (this->m_hackingButtons) {
		delete this->m_hackingButtons;
	}
	this->m_hackingButtons = nullptr;

	this->m_hackingButtons = new fmButtonContainer();

	fmButton* button01 = new fmButton(0, 420, 280, 60, 40, 1027); // Old -> (0, 420, 260, 60, 60, 1027);
	this->m_hackingButtons->AddButton(button01);

	fmButton* button02 = new fmButton(1, 0, 280, 60, 40, 1027); // Old -> (1, 0, 260, 60, 60, 1027);
	this->m_hackingButtons->AddButton(button02);
}

void  HackingGame::fillGameBoardRandomly(short array[5][6], int n, int n2, int n3) {
	Applet* app = CAppContainer::getInstance()->app;
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 6; ++j) {
			array[i][j] = 0;
		}
	}
	for (int k = 0; k < n2; ++k) {
		for (int l = 0; l < n3; ++l) {
			int n4 = app->nextByte() / ((255 + n) / n);
			int n5 = app->nextByte() / ((255 + n2) / n2);
			if (array[n4][n5] == 0) {
				array[n4][n5] = (short)(k + 1);
			}
			else {
				--l;
			}
		}
	}
	for (int n6 = 0; n6 < n2; ++n6) {
		for (int n7 = 0; n7 < n - 1; ++n7) {
			if (array[n7][n6] != 0 && array[n7 + 1][n6] == 0) {
				for (int n8 = n7; n8 >= 0; --n8) {
					array[n8 + 1][n6] = array[n8][n6];
				}
				array[0][n6] = 0;
			}
		}
	}
}

void HackingGame::fillGameBoardRandomly(short array[5][6], int n, int n2, int n3, int n4) {
	Applet* app = CAppContainer::getInstance()->app;
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 6; ++j) {
			array[i][j] = 0;
		}
	}
	for (int k = 0; k < n2; ++k) {
		for (int l = 0; l < n3; ++l) {
			array[n - l - 1][k] = (short)(k + 1);
		}
	}
	uint8_t b = 0;
	for (uint8_t b2 = 0; b2 < this->columnCount; ++b2) {
		this->mostRecentSrc[b2] = b2;
		this->mostRecentDest[b2] = b2;
	}
	for (int n5 = 0; n5 < n4; ++n5) {
		int n6 = app->nextByte() / ((255 + n2) / n2);
		int n7 = app->nextByte() / ((255 + n2) / n2);
		if (n6 == n7) {
			--n5;
		}
		else if (n6 == b) {
			--n5;
		}
		else if (this->mostRecentDest[n6] == n7 && this->mostRecentSrc[n7] == n6) {
			--n5;
		}
		else if (array[n - 1][n6] == 0 || array[0][n7] != 0) {
			--n5;
		}
		else {
			int n8;
			for (n8 = n - 1; n8 > 0 && array[n8 - 1][n6] != 0; --n8) {}
			int n9;
			for (n9 = n - 1; array[n9][n7] != 0; --n9) {}
			array[n9][n7] = array[n8][n6];
			array[n8][n6] = 0;
			this->mostRecentSrc[n6] = (uint8_t)n7;
			this->mostRecentDest[n7] = (uint8_t)n6;
			b = (uint8_t)n7;
		}
	}
	if (this->gameIsSolved(array)) {
		this->fillGameBoardRandomly(array, n, n2, n3, n4);
	}
}

void HackingGame::handleInput(int action) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->stateVars[1] == 0) {
		//if (action == Enums::ACTION_AUTOMAP || action == Enums::ACTION_BACK) { // Old
		if (action == Enums::ACTION_MENU || action == Enums::ACTION_BACK) { // [GEC]
			switch (this->stateVars[2]) {
				case 0:
					this->endGame(2);
					break;
				case 1:
					this->endGame(1);
					break;
				case 2:
					this->endGame(0);
					break;
			}
		}
		else if (action == Enums::ACTION_FIRE) {
			app->canvas->stateVars[1] = 1;
			app->canvas->clearSoftKeys();
		}
		else if (action == Enums::ACTION_UP) {
			app->canvas->handleMiniGameHelpScreenScroll(-1);
		}
		else if (action == Enums::ACTION_DOWN) {
			app->canvas->handleMiniGameHelpScreenScroll(1);
		}
		else if (action == Enums::ACTION_RIGHT) {
			app->canvas->handleMiniGameHelpScreenScroll(16);
		}
		else if (action == Enums::ACTION_LEFT) {
			app->canvas->handleMiniGameHelpScreenScroll(-16);
		}
	}
	else if (this->stateVars[1] == 1) {
		if (this->stateVars[2] == 0) {
			switch (action) {
				case Enums::ACTION_LEFT:
					app->menuSystem->soundClick(); // [GEC]
					this->attemptToMove(this->selectedColumn - 1);
					break;
				case Enums::ACTION_RIGHT:
					app->menuSystem->soundClick(); // [GEC]
					this->attemptToMove(this->selectedColumn + 1);
					break;
				case Enums::ACTION_AUTOMAP: // Old Enums::ACTION_MENU
				case Enums::ACTION_BACK:
					this->stateVars[1] = 0;
					app->canvas->initMiniGameHelpScreen();
					app->canvas->clearSoftKeys();
					break;
				case Enums::ACTION_FIRE:
					if (this->selected) {
						this->selected = !this->selected;
						if (this->columnBlockCameFrom != (int)this->selectedColumn) {
							this->turnsLeft = this->turnsLeft + -1;
						}
						if (this->gameIsSolved(this->gameBoard)) {
							this->stateVars[2] = 1;
							app->canvas->clearSoftKeys();
						}
						else if (this->turnsLeft == 0) {
							this->stateVars[2] = 2;
							app->canvas->clearSoftKeys();
						}
					}
					else {
						if (this->gameBoard[this->selectedRow][this->selectedColumn] != 0) {
							this->selected = !this->selected;
							this->columnBlockCameFrom = (int)this->selectedColumn;
						}
					}
					break;
				case Enums::ACTION_MENU:  // Old Enums::ACTION_AUTOMAP
					this->stateVars[1] = 2;
					app->canvas->clearSoftKeys();
					break;
				default:
					break;
			}
		}
		else if (this->stateVars[2] == 1) {
			switch (action) {
			case Enums::ACTION_FIRE:
			case Enums::ACTION_AUTOMAP:
				this->endGame(1);
				break;
			default:
				break;
			}
		}
		else if (this->stateVars[2] == 2) {
			switch (action) {
			case Enums::ACTION_FIRE:
			case Enums::ACTION_AUTOMAP:
				this->endGame(0);
				break;
			default:
				break;
			}
		}
	}
	else if (this->stateVars[1] == 2) {
		if (action == Enums::ACTION_FIRE) {
            if (this->confirmationCursor == 1) {
				this->endGame(2);
            }
			else {
				this->stateVars[1] = 1;
				app->canvas->clearSoftKeys();
			}
        } else if (action == Enums::ACTION_LEFT || action == Enums::ACTION_RIGHT) {
			this->confirmationCursor = confirmationCursor == 0 ? 1 : 0;
        } else if (action == Enums::ACTION_BACK) {
			this->stateVars[1] = 1;
			app->canvas->clearSoftKeys();
        }
	}
}

void HackingGame::attemptToMove(short n) {
	short selectedColumn = (short)((n + this->columnCount) % this->columnCount);
	if (this->selectedColumn != selectedColumn) {
		int n2 = -1;
		for (int i = 4; i >= 0; --i) {
			if (this->gameBoard[i][selectedColumn] == 0) {
				n2 = (short)i;
				break;
			}
		}
		if (this->selected) {
			short n3;
			short n4;
			for (n3 = (short)(selectedColumn - this->selectedColumn), n4 = (short)((selectedColumn + n3 + this->columnCount) % this->columnCount); n4 != this->selectedColumn && n2 == -1; n4 = (short)((n4 + n3 + this->columnCount) % this->columnCount)) {
				for (int j = 4; j >= 0; --j) {
					if (this->gameBoard[j][n4] == 0) {
						n2 = (short)j;
						break;
					}
				}
			}
			short selectedColumn2 = (short)((n4 - n3 + this->columnCount) % this->columnCount);

			if (n2 != -1) {
				this->gameBoard[n2][selectedColumn2] = this->gameBoard[this->selectedRow][this->selectedColumn];
				this->gameBoard[this->selectedRow][this->selectedColumn] = 0;
				this->selectedRow = n2;
				this->selectedColumn = selectedColumn2;
			}
			else {
				this->selected = false;
			}
		}
		else {
			if (n2 != 4) {
				n2 = (short)(n2 + 1);
			}
			this->selectedRow = n2;
			this->selectedColumn = selectedColumn;
		}
	}
}

void HackingGame::updateGame(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	graphics->clipRect(0, 0, 480, 320);
	if (this->stateVars[1] == 0) {
		this->drawHelpScreen(graphics);
	}
	else {
		this->drawGameScreen(graphics);
	}
	app->canvas->staleView = true;
	graphics->resetScreenSpace();
}

void HackingGame::drawHelpScreen(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	Text* text;

	app->canvas->drawMiniGameHelpScreen(graphics, 164, 165, this->imgHelpScreenAssets);
	text = app->localization->getSmallBuffer();
	text->setLength(0);
	app->localization->composeText((short)0, (short)30, text); // Old -> text->append("Exit");
	if (!this->m_hackingButtons->GetButton(0)->highlighted) {
		app->setFontRenderMode(2);
	}
	graphics->drawString(text, 445, 302, 3); // Old -> 310, 3);
	app->setFontRenderMode(0);
	text->dispose();
}

void HackingGame::drawGameScreen(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	Text* smallBuffer;
	int color;

	graphics->drawImage(this->imgEnergyCore, 0, 0, 0, 0, 0);

	for (int i = 0; i < 6; ++i) {
		int xPos = 25 + i * (this->CORE_WIDTH + 44);
		if (i < this->columnCount) {
			if (this->touched && pointInRectangle(this->currentPress_x, this->currentPress_y, xPos, 90, this->CORE_WIDTH, this->CORE_HEIGHT)) {
				graphics->drawRect(xPos, 90, this->CORE_WIDTH, this->CORE_HEIGHT, -1);
				if (i != HackingGame::touchedColumn) {
					HackingGame::touchedColumn = i;
					app->menuSystem->soundClick();
				}
			}
			else if (pointInRectangle(this->oldPress_x, this->oldPress_y, xPos, 90, this->CORE_WIDTH, this->CORE_HEIGHT)) {
				HackingGame::touchedColumn = -1;
				this->attemptToMove(i);
				this->handleInput(Enums::ACTION_FIRE);
				this->oldPress_x = 0;
				this->oldPress_y = 0;
			}
		}
	}

	for (int j = 0; j < this->columnCount; ++j) {
		graphics->drawRegion(this->imgGameColors, j * 12, 18, 12, 12, 37 + j * (this->CORE_WIDTH + 44), 95 + this->CORE_HEIGHT, 0, 0, 0);
	}

	smallBuffer = app->localization->getSmallBuffer();
	smallBuffer->setLength(0);
	app->localization->composeText(0, 167, smallBuffer);
	smallBuffer->dehyphenate();
	smallBuffer->append(this->turnsLeft);
	graphics->drawString(smallBuffer, 148, 28, 1);

	this->drawGoalTextAndBars(graphics, smallBuffer);

	smallBuffer->setLength(0);
	if (this->stateVars[1] == 1) {
		switch (this->stateVars[2]) {
			case 1:
				app->localization->composeText(0, 169, smallBuffer);
				break;
			case 2:
				app->localization->composeText(0, 170, smallBuffer);
				break;
			case 0:
				app->localization->composeText(0, 166, smallBuffer);
				app->canvas->setLeftSoftKey(3, 80);
				break;
		}

		smallBuffer->wrapText(47, 0xAu);
		graphics->drawString(smallBuffer, 12, 215, 4);
		app->canvas->setRightSoftKey(0, 30);

		//----------------------------------------------------------------
		smallBuffer->setLength(0);
		app->localization->composeText(3, 80, smallBuffer); // Old -> SmallBuffer->append("Back");

		if (!this->m_hackingButtons->GetButton(1)->highlighted) {
			app->setFontRenderMode(2);
		}
		graphics->drawString(smallBuffer, 35, 302, 3); // Old -> 310, 3);
		app->setFontRenderMode(0);

		//----------------------------------------------------------------
		smallBuffer->setLength(0);
		app->localization->composeText(0, 30, smallBuffer); // Old -> SmallBuffer->append("Exit");

		if (!this->m_hackingButtons->GetButton(0)->highlighted) {
			app->setFontRenderMode(2);
		}
		graphics->drawString(smallBuffer, 445, 302, 3); // Old -> 310, 3);
		app->setFontRenderMode(0);
	}
	else {
		smallBuffer->setLength(0);
		app->localization->composeText(0, 189, smallBuffer);
		smallBuffer->wrapText(47, 0xAu);
		graphics->drawString(smallBuffer, 75, 215, 4);

		int rectX = 142;
		int rectY = 240;
		int rectW = 45;
		int rectH = 25;

		color = 0x808080;
		if (this->touched && pointInRectangle(this->currentPress_x, this->currentPress_y, rectX, rectY, rectW, rectH)) {
			color = 0x7878FF;
		}
		else if(pointInRectangle(this->currentPress_x, this->currentPress_y, rectX, rectY, rectW, rectH)) {
			this->confirmationCursor = 1;
			this->handleInput(Enums::ACTION_FIRE);
			this->currentPress_x = 0;
			this->currentPress_y = 0;
		}

		graphics->fillRect(rectX, rectY, rectW, rectH, color);
		graphics->drawRect(rectX, rectY, rectW, rectH, 0xFFFFFFFF);
		smallBuffer->setLength(0);
		app->localization->composeText(3, 197, smallBuffer);
		graphics->drawString(smallBuffer, 150, 245, 4);

		rectX = 258;
		rectY = 240;
		rectW = 45;
		rectH = 25;

		color = 0x808080;
		if (this->touched && pointInRectangle(this->currentPress_x, this->currentPress_y, rectX, rectY, rectW, rectH)) {
			color = 0x7878FF;
		}
		else if (pointInRectangle(this->currentPress_x, this->currentPress_y, rectX, rectY, rectW, rectH)) {
			this->confirmationCursor = 0;
			this->handleInput(Enums::ACTION_FIRE);
			this->currentPress_x = 0;
			this->currentPress_y = 0;
		}

		graphics->fillRect(rectX, rectY, rectW, rectH, color);
		graphics->drawRect(rectX, rectY, rectW, rectH, 0xFFFFFFFF);
		smallBuffer->setLength(0);
		app->localization->composeText(3, 198, smallBuffer);
		graphics->drawString(smallBuffer, 270, 245, 4);

		int n11 = ((this->confirmationCursor == 1) ? 142-6 : 258-6) + app->canvas->OSC_CYCLE[app->time / 100 % 4];
		graphics->drawCursor(n11, 245, 8);
	}

	smallBuffer->dispose();
	this->drawGamePieces(graphics, 34, 94);
}

void HackingGame::drawGoalTextAndBars(Graphics* graphics, Text* text) {
	Applet* app = CAppContainer::getInstance()->app;

	int n = 335 - (5 + 9 * (this->columnCount - 1)) / 2;
	text->setLength(0);
	app->localization->composeText(0, 168, text);
	text->dehyphenate();
	graphics->drawString(text, 335, 38, 1);

	for (int i = 0; i < this->columnCount; ++i) {
		int color = 0xFFA9E94E;
		switch (i) {
			case 0: {
				color = 0xFFA9E94E;
				break;
			}
			case 1: {
				color = 0xFFE77313;
				break;
			}
			case 2: {
				color = 0xFFFD1100;
				break;
			}
			case 3: {
				color = 0xFFC450FF;
				break;
			}
			case 4: {
				color = 0xFFFFF395;
				break;
			}
			case 5: {
				color = 0xFF101ADE;
				break;
			}
		}
		graphics->fillRect(n + i * 9, 22, 5, 15, color);
	}
}

void HackingGame::drawGamePieces(Graphics* graphics, int x, int y) {
	Applet* app = CAppContainer::getInstance()->app;

	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < this->columnCount; ++j) {
			short n = this->gameBoard[i][j];
			if (n != 0) {
				this->drawPiece(n, x + (j * (this->CORE_WIDTH + 44)), y + (i * 19), graphics);
			}
		}
	}
	int posX = x + this->selectedColumn * (this->CORE_WIDTH + 44);
	int posY = y + this->selectedRow * 19;
	int time = app->time & 0x3FF;
	int i;
	if (time < 511) {
		i = (95 * time >> 9) + 160;
	}
	else {
		i = (-95 * (time - 511) >> 9) + 255;
	}
	int color = 0xFF000000 | i << 16 | i << 8 | i;
	if (this->selected) {
		graphics->drawRect(posX, posY, 18, 18, color);
		graphics->drawRect(posX + 1, posY + 1, 16, 16, color);
	}
	else { // [GEC] Restore
		graphics->drawRect(posX, posY, 18, 18, color); // J2ME/BREW
	}
}

bool HackingGame::gameIsSolved(short array[5][6]) {
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 6; ++j) {
			if (array[i][j] != 0 && array[i][j] != j + 1) {
				return false;
			}
		}
	}
	return true;
}

void HackingGame::drawPiece(int i, int x, int y, Graphics* graphics) {
	graphics->drawRegion(this->imgGameColors, (i * 18) - 18, 0, 18, 18, x, y, 0, 0, 0);
}

void HackingGame::forceWin() {
	Applet* app = CAppContainer::getInstance()->app;

	for (int i = 0; i < this->columnCount; ++i) {
		for (int j = 0; j < 5; ++j) {
			if (j < 3) {
				this->gameBoard[4 - j][i] = (short)(i + 1);
			}
			else {
				this->gameBoard[4 - j][i] = 0;
			}
		}
	}
	this->selectedRow = 2;
	this->selectedColumn = 0;
	this->stateVars[1] = 1;
	this->stateVars[2] = 1;
	app->canvas->clearSoftKeys();
}

void HackingGame::endGame(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	app->sound->playSound((n == 1) ? 1043 : 1040, '\0', 3, false);
	if (!this->gamePlayedFromMainMenu) {
		app->canvas->evaluateMiniGameResults(n);
		app->game->scriptStateVars[7] = (short)n;
	}
	if (!this->gamePlayedFromMainMenu) {
		app->canvas->setState(Canvas::ST_PLAYING);
		this->callingThread->run();
		this->callingThread = nullptr;
	}
	else {
		app->sound->playSound(1071, '\x01', 3, false);
		app->menuSystem->setMenu(Menus::MENU_MAIN_MINIGAME);
	}
}

void HackingGame::touchStart(int x, int y) {
	this->m_hackingButtons->HighlightButton(x, y, true);
	this->touched = true;
	this->currentPress_x = x;
	this->currentPress_y = y;
	this->oldPress_x = x;
	this->oldPress_y = y;
}

void HackingGame::touchMove(int x, int y) {
	int oldPress;

	this->m_hackingButtons->HighlightButton(x, y, true);
	oldPress = this->currentPress_x;
	this->currentPress_x = x;
	this->oldPress_x = oldPress;

	oldPress = this->currentPress_y;
	this->currentPress_y = y;
	this->oldPress_y = oldPress;
}

void HackingGame::touchEnd(int x, int y) {
	this->m_hackingButtons->HighlightButton(0, 0, false);
	this->touched = false;
	this->currentPress_x = x;
	this->currentPress_y = y;
	if (this->stateVars[1] == 0) {
		if (this->m_hackingButtons->GetTouchedButtonID(x, y) == 0) { // exit
			this->handleInput(Enums::ACTION_MENU);  // Old Enums::ACTION_AUTOMAP
		}
		else {
			this->currentPress_x = 0;
			this->currentPress_y = 0;
			this->oldPress_x = 0;
			this->oldPress_y = 0;
			this->handleInput(Enums::ACTION_FIRE);
		}
	}
	else if (this->stateVars[1] == 1) {
		if ((this->stateVars[2] == 1) || (this->stateVars[2] == 2)) {
			this->handleInput(Enums::ACTION_FIRE);
		}
		else if (this->m_hackingButtons->GetTouchedButtonID(x, y) == 0) { // exit
			this->handleInput(Enums::ACTION_MENU); // Old Enums::ACTION_AUTOMAP
		}
		else if (this->m_hackingButtons->GetTouchedButtonID(x, y) == 1) { // back
			this->handleInput(Enums::ACTION_BACK);
		}
	}
}
