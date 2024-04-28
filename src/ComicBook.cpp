#include <stdexcept>

#include "SDLGL.h"
#include "CAppContainer.h"
#include "App.h"
#include "ComicBook.h"
#include "Image.h"
#include "Graphics.h"
#include "Text.h"
#include "Sound.h"
#include "MenuSystem.h"
#include "Enums.h"
#include "Canvas.h"

ComicBook::ComicBook() {
	//printf("ComicBook::Constructor %d\n", sizeof(ComicBook));
	std::memset(this, 0, sizeof(ComicBook));

    this->field_0x10 = 0;
    this->field_0x0 = 0;
    this->field_0x4 = 0;
    this->begPoint = 0;
    this->curX = 0;
    this->curY = 0;
    for (int i = 0; i < 17; i++) {
        this->imgComicBook[i] = nullptr;
    }
    for (int i = 0; i < 39; i++) {
        this->imgiPhoneComicBook[i] = nullptr;
    }
    this->accelerationX = 0.0;
    this->accelerationY = 0.0;
    this->accelerationZ = 0.0;
    this->exitBtnRect[0] = 10;
    this->exitBtnRect[1] = 10;
    this->exitBtnRect[2] = 60;
    this->exitBtnRect[3] = 30;
    this->field_0x16c = 200;
    this->field_0x170 = 20;
    this->field_0x164 = 140;
    this->isLoaded = 0;
    this->comicBookIndex = 0;
    this->iPhoneComicIndex = 0;
    this->field_0x1c = 0;
    this->iPhonePage = 0;
    this->page = 0;
    this->curPage = 0;
    this->field_0x110 = 0;
    this->endPoint = 0;
    this->field_0x118 = 0;
    this->field_0x11c = 0;
    this->midPoint = 0;
    this->is_iPhoneComic = false;
    this->field_0x135 = 0;
    this->field_0x138 = 0;
    this->field_0x13c = 0;
    this->field_0x140 = 0;
    this->drawExitButton = 0;
    this->field_0x14c = 0;
    this->field_0x168 = 280;
    this->field_0x174 = 0;
}

ComicBook::~ComicBook() {
}

int g_interfaceOrientation = 1; 
int resetOrientation;
bool GetOrientation(void)
{
    return g_interfaceOrientation != 4;
}
void SetOrientationRight(void)
{
    resetOrientation = 1;
}

void ComicBook::Draw(Graphics* graphics) {
    int comicBookIndex; // r1
    int iPhoneComicIndex; // r1
    bool v5; // zf
    float v6; // s17
    int is_iPhoneComic; // r5
    float v8; // s16
    bool v9; // r0
    bool v10; // r3
    struct Image** imgiPhoneComicBook; // r8
    int iPhonePage; // r6
    char v13; // r11
    bool v14; // r3
    int v15; // r1
    int v16; // r2
    int v17; // r2
    int v18; // r5
    Image* v19; // r12
    int v20; // r10
    Image** v21; // r1
    int v23; // [sp+10h] [bp-28h]

    ++this->field_0x14c;
    this->isLoaded = true;

    if (this->comicBookIndex <= 16) {
        if (!this->imgComicBook[this->comicBookIndex]) {
            this->loadImage(this->comicBookIndex, 1);
        }
        this->comicBookIndex++;
        this->isLoaded = false;
    }

    if (this->iPhoneComicIndex <= 38) {
        if (!this->imgiPhoneComicBook[this->iPhoneComicIndex]) {
            this->loadImage(this->iPhoneComicIndex, 0);
        }
        this->iPhoneComicIndex++;
        this->isLoaded = false;
    }

    if (!this->isLoaded) {
        this->DrawLoading(graphics);
        return;
    }

    if (this->is_iPhoneComic && !GetOrientation())
    {
        v5 = !this->field_0x13c;
        if (!this->field_0x13c)
            v5 = !this->field_0x135;
        if (v5 && !this->field_0x110) {
            SetOrientationRight();
        }
    }

    this->UpdateMovement();
    this->UpdateTransition();
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (this->field_0x13c)
    {
        v6 = (float)((float)this->field_0x140 / -30.0) + 1.0;
        v8 = 1.0 - v6;
        if (this->is_iPhoneComic)
        {
            this->DrawImage(this->imgiPhoneComicBook[this->iPhonePage], 0, 0, 0, v8, 0);
        }
        else
        {
            v9 = !GetOrientation();
            this->DrawImage(this->imgComicBook[this->page], 0, 0, !this->is_iPhoneComic, v8, v9);
        }
    }
    else
    {
        v6 = 1.0;
    }
    v10 = this->is_iPhoneComic;
    imgiPhoneComicBook = this->imgiPhoneComicBook;
    iPhonePage = this->iPhonePage;
    if (v10)
    {
        iPhonePage = this->page;
        imgiPhoneComicBook = this->imgComicBook;
        if (!GetOrientation())
        {
            v13 = 1;
            v10 = this->is_iPhoneComic;
            goto LABEL_29;
        }
        v10 = this->is_iPhoneComic;
    }
    v13 = 0;
LABEL_29:
    this->DrawImage(imgiPhoneComicBook[iPhonePage], this->endPoint, 0, v10, v6, v13);
    if (this->field_0x10 || this->field_0x110)
    {
        v14 = this->is_iPhoneComic;
        v15 = iPhonePage + 1;
        if (v14)
            v16 = -320;
        else
            v16 = 480;
        v23 = v16;
        if (this->is_iPhoneComic)
            v17 = 17;
        else
            v17 = 39;
        v18 = v17 - 1;
        if (v15 > v17 - 1)
            v15 = 0;
        v19 = imgiPhoneComicBook[v15];
        v20 = v15;
        if (!v19)
        {
            this->loadImage(v15, v14);
            v14 = this->is_iPhoneComic;
            v19 = imgiPhoneComicBook[v20];
        }
        this->DrawImage(v19, v23 + this->endPoint, 0, v14, v6, v13);
        if (iPhonePage - 1 >= 0) {
            v18 = iPhonePage - 1;
        }
        if (!imgiPhoneComicBook[v18])
        {
            this->loadImage(v18, this->is_iPhoneComic);
        }
        this->DrawImage(imgiPhoneComicBook[v18], this->endPoint - v23, 0, this->is_iPhoneComic, v6, v13);
    }

    if (this->drawExitButton) {
        this->DrawExitButton(graphics);
    }
}

void ComicBook::DrawLoading(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;
    int x; // r10
    int y; // r8
    int flags; // r0
    char __s[148]; // [sp+4h] [bp-94h] BYREF

    ++this->field_0x1c;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (this->is_iPhoneComic) {
        x = 160;
        y = 240;
    }
    else {
        x = 240;
        y = 160;
    }
    Text* textBuff = app->localization->getSmallBuffer();
    textBuff->setLength(0);
    switch (this->field_0x14c / 5 % 4)
    {
    case 0:
        sprintf(__s, "Loading   ");
        break;
    case 1:
        sprintf(__s, "Loading.  ");
        break;
    case 2:
        sprintf(__s, "Loading.. ");
        break;
    case 3:
        sprintf(__s, "Loading...");
        break;
    default:
        sprintf(__s, "Loading....");
        break;
    }
    textBuff->append(__s);
    textBuff->dehyphenate();
    graphics->drawString(textBuff, x, y, (this->is_iPhoneComic) ? 67 : 3);
    textBuff->dispose();
}


void ComicBook::loadImage(int index, bool vComic) {
    Applet* app = CAppContainer::getInstance()->app;
    if (vComic)
    {
        switch (index)
        {
        case 0:
            this->imgComicBook[0] = app->loadImage("ComicBook/Cover.bmp", true);
            break;
        case 1:
            this->imgComicBook[1] = app->loadImage("ComicBook/page_01.bmp", true);
            break;
        case 2:
            this->imgComicBook[2] = app->loadImage("ComicBook/page_02.bmp", true);
            break;
        case 3:
            this->imgComicBook[3] = app->loadImage("ComicBook/page_03.bmp", true);
            break;
        case 4:
            this->imgComicBook[4] = app->loadImage("ComicBook/page_04.bmp", true);
            break;
        case 5:
            this->imgComicBook[5] = app->loadImage("ComicBook/page_05.bmp", true);
            break;
        case 6:
            this->imgComicBook[6] = app->loadImage("ComicBook/page_06.bmp", true);
            break;
        case 7:
            this->imgComicBook[7] = app->loadImage("ComicBook/page_07.bmp", true);
            break;
        case 8:
            this->imgComicBook[8] = app->loadImage("ComicBook/page_08.bmp", true);
            break;
        case 9:
            this->imgComicBook[9] = app->loadImage("ComicBook/page_09.bmp", true);
            break;
        case 10:
            this->imgComicBook[10] = app->loadImage("ComicBook/page_10.bmp", true);
            break;
        case 11:
            this->imgComicBook[11] = app->loadImage("ComicBook/page_11.bmp", true);
            break;
        case 12:
            this->imgComicBook[12] = app->loadImage("ComicBook/page_12.bmp", true);
            break;
        case 13:
            this->imgComicBook[13] = app->loadImage("ComicBook/page_13.bmp", true);
        case 14:
            this->imgComicBook[14] = app->loadImage("ComicBook/page_14.bmp", true);
            break;
        case 15:
            this->imgComicBook[15] = app->loadImage("ComicBook/page_15.bmp", true);
            break;
        case 16:
            this->imgComicBook[16] = app->loadImage("ComicBook/page_16.bmp", true);
            break;
        }
    }
    else
    {
        switch (index)
        {
        case 0:
            this->imgiPhoneComicBook[0] = app->loadImage("ComicBook/frames/iPhone cover.bmp", true);
            break;
        case 1:
            this->imgiPhoneComicBook[1] = app->loadImage("ComicBook/frames/iPhone page 1a.bmp", true);
            break;
        case 2:
            this->imgiPhoneComicBook[2] = app->loadImage("ComicBook/frames/iPhone page 1b.bmp", true);
            break;
        case 3:
            this->imgiPhoneComicBook[3] = app->loadImage("ComicBook/frames/iPhone page 2a.bmp", true);
            break;
        case 4:
            this->imgiPhoneComicBook[4] = app->loadImage("ComicBook/frames/iPhone page 2b.bmp", true);
            break;
        case 5:
            this->imgiPhoneComicBook[5] = app->loadImage("ComicBook/frames/iPhone page 2c.bmp", true);
            break;
        case 6:
            this->imgiPhoneComicBook[6] = app->loadImage("ComicBook/frames/iPhone page 3a.bmp", true);
            break;
        case 7:
            this->imgiPhoneComicBook[7] = app->loadImage("ComicBook/frames/iPhone page 3b.bmp", true);
            break;
        case 8:
            this->imgiPhoneComicBook[8] = app->loadImage("ComicBook/frames/iPhone page 4a.bmp", true);
            break;
        case 9:
            this->imgiPhoneComicBook[9] = app->loadImage("ComicBook/frames/iPhone page 4b.bmp", true);
            break;
        case 10:
            this->imgiPhoneComicBook[10] = app->loadImage("ComicBook/frames/iPhone page 4c.bmp", true);
            break;
        case 11:
            this->imgiPhoneComicBook[11] = app->loadImage("ComicBook/frames/iPhone page 5a.bmp", true);
            break;
        case 12:
            this->imgiPhoneComicBook[12] = app->loadImage("ComicBook/frames/iPhone page 5b.bmp", true);
            break;
        case 13:
            this->imgiPhoneComicBook[13] = app->loadImage("ComicBook/frames/iPhone page 5c.bmp", true);
            break;
        case 14:
            this->imgiPhoneComicBook[14] = app->loadImage("ComicBook/frames/iPhone page 6a.bmp", true);
            break;
        case 15:
            this->imgiPhoneComicBook[15] = app->loadImage("ComicBook/frames/iPhone page 6b.bmp", true);
            break;
        case 16:
            this->imgiPhoneComicBook[16] = app->loadImage("ComicBook/frames/iPhone page 6c.bmp", true);
            break;
        case 17:
            this->imgiPhoneComicBook[17] = app->loadImage("ComicBook/frames/iPhone page 7a.bmp", true);
            break;
        case 18:
            this->imgiPhoneComicBook[18] = app->loadImage("ComicBook/frames/iPhone page 7b.bmp", true);
            break;
        case 19:
            this->imgiPhoneComicBook[19] = app->loadImage("ComicBook/frames/iPhone page 8a.bmp", true);
            break;
        case 20:
            this->imgiPhoneComicBook[20] = app->loadImage("ComicBook/frames/iPhone page 8b.bmp", true);
            break;
        case 21:
            this->imgiPhoneComicBook[21] = app->loadImage("ComicBook/frames/iPhone page 8c.bmp", true);
            break;
        case 22:
            this->imgiPhoneComicBook[22] = app->loadImage("ComicBook/frames/iPhone page 9.bmp", true);
            break;
        case 23:
            this->imgiPhoneComicBook[23] = app->loadImage("ComicBook/frames/iPhone page 10a.bmp", true);
            break;
        case 24:
            this->imgiPhoneComicBook[24] = app->loadImage("ComicBook/frames/iPhone page 10b.bmp", true);
            break;
        case 25:
            this->imgiPhoneComicBook[25] = app->loadImage("ComicBook/frames/iPhone page 10c.bmp", true);
            break;
        case 26:
            this->imgiPhoneComicBook[26] = app->loadImage("ComicBook/frames/iPhone page 11a.bmp", true);
            break;
        case 27:
            this->imgiPhoneComicBook[27] = app->loadImage("ComicBook/frames/iPhone page 11b.bmp", true);
            break;
        case 28:
            this->imgiPhoneComicBook[28] = app->loadImage("ComicBook/frames/iPhone page 12a.bmp", true);
            break;
        case 29:
            this->imgiPhoneComicBook[29] = app->loadImage("ComicBook/frames/iPhone page 12b.bmp", true);
            break;
        case 30:
            this->imgiPhoneComicBook[30] = app->loadImage("ComicBook/frames/iPhone page 13a.bmp", true);
            break;
        case 31:
            this->imgiPhoneComicBook[31] = app->loadImage("ComicBook/frames/iPhone page 13b.bmp", true);
            break;
        case 32:
            this->imgiPhoneComicBook[32] = app->loadImage("ComicBook/frames/iPhone page 14a.bmp", true);
            break;
        case 33:
            this->imgiPhoneComicBook[33] = app->loadImage("ComicBook/frames/iPhone page 14b.bmp", true);
            break;
        case 34:
            this->imgiPhoneComicBook[34] = app->loadImage("ComicBook/frames/iPhone page 15a.bmp", true);
            break;
        case 35:
            this->imgiPhoneComicBook[35] = app->loadImage("ComicBook/frames/iPhone page 15b.bmp", true);
            break;
        case 36:
            this->imgiPhoneComicBook[36] = app->loadImage("ComicBook/frames/iPhone page 15c.bmp", true);
            break;
        case 37:
            this->imgiPhoneComicBook[37] = app->loadImage("ComicBook/frames/iPhone page 16a.bmp", true);
            break;
        case 38:
            this->imgiPhoneComicBook[38] = app->loadImage("ComicBook/frames/iPhone page 16b.bmp", true);
            break;
        }
    }
}


void ComicBook::CheckImageExistence(Image* image) {
    uint8_t* pBmp;
    uint16_t* data;
    int texWidth, texHeight;

    if (!image->piDIB)
        return;

    if (image->texture == -1) {

        pBmp = image->piDIB->pBmp;
        image->texWidth = 1;
        image->texHeight = 1;

        while (texWidth = image->texWidth, texWidth < image->piDIB->width) {
            image->texWidth = texWidth << 1;
        }

        while (texHeight = image->texHeight, texHeight < image->piDIB->height) {
            image->texHeight = texHeight << 1;
        }

        data = (uint16_t*)malloc(sizeof(uint16_t) * texWidth * texHeight);
        image->isTransparentMask = false;
        for (int i = 0; i < image->piDIB->height; i++) {
            for (int j = 0; j < image->piDIB->width; j++) {
                int rgb = image->piDIB->pRGB565[pBmp[(image->piDIB->width * i) + j]];
                if (rgb == 0xF81F) {
                    image->isTransparentMask = true;
                }
                if (data != nullptr) {
                    data[(image->texWidth * i) + j] = rgb;
                }
            }
        }

        if (data) {
            image->CreateTexture(data, image->texWidth, image->texHeight);
            std::free(data);
        }
    }
}

void ComicBook::DrawImage(Image* image, int a3, int a4, char a5, float alpha, char a7)
{
    PFNGLACTIVETEXTUREPROC glActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");

    int v10; // r3
    float width; // s13
    float height; // s11
    float v13; // s17
    float v14; // s16
    float v15; // s14
    float v16; // s15
    float v17; // s12
    float v18; // s14
    float v19; // s13
    float v20; // s15
    float vp[12]; // [sp+4h] [bp-7Ch] BYREF
    float st[8]; // [sp+34h] [bp-4Ch] BYREF

    this->CheckImageExistence(image);
    v10 = (uint8_t)a5;
    width = (float)image->width;
    height = (float)image->height;
    if (a5)
        v10 = a4;
    if (a5)
    {
        a4 = a3;
        a3 = v10;
    }
    v13 = width * 0.5;
    v14 = height * 0.5;
    vp[2] = 0.5;
    vp[5] = 0.5;
    vp[0] = width * 0.5;
    vp[1] = -(float)(height * 0.5);
    vp[8] = 0.5;
    vp[11] = 0.5;
    vp[3] = -(float)(width * 0.5);
    vp[4] = vp[1];
    vp[6] = width * 0.5;
    vp[7] = height * 0.5;
    vp[9] = vp[3];
    vp[10] = height * 0.5;
    memset(st, 0, sizeof(st));
    v15 = 1.0 / (float)image->texWidth;
    v16 = 1.0 / (float)image->texHeight;
    v17 = width * v15;
    v18 = v15 * 0.0;
    st[0] = v17;
    st[2] = v18;
    st[4] = v17;
    st[6] = v18;
    v19 = v16 * 0.0;
    v20 = height * v16;
    st[1] = v19;
    st[3] = v19;
    st[5] = v20;
    st[7] = v20;
    if (a7)
    {
        st[0] = v18;
        st[1] = v20;
        st[2] = v17;
        st[3] = v20;
        st[4] = v18;
        st[5] = v19;
        st[6] = v17;
        st[7] = v19;
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f(1.0, 1.0, 1.0, alpha);
    glDisable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glVertexPointer(3, GL_FLOAT, 0, &vp);
    glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, &st);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    if (a5)
    {
        glTranslatef((float)(v14 + (float)a3) + (float)(240.0 - v14), v13 + (float)a4, 0.0);
        glRotatef(-90.0, 0.0, 0.0, 1.0);
    }
    else
    {
        glTranslatef(v13 + (float)a3, v14 + (float)a4, 0.0);
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPopMatrix();
}


void ComicBook::UpdateMovement() {
    int v1; // r5
    bool is_iPhoneComic; // r4
    int cur_iPhonePage; // r6
    int iPhonePage; // lr
    int curPage; // r12
    int midPoint; // r2
    int v7; // r3
    int v8; // r1
    int endPoint; // r3
    int v10; // r3
    int v11; // r2
    bool v12; // zf
    int v13; // r2
    int v14; // r3

    if (this->field_0x10)
        return;
    is_iPhoneComic = this->is_iPhoneComic;
    cur_iPhonePage = this->cur_iPhonePage;
    iPhonePage = this->iPhonePage;
    curPage = cur_iPhonePage;
    if (this->is_iPhoneComic)
    {
        iPhonePage = this->page;
        curPage = this->curPage;
    }
    else
    {
        v1 = 39;
    }
    if (this->is_iPhoneComic)
        v1 = 17;
    if (iPhonePage != curPage)
    {
        midPoint = this->midPoint;
        this->field_0x110 = 1;
        if (is_iPhoneComic)
            v7 = 5;
        else
            v7 = 8;
        v8 = std::abs(midPoint);
        if (v8 < v7)
            v8 = v7;
        if (!is_iPhoneComic)
            v8 = -v8;
        endPoint = this->endPoint;
        if (iPhonePage >= curPage)
            v10 = endPoint - v8;
        else
            v10 = v8 + endPoint;
        this->endPoint = v10;
        if (iPhonePage)
        {
            v11 = v1 - 1;
        }
        else
        {
            v11 = v1 - 1;
            if (curPage == v1 - 1)
            {
                v10 -= 2 * v8;
                goto LABEL_27;
            }
        }
        v12 = iPhonePage == v11;
        if (iPhonePage == v11)
            v12 = curPage == 0;
        if (!v12)
            goto LABEL_28;
        v10 += 2 * v8;
    LABEL_27:
        this->endPoint = v10;
    LABEL_28:
        if (is_iPhoneComic)
            v13 = 320;
        else
            v13 = 480;
        if (v13 <= v10 || v10 <= -v13)
        {
            if (is_iPhoneComic)
                this->page = this->curPage;
            else
                this->iPhonePage = cur_iPhonePage;
            this->midPoint = 0;
            this->endPoint = 0;
            this->field_0x110 = 0;
        }
    }
    if (this->field_0x118)
    {
        this->field_0x110 = 1;
        v14 = 4 * this->endPoint / 5;
        this->endPoint = v14;
        if (!v14)
        {
            this->field_0x118 = 0;
            this->field_0x110 = 0;
        }
    }
}

void ComicBook::UpdateTransition()
{
    bool v3; // zf
    int v4; // r3
    bool v5; // nf
    bool is_iPhoneComic; // r2
    float v7; // s15
    float v8; // s14
    int v9; // r3
    float v10; // s15

    v3 = !this->field_0x110;
    if (!this->field_0x110)
    {
        v3 = !this->field_0x10;
    }
    if (v3)
    {
        if (this->field_0x135)
        {
            v4 = this->field_0x138 + 1;
            v5 = this->field_0x138 - 29 < 0;
            this->field_0x138 = v4;
            
            if (!(v5 ^ (v4 | 30) | (v4 == 30)))
            {
                this->field_0x138 = 0;
                this->field_0x135 = 0;
                this->field_0x140 = 0;
                this->field_0x13c = 1;
            }
        }
        if (this->field_0x13c)
        {
            is_iPhoneComic = this->is_iPhoneComic;
            v7 = -3.0;
            if (is_iPhoneComic)
                v7 = 3.0;
            v8 = this->field_0x144;
            v9 = this->field_0x140 + 1;
            this->field_0x140 = v9;
            v10 = v7 + v8;
            if (v9 > 29)
            {
                this->is_iPhoneComic = !is_iPhoneComic;
                this->field_0x140 = 0;
                this->field_0x13c = 0;
            }
            this->drawExitButton = false;
            this->field_0x144 = v10;
        }
    }
}


void ComicBook::Touch(int x, int y, bool b)
{
    Applet* app = CAppContainer::getInstance()->app;
    bool v4; // r6
    int v7; // r5
    bool v9; // r3
    bool v10; // r2
    bool is_iPhoneComic; // r3
    int endPoint; // r1
    int v13; // r0
    int midPoint; // r12
    bool v15; // zf
    int page; // r3
    int v17; // r3
    int v18; // r3
    int curPage; // r3
    int v20; // r3
    int cur_iPhonePage; // r3
    int v22; // r3

    v4 = b;
    v7 = y;
    //printf("this->isLoaded %d\n", this->isLoaded);
    if (!this->isLoaded)
        return;

    this->curX = x;
    this->curY = y;
    if (this->ButtonTouch(x, y) && !app->sound->isSoundPlaying(1064)) {
        app->sound->playSound(1064, 0, 5, 0);
    }

    if (v4)
    {
        this->field_0x14c = 0;
    }
    else
    {
        if (this->exitBtnHighlighted)
        {
            v9 = this->field_0x174;
            this->exitBtnHighlighted = 0;
            if (!v9)
            {
                this->DeleteImages();
                app->menuSystem->back();
            }
        }
        this->field_0x174 = 0;
        //printf("this->field_0x14c %d\n", this->field_0x14c);
        if (this->field_0x14c <= 8) { // Old 4
            this->drawExitButton ^= true;
        }
    }
    v10 = this->field_0x110;
    if (this->field_0x110 || !this->field_0x10 && !v4)
        return;
    is_iPhoneComic = this->is_iPhoneComic;
    this->field_0x4 = v7;
    this->field_0x10 = v4;
    if (!is_iPhoneComic)
        v7 = x;
    this->field_0x0 = x;
    this->begPoint = v7;
    if (v4)
    {
        this->field_0x118 = v10;
        this->midPoint = v10;
        return;
    }
    endPoint = this->endPoint;
    if (is_iPhoneComic)
        v13 = 160;
    else
        v13 = 240;
    if (v13 < endPoint || (midPoint = this->midPoint, midPoint > 2))
    {
        v15 = !is_iPhoneComic;
        this->field_0x118 = v10;
        if (is_iPhoneComic)
            page = this->page;
        else
            page = this->iPhonePage;
        if (!v15)
        {
            v17 = page + 1;
        LABEL_35:
            this->curPage = v17;
            goto LABEL_41;
        }
        v18 = page - 1;
    LABEL_36:
        this->cur_iPhonePage = v18;
        goto LABEL_41;
    }
    if (endPoint < -v13 || midPoint < -2)
    {
        this->field_0x118 = v10;
        if (is_iPhoneComic)
        {
            v17 = this->page - 1;
            goto LABEL_35;
        }
        v18 = this->iPhonePage + 1;
        goto LABEL_36;
    }
    this->curPage = this->page;
    if (endPoint)
        this->field_0x118 = 1;
    this->cur_iPhonePage = this->iPhonePage;
    if (endPoint)
        this->field_0x118 = 1;
LABEL_41:
    curPage = this->curPage;
    if (curPage < 0)
    {
        v20 = 16;
    LABEL_45:
        this->curPage = v20;
        goto LABEL_46;
    }
    if (curPage > 16)
    {
        v20 = 0;
        goto LABEL_45;
    }
LABEL_46:
    cur_iPhonePage = this->cur_iPhonePage;
    if (cur_iPhonePage >= 0)
    {
        if (cur_iPhonePage <= 38)
            return;
        v22 = 0;
    }
    else
    {
        v22 = 38;
    }
    this->cur_iPhonePage = v22;
}

bool ComicBook::ButtonTouch(int x, int y)
{
    int v5; // r12
    int v6; // r2
    int v7; // lr
    int v8; // r1
    int v9; // r3
    bool v10; // cc

    this->exitBtnHighlighted = false;
    if (!this->drawExitButton)
        return false;
    v5 = this->exitBtnRect[0];
    v6 = this->exitBtnRect[1];
    v7 = this->exitBtnRect[2];
    v8 = this->exitBtnRect[3];
    if (this->is_iPhoneComic)
    {
        v9 = v5 + v7;
        v5 = this->exitBtnRect[1];
        v6 = 320 - v9;
        v8 = this->exitBtnRect[2];
        v7 = this->exitBtnRect[3];
    }
    v10 = v5 <= x;
    if (v5 <= x)
        v10 = x <= v5 + v7;
    if (!v10 || v6 > y || y > v6 + v8)
        return false;
    this->exitBtnHighlighted = true;
    return true;
}

void ComicBook::TouchMove(int x, int y)
{
    bool is_iPhoneComic; // r3
    int v7; // r3
    int v8; // r2
    int endPoint; // r3
    int begPoint; // r3

    if (this->isLoaded && !this->field_0x174)
    {
        this->ButtonTouch(x, y);
        if (!this->field_0x110 && this->field_0x10)
        {
            is_iPhoneComic = this->is_iPhoneComic;
            this->curX = x;
            this->curY = y;
            if (is_iPhoneComic)
            {
                begPoint = this->begPoint;
                this->begPoint = y;
                v8 = y - begPoint;
            }
            else
            {
                v7 = this->begPoint;
                this->begPoint = x;
                v8 = x - v7;
            }
            endPoint = this->endPoint;
            this->midPoint = v8;
            this->endPoint = v8 + endPoint;
        }
    }
}

void ComicBook::DeleteImages() {
    this->isLoaded = false;
    for (int i = 0; i < 17; i++) {
        this->imgComicBook[i]->~Image();
        this->imgComicBook[i] = nullptr;
    }
    for (int i = 0; i < 39; i++) {
        this->imgiPhoneComicBook[i]->~Image();
        this->imgiPhoneComicBook[i] = nullptr;
    }
    this->comicBookIndex = 0;
    this->iPhoneComicIndex = 0;
    this->field_0x1c = 0;
}

void ComicBook::DrawExitButton(Graphics* graphics) {
    Applet* app = CAppContainer::getInstance()->app;
    int v3; // r11
    int v4; // r10
    int v5; // r6
    int v6; // r8
    int v7; // r3
    int v8; // r5
    Text* SmallBuffer; // r5
    int v10; // r0
    char __s[152]; // [sp+14h] [bp-98h] BYREF

    v3 = this->exitBtnRect[0];
    v4 = this->exitBtnRect[1];
    v5 = this->exitBtnRect[2];
    v6 = this->exitBtnRect[3];
    if (this->is_iPhoneComic)
    {
        v7 = v3 + v5;
        v3 = this->exitBtnRect[1];
        v4 = 320 - v7;
        v6 = this->exitBtnRect[2];
        v5 = this->exitBtnRect[3];
    }
    if (this->exitBtnHighlighted) {
        v8 = 150;
    }
    else {
        v8 = 50;
    }
    graphics->fillRect(v3 - 1, v4 - 1, v5 + 2, v6 + 2, 0);
    graphics->fillRect(v3, v4, v5, v6, (v8 << 8) | (v8 << 16) | 0xFF);
    SmallBuffer = app->localization->getSmallBuffer();
    SmallBuffer->setLength(0);
    sprintf(__s, "Done", this->curPage + 1);
    SmallBuffer->append(__s);
    SmallBuffer->dehyphenate();
    graphics->drawString(SmallBuffer, v3 + v5 / 2, v4 + v6 / 2, (this->is_iPhoneComic) ? 67 : 3);
    SmallBuffer->dispose();
}


void ComicBook::handleComicBookEvents(int key, int keyAction) {
    Applet* app = CAppContainer::getInstance()->app;
    int cX = app->canvas->SCR_CX;
    int cY = app->canvas->SCR_CY;
    int i;

    printf("handleComicBookEvents %d, %d\n", key, keyAction);

    CAppContainer::getInstance()->userPressed(cX, cY);
    if (keyAction == Enums::ACTION_LEFT) {
        for (i = 0; i < 64; i++) {
            CAppContainer::getInstance()->userMoved(cX++, cY);
        }
    }
    else if (keyAction == Enums::ACTION_RIGHT) {

        for (i = 0; i < 64; i++) {
            CAppContainer::getInstance()->userMoved(cX--, cY);
        }
    }
    CAppContainer::getInstance()->userReleased(cX, cY);
    this->drawExitButton = false;
}