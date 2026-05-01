#include "NpcInTroubleInvestigation.h"
#include "../NpcBase.h"

void NpcInTroubleInvestigation::Finish()
{
	if (!causer.empty())
	{
		NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(causer));

		if (npcRef)
			npcRef->needHelpStunned = false;
	}

	AlertedInvestigationBase::Finish();
}
