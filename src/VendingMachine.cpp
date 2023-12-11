#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Text.h"
#include "Game.h"
#include "Button.h"
#include "Canvas.h"
#include "Player.h"
#include "VendingMachine.h"
#include "MenuStrings.h"
#include "MenuSystem.h"
#include "Image.h"
#include "Hud.h"
#include "Sound.h"
#include "Enums.h"
#include "Menus.h"

VendingMachine::VendingMachine() {
	std::memset(this, 0, sizeof(VendingMachine));
}

VendingMachine::~VendingMachine() {
}

bool VendingMachine::startup() {
	printf("VendingMachine::startup\n");

	return false;
}

void VendingMachine::playFromMainMenu() {
    this->initGame(nullptr, 1, 1);
    this->stateVars[1] = 0;
    this->stateVars[2] = 0;
}

void VendingMachine::initGame(ScriptThread* callingThread, int a, int a2) {
    Applet* app = CAppContainer::getInstance()->app;
    this->stateVars = app->canvas->stateVars;
    this->callingThread = callingThread;
    this->gamePlayedFromMainMenu = (this->callingThread == nullptr);
    app->canvas->setState(Canvas::ST_MINI_GAME);
    this->stateVars[0] = 4;
    this->stateVars[1] = 2;
    this->stateVars[2] = 0;
    this->stateVars[4] = 0;
    this->stateVars[3] = app->time;
    app->canvas->clearSoftKeys();
    app->canvas->initMiniGameHelpScreen();
    this->machineJustHacked = false;
    this->currentMapNumber = std::max(std::min(a, 9), 1);
    this->currentMachineNumber = std::max(std::min(a2, 18), 1);
    this->triesLeft = (this->gamePlayedFromMainMenu ? 4 : app->player->getVendingMachineTriesLeft(this->currentMachineNumber));
    this->machineHasBeenHacked = (!this->gamePlayedFromMainMenu && app->player->vendingMachineIsHacked(this->currentMachineNumber));
    this->correctSum = 0;
    if (this->machineCanBeHacked()) {
        this->randomizeGame();
    }
    this->mainTerminalCursor = 0;
    this->gameCursor = 0;
    this->iqBonusAwarded = 0;
    this->gameCursor2 = 0; // [GEC]

    int w = app->canvas->screenRect[2] - app->canvas->screenRect[0];
    if (w - 97 >= 0) {
        w -= 97;
    }
    else {
        w -= 94;
    }
    this->widthOfContainingBox = (w / 4) + 1;

    if (this->m_vendingButtons) {
        delete this->m_vendingButtons;
    }
    this->m_vendingButtons = nullptr;
    this->m_vendingButtons = new fmButtonContainer();

    fmButton* btnSubmit = new fmButton(0, 400, 160 - (this->imgSubmitButton->height / 2), this->imgSubmitButton->width, this->imgSubmitButton->height, 1027);
    btnSubmit->SetImage(this->imgSubmitButton, true);
    btnSubmit->SetHighlightImage(this->imgVending_submit_pressed, true);
    btnSubmit->normalRenderMode = 12;
    btnSubmit->highlightRenderMode = 0;
    this->m_vendingButtons->AddButton(btnSubmit);

    Image* imgVending_arrow;
    Image* imgVending_arrow_pressed;
    for (int i = 1; i < 9; i++) {
        if (i % 2 == 1) {
            imgVending_arrow = this->imgVending_arrow_up;
            imgVending_arrow_pressed = this->imgVending_arrow_up_pressed;
        }
        else {
            imgVending_arrow = this->imgVending_arrow_down;
            imgVending_arrow_pressed = this->imgVending_arrow_down_pressed;
        }
        fmButton* btnArrow = new fmButton(i, 0, 0, 0, 0, 1027);
        btnArrow->SetImage(imgVending_arrow, false);
        btnArrow->SetHighlightImage(imgVending_arrow_pressed, false);
        btnArrow->normalRenderMode = 12;
        btnArrow->highlightRenderMode = 0;
        this->m_vendingButtons->AddButton(btnArrow);
    }

    fmButton* btnBack = new fmButton(9, 0, 290, 54, 30, 1027); // Old -> 50, 30, 1027
    this->m_vendingButtons->AddButton(btnBack);

    fmButton* btnExit = new fmButton(10, 428, 290, 54, 30, 1027); // Old -> 430, 290, 50, 30, 1027
    this->m_vendingButtons->AddButton(btnExit);

    this->touched = false;
    this->currentPress_x = 0;
    this->currentPress_y = 0;
}

void VendingMachine::returnFromBuying() {
    Applet* app = CAppContainer::getInstance()->app;
    app->canvas->setState(Canvas::ST_MINI_GAME);
    this->stateVars[0] = 4;
    this->stateVars[1] = 2;
    this->stateVars[2] = 0;
    this->stateVars[4] = 0;
    this->machineJustHacked = false;
    app->canvas->clearSoftKeys();
    if (!this->machineCanBeHacked()) {
        this->endGame(2);
    }
}

bool VendingMachine::machineCanBeHacked() {
    return !this->machineHasBeenHacked && this->triesLeft > 0;
}

void VendingMachine::randomizeGame() {
    Applet* app = CAppContainer::getInstance()->app;
    this->correctSum = 0;
    for (int i = 0; i < 4; ++i) {
        this->solution[i] = app->nextInt() % 9 + 0;
        this->sliderPositions[i] = 5;
        this->correctSum += this->solution[i];
    }
    for (int j = 0; j < 4; ++j) {
        this->playersGuess[j] = this->sliderPositions[j];
        this->minimums[j] = 0;
        this->maximums[j] = 9;
        this->chevronAnimationOffset[j] = 0;
    }
    int n = this->gamePlayedFromMainMenu ? 0 : app->player->ce->getIQPercent();
    int k;
    if (n >= 80) {
        k = 3;
    }
    else if (n >= 50) {
        k = 2;
    }
    else if (n >= 20) {
        k = 1;
    }
    else {
        k = 0;
    }
    while (k > this->numbersCorrect()) {
        int n2 = app->nextInt() % 4;
        this->correctSum -= this->solution[n2] - 5;
        this->solution[n2] = 5;
    }
    this->machineHasBeenHacked = false;
}

void VendingMachine::handleInput(int action) {
    switch (this->stateVars[1]) {
        case 1: {
            this->handleInputForGame(action);
            break;
        }
        case 2: {
            this->handleInputForBasicVendingMachine(action);
            break;
        }
        case 0: {
            this->handleInputForHelpScreen(action);
            break;
        }
    }
}

void VendingMachine::handleInputForBasicVendingMachine(int action) {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->machineCanBeHacked()) {
        if (action == Enums::ACTION_MENU /*Enums::ACTION_AUTOMAP*/ || action == Enums::ACTION_BACK) {
            this->endGame(2);
        }
        else if (action == Enums::ACTION_LEFT || action == Enums::ACTION_RIGHT) {
            ++this->mainTerminalCursor;
            this->mainTerminalCursor %= 2;
        }
        else if (action == Enums::ACTION_FIRE) {
            if (this->mainTerminalCursor == 0) {
                app->menuSystem->setMenu(Menus::MENU_VENDING_MACHINE);
            }
            else if (this->mainTerminalCursor == 1) {
                app->canvas->stateVars[1] = 0;
                app->canvas->initMiniGameHelpScreen();
                app->canvas->clearSoftKeys();
            }
        }
    }
    else if (action == Enums::ACTION_MENU /*Enums::ACTION_AUTOMAP*/ || action == Enums::ACTION_BACK) {
        this->endGame(2);
    }
    else if (action == Enums::ACTION_LEFT || action == Enums::ACTION_RIGHT) {
        ++this->mainTerminalCursor;
        this->mainTerminalCursor %= 2;
    }
    else if (action == Enums::ACTION_FIRE) {
        if (this->mainTerminalCursor == 0) {
            app->menuSystem->setMenu(Menus::MENU_VENDING_MACHINE);
        }
        else if (this->mainTerminalCursor == 1) {
            this->endGame(2);
        }
    }
}

void VendingMachine::handleInputForHelpScreen(int action) {
    Applet* app = CAppContainer::getInstance()->app;
    if (action == Enums::ACTION_AUTOMAP) {
        this->endGame(2);
    }
    else if (action == Enums::ACTION_MENU || action == Enums::ACTION_BACK) {
        bool gamePlayedFromMainMenu = this->gamePlayedFromMainMenu;
        if (!this->gamePlayedFromMainMenu)
        {
            this->stateVars[1] = 2;
            this->stateVars[4] = gamePlayedFromMainMenu;
            app->canvas->clearSoftKeys();
        }
        else {
            this->endGame(2);
        }
    }
    else if (action == Enums::ACTION_FIRE) {
        this->stateVars[1] = 1;
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

void VendingMachine::handleInputForGame(int action) {
    Applet* app = CAppContainer::getInstance()->app;
    if (this->stateVars[2] == 2) {
        if (action == Enums::ACTION_FIRE) {
            if (this->triesLeft == 0) {
                if (this->gamePlayedFromMainMenu) {
                    this->endGame(this->stateVars[2]);
                }
                else {
                    this->stateVars[1] = 2;
                    app->canvas->clearSoftKeys();
                }
            }
            else {
                this->gameCursor = 0;
                for (int n3 = 0; n3 < 4; n3++) {
                    if (this->playersGuess[n3] != this->solution[n3]) {
                        this->playersGuess[n3] = -1;
                    }
                }
                while (this->playersGuess[this->gameCursor] != -1) {
                    this->gameCursor++;
                }
                this->stateVars[2] = 0;
            }
        }
        else if (action == Enums::ACTION_MENU) { // Old Enums::ACTION_AUTOMAP
            this->endGame(this->stateVars[2]);
        }
        else if ((action == Enums::ACTION_AUTOMAP/*Enums::ACTION_MENU*/ || action == Enums::ACTION_BACK)) {
            if (this->triesLeft != 0) {
                this->stateVars[1] = 0;
                app->canvas->initMiniGameHelpScreen();
            }
            app->canvas->clearSoftKeys();
        }
    }
    else if (this->stateVars[2] == 1) {
        if (action == Enums::ACTION_FIRE) {
            if (this->gamePlayedFromMainMenu) {
                this->endGame(this->stateVars[2]);
            }
            else {
                this->stateVars[1] = 2;
                app->canvas->clearSoftKeys();
                this->machineJustHacked = true;
                this->machineHasBeenHacked = true;
                app->player->setVendingMachineHack(this->currentMachineNumber);
            }
        }
        else if (action == Enums::ACTION_MENU) { // Old Enums::ACTION_AUTOMAP
            this->endGame(this->stateVars[2]);
        }
    }
    else if (action == Enums::ACTION_MENU) { // Old Enums::ACTION_AUTOMAP
        this->endGame(2);
    }
    else if (action == Enums::ACTION_AUTOMAP/*Enums::ACTION_MENU*/ || action == Enums::ACTION_BACK) {
        this->stateVars[1] = 0;
        app->canvas->initMiniGameHelpScreen();
        app->canvas->clearSoftKeys();
    }
    else if (action == Enums::ACTION_FIRE) {
        this->playersGuess[this->gameCursor] = this->sliderPositions[this->gameCursor];
        ++this->gameCursor;
        while (this->gameCursor < 4 && this->playersGuess[this->gameCursor] != -1) {
            ++this->gameCursor;
        }
        if (this->gameCursor == 4) {
            app->canvas->clearSoftKeys();
            this->updateHighLowState();
            --this->triesLeft;
            if (this->playerHasWon()) {
                app->sound->playSound(1043, 0, 3, 0);
                this->stateVars[2] = 1;
                this->machineHasBeenHacked = true;
                if (!this->gamePlayedFromMainMenu) {
                    this->iqBonusAwarded = app->player->modifyStat(7, 2);
                }
            }
            else {
                app->sound->playSound(1040, 0, 3, 0);
                if (!this->gamePlayedFromMainMenu) {
                    app->player->removeOneVendingMachineTry(this->currentMachineNumber);
                }
                this->stateVars[2] = 2;
            }
        }
    }
    else if (action == Enums::ACTION_UP) {
        int gameCursor = this->gameCursor2; // Old this->gameCursor
        app->menuSystem->soundClick(); // [GEC]
        if (this->sliderPositions[gameCursor] < 9) {
            ++this->sliderPositions[gameCursor];
            this->playersGuess[gameCursor] = this->sliderPositions[gameCursor];
        }
    }
    else if (action == Enums::ACTION_DOWN) {
        int gameCursor = this->gameCursor2; // Old this->gameCursor
        app->menuSystem->soundClick(); // [GEC]
        if (this->sliderPositions[gameCursor] > 0) {
            --this->sliderPositions[gameCursor];
            this->playersGuess[gameCursor] = this->sliderPositions[gameCursor];
        }
    }
    else if (action == Enums::ACTION_LEFT) { // [GEC]
        app->menuSystem->soundClick(); // [GEC]
        this->gameCursor2 = (this->gameCursor2 + 3) % 4;
    }
    else if (action == Enums::ACTION_RIGHT) { // [GEC]
        app->menuSystem->soundClick(); // [GEC]
        this->gameCursor2 = (this->gameCursor2 + 1) % 4;
    }
}

void VendingMachine::updateHighLowState() {
    for (int i = 0; i < 4; ++i) {
        int n = this->sliderPositions[i] - this->solution[i];
        if (n > 0) {
            this->maximums[i] = std::min(this->sliderPositions[i] - 1, this->maximums[i]);
        }
        else if (n < 0) {
            this->minimums[i] = std::max(this->sliderPositions[i] + 1, this->minimums[i]);
        }
        else {
            this->maximums[i] = this->sliderPositions[i];
            this->minimums[i] = this->sliderPositions[i];
        }
    }
}

void VendingMachine::updateGame(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;
    graphics->clipRect(0, 0, app->canvas->screenRect[2], app->canvas->screenRect[3]);
    if (app->time - this->stateVars[3] > 100) {
        this->stateVars[3] = app->time;
        for (int i = 0; i < 4; ++i) {
            ++this->chevronAnimationOffset[i];
            this->chevronAnimationOffset[i] %= 11;
        }
    }
    switch (this->stateVars[1]) {
        case 1: {
            this->drawGameScreen(graphics);
            break;
        }
        case 2: {
            this->drawMainScreen(graphics);
            break;
        }
        case 0: {
            this->drawHelpScreen(graphics);
            break;
        }
    }
    app->canvas->staleView = true;
    graphics->resetScreenSpace();
}

void VendingMachine::drawGameResults(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;
    Text* smallBuffer = app->localization->getSmallBuffer();
    smallBuffer->setLength(0);
    if (this->machineHasBeenHacked) {
        if (this->gamePlayedFromMainMenu) {
            app->localization->composeText((short)0, (short)239, smallBuffer);
        }
        else {
            app->localization->resetTextArgs();
            if (this->iqBonusAwarded > 0) {
                app->localization->addTextArg(this->iqBonusAwarded);
                app->localization->composeText((short)0, (short)236, smallBuffer);
            }
            else {
                app->localization->composeText((short)0, (short)237, smallBuffer);
            }
        }
    }
    else if (this->triesLeft > 0) {
        app->localization->composeText((short)0, (short)209, smallBuffer);
        smallBuffer->append(this->triesLeft);
    }
    else if (this->gamePlayedFromMainMenu) {
        app->localization->composeText((short)0, (short)240, smallBuffer);
    }
    else {
        app->localization->composeText((short)0, (short)201, smallBuffer);
    }
#if 0 //J2ME
    Text* smallBuffer2 = app->localization->getSmallBuffer();
    smallBuffer2->setLength(0);
    app->localization->composeText((short)0, (short)96, smallBuffer2);
    smallBuffer->append('|');
    smallBuffer->append('|');
    smallBuffer->append(smallBuffer2);
    smallBuffer2->dispose();
#endif

    smallBuffer->wrapText(20, '\n');
    int stringWidth = smallBuffer->getStringWidth();
    int n = smallBuffer->getNumLines() * 16;
    graphics->fillRect(app->canvas->SCR_CX - (stringWidth / 2) - 6, app->canvas->SCR_CY - (n / 2) - 4, stringWidth + 12, n + 8, 0xFF000000);
    graphics->drawRect(app->canvas->SCR_CX - (stringWidth / 2) - 6, app->canvas->SCR_CY - (n / 2) - 4, stringWidth + 12, n + 8, 0xFFFFFFFF);
    graphics->drawString(smallBuffer, app->canvas->SCR_CX, app->canvas->SCR_CY, 3);
    smallBuffer->dispose();
}

void VendingMachine::drawMainScreen(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;

    this->drawVendingMachineBackground(graphics, true);
    int n = 14;
    int n2 = 1;
    Text* smallBuffer = app->localization->getSmallBuffer();
    smallBuffer->setLength(0);
    app->localization->composeText(Strings::FILE_MENUSTRINGS, MenuStrings::VENDING_MACHINE_TITLE, smallBuffer);
    smallBuffer->dehyphenate();
    app->menuSystem->buildDivider(smallBuffer, smallBuffer->length() + 10);
    graphics->drawString(smallBuffer, app->canvas->SCR_CX + n2, n, 1);
    n += 16; // Old 13
    app->localization->resetTextArgs();
    smallBuffer->setLength(0);
    smallBuffer->append(app->player->inventory[24]);
    app->localization->addTextArg(smallBuffer);
    smallBuffer->setLength(0);
    app->localization->composeText(Strings::FILE_CODESTRINGS, (short)197, smallBuffer);
    smallBuffer->dehyphenate();
    graphics->drawString(smallBuffer, app->canvas->SCR_CX + n2, n, 17);
    int n3 = 60;
    smallBuffer->setLength(0);
    if (this->machineCanBeHacked()) {
        app->localization->composeText((short)0, (short)198, smallBuffer);
    }
    else if (this->machineJustHacked) {
        app->localization->composeText((short)0, (short)200, smallBuffer);
        app->localization->composeText((short)0, (short)199, smallBuffer);
    }
    else {
        app->localization->composeText((short)0, (short)199, smallBuffer);
    }
    smallBuffer->wrapText(26, '\n');
    graphics->drawString(smallBuffer, 240, n3, 17);

    short n4;
    short n5;
    if (this->machineCanBeHacked()) {
        n4 = 202;
        n5 = 203;
    }
    else {
        n4 = 141;
        n5 = 140;
    }

    int oldMainTerminalCursor = this->mainTerminalCursor;
    //this->mainTerminalCursor = -1; //[GEC] Block this line code

    int posY = (16 * smallBuffer->getNumLines()) + 95;
    int posX = 112;
    

    int width = this->imgVending_button_small->width;
    int height = this->imgVending_button_small->height / 2;

    int renderMode = 0;
    if (this->touched && 
        ((this->currentPress_x > posX) && (this->currentPress_x < posX + width)) &&
        ((this->currentPress_y > posY - height) && (this->currentPress_y < posY + height))) {
        this->mainTerminalCursor = 0;
    }
    else {
        renderMode = 1;
    }
    // Left Button
    graphics->drawImage(this->imgVending_button_small, posX, posY - height, 0, 0, renderMode);

    smallBuffer->setLength(0);
    app->localization->composeText((short)0, n4, smallBuffer);
    smallBuffer->dehyphenate();
    graphics->drawString(smallBuffer, posX + (width / 2), posY, 3);
    
    int cursorX = cursorX = ((posX + (width / 2)) - (smallBuffer->getStringWidth() / 2)) - 5;
    int cursorY = posY;

    posX = width + 135;
    renderMode = 0;
    if (this->touched &&
        ((this->currentPress_x > posX) && (this->currentPress_x < posX + width)) &&
        ((this->currentPress_y > posY - height) && (this->currentPress_y < posY + height))) {
        this->mainTerminalCursor = 1;
    }
    else {
        renderMode = 1;
    }

    // Right Button
    graphics->drawImage(this->imgVending_button_small, posX, posY - height, 0, 0, renderMode);

    smallBuffer->setLength(0);
    app->localization->composeText((short)0, n5, smallBuffer);
    smallBuffer->dehyphenate();
    graphics->drawString(smallBuffer, posX + (width / 2), posY, 3);

    if (this->mainTerminalCursor == 1) {
        cursorX = ((posX + (width / 2)) - (smallBuffer->getStringWidth() / 2)) - 5;
    }

    graphics->drawCursor(cursorX + app->canvas->OSC_CYCLE[(app->time / 100) % 4], cursorY - 8, 24); // [GEC] Restored from J2ME/BREW

    Image* imgVendingSoftkey = app->hud->imgVendingSoftkeyNormal;
    width = imgVendingSoftkey->width;
    height = imgVendingSoftkey->height / 2;
    posX = 362;
    posY = 297;
    if (this->touched &&
        ((this->currentPress_x > posX) && (this->currentPress_x < posX + width)) &&
        ((this->currentPress_y > (posY - height)) && (this->currentPress_y < posY + height))) {
        imgVendingSoftkey = app->hud->imgVendingSoftkeyPressed;
        this->mainTerminalCursor = 2;
    }
    else {
        if (((this->currentPress_x > posX) && (this->currentPress_x < posX + width)) &&
            ((this->currentPress_y > (posY - height)) && (this->currentPress_y < posY + height))) {
            this->handleInput(Enums::ACTION_BACK);
        }
    }

    posY -= height;
    graphics->drawImage(imgVendingSoftkey, posX, posY, 0, 0, 0);
    smallBuffer->setLength(0);
    app->localization->composeText(0, 30, smallBuffer); // Old -> smallBuffer->append("Exit");
    graphics->drawString(smallBuffer, posX + (width / 2), posY + height, 3);

    smallBuffer->dispose();

    if (oldMainTerminalCursor != this->mainTerminalCursor && this->mainTerminalCursor != -1) {
        app->menuSystem->soundClick();
    }
}

void VendingMachine::drawVendingMachineBackground(Graphics* graphics, bool b) {
    Applet* app = CAppContainer::getInstance()->app;
    int x, y;
    graphics->drawImage(this->imgVendingBG, 0, 0, 0, 0, 0);
    for (x = app->canvas->screenRect[0] + 200; x < app->canvas->screenRect[2]; x = x + 10) {
    }
    if (b != false) {
        x = 2;
        do {
            y = 3;
            do {
                y--;
            } while (y != 0);
            x--;
        } while (x != 0);
    }
}

void VendingMachine::drawHelpScreen(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;
    app->canvas->drawMiniGameHelpScreen(graphics, 195, 196, this->imgHelpScreenAssets);
    Text* textBuff = app->localization->getSmallBuffer();
    textBuff->setLength(0);
    app->localization->composeText(0, 30, textBuff); // Old -> text->append("Exit");
    if (this->m_vendingButtons->GetButton(10)->highlighted ==  false) {
        app->setFontRenderMode(2);
    }
    graphics->drawString(textBuff, 445, 302, 3); // Old -> 310, 3);
    app->setFontRenderMode( 0);
    textBuff->dispose();
}

void VendingMachine::drawGameScreen(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;

    for (int i = 0; i < 4; i++) {
        this->playersGuess[i] = this->sliderPositions[i];
    }

    graphics->drawImage(this->imgVending_BG, 0, 0, 0, 0, 0);
    Text* textBuff = app->localization->getSmallBuffer();
    this->drawGameTopBar(graphics, textBuff);
    this->drawGameMiddleBar(graphics, textBuff);
    textBuff->setLength(0);

    if (this->stateVars[2] != 0) {
        this->drawGameResults(graphics);
        app->localization->composeText((short)0, (short)208, textBuff);
        if (this->triesLeft > 0) {
            app->canvas->setLeftSoftKey((short)3, (short)80);
        }
    }
    else {
        app->localization->composeText((short)0, (short)207, textBuff);
        app->canvas->setLeftSoftKey((short)3, (short)80);
    }
    app->canvas->setRightSoftKey((short)0, (short)30);

    textBuff->dehyphenate();
    graphics->drawString(textBuff, (app->canvas->screenRect[2] - app->canvas->screenRect[0]) / 2, app->canvas->screenRect[3] - 12, 3);

    //----------------------------------------------------------------
    textBuff->setLength(0);
    app->localization->composeText(0, 30, textBuff); // Old -> textBuff->append("Exit");
    if (!this->m_vendingButtons->GetButton(10)->highlighted) {
        app->setFontRenderMode(2);
    }
    graphics->drawString(textBuff, 445, 302, 3); // Old -> 310, 3);
    app->setFontRenderMode(0);

    //----------------------------------------------------------------
    textBuff->setLength(0);
    app->localization->composeText(3, 80, textBuff); // Old -> textBuff->append("Back");
    if (!this->m_vendingButtons->GetButton(9)->highlighted) {
        app->setFontRenderMode(2);
    }
    graphics->drawString(textBuff, 35, 302, 3); // Old -> 310, 3);
    textBuff->dispose();
    app->setFontRenderMode(0);

    this->m_vendingButtons->Render(graphics);
    textBuff->setLength(0);
    textBuff->append("OK");
    graphics->drawString(textBuff, this->imgSubmitButton->width / 2 + 398, 160, 3);
}

void VendingMachine::drawGameTopBar(Graphics* graphics, Text* text) {
    Applet* app = CAppContainer::getInstance()->app;

    int y = app->canvas->screenRect[1];
    int x = app->canvas->screenRect[0] + 32;

    text->setLength(0);
    text->append("Current Sum");
    graphics->drawString(text, 240, 9, 3);

    int n4 = this->widthOfContainingBox - 33;
    for (int i = 0; i < 5; ++i) {
        graphics->drawRegion(this->imgVendingGame, 38, 37, 33, 29, x, y + 17,0, 0, 0);
        text->setLength(0);
        if (i < 4) {
            if (this->playersGuess[i] != -1) {
                text->append(this->playersGuess[i]);
            }
        }
        else {
            text->append(this->correctSum);
        }
        graphics->drawString(text, x + 14, y + 31, 3);
        if (i > 0) {
            text->setLength(0);
            if (i == 4) {
                text->append("=");
            }
            else {
                text->append("+");
            }
            graphics->drawString(text, x - n4 / 2 - 1, y + 31, 3);
        }
        x += 33 + n4;
    }

    text->setLength(0);
    text->append(this->correctSum);
    graphics->drawString(text, 300, 280, 3);

    text->setLength(0);
    text->append("Hack Goal Sum =");
    graphics->drawString(text, 200, 280, 3);
}

void VendingMachine::drawGameMiddleBar(Graphics* graphics, Text* text) {
    Applet* app = CAppContainer::getInstance()->app;

    int y = app->canvas->screenRect[1];
    int x = app->canvas->screenRect[0] + 32;

    int intensity = 187 + ((std::abs(1000 - (app->time % 2000)) * 68) / 1000); // [GEC] restored from J2ME/BREW
    int color = 0xFF000000 + (intensity << 16) + (intensity << 8); // [GEC] restored from J2ME/BREW

    int buttonId = 1;
    for (int i = 0; i < 4; i++) {
        this->m_vendingButtons->GetButton(buttonId)->SetTouchArea(
            x + 16 - this->imgSubmitButton->width / 2 - 2,
            y + 104 - this->imgSubmitButton->height + 5,
            this->imgSubmitButton->width, this->imgSubmitButton->height);

        this->m_vendingButtons->GetButton(buttonId + 1)->SetTouchArea(
            x + 16 - this->imgSubmitButton->width / 2 - 2,
            y + 213,
            this->imgSubmitButton->width, this->imgSubmitButton->height);

        if (this->touched) {
            if ((this->currentPress_x > x + 5) && (this->currentPress_x < x + 27)) {
                if ((this->currentPress_y > y + 104) && (this->currentPress_y < y + 214)) {
                    graphics->drawRect(x + 5, y + 104, 22, 110, 0xFFFFFF00);
                    this->gameCursor = i;

                    this->sliderPositions[i] = (y + 214 - this->currentPress_y) / 11;
                    if (this->sliderPositions[i] > 9) {
                        this->sliderPositions[i] = 9;
                    }
                    if (this->sliderPositions[i] < 0) {
                        this->sliderPositions[i] = 0;
                    }
                }
            }
        }

        int posX = x + 5;
        int posY = y + 203;

        for (int j = 0; j < this->minimums[i]; ++j) {
            graphics->drawRegion(this->imgVendingGame, 22, 97 - this->chevronAnimationOffset[i], 22, 11,
                posX, posY, 20, 6, 0); // Old 96 - this->chevronAnimationOffset[i]
            posY -= 11;
        }

        posY = y + 104;
        for (int k = 9; k > this->maximums[i]; --k) {
            graphics->drawRegion(this->imgVendingGame, 22, 97 - this->chevronAnimationOffset[i], 22, 11,
                posX, posY, 20, 0, 0); // Old 96 - this->chevronAnimationOffset[i]
            posY += 11;
        }

        posY = y + 109 + (9 - this->sliderPositions[i]) * 99 / 9;

        if (this->gameCursor2 == i) { // [GEC]
            graphics->drawRect(posX, (posY - 5), 21, 10, color);
            graphics->drawRect(posX + 1, (posY - 5) + 1, 21 - 2, 10 - 2, color);
        }
        else {
            graphics->drawRect(posX, posY - 5, 21, 10, -1);
        }
        text->setLength(0);
        text->append(this->sliderPositions[i]);
        graphics->drawString(text, x + 15, posY + 1, 3);

        x += this->widthOfContainingBox;
        buttonId += 2;
    }
}

bool VendingMachine::drinkInThisVendingMachine(int n) {
    return (this->energyDrinkData[(n - 0) * 3 + 2] & 1 << this->currentMapNumber) == 1 << this->currentMapNumber;
}

short VendingMachine::getDrinkPrice(int n) {
    return this->energyDrinkData[(n - 0) * 3 + (this->machineHasBeenHacked ? 1 : 0)];
}

bool VendingMachine::buyDrink(int n) {
    Applet* app = CAppContainer::getInstance()->app;
    if (app->player->inventory[24] < this->currentItemPrice * this->currentItemQuantity) {
        return false;
    }
    for (int i = 0; i < this->currentItemQuantity; ++i) {
        app->player->inventory[24] -= (short)this->currentItemPrice;
        app->player->give(0, n, 1, false);
    }
    return true;
}

int VendingMachine::getSnackPrice() {
    return this->machineHasBeenHacked ? 8 : 10;
}

int VendingMachine::numbersCorrect() {
    int n = 0;
    for (int i = 0; i < 4; ++i) {
        if (this->sliderPositions[i] == this->solution[i]) {
            ++n;
        }
    }
    return n;
}

bool VendingMachine::playerHasWon() {
    return this->numbersCorrect() == 4;
}

void VendingMachine::forceWin() {
    for (int i = 0; i < 4; ++i) {
        this->sliderPositions[i] = this->solution[i];
    }
    this->updateHighLowState();
}

void VendingMachine::endGame(int n) {
    Applet* app = CAppContainer::getInstance()->app;
    if (!this->gamePlayedFromMainMenu) {
        app->game->scriptStateVars[7] = (short)n;
    }
    if (!this->gamePlayedFromMainMenu) {
        app->canvas->setState(Canvas::ST_PLAYING);
        this->callingThread->run();
        this->callingThread = nullptr;
    }
    else {
        app->sound->playSound(1071, 1, 3, false);
        app->menuSystem->setMenu(Menus::MENU_MAIN_MINIGAME);
    }
}

void VendingMachine::touchStart(int pressX, int pressY) {
    this->m_vendingButtons->HighlightButton(pressX, pressY, true);
    this->touched = true;
    this->currentPress_x = pressX;
    this->currentPress_y = pressY;
}

void VendingMachine::touchMove(int pressX, int pressY) {
    this->m_vendingButtons->HighlightButton(pressX, pressY, true);
    this->currentPress_x = pressX;
    this->currentPress_y = pressY;
}

void VendingMachine::touchEnd(int pressX, int pressY) {
    this->m_vendingButtons->HighlightButton(0, 0, false);
    this->touched = false;
    this->currentPress_x = pressX;
    this->currentPress_y = pressY;
    switch (this->stateVars[1]) {
        case 1:
            this->handleTouchForGame(pressX, pressY);
            break;
        case 2:
            this->handleTouchForBasicVendingMachine(pressX, pressY);
            break;
        case 0:
            this->handleTouchForHelpScreen(pressX, pressY);
            break;
    }
}

void VendingMachine::handleTouchForHelpScreen(int pressX, int pressY) {
    if (this->m_vendingButtons->GetTouchedButtonID(pressX, pressY) == 10) {
        this->handleInput(Enums::ACTION_BACK);
    }
    else {
        this->handleInput(Enums::ACTION_FIRE);
    }
}

void VendingMachine::handleTouchForGame(int pressX, int pressY) {

    if ((this->stateVars[2] >= 1 && this->stateVars[2] <= 2)) {
        this->handleInput(Enums::ACTION_FIRE);
        return;
    }

    int buttonID = this->m_vendingButtons->GetTouchedButtonID(pressX, pressY);
    switch (buttonID)
    {
        case 0:
            this->handleInput(Enums::ACTION_FIRE);
            return;
        case 9:
            this->handleInput(Enums::ACTION_BACK);
            return;
        case 10:
            this->handleInput(Enums::ACTION_MENU); // Old Enums::ACTION_AUTOMAP
            return;
    }

    switch (buttonID)
    {
        case 1:
            this->gameCursor2 = 0; // [GEC]
            if (++this->sliderPositions[0] >= 9) {
                this->sliderPositions[0] = 9;
            }
            break;
        case 2:
            this->gameCursor2 = 0; // [GEC]
            if (--this->sliderPositions[0] < 0) {
                this->sliderPositions[0] = 0;
            }
            break;
        case 3:
            this->gameCursor2 = 1; // [GEC]
            if (++this->sliderPositions[1] >= 9){
                this->sliderPositions[1] = 9;
            }
            break;
        case 4:
            this->gameCursor2 = 1; // [GEC]
            if (--this->sliderPositions[1] < 0) {
                this->sliderPositions[1] = 0;
            }
            break;
        case 5:
            this->gameCursor2 = 2; // [GEC]
            if (++this->sliderPositions[2] >= 9) {
                this->sliderPositions[2] = 9;
            }
            break;
        case 6:
            this->gameCursor2 = 2; // [GEC]
            if (--this->sliderPositions[2] < 0) {
                this->sliderPositions[2] = 0;
            }
            break;
        case 7:
            this->gameCursor2 = 3; // [GEC]
            if (++this->sliderPositions[3] >= 9) {
                this->sliderPositions[3] = 9;
            }
            break;
        case 8:
            this->gameCursor2 = 3; // [GEC]
            if (--this->sliderPositions[3] < 0) {
                this->sliderPositions[3] = 0;
            }
            break;
    }
}

void VendingMachine::handleTouchForBasicVendingMachine(int pressX, int pressY) {
    int buttonID = this->m_vendingButtons->GetTouchedButtonID(pressX, pressY);

    if (buttonID == 9) {
        this->handleInput(Enums::ACTION_BACK);
    }
    else if (buttonID == 10) {
        this->handleInput(Enums::ACTION_MENU); // Old Enums::ACTION_AUTOMAP
    }
    else {
        this->handleInput(Enums::ACTION_FIRE);
    }
}
