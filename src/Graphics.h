#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

class IDIB;
class Image;
class Text;

class Graphics
{
private:
	
public:

	static constexpr short ANCHORS_NONE = 0;
	static constexpr short ANCHORS_HCENTER = 1;
	static constexpr short ANCHORS_VCENTER = 2;
	static constexpr short ANCHORS_LEFT = 4;
	static constexpr short ANCHORS_RIGHT = 8;
	static constexpr short ANCHORS_TOP = 16;
	static constexpr short ANCHORS_BOTTOM = 32;
	static constexpr short ANCHORS_TOP_CENTER = 17;
	static constexpr short ANCHORS_CENTER = 3;
	static constexpr short ANCHORS_HORIZONTAL = 13;
	static constexpr short ANCHORS_VERTICAL = 50;

	static constexpr uint32_t charColors[12] = {
		   0xFFFFFFFF, 0xFFFF0000, 0xFF00FF00, 0xFF8BBC5D,
		   0xFF0000FF, 0xFF3180C3, 0xFFFFAFCC, 0xFFFF7F00,
		   0xFF7F7F7F, 0xFF000000, 0xFF3F3F3F, 0xFFBFBFBF };

	int curColor;
	int currentCharColor;
	IDIB* backBuffer;
	int graphClipRect[4];
	int transX;
	int transY;

	// Constructor
	Graphics();
	// Destructor
	~Graphics();

	void setGraphics();
	void setColor(int color);
	void fillCircle(int x, int y, int rad);
	void fillRect(int x, int y, int w, int h);
	void fillRect(int x, int y, int w, int h, int color);
	void FMGL_fillRect(int x, int y, int w, int h, float r, float g, float b, float a);
	void drawRect(int x, int y, int w, int h);
	void drawRect(int x, int y, int w, int h, int color);
	void eraseRgn(int x, int y, int w, int h);
	void eraseRgn(int* rect);
	void drawLine(int x1, int y1, int x2, int y2);
	void drawLine(int x1, int y1, int x2, int y2, int color);
	void drawImage(Image* img, int x, int y, int flags, int rotateMode, int renderMode);
	void drawRegion(Image* img, int texX, int texY, int texW, int texH, int posX, int posY, int flags, int rotateMode, int renderMode);
	void fillRegion(Image* img, int x, int y, int w, int h);
	void fillRegion(Image* img, int x, int y, int w, int h, int rotateMode);
	void fillRegion(Image* img, int texX, int texY, int texW, int texH, int x, int y, int w, int h, int rotateMode);
	void drawBevel(int color1, int color2, int x, int y, int w, int h);
	void drawString(Text* text, int x, int y, int flags);
	void drawString(Text* text, int x, int y, int flags, int strBeg, int strEnd);
	void drawString(Text* text, int x, int y, int h, int flags, int strBeg, int strEnd);
	void drawString(Image* img, Text* text, int x, int y, int h, int flags, int strBeg, int strEnd);
	void drawChar(Image* img, char c, int x, int y, int rotateMode);
	void drawBuffIcon(int texY, int posX, int posY, int flags);
	void drawCursor(int x, int y, int flags);
	void drawCursor(int x, int y, int flags, bool b);
	void clipRect(int x, int y, int w, int h);
	void setClipRect(int x, int y, int w, int h);
	void clearClipRect();
	void setScreenSpace(int* rect);
	void setScreenSpace(int x, int y, int w, int h);
	void resetScreenSpace();
	void fade(int* rect, int alpha, int color);
	void drawPixelPortal(int* rect, int x, int y, uint32_t color);
};

#endif