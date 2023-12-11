#include <stdexcept>
#include <al.h>
#include <alc.h>

#include "App.h"
#include "CAppContainer.h"
#include "Sound.h"
#include "Sounds.h"
#include "JavaStream.h"

Sound::Sound() {
	std::memset(this, 0, sizeof(Sound));
}

Sound::~Sound() {
	this->soundStop();
	this->openAL_Close();
}

#define OpenAL_ERROR(id)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		printf("OpenAL error: %s, file: %s [%d]\n", alGetString(error) , "/Users/greghodges/doom2rpg/trunk/Doom2rpg_iphone/xcode/Classes/Sound.cpp", id);	\
	}

bool Sound::startup() {
	Applet* app = CAppContainer::getInstance()->app;
	printf("Sound::startup\n");

	this->field_0x162 = 0;
	this->app = app;
	this->allowSounds = true;
	this->allowMusics = true;
	this->soundFxVolume = 80;
	this->musicVolume = 80;
	this->field_0x10 = 100;
	this->field_0x4 = 0;
	this->field_0x160 = -1;
	this->soundsLoaded = false;
	this->alContext = nullptr;

	for (int i = 0; i < 10; i++) {
		this->channel[i].resID = -1;
		this->channel[i].priority = 1;
	}
	this->openAL_Init();

	return true;
}

void Sound::openAL_Init() {
	ALenum error;
	printf("Sound::openAL_Init\n");

	alGetError();
	if (alcGetCurrentContext()) {
		puts("WARNING! Trying to init OpenAL, but we already have a context!");
	}
	else {
		this->alDevice = alcOpenDevice(nullptr);
		OpenAL_ERROR(522);

		if (this->alDevice) {
			ALCcontext* context = alcCreateContext(this->alDevice, nullptr);
			OpenAL_ERROR(527);

			if (context) {
				alcMakeContextCurrent(context);
				this->alContext = context;
			}
			else {
				alcCloseDevice(this->alDevice);
			}
		}
		OpenAL_ERROR(540);

		//this->openAL_SetSystemVolume(100);
		alListener3i(AL_POSITION, 0, 0, 0);
		alListener3i(AL_VELOCITY, 0, 0, 0);
		OpenAL_ERROR(546);
	}
}

void Sound::openAL_Close() {
	for (int i = 0; i < 10; i++) {
		alDeleteBuffers(1, &this->channel[i].bufferId);
		alDeleteSources(1, &this->channel[i].sourceId);
	}

	alcMakeContextCurrent(nullptr);
	if (this->alContext) {
		alcDestroyContext(this->alContext);
		this->alContext = nullptr;
	}
	if (this->alDevice) {
		alcCloseDevice(this->alDevice);
		this->alDevice = nullptr;
	}
}

void Sound::openAL_SetSystemVolume(int volume) {
	if (volume > 100) {
		volume = 100;
	}
	alListenerf(AL_GAIN, (float)volume / 100.0f);
}

void Sound::openAL_SetVolume(ALuint source, int volume) {
	if (volume > 100) {
		volume = 100;
	}
	alSourcef(source, AL_MAX_GAIN, (float)volume / 100.0);
}

void Sound::openAL_Suspend() {
	if (this->alContext) {
		alcMakeContextCurrent(nullptr);
		alcSuspendContext(this->alContext);
	}
}

void Sound::openAL_Resume() {
	if (this->alContext) {
		alcMakeContextCurrent(this->alContext);
		alcProcessContext(this->alContext);
	}
}

bool Sound::openAL_IsPlaying(ALuint source) {
	ALenum error;
	ALint source_state;
	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	OpenAL_ERROR(685);
	return (source_state == AL_PLAYING) ? true : false;
}

bool Sound::openAL_IsPaused(ALuint source) {
	ALenum error;
	ALint source_state;
	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	OpenAL_ERROR(694);
	return (source_state == AL_PAUSED) ? true : false;
}

bool Sound::openAL_GetALFormat(AudioStreamBasicDescription aStreamBD, ALenum* format) {
	if ((aStreamBD.mFormatID == 'lpcm') && (aStreamBD.mChannelsPerFrame - 1U < 2)) {
		if (aStreamBD.mBitsPerChannel == 8) {
			if (aStreamBD.mChannelsPerFrame == 1) {
				*format = AL_FORMAT_MONO8;
			}
			else {
				*format = AL_FORMAT_STEREO8;
			}
			return true;
		}
		if (aStreamBD.mBitsPerChannel == 16) {
			if (aStreamBD.mChannelsPerFrame == 1) {
				*format = AL_FORMAT_MONO16;
			}
			else {
				*format = AL_FORMAT_STEREO16;
			}
			return true;
		}
	}
	return false;
}

void Sound::openAL_PlaySound(ALuint source, ALint loop) {
	ALenum error;
	alSourcei(source, AL_LOOPING, (loop != 0) ? AL_TRUE : AL_FALSE);
	OpenAL_ERROR(708);
	alSourcePlay(source);
	OpenAL_ERROR(711);
}

void Sound::openAL_LoadSound(int resID, Sound::SoundStream* channel) {
	ALenum error;
	printf("openAL_LoadSound... resID: %d buffer: %d\n", resID, channel->bufferId);
	int index = (unsigned __int16)(resID - 1000);
	OpenAL_ERROR(596);

	printf("Loading sound...\nName: %s\nResourceID: %d\nAL buffer: %d\n", Sounds::RESOURCE_FILE_NAMES[index], resID, channel->bufferId);
	if (this->openAL_LoadWAVFromFile(channel->bufferId, Sounds::RESOURCE_FILE_NAMES[index])) {
		OpenAL_ERROR(606);
	}
	else {
		this->app->Error("Sound resource not found\n!");
	}
}

bool Sound::openAL_LoadWAVFromFile(ALuint bufferId, const char* fileName) {
	ALenum error;
	ALsizei freq;
	ALvoid* data;
	ALsizei size;
	ALenum format;

	data = nullptr;
	printf("Loading wav: %s\n", fileName);
	if (this->openAL_LoadAudioFileData(fileName, &format, &data, &size, &freq)) {
		alBufferData(bufferId, format, data, size, freq);
		std::free(data);
		OpenAL_ERROR(939);
		return true;
	}
	return false;
}

bool Sound::openAL_LoadAudioFileData(const char* fileName, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq) {
	InputStream IS;
	AudioStreamBasicDescription outPropertyData;
	char buffer[4];

	if (this->openAL_OpenAudioFile(fileName, &IS)) {
		*(int*)buffer = IS.readInt();
		if (strncmp(buffer, "RIFF", 4) != 0) {
			this->app->Error("Not a valid WAV file, RIFF not found in header\n!");
		}

		IS.readInt(); // size of file

		*(int*)buffer = IS.readInt();
		if (strncmp(buffer, "WAVE", 4) != 0) {
			this->app->Error("Not a valid WAV file, WAVE not found in header\n!");
		}

		*(int*)buffer = IS.readInt();
		if (strncmp(buffer, "fmt ", 4) != 0) {
			this->app->Error("Not a valid WAV file, Format Marker not found in header\n!");
		}

		int chunklength = IS.readInt();
		if ((chunklength != 16) && (chunklength != 18)) {
			this->app->Error("Not a valid WAV file, format length wrong in header\n!");
		}

		if (IS.readShort() != 1) {
			this->app->Error("Not a valid WAV file, file not in PCM format\n!");
		}

		outPropertyData.mFormatID = 'lpcm';
		outPropertyData.mChannelsPerFrame = IS.readShort(); // Get number of channels. 
		outPropertyData.mSampleRate = IS.readInt(); // Get sampler rate. 
		IS.readInt(); // Block Align
		IS.readShort(); // ByteRate
		outPropertyData.mBitsPerChannel = IS.readShort(); // Get Bits Per Sample.
		if (chunklength == 18) { // SpiderMastermind_sight.wav use chunklength 18
			IS.readShort();
		}
		IS.readInt(); // "data" chunk. 
		*size = IS.readInt(); // Get size of the data.

		*data = (ALvoid*)std::malloc(*size);
		IS.read((uint8_t*)*data, 0, *size); // Read audio data into buffer.

		IS.close();
		IS.~InputStream();

		if (!this->openAL_GetALFormat(outPropertyData, format)) {
			this->app->Error("openAL: Error formatting");
		}

		*freq = (int)outPropertyData.mSampleRate;
		return true;
	}
	IS.~InputStream();
	return false;
}

bool Sound::openAL_OpenAudioFile(const char* fileName, InputStream* IS) {
	if (!IS->loadFile(fileName, LT_SOUND_RESOURCE)) {
		this->app->Error("Failed to open audio: %s", fileName);
		return false;
	}

	return true;
}

bool Sound::openAL_LoadAllSounds() {
	ALenum error;
	if (this->soundsLoaded == false) {
		for (int i = 0; i < 10; i++) {
			alGenBuffers(1, &this->channel[i].bufferId);
			alGenSources(1, &this->channel[i].sourceId);
			alSource3i(this->channel[i].sourceId, AL_POSITION, 0, 0, 0);
			alSourcei(this->channel[i].sourceId, AL_REFERENCE_DISTANCE, 5000000);
		}
		OpenAL_ERROR(643);
		this->soundsLoaded = true;
		return true;
	}

	return false;
}


bool Sound::cacheSounds() {
	if (this->openAL_LoadAllSounds()) {
		this->updateVolume();
		return true;
	}
	return false;
}

void Sound::playSound(__int16 resID, unsigned __int8 flags, int priority, bool a5) {
	ALenum error;

	int v5; // r5
	bool v7; // zf
	bool v9; // zf
	int v10; // r10
	int v12; // r6
	int v14; // r1
	int v15; // r2
	int v17; // r6
	int musicVolume; // r2
	ALenum v20; // r0
	const ALchar* v21; // r0
	int soundFxVolume; // r2
	ALenum Error; // r0
	const ALchar* String; // r0
	int v26; // r2
	ALuint v28; // r1
	int FreeSlot; // [sp+8h] [bp-1Ch]

	SoundStream* channel;

	v5 = resID;
	v7 = resID == -1;
	if (resID != -1)
		v7 = resID == 255;

	if (!v7)
	{
		v9 = resID == 1255;
		if (resID != 1255)
			v9 = this->field_0x10 == 0;
		if (!v9 && !this->field_0x4)
		{
			if ((unsigned int)(resID - 1067) <= 4)
			{
				if (!this->allowMusics /* || isUserMusicOn()*/)
					return;
			}
			else if (!this->allowSounds)
			{
				return;
			}
			if (this->resID == v5)
				return;
			FreeSlot = -1;
			for (v12 = 0; v12 < 10; v12++) {
				if (this->channel[v12].resID == v5)
				{
					v14 = 0;
					while (this->channel[v14].resID != v5)
					{
						++v14;
						if (v14 == 10)
							goto LABEL_19;
					}
					if (!this->openAL_IsPlaying(this->channel[v14].sourceId))
					{
					LABEL_19:
						v10 = v12;
						goto LABEL_29;
					}
					if (priority == 6)
					{
						v10 = v12;
						if (v12 != -1 && this->openAL_IsPlaying(this->channel[v12].sourceId))
							return;
					}
				}

				if (FreeSlot == -1) {
					if (this->channel[v12].resID == -1) {
						FreeSlot = v12;
					}
					else {
						FreeSlot = -1;
					}
				}
			}


			v10 = -1;
		LABEL_29:
			if (priority == 6 || (flags & 1) != 0)
			{
			LABEL_31:
				if (v10 != -1 && this->openAL_IsPlaying(this->channel[v10].sourceId))
					return;
			}
			if ((flags & 2) != 0)
			{
				for (v17 = 0; v17 < 10; v17++) {
					alSourceStop(this->channel[v17].sourceId);
					this->channel[v17].priority = 1;
					this->channel[v17].fadeInProgress = 0;
				}
			}


			if (v10 == -1)
			{
				if (FreeSlot == -1)
				{
					FreeSlot = this->getFreeSlot(priority);
					if (FreeSlot == -1)
						return;
				}
				channel = &this->channel[FreeSlot];
				channel->resID = resID;
				channel->priority = priority;
				alSourceStop(channel->sourceId);
				alSourcei(channel->sourceId, AL_BUFFER, NULL);
				this->openAL_LoadSound(v5, &this->channel[FreeSlot]);
				alSourcei(channel->sourceId, AL_BUFFER, channel->bufferId);
				if ((unsigned int)(channel->resID - 1067) > 4)
					soundFxVolume = this->soundFxVolume;
				else
					soundFxVolume = this->musicVolume;
				this->openAL_SetVolume(channel->sourceId, soundFxVolume);
				OpenAL_ERROR(258);
			}
			else
			{
				channel = &this->channel[v10];
				if ((unsigned int)(channel->resID - 1067) > 4)
					musicVolume = this->soundFxVolume;
				else
					musicVolume = this->musicVolume;
				this->openAL_SetVolume(channel->sourceId, musicVolume);
				OpenAL_ERROR(214);
			}

			this->openAL_PlaySound(channel->sourceId, (flags & 1) ? 1 : 0);
		}
	}
}


int Sound::getFreeSlot(int a2) {
	int v4; // r5
	int v6; // r11
	int v7; // r10
	int priority; // r3
	bool v9; // cc

	v4 = 0;
	v6 = -1;
	v7 = 6;
	while (this->channel[v4].resID != -1
		&& (this->openAL_IsPlaying(this->channel[v4].sourceId)
			|| this->openAL_IsPaused(this->channel[v4].sourceId)))
	{
		priority = this->channel[v4].priority;
		v9 = priority < a2;
		if (priority < a2)
			v9 = priority < v7;
		if (v9)
			v6 = v4;
		++v4;
		if (v9)
			v7 = priority;
		if (v4 == 10)
			return v6;
	}
	return v4;
}

void Sound::soundStop() {
	for (int i = 0; i < 10; i++) {
		alSourceStop(this->channel[i].sourceId);
		this->channel[i].priority = 1;
		this->channel[i].fadeInProgress = false;
	}
}

void Sound::stopSound(int resID, bool fadeOut) {
	int volume;
	for (int i = 0; i < 10; i++) {
		if (this->channel[i].resID == resID) {
			if (fadeOut) {
				if (!this->channel[i].fadeInProgress)
				{
					if ((unsigned int)((__int16)resID - 1067) > 4)
						volume = this->soundFxVolume;
					else
						volume = this->musicVolume;
					this->channel[i].StartFade(volume, 0, 500);
				}
			}
			else {
				alSourceStop(this->channel[i].sourceId);
				this->channel[i].priority = 1;
				this->channel[i].fadeInProgress = false;
			}
			return;
		}
	}
}

bool Sound::isSoundPlaying(__int16 resID) {
	for (int i = 0; i < 10; i++) {
		if (this->channel[i].resID == resID) {
			return this->openAL_IsPlaying(this->channel[i].sourceId);
		}
	}
	return false;
}

void Sound::updateVolume() {
	int volume;
	if (this->soundsLoaded) {
		for (int i = 0; i < 10; i++) {
			if (!this->channel[i].fadeInProgress) {
				if ((unsigned int)(this->channel[i].resID - 1067) > 4) {
					volume = this->soundFxVolume;
				}
				else {
					volume = this->musicVolume;
				}
				this->openAL_SetVolume(this->channel[i].sourceId, volume);
			}
		}

		this->allowMusics = (this->musicVolume != 0) ? true : false;
		this->allowSounds = (this->soundFxVolume != 0) ? true : false;
	}
}

void Sound::playCombatSound(__int16 resID, unsigned __int8 flags, int priority) {
	this->playSound(resID, flags, priority, false);
}

bool Sound::cacheCombatSound(int resID) {
	// no implementado
	return true;
}
void Sound::freeMonsterSounds() {
	// no implementado
}

void Sound::volumeUp(int volume) {
	this->soundFxVolume += volume;
	if (this->soundFxVolume > 100) {
		this->soundFxVolume = 100;
	}
	this->updateVolume();
}

void Sound::volumeDown(int volume) {
	this->soundFxVolume -= volume;
	if (this->soundFxVolume < 0) {
		this->soundFxVolume = 0;
	}
	this->updateVolume();
}

void Sound::startFrame() {
	if (this->field_0x162 != 0) {
		if (this->field_0x4 == false) {
			this->playSound(this->resID, this->flags, this->priority, this->field_0x4);
			this->field_0x160 = -1;
			this->resID = -1;
			this->flags = 0;
			this->priority = 0;
			this->field_0x15c = -1;
			this->field_0x162 = 0;
		}
	}
}

void Sound::endFrame() {
	// no implementado
}

void Sound::SoundStream::StartFade(int volume, int fadeBeg, int fadeEnd) {
	this->fadeBeg = fadeBeg;
	this->fadeInProgress = true;
	this->volume = volume;
	this->fadetime = CAppContainer::getInstance()->app->gameTime;
	this->fadeVolume = (float)(fadeBeg - volume) / (float)fadeEnd;
}

void Sound::musicVolumeUp(int volume) { // [GEC]
	this->musicVolume += volume;
	if (this->musicVolume > 100) {
		this->musicVolume = 100;
	}
	this->updateVolume();
}

void Sound::musicVolumeDown(int volume) { // [GEC]
	this->musicVolume -= volume;
	if (this->musicVolume < 0) {
		this->musicVolume = 0;
	}
	this->updateVolume();
}