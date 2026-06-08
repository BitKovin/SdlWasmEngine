#include "AttackedInvestigation.h"
#include "../NpcBase.h"

void AttackedInvestigation::OnStart()
{
	owner->PlayPhrace("shots_fired");
	AlertedInvestigationBase::OnStart();
}

void AttackedInvestigation::Update(float deltaTime)
{

	SearchInvestigationBase::Update(deltaTime);

	if (searchTimer > 8 && causer.empty() == false)
		causer = "";

}

void AttackedInvestigation::OnNoGuardFound()
{
	owner->UpdateTargetLocation(causer, target);
}
