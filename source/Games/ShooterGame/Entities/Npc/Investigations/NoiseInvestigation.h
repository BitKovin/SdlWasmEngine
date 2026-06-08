#pragma once
#include "InvestigationBase.h"
#include "../NpcBase.h"

class NoiseInvestigation : public InvestigationBase
{
public:
	NoiseInvestigation(NpcBase* owner) : InvestigationBase(owner) {}

	void OnStart() override
	{
		if (!sharedByNpc && !restarted)
			owner->PlayPhrace("heard_sound");
	}

	bool StopWhenSeeCauser() const override { return true; }

};
