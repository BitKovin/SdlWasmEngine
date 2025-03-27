#ifndef SOUND_H
#define SOUND_H

#include <AL/al.h>
#include <AL/alc.h>

#include "../EObject.hpp"

class SoundInstance : public EObject
{

	ALuint sourceRef = -1;

public:

	void SetSoundRef(ALint ref)
	{
		sourceRef = ref;
	}

	void Update()
	{
		if (sourceRef < 0)
			return;

		alSourcef(sourceRef, AL_GAIN, Volume);
		alSourcei(sourceRef, AL_LOOPING, Loop);
	}

	void Play()
	{
		if (sourceRef < 0)
			return;

		Update();

		alSourcePlay(sourceRef);
	}

	float Volume = 1;
	bool Loop = false;

protected:

	void OnDispose()
	{
		alDeleteSources(1, &sourceRef);
	}

};

#endif