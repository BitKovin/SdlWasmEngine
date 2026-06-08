#pragma once
#include "InvestigationBase.h"
#include "../NpcBase.h"

class LookAtInvestigation : public InvestigationBase
{
public:
	LookAtInvestigation(NpcBase* owner) : InvestigationBase(owner)
	{
		StayInPlaceAfterFinishTime = 2.0f;
	}

	void OnStart() override
	{
		if (!sharedByNpc && !restarted)
			owner->PlayPhrace("");
	}

	void Update(float deltaTime) override
	{
		InvestigationBase::Update(deltaTime);

		if (orientTimer < 0)
		{
			Finish();
		}
	}

};
