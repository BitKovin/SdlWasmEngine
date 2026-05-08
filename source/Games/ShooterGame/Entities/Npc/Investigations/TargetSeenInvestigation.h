#pragma once
#include "SearchInvestigationBase.h"

class TargetSeenInvestigation : public SearchInvestigationBase
{
public:
	TargetSeenInvestigation(NpcBase* owner) : SearchInvestigationBase(owner) {}

	void OnStart() override
	{
		SearchInvestigationBase::OnStart();
		//owner->StopTargetFollow(causer);
	}

	void Finish() override
	{
		SearchInvestigationBase::Finish();
	}
protected:

	float SearchDuration() const override { return 10.0f; }
	float SearchRadius()   const override { return 13.0f; }

};
