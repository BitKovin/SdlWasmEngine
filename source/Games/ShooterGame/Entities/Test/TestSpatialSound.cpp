#include <Entities/SoundPlayer.h>
#include <SpatialSound/SpatialSoundManager.h>
#include <Input.h>
#include <Camera.h>
#include <SoundSystem/SoundInstance.hpp>
#include <SoundSystem/FmodEventInstance.h>

class TestSpatialSoundPlayer : public SoundPlayer
{
public:
	
	void Start() override 
	{
		SetSound(FmodEventInstance::Create("event:/Ambience/Forest"));
        Play();
		Sound->Loop = true;
		Sound->EnvironmentalSound = true;
        MaxDistance = 70;
	}



private:

};

REGISTER_ENTITY(TestSpatialSoundPlayer, "TestSpatialSoundPlayer")
