#include "stdafx.h"
#include "ResourceManager.h"
#include "Sound.h"

Sound::Sound(SoundResource* res)
{
	this->res = res;
	FMOD::System* s = ResourceManager::instance->GetSoundSystem();
	s->createSound(res->path.c_str(), res->is3d ? FMOD_3D : FMOD_DEFAULT, 0, &sound);
	sound->setMode(res->loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	if (res->is3d)
	{
		sound->set3DMinMaxDistance(res->distances3d.x, res->distances3d.y);
	}
}

Sound::~Sound()
{
	sound->release();
}

bool Sound::IsPlaying()
{
	if (channel == 0)
		return false;
	bool p;
	channel->isPlaying(&p);
	return p;
}

void Sound::Play(bool multiple, Vector3 position, Vector3 velocity)
{
	if (!multiple && IsPlaying())
		return;
	FMOD::System* s = ResourceManager::instance->GetSoundSystem();
	s->playSound(sound, 0, false, &channel);
	if (res->is3d)
	{
		FMOD_VECTOR pos, vel;
		pos.x = position.x;
		pos.y = position.y;
		pos.z = position.z;
		vel.x = velocity.x;
		vel.y = velocity.y;
		vel.z = velocity.z;
		channel->set3DAttributes(&pos, &vel);
	}
}

void Sound::Stop()
{
	channel->stop();
}