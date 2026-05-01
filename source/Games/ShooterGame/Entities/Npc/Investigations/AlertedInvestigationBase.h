#pragma once

#include "InvestigationBase.h"

// Shared base for all alert-level investigations (TargetSeen, NpcInTrouble,
// WeaponFire, Body, Explosion). Handles the guard-reporting logic on start and
// the guard-sharing logic on finish so subclasses don't repeat it.
class AlertedInvestigationBase : public InvestigationBase
{
public:
	AlertedInvestigationBase(NpcBase* owner) : InvestigationBase(owner) {}

	void OnStart() override;
	void Finish() override;
	bool Alerted() override { return true; }

protected:
	// Called during OnStart when the owner is a civilian and no nearby guard
	// was found. Override to add reason-specific fallback behaviour.
	virtual void OnNoGuardFound() {}
};
