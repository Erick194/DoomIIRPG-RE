#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Canvas.h"
#include "Button.h"
#include "Image.h"
#include "Graphics.h"
#include "Sound.h"


// ---------------
// GuiRect Class
// ---------------
void GuiRect::Set(int x, int y, int w, int h) {
	this->y = y;
	this->w = w;
	this->x = x;
	this->h = h;
}

bool GuiRect::ContainsPoint(int x, int y) {
	return ((x >= this->x) && (y >= this->y) && (x <= (this->x + this->w)) && (y <= (this->y + this->h)));
}

// ---------------
// fmButton Class
// ---------------

fmButton::fmButton(int buttonID, int x, int y, int w, int h, int soundResID) {
	//printf("fmButton::init\n");
	std::memset(this, 0, sizeof(fmButton));

	this->buttonID = buttonID;
	this->normalIndex = -1;
	this->highlightIndex = -1;
	this->SetTouchArea(x, y, w, h);
	this->imgNormal = nullptr;
	this->ptrNormalImages = nullptr;
	this->imgHighlight = nullptr;
	this->ptrHighlightImages = nullptr;
	this->selectedIndex = -1;
	this->drawButton = true;
	this->highlighted = false;
	this->normalRenderMode = 0;
	this->highlightRenderMode = 0;
	this->centerX = 0;
	this->centerY = 0;
	this->highlightCenterX = 0;
	this->highlightCenterY = 0;
	this->drawTouchArea = false;
	this->next = nullptr;
	this->soundResID = (short)soundResID;
	this->highlightRed = 0.0f;
	this->highlightGreen = 0.3f;
	this->highlightBlue = 0.9f;
	this->highlightAlpha = 0.9f;
	this->normalRed = 0.2f;
	this->normalGreen = 0.6f;
	this->normalBlue = 0.3f;
	this->normalAlpha = 0.2f;
}

fmButton::~fmButton() {
}

void fmButton::SetImage(Image* image, bool center) {
	this->ptrNormalImages = nullptr;
	this->imgNormal = image;
	this->normalIndex = -1;
	if (center) {
		this->centerX = (this->touchArea.w - image->width) / 2;
		this->centerY = (this->touchArea.h - image->height) / 2;
	}
}
void fmButton::SetImage(Image** ptrImages, int imgIndex, bool center) {
	this->imgNormal = nullptr;
	this->ptrNormalImages = ptrImages;
	this->normalIndex = imgIndex;
	if (center) {
		this->centerX = (this->touchArea.w - ptrImages[imgIndex]->width) / 2;
		this->centerY = (this->touchArea.h - ptrImages[imgIndex]->height) / 2;
	}
}

void fmButton::SetHighlightImage(Image* imgHighlight, bool center) {
	this->ptrHighlightImages = nullptr;
	this->imgHighlight = imgHighlight;
	this->highlightIndex = -1;
	if (center) {
		this->highlightCenterX = (this->touchArea.w - imgHighlight->width) / 2;
		this->highlightCenterY = (this->touchArea.h - imgHighlight->height) / 2;
	}
}

void fmButton::SetHighlightImage(Image** ptrImgsHighlight, int imgHighlightIndex, bool center) {
	this->imgHighlight = nullptr;
	this->ptrHighlightImages = ptrImgsHighlight;
	this->highlightIndex = imgHighlightIndex;
	if (center) {
		this->highlightCenterX = (this->touchArea.w - ptrImgsHighlight[imgHighlightIndex]->width) / 2;
		this->highlightCenterY = (this->touchArea.h - ptrImgsHighlight[imgHighlightIndex]->height) / 2;
	}
}

void fmButton::SetGraphic(int index)
{
	if (this->ptrNormalImages) {
		this->normalIndex = index;
		this->centerX = (this->touchArea.w - this->ptrNormalImages[index]->width) / 2;
		this->centerY = (this->touchArea.h - this->ptrNormalImages[index]->height) / 2;
	}

	if (this->ptrHighlightImages) {
		this->highlightIndex = index;
		this->highlightCenterX = (this->touchArea.w - this->ptrHighlightImages[index]->width) / 2;
		this->highlightCenterY = (this->touchArea.h - this->ptrHighlightImages[index]->height) / 2;
	}
}

void fmButton::SetTouchArea(int x, int y, int w, int h) {
#if 0 // Old
	this->touchArea.x = x;
	this->touchArea.y = y;
	this->touchArea.w = w;
	this->touchArea.h = h;
#endif
	this->SetTouchArea(x, y, w, h, true);
}

void fmButton::SetTouchArea(int x, int y, int w, int h, bool drawing) { // Port: new
	this->touchArea.x = x;
	this->touchArea.y = y;
	this->touchArea.w = w;
	this->touchArea.h = h;

	if (drawing) {
		this->touchAreaDrawing.x = x;
		this->touchAreaDrawing.y = y;
		this->touchAreaDrawing.w = w;
		this->touchAreaDrawing.h = h;
	}
}

void fmButton::SetHighlighted(bool highlighted) {
	Applet* app = CAppContainer::getInstance()->app;

	if (this->drawButton != 0) {

		if (!this->highlighted && highlighted) {
			if (this->soundResID != -1) {
				if (!app->sound->isSoundPlaying(this->soundResID)) {
					app->sound->playSound(this->soundResID, 0, 5, false);
				}
			}
		}
		this->highlighted = highlighted;
	}
}

void fmButton::Render(Graphics* graphics) {
	Applet* app = CAppContainer::getInstance()->app;

	//printf("fmButton::Render\n");

	if (!this->drawButton)
		return;

	if (this->highlighted) {
		if (this->imgHighlight){
			if (this->highlightRenderMode == 13) {
				app->canvas->setBlendSpecialAlpha((float)(app->canvas->m_controlAlpha * 0.01f));
			}
			graphics->drawImage(this->imgHighlight,
				this->touchAreaDrawing.x + this->highlightCenterX, this->touchAreaDrawing.y + this->highlightCenterY, 0, 0, this->highlightRenderMode);
			return;
		}
		else {
			if (!this->drawTouchArea && this->imgNormal) {
				graphics->drawImage(this->imgNormal,
					this->touchAreaDrawing.x + this->centerX, this->touchAreaDrawing.y + this->centerY, 0, 0, this->highlightRenderMode);
				return;
			}
			else if (this->ptrHighlightImages && this->highlightIndex != -1) {
				if (this->highlightRenderMode == 13) {
					app->canvas->setBlendSpecialAlpha((float)(app->canvas->m_controlAlpha * 0.01f));
				}
				graphics->drawImage(ptrHighlightImages[this->highlightIndex],
					this->touchAreaDrawing.x + this->highlightCenterX, this->touchAreaDrawing.y + this->highlightCenterY, 0, 0, this->highlightRenderMode);
				return;
			}
		}
	}

	if (this->imgNormal) {
		if (this->normalRenderMode == 13){
			app->canvas->setBlendSpecialAlpha((float)(app->canvas->m_controlAlpha * 0.01f));
		}
		graphics->drawImage(this->imgNormal,
			this->touchAreaDrawing.x + this->centerX, this->touchAreaDrawing.y + this->centerY, 0, 0, this->normalRenderMode);
		return;
	}

	if (!this->ptrNormalImages || (this->ptrNormalImages && this->normalIndex == -1))
	{
		if (this->drawTouchArea) {
			if (this->highlighted) {
				graphics->FMGL_fillRect(this->touchAreaDrawing.x, this->touchAreaDrawing.y, this->touchAreaDrawing.w, this->touchAreaDrawing.h,
					this->highlightRed, this->highlightGreen, this->highlightBlue, this->highlightAlpha);
			}
			else {
				graphics->FMGL_fillRect(this->touchAreaDrawing.x, this->touchAreaDrawing.y, this->touchAreaDrawing.w, this->touchAreaDrawing.h,
					this->normalRed, this->normalGreen, this->normalBlue, this->normalAlpha);
			}
		}
	}
	else {
		if (this->normalRenderMode == 13) {
			app->canvas->setBlendSpecialAlpha((float)(app->canvas->m_controlAlpha * 0.01f));
		}
		graphics->drawImage(this->ptrNormalImages[this->normalIndex],
			this->touchAreaDrawing.x + this->centerX, this->touchAreaDrawing.y + this->centerY, 0, 0, this->normalRenderMode);
	}
}

// ------------------------
// fmButtonContainer Class
// ------------------------

fmButtonContainer::fmButtonContainer() {
	//printf("fmButtonContainer::init\n");
	std::memset(this, 0, sizeof(fmButtonContainer));
	this->next = nullptr;
	this->prev = nullptr;
}

fmButtonContainer::~fmButtonContainer() {
	fmButton* next;
	for (fmButton* button = this->next; button != nullptr; button = next){
		next = button->next;
		delete button;
	}
}

void fmButtonContainer::AddButton(fmButton* button) {
	if (this->next == nullptr) {
		this->next = button;
	}
	else {
		this->prev->next = button;
	}
	this->prev = button;
}

fmButton* fmButtonContainer::GetButton(int buttonID) {
	for (fmButton* next = this->next; next != nullptr; next = next->next) {
		if (next->buttonID == buttonID) {
			return next;
		}
	}

	return nullptr;
}

fmButton* fmButtonContainer::GetTouchedButton(int x, int y) {
	for (fmButton* next = this->next; next != nullptr; next = next->next) {
		if (next->drawButton && 
			(x >= next->touchArea.x) && (y >= next->touchArea.y) &&
			(x <= (next->touchArea.x + next->touchArea.w)) &&
			(y <= (next->touchArea.y + next->touchArea.h))) {
			return next;
		}
	}
	return nullptr;
}

int fmButtonContainer::GetTouchedButtonID(int x, int y) {
	for (fmButton* next = this->next; next != nullptr; next = next->next) {
		if (next->drawButton && 
			(x >= next->touchArea.x) && (y >= next->touchArea.y) &&
			(x <= (next->touchArea.x + next->touchArea.w)) &&
			(y <= (next->touchArea.y + next->touchArea.h))) {
			return next->buttonID;
		}
	}
	return -1;
}

int fmButtonContainer::GetHighlightedButtonID() {
	for (fmButton* next = this->next; next != nullptr; next = next->next) {
		if (next->drawButton && next->highlighted) {
			return next->buttonID;
		}
	}
	return -1;
}

void fmButtonContainer::HighlightButton(int x, int y, bool highlighted) {
	if (highlighted) {
		for (fmButton* next = this->next; next != nullptr; next = next->next) {
			if ((x >= next->touchArea.x) && (y >= next->touchArea.y) &&
				(x <= (next->touchArea.x + next->touchArea.w)) &&
				(y <= (next->touchArea.y + next->touchArea.h))) {
				next->SetHighlighted(true);
			}
			else {
				next->SetHighlighted(false);
			}
		}
	}
	else {
		for (fmButton* next = this->next; next != nullptr; next = next->next) {
			next->SetHighlighted(false);
		}
	}
}

void fmButtonContainer::SetGraphic(int index) {
	for (fmButton* next = this->next; next != nullptr; next = next->next) {
		next->SetGraphic(index);
	}
}

void fmButtonContainer::FlipButtons() {
	for (fmButton* next = this->next; next != nullptr; next = next->next) {
		next->touchArea.x = -next->touchArea.x - next->touchArea.w + 480;
		next->touchAreaDrawing.x = -next->touchAreaDrawing.x - next->touchAreaDrawing.w + 480; // New
	}
}

void fmButtonContainer::Render(Graphics* graphics) {
	for (fmButton* next = this->next; next != nullptr; next = next->next) {
		next->Render(graphics);
	}
}

// ---------------------
// fmScrollButton Class
// ---------------------

fmScrollButton::fmScrollButton(int x, int y, int w, int h, bool b, int soundResID) {

	this->imgBar = nullptr;
	this->imgBarTop = nullptr;
	this->imgBarMiddle = nullptr;
	this->imgBarBottom = nullptr;
	this->field_0x14_ = 0;
	this->field_0x15_ = b;
	this->barRect.x = x;
	this->barRect.y = y;
	this->barRect.w = w;
	this->barRect.h = h;
	this->field_0x0_ = 0;
	this->boxRect.x = 0;
	this->boxRect.y = 0;
	this->boxRect.w = 0;
	this->boxRect.h = 0;
	this->field_0x38_ = 0;
	this->field_0x3c_ = 0;
	this->field_0x40_ = 0;
	this->field_0x44_ = 0;
	this->field_0x48_ = 0;
	this->field_0x4c_ = 0;
	this->field_0x50_ = 0;
	this->field_0x54_ = 0;
	this->field_0x58_ = 0;
	this->soundResID = (short)soundResID;
}

fmScrollButton::~fmScrollButton() {
}

void fmScrollButton::SetScrollBarImages(Image* imgBar, Image* imgBarTop, Image* imgBarMiddle, Image* imgBarBottom) {
	this->imgBar = imgBar;
	this->imgBarTop = imgBarTop;
	this->imgBarMiddle = imgBarMiddle;
	this->imgBarBottom = imgBarBottom;
}

void fmScrollButton::SetScrollBox(int x, int y, int w, int h, int i) {
	int iVar1;
	int iVar2;
	int iVar3;
	float fVar4;
	float fVar5;

	this->field_0x3c_ = w;
	(this->boxRect).w = w;
	iVar2 = (this->barRect).w;
	if (this->field_0x15_ != false) {
		this->field_0x3c_ = h;
		iVar2 = (this->barRect).h;
	}
	iVar3 = this->field_0x3c_;
	(this->boxRect).y = y;
	(this->boxRect).x = x;
	(this->boxRect).h = h;
	this->field_0x40_ = i;
	this->field_0x44_ = 0;
	iVar1 = (iVar3 * iVar2/ this->field_0x40_);
	this->field_0x48_ = 0;
	fVar4 = (float)(this->field_0x40_ - iVar3);
	this->field_0x4c_ = iVar1;
	fVar5 = (float)(iVar2 - iVar1);
	this->field_0x50_ = (fVar4 / fVar5);
}

void fmScrollButton::SetScrollBox(int x, int y, int w, int h, int i, int i2)
{
	int iVar1;
	int iVar2;
	int iVar3;
	bool bVar4;
	float fVar5;
	float fVar6;

	this->field_0x3c_ = w;
	(this->boxRect).w = w;
	bVar4 = this->field_0x15_;
	(this->boxRect).x = x;
	(this->boxRect).y = y;
	bVar4 = bVar4 != false;
	this->field_0x40_ = i;
	this->field_0x44_ = 0;
	if (bVar4) {
		this->field_0x3c_ = h;
	}
	this->field_0x48_ = 0;
	iVar3 = this->field_0x40_;
	iVar1 = this->field_0x3c_;
	iVar2 = (this->barRect).w;
	if (bVar4) {
		iVar2 = (this->barRect).h;
	}
	(this->boxRect).h = h;
	fVar5 = (float)(iVar3 - iVar1);
	this->field_0x4c_ = i2;
	fVar6 = (float)(iVar2 - i2);
	this->field_0x50_ = fVar5 / fVar6;
	return;
}

void fmScrollButton::SetContentTouchOffset(int x, int y)
{
	if (this->field_0x0_ == 0) {
		return;
	}
	if (this->field_0x15_ != false) {
		x = y;
	}
	this->field_0x58_ = x;
	this->field_0x5c_ = this->field_0x44_;
	return;
}

void fmScrollButton::UpdateContent(int x, int y)
{
	int iVar1;
	int iVar2;
	float fVar3;

	if (this->field_0x0_ != 0) {
		if (this->field_0x15_ == false) {
			iVar1 = (this->field_0x5c_ - x) + this->field_0x58_;
		}
		else {
			iVar1 = (this->field_0x5c_ - y) + this->field_0x58_;
		}
		this->field_0x44_ = iVar1;
		iVar1 = this->field_0x44_;
		iVar2 = this->field_0x40_ - this->field_0x3c_;
		if (iVar1 < 0) {
			iVar1 = 0;
			this->field_0x44_ = 0;
		}
		if (iVar2 < iVar1) {
			this->field_0x44_ = iVar2;
			iVar1 = iVar2;
		}
		fVar3 = (float)iVar1;
		this->field_0x48_ = (int)(fVar3 / this->field_0x50_);
	}
}

void fmScrollButton::SetTouchOffset(int x, int y)
{
	if (this->field_0x0_ != 0) {
		if (this->field_0x15_ == false) {
			this->field_0x54_ = (x - (this->barRect).x) - this->field_0x48_;
		}
		else {
			this->field_0x54_ = (y - (this->barRect).y) - this->field_0x48_;
		}
	}
}

void fmScrollButton::Update(int x, int y) {
	Applet* app = CAppContainer::getInstance()->app;
	int iVar1;
	int iVar3;
	int iVar4;
	int in_cr7;
	int uVar5;

	if (this->field_0x0_ != 0) {
		iVar4 = this->field_0x54_;
		iVar1 = (this->barRect).h;
		iVar3 = ((y - (this->barRect).y) - (this->field_0x4c_ >> 1)) - iVar4;
		if (iVar3 < 0) {
			iVar4 = 0;
		}
		this->field_0x48_ = iVar3;
		if (iVar3 < 0) {
			this->field_0x48_ = iVar4;
		}
		else {
			iVar1 = iVar1 - this->field_0x4c_;
			if (iVar3 <= iVar1) {
				this->field_0x44_ = (int)((float)(iVar3) * this->field_0x50_);
				if (this->soundResID == -1) {
					return;
				}
				if (!app->sound->isSoundPlaying(this->soundResID)) {
					app->sound->playSound(this->soundResID, 0, 5, false);
				}
				return;
			}
			this->field_0x48_ = iVar1;
			iVar4 = this->field_0x40_ - this->field_0x3c_;
		}
		this->field_0x44_ = iVar4;
	}
	return;
}

void fmScrollButton::Render(Graphics* graphics) {
	//printf("fmScrollButton::Render\n");

	Image* img;
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	int x;

	if (this->field_0x0_) {
		img = this->imgBar;
		if (img == (Image*)0x0) {
			graphics->setColor(0x7f303030);
			graphics->fillRect(this->barRect.x, this->barRect.y, this->barRect.w, this->barRect.h);
			if (this->field_0x14_ == 0) {
				iVar1 = -0x555556;
			}
			else {
				iVar1 = -0x111156;
			}
			graphics->setColor(iVar1);
			if (this->field_0x15_ == false) {
				iVar1 = (this->barRect).h;
				iVar3 = (this->barRect).y;
				iVar4 = this->field_0x4c_;
				iVar2 = this->field_0x48_ + (this->barRect).x;
			}
			else {
				iVar1 = this->field_0x4c_;
				iVar2 = (this->barRect).x;
				iVar4 = (this->barRect).w;
				iVar3 = this->field_0x48_ + (this->barRect).y;
			}
			graphics->fillRect(iVar2, iVar3, iVar4, iVar1);
		}
		else {
			iVar1 = this->barRect.w;
			iVar2 = this->barRect.x;
			x = iVar2 + (iVar1 - this->imgBarTop->width >> 1);
			graphics->drawImage(img, iVar2 + (iVar1 - img->width >> 1), (this->barRect).y, 0, 0, 0);
			graphics->drawImage(this->imgBarTop, x, (this->barRect).y + this->field_0x48_, 0, 0, 0);
			iVar1 = (this->barRect).y + this->field_0x48_;
			iVar2 = this->field_0x4c_;
			iVar3 = this->imgBarBottom->height;
			for (iVar4 = iVar1 + this->imgBarTop->height; iVar4 < (iVar1 + iVar2) - iVar3;
				iVar4 = iVar4 + this->imgBarMiddle->height) {
				graphics->drawImage(this->imgBarMiddle, x, iVar4, 0, 0, 0);
			}
			graphics->drawImage(this->imgBarBottom, x, (this->barRect.y + this->field_0x48_ + this->field_0x4c_) -
				this->imgBarBottom->height, 0, 0, 0);
		}
	}
}

// ------------------
// fmSwipeArea Class
// ------------------

fmSwipeArea::fmSwipeArea(int x, int y, int w, int h, int endX, int endY) {
	//printf("fmSwipeArea::fmSwipeArea, %d, %d, %d, %d\n", x, y, w, h);
	std::memset(this, 0, sizeof(fmSwipeArea));
	this->rect.x = x;
	this->rect.y = y;
	this->rect.w = w;
	this->rect.h = h;
	this->enable = true;
	this->begX = -1;
	this->begY = -1;
	this->curX = -1;
	this->curY = -1;
	this->endX = endX;
	this->endY = endY;
	this->touched = false;
	this->drawTouchArea = false;
}

fmSwipeArea::~fmSwipeArea() {
}

int fmSwipeArea::UpdateSwipe(int x, int y, SwipeDir* swDir) {
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	int iVar5;

	*swDir = SwipeDir::Null;
	if ((this->touched == false) || (this->enable == false)) {
		return 0;
	}
	iVar3 = this->curX;
	if ((iVar3 < 0) || (iVar1 = this->curY, iVar1 < 0)) {
		this->curX = x;
		this->curY = y;
		return 0;
	}
	iVar2 = this->endX;
	iVar5 = this->begY - iVar2;
	if (y <= iVar5) {
		iVar1 = 0;
	}
	if (iVar5 < y) {
		iVar5 = this->begY + iVar2;
		if ((y < iVar5) && (iVar3 <= x)) {
			iVar4 = this->begX;
			if (iVar3 - iVar4 <= this->endY) goto LAB_00074b9c;
			iVar1 = 1;
			this->touched = false;
			*swDir = SwipeDir::Right;
			iVar4 = this->begY;
			iVar2 = this->endX;
			iVar3 = iVar4 - iVar2;
			if (iVar3 < y) {
				iVar5 = iVar4 + iVar2;
			}
			if (iVar3 < y) {
				iVar4 = this->begX;
			}
			if (y <= iVar3) goto LAB_00074bd0;
		}
		else {
			iVar4 = this->begX;
		LAB_00074b9c:
			iVar1 = 0;
		}
		if ((y < iVar5) && (this->endY < iVar4 - this->curX)) {
			this->touched = false;
			*swDir = SwipeDir::Left;
			iVar2 = this->endX;
			iVar1 = 1;
			goto LAB_00074bd0;
		}
	}
	else {
	LAB_00074bd0:
		iVar4 = this->begX;
	}
	if ((x <= iVar4 - iVar2) || (iVar4 = iVar4 + iVar2, iVar4 <= x)) goto LAB_00074c68;
	if (this->endY < this->curY - this->begY) {
		this->touched = false;
		*swDir = SwipeDir::Down;
		if (this->begX - this->endX < x) {
			iVar1 = 1;
			iVar4 = this->begX + this->endX;
			goto LAB_00074c34;
		}
	}
	else {
	LAB_00074c34:
		if ((iVar4 <= x) || (this->begY - this->curY <= this->endY))
			goto LAB_00074c68;
		this->touched = false;
		*swDir = SwipeDir::Up;
	}
	iVar1 = 1;
LAB_00074c68:
	this->curX = x;
	this->curY = y;
	return iVar1;
}

void fmSwipeArea::Render(Graphics* graphics) {
	if (this->enable && this->drawTouchArea)
	{
		if (this->touched) {
			graphics->FMGL_fillRect(
				this->rect.x,// + 1,
				this->rect.y,// + 50,
				this->rect.w,// - 50,
				this->rect.h,// - 50,
				0.8f,
				0.8f,
				0.8f,
				0.7f);
		}
		else {
			graphics->FMGL_fillRect(
				this->rect.x,// + 1,
				this->rect.y,// + 50,
				this->rect.w,// - 50,
				this->rect.h,// - 50,
				0.5f,
				0.5f,
				0.5f,
				0.7f);
		}
	}
}
