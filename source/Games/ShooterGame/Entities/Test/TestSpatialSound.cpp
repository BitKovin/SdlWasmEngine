#include <Entities/SoundPlayer.h>
#include <SpatialSound/SpatialSoundManager.h>
#include <Input.h>
#include <Camera.h>
#include <SoundSystem/SoundInstance.hpp>

class TestSpatialSoundPlayer : public SoundPlayer
{
public:
	
	void Start() override 
	{
		SetSound(SoundManager::GetSoundFromPath("GameData/sounds/mew.wav"));
        Play();
		Sound->Loop = true;
        MaxDistance = 70;
	}

	void Update() override
	{


	}

    void LateUpdate() override{}

private:

};

REGISTER_ENTITY(TestSpatialSoundPlayer, "TestSpatialSoundPlayer")
