#include "FmodReverbZone.h"

#include <SoundSystem/SoundManager.hpp>

FmodReverbZone::FmodReverbZone() : AreaBase()
{

	DefaultBrushGroup = BodyType::Area2;
	DefaultBrushCollisionMask = BodyType::None;

	SaveGame = false;


	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Master.bank");
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Master.strings.bank");

	reverbEvent = FmodEventInstance::Create("snapshot:/Reverb/Cave Reverb");


}

FmodReverbZone::~FmodReverbZone()
{
	if (reverbEvent)
	{
		reverbEvent->Stop();
		reverbEvent = nullptr;
	}
}

void FmodReverbZone::LateUpdate()
{

	AreaBase::LateUpdate();

	vec3 cameraPosition = Camera::finalizedPosition; // Use the finalized camera position for accurate reverb zone detection

	auto hitResults = Physics::PointTrace(cameraPosition, BodyType::Area2);

	bool isCameraInside = false;
	int maxPriorityFound = -99999; // Start arbitrarily low

	// Check all overlapping areas to find the highest priority
	for (auto& hitResult : hitResults)
	{
		// Note: Adjust 'hitResult.entity' to whatever your engine uses to store the hit actor/object
		FmodReverbZone* hitZone = dynamic_cast<FmodReverbZone*>(hitResult.entity);

		if (hitZone)
		{
			if (hitZone == this)
			{
				isCameraInside = true;
			}

			if (hitZone->priority > maxPriorityFound)
			{
				maxPriorityFound = hitZone->priority;
			}
		}
	}

	// This one will be selected if the camera is inside it AND no other overlapping zone has a higher priority
	bool thisSelected = (isCameraInside && priority >= maxPriorityFound);

	// Handle Play/Stop state changes
	if (thisSelected && !isActive)
	{
		reverbEvent->Play(); // Starts the AHDSR Attack fade in FMOD
		isActive = true;
	}
	else if (!thisSelected && isActive)
	{
		reverbEvent->Stop(); // Triggers the AHDSR Release fade in FMOD
		isActive = false;
	}

	if (reverbEvent)
	{
		reverbEvent->Update(Time::DeltaTimeF);
	}

}
