#include "../AreaBase.hpp"
#include <SoundSystem/FmodEventInstance.h>

class FmodReverbZone : public AreaBase
{
public:
	FmodReverbZone();

	~FmodReverbZone();

	void LateUpdate();

private:
	int priority = 0;
	bool isActive = false; // Tracks whether this snapshot is currently triggered

	std::shared_ptr<FmodEventInstance> reverbEvent = nullptr;
};