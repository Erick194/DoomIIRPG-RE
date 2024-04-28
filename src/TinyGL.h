#ifndef __TINYGL_H__
#define __TINYGL_H__

#include <climits>

#include "Span.h"
#include "TGLVert.h"
#include "TGLEdge.h"

class TGLVert;
class TGLEdge;

class TinyGL
{
private:

public:

    static constexpr int SHIFT_STRETCH = 12;
    static constexpr int UNIT_SCALE = 65536;
    static constexpr int MATRIX_ONE_SHIFT = 14;
    static constexpr int MATRIX_ONE = 16384;
    static constexpr int SCREEN_SHIFT = 3;
    static constexpr int SCREEN_ONE = 8;
    static constexpr int INTERPOLATE_SHIFT = 16;
    static constexpr int INTERPOLATE_TO_PIXELS_SHIFT = 19;
    static constexpr int SCREEN_PRESTEP = 7;
    static constexpr int INTERPOLATE_PRESTEP = 524287;
    static constexpr bool CLAMP_TO_VIEWPORT = false;
    static constexpr int PIXEL_GUARD_SIZE = 1;
    static constexpr int MAX_PRIMITIVE_VERTS = 20;
    static constexpr int OE_SHIFT = 4;
    static constexpr int CULL_NONE = 0;
    static constexpr int CULL_CW = 1;
    static constexpr int CULL_CCW = 2;
    static constexpr int NEAR_CLIP = 256;
    static constexpr int CULL_EXTRA = NEAR_CLIP + 16;
    static constexpr int NUM_FOG_LEVELS = 16;
    static constexpr int COLUMN_SCALE_INIT = INT_MAX;
    static constexpr int COLUMN_SCALE_OCCLUDED = (INT_MAX - 1);

    int faceCull;
    SpanType* span;
    uint8_t* textureBase;
    int imageBounds[4];
    uint16_t* spanPalette;
    uint16_t** paletteBase;
    uint16_t* scratchPalette;
    int sWidth;
    int sShift;
    int sMask;
    int tHeight;
    int tShift;
    int tMask;
    bool swapXY;
    int screenWidth;
    int screenHeight;
    uint16_t* pixels;
    int* columnScale;
    int view[16];
    int view2D[16];
    int projection[16];
    int mvp[16];
    int mvp2D[16];
    TGLVert cv[32];
    TGLVert mv[20];
    TGLEdge edges[2];
    int fogMin;
    int fogRange;
    int fogColor;
    int countBackFace;
    int countDrawn;
    int spanPixels;
    int spanCalls;
    int zeroDT;
    int zeroDS;
    int viewportX;
    int viewportY;
    int viewportWidth;
    int viewportHeight;
    int viewportClampX1;
    int viewportClampY1;
    int viewportClampX2;
    int viewportClampY2;
    int viewportX2;
    int viewportY2;
    int viewportXScale;
    int viewportXBias;
    int viewportYScale;
    int viewportYBias;
    int viewportZScale;
    int viewportZBias;
    int viewX;
    int viewY;
    int viewZ;
    int viewYaw;
    int viewPitch;
    int c_backFacedPolys;
    int c_frontFacedPolys;
    int c_totalQuad;
    int c_clippedQuad;
    int c_unclippedQuad;
    int c_rejectedQuad;
    int unk03;
    int unk04;
    uint32_t textureBaseSize; // [GEC] new
    uint32_t paletteBaseSize; // [GEC] new
    int16_t paletteTransparentMask;// [GEC] new
    uint32_t mediaID;// [GEC] new
    int colorBuffer;// [GEC] new

	// Constructor
	TinyGL();
	// Destructor
	~TinyGL();

	bool startup(int screenWidth, int screenHeight);
    uint16_t* getFogPalette(int i);
    void clearColorBuffer(int color);
    void buildViewMatrix(int x, int y, int z, int yaw, int pitch, int roll, int* matrix);
    void buildProjectionMatrix(int fov, int aspect, int* matrix);
    void multMatrix(int* matrix1, int* matrix2, int* destMtx);
    void _setViewport(int viewportX, int viewportY, int viewportWidth, int viewportHeight);
    void setViewport(int x, int y, int w, int h);
    void resetViewPort();
    void setView(int viewX, int viewY, int viewZ, int viewYaw, int viewPitch, int viewRoll, int viewFov, int viewAspect);
    void viewMtxMove(TGLVert* tglVert, int n, int n2, int n3);
    void drawModelVerts(TGLVert* array, int n);
    TGLVert* transform3DVerts(TGLVert* array, int n);
    TGLVert* transform2DVerts(TGLVert* array, int n);
    void ClipQuad(TGLVert* tglVert, TGLVert* tglVert2, TGLVert* tglVert3, TGLVert* tglVert4);
    void ClipPolygon(int i, int n);
    bool clipLine(TGLVert* array);
    void projectVerts(TGLVert* array, int n);
    void RasterizeConvexPolygon(int n);
    bool clippedLineVisCheck(TGLVert* tglVert, TGLVert* tglVert2, bool b);
    bool occludeClippedLine(TGLVert* tglVert, TGLVert* tglVert2);
    void drawClippedSpriteLine(TGLVert* tglVert, TGLVert* tglVert2, TGLVert* tglVert3, int n, bool b);
    void resetCounters();  
    void applyClearColorBuffer(); // [GEC]
};

#endif