#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Canvas.h"
#include "Image.h"
#include "SDLGL.h"

Image::Image() {
}

Image::~Image() {
    if (this) {
        if (this->piDIB) {
            this->piDIB->~IDIB();
            std::free(this->piDIB);
        }
        this->piDIB = nullptr;
        glDeleteTextures(1, &this->texture);
        this->texture = -1;
        std::free(this);
    }
}

void Image::CreateTexture(uint16_t* data, uint32_t width, uint32_t height) {
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &this->texture);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if (this->isTransparentMask == false) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data);
    }
    else {
        uint32_t i = 0;
        uint16_t* dat = data;
        while (i < height * width) {
            uint32_t pix = *dat;
            if (pix == 0xf81f) {
                pix = 0x0000;
            }
            else {
                pix = pix & 0xffc0 | (uint16_t)((pix & 0x1f) << 1) | 1;
            }
            *dat++ = pix;
            i++;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, data);
    }
}

void Image::DrawTexture(int texX, int texY, int texW, int texH, int posX, int posY, int rotateMode, int renderMode) {
    float scaleW, scaleH;
    float scaleTexW, scaleTexH;
    float scaleX, scaleY;
    float vp[12];
    float st[8];

    PFNGLACTIVETEXTUREPROC glActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");

    this->setRenderMode(renderMode);
    scaleW = (float)texW * 0.5f;
    scaleH = (float)texH * 0.5f;
    scaleX = (float)posX;
    scaleY = (float)posY;

    //CAppContainer::getInstance()->sdlGL->transformCoord2f(&scaleW, &scaleH);
    //CAppContainer::getInstance()->sdlGL->transformCoord2f(&scaleX, &scaleY);

    vp[2] = 0.5f;
    vp[5] = 0.5f;
    vp[0] = scaleW;
    vp[8] = 0.5f;
    vp[11] = 0.5f;
    vp[3] = -scaleW;
    vp[6] = scaleW;
    vp[7] = scaleH;
    vp[1] = -scaleH;
    vp[4] = -scaleH;
    vp[9] = -scaleW;
    vp[10] = scaleH;

    st[0] = 0.0f;
    st[1] = 0.0f;
    st[2] = 0.0f;
    st[3] = 0.0f;
    st[4] = 0.0f;
    st[5] = 0.0f;
    st[6] = 0.0f;
    st[7] = 0.0f;

    scaleTexW = 1.0f / (float)this->texWidth;
    scaleTexH = 1.0f / (float)this->texHeight;
    st[0] = scaleTexW * (float)(texW + texX);
    st[4] = st[0];
    st[1] = scaleTexH * (float)texY;
    st[3] = st[1];
    st[2] = scaleTexW * (float)texX;
    st[6] = st[2];
    st[5] = scaleTexH * (float)(texH + texY);
    st[7] = st[5];

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);
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
    glTranslatef(scaleW + (float)(scaleX), scaleH + (float)(scaleY), 0.0);
    switch (rotateMode)
    {
    case 1:
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        break;
    case 2:
        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
        break;
    case 3:
        glRotatef(270.0f, 0.0f, 0.0f, 1.0f);
        break;
    case 4:
        glScalef(-1.0f, 1.0f, 1.0f);
        break;
    case 5:
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        glScalef(-1.0f, 1.0f, 1.0f);
        break;
    case 6:
        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
        glScalef(-1.0f, 1.0f, 1.0f);
        break;
    case 7:
        glRotatef(270.0f, 0.0f, 0.0f, 1.0f);
        glScalef(-1.0f, 1.0f, 1.0f);
        break;

    case 8: // New
        glScalef(1.0f, -1.0f, 1.0f);
        break;
    default:
        break;
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPopMatrix();
}

void Image::setRenderMode(int renderMode) {
    Applet* app = CAppContainer::getInstance()->app;
    int color;

    switch (renderMode) {
    case 0:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        if (this->isTransparentMask == false) {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            return;
        }
        glEnable(GL_ALPHA_TEST);
        break;
    case 1:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
        glDisable(GL_ALPHA_TEST);
        break;
    case 2:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        glDisable(GL_ALPHA_TEST);
        break;
    case 3:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_COLOR, GL_ONE);
        return;
    case 8:
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glEnable(GL_BLEND);
        color = Graphics::charColors[app->canvas->graphics.currentCharColor];
        glColor4ub(color >> 0x10 & 0xff, color >> 8 & 0xff, color & 0xff, color >> 0x18);
        return;
    case 12:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
        glDisable(GL_ALPHA_TEST);
        break;
    case 13:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor4f(1.0f, 1.0f, 1.0f, app->canvas->blendSpecialAlpha);
        glDisable(GL_ALPHA_TEST);
        break;
    default:
        return;
    }
    glAlphaFunc(GL_GREATER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
