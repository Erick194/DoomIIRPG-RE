#ifndef __LERPSPRITE_H__
#define __LERPSPRITE_H__

class ScriptThread;
class OutputStream;
class InputStream;

class LerpSprite
{
private:

public:
    int touchMe;
    ScriptThread* thread;
    int travelTime;
    int startTime;
    int hSprite;
    int srcX;
    int srcY;
    int srcZ;
    int dstX;
    int dstY;
    int dstZ;
    int height;
    int dist;
    int srcScale;
    int dstScale;
    int flags;

	// Constructor
	LerpSprite();
	// Destructor
	~LerpSprite();

    void saveState(OutputStream* OS);
    void calcDist();
    void loadState(InputStream* IS);
};

#endif