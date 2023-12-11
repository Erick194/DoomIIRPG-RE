#ifndef __TGLVERT_H__
#define __TGLVERT_H__

class TGLVert
{
private:

public:
	static constexpr int MV_TC_ONE = 2048;

	int x;
	int y;
	int z;
	int w;
	int s;
	int t;
	int clipDist;
};

#endif