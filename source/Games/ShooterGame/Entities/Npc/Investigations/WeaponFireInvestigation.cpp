#include "WeaponFireInvestigation.h"
#include "../NpcBase.h"

void WeaponFireInvestigation::OnStart()
{
	owner->PlayPhrace("shots_fired");
	AlertedInvestigationBase::OnStart();
}

void WeaponFireInvestigation::Update(float deltaTime)
{

	SearchInvestigationBase::Update(deltaTime);

	if (searchTimer > 5 && causer.empty() == false)
		causer = "";

}

void WeaponFireInvestigation::OnNoGuardFound()
{
	owner->UpdateTargetLocation(causer, target);
}
