#include <stdexcept>

#include "CAppContainer.h"
#include "Graphics.h"
#include "Render.h"
#include "TinyGL.h"
#include "Image.h"
#include "Canvas.h"
#include "Text.h"
#include "SDLGL.h"

#define COLOR_BYTE_TO_FLOAT(x) (float)((float)(x) * (1.f / 256)) // (1.f / 256) -> 0.0039062f

Graphics::Graphics() {
	std::memset(this, 0, sizeof(Graphics));
}

Graphics::~Graphics() {
}

void Graphics::setGraphics() {
	Canvas* canvas = CAppContainer::getInstance()->app->canvas;
	printf("Graphics::setGraphics\n");

	this->graphClipRect[0] = canvas->displayRect[0];
	this->graphClipRect[1] = canvas->displayRect[1];
	this->graphClipRect[2] = canvas->displayRect[2];
	this->graphClipRect[3] = canvas->displayRect[3];
	this->transX = canvas->displayRect[0];
	this->transY = canvas->displayRect[1];
}

void Graphics::setColor(int color) {
	this->curColor = color;
}

void Graphics::fillCircle(int x, int y, int rad) {
	float r, g, b;
	float vp[48];
	float amount;
	int vertIndex;
    float scaleX, scaleY;

    r = COLOR_BYTE_TO_FLOAT(this->curColor >> 16 & 0xFF);
    g = COLOR_BYTE_TO_FLOAT(this->curColor >> 8 & 0xFF);
    b = COLOR_BYTE_TO_FLOAT(this->curColor & 0xFF);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glPushMatrix();
    scaleX = (float)(x);
    scaleY = (float)(y);
    //CAppContainer::getInstance()->sdlGL->transformCoord2f(&scaleX, &scaleY);

	glTranslatef(scaleX, scaleY, 0.0f);

	amount = 0.0f;
	vertIndex = 0;
	do
	{
		float v16 = (float)(amount * 0.017444f);

        scaleX = (float)(rad) * (float)cos(v16);
        scaleY = (float)(rad) * (float)sin(v16);
        //CAppContainer::getInstance()->sdlGL->transformCoord2f(&scaleX, &scaleY);

		vp[vertIndex++] = scaleX;
		vp[vertIndex++] = scaleY;
		vp[vertIndex++] = 0.5f;
		amount += 22.5f;
	} while (vertIndex != 48);

	glColor4f(r, g, b, 1.0f);
	glVertexPointer(3, GL_FLOAT, 0, vp);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 16);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
}

void Graphics::fillRect(int x, int y, int w, int h) {
    float a = COLOR_BYTE_TO_FLOAT(this->curColor >> 24 & 0xFF);
    float r = COLOR_BYTE_TO_FLOAT(this->curColor >> 16 & 0xFF);
    float g = COLOR_BYTE_TO_FLOAT(this->curColor >> 8 & 0xFF);
    float b = COLOR_BYTE_TO_FLOAT(this->curColor & 0xFF);

	this->FMGL_fillRect(x, y, w, h, r, g, b, 1.0);
}

void Graphics::fillRect(int x, int y, int w, int h, int color) {
	int oldColor = this->curColor;
    this->setColor(color);
	this->fillRect(x, y, w, h);
    this->setColor(oldColor);
}

void Graphics::FMGL_fillRect(int x, int y, int w, int h, float r, float g, float b, float a) {
    int v9; // r6
    int v11; // r1
    int v13; // r2
    int v14; // lr
    int v15; // r12
    int v16; // r0
    int v17; // r12
    bool v18; // cc
    int v19; // r8

    float vp[12];

    v11 = this->graphClipRect[0];
    v14 = this->graphClipRect[1];
    v15 = this->graphClipRect[2];
    v16 = this->graphClipRect[3];
    v13 = x + w;
    if (v11 <= x + w)
    {
        v17 = v11 + v15;
        v18 = x <= v17;
        if (x <= v17)
        {
            v13 = h;
            v9 = y + h;
            v18 = v14 <= y + h;
        }
        if (v18)
        {
            v16 += v14;
            v18 = y <= v16;
        }
        if (v18)
        {
            if (v11 > x)
            {
                v13 = v11 - x;
                if (w < v11 - x) {
                    return;
                }
                w -= v13;
                x = v11;
            }
            if (v14 > y)
            {
                v13 = v14 - y;
                if (h < v14 - y) {
                    return;
                }
                y = v14;
                v9 = v14 + h - v13;
                h -= v13;
            }
            v19 = x + w;
            if (v17 < x + w)
            {
                v13 = v19 - v17;
                w = v17 - x;
                v19 = v17;
            }
            if (v16 < v9) {
                w = v9 - v16;
            }
            if (v16 < v9) {
                v13 = h;
            }
            if (v16 < v9) {
                v9 = y + v13 - w;
            }

            // v1
            vp[0] = 0.0;
            vp[1] = 0.0;
            vp[2] = 0.5f;

            // v2
            vp[3] = 0.0;
            vp[4] = 0.0;
            vp[5] = 0.5f;

            // v3
            vp[6] = 0.0;
            vp[7] = 0.0;
            vp[8] = 0.5f;

            // v4
            vp[9] = 0.0;
            vp[10] = 0.0;
            vp[11] = 0.5f;

            glDisable(GL_TEXTURE_2D);
            if (a >= 1.0)
            {
                glDisable(GL_ALPHA_TEST);
                glDisable(GL_BLEND);
            }
            else
            {
                glEnable(GL_ALPHA_TEST);
                glAlphaFunc(GL_GREATER, 0.0);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }

            float scaleX = (float)(x);
            float scaleY = (float)(y);
            float scaleW = (float)(v19);
            float scaleH = (float)(v9);
            //CAppContainer::getInstance()->sdlGL->transformCoord2f(&scaleX, &scaleY);
            //CAppContainer::getInstance()->sdlGL->transformCoord2f(&scaleW, &scaleH);

            vp[0] = (float)scaleW;
            vp[6] = (float)scaleW;
            vp[1] = scaleY;
            vp[4] = scaleY;
            vp[3] = scaleX;
            vp[9] = scaleX;
            vp[7] = (float)scaleH;
            vp[10] = (float)scaleH;


            glColor4f(r, g, b, a);
            glVertexPointer(3, GL_FLOAT, 0, vp);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}

void Graphics::drawRect(int x, int y, int w, int h) {
    this->fillRect(x, y, 1, h);
    this->fillRect(x, y + h, w, 1);
    this->fillRect(x + 1, y, w, 1);
    this->fillRect(x + w, y + 1, 1, h);
}

void Graphics::drawRect(int x, int y, int w, int h, int color) {
    int oldColor = this->curColor;
    this->setColor(color);
    this->drawRect(x, y, w, h);
    this->setColor(oldColor);
}

void Graphics::eraseRgn(int x, int y, int w, int h) {
    int oldColor = this->curColor;
    this->setColor(0);
    this->fillRect(x, y, w, h);
    this->setColor(oldColor);
}

void Graphics::eraseRgn(int* rect) {
    this->eraseRgn(rect[0], rect[1], rect[2], rect[3]);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2) {
    float v9[6];
    float r = COLOR_BYTE_TO_FLOAT(this->curColor >> 16 & 0xFF);
    float g = COLOR_BYTE_TO_FLOAT(this->curColor >> 8 & 0xFF);
    float b = COLOR_BYTE_TO_FLOAT(this->curColor & 0xFF);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glColor4f(r, g, b, 1.0f);

    // v1
    v9[0] = (float)x1;
    v9[1] = (float)y1;
    v9[2] = 0.5f;

    // v2
    v9[3] = (float)x2;
    v9[4] = (float)y2;
    v9[5] = 0.5f;

    //CAppContainer::getInstance()->sdlGL->transformCoord2f(&v9[0], &v9[1]);
    //CAppContainer::getInstance()->sdlGL->transformCoord2f(&v9[3], &v9[4]);

    glVertexPointer(3, GL_FLOAT, 0, v9);
    glDrawArrays(GL_LINE_LOOP, 0, 2);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2, int color) {
    int oldColor = this->curColor;
    this->setColor(color);
    this->drawLine(x1, y1, x2, y2);
    this->setColor(oldColor);
}

void Graphics::drawImage(Image* img, int x, int y, int flags, int rotateMode, int renderMode) {
    if (img) {
        this->drawRegion(img, 0, 0, img->width, img->height, x, y, flags, rotateMode, renderMode);
    }
}

void Graphics::drawRegion(Image* img, int texX, int texY, int texW, int texH, int posX, int posY, int flags, int rotateMode, int renderMode) {
    IDIB* piDIB;
    uint16_t* data;
    int x, y, w, h, tX, tY;

    if (img == nullptr) {
        return;
    }

    if (img->piDIB == nullptr) {
        return;
    }

    x = posX;
    y = posY;
    piDIB = img->piDIB;

    if (img->texture == -1) {
        img->texWidth = 1;
        img->texHeight = 1;

        while (w = img->texWidth, w < piDIB->width) {
            img->texWidth = w << 1;
        }
        while (h = img->texHeight, h < piDIB->height) {
            img->texHeight = h << 1;
        }

        data = (uint16_t*)std::malloc(w * h * sizeof(uint16_t));
        img->isTransparentMask = false;

        for (w = 0; w < piDIB->height; w++) {
            for (h = 0; h < piDIB->width; h++) {
                uint16_t rgb = piDIB->pRGB565[piDIB->pBmp[piDIB->width * w + h]];
                if (rgb == 0xf81f) {
                    img->isTransparentMask = true;
                }
                if (data != nullptr) {
                    data[(w * img->texWidth) + h] = rgb;
                }
            }
        }
        if (data != nullptr) {
            img->CreateTexture(data, img->texWidth, img->texHeight);
            std::free(data);
        }
    }

    if ((flags & 1U) == 0) {
        if ((flags & 8U) != 0) {
            x = 0 - texW;
            goto LAB_0003d4f8;
        }
    }
    else {
        x = 0 - (texW / 2);
    LAB_0003d4f8:
        x = posX + x;
    }
    if ((flags & 2U) == 0) {
        if ((flags & 0x20U) == 0) goto LAB_0003d52c;
        y = 0 - texH;
    }
    else {
        y = 0 - (texH / 2);
    }
    y = posY + y;
LAB_0003d52c:
    x = x + this->transX;
    y = y + this->transY;
    w = this->graphClipRect[0];
    tX = texX;
    if (x < w) {
        texW = texW - (w - x);
        tX = texX + (w - x);
        x = w;
    }
    h = this->graphClipRect[1];
    tY = texY;
    if (y < h) {
        texH = texH - (h - y);
        tY = texY + (h - y);
        y = h;
    }
    if (w + this->graphClipRect[2] < texW + x) {
        texW = this->graphClipRect[2] - (x - w);
    }
    if (h + this->graphClipRect[3] < texH + y) {
        texH = this->graphClipRect[3] - (y - h);
    }
    w = texH;
    if (0 < texH) {
        w = texW;
    }
    if (0 < w) {
        img->DrawTexture(tX, tY, texW, texH, x, y, rotateMode, renderMode);
    }
}

void Graphics::fillRegion(Image* img, int x, int y, int w, int h) {
    if (img != nullptr) {
        int texW = img->width;
        int texH = img->height;
        if (w < texW) {
            texW = w;
        }
        if (h < texH) {
            texH = h;
        }
        this->fillRegion(img, 0, 0, texW, texH, x, y, w, h, 0);
    }
}

void Graphics::fillRegion(Image* img, int x, int y, int w, int h, int rotateMode) {
    if (img != nullptr) {
        int texW = img->width;
        int texH = img->height;
        if (w < texW) {
            texW = w;
        }
        if (h < texH) {
            texH = h;
        }
        this->fillRegion(img, 0, 0, texW, texH, x, y, w, h, rotateMode);
    }
}

void Graphics::fillRegion(Image* img, int texX, int texY, int texW, int texH, int x, int y, int w, int h, int rotateMode) {
    int xBeg, xEnd;
    int yBeg, yEnd;
    int texw, texh;

    yBeg = y;
    yEnd = y + h;
    xEnd = x + w;
    while (yBeg < yEnd)
    {
        texh = yEnd - yBeg;
        xBeg = x;
        if (yEnd - yBeg >= texH) {
            texh = texH;
        }
        while (xBeg < xEnd)
        {
            texw = xEnd - xBeg;
            if (xEnd - xBeg >= texW) {
                texw = texW;
            }
            this->drawRegion(img, texX, texY, texw, texh, xBeg, yBeg, 0, rotateMode, 0);
            xBeg += texw;
        }
        yBeg += texh;
    }
}

void Graphics::drawBevel(int color1, int color2, int x, int y, int w, int h) {
    --w;
    --h;
    this->setColor(color1);
    this->drawLine(x, y, x, y + h);
    this->drawLine(x, y, x + w, y);
    this->setColor(color2);
    this->drawLine(x + w, y, x + w, y + h);
    this->drawLine(x, y + h, x + w, y + h);
}

void Graphics::drawString(Text* text, int x, int y, int flags) {
    Canvas* canvas = CAppContainer::getInstance()->app->canvas;
    this->drawString(canvas->imgFont, text, x, y, 16, flags, 0, text->length());
}

void Graphics::drawString(Text* text, int x, int y, int flags, int strBeg, int strEnd) {
    Canvas* canvas = CAppContainer::getInstance()->app->canvas;
    this->drawString(canvas->imgFont, text, x, y, 16, flags, strBeg, strEnd);
}

void Graphics::drawString(Text* text, int x, int y, int h, int flags, int strBeg, int strEnd) {
    Canvas* canvas = CAppContainer::getInstance()->app->canvas;
    this->drawString(canvas->imgFont, text, x, y, h, flags, strBeg, strEnd);
}

void Graphics::drawString(Image* img, Text* text, int x, int y, int h, int flags, int strBeg, int strEnd) {
    int rotateMode = 0;

    if (text == nullptr) {
        return;
    }

    if (strEnd == -1) {
        strEnd = text->length();
    }

    if (strEnd == 0) {
        return;
    }

    if (flags & 0x40) {
        rotateMode = 3;
    }

    int stringWidth = text->getStringWidth(strBeg, strBeg + strEnd, false);
    int n6 = x;

    if (rotateMode == 3) {
        n6 = y;
    }

    if ((flags & 0x8) != 0x0) {
        if (rotateMode == 3) {
            y += stringWidth;
            n6 = y;
        }
        else {
            x -= stringWidth;
            n6 = x;
        }
    }
    else if ((flags & 0x1) != 0x0) {
        if (rotateMode == 3) {
            y += stringWidth / 2 + (stringWidth & 0x1);
        }
        else {
            x -= stringWidth / 2 + (stringWidth & 0x1);
        }

    }
    if ((flags & 0x20) != 0x0) {
        if (rotateMode == 3) {
            x += h;
        }
        else {
            y -= h;
        }
    }
    else if ((flags & 0x2) != 0x0) {
        if (rotateMode == 3) {
            x -= h * text->getNumLines() / 2;
        }
        else {
            y -= h * text->getNumLines() / 2;
        }
    }

    int n7 = strBeg + std::min(strEnd, text->length() - strBeg);
    for (int i = strBeg; i < n7; ++i) {
        char char1 = text->charAt(i);
        if (char1 == '\n' || char1 == '|') {

            if (rotateMode == 3) {
                x += h;
            }
            else {
                y += h;
            }

            if ((flags & 0x1) != 0x0) {
                if (rotateMode == 3) {
                    y = n6 + text->getStringWidth(i + 1, n7, false) / 2;
                }
                else {
                    x = n6 - text->getStringWidth(i + 1, n7, false) / 2;
                }
            }
            else {
                if (rotateMode == 3) {
                    y = n6;
                }
                else {
                    x = n6;
                }
            }
        }
        else if (char1 == ' ' || char1 == ' ') {
            if (rotateMode == 3) {
                y -= 9;
            }
            else {
                x += 9;
            }
        }
        else {
            if (char1 == '\\'/* && i < n7*/) {
                char char2 = text->charAt(++i);
                int n8 = char2 - 'A';
                if (n8 < 0 || n8 >= 15) {
                    if (rotateMode == 3) {
                        this->drawChar(img, char2, x, y, rotateMode);
                        y -= 9;
                    }
                    else {
                        this->drawChar(img, char2, x, y, rotateMode);
                        x += 9;
                    }
                }
                else {
                    this->drawBuffIcon(n8, x, y, 0);
                    if (rotateMode == 3) {
                        y -= 9;
                    }
                    else {
                        x += (9 * 3);
                    }
                }
            }
            else if (char1 == '^'/* && i < n7*/) {
                short currentCharColor = (short)(text->charAt(++i) - '0');
                if (currentCharColor >= 0 && currentCharColor <= 9) {
                    this->currentCharColor = currentCharColor;
                    continue;
                }
                if (rotateMode == 3) {
                    this->drawChar(img, '^', x, y, rotateMode);
                    y -= 9;
                }
                else {
                    this->drawChar(img, '^', x, y, rotateMode);
                    x += 9;
                }
                //--i;
            }
            else {
                if (rotateMode == 3) {
                    this->drawChar(img, char1, x, y, rotateMode);
                    y -= 9;
                }
                else {
                    this->drawChar(img, char1, x, y, rotateMode);
                    x += 9;
                }
            }
        }
    }
    this->currentCharColor = 0;
}

void Graphics::drawChar(Image* img, char c, int x, int y, int rotateMode) {
    Localization* loc = CAppContainer::getInstance()->app->localization;
    int renderMode, index1, index2;

    loc->getCharIndices(c, &index1, &index2);
    if (((143 < (uint32_t)index1) || (index2 < 0)) || (0x8f < index2)) {
        index1 = 30;
        index2 = 0;
    }
    if (this->currentCharColor == 0) {
        renderMode = 0;

        // [GEC] Estas lineas no existen el codigo
        // pero son funcionales
        if (CAppContainer::getInstance()->app->canvas->fontRenderMode != 0) { 
            renderMode = CAppContainer::getInstance()->app->canvas->fontRenderMode;
        }
    }
    else {
        renderMode = 8;
    }

    this->drawRegion(img, (index1 & 15) * 12, index1 & 240, 12, 16, x, y, 0, rotateMode, renderMode);
    if (index2 != 0) {
        this->drawRegion(img, (index2 & 15) * 12, index2 & 240, 12, 16, x, y, 0, rotateMode, renderMode);
    }
}

void Graphics::drawBuffIcon(int texY, int posX, int posY, int flags) {
    Canvas* canvas = CAppContainer::getInstance()->app->canvas;
    this->drawRegion(canvas->imgIcons_Buffs, 0, texY * 30, 30, 30, posX, posY, flags, 0, 0);
}

void Graphics::drawCursor(int x, int y, int flags) {
    this->drawCursor(x, y, flags, true);
}

void Graphics::drawCursor(int x, int y, int flags, bool b) {
    Localization* loc = CAppContainer::getInstance()->app->localization;
    Text* smallBuffer = loc->getSmallBuffer();
    smallBuffer->setLength(0);
    if (b) {
        smallBuffer->append('\x8A');
    }
    else {
        smallBuffer->append('\x84');
    }
    this->drawString(smallBuffer, x, y, flags);
    smallBuffer->dispose();
}

void Graphics::clipRect(int x, int y, int w, int h) {
    int clipX = this->graphClipRect[0];
    int clipY = this->graphClipRect[1];
    int clipW = this->graphClipRect[2];
    int clipH = this->graphClipRect[3];

    x += this->transX;
    y += this->transY;

    if (x < clipX) {
        w -= clipX - x;
        x = clipX;
    }
    if (y < clipY) {
        h -= clipY - y;
        y = clipY;
    }
    if (x + w > clipX + clipW) {
        w = clipW - (x - clipX);
    }
    if (y + h > clipY + clipH) {
        h = clipH - (y - clipY);
    }

    this->graphClipRect[0] = x;
    this->graphClipRect[1] = y;
    this->graphClipRect[2] = w;
    this->graphClipRect[3] = h;
}

void Graphics::setClipRect(int x, int y, int w, int h) {
    this->graphClipRect[0] = x;
    this->graphClipRect[1] = y;
    this->graphClipRect[2] = w;
    this->graphClipRect[3] = h;
}

void Graphics::clearClipRect() {
    this->graphClipRect[0] = 0;
    this->graphClipRect[1] = 0;
    this->graphClipRect[2] = this->backBuffer->width;
    this->graphClipRect[3] = this->backBuffer->height;
}

void Graphics::setScreenSpace(int* rect) {
    this->graphClipRect[0] = rect[0];
    this->graphClipRect[1] = rect[1];
    this->graphClipRect[2] = rect[2];
    this->graphClipRect[3] = rect[3];
    this->transX = rect[0];
    this->transY = rect[1];
}

void Graphics::setScreenSpace(int x, int y, int w, int h) {
    this->graphClipRect[0] = x;
    this->graphClipRect[1] = y;
    this->graphClipRect[2] = w;
    this->graphClipRect[3] = h;
    this->transX = x;
    this->transY = y;
}

void Graphics::resetScreenSpace() {
    Canvas* canvas = CAppContainer::getInstance()->app->canvas;
    this->setScreenSpace(canvas->displayRect);
}

void Graphics::fade(int* rect, int alpha, int color) {
    float r = COLOR_BYTE_TO_FLOAT(color >> 16 & 0xFF);
    float g = COLOR_BYTE_TO_FLOAT(color >> 8 & 0xFF);
    float b = COLOR_BYTE_TO_FLOAT(color & 0xFF);
    float a = COLOR_BYTE_TO_FLOAT(alpha);

    this->graphClipRect[0] = 0;
    this->graphClipRect[1] = 0;
    this->graphClipRect[2] = this->backBuffer->width;
    this->graphClipRect[3] = this->backBuffer->height;
    this->FMGL_fillRect(rect[0], rect[1], rect[2], rect[3], r, g, b, 1.0f - a);
}


void Graphics::drawPixelPortal(int* rect, int x, int y, uint32_t color) {
    Applet* app = CAppContainer::getInstance()->app;
    int screenWidth = app->tinyGL->screenWidth;
    int screenHeight = app->tinyGL->screenHeight;

    uint32_t uVar1;
    int iVar2;
    uint32_t uVar4;
    int iVar5;

    uint16_t* pixels = &app->tinyGL->pixels[x + (y * screenWidth)];

    uVar1 = color >> 0x18;
    if (uVar1 != 0) {
        if (uVar1 != 0xff) {
            uVar4 = (uint32_t)*pixels;
            iVar2 = uVar1 << 16;
            iVar5 = -(uVar1 << 16) + 0xff0000;
            uVar1 = (uVar4 & 0x1f) << 3;
            uVar4 = (uVar4 & 0xf800) << 8 | (uVar4 & 0x7e0) << 5 | uVar1;
            color = (((uVar4 << 8) >> 0x18) * iVar5 + iVar2 * ((color << 8) >> 0x18) >> 0x18) << 0x10 |
                ((int)(((uVar4 << 0x10) >> 0x18) * iVar5 + iVar2 * ((color << 0x10) >> 0x18)) >> 0x18
                    & 0xfeU) << 8 | uVar1 * iVar5 + iVar2 * (color & 0xff) >> 0x18 | 0xff000000;
        }
        iVar2 = Render::upSamplePixel(color);
        *pixels = (uint16_t)iVar2;
    }
    return;
}