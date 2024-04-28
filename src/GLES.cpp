#include <stdexcept>
#include <assert.h>

#include "SDLGL.h"
#include "CAppContainer.h"
#include "App.h"
#include "Canvas.h"
#include "GLES.h"
#include "Render.h"
#include "TinyGL.h"
#include "TGLVert.h"
#include "Enums.h"
#include "Image.h"
#include "Utils.h"

gles* _glesObj;

#define BYTES_TO_MEGABYTES(x)	(float)((float)(x) * (1.f / (1024 * 1024)))	// (1.f / (1024 * 1024)) -> 0,00000095367
#define VERT_COORDS_TO_FLOAT(x)	(float)((float)(x) * (1.f / 16384))	// (1.f / 16384) -> 0.000061035f
#define TEXT_COORDS_TO_FLOAT(x)	(float)((float)(x) * (1.f / 1024))	// (1.f / 1024)  -> 0.00097656f
#define COLOR_BYTE_TO_FLOAT(x)	(float)((float)(x) * (1.f / 256))	// (1.f / 256)   -> 0.0039062f
#define YAW_TO_FLOAT(x)			(float)((float)(x) * (1.f / 256))	// (1.f / 256)   -> 0.0039062f
#define FOG_SCALE(x)			(float)(1.f / (float)(x)) // (1.f / 8000) -> 0.000125f

gles::gles() {
	std::memset(this, 0, sizeof(gles));
}

gles::~gles() {
}

void gles::WindowInit() {
}

void gles::SwapBuffers() {
}

void gles::GLInit(Render* render) {
	Applet* app = CAppContainer::getInstance()->app;

	_glesObj = this;
	this->render = render;
	this->tinyGL = app->tinyGL;
	this->isInit = true;

	int j = 0;
	for (int i = 2; i < 16; i++) {
		this->quad_indexes[j + 0] = 0;
		this->quad_indexes[j + 1] = i-1;
		this->quad_indexes[j + 2] = i;
		j += 3;
	}

	this->fogScale = FOG_SCALE(8000);
	std::memset(this->chains, 0, sizeof(this->chains));
	this->activeChain.prev = &this->activeChain;
	this->activeChain.next = &this->activeChain;
	this->activeTexels = 0;
}

bool gles::ClearBuffer(int color) {
	if (this->isInit)
	{
		float r = COLOR_BYTE_TO_FLOAT(color >> 16 & 0xff);
		float g = COLOR_BYTE_TO_FLOAT(color >> 8 & 0xff);
		float b = COLOR_BYTE_TO_FLOAT(color & 0xff);

		glClearColor(r, g, b, 1.0f);
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		return true;
	}
	return false;
}

void gles::SetGLState() {

	PFNGLACTIVETEXTUREPROC glActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
	PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glClientActiveTexture");

	glViewport(this->vPortRect[0], this->vPortRect[1], this->vPortRect[2], this->vPortRect[3]);
	glScissor(this->vPortRect[0], this->vPortRect[1], this->vPortRect[2], this->vPortRect[3]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(this->modelViewMatrix);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(this->projectionMatrix);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_ALPHA_TEST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glClientActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE0);
	glVertexPointer(4, GL_FLOAT, sizeof(Vertex), this->immediate);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), this->immediate[0].st);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, this->fogColor);
	glFogf(GL_FOG_START, this->fogStart);
	glFogf(GL_FOG_END, this->fogEnd);
	this->fogMode = 0;
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);
	this->renderMode = -1;
	this->flags = -1;
}

void gles::BeginFrame(int x, int y, int w, int h, int* mtxView, int* mtxProjection) {
	Applet* app = CAppContainer::getInstance()->app;
	Canvas* canvas = app->canvas;
	int posX, posY;

	//printf("gles::BeginFrame\n");

	//printf("x %d | y %d | w %d | h %d \n", x, y, w, h);

	posX = (x + canvas->viewRect[0]);
	posY = (y + canvas->viewRect[1]);
	if (posY == 37) {
		posY = (36 + 1); // Hud_Test height?
	}
	else {
		posY = (64 + 1); // Panel_bottom status bar height?
	}

#if 0 //iPhone
	this->vPortRect[0] = gles::scale * posY;
	this->vPortRect[1] = gles::scale * posX;
	this->vPortRect[3] = gles::scale * w;
	this->vPortRect[2] = gles::scale * h;
#else
	this->vPortRect[0] = gles::scale * posX;
	this->vPortRect[1] = gles::scale * posY;
	this->vPortRect[2] = gles::scale * w;
	this->vPortRect[3] = gles::scale * h;

	CAppContainer::getInstance()->sdlGL->transformCoord2f((float*)&this->vPortRect[0], (float*)&this->vPortRect[1]);
	CAppContainer::getInstance()->sdlGL->transformCoord2f((float*)&this->vPortRect[2], (float*)&this->vPortRect[3]);
#endif
	
	//printf("this->vPortRect[0] %d\n", this->vPortRect[0]);
	//printf("this->vPortRect[1] %d\n", this->vPortRect[1]);
	//printf("this->vPortRect[2] %d\n", this->vPortRect[2]);
	//printf("this->vPortRect[3] %d\n", this->vPortRect[3]);

	for (int i = 0; i < 16; i++) {
		this->modelViewMatrix[i] = VERT_COORDS_TO_FLOAT(mtxView[i]);
	}

	for (int i = 0; i < 16; i++) {
		this->projectionMatrix[i] = VERT_COORDS_TO_FLOAT(mtxProjection[i]);
	}

#if 0 //iPhone
	float* projectionMatrix = this->projectionMatrix;
	for (int i = 0; i < 4; i++) {
		float temp = projectionMatrix[1];
		projectionMatrix[1] = projectionMatrix[0];
		projectionMatrix[0] = temp;
		projectionMatrix += 4;
	}

	this->projectionMatrix[0] = 0.0f - this->projectionMatrix[0];
#endif
	this->projectionMatrix[4] = 0.0f - this->projectionMatrix[4];
	this->projectionMatrix[8] = 0.0f - this->projectionMatrix[8];
	this->projectionMatrix[1] = 0.0f - this->projectionMatrix[1];
	this->projectionMatrix[5] = 0.0f - this->projectionMatrix[5];
	this->projectionMatrix[9] = 0.0f - this->projectionMatrix[9];

	if (!app->render->chatZoom) {
		this->projectionMatrix[14] *= 0.5f;
	}

	uint8_t* fogColor = (uint8_t*)&this->tinyGL->fogColor;

	this->fogColor[2] = COLOR_BYTE_TO_FLOAT(fogColor[0]);
	this->fogColor[1] = COLOR_BYTE_TO_FLOAT(fogColor[1]);
	this->fogColor[0] = COLOR_BYTE_TO_FLOAT(fogColor[2]);
	this->fogColor[3] = COLOR_BYTE_TO_FLOAT(fogColor[3]);

	if (this->fogColor[3] == 0.0f) {
		this->fogColor[3] = 1.0f;
	}

	this->fogStart = ((float)this->tinyGL->fogMin) * this->fogScale;
	this->fogEnd = ((((float)this->tinyGL->fogRange) / this->fogColor[3]) + (float)this->tinyGL->fogMin) * this->fogScale;

	if ((this->fogEnd > 0.499f) && (canvas->loadMapID == 2)) {
		this->fogStart = 9999.f;
		this->fogEnd = 10000.f;
	}

	this->fogBlack[0] = 0.0;
	this->fogBlack[1] = 0.0;
	this->fogBlack[2] = 0.0;
	this->fogBlack[3] = 0.0;
	this->SetGLState();
}

void gles::ResetGLState() {
	SDLGL* sdlGL = CAppContainer::getInstance()->sdlGL;
	int width = sdlGL->vidWidth;
	int height = sdlGL->vidHeight;

	int cx, cy;
	SDL_GetWindowSize(sdlGL->window, &cx, &cy);
	if (width != cx || height != cy) {
		width = cx; height = cy;
	}

	this->renderMode = -1;
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
	//glViewport(0, 0, 320, 480);
	//glScissor(0, 0, 320, 480);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (GLfloat)Applet::IOS_WIDTH, (GLfloat)Applet::IOS_HEIGHT, 0.0, -1.0, 1.0);
	//glOrthof(0.0, 320.0, 480.0, 0.0, -1.0, 1.0);
	//glRotatef(90.0, 0.0, 0.0, 1.0);
	//glTranslatef(0.0, -320.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_CULL_FACE);
}


void gles::CreateFadeTexture(int mediaID) {
	glChain* ct;

	#define FADE_WIDTH 16
	#define FADE_HEIGHT 64

	this->activeTexels += (FADE_WIDTH * FADE_HEIGHT);
	while (this->activeTexels > 0x800000)
	{
		ct = this->activeChain.prev;
		assert(ct != &activeChain);
		//__assert_rtn("CreateFadeTexture", "/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 366, "ct != &activeChain");

		printf("Freeing media ID %i, %ix%i\n", (int)((intptr_t)ct - (intptr_t)this->chains) / sizeof(glChain), ct->width, ct->height);
		ct->next->prev = ct->prev;
		ct->prev->next = ct->next;
		ct->prev = NULL;
		ct->next = NULL;
		glDeleteTextures(1, &ct->texnum);
		ct->texnum = 0;
		this->activeTexels -= (ct->width * ct->height);
	}

	ct = &this->chains[mediaID];
	assert(ct->texnum == 0);
	//__assert_rtn("CreateFadeTexture", "/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 367, "ct->texnum == 0");

	assert(ct->next == NULL);
	//__assert_rtn("CreateFadeTexture", "/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 368, "ct->next == NULL");

	assert(ct->prev == NULL);
	//__assert_rtn("CreateFadeTexture", "/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 369, "ct->prev == NULL");

	ct->prev = &this->activeChain;
	ct->next = this->activeChain.next;
	ct->next->prev = ct;
	ct->prev->next = ct;
	ct->width = FADE_WIDTH;
	ct->height = FADE_HEIGHT;
	printf("Allocating media ID %i, %ix%i, activeTexels = %3.1f meg\n", (int)((intptr_t)ct - (intptr_t)this->chains) / sizeof(glChain), ct->width, ct->height, BYTES_TO_MEGABYTES(this->activeTexels));

	glGenTextures(1, &ct->texnum);
	glBindTexture(GL_TEXTURE_2D, ct->texnum);
	uint8_t* pixels = (uint8_t*)std::malloc((FADE_WIDTH * FADE_HEIGHT) * sizeof(int));
	std::memset(pixels, 0x00, (FADE_WIDTH * FADE_HEIGHT) * sizeof(int));
	int alpha = 255;
	for (int i = 0; i < (FADE_WIDTH * FADE_HEIGHT); i++) {
		pixels[(i * sizeof(int)) + 3] = alpha;
		if (i == 15 * (i / 15) && i > 512) {
			alpha -= 10;
		}
		if (alpha < 0) {
			alpha = 0;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FADE_WIDTH, FADE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	std::free(pixels);
}

void gles::CreateAllActiveTextures() {
}

bool gles::RasterizeConvexPolygon(int numVerts, TGLVert* verts) {
	Applet* app = CAppContainer::getInstance()->app;
	Vertex* immediate;
	GLfloat projectionMatrix[MAX_GLVERTS];
	int fogMode;

	if (this->isInit) {
		fogMode = this->fogMode;
		this->fogMode = 0;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		std::memset(projectionMatrix, 0, sizeof(projectionMatrix));
#if 0 // Iphone
		projectionMatrix[1] = -1.0;	// {0.0, -1.0, 0.0, 0.0}
		projectionMatrix[4] = 1.0;	// {1.0, 0.0, 0.0, 0.0}
#else
		projectionMatrix[0] = 1.0;	// {1.0, 0.0, 0.0, 0.0}
		projectionMatrix[5] = 1.0;	// {0.0, 1.0, 0.0, 0.0}
#endif
		projectionMatrix[10] = 1.0;	// {0.0, 0.0, 1.0, 0.0}
		projectionMatrix[15] = 1.0;	// {0.0, 0.0, 0.0, 1.0}
		glLoadMatrixf(projectionMatrix);

		assert(numVerts <= MAX_GLVERTS);

		if (this->render->isSkyMap) {
			immediate = this->immediate;
			for (int i = 0; i < numVerts; i++) {
				immediate->xyzw[0] = (float)verts->x;
				immediate->xyzw[1] = (float)-verts->y;
				immediate->xyzw[2] = (float)verts->z;
				immediate->xyzw[3] = (float)verts->w;
				immediate->st[0] = (immediate->xyzw[0] / immediate->xyzw[3]) * 0.5f;
				immediate->st[1] = ((immediate->xyzw[1] / immediate->xyzw[3]) * -0.5f) + 0.5f;
				//------------------------------------------------------------------------------------
				immediate->xyzw[0] /= immediate->xyzw[3];
				immediate->xyzw[1] /= immediate->xyzw[3];
				immediate->xyzw[2] = 1.0f;
				immediate->xyzw[3] = 1.0f;
				immediate->st[0] += -YAW_TO_FLOAT(this->tinyGL->viewYaw);

				immediate++;
				verts++;
			}
		}
		else {
			if(app->canvas->loadMapID != 3) {
				glDisable(GL_FOG);
			}
			immediate = this->immediate;
			for (int i = 0; i < numVerts; i++) {
				immediate->xyzw[0] = (float)verts->x;
				immediate->xyzw[1] = (float)-verts->y;
				immediate->xyzw[2] = (float)verts->z;
				immediate->xyzw[3] = (float)verts->w;
				immediate->st[0] = TEXT_COORDS_TO_FLOAT(verts->s);
				immediate->st[1] = TEXT_COORDS_TO_FLOAT(verts->t);
				//------------------------------------------------------------------------------------
				immediate->xyzw[0] /= immediate->xyzw[3];
				immediate->xyzw[1] /= immediate->xyzw[3];
				immediate->xyzw[2] /= immediate->xyzw[3];
				immediate->xyzw[3] = 1.0f;

				immediate++;
				verts++;
			}
		}

		glDrawElements(GL_TRIANGLES, (numVerts * 3) - (2 * 3), GL_UNSIGNED_SHORT, this->quad_indexes);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(this->modelViewMatrix);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(this->projectionMatrix);
		this->fogMode = fogMode;
		if (this->fogMode) {
			glEnable(GL_FOG);
			glFogfv(GL_FOG_COLOR, (this->fogMode == 1) ? this->fogBlack : this->fogColor);
		}
		else {
			glDisable(GL_FOG);
		}
		return true;
	}
	return false;
}

bool gles::RasterizeConvexPolygon(int numVerts, GLVert* verts) {
	Vertex* immediate;
	GLfloat projectionMatrix[MAX_GLVERTS];

	if (this->isInit) {
		this->fogMode = 0;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		std::memset(projectionMatrix, 0, sizeof(projectionMatrix));
#if 0 // Iphone
		projectionMatrix[1] = -1.0;	// {0.0, -1.0, 0.0, 0.0}
		projectionMatrix[4] = 1.0;	// {1.0, 0.0, 0.0, 0.0}
#else
		projectionMatrix[0] = 1.0;	// {1.0, 0.0, 0.0, 0.0}
		projectionMatrix[5] = 1.0;	// {0.0, 1.0, 0.0, 0.0}
#endif
		projectionMatrix[10] = 1.0;	// {0.0, 0.0, 1.0, 0.0}
		projectionMatrix[15] = 1.0;	// {0.0, 0.0, 0.0, 1.0}
		glLoadMatrixf(projectionMatrix);

		assert(numVerts <= MAX_GLVERTS);

		if (this->render->isSkyMap) {
			immediate = this->immediate;
			for (int i = 0; i < numVerts; i++) {
				immediate->xyzw[0] = (float)verts->x;
				immediate->xyzw[1] = (float)-verts->y;
				immediate->xyzw[2] = (float)verts->z;
				immediate->xyzw[3] = (float)verts->w;
				immediate->st[0] = (immediate->xyzw[0] / immediate->xyzw[3]) * 0.5f;
				immediate->st[1] = ((immediate->xyzw[1] / immediate->xyzw[3]) * -0.5f) + 0.5f;
				//------------------------------------------------------------------------------------
				immediate->xyzw[0] /= immediate->xyzw[3];
				immediate->xyzw[1] /= immediate->xyzw[3];
				immediate->xyzw[2] = 1.0f;
				immediate->xyzw[3] = 1.0f;
				immediate->st[0] += -YAW_TO_FLOAT(this->tinyGL->viewYaw);

				immediate++;
				verts++;
			}
		}
		else {
			glDisable(GL_FOG);
			immediate = this->immediate;
			for (int i = 0; i < numVerts; i++) {
				immediate->xyzw[0] = (float)verts->x;
				immediate->xyzw[1] = (float)-verts->y;
				immediate->xyzw[2] = (float)verts->z;
				immediate->xyzw[3] = (float)verts->w;
				immediate->st[0] = TEXT_COORDS_TO_FLOAT(verts->s);
				immediate->st[1] = TEXT_COORDS_TO_FLOAT(verts->t);
				//------------------------------------------------------------------------------------
				immediate->xyzw[0] /= immediate->xyzw[3];
				immediate->xyzw[1] /= immediate->xyzw[3];
				immediate->xyzw[2] /= immediate->xyzw[3];
				immediate->xyzw[3] = 1.0f;
				
				immediate++;
				verts++;
			}
		}

		glDrawElements(GL_TRIANGLES, (numVerts * 3) - (2 * 3), GL_UNSIGNED_SHORT, this->quad_indexes);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(this->modelViewMatrix);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(this->projectionMatrix);
		return true;
	}
	return false;
}

void gles::UnloadSkyMap() {
	glChain* chain; // r4
	glChain* next; // r1
	glChain* prev; // r2

	if (this->render->mediaMappings)
	{
		int index = this->render->mediaMappings[Enums::TILENUM_SKY_BOX];
		if (this->chains[index].texnum)
		{
			chain = &this->chains[index];
			next = chain->next;
			next->prev = chain->prev;
			prev = chain->prev;
			chain->prev = 0;
			prev->next = next;
			chain->next = 0;
			glDeleteTextures(1, &chain->texnum);
			chain->texnum = 0;
		}
	}
}

bool gles::DrawWorldSpaceSpriteLine(TGLVert* vert1, TGLVert* vert2, TGLVert* vert3, int flags) {
	TGLVert verts[4];

	if (this->isInit) {
		verts[0].x = vert1->x;
		verts[0].y = vert1->y;
		verts[0].z = vert1->z;
		verts[0].w = vert1->w;
		verts[0].s = vert1->s;
		verts[0].t = vert1->t;
		verts[0].clipDist = vert1->clipDist;

		verts[1].x = vert2->x;
		verts[1].y = vert2->y;
		verts[1].z = vert2->z;
		verts[1].w = vert2->w;
		verts[1].s = vert2->s;
		verts[1].t = vert2->t;
		verts[1].clipDist = vert2->clipDist;

		verts[2].x = vert3->x;
		verts[2].y = vert3->y;
		verts[2].z = vert3->z;
		verts[2].w = vert3->w;
		verts[2].s = vert3->s;
		verts[2].t = vert3->t;
		verts[2].clipDist = vert3->clipDist;

		verts[3].x = vert3->x - (vert2->x - verts[0].x);
		verts[3].y = vert3->y - (vert2->y - verts[0].y);
		verts[3].z = vert3->z - (vert2->z - verts[0].z);

		verts[0].s = 0;
		verts[1].s = 1024;
		verts[2].s = 1024;
		verts[3].s = 0;

		if (((flags ^ 0x60000U) & 0x20000)) {
			verts[0].s = 1024;
			verts[1].s = 0;
			verts[2].s = 0;
			verts[3].s = 1024;
		}

		verts[0].t = 0;
		verts[1].t = 0;
		verts[2].t = 1024;
		verts[3].t = 1024;

		if (((flags ^ 0x60000U) & 0x40000) != 0) {
			verts[0].t = 1024;
			verts[1].t = 1024;
			verts[2].t = 0;
			verts[3].t = 0;
		}

		for (int i = 0; i < 4; i++) {
			verts[i].s = (verts[i].s * 176) / this->tinyGL->sWidth;
			verts[i].t = (verts[i].t * 176) / this->tinyGL->tHeight;
		}

		return this->DrawModelVerts(verts, 4);
	}
	return false;
}

bool gles::DrawModelVerts(TGLVert* verts, int numVerts) {
	Vertex* immediate;

	if (this->isInit) {
		if (!this->render->isSkyMap) {
			if (numVerts > MAX_GLVERTS) {
				assert(numVerts <= MAX_GLVERTS);
				//__assert_rtn("DrawModelVerts","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 0x50f,"numVerts <= MAX_GLVERTS");
			}

			immediate = this->immediate;
			for (int i = 0; i < numVerts; i++) {
				immediate->xyzw[0] = VERT_COORDS_TO_FLOAT(verts->x);
				immediate->xyzw[1] = VERT_COORDS_TO_FLOAT(verts->y);
				immediate->xyzw[2] = VERT_COORDS_TO_FLOAT(verts->z);
				immediate->xyzw[3] = 1.0f;
				immediate->st[0] = TEXT_COORDS_TO_FLOAT(verts->s);
				immediate->st[1] = TEXT_COORDS_TO_FLOAT(verts->t);
				immediate++;
				verts++;
			}

			glDrawElements(GL_TRIANGLES, (numVerts * 3) - (2 * 3), GL_UNSIGNED_SHORT, this->quad_indexes);

			return (this->isInit == true);
		}
	}

	return false;
}

void gles::SetupTexture(int n, int n2, int renderMode, int flags) {
	Applet* app = CAppContainer::getInstance()->app;
	glChain* chain;
	glChain* next;
	glChain* prev;

	int mediaID = this->render->mediaMappings[n] + n2;

	if (n == Enums::TILENUM_ELECTRIC_SLIDE) {
		mediaID = this->render->mediaMappings[Enums::TILENUM_ELECTRIC_SLIDE];
	}

	assert(mediaID >= 0 && mediaID < MAX_MEDIA);

	chain = &this->chains[mediaID];
	if (!chain->texnum) {
		this->CreateTextureForMediaID(n, mediaID, true);
	}

	next = this->chains[(int)mediaID].next;
	next->prev = chain->prev;
	chain->prev->next = next;
	next = this->activeChain.next;
	chain->prev = &this->activeChain;
	next->prev = chain;
	prev = chain->prev;
	this->chains[mediaID].next = next;
	prev->next = chain;

	if (n == Enums::TILENUM_SCORCH_MARK) {
		renderMode = Render::RENDER_SUB;
	}

	glBindTexture(GL_TEXTURE_2D, chain->texnum);
	
	if (renderMode != this->renderMode || flags != this->flags)
	{
		bool isMultiply = (flags & Render::RENDER_FLAG_MULTYPLYSHIFT); // [GEC]

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // [GEC] Default Combiner

		this->renderMode = renderMode;
		this->flags = flags;
		switch (renderMode)
		{
			case Render::RENDER_NORMAL: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				if (flags & Render::RENDER_FLAG_BRIGHTREDSHIFT) {
					glColor4f(1.0f, 0.5, 0.5, 1.0f);
				}
				else {
					glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				}
				this->fogMode = 2;
				break;
			}
			case Render::RENDER_BLEND25: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
				this->fogMode = 2;
				break;
			}
			case Render::RENDER_BLEND50: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, 0.50f);
				this->fogMode = 2;
				break;
			}
			case Render::RENDER_ADD: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE); // glBlendFunc(GL_ONE, GL_ONE);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				this->fogMode = 0;
				break;
			}
			case Render::RENDER_ADD75: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE); // glBlendFunc(GL_ONE, GL_ONE);
				glColor4f(0.75f, 0.75f, 0.75f, 1.0f);
				this->fogMode = 0;
				break;
			}
			case Render::RENDER_ADD50: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE); // glBlendFunc(GL_ONE, GL_ONE);
				glColor4f(0.50f, 0.50f, 0.50f, 1.0f);
				this->fogMode = 0;
				break;
			}
			case Render::RENDER_ADD25: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE); // glBlendFunc(GL_ONE, GL_ONE);
				glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
				this->fogMode = 0;
				break;
			}
			case Render::RENDER_SUB: {
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				this->fogMode = 0;
				break;
			}

			case Render::RENDER_PERF: { // New
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, 0.50f);
				this->fogMode = 2;
				break;
			}
			// case Render::RENDER_PERF: Old
			case Render::RENDER_NONE: {
				glBlendFunc(GL_ZERO, GL_ONE);
				this->fogMode = 0;
				break;
			}
			case Render::RENDER_BLEND75: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
				this->fogMode = 2;
				break;
			}
			case Render::RENDER_BLENDSPECIALALPHA: {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, app->canvas->blendSpecialAlpha);
				this->fogMode = 2;
				break;
			}
			default: {
				assert(0);
				break;
			}
		}

		if (this->fogMode) {
			glEnable(GL_FOG);
			glFogfv(GL_FOG_COLOR, (this->fogMode == 1) ? this->fogBlack : this->fogColor);
		}
		else {
			glDisable(GL_FOG);
		}

		int v28, v31, v32, v33;
		v28 = 0;
		if (flags & Render::RENDER_FLAG_PULSATE) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			int time = app->time >> 2;
			int rgba = time & 0x1FF;

			if (time & 0x100) {
				rgba = (508 - rgba) + 3;
			}

			if (rgba < 31) {
				rgba = 31;
			}

			glColor4ub(rgba, rgba, rgba, rgba);
		}
		else if(flags & Render::RENDER_FLAG_REDSHIFT) {
			if (isMultiply) {
				glColor4ub(0xFF, 0x00, 0x00, 0xFF); // IOS
			}
			else { // [GEC] like J2ME/BREW version
				this->TexCombineShift(64, 0, 0);
			}
		}
		else if (flags & Render::RENDER_FLAG_GREENSHIFT) {
			if (isMultiply) {
				glColor4ub(0x00, 0xFF, 0x00, 0xFF); // IOS
			}
			else { // [GEC] like J2ME/BREW version
				this->TexCombineShift(0, 64, 0);
			}
		}
		else if (flags & Render::RENDER_FLAG_BLUESHIFT) {
			if (isMultiply) {
				glColor4ub(0x00, 0x00, 0xFF, 0xFF); // IOS
			}
			else { // [GEC] like J2ME/BREW version
				this->TexCombineShift(0, 0, 64);
			}
		}
	}
}

void gles::CreateTextureForMediaID(int n, int mediaID, bool b) {
	//printf("CreateTextureForMediaID %d\n", mediaID);
	Applet* app = CAppContainer::getInstance()->app;

	Render* render; // r2
	int v5; // r1
	int v7; // r8
	char v8; // r1
	uint16_t* v9; // r4
	unsigned int v10; // r3
	unsigned int v12; // r2
	int v13; // r11
	int v14; // r10
	int v15; // lr
	uint8_t* v16; // r12
	int rgb565; // r3
	int b5; // r0
	int r5; // r1
	int g6; // r2
	Render* v23; // r1
	int v27; // r4
	int v28; // r3
	int shapeMin; // lr
	int v31; // r8
	unsigned int v34; // r2
	int i; // r6
	int x;
	int y; // r4
	int runHeight; // r0
	char* v43; // r1
	int v44; // r2
	uint8_t* v45; // r3
	uint8_t* v46; // r1
	bool v47; // r4
	int16_t* v48; // r2
	int v54; // lr
	uint8_t* v55; // r2
	int v58; // r1
	glChain* ct; // r4
	glChain* next; // r2
	glChain* prev; // r3
	char* data; // [sp+18h] [bp-470h]
	bool v70; // [sp+1Ch] [bp-46Ch]
	int __len; // [sp+2Ch] [bp-45Ch]
	uint8_t* __src; // [sp+34h] [bp-454h]
	int width; // [sp+38h] [bp-450h]
	int height; // [sp+3Ch] [bp-44Ch]
	int v78; // [sp+40h] [bp-448h]
	uint8_t* v79; // [sp+44h] [bp-444h]
	int v80; // [sp+48h] [bp-440h]
	int shapeMax; // [sp+4Ch] [bp-43Ch]
	int maxY; // [sp+50h] [bp-438h]
	int minY;
	int v83; // [sp+54h] [bp-434h]
	int v85; // [sp+5Ch] [bp-42Ch]
	char* v86; // [sp+60h] [bp-428h]
	char* __b; // [sp+64h] [bp-424h]
	uint8_t rgba[1024]; // [sp+6Bh] [bp-41Dh] BYREF
	bool transAlpha = false;

	v70 = b;
	render = this->render;
	v5 = render->mediaMappings[Enums::TILENUM_SKY_BOX];
	if (mediaID == render->mediaMappings[Enums::TILENUM_FADE] && n == Enums::TILENUM_FADE) {
		this->CreateFadeTexture(mediaID);
		return;
	}

	assert(mediaID >= 0 && mediaID < MAX_MEDIA);
	//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 471,"mediaID >= 0 && mediaID < MAX_MEDIA");

	int Size = 0;
	if (v5 == mediaID)
	{
		if (!render->skyMapTexels)
			return;
		v7 = 0;
		__src = render->skyMapTexels;
		v9 = render->skyMapPalette[0];
		v8 = 8;
		v10 = 8;
	}
	else
	{
		v7 = render->mediaTexelSizes[mediaID] & 0x3FFF;
		__src = render->mediaTexels[v7];
		Size = render->mediaTexelSizes2[v7];
		v9 = render->mediaPalettes[render->mediaPalColors[mediaID] & 0x3FFF][0];
		v12 = render->mediaDimensions[mediaID];
		v8 = v12 & 0xF;
		v10 = (v12 >> 4) & 0xF;

		if (mediaID == render->mediaMappings[Enums::TILENUM_ANIM_WATER] && n == Enums::TILENUM_ANIM_WATER) {
			printf("TILENUM_ANIM_WATER %d\n", mediaID);
		}
	}
	v13 = 0;
	v14 = 0;
	v15 = 0;
	v16 = rgba;
	width = 1 << v10;
	height = 1 << v8;
	v78 = 0;
	do
	{
		rgb565 = v9[v15];
		b5 = rgb565 & 0x1F; // b5
		g6 = (rgb565 >> 5) & 0x3F; // g6
		r5 = (rgb565 >> 11) & 0x1F; // r5
		
		if (!(v15 <= 0)) {
			v14 += r5;
			v13 += g6;
			v78 += b5;
		}
		v16[0] = (uint8_t)(r5 << 3) | (r5 >> 2); // old->((r5 >> 3) | (r5 << 3));
		v16[1] = (uint8_t)(g6 << 2) | (g6 >> 4); // old->((g6 >> 4) | (g6 << 2));
		v16[2] = (uint8_t)(b5 << 3) | (b5 >> 2); // old->((b5 >> 3) | (b5 << 3));
		v16[3] = 0xFF;

		// [GEC] verifica que realmente este el color designado para la trasparencia
		if ((v16[0] >= 250) && (v16[1] == 0) && (v16[2] >= 250)) {
			transAlpha = true;
		}
		// [GEC] Arachnotron attack 
		if ((v16[0] >= 250) && (v16[1] == 4) && (v16[2] >= 250)) {
			transAlpha = true;
		}

		v16 += 4;
		v15++;
	} while (v15 < 256);
	__len = height * width;
	data = (char*)malloc(height * width + 512);
	__b = data + 512;
	v23 = this->render;
	v79 = render->mediaTexels[v7];
	bool sprite = false;
	if ((v5 == mediaID) || (Size == __len))
	{
		v48 = v23->mediaMappings;
		if (v48[257] <= mediaID) {
			v47 = 0;
		}
		if (mediaID < v48[257]) {
			v47 = 1;
		}
		if (mediaID == v48[197]) {
			v47 = 0;
		}
		if (mediaID == v48[202]) {
			v47 = 0;
		}
		if (mediaID == v48[128]) {
			v47 = 0;
		}
		if (mediaID == v48[160]) {
			v47 = 0;
		}
		if (mediaID == v48[168]) {
			v47 = 0;
		}
		if (mediaID == v48[162]) {
			v47 = 0;
		}
		if ((unsigned int)(n - 184) <= 9) {
			v47 = 0;
		}
		if (mediaID == v48[129]) {
			v47 = 0;
		}
		v46 = __src;
	}
	else
	{
		//++numSprites;
		memset(__b, 0, __len);
		if (transAlpha) {
			rgba[0] = (uint8_t)(v14 / 255);
			rgba[1] = (uint8_t)(v13 / 255);
			rgba[2] = (uint8_t)(v78 / 255);
			rgba[3] = 0;
		}

		if (Size != 0) {
			v27 = v79[Size - 1] << 8;
		}
		else {
			app->Error("Error 1"); // IphoneDebugError("Error");
		}

		if (Size >= 2) {
			v28 = v79[Size - 2];
		}
		else {
			app->Error("Error 2"); // IphoneDebugError("Error");
		}

		v80 = Size - (v27 | v28) - 2;
		shapeMin = (uint8_t)(this->render->mediaBounds[4 * mediaID + 0]);
		shapeMax = (uint8_t)(this->render->mediaBounds[4 * mediaID + 1]);
		minY = (uint8_t)(this->render->mediaBounds[4 * mediaID + 2]);
		maxY = (uint8_t)(this->render->mediaBounds[4 * mediaID + 3]);

		v31 = v80 + ((shapeMax - shapeMin + 1) >> 1);

		assert(shapeMax <= width);
		//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 606,"shapeMax <= width");

		assert(shapeMin <= shapeMax);
		//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 607,"shapeMin <= shapeMax");

		assert(maxY <= height);
		//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 609,"maxY <= height");

		assert(minY <= maxY);
		//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 610,"minY <= maxY");

		v86 = &__b[shapeMin];
		v85 = 0;
		v83 = 0;
		while (shapeMin < shapeMax)
		{
			v34 = v80 + (v85 >> 1);
			if (v34 < Size) {
				x = (v79[v34] >> ((v85 & 1) << 2)) & 0xF;

				// [GEC]: Fix animated water sprite
				{
					render->fixTexels(v34, v85, mediaID, &x);
				}
			}
			else {
				app->Error("Error 3"); // IphoneDebugError("Error");
			}

			for (i = x; i > 0; --i)
			{
				if (v31 < Size) {
					y = v79[v31++];
				}
				else {
					app->Error("Error 4"); //IphoneDebugError("Error");
				}

				if (v31 < Size) {
					runHeight = v79[v31++];
				}
				else {
					app->Error("Error 5"); //IphoneDebugError("Error");
				}

				assert(y + runHeight <= maxY);
				//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 622,"y + runHeight <= maxY");

				v43 = &v86[y * width];
				v45 = &__src[v83];
				for(v44 = 0; v44 < runHeight; v44++)
				{
					*v43 = *v45++;
					v43 += width;
					v83++;
				}
			}
			shapeMin++;
			++v85;
			++v86;
		}
		v46 = (uint8_t*)(data + 512);
		v47 = 1;
	}

	std::memcpy(__b, v46, __len);
	if (v47)
	{
		if (transAlpha) {
			rgba[0] -= rgba[0];
			rgba[1] -= rgba[1];
			rgba[2] -= rgba[2];
			rgba[3] = 0;
		}
	}

	v54 = 0;
	v55 = rgba;
	do
	{
		if ((v55[0] >= 250) && (v55[1] == 0) && (v55[2] >= 250)) {
			v55[0] = 0;
			v55[1] = 0;
			v55[2] = 0;
			v55[3] = 0;	
		}

		v58 = (((v55[0] >> 3) << 11) |  ((v55[1] >> 3) << 6) |  ((v55[2] >> 3) << 1) | (v55[3] >> 7)); // rgba to rgb5551
		v55 += 4;
		*(unsigned short*)&data[v54] = (unsigned short) v58;
		v54 += 2;
	} while (v54 != 512);

	this->activeTexels = __len + this->activeTexels;
	while (this->activeTexels > 0x800000) {
		ct = this->activeChain.prev;

		assert(ct != &activeChain);
		//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 753,"ct != &activeChain");

		printf ("Freeing media ID %i, %ix%i\n", (int)((intptr_t)ct - (intptr_t)this->chains) / sizeof(glChain), ct->width, ct->height);

		next = ct->next;
		next->prev = ct->prev;
		prev = ct->prev;
		ct->prev = nullptr;
		prev->next = next;
		ct->next = nullptr;
		glDeleteTextures(1, &ct->texnum);
		ct->texnum = 0;
		this->activeTexels -= ct->width * ct->height;
	}

	ct = this->chains + mediaID;
	assert(ct->texnum == 0);
	//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 774,"ct->texnum == 0");

	assert(ct->next == nullptr);
	//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 775,"ct->next == NULL");

	assert(ct->prev == nullptr);
	//__assert_rtn("CreateTextureForMediaID","/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/GLES.cpp", 776,"ct->prev == NULL");

	next = this->activeChain.next;
	ct->next = next;
	ct->prev = &this->activeChain;
	next->prev = ct;
	ct->prev->next = ct;
	ct->width = width;
	ct->height = height;
	printf("Allocating media ID %i, %ix%i, activeTexels = %3.1f meg\n", (int)((intptr_t)ct - (intptr_t)this->chains) / sizeof(glChain), width, height, BYTES_TO_MEGABYTES(this->activeTexels));

	glGenTextures(1, &ct->texnum);
	glBindTexture(GL_TEXTURE_2D, ct->texnum);

	//PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)SDL_GL_GetProcAddress("glCompressedTexImage2D");

	//glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_PALETTE8_RGB5_A1_OES, width, height, 0, height * width + 512, data);

	uint16_t* texData = (uint16_t*)std::malloc(width * height * 2);
	uint16_t* texPal = (uint16_t*)data;
	uint8_t* texData8 = (uint8_t*)data + 512;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			texData[(i * width) + j] = texPal[texData8[(i * width) + j]];
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, texData);
	std::free(texData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, v70 ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, v70 ? GL_LINEAR : GL_NEAREST);
	std::free(data);
}

bool gles::DrawSkyMap() {
	int fogMode; // r6
	GLVert v5[4]; // [sp+4h] [bp-D0h] BYREF

	if (this->isInit) {
		this->SetupTexture(Enums::TILENUM_SKY_BOX, 0, 0, 0);
		v5[0].x = -100;
		v5[0].y = -100;
		v5[0].w = 100;
		v5[1].x = 100;
		v5[1].y = -100;
		v5[1].w = 100;
		v5[2].x = 100;
		v5[2].y = 100;
		v5[2].w = 100;
		v5[3].x = -100;
		v5[3].y = 100;
		v5[3].w = 100;
		fogMode = this->fogMode;
		this->fogMode = 0;
		glDisable(GL_FOG);
		this->render->isSkyMap = true;
		this->RasterizeConvexPolygon(4, v5);
		this->render->isSkyMap = false;
		this->fogMode = fogMode;
		if (this->fogMode) {
			glEnable(GL_FOG);
			glFogfv(GL_FOG_COLOR, (this->fogMode == 1) ? this->fogBlack : this->fogColor);
		}
		else {
			glDisable(GL_FOG);
		}
		return this->isInit == 1;
	}
	return false;
}

void gles::DrawPortalTexture(Image* img, int x, int y, int w, int h, float tx, float ty, float scale, float angle, char mode) {
	uint8_t* pBmp;
	uint16_t* data;
	float vp[12];
	float st[8];
	int texWidth, texHeight;

	PFNGLACTIVETEXTUREPROC glActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");

	if (!img->piDIB)
		return;

	if (img->texture == -1) {

		pBmp = img->piDIB->pBmp;
		img->texWidth = 1;
		img->texHeight = 1;

		while (texWidth = img->texWidth, texWidth < img->piDIB->width) {
			img->texWidth = texWidth << 1;
		}

		while (texHeight = img->texHeight, texHeight < img->piDIB->height) {
			img->texHeight = texHeight << 1;
		}

		data = (uint16_t*)malloc(sizeof(uint16_t) * texWidth * texHeight);
		img->isTransparentMask = false;
		for (int i = 0; i < img->piDIB->height; i++) {
			for (int j = 0; j < img->piDIB->width; j++) {
				int rgb = img->piDIB->pRGB565[pBmp[(img->piDIB->width * i) + j]];
				if (rgb == 0xF81F) {
					img->isTransparentMask = true;
				}
				if (data != nullptr) {
					data[(img->texWidth * i) + j] = rgb;
				}
			}
		}

		if (data) {
			img->CreateTexture(data, img->texWidth, img->texHeight);
			std::free(data);
		}
	}

	float scaleW = (float)((float)w * 0.5f) * scale;
	float scaleH = (float)((float)h * 0.5f) * scale;

	vp[2] = 0.5f;
	vp[5] = 0.5f;
	vp[8] = 0.5f;
	vp[11] = 0.5f;
	vp[0] = scaleW;
	vp[6] = scaleW;
	vp[3] = -scaleW;
	vp[9] = -scaleW;
	vp[7] = scaleH;
	vp[10] = scaleH;
	vp[1] = -scaleH;
	vp[4] = -scaleH;

	std::memset(st, 0, sizeof(st));
	float scaleTexW = 1.0f / (float)img->texWidth;
	float scaleTexH = 1.0f / (float)img->texHeight;
	if (mode) {
		st[2] = st[0] = scaleTexW * (float)x;
		st[5] = st[1] = scaleTexH * (float)(h + y);
		st[7] = st[3] = scaleTexH * (float)y;
		st[6] = st[4] = scaleTexW * (float)(w + x);
	}
	else {
		st[4] = st[0] = scaleTexW * (float)(w + x);
		st[3] = st[1] = scaleTexH * (float)y;
		st[6] = st[2] = scaleTexW * (float)x;
		st[7] = st[5] = scaleTexH * (float)(h + y);
	}

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, img->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glVertexPointer(3, GL_FLOAT, 0, vp);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(scaleW + tx, scaleH + ty, 0.0);
	glRotatef(angle, 0.0, 0.0, 1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();
}


void gles::TexCombineShift(int r, int g, int b) { // [GEC]
	float color[4] = { 0 };
	color[0] = ((float)r / 255.0f);
	color[1] = ((float)g / 255.0f);
	color[2] = ((float)b / 255.0f);

	// glTexCombColorf
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	// glTexCombReplaceAlpha
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
}