#include "InvestigationBase.h"
#include "../NpcBase.h"

void InvestigationBase::Start(InvestigationReason reason, vec3 target, std::string causer, bool sharedByNpc)
{
	this->owner = owner;
	this->target = target;
	this->causer = causer;
	this->sharedByNpc = sharedByNpc;
	this->reason = reason;

	OnStart();

}

void InvestigationBase::OnStart()
{

	if (reason == InvestigationReason::LoudNoise || reason == InvestigationReason::Noise)
	{

		if (sharedByNpc == false)
			owner->PlayPhrace("heard_sound");
	}

	if (reason == InvestigationReason::WeaponFire)
	{
		owner->PlayPhrace("shots_fired");
	}

	if (reason < InvestigationReason::LoudNoise && owner->isGuard == false)
	{
		owner->report_to_guard = true;

		owner->FindClosestGuard();

		if (owner->found_guard == false)
		{

			if (reason == InvestigationReason::Body)
			{
				owner->currentInvestigation->Finish();
			}

			else if (reason == InvestigationReason::WeaponFire)
			{
				owner->UpdateTargetLocation(causer, target);
			}


		}


	}

	TargetLocation = target;


}

void InvestigationBase::Update(float deltaTime)
{}

void InvestigationBase::ReachedTarget()
{

	Finish();

}

bool InvestigationBase::Alerted()
{
	return reason < InvestigationReason::LoudNoise;
}

void InvestigationBase::Finish()
{

	if (reason == InvestigationReason::NpcInTrouble)
	{
		if (causer.empty() == false)
		{
			NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(causer));

			if (npcRef)
			{
				npcRef->needHelpStunned = false;
			}

		}
	}

	if (owner->isGuard)
	{
		if (reason == InvestigationReason::Body)
		{
			if (causer.empty() == false)
			{
				NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(causer));

				if (npcRef)
				{
					npcRef->BodyInvestigated();
					owner->PlayPhrace("dead_body");
				}

			}
		}
	}
	else
	{

		if (reason == InvestigationReason::Body && owner->found_guard == false)
		{
			if (causer.empty() == false)
			{
				NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(causer));

				if (npcRef)
				{
					npcRef->BodyInvestigated();
					owner->PlayPhrace("dead_body");
				}

			}
		}

		if (owner->found_guard)
		{
			auto guardRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(owner->closestGuard));

			if (owner->currentInvestigation != nullptr)
			{

				if (guardRef)
				{
					guardRef->TryStartInvestigation(owner->currentInvestigation->reason, owner->currentInvestigation->target, owner->Id, true);
					owner->report_to_guard = false;
				}

			}
			else
			{

				owner->ShareTargetKnowlageWith(guardRef);

			}

		}

	}

	owner->movementLockDelay.AddDelay(2);

	owner->currentInvestigation = nullptr;

	owner->report_to_guard = false;


}

nlohmann::json InvestigationBase::Serialize()
{
	nlohmann::json data;

	SERIALIZE_FIELD(data, target);
	SERIALIZE_FIELD(data, causer);
	SERIALIZE_FIELD(data, sharedByNpc);
	SERIALIZE_FIELD(data, reason);

	return data;
}

void InvestigationBase::Deserialize(nlohmann::json& data)
{
	DESERIALIZE_FIELD(data, target);
	DESERIALIZE_FIELD(data, causer);
	DESERIALIZE_FIELD(data, sharedByNpc);
	DESERIALIZE_FIELD(data, reason);
}
