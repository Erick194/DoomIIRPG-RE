#ifndef __GAMESPRITE_H__
#define __GAMESPRITE_H__

class Entity;

class GameSprite
{
private:

public:
	static constexpr int FLAG_INUSE = 1;
	static constexpr int FLAG_PROPOGATE = 2;
	static constexpr int FLAG_NORELINK = 4;
	static constexpr int FLAG_NOREFRESH = 8;
	static constexpr int FLAG_ANIM = 64;
	static constexpr int FLAG_NOEXPIRE = 512;
	static constexpr int FLAG_SCALE = 1024;
	static constexpr int FLAG_UNLINK = 2048;
	static constexpr int FLAG_FACEPLAYER = 4096;
	static constexpr int FLAG_LERP_ENT = 8192;
	static constexpr int FLAG_LERP_PARABOLA = 16384;
	static constexpr int FLAG_LERP_TRUNC = 32768;

	int touchMe;
	int time;
	int flags;
	short pos[6];
	int sprite;
	Entity* data;
	uint8_t startScale;
	uint8_t destScale;
	int duration;
	int vel[3];
	int scaleStep;
	uint8_t numAnimFrames;
};

#endif