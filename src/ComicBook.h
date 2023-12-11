#ifndef __COMICBOOK_H__
#define __COMICBOOK_H__

class Image;
class Graphics;

class ComicBook
{
private:

public:
	int field_0x0;
	int field_0x4;
	int curX;
	int curY;
	bool field_0x10;
	bool isLoaded;
	uint8_t field_0x12;
	uint8_t field_0x13;
	int comicBookIndex;
	int iPhoneComicIndex;
	int field_0x1c;
	Image* imgComicBook[17];
	Image* imgiPhoneComicBook[39];
	int iPhonePage;
	int cur_iPhonePage;
	int page;
	int curPage;
	bool field_0x110;
	uint8_t field_0x111;
	uint8_t field_0x112;
	uint8_t field_0x113;
	int endPoint;
	bool field_0x118;
	uint8_t field_0x119;
	uint8_t field_0x11a;
	uint8_t field_0x11b;
	int field_0x11c;
	int begPoint;
	int midPoint;
	float accelerationX;
	float accelerationY;
	float accelerationZ;
	bool is_iPhoneComic;
	bool field_0x135;
	uint8_t field_0x136;
	uint8_t field_0x137;
	int field_0x138;
	bool field_0x13c;
	uint8_t field_0x13d;
	uint8_t field_0x13e;
	uint8_t field_0x13f;
	int field_0x140;
	float field_0x144;
	bool drawExitButton;
	uint8_t field_0x149;
	uint8_t field_0x14a;
	uint8_t field_0x14b;
	int field_0x14c;
	int exitBtnRect[4];
	bool exitBtnHighlighted;
	uint8_t field_0x161;
	uint8_t field_0x162;
	uint8_t field_0x163;
	int field_0x164;
	int field_0x168;
	int field_0x16c;
	int field_0x170;
	bool field_0x174;
	uint8_t field_0x175;
	uint8_t field_0x176;
	uint8_t field_0x177;

	// Constructor
	ComicBook();
	// Destructor
	~ComicBook();

	void Draw(Graphics* graphics);
	void DrawLoading(Graphics* graphics);
	void loadImage(int index, bool vComic);
	void CheckImageExistence(Image* image);
	void DrawImage(Image* image, int a3, int a4, char a5, float alpha, char a7);

	void UpdateMovement();
	void UpdateTransition();

	void Touch(int x, int y, bool b);
	bool ButtonTouch(int x, int y);
	void TouchMove(int x, int y);
	void DeleteImages();
	void DrawExitButton(Graphics* graphics);
	void handleComicBookEvents(int key, int keyAction);
};

#endif