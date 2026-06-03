#include "SoundPlayer.h"

#include "../MathHelper.hpp"
#include "../SoundSystem/FmodEventInstance.h"

SoundPlayer::SoundPlayer()
{
	LateUpdateWhenPaused = true;
}

SoundPlayer::~SoundPlayer()
{

}

void SoundPlayer::SetSound(shared_ptr<SoundInstanceBase> sound)
{
	Sound = sound;
}

void SoundPlayer::LateUpdate()
{
	if (Sound)
	{

		Sound->Position = Position;
		Sound->Direction = MathHelper::GetForwardVector(Rotation);

		Sound->Volume = Volume;
		Sound->Pitch = Pitch;
		Sound->Is2D = Is2D;
		Sound->MinDistance = MinDistance;
		Sound->MaxDistance = MaxDistance;
		Sound->Paused = Paused;

		Sound->Update(Time::DeltaTimeF);
	}

	UpdateDestroyDelay();

}

void SoundPlayer::Play()
{
	if (Sound.get())
	{
		LateUpdate();
		Sound->Stop();
		Sound->Play();
	}
}

float SoundPlayer::CalculateVolume()
{
	return Volume * SoundManager::GlobalVolume * (IsMusic ? SoundManager::MusicVolume : SoundManager::SfxVolume);
}


void SoundPlayer::Stop()
{
	if (Sound)
	{
		Sound->Stop();
	}
}

SoundPlayer* SoundPlayer::Create()
{

	SoundPlayer* player = new SoundPlayer();

	Level::Current->AddEntity(player);
	player->Start();

	return player;

}

SoundPlayer* SoundPlayer::Create(string soundPath)
{
	auto player = Create();

	if (soundPath.starts_with("event:"))
	{
		player->SetSound(FmodEventInstance::Create(soundPath));
	}
	else
	{
		player->SetSound(SoundManager::GetSoundFromPath(soundPath));
	}

	

	return player;
}

void SoundPlayer::PlayOneshot(string soundPath,float destroyAfterSeconds,float volume, bool UiSound, vec3 position)
{

	auto player = Create(soundPath);
	player->Volume = volume;
	player->Position = position;

	if(player->Sound)
		player->Sound->IsUISound = UiSound;

	player->DestroyWithDelay(destroyAfterSeconds);

	player->Play();

}
