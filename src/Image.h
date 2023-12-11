#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <SDL_opengl.h>

class IDIB;

class Image
{
private:

public:
	IDIB* piDIB;
	int width;
	int height;
	int depth;
	bool isTransparentMask;
	int texWidth;
	int texHeight;
	GLuint texture;

	// Constructor
	Image();
	// Destructor
	~Image();

	void CreateTexture(uint16_t* data, uint32_t width, uint32_t height);
	void DrawTexture(int texX, int texY, int texW, int texH, int posX, int posY, int rotateMode, int renderMode);
	void setRenderMode(int renderMode);
};

#endif