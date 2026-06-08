#pragma once
#include "InvestigationBase.h"
#include "../NpcBase.h"

class LoudNoiseInvestigation : public InvestigationBase
{
public:
	LoudNoiseInvestigation(NpcBase* owner) : InvestigationBase(owner) {}

	void OnStart() override
	{
		if (!sharedByNpc && !restarted)
			owner->PlayPhrace("heard_sound");
	}
};
