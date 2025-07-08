#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "Image.h"

/*
typedef unsigned char   uint8_t;
typedef unsigned long DWORD_PTR;
#define BYTE		uint8_t
#define WORD		uint16_t
#define LOBYTE(w)	((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)	((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#define LOWORD(l)   ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)   ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define BYTEn(x, n)	(*((BYTE*)&(x)+n))
#define WORDn(x, n)	(*((WORD*)&(x)+n))
#define BYTE0(x)	BYTEn(x,  0)         // byte 0 (counting from 0)
#define BYTE1(x)	BYTEn(x,  1)         // byte 1 (counting from 0)
#define BYTE2(x)	BYTEn(x,  2)
#define BYTE3(x)	BYTEn(x,  3)
#define BYTE4(x)	BYTEn(x,  4)
#define WORD1(x)   WORDn(x,  1)
#define WORD2(x)   WORDn(x,  2)         // third word of the object, unsigned
#define WORD3(x)   WORDn(x,  3)
#define WORD4(x)   WORDn(x,  4)
#define WORD5(x)   WORDn(x,  5)
#define WORD6(x)   WORDn(x,  6)
#define WORD7(x)   WORDn(x,  7)
*/


bool getFileMD5Hash(void* data, uint64_t numBytes, uint64_t& hashWord1, uint64_t& hashWord2);
bool checkFileMD5Hash(const void* data, uint64_t numBytes, uint64_t checkHashWord1, uint64_t checkHashWord2);
bool pointInRectangle(int x, int y, int rectX, int rectY, int rectW, int rectH);
float AxisHit(int aX, int aY, int x, int y, int w, int h, bool isXaxis, float activeFraction);
void fixImage(Image* img);
void enlargeButtonImage(Image* img);

#endif
