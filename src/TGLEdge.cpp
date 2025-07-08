#include <stdexcept>
#include <stdio.h>

#include "TGLEdge.h"
#include "TGLVert.h"

TGLEdge::TGLEdge() {
}

TGLEdge::~TGLEdge() {
}

bool TGLEdge::startup() {
	printf("TGLEdge::startup\n");

	return false;
}

void TGLEdge::setFromVerts(TGLVert* tglVert, TGLVert* tglVert2) {
    this->stopY = tglVert2->y + 7 >> 3;
    int n = tglVert2->y - tglVert->y;
    if (n != 0) {
        this->stepX = ((tglVert2->x - tglVert->x) << 16) / n;
        this->stepZ = ((tglVert2->z - tglVert->z) << 16) / n;
        this->stepS = (tglVert2->s - tglVert->s) / n;
        this->stepT = (tglVert2->t - tglVert->t) / n;
        int n2 = (8 - (uint32_t)tglVert->y) & 0x7;
        this->fracX = (tglVert->x << 16) + n2 * this->stepX;
        this->fracZ = (tglVert->z << 16) + n2 * this->stepZ;
        this->fracS = tglVert->s + n2 * this->stepS;
        this->fracT = tglVert->t + n2 * this->stepT;
        this->stepX <<= 3;
        this->stepZ <<= 3;
        this->stepS <<= 3;
        this->stepT <<= 3;
    }
    else {
        this->fracX = tglVert->x << 16;
        this->fracZ = tglVert->z << 16;
        this->fracS = tglVert->s;
        this->fracT = tglVert->t;
        this->stepX = 0;
        this->stepZ = 0;
        this->stepS = 0;
        this->stepT = 0;
    }
}
