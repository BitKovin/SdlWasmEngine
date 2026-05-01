#pragma once
#include "AlertedInvestigationBase.h"

class NpcInTroubleInvestigation : public AlertedInvestigationBase
{
public:
	NpcInTroubleInvestigation(NpcBase* owner) : AlertedInvestigationBase(owner) {}
	void Finish() override;
};
