#pragma once

#include "../AreaBase.hpp"
#include <SoundSystem/FmodEventInstance.h>
#include <string>
#include <memory>

class FmodReverbZone : public AreaBase
{
public:
	FmodReverbZone();
	~FmodReverbZone();

	void LateUpdate();
	void FromData(EntityData data) override;

	// Set this static property from outside before loading/FromData.
	// Can be left empty ("") to play no default reverb.
	static std::string DefaultSnapshotPath;

private:
	int priority = 1;
	std::string mySnapshotPath; // Path specific to this instance

	// Static members to ensure only one snapshot plays globally
	static std::shared_ptr<FmodEventInstance> GlobalReverbEvent;
	static std::string GlobalActivePath;

	// Track instances to safely clean up the static event and handle updates
	static int InstanceCount;
	static FmodReverbZone* FallbackUpdater;
};