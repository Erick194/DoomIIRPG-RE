#include <stdexcept>

#include "SDLGL.h"
#include "CAppContainer.h"
#include "App.h"
#include "Canvas.h"
#include "Graphics.h"
#include "GLES.h"
#include "Render.h"
#include "TinyGL.h"
#include "Span.h"


TinyGL::TinyGL() {
	std::memset(this, 0, sizeof(TinyGL));
}

TinyGL::~TinyGL() {
}

bool TinyGL::startup(int screenWidth, int screenHeight) {
	Applet* app = CAppContainer::getInstance()->app;
	Canvas* canvas = app->canvas;

	printf("TinyGL::startup, w [%d], h [%d]\n", screenWidth, screenHeight);

	this->scratchPalette = new uint16_t[256];
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	this->columnScale = new int[screenWidth];

	this->setViewport(canvas->viewRect[0], canvas->viewRect[1], canvas->viewRect[2], canvas->viewRect[3]);
	this->setView( 0, 0, 0, 0, 0, 0, 290, 290);
	this->fogMin = 32752;
	this->fogRange = 1;
	this->fogColor = 0;
	this->countBackFace = 0;
	this->countDrawn = 0;
	this->spanPixels = 0;
	this->spanCalls = 0;
	this->zeroDT = 0;
	this->zeroDS = 0;
	this->colorBuffer = 0; // [GEC]
	return true;
}

uint16_t* TinyGL::getFogPalette(int i) {
	i = (((0x7FFFFF / (i >> 16)) - this->fogMin) << 4) / this->fogRange;
	i = (i & ~i >> 31) - 15;
	i = (i & i >> 31) + 15;
	return this->paletteBase[i];
}

void TinyGL::clearColorBuffer(int color) {
	Applet* app = CAppContainer::getInstance()->app;
	Canvas* canvas = app->canvas;

	if (!app->render->_gles->ClearBuffer(color)) {
		/*canvas->graphics.fillRect(
			canvas->viewRect[0] + this->viewportX,
			canvas->viewRect[1] + this->viewportY, 
			this->viewportWidth,this->viewportHeight, color);*/

		this->colorBuffer = color; // [GEC]
	}
}

void TinyGL::buildViewMatrix(int x, int y, int z, int yaw, int pitch, int roll, int* matrix) {
	Applet* app = CAppContainer::getInstance()->app;
	int *sinTable = app->render->sinTable;

	int iVar7 = sinTable[yaw + 512 & 0x3ff] >> 2;
	int iVar6 = sinTable[yaw + 256 & 0x3ff] >> 2;
	int iVar1 = sinTable[pitch + 512 & 0x3ff] >> 2;
	int iVar2 = sinTable[pitch + 256 & 0x3ff] >> 2;
	int iVar3 = sinTable[roll & 0x3ff] >> 2;
	int iVar5 = sinTable[roll + 256 & 0x3ff] >> 2;
	int n13 = iVar3 * iVar1 >> 14;
	int n14 = iVar5 * iVar1 >> 14;
	matrix[0] = n13 * iVar6 + iVar5 * -iVar7 >> 14;
	matrix[4] = n13 * iVar7 + iVar5 * iVar6 >> 14;
	matrix[8] = iVar3 * iVar2 >> 14;
	matrix[1] = -(n14 * iVar6 + -iVar3 * -iVar7) >> 14;
	matrix[5] = -(n14 * iVar7 + -iVar3 * iVar6) >> 14;
	matrix[9] = -(iVar5 * iVar2) >> 14;
	matrix[2] = -(iVar2 * iVar6) >> 14;
	matrix[6] = -(iVar2 * iVar7) >> 14;
	matrix[10] = -(-iVar1);

	for (int i = 0; i < 3; ++i) {
		matrix[12 + i] = -(x * matrix[0 + i] + y * matrix[4 + i] + z * matrix[8 + i]) >> 14;
	}

	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 16384;
}

void TinyGL::buildProjectionMatrix(int fov, int aspect, int* matrix) {
	Applet* app = CAppContainer::getInstance()->app;
	int* sinTable = app->render->sinTable;

	int n3 = aspect >> 1;
	int n4 = (fov << 14) / aspect;
	int n5 = sinTable[n3 & 0x3FF] >> 2;
	int n6 = sinTable[n3 + 256 & 0x3FF] >> 2;
	matrix[0] = (n6 << 14) / (n4 * n5 >> 14);
	matrix[8] = (matrix[4] = 0);
	matrix[1] = (matrix[12] = 0);
	matrix[5] = (n6 << 14) / n5;
	matrix[13] = (matrix[9] = 0);
	matrix[6] = (matrix[2] = 0);
	matrix[10] = -TinyGL::MATRIX_ONE;
	matrix[14] = -(2 * TinyGL::NEAR_CLIP);
	matrix[7] = (matrix[3] = 0);
	matrix[11] = -TinyGL::MATRIX_ONE;
	matrix[15] = 0;
}

void TinyGL::multMatrix(int* matrix1, int* matrix2, int* destMtx) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			destMtx[i * 4 + j] = (matrix1[i * 4 + 0] * matrix2[0 + j] + matrix1[i * 4 + 1] * matrix2[4 + j] + matrix1[i * 4 + 2] * matrix2[8 + j] + matrix1[i * 4 + 3] * matrix2[12 + j]) >> 14;
		}
	}
}

void TinyGL::_setViewport(int viewportX, int viewportY, int viewportWidth, int viewportHeight)
{
	this->viewportX = viewportX;
	this->viewportY = viewportY;
	this->viewportWidth = viewportWidth;
	this->viewportHeight = viewportHeight;
	this->viewportX2 = viewportX + viewportWidth;
	this->viewportY2 = viewportY + viewportHeight;
	this->viewportClampX1 = viewportX << TinyGL::SCREEN_SHIFT;
	this->viewportClampY1 = viewportY << TinyGL::SCREEN_SHIFT;
	this->viewportClampX2 = (this->viewportX2 << TinyGL::SCREEN_SHIFT) + TinyGL::SCREEN_ONE - 1;
	this->viewportClampY2 = (this->viewportY2 << TinyGL::SCREEN_SHIFT) + TinyGL::SCREEN_ONE - 1;
	this->viewportXScale = viewportWidth << 2;
	this->viewportYScale = viewportHeight << 2;
	this->viewportXBias = ((viewportX + viewportWidth / 2) << 3) - 4;
	this->viewportYBias = ((viewportY + viewportHeight / 2) << 3) - 4;
	this->viewportZScale = (TinyGL::UNIT_SCALE / 2);
	this->viewportZBias = (TinyGL::UNIT_SCALE / 2);
}

void TinyGL::setViewport(int x, int y, int w, int h) {
	Applet* app = CAppContainer::getInstance()->app;

	int posX = x - app->canvas->viewRect[0];
	int posY = y - app->canvas->viewRect[1];

	if (!app->render->_gles->isInit) { // [GEC] Adjusted like this to match the Y position on the GL version
		posY = 3;  // posY = app->canvas->viewRect[1];
	}

	if (this->viewportX == posX && this->viewportY == posY && this->viewportWidth == w && this->viewportHeight == h) {
		return;
	}

	uint16_t* backBuff = (uint16_t*)app->backBuffer->pBmp;
	this->pixels = backBuff + (this->screenWidth * 3) + app->canvas->viewRect[0];

	this->_setViewport(posX + 1, posY + 1, w - 2, h - 2);

	if (app->render->_gles->isInit) { // [GEC] <- adding new line code
		app->canvas->repaintFlags &= ~Canvas::REPAINT_VIEW3D;
	}
}

void TinyGL::resetViewPort() {
	Applet* app = CAppContainer::getInstance()->app;
	this->setViewport(app->canvas->viewRect[0], app->canvas->viewRect[1], app->canvas->viewRect[2], app->canvas->viewRect[3]);
}

void TinyGL::setView(int viewX, int viewY, int viewZ, int viewYaw, int viewPitch, int viewRoll, int viewFov, int viewAspect) {
	
	Applet* app = CAppContainer::getInstance()->app;

	this->viewX = viewX;
	this->viewY = viewY;
	this->viewZ = viewZ;
	this->viewPitch = viewPitch & 0x3ff;
	this->viewYaw = viewYaw & 0x3ff;

	this->buildViewMatrix(viewX, viewY, viewZ, this->viewYaw, this->viewPitch, viewRoll, this->view);
	this->buildViewMatrix(viewX, viewY, 0, this->viewYaw, 0, 0, this->view2D);
	this->buildProjectionMatrix(viewFov, viewAspect, this->projection);
	this->multMatrix(this->view, this->projection, this->mvp);

	app->render->_gles->BeginFrame(this->viewportX, this->viewportY, this->viewportWidth, this->viewportHeight, this->view, this->projection);

	if (this->viewPitch > 512) {
		this->viewPitch -= 1024;
	}

	this->buildProjectionMatrix(viewFov + std::abs(this->viewPitch), viewAspect, this->projection);
	this->multMatrix(this->view2D, this->projection, this->mvp2D);
}

void TinyGL::viewMtxMove(TGLVert* tglVert, int n, int n2, int n3) {
	if (n != 0) {
		n = -n;
		tglVert->x += this->view[2] * n >> 14;
		tglVert->y += this->view[6] * n >> 14;
		tglVert->z += this->view[10] * n >> 14;
	}
	if (n2 != 0) {
		tglVert->x += this->view[0] * n2 >> 14;
		tglVert->y += this->view[4] * n2 >> 14;
		tglVert->z += this->view[8] * n2 >> 14;
	}
	if (n3 != 0) {
		n3 = -n3;
		tglVert->x += this->view[1] * n3 >> 14;
		tglVert->y += this->view[5] * n3 >> 14;
		tglVert->z += this->view[9] * n3 >> 14;
	}
}

void TinyGL::drawModelVerts(TGLVert* array, int n) {
	Applet* app = CAppContainer::getInstance()->app;

	if ((app->render->renderMode & 0x1) == 0x0) {
		return;
	}

	if (!app->render->_gles->DrawModelVerts(array, n)) {
		if (this->faceCull != TinyGL::CULL_NONE) {
			TGLVert* tglVert = &array[0];
			TGLVert* tglVert2 = &array[1];
			TGLVert* tglVert3 = &array[2];
			int n2 = tglVert2->x - tglVert->x;
			int n3 = tglVert2->y - tglVert->y;
			int n4 = tglVert2->z - tglVert->z;
			int n5 = tglVert3->x - tglVert->x;
			int n6 = tglVert3->y - tglVert->y;
			int n7 = tglVert3->z - tglVert->z;
			int n8 = (this->viewX - tglVert->x) * (n3 * n7 - n4 * n6 >> 14) + (this->viewY - tglVert->y) * (n4 * n5 - n2 * n7 >> 14) + (this->viewZ - tglVert->z) * (n2 * n6 - n3 * n5 >> 14);
			if ((this->faceCull == TinyGL::CULL_CCW && n8 < 0) || (this->faceCull == TinyGL::CULL_CW && n8 > 0)) {
				++this->c_backFacedPolys;
				return;
			}
		}
		++this->c_frontFacedPolys;
		TGLVert* transform3DVerts = this->transform3DVerts(array, n);
		if ((app->render->renderMode & 0x2) == 0x0) {
			return;
		}
		if (n == 4) {
			this->ClipQuad(&transform3DVerts[0], &transform3DVerts[1], &transform3DVerts[2], &transform3DVerts[3]);
		}
		else {
			this->ClipPolygon(0, n);
		}
	}
}

TGLVert* TinyGL::transform3DVerts(TGLVert* array, int n) {
	int* mvp = this->mvp;
	for (int i = 0; i < n; ++i) {
		TGLVert* tglVert = &array[i];
		TGLVert* tglVert2 = &this->cv[i];
		int x = tglVert->x;
		int y = tglVert->y;
		int z = tglVert->z;
		tglVert2->x = (x * mvp[0] + y * mvp[4] + z * mvp[8] >> 14) + mvp[12];
		tglVert2->y = (x * mvp[1] + y * mvp[5] + z * mvp[9] >> 14) + mvp[13];
		tglVert2->z = (x * mvp[2] + y * mvp[6] + z * mvp[10] >> 14) + mvp[14];
		tglVert2->w = (x * mvp[3] + y * mvp[7] + z * mvp[11] >> 14) + mvp[15];
		tglVert2->s = tglVert->s;
		tglVert2->t = tglVert->t;
	}
	return this->cv;
}

TGLVert* TinyGL::transform2DVerts(TGLVert* array, int n) {
	int* mvp2D = this->mvp2D;
	for (int i = 0; i < n; ++i) {
		TGLVert* tglVert = &array[i];
		TGLVert* tglVert2 = &this->cv[i];
		int x = tglVert->x;
		int y = tglVert->y;
		tglVert2->x = (x * mvp2D[0] + y * mvp2D[4] >> 14) + mvp2D[12];
		tglVert2->z = (x * mvp2D[2] + y * mvp2D[6] >> 14) + mvp2D[14];
		tglVert2->w = (x * mvp2D[3] + y * mvp2D[7] >> 14) + mvp2D[15];
		tglVert2->s = tglVert->s;
		tglVert2->t = tglVert->t;
	}
	return this->cv;
}

void TinyGL::ClipQuad(TGLVert* tglVert, TGLVert* tglVert2, TGLVert* tglVert3, TGLVert* tglVert4) {
	++this->c_totalQuad;
	int i = 0;
	while (i < 5) {
		int n = 0;
		int n2 = 0;
		int n3 = 0;
		int n4 = 0;
		switch (i) {
		default: {
			n = tglVert->w + tglVert->x;
			n2 = tglVert2->w + tglVert2->x;
			n3 = tglVert3->w + tglVert3->x;
			n4 = tglVert4->w + tglVert4->x;
			break;
		}
		case 1: {
			n = tglVert->w - tglVert->x;
			n2 = tglVert2->w - tglVert2->x;
			n3 = tglVert3->w - tglVert3->x;
			n4 = tglVert4->w - tglVert4->x;
			break;
		}
		case 2: {
			n = tglVert->w + tglVert->y;
			n2 = tglVert2->w + tglVert2->y;
			n3 = tglVert3->w + tglVert3->y;
			n4 = tglVert4->w + tglVert4->y;
			break;
		}
		case 3: {
			n = tglVert->w - tglVert->y;
			n2 = tglVert2->w - tglVert2->y;
			n3 = tglVert3->w - tglVert3->y;
			n4 = tglVert4->w - tglVert4->y;
			break;
		}
		case 4: {
			n = tglVert->w + tglVert->z;
			n2 = tglVert2->w + tglVert2->z;
			n3 = tglVert3->w + tglVert3->z;
			n4 = tglVert4->w + tglVert4->z;
			break;
		}
		case 5: {
			n = tglVert->w - tglVert->z;
			n2 = tglVert2->w - tglVert2->z;
			n3 = tglVert3->w - tglVert3->z;
			n4 = tglVert4->w - tglVert4->z;
			break;
		}
		}
		int n5 = ((n < 0) ? 1 : 0) | ((n2 < 0) ? 2 : 0) | ((n3 < 0) ? 4 : 0) | ((n4 < 0) ? 8 : 0);
		if (n5 == 0) {
			++i;
		}
		else {
			if (n5 == 15) {
				++this->c_rejectedQuad;
				return;
			}
			++this->c_clippedQuad;
			this->ClipPolygon(i, 4);
			return;
		}
	}
	++this->c_unclippedQuad;
	this->RasterizeConvexPolygon(4);
}

void TinyGL::ClipPolygon(int i, int n) {
	while (i < 5) {
		switch (i) {
			case 0:
			default: {
				for (int j = 0; j < n; ++j) {
					this->cv[j].clipDist = this->cv[j].w + this->cv[j].x;
				}
				break;
			}
			case 1: {
				for (int k = 0; k < n; ++k) {
					this->cv[k].clipDist = this->cv[k].w - this->cv[k].x;
				}
				break;
			}
			case 2: {
				for (int l = 0; l < n; ++l) {
					this->cv[l].clipDist = this->cv[l].w + this->cv[l].y;
				}
				break;
			}
			case 3: {
				for (int n2 = 0; n2 < n; ++n2) {
					this->cv[n2].clipDist = this->cv[n2].w - this->cv[n2].y;
				}
				break;
			}
			case 4: {
				for (int n3 = 0; n3 < n; ++n3) {
					this->cv[n3].clipDist = this->cv[n3].w + this->cv[n3].z;
				}
				break;
			}
			case 5: {
				for (int n4 = 0; n4 < n; ++n4) {
					this->cv[n4].clipDist = this->cv[n4].w - this->cv[n4].z;
				}
				break;
			}
		}
		for (int n5 = 0; n5 < n; ++n5) {
			int n6 = n5 + 1;
			if (n6 == n) {
				n6 = 0;
			}
			if (this->cv[n5].clipDist < 0 != this->cv[n6].clipDist < 0) {
				int n7 = n - 1 - n5;
				if (n7 > 0) {
					TGLVert* tglVert = &this->cv[n];
					std::memcpy(&this->cv[n5 + 2], &this->cv[n5 + 1], n7 * sizeof(TGLVert));
					std::memcpy(&this->cv[n5 + 1], tglVert, sizeof(TGLVert));
					++n6;
				}
				TGLVert* tglVert2 = &this->cv[n5 + 1];
				tglVert2->clipDist = 0;
				TGLVert* tglVert3;
				TGLVert* tglVert4;
				if (this->cv[n5].clipDist < 0) {
					tglVert3 = &this->cv[n6];
					tglVert4 = &this->cv[n5];
				}
				else {
					tglVert3 = &this->cv[n5];
					tglVert4 = &this->cv[n6];
				}
				int n8 = (tglVert3->clipDist << 16) / (tglVert3->clipDist - tglVert4->clipDist);
				tglVert2->x = tglVert3->x + ((tglVert4->x - tglVert3->x) * n8 >> 16);
				tglVert2->y = tglVert3->y + ((tglVert4->y - tglVert3->y) * n8 >> 16);
				tglVert2->z = tglVert3->z + ((tglVert4->z - tglVert3->z) * n8 >> 16);
				tglVert2->w = tglVert3->w + ((tglVert4->w - tglVert3->w) * n8 >> 16);
				tglVert2->s = tglVert3->s + ((tglVert4->s - tglVert3->s) * n8 >> 16);
				tglVert2->t = tglVert3->t + ((tglVert4->t - tglVert3->t) * n8 >> 16);
				++n;
				++n5;
			}
		}
		for (int n9 = 0; n9 < n; ++n9) {
			TGLVert* tglVert5 = &this->cv[n9];
			if (tglVert5->clipDist < 0) {
				std::memcpy(&this->cv[n9 + 0], &this->cv[n9 + 1], (n - 1 - n9) * sizeof(TGLVert));
				--n9;
				--n;
				std::memcpy(&this->cv[n], tglVert5, sizeof(TGLVert));
			}
		}
		if (n < 3) {
			return;
		}
		++i;
	}
	this->RasterizeConvexPolygon(n);
}

bool TinyGL::clipLine(TGLVert* array) {
	TGLVert* tglVert = &array[0];
	TGLVert* tglVert2 = &array[1];
	for (int i = 0; i < 3; ++i) {
		int n = 0;
		int n2 = 0;
		switch (i) {
			case 0:
			default: {
				n = tglVert->w + tglVert->x;
				n2 = tglVert2->w + tglVert2->x;
				break;
			}
			case 1: {
				n = tglVert->w - tglVert->x;
				n2 = tglVert2->w - tglVert2->x;
				break;
			}
			case 2: {
				n = tglVert->w + tglVert->z;
				n2 = tglVert2->w + tglVert2->z;
				break;
			}
		}
		int n3 = ((n < 0) ? 1 : 0) | ((n2 < 0) ? 2 : 0);
		if (n3 != 0) {
			if (n3 == 3) {
				return false;
			}
			switch (n3) {
				case 1: {
					int n4 = (n2 << 14) / (n2 - n);
					tglVert->x = tglVert2->x + (((tglVert->x - tglVert2->x) * n4) >> 14);
					tglVert->y = tglVert2->y + (((tglVert->y - tglVert2->y) * n4) >> 14);
					tglVert->z = tglVert2->z + (((tglVert->z - tglVert2->z) * n4) >> 14);
					tglVert->w = tglVert2->w + (((tglVert->w - tglVert2->w) * n4) >> 14);
					tglVert->s = tglVert2->s + (((tglVert->s - tglVert2->s) * n4) >> 14);
					tglVert->t = tglVert2->t + (((tglVert->t - tglVert2->t) * n4) >> 14);
					break;
				}
				case 2: {
					int n5 = (n << 14) / (n - n2);
					tglVert2->x = tglVert->x + (((tglVert2->x - tglVert->x) * n5) >> 14);
					tglVert2->y = tglVert->y + (((tglVert2->y - tglVert->y) * n5) >> 14);
					tglVert2->z = tglVert->z + (((tglVert2->z - tglVert->z) * n5) >> 14);
					tglVert2->w = tglVert->w + (((tglVert2->w - tglVert->w) * n5) >> 14);
					tglVert2->s = tglVert->s + (((tglVert2->s - tglVert->s) * n5) >> 14);
					tglVert2->t = tglVert->t + (((tglVert2->t - tglVert->t) * n5) >> 14);
					break;
				}
			}
		}
	}
	return true;
}


void TinyGL::projectVerts(TGLVert* array, int n) {
	for (int i = 0; i < n; i++) {
		TGLVert* tglVert = &array[i];
		tglVert->x = this->viewportXBias + ((tglVert->x * this->viewportXScale) / tglVert->w);
		tglVert->y = this->viewportYBias + ((tglVert->y * this->viewportYScale) / tglVert->w);
		tglVert->z = 0x7fffff / tglVert->w;
		tglVert->s *= tglVert->z;
		tglVert->t *= tglVert->z;
	}
}

void TinyGL::RasterizeConvexPolygon(int n) {
	Applet* app = CAppContainer::getInstance()->app;

	if ((app->render->renderMode & 0x4) == 0x0) {
		return;
	}

	if (!app->render->_gles->RasterizeConvexPolygon(n, this->cv)) {
		for (int i = 0; i < n; ++i) {
			TGLVert* tglVert = &this->cv[i];
			tglVert->x = this->viewportXBias + ((tglVert->x * this->viewportXScale) / tglVert->w);
			tglVert->y = this->viewportYBias + ((tglVert->y * this->viewportYScale) / tglVert->w);
			if (app->render->isSkyMap) {
				this->swapXY = false;
				// Original BREW version
				//tglVert->s = app->render->skyMapX + tglVert->x << (this->sShift - 7U & 0xff);
				//tglVert->t = app->render->skyMapY + tglVert->y << (this->tShift + 1U & 0xff);
				//tglVert->z = 4096;

				// [GEC] It is adjusted like this to be consistent with the IOS version
				tglVert->s = (132 << 4) + (app->render->skyMapX * 2) + tglVert->x << (this->sShift - 8U & 0xff);
				tglVert->t = app->render->skyMapY + tglVert->y << (this->tShift + 1U & 0xff);
				tglVert->z = 4096;
			}
			else {
				tglVert->z = 0x7FFFFF / tglVert->w;
				tglVert->s *= tglVert->z;
				tglVert->t *= tglVert->z;
			}
		}
		//++Span.countDrawn;
		if ((app->render->renderMode & 0x8) == 0x0) {
			return;
		}
		if (this->swapXY) {
			for (int j = 0; j < n; ++j) {
				int x = this->cv[j].x;
				this->cv[j].x = this->cv[j].y;
				this->cv[j].y = x;
			}
		}
		int n2 = this->cv[0].y;
		int n3 = 0;
		for (int k = 1; k < n; ++k) {
			if (this->cv[k].y < n2) {
				n2 = this->cv[k].y;
				n3 = k;
			}
		}
		int n4 = n3;
		TGLVert* tglVert2 = &this->cv[n3];
		TGLVert* tglVert3 = &this->cv[n4];
		int l = n2 + 7 >> 3;
		TGLEdge* tglEdge = &this->edges[0];
		TGLEdge* tglEdge2 = &this->edges[1];
		TGLEdge* tglEdge3 = tglEdge;
		TGLEdge* tglEdge4 = tglEdge2;
		int n5 = l;
		tglEdge4->stopY = n5;
		tglEdge3->stopY = n5;
		int screenWidth;
		if (this->swapXY) {
			screenWidth = this->screenWidth;
		}
		else {
			screenWidth = 1;
		}
		int n6 = n3;
		while (true) {
			if (l == tglEdge->stopY) {
				if (n3 == n4 && n3 != n6) {
					break;
				}
				if (n3 == 0) {
					n3 = n - 1;
				}
				else {
					--n3;
				}
				TGLVert* tglVert4 = tglVert2;
				tglVert2 = &this->cv[n3];
				tglEdge->setFromVerts(tglVert4, tglVert2);
			}
			if (l == tglEdge2->stopY) {
				if (n3 == n4) {
					break;
				}
				if (++n4 == n) {
					n4 = 0;
				}
				TGLVert* tglVert5 = tglVert3;
				tglVert3 = &this->cv[n4];
				tglEdge2->setFromVerts(tglVert5, tglVert3);
			}
			int n7 = (tglEdge->stopY < tglEdge2->stopY) ? tglEdge->stopY : tglEdge2->stopY;
			if (l > n7) {
				return;
			}
			while (l != n7) {
				TGLEdge* tglEdge5;
				TGLEdge* tglEdge6;
				if (tglEdge2->fracX < tglEdge->fracX) {
					tglEdge5 = tglEdge2;
					tglEdge6 = tglEdge;
				}
				else {
					tglEdge5 = tglEdge;
					tglEdge6 = tglEdge2;
				}
				int n8 = tglEdge5->fracX + 0x7FFFF >> 19;
				int n9 = tglEdge6->fracX + 0x7FFFF >> 19;
				if (n9 > n8) {
					int n10;
					if (this->swapXY) {
						n10 = n8 * this->screenWidth + l;
					}
					else {
						n10 = l * this->screenWidth + n8;
					}
					int n11 = tglEdge6->fracX - tglEdge5->fracX >> 16;
					int n12 = tglEdge5->fracZ >> 16;
					int n13 = (tglEdge5->fracS / n12) << 16;
					int n14 = (tglEdge5->fracT / n12) << 16;
					int n15 = tglEdge6->fracZ >> 16;
					int n16 = (tglEdge6->fracS / n15) << 16;
					int n17 = (tglEdge6->fracT / n15) << 16;
					if (n11 == 0) {
						this->spanPalette = this->getFogPalette(app->render->isSkyMap ? 0x10000000 : tglEdge6->fracZ);
						this->span->Span->DT(&this->pixels[n10], n16, n17, screenWidth, 0, 0, n9 - n8, this);
					}
					else {
						int n18 = (n8 << 3) - (tglEdge5->fracX >> 16);
						int n19 = (tglEdge6->fracZ - tglEdge5->fracZ) / n11;
						int n20 = (n16 - n13) / n11;
						int n21 = (n17 - n14) / n11;
						this->spanPalette = this->getFogPalette(app->render->isSkyMap ? 0x10000000 : tglEdge5->fracZ);
						if (n20 == 0) {
							this->span->Span->DT(&this->pixels[n10], n13, n14 + n18 * n21, screenWidth, 0, n21 << 3, n9 - n8, this);
						}
						else if (n21 == 0) {
							this->span->Span->DS(&this->pixels[n10], n13 + n20 * n18, n14, screenWidth, n20 << 3, 0, n9 - n8, this);
						}
						else {
							this->span->Span->Normal(&this->pixels[n10], n13 + n20 * n18, n14 + n18 * n21, screenWidth, n20 << 3, n21 << 3, n9 - n8, this);
						}
					}
				}

				tglEdge2->fracX += tglEdge2->stepX;
				tglEdge2->fracZ += tglEdge2->stepZ;
				tglEdge2->fracS += tglEdge2->stepS;
				tglEdge2->fracT += tglEdge2->stepT;
				tglEdge->fracX += tglEdge->stepX;
				tglEdge->fracZ += tglEdge->stepZ;
				tglEdge->fracS += tglEdge->stepS;
				tglEdge->fracT += tglEdge->stepT;
				++l;
			}
		}
	}
}

bool TinyGL::clippedLineVisCheck(TGLVert* tglVert, TGLVert* tglVert2, bool b) {
	int begX = tglVert->x + 7 >> 3;
	int endX = tglVert2->x + 7 >> 3;
	if (begX < 0) {
		begX = 0;
	}
	if (endX > this->screenWidth) {
		endX = this->screenWidth;
	}
	if (endX <= begX) {
		return false;
	}
	if (b) {
		int z = tglVert->z;
		for (int n = (tglVert2->z - tglVert->z) / (endX - begX), n2 = z + (n * ((begX << 3) - tglVert->x) >> 3); begX < endX; ++begX, n2 += n) {
			if (0x7fffff / n2 < this->columnScale[begX]) {
				return true;
			}
		}
		return false;
	}
	while (begX < endX) {
		if (this->columnScale[begX] == TinyGL::COLUMN_SCALE_INIT) {
			return true;
		}
		++begX;
	}
	return false;
}

bool TinyGL::occludeClippedLine(TGLVert* tglVert, TGLVert* tglVert2) {
	int begX = tglVert->x + 7 >> 3;
	int endX = tglVert2->x + 7 >> 3;
	if (begX < 0) {
		begX = 0;
	}
	if (endX > this->screenWidth) {
		endX = this->screenWidth;
	}
	if (endX <= begX) {
		return false;
	}
	int z = tglVert->z;
	int n = (tglVert2->z - tglVert->z) / (endX - begX);
	int n2 = z + (n * ((begX << 3) - tglVert->x) >> 3);
	bool b = false;
	while (begX < endX) {
		int n3 = 0x7fffff / n2;
		if (n3 < this->columnScale[begX]) {
			this->columnScale[begX] = n3;
			b = true;
		}
		++begX;
		n2 += n;
	}
	return b;
}

void TinyGL::drawClippedSpriteLine(TGLVert* tglVert, TGLVert* tglVert2, TGLVert* tglVert3, int n, bool b) {
	Applet* app = CAppContainer::getInstance()->app;

	int i = 0;
	int n2 = (tglVert->y - tglVert3->y << 12) / 176 >> 3;
	int n3 = tglVert3->y >> 3;
	int n4 = 16777216 / n2;
	if (0x0 != (n & 0x40000)) {
		n4 = -n4;
	}
	uint8_t* textureBase = this->textureBase;
	int n5 = this->textureBaseSize - ((textureBase[this->textureBaseSize - 1] & 0xFF) << 8 | (textureBase[this->textureBaseSize - 2] & 0xFF)) - 2;
	int n6 = this->imageBounds[0];
	int n7 = this->imageBounds[1];
	int n8 = n5 + (n7 - n6 + 1 >> 1);
	int n9 = 0;
	int j = tglVert->x + 7 >> 3;
	int viewportX2 = tglVert2->x + 7 >> 3;
	if (j < this->viewportX) {
		j = this->viewportX;
	}
	if (viewportX2 > this->viewportX2) {
		viewportX2 = this->viewportX2;
	}
	if (viewportX2 - j <= 0) {
		return;
	}
	int n10 = (tglVert2->s - tglVert->s) / (viewportX2 - j);
	int s = tglVert->s;
	while (j < viewportX2) {
		int n11 = (176 * (s / tglVert->z) + (176 * (s / tglVert->z) < 0 ? 63 : 0)) >> 10;
		if (0x0 != (n & 0x20000)) {
			n11 = 175 - n11;
		}
		if (n11 >= n6 && n11 < n7) {
			int n12;
			for (n12 = n11 - n6; i < n12; ++i) {
				int n13 = textureBase[n5 + (i >> 1)] >> ((i & 0x1) << 2) & 0xF;
				// [GEC]: Fix animated water sprite
				{
					app->render->fixTexels(n5 + (i >> 1), (i & 0x1), this->mediaID, &n13);
				}
				while (n13-- > 0) {
					n9 += textureBase[n8 + 1];
					n8 += 2;
				}
			}
			while (i > n12) {
				--i;
				int n14 = textureBase[n5 + (i >> 1)] >> ((i & 0x1) << 2) & 0xF;
				// [GEC]: Fix animated water sprite
				{
					app->render->fixTexels(n5 + (i >> 1), (i & 0x1), this->mediaID, &n14);
				}
				while (n14-- > 0) {
					n9 -= textureBase[n8 - 1];
					n8 -= 2;
				}
			}
			int n15 = textureBase[n5 + (i >> 1)] >> ((i & 0x1) << 2) & 0xF;
			// [GEC]: Fix animated water sprite
			{
				app->render->fixTexels(n5 + (i >> 1), (i & 0x1), this->mediaID, &n15);
			}
			while (n15-- > 0) {
				int n16 = textureBase[n8++];
				uint8_t b2 = textureBase[n8++];
				int n17 = n9 << 12;
				n9 += b2;
				if (0x0 != (n & 0x40000)) {
					n16 = 176 - (n16 + b2);
					n17 += (b2 << 12) - 1;
				}
				int viewportY = n3 + (n16 * n2 >> 12);
				int n18 = n2 * b2 >> 12;
				if (viewportY < this->viewportY) {
					int n19 = this->viewportY - viewportY;
					n18 -= n19;
					n17 += n19 * n4;
					viewportY = this->viewportY;
				}
				if (viewportY + n18 > this->viewportY2) {
					n18 = this->viewportY2 - viewportY;
				}
				if (n18 > 0) {
					//TinyGL.span.span(j + viewportY * TinyGL.screenWidth, n17, TinyGL.screenWidth, n4, n18);
					this->span->Span->Stretch(&this->pixels[j + this->screenWidth * viewportY], n17, n4, this->screenWidth, n18, this);
				}
			}
			++i;
		}
		s += n10;
		++j;
	}
}

void TinyGL::resetCounters() {
	this->countBackFace = 0;
	this->countDrawn = 0;
	this->spanPixels = 0;
	this->spanCalls = 0;
	this->zeroDT = 0;
	this->zeroDS = 0;
}


// [GEC]

void TinyGL::applyClearColorBuffer() {
	Applet* app = CAppContainer::getInstance()->app;
	uint16_t* pixels = this->pixels;
	int color = this->colorBuffer;
	int n2 = this->viewportX + this->viewportY * this->screenWidth;
	for (int i = 0; i < this->viewportWidth; ++i) {
		pixels[n2 + i] = Render::upSamplePixel(color);
	}
	int n3 = n2;
	for (int j = 1; j < this->viewportHeight; ++j) {
		n3 += this->screenWidth;
		std::memcpy(&pixels[n3], &pixels[n2], this->viewportWidth * sizeof(uint16_t));
	}
}
