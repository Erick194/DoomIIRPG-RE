#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Canvas.h"
#include "MenuItem.h"
#include "MenuSystem.h"
#include "Text.h"

MenuItem::MenuItem() {
}

MenuItem::~MenuItem() {
}

void MenuItem::Set(int textField, int textField2, int flags) {
	this->Set(textField, textField2, flags, 0, 0, MenuSystem::EMPTY_TEXT);
}

void MenuItem::Set(int textField, int textField2, int flags, int action, int param, int helpField) {
	this->textField = textField;
	this->textField2 = textField2;
	this->flags = flags;
	this->helpField = helpField;
	this->param = param;
	this->action = action;
}

void MenuItem::WrapHelpText(Text* text) {
	Applet* app = CAppContainer::getInstance()->app;

	app->localization->composeTextField(this->helpField, text);
	text->wrapText(app->canvas->menuHelpMaxChars, 0xc, '\n');
}