#pragma once

#include "fmod.hpp"

class SoundResource;

class Sound
{
public:
	Sound(SoundResource* res);
	~Sound();

	void Play(bool multiple = false, Vector3 position = Vector3(), Vector3 velocity = Vector3());
	void Stop();
	bool IsPlaying();
	
	SoundResource* res;
	FMOD::Channel* channel = 0;
	FMOD::Sound* sound;
};