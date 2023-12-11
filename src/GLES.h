#ifndef __GLES_H__
#define __GLES_H__

#include "SDLGL.h"

typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);

class Render;
class TinyGL;
class TGLVert;

typedef struct _glChain
{
	_glChain* next;
	_glChain* prev;
	GLuint texnum;
	GLuint width;
	GLuint height;
}glChain;

typedef struct _Vertex
{
	float xyzw[4];
	float st[2];
}Vertex;

typedef struct _GLVert
{
	int x;
	int y;
	int z;
	int w;
	int s;
	int t;
}GLVert;

class gles
{
public:	bool isInit;
	static constexpr int scale = 1;
	static constexpr int MAX_GLVERTS = 16;
	static constexpr int MAX_MEDIA = 1024;

private:
	Render* render;
	TinyGL* tinyGL;
	int activeTexels;
	Vertex immediate[MAX_GLVERTS];
	uint16_t quad_indexes[42];
	glChain chains[MAX_MEDIA];
	glChain activeChain;
	float fogScale;
	float modelViewMatrix[MAX_GLVERTS];
	float projectionMatrix[MAX_GLVERTS];
	int vPortRect[4];
	int fogMode;
	int renderMode;
	int flags;
	float fogStart;
	float fogEnd;
	float fogColor[4];
	float fogBlack[4];
public:

	// Constructor
	gles();
	// Destructor
	~gles();

	void WindowInit();
	void SwapBuffers();
	void GLInit(Render* render);
	bool ClearBuffer(int color);
	void SetGLState();
	void BeginFrame(int x, int y, int w, int h, int* mtxView, int* mtxProjection);
	void ResetGLState();
	void CreateFadeTexture(int mediaID);
	void CreateAllActiveTextures();
	bool RasterizeConvexPolygon(int numVerts, TGLVert* verts);
	bool RasterizeConvexPolygon(int numVerts, GLVert* verts);
	void UnloadSkyMap();
	bool DrawWorldSpaceSpriteLine(TGLVert* vert1, TGLVert* vert2, TGLVert* vert3, int flags);
	bool DrawModelVerts(TGLVert* verts, int numVerts);
	void SetupTexture(int n, int n2, int renderMode, int flags);
	void CreateTextureForMediaID(int n, int mediaID, bool b);
	bool DrawSkyMap();
	void DrawPortalTexture(Image* img, int x, int y, int w, int h, float tx, float ty, float scale, float angle, char mode);
	void TexCombineShift(int r, int g, int b); // [GEC]
};

extern gles* _glesObj;

#endif