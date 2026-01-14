#pragma once
#include "../Entity.h"
#include "../SoundSystem/SoundInstance.hpp"
#include "../SoundSystem/SoundManager.hpp"

#include "../Delay.hpp"

class SoundPlayer: public Entity
{
public:

	shared_ptr<SoundInstanceBase> Sound = nullptr;

	vec3 Velocity = vec3();
	float Volume = 1;
	float Pitch = 1;
	float MinDistance = 1;
	float MaxDistance = 20;

	bool IsMusic = false;
	bool Is2D = false;

	bool Paused = false;

	SoundPlayer();
	~SoundPlayer();

	void SetSound(shared_ptr<SoundInstanceBase> sound);

	void LateUpdate();

	void Play();
	void Stop();


	static SoundPlayer* Create();
	static SoundPlayer* Create(string soundPath);

	static void PlayOneshot(string soundPath, float destroyAfterSeconds, float volume = 1.0f, bool UiSound = false, vec3 position = vec3(0));

private:

	float CalculateVolume();

};

#define SET_SOUND_SAFE(playerPtr, soundExpr)               \
    do {                                                  \
        shared_ptr<SoundInstanceBase> __tmpSound = (soundExpr);          \
        if ((playerPtr) != nullptr)                       \
            (playerPtr)->SetSound(__tmpSound);            \
    } while (0)
