#include "FmodReverbZone.h"
#include <SoundSystem/SoundManager.hpp>

// --- Initialize static variables ---
std::string FmodReverbZone::DefaultSnapshotPath = "";
std::shared_ptr<FmodEventInstance> FmodReverbZone::GlobalReverbEvent = nullptr;
std::string FmodReverbZone::GlobalActivePath = "";
int FmodReverbZone::InstanceCount = 0;
FmodReverbZone* FmodReverbZone::FallbackUpdater = nullptr;

FmodReverbZone::FmodReverbZone() : AreaBase()
{
	DefaultBrushGroup = BodyType::Area2;
	DefaultBrushCollisionMask = BodyType::None;
	SaveGame = false;

	InstanceCount++;

	// Assign the first created instance as the fallback updater for the default snapshot
	if (FallbackUpdater == nullptr)
	{
		FallbackUpdater = this;
	}
}

FmodReverbZone::~FmodReverbZone()
{
	InstanceCount--;

	if (FallbackUpdater == this)
	{
		FallbackUpdater = nullptr;
	}

	// If this was the last reverb zone in the world, cleanly release the global event
	if (InstanceCount <= 0)
	{
		if (GlobalReverbEvent)
		{
			GlobalReverbEvent->Stop();
			GlobalReverbEvent = nullptr;
		}
		GlobalActivePath = "";
	}
}

void FmodReverbZone::LateUpdate()
{
	AreaBase::LateUpdate();

	// Guard against updates running during teardown
	if (InstanceCount <= 0) return;

	vec3 cameraPosition = Camera::finalizedPosition; // Use the finalized camera position for accurate reverb zone detection
	auto hitResults = Physics::PointTrace(cameraPosition, BodyType::Area2);

	int maxPriorityFound = -99999;
	FmodReverbZone* bestZone = nullptr;

	// Check all overlapping areas to find the highest priority zone
	for (auto& hitResult : hitResults)
	{
		FmodReverbZone* hitZone = dynamic_cast<FmodReverbZone*>(hitResult.entity);

		if (hitZone)
		{
			if (hitZone->priority > maxPriorityFound)
			{
				maxPriorityFound = hitZone->priority;
				bestZone = hitZone;
			}
			else if (hitZone->priority == maxPriorityFound)
			{
				// Tie-breaker using memory address to ensure all instances consistently pick the SAME winner
				if (bestZone == nullptr || hitZone > bestZone)
				{
					bestZone = hitZone;
				}
			}
		}
	}

	// 1. Determine the globally desired snapshot path for this frame
	std::string desiredPath = DefaultSnapshotPath;
	if (bestZone != nullptr)
	{
		desiredPath = bestZone->mySnapshotPath;
	}

	// 2. Handle State Change (Only fires when transitioning to a NEW path)
	// Overlapping zones sharing the same snapshot string will skip this, preventing restart glitches
	if (GlobalActivePath != desiredPath)
	{
		if (GlobalReverbEvent)
		{
			GlobalReverbEvent->Stop(); // Triggers the AHDSR Release fade in FMOD
			GlobalReverbEvent = nullptr;
		}

		GlobalActivePath = desiredPath;

		// Only create and play if the path is not empty/null
		if (!GlobalActivePath.empty())
		{
			GlobalReverbEvent = FmodEventInstance::Create(GlobalActivePath);
			if (GlobalReverbEvent)
			{
				GlobalReverbEvent->Play(); // Starts the AHDSR Attack fade in FMOD
			}
		}
	}

	// 3. Delegate the Event Update execution
	// We only want to call Update() ONCE per frame, not per instance. 
	if (GlobalReverbEvent)
	{
		// If there is a best zone, let the winner handle the update call this frame
		if (bestZone == this)
		{
			GlobalReverbEvent->Update(Time::DeltaTimeF);
		}
		// If NO zones were hit (fallback active), let our designated fallback instance update it
		else if (bestZone == nullptr && FallbackUpdater == this)
		{
			GlobalReverbEvent->Update(Time::DeltaTimeF);
		}
		// Edge case: FallbackUpdater was destroyed but another zone exists to take its place
		else if (bestZone == nullptr && FallbackUpdater == nullptr)
		{
			FallbackUpdater = this;
			GlobalReverbEvent->Update(Time::DeltaTimeF);
		}
	}
}

void FmodReverbZone::FromData(EntityData data)
{
	AreaBase::FromData(data);

	priority = data.GetPropertyFloat("priority", 1);

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Master.bank");
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Master.strings.bank");

	// Save the path to THIS specific instance instead of immediately creating an FMOD event
	mySnapshotPath = data.GetPropertyString("snapshot", "snapshot:/Reverb/Cave Reverb");
}