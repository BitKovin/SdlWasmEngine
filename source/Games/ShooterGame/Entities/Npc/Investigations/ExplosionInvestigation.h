#pragma once
#include "SearchInvestigationBase.h"

class ExplosionInvestigation : public SearchInvestigationBase
{
public:
	ExplosionInvestigation(NpcBase* owner) : SearchInvestigationBase(owner) {}

protected:
	float SearchDuration() const override { return 30.0f; }
	float SearchRadius()   const override { return 10.0f; }
	float WaitChance()     const override { return 0.4f; }
};
