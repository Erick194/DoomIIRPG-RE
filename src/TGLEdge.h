#ifndef __TGLEDGE_H__
#define __TGLEDGE_H__

class TGLVert;

class TGLEdge
{
private:

    static constexpr int SCREEN_SHIFT = 3;
    static constexpr int SCREEN_ONE = 8;
    static constexpr int SCREEN_PRESTEP = 7;
    static constexpr int INTERPOLATE_SHIFT = 16;
    static constexpr int INTERPOLATE_TO_PIXELS_SHIFT = 19;
    static constexpr int INTERPOLATE_PRESTEP = 524287;

public:
    int stopY;
    int fracX;
    int fracZ;
    int fracS;
    int fracT;
    int stepX;
    int stepZ;
    int stepS;
    int stepT;

	// Constructor
	TGLEdge();
	// Destructor
	~TGLEdge();

	bool startup();
    void setFromVerts(TGLVert* tglVert, TGLVert* tglVert2);
};

#endif