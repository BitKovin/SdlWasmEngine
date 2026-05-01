#pragma once
#include "AlertedInvestigationBase.h"

class TargetSeenInvestigation : public AlertedInvestigationBase
{
public:
	TargetSeenInvestigation(NpcBase* owner) : AlertedInvestigationBase(owner) {}
};
