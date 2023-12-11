#ifndef __BUTTON_H__
#define __BUTTON_H__

class Image;
class Graphics;

// ------------------------
// GuiRect Class
// ------------------------

class GuiRect
{
public:
	int x;
	int y;
	int w;
	int h;
	void Set(int x, int y, int w, int h);
	bool ContainsPoint(int x, int y);
};

// ---------------
// fmButton Class
// ---------------

class fmButton
{
private:

public:
	fmButton* next;
	int buttonID;
	int selectedIndex;
	bool drawButton;
	bool highlighted;
	int normalRenderMode;
	int highlightRenderMode;
	bool drawTouchArea;
	Image* imgNormal;
	Image* imgHighlight;
	Image** ptrNormalImages;
	Image** ptrHighlightImages;
	int normalIndex;
	int highlightIndex;
	int centerX;
	int centerY;
	int highlightCenterX;
	int highlightCenterY;
	float normalRed;
	float normalGreen;
	float normalBlue;
	float normalAlpha;
	float highlightRed;
	float highlightGreen;
	float highlightBlue;
	float highlightAlpha;
	short soundResID;
	GuiRect touchArea;
	GuiRect touchAreaDrawing; // Port: New

	// Constructor
	fmButton(int buttonID, int x, int y, int w, int h, int soundResID);
	// Destructor
	~fmButton();

	void SetImage(Image* image, bool center);
	void SetImage(Image** ptrImages, int imgIndex, bool center);
	void SetHighlightImage(Image* imgHighlight, bool center);
	void SetHighlightImage(Image** ptrImgsHighlight, int imgHighlightIndex, bool center);
	void SetGraphic(int index);
	void SetTouchArea(int x, int y, int w, int h);
	void SetTouchArea(int x, int y, int w, int h, bool drawing); // Port: new
	void SetHighlighted(bool highlighted);
	void Render(Graphics* graphics);
};

// ------------------------
// fmButtonContainer Class
// ------------------------

class fmButtonContainer
{
private:

public:
	fmButton* next;
	fmButton* prev;

	// Constructor
	fmButtonContainer();
	// Destructor
	~fmButtonContainer();

	void AddButton(fmButton* button);
	fmButton* GetButton(int buttonID);
	fmButton* GetTouchedButton(int x, int y);
	int GetTouchedButtonID(int x, int y);
	int GetHighlightedButtonID();
	void HighlightButton(int x, int y, bool highlighted);
	void SetGraphic(int index);
	void FlipButtons();
	void Render(Graphics* graphics);
};

// ---------------------
// fmScrollButton Class
// ---------------------

class fmScrollButton
{
private:

public:
	uint8_t field_0x0_;
	Image* imgBar;
	Image* imgBarTop;
	Image* imgBarMiddle;
	Image* imgBarBottom;
	uint8_t field_0x14_;
	bool field_0x15_;
	GuiRect barRect;
	GuiRect boxRect;
	uint8_t field_0x38_;
	int field_0x3c_;
	int field_0x40_;
	int field_0x44_;
	int field_0x48_;
	int field_0x4c_;
	float field_0x50_;
	int field_0x54_;
	int field_0x58_;
	int field_0x5c_;
	short soundResID;

	// Constructor
	fmScrollButton(int x, int y, int w, int h, bool b, int soundResID);
	// Destructor
	~fmScrollButton();

	//bool startup();
	void SetScrollBarImages(Image* imgBar, Image* imgBarTop, Image* imgBarMiddle, Image* imgBarBottom);
	void SetScrollBox(int x, int y, int w, int h, int i);
	void SetScrollBox(int x, int y, int w, int h, int i, int i2);
	void SetContentTouchOffset(int x, int y);
	void UpdateContent(int x, int y);
	void SetTouchOffset(int x, int y);
	void Update(int x, int y);
	void Render(Graphics* graphics);
};

// ------------------
// fmSwipeArea Class
// ------------------

class fmSwipeArea
{
private:

public:
	//typedef int SwipeDir;
	enum SwipeDir {Null = -1, Left, Right, Down, Up};

	bool enable;
	bool touched;
	bool drawTouchArea;
	GuiRect rect;
	int begX;
	int begY;
	int curX;
	int curY;
	int field_0x22_;
	int endX;
	int endY;

	// Constructor
	fmSwipeArea(int x, int y, int w, int h, int endX, int endY);
	// Destructor
	~fmSwipeArea();

	int UpdateSwipe(int x, int y, SwipeDir* swDir);
	void Render(Graphics* graphics);
};

#endif