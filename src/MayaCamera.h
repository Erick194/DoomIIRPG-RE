#ifndef __MAYACAMERA_H__
#define __MAYACAMERA_H__

class ScriptThread;

class MayaCamera
{
private:

public:
	int keyOffset;
	int numKeys;
	int curTweenTime;
	int curTween;
	ScriptThread* cameraThread;
	ScriptThread* keyThread;
	int keyThreadResumeCount;
	bool complete;
	int x;
	int y;
	int z;
	int pitch;
	int yaw;
	int roll;
	short aggComponents[6];
	int sampleRate;
	short keyOfs[6];
	bool isTableCam;
	bool inheritYaw;
	bool inheritPitch;
	bool inheritX;
	bool inheritY;
	bool inheritZ;

	// Constructor
	MayaCamera();
	// Destructor
	~MayaCamera();

	void NextKey();
	void Update(int i, int i2);
	int getAngleDifference(int i, int i2);
	bool hasTweens(int i);
	int estNumTweens(int i);
	short* getTweenData(int8_t* array, int i, int i2);
	short* getKeyOfs(int16_t* array, int i);
	void Interpolate(int16_t* array, int i);
	void resetTweenBase(int i);
	void updateTweenBase(int i, int i2);
	void Render();
	void Snap(int i);
};

#endif