#include <stdexcept>
#include <algorithm>

#include "CAppContainer.h"
#include "App.h"
#include "Hud.h"
#include "Text.h"
#include "Canvas.h"
#include "Combat.h"
#include "Player.h"
#include "Render.h"
#include "Button.h"
#include "TinyGL.h"
#include "Image.h"
#include "Graphics.h"
#include "Button.h"
#include "Entity.h"
#include "EntityDef.h"
#include "EntityMonster.h"
#include "CombatEntity.h"
#include "MenuSystem.h"
#include "Enums.h"

Hud::Hud() {
	std::memset(this, 0, sizeof(Hud));
}

Hud::~Hud() {
}

bool Hud::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	printf("Hud::startup\n");

	for (int i = 0; i < Hud::MAX_MESSAGES; i++) {
		this->messages[i] = new Text(Hud::MS_PER_CHAR);
	}

	app->beginImageLoading();
	this->imgAmmoIcons = app->loadImage("hud_ammo_icons.bmp", true);
	this->imgAttArrow = app->loadImage("Hud_Attack_Arrows.bmp", true);
	this->imgPortraitsSM = app->loadImage("Hud_Portrait_Small.bmp", true);
	this->imgPanelTop = app->loadImage("HUD_Panel_top.bmp", true);
	this->imgPanelTopSentrybot = app->loadImage("HUD_Panel_top_sentrybot.bmp", true);
	this->imgShieldNormal = app->loadImage("HUD_Shield_Normal.bmp", true);
	this->imgShieldButtonActive = app->loadImage("Hud_Shield_Button_Active.bmp", true);
	this->imgKeyNormal = app->loadImage("Hud_Key_Normal.bmp", true);
	this->imgKeyActive = app->loadImage("Hud_Key_Active.bmp", true);
	this->imgHealthNormal = app->loadImage("HUD_Health_Normal.bmp", true);
	this->imgHealthButtonActive = app->loadImage("Hud_Health_Button_Active.bmp", true);
	this->imgSwitchRightNormal = app->loadImage("Switch_Right_Normal.bmp", true);
	this->imgSwitchRightActive = app->loadImage("Switch_Right_Active.bmp", true);
	this->imgSwitchLeftNormal = app->loadImage("Switch_Left_Normal.bmp", true);
	this->imgSwitchLeftActive = app->loadImage("Switch_Left_Active.bmp", true);
	this->imgVendingSoftkeyPressed = app->loadImage("vending_softkey_pressed.bmp", true);
	this->imgVendingSoftkeyNormal = app->loadImage("vending_softkey_normal.bmp", true);
	this->imgInGameMenuSoftkey = app->loadImage("inGame_menu_softkey.bmp", true);
	this->imgNumbers = app->loadImage("Hud_Numbers.bmp", true);
	this->imgWeaponNormal = app->loadImage("Hud_Weapon_Normal.bmp", true);
	this->imgWeaponActive = app->loadImage("HUD_Weapon_Active.bmp", true);
	this->imgPlayerFaces = app->loadImage("Hud_Player.bmp", true);
	this->imgPlayerActive = app->loadImage("HUD_Player_Active.bmp", true);
	app->endImageLoading();

	this->m_hudButtons = new fmButtonContainer;

	fmButton *btnSwitchRight = new fmButton(0, 0, 256, 52, 64, 1121);
	this->m_hudButtons->AddButton(btnSwitchRight);

	fmButton* btnSwitchLeft = new fmButton(1, 428, 256, 52, 64, 1121);
	this->m_hudButtons->AddButton(btnSwitchLeft);

	fmButton* btnWeapon = new fmButton(2, 268, 258, this->imgWeaponNormal->width, 44, 1121);
	this->m_hudButtons->AddButton(btnWeapon);

	fmButton* btnPlayer = new fmButton(3, 219, 264, this->imgPlayerFaces->width + 10, 36, 1121);
	this->m_hudButtons->AddButton(btnPlayer);

	fmButton* btnShield = new fmButton(4, 49, 258, this->imgShieldNormal->width, this->imgShieldNormal->height, 1121);
	this->m_hudButtons->AddButton(btnShield);

	fmButton* btnHealth = new fmButton(5, 133, 258, this->imgHealthNormal->width, this->imgHealthNormal->height, 1121);
	this->m_hudButtons->AddButton(btnHealth);

	fmButton* btnKey = new fmButton(6, 375, 258, this->imgKeyNormal->width, 44, 1121);
	this->m_hudButtons->AddButton(btnKey);
	
	this->m_weaponsButtons = new fmButtonContainer;
	for (int i = 0; i < Hud::MAX_WEAPON_BUTTONS; i++) {
		fmButton* btnWpn = new fmButton(i, 480, 320, this->imgWeaponNormal->width, 44, 1027);
		this->m_weaponsButtons->AddButton(btnWpn);
	}

	this->msgCount = 0;
	this->weaponPressTime = 0;
	this->isInWeaponSelect = false;

	return true;
}

void Hud::shiftMsgs() {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->messageFlags[0] & 0x2) {
		app->canvas->invalidateRect();
	}

	for (int i = 0; i < this->msgCount - 1; ++i) {
		this->messages[i]->setLength(0);
		this->messages[i]->append(this->messages[i + 1]);
		this->messageFlags[i] = this->messageFlags[i + 1];
	}

	this->msgCount--;
	this->messages[this->msgCount]->setLength(0);
	this->messageFlags[this->msgCount] = 0;
	if (this->msgCount > 0) {
		this->calcMsgTime();
	}
}

void Hud::calcMsgTime() {
	Applet* app = CAppContainer::getInstance()->app;

	this->msgTime = app->time;
	int length = this->messages[0]->length();
	if (length <= app->canvas->menuHelpMaxChars) {
		this->msgDuration = 700;
	}
	else {
		this->msgDuration = length * 50;
		if ((this->messageFlags[0] & 0x2) != 0x0 && this->msgDuration > 1500) {
			this->msgDuration = 1500;
		}
	}
}

void Hud::addMessage(short i) {
	this->addMessage(0, i, 0);
}

void Hud::addMessage(short i, short i2) {
	this->addMessage((short)i, i2, 0);
}

void Hud::addMessage(short i, int i2) {
	this->addMessage((short)0, i, i2);
}

void Hud::addMessage(short i, short i2, int i3) {
	Applet* app = CAppContainer::getInstance()->app;

	Text* text = app->localization->getSmallBuffer();
	app->localization->composeText(i, i2, text);
	this->addMessage(text, i3);
	text->dispose();
}

void Hud::addMessage(Text* text) {
	this->addMessage(text, 0);
}

void Hud::addMessage(Text* text, int flags) {
	Applet* app = CAppContainer::getInstance()->app;

	if (text == nullptr) {
		return;
	}

	if (flags & 0x1) {
		this->msgCount = 0;
	}

	if (this->msgCount > 0 && text->compareTo(this->messages[this->msgCount - 1])) {
		return;
	}

	if (this->msgCount == 5) {
		shiftMsgs();
	}

	this->messages[this->msgCount]->setLength(0);
	this->messages[this->msgCount]->append(text);

	if (flags & 0x2) {
		this->messages[this->msgCount]->wrapText((app->canvas->viewRect[2] - 9) / 9);
	}
	else {
		this->messages[this->msgCount]->dehyphenate();
	}

	this->messageFlags[this->msgCount] = flags;
	this->msgCount++;

	if (this->msgCount == 1) {
		this->calcMsgTime();
		if (flags & 0x1) {
			this->msgDuration *= 2;
		}
	}
}

Text* Hud::getMessageBuffer() {
	return this->getMessageBuffer(0);
}

Text* Hud::getMessageBuffer(int flags) {
	if ((flags & 0x1) != 0x0) {
		this->msgCount = 0;
	}
	if (this->msgCount == 5) {
		this->shiftMsgs();
	}
	this->messages[this->msgCount]->setLength(0);
	this->messageFlags[this->msgCount] = flags;
	return this->messages[this->msgCount];
}

void Hud::finishMessageBuffer() {
	this->messages[this->msgCount]->dehyphenate();
	this->msgCount++;
	if (this->msgCount == 1) {
		this->calcMsgTime();
	}
}

bool Hud::isShiftingCenterMsg() {
	Applet* app = CAppContainer::getInstance()->app;

	return ((this->msgCount > 0) && ((app->time - this->msgTime) > this->msgDuration) && (this->messageFlags[0] & 0x2)) ? true : false;
}

void Hud::drawTopBar(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	Text* smallBuffer = app->localization->getSmallBuffer();
	int n = 2;
	Entity* facingEntity = app->player->facingEntity;
	graphics->drawImage(this->imgPanelTop, 240, 0, 0x11, 0, 0);
	if (app->player->isFamiliar) {
		graphics->drawImage(this->imgPanelTopSentrybot, 240, 0, 0x11, 0, 0);
	}
	else {
		graphics->drawBevel(0xFF9A9987, 0xFF1D1E0C, -app->canvas->screenRect[0], 0, app->canvas->hudRect[2] + 1, 20);
	}
	if (app->canvas->state != Canvas::ST_DYING) {
		this->drawMonsterHealth(graphics);
	}
	if (this->msgCount > 0 && app->time - this->msgTime > this->msgDuration + 100) {
		this->shiftMsgs();
	}
	int n2 = -1;
	bool b = false;
	bool b2 = false;
	int n3 = 0;
	if (this->msgCount > 0) {
		if ((this->messageFlags[0] & 0x4) != 0x0) {
			this->drawImportantMessage(graphics, this->messages[0], 0xFF7F0000);
		}
		else {
			smallBuffer->setLength(0);
			smallBuffer->append(this->messages[0]);
			b2 = ((this->messageFlags[0] & 0x2) != 0x0);
			if (app->time - this->msgTime > this->msgDuration) {
				b = true;
			}
			else {
				int n4 = smallBuffer->length() - app->canvas->menuHelpMaxChars;
				if (n4 > 0) {
					int n5 = app->time - this->msgTime;
					if (n5 > 750) {
						n3 = (n5 - 750) / 50;
						if (n3 > n4) {
							n3 = n4;
						}
					}
				}
			}
		}
	}
	else if (app->player->inTargetPractice) {
		b = true;
	}
	else if (app->canvas->state != Canvas::ST_MENU && facingEntity != nullptr && facingEntity->def->eType != 0 && (facingEntity->def->eType != 12 || (facingEntity->info & 0x40000) != 0x0)) {
		smallBuffer->setLength(0);
		int n6 = (facingEntity->info & 0xFFFF) - 1;
		int n7 = app->render->mapSpriteInfo[n6] & 0xFF;
		uint8_t eType = facingEntity->def->eType;
		if ((facingEntity->name & 0x3FF) != facingEntity->def->name || facingEntity->def->longName == 159) {
			app->localization->composeTextField(facingEntity->name, smallBuffer);
		}
		else {
			app->localization->composeTextField(0x400 | facingEntity->def->longName, smallBuffer);
		}
		smallBuffer->dehyphenate();
		if (eType == 3) {
			n2 = 2;
		}
		else if (eType == 2) {
			smallBuffer->setLength(0);
			if ((facingEntity->name & 0x3FF) != facingEntity->def->name) {
				app->localization->composeTextField(facingEntity->name, smallBuffer);
			}
			else {
				app->localization->composeTextField(facingEntity->def->longName | 0x400, smallBuffer);
			}
			n2 = 1;
			smallBuffer->dehyphenate();
		}
		else if (eType == 6) {
			n2 = 0;
		}
		else if (eType == 5) {
			if ((facingEntity->name & 0x3FF) == facingEntity->def->name || !app->localization->isEmptyString(facingEntity->name)) {
				n2 = 3;
			}
			if (eType == 5 && (facingEntity->def->parm & 0x2) != 0x0) {
				b2 = true;
			}
		}
		else if (eType == 12) {
			b2 = true;
		}
		else if (eType == 7 && facingEntity->def->eSubType == 1) {
			b2 = true;
		}
		else if (eType == 7) {
			if (facingEntity->def->eSubType == 3) {
				n2 = 1;
			}
			else if (facingEntity->def->parm == 1) {
				n2 = 3;
			}
			else if (facingEntity->def->parm == 2 && (app->render->mapSpriteInfo[n6] & 0xFF00) >> 8 == 0) {
				n2 = 3;
			}
		}
		else if (eType == 14) {
			if (n7 == 142) {
				b2 = true;
			}
			else if (n7 == 143) {
				n2 = 3;
			}
			else if (n7 == 141) {
				n2 = 1;
			}
			else if (n7 == 134) {
				n2 = 0;
			}
			else if (facingEntity->def->eSubType == 8) {
				n2 = 3;
			}
		}
		else if (eType == 10) {
			if (facingEntity->def->eSubType == 3 || facingEntity->def->eSubType == 2) {
				n2 = 0;
			}
			else {
				n2 = 1;
			}
		}
	}
	else if (facingEntity != nullptr && facingEntity->def->eType == 12) {
		int n8 = (facingEntity->info & 0xFFFF) - 1;
		if ((app->render->mapSpriteInfo[n8] & 0xFF) == 0x99 && (app->render->mapSpriteInfo[n8] & 0xFF00) >> 8 == 0) {
			n2 = 3;
		}
		else {
			b = true;
		}
	}
	else {
		b = true;
	}

	if (!b) {
		if (b2 && smallBuffer->length() > 0) {
			drawCenterMessage(graphics, smallBuffer, -16777216);
		}
		else {
			if (n2 != -1) {
				graphics->drawRegion(this->imgActions, 0, n2 * 18, 18, 18, n - app->canvas->screenRect[0], app->canvas->screenRect[1] + 1, 20, 0, 0);
				n += 20;
			}
			int length = smallBuffer->length();
			if (n + 12 * (length + 1) > app->canvas->hudRect[2]) {
				length = (app->canvas->hudRect[2] - n) / 9 - 1;
			}
			graphics->drawString(smallBuffer, n - app->canvas->screenRect[0], 3, 0, n3, length);
		}
	}
	smallBuffer->dispose();
}

void Hud::drawImportantMessage(Graphics* graphics, Text* text, int color) {
	Canvas* canvas = CAppContainer::getInstance()->app->canvas;

	canvas->dialogRect[0] = canvas->viewRect[0];
	canvas->dialogRect[1] = canvas->viewRect[1];
	canvas->dialogRect[2] = (canvas->viewRect[2] - canvas->viewRect[0]) - 1;
	canvas->dialogRect[3] = 18;

	graphics->setColor(color);
	graphics->fillRect(canvas->dialogRect[0], canvas->dialogRect[1], canvas->dialogRect[2], canvas->dialogRect[3]);
	graphics->setColor(0xffffffff);
	graphics->drawRect(canvas->dialogRect[0], canvas->dialogRect[1], canvas->dialogRect[2], canvas->dialogRect[3]);
	text->dehyphenate();
	graphics->drawString(text, canvas->dialogRect[0] + 2, canvas->dialogRect[1] + 2, 4);
}

void Hud::drawCenterMessage(Graphics* graphics, Text* text, int color) {
	Applet* app = CAppContainer::getInstance()->app;

	int w = text->getStringWidth() + 8;
	if (w > app->canvas->hudRect[2]) {
		w = app->canvas->hudRect[2];
	}
	int y = -app->canvas->screenRect[1] + 40;
	int x = -app->canvas->screenRect[0] + (app->canvas->hudRect[2] / 2);

	int numLines = text->getNumLines();
	graphics->setColor(color);
	graphics->fillRect(x - (w / 2), y, w - 1, (16 * numLines) + 3);
	graphics->setColor(0xFFAAAAAA);
	graphics->drawRect(x - (w / 2), y, w - 1, (16 * numLines) + 3);

	y += 3;
	int i = 0;
	Text* textBuff = app->localization->getSmallBuffer();
	int first;
	while ((first = text->findFirstOf('|', i)) >= 0) {
		textBuff->setLength(0);
		text->substring(textBuff, i, first);
		graphics->drawString(textBuff, x, y, 17, 0, first - i);
		y += 16;
		i = first + 1;
	}

	textBuff->setLength(0);
	text->substring(textBuff, i);
	graphics->drawString(textBuff, x, y, 17, 0, text->length() - i);
	textBuff->dispose();
}


void Hud::drawCinematicText(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	Canvas* canvas = CAppContainer::getInstance()->app->canvas;

	int scr_CX = canvas->SCR_CX;
	int flags = 1;
	int width = this->imgPlayerFaces->width;

	graphics->eraseRgn(0, 0, canvas->displayRect[2], canvas->cinRect[1]);
	graphics->eraseRgn(0, canvas->cinRect[1] + canvas->cinRect[3], canvas->displayRect[2], canvas->softKeyY - (canvas->cinRect[1] + canvas->cinRect[3]));
	int n2 = app->hud->showCinPlayer ? (canvas->subtitleMaxChars - 7) : canvas->subtitleMaxChars;

	Text* largeBuffer = app->localization->getLargeBuffer();

	if (app->hud->cinTitleID != -1 && app->hud->cinTitleTime > app->gameTime) {
		largeBuffer->setLength(0);
		app->localization->composeText(app->hud->cinTitleID, largeBuffer);
		largeBuffer->wrapText(n2, 1, '\n');
		graphics->drawString(largeBuffer, scr_CX, 1, 1);
	}

	if (app->hud->subTitleID != -1 && app->hud->subTitleTime > app->gameTime) {
		int n3 = canvas->cinRect[1] + canvas->cinRect[3];
		int n4 = (n3 + (canvas->screenRect[3] - n3 - 32 >> 1)) - 10;
		largeBuffer->setLength(0);
		app->localization->composeText(app->hud->subTitleID, largeBuffer);
		largeBuffer->wrapText(n2, 2, '\n');
		if (app->hud->showCinPlayer) {
			graphics->drawRegion(app->hud->imgPlayerFaces, 0, 0, 32, 30, 5, n4 - (width - 32) / 2, 0, 0, 0);
			flags = 4;
			scr_CX = width + 10;
		}

		int first = largeBuffer->findFirstOf('\n', 0);
		if (first == -1) {
			graphics->drawString(largeBuffer, scr_CX, n4, flags);
		}
		else {
			graphics->drawString(largeBuffer, scr_CX, n4, flags, 0, first);
			graphics->drawString(largeBuffer, scr_CX, n4 + 16, flags, first + 1, 9999);
		}
	}
	largeBuffer->dispose();
	this->drawBubbleText(graphics);
}

void Hud::drawEffects(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->canvas->loadMapID >= 8 && app->tinyGL->fogRange < -1) {
		int n = (1024 + (app->tinyGL->fogRange >> 4) << 8) / 1024;
		int n2 = 0;
		int n3 = -(n * app->hud->imgIce->height >> 8);
		graphics->setScreenSpace(app->canvas->viewRect);
		graphics->drawImage(app->hud->imgIce, n2, n3, 0, 0, 0);
		graphics->drawImage(app->hud->imgIce, n2 + app->canvas->viewRect[2], n3, 24, 4, 0);
		graphics->resetScreenSpace();
	}
	if (app->canvas->state != Canvas::ST_DYING) {
		app->player->drawBuffs(graphics);
	}
	if (app->time < this->damageTime && this->damageCount > 0 && app->combat->totalDamage > 0) {
		if ((1 << this->damageDir & 0xC1) != 0x0) {
			int n4 = app->canvas->screenRect[3] - 20 - 25;
			int n5 = 0;
			int n6;
			if (this->damageDir == 0) {
				n6 = app->canvas->screenRect[2] - 20;
				n5 = 24;
			}
			else if (this->damageDir == 6) {
				n6 = 20;
				n5 = 12;
			}
			else {
				n6 = app->canvas->screenRect[2] >> 1;
			}
			graphics->drawRegion(this->imgAttArrow, 0, n5, 12, 12, n6, n4, 3, 0, 0);
		}
	}
	else if (this->damageTime != 0) {
		this->damageTime = 0;
		this->stopBrightenScreen();
		this->stopScreenSmack();
	}
	this->drawDamageVignette(graphics);
}

void Hud::drawDamageVignette(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->time < this->damageTime && this->damageCount >= 0 && app->combat->totalDamage > 0) {
		int n = 0;
		switch (this->damageDir) {
			case 1: {
				n = 2;
				break;
			}
			case 2: {
				n = 3;
				break;
			}
			case 3: {
				n = 15;
				break;
			}
			case 4: {
				n = 5;
				break;
			}
			case 5: {
				n = 4;
				break;
			}
			case 0:
			case 6:
			case 7: {
				n = 8;
				break;
			}
		}
		Image* image = app->player->isFamiliar ? this->imgDamageVignetteBot : this->imgDamageVignette;
		int width = image->width;
		if (n & 0x1) {
			graphics->fillRegion(image, app->canvas->viewRect[0], app->canvas->viewRect[1], app->canvas->viewRect[2], width, 1);
		}
		if (n & 0x2) {
			graphics->fillRegion(image, app->canvas->viewRect[0] + (app->canvas->viewRect[2] - width), app->canvas->viewRect[1], width, app->canvas->viewRect[3], 4);
		}
		if (n & 0x4) {
			graphics->fillRegion(image, app->canvas->viewRect[0], app->canvas->viewRect[1], width, app->canvas->viewRect[3], 0);
		}
		if (n & 0x8) {
			graphics->fillRegion(image, app->canvas->viewRect[0], (256 - width), app->canvas->viewRect[2], width, 3);
		}
	}
}

void Hud::smackScreen(int vScrollVelocity) {
	Applet* app = CAppContainer::getInstance()->app;

	app->render->vScrollVelocity = (vScrollVelocity * 90) / 100;
	app->render->lastScrollChangeTime = app->time;
}

void Hud::stopScreenSmack() {
	Applet* app = CAppContainer::getInstance()->app;

	app->render->vScrollVelocity = 0;
	app->render->screenVScrollOffset = 0;
	app->render->lastScrollChangeTime = 0;
}

void Hud::brightenScreen(int maxLocalBrightness, int brightnessInitialBoost) {
	Applet* app = CAppContainer::getInstance()->app;

	app->render->maxLocalBrightness = maxLocalBrightness;
	app->render->brightenPPMaxReachedTime = 0;
	app->render->brightenPostProcessBeginTime = app->time;
	app->render->brightenPostProcess = true;
	app->render->brightnessInitialBoost = brightnessInitialBoost;
}

void Hud::stopBrightenScreen() {
	Applet* app = CAppContainer::getInstance()->app;

	app->render->maxLocalBrightness = 0;
	app->render->brightenPPMaxReachedTime = 0;
	app->render->brightenPostProcessBeginTime = 0;
	app->render->brightenPostProcess = false;
	app->render->brightnessInitialBoost = 0;
}

void Hud::drawOverlay(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	graphics->drawImage(app->hud->imgCockpitOverlay, app->canvas->cinRect[0], app->canvas->cinRect[1], 0, 0, 0);
	graphics->drawImage(app->hud->imgCockpitOverlay, app->canvas->cinRect[2], app->canvas->cinRect[1], 24, 4, 0);
}

void Hud::drawHudOverdraw(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	if (app->player->isFamiliar) {
		return;
	}
	graphics->drawRegion(app->hud->imgHudTest, 0, 0, app->canvas->hudRect[2], 13, app->canvas->hudRect[0], app->canvas->hudRect[3] - 49, 20, 0, 0);
}

void Hud::drawBottomBar(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	fmButton* bnt0 = this->m_hudButtons->GetButton(0);
	fmButton* bnt1 = this->m_hudButtons->GetButton(1);
	app->canvas->drawTouchSoftkeyBar(graphics, bnt0->highlighted, bnt1->highlighted);

	bool noclip = app->player->noclip;
	if (app->player->isFamiliar) {
		int n = 118;
		short n2 = app->player->ammo[7];
		int n3;
		if (n2 > 80) {
			n3 = 0xFF59A907;
		}
		else if (n2 > 60) {
			n3 = 0xFF96DA0E;
		}
		else if (n2 > 40) {
			n3 = 0xFFEDD703;
		}
		else if (n2 > 20) {
			n3 = 0xFFFF8B00;
		}
		else {
			n3 = 0xFFFF0C00;
		}

		if (this->m_hudButtons->GetButton(3)->highlighted) {
			n3 = 0xFFFFFFFF;
			graphics->drawImage(this->imgSentryBotActive, 143, 261, 0, 0, 0); // Old 262, 0, 0, 0);
		}

		graphics->drawRegion(this->imgSentryBotFace, 0, (app->player->familiarType == 3 || app->player->familiarType == 4) ? 17 : 0, 29, 17, app->canvas->SCR_CX - (15 + n) / 2, 278, 3, /*noclip ? 3 :*/ 0, 0);

		int n5 = app->canvas->SCR_CX + this->imgSentryBotFace->width - ((15 + n) / 2);
		int n7 = n2 * 20 / 100;
		if (n7 < 20 && n2 != 0) {
			++n7;
		}
		int n8 = n5;
		for (int i = 0; i < n7; ++i) {
			graphics->fillRect(n8, 271, 4, 14, n3);
			n8 += 6;
		}
	}
	else {
		this->drawWeapon(graphics, 268, 258, app->player->ce->weapon, this->m_hudButtons->GetButton(2)->highlighted);

		Image* imgShield = (this->m_hudButtons->GetButton(4)->highlighted) ? this->imgShieldButtonActive : this->imgShieldNormal;
		graphics->drawImage(imgShield, 49, 258, 0, 0, 0);
		this->drawNumbers(graphics, 89, 268, 1, app->player->ce->getStat(2), -1);
		this->drawCurrentKeys(graphics, 375, 258);

		Image* imgHealth = (this->m_hudButtons->GetButton(5)->highlighted) ? this->imgHealthButtonActive : this->imgHealthNormal;
		graphics->drawImage(imgHealth, 133, 258, 0, 0, 0);
		this->drawNumbers(graphics, 173, 268, 1, app->player->ce->getStat(0), -1);

		int v28 = app->player->ce->getStat(0);
		int v29 = 4 - 5 * v28 / app->player->ce->getStat(1);
		if (v29 < 0) {
			v29 = 0;
		}

		if (this->m_hudButtons->GetButton(3)->highlighted) {
			graphics->drawRegion(this->imgPlayerActive, 0, 32 * v29, this->imgPlayerActive->width, 32, 224, 263, 20, 0, 0);
			graphics->drawImage(this->imgPlayerFrameActive, 216, 255, 20, 0, 0);
		}
		else {
			if (this->imgPlayerFaces) {
				graphics->drawRegion(this->imgPlayerFaces, 0, 32 * v29, this->imgPlayerFaces->width, 32, 224, 263, 20, 0, 0);
			}
			graphics->drawImage(this->imgPlayerFrameNormal, 216, 255, 20, 0, 0);
		}
	}

	if (app->canvas->softKeyLeftID != -1 && app->canvas->softKeyRightID != -1)
	{
		Text* texBuff = app->localization->getSmallBuffer();
		texBuff->setLength(0);
		texBuff->append("Wait");
		texBuff->dehyphenate();
		graphics->drawString(texBuff, 240, 320, 33);
		texBuff->dispose();
	}
}

void Hud::draw(Graphics* graphics) {
	//printf("Hud::draw");
	Applet* app = CAppContainer::getInstance()->app;

	this->drawTime = app->upTimeMs;
	if ((this->repaintFlags & 0x1) != 0x0) {
		//this->repaintFlags &= 0xFFFFFFFE;
		if (app->canvas->isZoomedIn) {
			graphics->drawImage(this->imgScope, 0, 0x14, 0, 0, 0);
		}
		this->drawEffects(graphics);
	}
	if ((this->repaintFlags & 0x2) != 0x0) {
		//this->repaintFlags &= 0xFFFFFFFD;
		if (app->canvas->state == Canvas::ST_PLAYING) {
			app->canvas->updateFacingEntity = true;
			app->canvas->checkFacingEntity();
		}
		this->drawTopBar(graphics);
	}
	//if ((this->repaintFlags & 0x20) != 0x0) {
		//this->repaintFlags &= 0xFFFFFFDF;
		//this->drawHudOverdraw(graphics);
	//}
	if ((this->repaintFlags & 0x4) != 0x0) {
		//this->repaintFlags &= 0xFFFFFFFB;
		if (app->canvas->state != Canvas::ST_DIALOG) {
			if (app->canvas->isZoomedIn == false) {
				this->drawArrowControls(graphics);
			}
			else {
				app->canvas->m_sniperScopeButtons->Render(graphics);
				fmScrollButton* m_sniperScopeDialScrollButton = app->canvas->m_sniperScopeDialScrollButton;
				int x = m_sniperScopeDialScrollButton->barRect.x;
				int y = m_sniperScopeDialScrollButton->barRect.y;
				graphics->drawImage(app->canvas->imgSniperScope_Dial, x, y, 0, 0, 0);
				int knobY = 0;
				if (m_sniperScopeDialScrollButton->field_0x0_) {
					knobY = m_sniperScopeDialScrollButton->field_0x48_ + (m_sniperScopeDialScrollButton->field_0x4c_ >> 1);
				}
				graphics->drawImage(
					app->canvas->imgSniperScope_Knob,
					x - (app->menuSystem->imgMenuDialKnob->width >> 1) + (app->canvas->imgSniperScope_Dial->width >> 1),
					y + (knobY << 2) / 5 - (app->menuSystem->imgMenuDialKnob->height >> 1) + 16,
					0,
					0,
					0);
			}
		}
		if (this->isInWeaponSelect != false) {
			this->drawWeaponSelection(graphics);
		}
		else {
			// [GEC] Prevent it from selecting checkboxes and producing sound
			for (int i = 0; i < 15; i++) {
				this->m_weaponsButtons->GetButton(i)->drawButton = false;
			}
		}

		this->drawBottomBar(graphics);
	}
	if ((this->repaintFlags & 0x8) != 0x0) {
		this->drawBubbleText(graphics);
	}
	if (this->cinTitleID != -1 && this->cinTitleTime < app->gameTime) {
		this->cinTitleID = -1;
		if (app->canvas->state == Canvas::ST_CAMERA) {
			this->repaintFlags |= 0x10;
		}
	}
	if (this->subTitleID != -1 && this->subTitleTime < app->gameTime) {
		this->subTitleID = -1;
		if (app->canvas->state == Canvas::ST_CAMERA) {
			this->repaintFlags |= 0x10;
		}
	}
	if ((this->repaintFlags & 0x10) != 0x0) {
		//this->repaintFlags &= 0xFFFFFFEF;
		this->drawCinematicText(graphics);

		if (app->canvas->softKeyRightID != -1) {
			Text* texBuff = app->localization->getSmallBuffer();
			texBuff->setLength(0);
			app->localization->composeText(app->canvas->softKeyRightID, texBuff);
			texBuff->dehyphenate();

			app->setFontRenderMode(2); // [GEC] New
			if (app->canvas->m_softKeyButtons->GetButton(20)->highlighted != false) { // [GEC] New
				app->setFontRenderMode(0); // [GEC] New
			}
			graphics->drawString(texBuff, 478, 320, 40);

			app->setFontRenderMode(0); // [GEC] New
			texBuff->dispose();
		}
	}
	this->drawTime = app->upTimeMs - this->drawTime;
}

void Hud::drawMonsterHealth(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	Entity* facingEntity = app->player->facingEntity;
	if (facingEntity == nullptr || facingEntity->monster == nullptr) {
		if (this->lastTarget != nullptr) {
			app->canvas->invalidateRect();
		}
		this->lastTarget = nullptr;
		return;
	}
	if (facingEntity->def->eType == 3 || (facingEntity->def->eType == 2 && (facingEntity->info & 0x20000) == 0x0)) {
		return;
	}
	int stat = facingEntity->monster->ce.getStat(1);
	int stat2 = facingEntity->monster->ce.getStat(0);
	if (facingEntity != this->lastTarget) {
		if (this->lastTarget != nullptr) {
			app->canvas->invalidateRect();
		}
		this->lastTarget = facingEntity;
		this->monsterDestHealth = (this->monsterStartHealth = stat2);
		this->monsterHealthChangeTime = 0;
	}
	else if (stat2 != this->monsterDestHealth) {
		this->monsterStartHealth = this->monsterDestHealth;
		this->monsterDestHealth = stat2;
		this->monsterHealthChangeTime = app->time;
	}
	if (this->monsterStartHealth > stat) {
		this->monsterStartHealth = stat;
	}
	if (app->time - this->monsterHealthChangeTime > 250) {
		this->monsterStartHealth = stat2;
	}
	else {
		stat2 = this->monsterStartHealth - (this->monsterStartHealth - this->monsterDestHealth) * (app->time - this->monsterHealthChangeTime) / 250;
	}
	int n = 25;
	int n2 = ((n << 8) * ((stat2 << 16) / (stat << 8)) >> 8) + 256 - 1 >> 8;
	if (n2 == 0 && stat2 > 0) {
		n2 = 1;
	}
	int n3 = 6;
	if (facingEntity->def->eSubType == 5 && facingEntity->def->parm == 0) {
		n3 = 50;
	}
	else if (app->canvas->isZoomedIn) {
		n3 += 20;
	}
	int n4 = 2 * (app->canvas->screenRect[2] << 8) / 128 >> 8;
	if (facingEntity->isBoss()) {
		++n4;
	}
	else if ((n4 & 0x1) != 0x0) {
		++n4;
	}
	int n5 = 2 + n4 * n;
	int n6 = app->canvas->SCR_CX - (n5 >> 1);
	graphics->setColor(0xFF000000);
	graphics->fillRect(n6, app->canvas->viewRect[1] + n3, n5, n4 * 2 + 1);
	graphics->setColor(0xFFAAAAAA);
	graphics->drawRect(n6, app->canvas->viewRect[1] + n3, n5, n4 * 2 + 1);
	if (n2 <= n / 4) {
		graphics->setColor(0xFFFF0000);
	}
	else if (n - n2 <= n / 4) {
		graphics->setColor(0xFF00FF00);
	}
	else {
		graphics->setColor(0xFFFF8800);
	}
	n6 += 2;
	for (int i = 0; i < n2; ++i) {
		graphics->fillRect(n6, app->canvas->viewRect[1] + n3 + 2, n4 - 1, n4 * 2 - 2);
		n6 += n4;
	}
}

void Hud::showSpeechBubble(int i, int i2) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->bubbleText == nullptr) {
		this->bubbleText = app->localization->getSmallBuffer();
	}

	app->localization->composeText((int16_t)app->canvas->loadMapStringID, (int16_t)i, this->bubbleText);
	this->bubbleText->dehyphenate();
	this->bubbleTextTime = app->time + 1500;

	switch (i2) {
		case 0:
			this->bubbleColor = 0xFF800000;
			break;
		case 1:
			this->bubbleColor = 0xFF002864;
			break;
		case 2:
			this->bubbleColor = Canvas::PLAYER_DLG_COLOR;
			break;
		case 3:
			this->bubbleColor = 0xFF2E0854;
			break;
		case 4:
			this->bubbleColor = 0xFFFF9600;
			break;
		default:
			this->bubbleColor = 0xFF800000;
			break;
	}
}

void Hud::drawBubbleText(Graphics* graphics) {

	Applet* app = CAppContainer::getInstance()->app;

	if (this->bubbleText == nullptr) {
		this->repaintFlags &= ~8;
		return;
	}
	if (app->time >= this->bubbleTextTime) {
		this->bubbleTextTime = 0;
		this->bubbleText->dispose();
		this->bubbleText = nullptr;
		this->repaintFlags &= ~8;
		return;
	}
	int n = app->canvas->viewRect[1] + 1;
	int n2 = app->canvas->SCR_CX + 5;
	int n3 = 0;
	int n4 = 6;
	switch ((unsigned int)this->bubbleColor) {
		case 0xFF800000: {
			n3 = 0;
			break;
		}
		case 0xFF002864: {
			n3 = 10;
			break;
		}
		case 0xFF2E0854: {
			n3 = 20;
			break;
		}
		case (unsigned int)Canvas::PLAYER_DLG_COLOR: {
			n3 = 30;
			n4 = 12;
			break;
		}
		case 0xFFFF9600: {
			n3 = 45;
			n4 = 12;
			break;
		}
	}
	/*if (app->canvas->state == Canvas::ST_CAMERA) { // J2ME
		n = app->canvas->cinRect[1] + 1;
	}*/
	if (app->player->facingEntity != nullptr && this->bubbleColor != Canvas::PLAYER_DLG_COLOR) {
		if (app->player->facingEntity->distFrom(app->canvas->destX, app->canvas->destY) <= app->combat->tileDistances[0]) {
			n += 10;
		}
		else {
			n += 20;
		}
	}
	int n5 = this->bubbleText->length() * 9 + 6;
	int n6 = 20;
	int n7 = n2 - std::max(0, n5 + 2 - (app->canvas->viewRect[2] - n2));
	if (this->bubbleColor == Canvas::PLAYER_DLG_COLOR) {
		n = app->canvas->screenRect[3] - (64 + n6 + 1);
	}
	else if (n7 + 15 < app->canvas->SCR_CX) {
		n4 = 12;
	}
	graphics->setColor(this->bubbleColor);
	graphics->fillRect(n7, n, n5, n6);
	graphics->setColor(-1);
	graphics->drawLine(n7, n, n7 + n5, n);
	graphics->drawLine(n7, n, n7, n + n6);
	graphics->drawLine(n7 + n5, n, n7 + n5, n + n6);
	graphics->drawLine(n7, n + n6, n7 + n5, n + n6);
	graphics->drawString(this->bubbleText, n7 + 2, n + 3, 4);
	graphics->drawRegion(app->canvas->imgUIImages, n3, n4, 10, 6, n7 + 5, n + n6, 0, 0, 0);
}

void Hud::drawArrowControls(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;
	Image* imgDpad;

	if (app->canvas->state != Canvas::ST_TREADMILL) {

		if (app->canvas->m_controlMode == 1) {
			app->canvas->setBlendSpecialAlpha((float)(app->canvas->m_controlAlpha * 0.01f));

			int x = app->canvas->m_controlButtons[app->canvas->m_controlMode + 2]->GetButton(5)->touchArea.x + 13;
			int y = app->canvas->m_controlButtons[app->canvas->m_controlMode + 0]->GetButton(3)->touchArea.y + 13;

			int buttonID = app->canvas->m_controlButtons[app->canvas->m_controlMode + 2]->GetHighlightedButtonID();
			if (buttonID == 5) {
				imgDpad = app->canvas->imgDpad_left_press;
			}
			else if (buttonID == 7) {
				imgDpad = app->canvas->imgDpad_right_press;
			}
			else {
				buttonID = app->canvas->m_controlButtons[app->canvas->m_controlMode + 0]->GetHighlightedButtonID();
				if ((buttonID == 16) || (buttonID == 3)) {
					imgDpad = app->canvas->imgDpad_up_press;
				}
				else if ((buttonID == 17) || (buttonID == 9)) {
					imgDpad = app->canvas->imgDpad_down_press;
				}
				else {
					imgDpad = app->canvas->imgDpad_default;
				}
			}
			graphics->drawImage(imgDpad, x, y, 0, 0, 13);
		}

		app->canvas->m_controlButtons[app->canvas->m_controlMode + 2]->Render(graphics);
		app->canvas->m_controlButtons[app->canvas->m_controlMode + 0]->Render(graphics);
		app->canvas->m_controlButtons[app->canvas->m_controlMode + 4]->Render(graphics);
	}
}

void Hud::drawWeapon(Graphics* graphics, int x, int y, int weapon, bool highlighted) {
	int texY;
	bool drawNumbers;
	Applet* app = CAppContainer::getInstance()->app;

	switch (weapon)
	{
	case 0:
		texY = 0;
		drawNumbers = true;
		break;
	case 1:
		texY = 1;
		drawNumbers = false;
		break;
	case 2:
		texY = 2;
		drawNumbers = true;
		break;
	case 3:
	case 4:
		texY = 10;
		drawNumbers = true;
		break;
	case 5:
	case 6:
		texY = 11;
		drawNumbers = true;
		break;
	case 7:
		texY = 3;
		drawNumbers = true;
		break;
	case 8:
		texY = 4;
		drawNumbers = true;
		break;
	case 9:
		texY = 5;
		drawNumbers = true;
		break;
	case 10:
		texY = 6;
		drawNumbers = true;
		break;
	case 11:
		texY = 7;
		drawNumbers = true;
		break;
	case 12:
		texY = 8;
		drawNumbers = true;
		break;
	case 13:
		texY = 9;
		drawNumbers = true;
		break;
	case 14:
		texY = 12;
		drawNumbers = true;
		break;
	default:
		texY = 13;
		drawNumbers = false;
		break;
	}
	if (highlighted) {
		graphics->drawRegion(this->imgWeaponActive, 0, texY * 44, this->imgWeaponActive->width, 44, x, y, 20, 0, 0);
	}
	else {
		graphics->drawRegion(this->imgWeaponNormal, 0, texY * 44, this->imgWeaponNormal->width, 44, x, y, 20, 0, 0);
	}
	if (drawNumbers) {
		this->drawNumbers(graphics, (x + (this->imgWeaponNormal->width >> 1) + 6), y + 8, 1,
			app->player->ammo[app->combat->weapons[9 * weapon + 4]],
			weapon);
	}
}

void Hud::drawNumbers(Graphics* graphics, int x, int y, int space, int num, int weapon) {
	int v11;
	int v12;
	int v15;
	Applet* app = CAppContainer::getInstance()->app;

	if (num < 1000)
	{
		v11 = num / 100;
		v12 = num % 100 / 10;
		v15 = num % 100 % 10;
		if (weapon == 13) {
			v11 = num % 100 % 10;
			v15 = 5;
		}
		if (weapon == 13) {
			v12 = -1;
		}
		graphics->drawRegion(this->imgNumbers, 0, 20 * (9 - v11), 10, 20, x, y, 20, 0, 0);
		int posX = x + space + 10;
		graphics->drawRegion(this->imgNumbers, 0, 20 * (9 - v12), 10, 20, posX, y, 20, 0, 0);
		graphics->drawRegion(this->imgNumbers, 0, 20 * (9 - v15), 10, 20, space + posX + 10, y, 20, 0, 0);
	}
	else {
		puts("ERROR: drawnumbers() does not currently support values over 999 ");
	}
}

void Hud::drawCurrentKeys(Graphics* graphics, int x, int y) {
	Applet* app = CAppContainer::getInstance()->app;
	int v9;

	Player* player = app->player;
	if (player->inventory[19] <= 0) {
		if (player->inventory[20] <= 0) {
			v9 = 0;
		}
		else {
			v9 = 2;
		}
	}
	else if (player->inventory[20] <= 0) {
		v9 = 1;
	}
	else {
		v9 = 3;
	}
	fmButton* Button = this->m_hudButtons->GetButton(6);
	if (Button->highlighted)
		return graphics->drawRegion(this->imgKeyActive, 0, 44 * v9, this->imgKeyActive->width, 44, x, y, 20, 0, 0);
	else
		return graphics->drawRegion(this->imgKeyNormal, 0, 44 * v9, this->imgKeyNormal->width, 44, x, y, 20, 0, 0);
}

void Hud::drawWeaponSelection(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	int weapons = ~app->player->disabledWeapons & app->player->weapons;
	int x = (app->canvas->screenRect[2] - 16 - 4 * this->imgWeaponNormal->width) / 2;
	int y = app->canvas->screenRect[3] - 108;
	graphics->FMGL_fillRect(0, this->imgPanelTop->height, 480, 320, 0.0f, 0.0f, 0.0f, 0.5f);

	for (int i = 0; i < 15; i++) {
		this->m_weaponsButtons->GetButton(i)->drawButton = false;
	}

	int posY = -4;
	int v8 = 0;
	int posX = 0;
	for (int i = 0; i < 15; i++) {
		if (((weapons >> i) & 1) != 0) {
			if (v8 > 0 && (v8 & 3) == 0)
			{
				posY -= 48;
				posX = 0;
			}
			v8++;

			fmButton* button = this->m_weaponsButtons->GetButton(i);
			button->drawButton = true;
			this->drawWeapon(graphics, posX + x, posY + y, i, button->highlighted);
			button->SetTouchArea(posX + x, posY + y, this->imgWeaponNormal->width, 44);
			posX += this->imgWeaponNormal->width + 4;
		}
	}
}

void Hud::handleUserMoved(int pressX, int pressY) {
	for (int i = 0; i < 7; i++) {
		this->m_hudButtons->GetButton(i)->SetHighlighted(false);
	}

	for (int i = 0; i < 15; i++) {
		this->m_weaponsButtons->GetButton(i)->SetHighlighted(false);
	}

	int buttonID_1 = this->m_hudButtons->GetTouchedButtonID(pressX, pressY);
	int buttonID_2 = this->m_weaponsButtons->GetTouchedButtonID(pressX, pressY);

	if (buttonID_2 >= 0 && this->m_weaponsButtons->GetButton(buttonID_2)->drawButton) {
		this->m_weaponsButtons->GetButton(buttonID_2)->SetHighlighted(true);
	}
	else if (buttonID_1 >= 0 && this->m_hudButtons->GetButton(buttonID_1)->drawButton) {
		this->m_hudButtons->GetButton(buttonID_1)->SetHighlighted(true);
	}
}

void Hud::handleUserTouch(int pressX, int pressY, bool highlighted) {
	Applet* app = CAppContainer::getInstance()->app;
	int buttonID;

	if (app->canvas->blockInputTime)
		return;

	if (app->player->isFamiliar)
	{
		buttonID = this->m_hudButtons->GetTouchedButtonID(pressX, pressY);
		if (buttonID >= 0) {
			if ((pressX > 143) && ((this->imgSentryBotActive->width + 143) > pressX) &&
				(pressY > 262) && ((this->imgSentryBotActive->height + 262) > pressY)) {
				buttonID = 3;
			}

			if (highlighted) {
				this->m_hudButtons->GetButton(buttonID)->SetHighlighted(true);
			}
			else {
				if (buttonID == 1) {
					app->canvas->handlePlayingEvents(0, Enums::ACTION_BOT_DISCARD /*OLD -> Enums::ACTION_AUTOMAP*/);
				}
				else if (buttonID == 3) {
					app->canvas->handlePlayingEvents(0, Enums::ACTION_PASSTURN);
				}
				else if (buttonID == 0) {
					app->canvas->handlePlayingEvents(0, Enums::ACTION_MENU);
				}
				this->m_hudButtons->HighlightButton(0, 0, false);
			}
		}
	}
	else {
		for (int i = 0; i < 7; i++) {
			this->m_hudButtons->GetButton(i)->SetHighlighted(false);
		}

		for (int i = 0; i < 15; i++) {
			this->m_weaponsButtons->GetButton(i)->SetHighlighted(false);
		}

		if (app->canvas->touched) { return; }

		int buttonID_1 = this->m_hudButtons->GetTouchedButtonID(pressX, pressY);
		int buttonID_2 = this->m_weaponsButtons->GetTouchedButtonID(pressX, pressY);

		if (buttonID_2 >= 0 && this->isInWeaponSelect) {
			if (highlighted) {
				this->m_weaponsButtons->GetButton(buttonID_2)->SetHighlighted(true);
			}
			else {
				app->player->selectWeapon(buttonID_2);
				this->isInWeaponSelect = false;
			}
		}
		else if (buttonID_1 >= 0) {
			if (highlighted) {
				this->m_hudButtons->GetButton(buttonID_1)->SetHighlighted(true);
				if (buttonID_1 == 2) {
					this->weaponPressTime = app->upTimeMs;
				}
			}
			else
			{
				if (buttonID_1 != 2) {
					this->isInWeaponSelect = false;
				}

				if (app->canvas->state != Canvas::ST_COMBAT) {
					switch (buttonID_1)
					{
						case 0:
							if (app->canvas->isZoomedIn) {
								app->canvas->handleZoomEvents(0, Enums::ACTION_BACK);
							}
							app->canvas->handlePlayingEvents(0, Enums::ACTION_MENU);
							break;
						case 1:
							if (app->canvas->isZoomedIn) {
								app->canvas->handleZoomEvents(0, Enums::ACTION_BACK);
							}

							if (app->player->weaponIsASentryBot(app->player->ce->weapon) && app->canvas->state != Canvas::ST_AUTOMAP) { // [GEC]
								app->canvas->handlePlayingEvents(0, Enums::ACTION_BOT_DISCARD);
							}
							else {
								app->canvas->handlePlayingEvents(0, Enums::ACTION_AUTOMAP);
							}
							break;
						case 2:
							if (app->canvas->isZoomedIn) {
								app->canvas->handleZoomEvents(0, Enums::ACTION_BACK);
							}
							else {
								if ((uint32_t)(app->upTimeMs - this->weaponPressTime) >= 300) {
									break;
								}
								else {
									if (this->isInWeaponSelect) {
										this->isInWeaponSelect = false;
									}
									else {
										app->canvas->handlePlayingEvents(this->isInWeaponSelect, Enums::ACTION_NEXTWEAPON);
									}
									this->weaponPressTime = 0;
								}
							}
							break;
						case 3:
							app->canvas->handlePlayingEvents(0, Enums::ACTION_PASSTURN);
							break;
						case 4:
							app->canvas->handlePlayingEvents(0, Enums::ACTION_ITEMS_DRINKS);
							break;
						case 5:
							app->canvas->handlePlayingEvents(0, Enums::ACTION_ITEMS);
							break;
						case 6:
							app->canvas->handlePlayingEvents(0, Enums::ACTION_QUESTLOG);
							break;
						default:
							printf("ERROR: undefined touch button ID: %d \n", buttonID_1);
							break;
					}
				}
			}
		}
	}
}

void Hud::update() {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->m_hudButtons->GetButton(2)->drawButton && this->m_hudButtons->GetButton(2)->highlighted) {
		if (this->weaponPressTime) {
			if ((uint32_t)(app->upTimeMs - this->weaponPressTime) >= 300) {
				if (app->canvas->state != Canvas::ST_COMBAT && !app->canvas->isZoomedIn) {
					this->isInWeaponSelect = true;
				}
				this->weaponPressTime = 0;
			}
		}
	}
}
