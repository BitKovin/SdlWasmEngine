#include "BodyInvestigation.h"
#include "../NpcBase.h"

void BodyInvestigation::OnNoGuardFound()
{
	owner->currentInvestigation->Finish();
}

void BodyInvestigation::Finish()
{
	// Guards always react to a found body.
	// Civilians only react when there is no guard to report to.
	if (owner->isGuard || !owner->found_guard)
		InvestigateCauserBody();

	AlertedInvestigationBase::Finish();
}

void BodyInvestigation::InvestigateCauserBody()
{
	if (causer.empty())
		return;

	NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(causer));

	if (npcRef)
	{
		npcRef->BodyInvestigated();
		owner->PlayPhrace("dead_body");
	}
}
