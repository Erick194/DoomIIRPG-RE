#ifndef __IDIB_H__
#define __IDIB_H__

class IDIB
{
private:

public:
	uint8_t* pBmp;
	uint32_t* pRGB888;
	uint16_t* pRGB565;
	unsigned int cntRGB;
	unsigned int nColorScheme;
	unsigned int ncTransparent;
	int width;
	int height;
	int depth;
	unsigned int pitch;

	// Constructor
	IDIB();
	// Destructor
	~IDIB();
};

#endif