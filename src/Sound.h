#ifndef __SOUND_H__
#define __SOUND_H__
#include <al.h>
#include <alc.h>

typedef uint32_t AudioFormatID;
typedef uint32_t AudioFormatFlags;
typedef uint32_t AudioFileID;
struct AudioStreamBasicDescription
{
	uint32_t mSampleRate;
	AudioFormatID mFormatID;
	AudioFormatFlags mFormatFlags;
	uint32_t mBytesPerPacket;
	uint32_t mFramesPerPacket;
	uint32_t mBytesPerFrame;
	uint32_t mChannelsPerFrame;
	uint32_t mBitsPerChannel;
	uint32_t mReserved;
};

class Applet;


class Sound
{
private:

public:

	class Sound::SoundStream
	{
	public:
		ALuint bufferId;
		ALuint sourceId;
		__int16 resID;
		__int16 priority;
		bool fadeInProgress;
		uint8_t field_0xd;
		uint8_t field_0xe;
		uint8_t field_0xf;
		int volume;
		int fadeBeg;
		int fadetime;
		float fadeVolume;

		void StartFade(int volume, int fadeBeg, int fadeEnd);
	};

	Applet* app;
	bool field_0x4;
	bool allowSounds;
	bool allowMusics;
	int soundFxVolume;
	int musicVolume;
	int field_0x10;
	SoundStream channel[10];
	short resID;
	uint8_t flags;
	int priority;
	int field_0x15c;
	short field_0x160;
	uint8_t field_0x162;
	bool soundsLoaded;
	ALCcontext* alContext;
	ALCdevice* alDevice;

	// Constructor
	Sound();
	// Destructor
	~Sound();

	bool startup();
	void openAL_Init();
	void openAL_Close();
	void openAL_SetSystemVolume(int volume);
	void openAL_SetVolume(ALuint source, int volume);
	void openAL_Suspend();
	void openAL_Resume();
	bool openAL_IsPlaying(ALuint source);
	bool openAL_IsPaused(ALuint source);
	bool openAL_GetALFormat(AudioStreamBasicDescription aStreamBD, ALenum* format);
	void openAL_PlaySound(ALuint source, ALint loop);
	void openAL_LoadSound(int resID, Sound::SoundStream* channel);
	bool openAL_LoadWAVFromFile(ALuint bufferId, const char* fileName);
	bool openAL_LoadAudioFileData(const char* fileName, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq);
	bool openAL_OpenAudioFile(const char* fileName, InputStream* IS);
	bool openAL_LoadAllSounds();

	bool cacheSounds();
	void playSound(__int16 resID, unsigned __int8 flags, int priority, bool a5);
	int getFreeSlot(int a2);
	void soundStop();
	void stopSound(int resID, bool fadeOut);
	bool isSoundPlaying(__int16 resID);
	void updateVolume();
	void playCombatSound(__int16 resID, unsigned __int8 flags, int priority);
	bool cacheCombatSound(int resID);
	void freeMonsterSounds();

	void volumeUp(int volume);
	void volumeDown(int volume);
	void startFrame();
	void endFrame();
	void musicVolumeUp(int volume); // [GEC]
	void musicVolumeDown(int volume); // [GEC]
};

#endif