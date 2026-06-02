#include "AlertedInvestigationBase.h"
#include "../NpcBase.h"

void AlertedInvestigationBase::OnStart()
{
	if (owner->npcType != NpcType::Guard)
	{
		owner->report_to_guard = true;
		owner->FindClosestGuard();

		if (owner->found_guard == false)
		{
			OnNoGuardFound();
		}
	}
}

void AlertedInvestigationBase::Finish()
{
	if (owner->npcType != NpcType::Guard && owner->found_guard)
	{
		auto guardRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(owner->closestGuard));

		if (owner->currentInvestigation != nullptr)
		{
			if (guardRef)
			{
				guardRef->TryStartInvestigation(
					owner->currentInvestigation->reason,
					owner->currentInvestigation->target,
					owner->Id, true);

				owner->report_to_guard = false;
			}
		}
		else
		{
			owner->ShareTargetKnowlageWith(guardRef);
		}
	}

	InvestigationBase::Finish();
}
