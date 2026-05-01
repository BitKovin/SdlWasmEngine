#include "InvestigationBase.h"
#include "../NpcBase.h"

void InvestigationBase::Start(InvestigationReason reason, vec3 target, std::string causer, bool sharedByNpc)
{
	this->target = target;
	this->causer = causer;
	this->sharedByNpc = sharedByNpc;
	this->reason = reason;
	TargetLocation = target;

	OnStart();
}

void InvestigationBase::ReachedTarget()
{
	Finish();
}

void InvestigationBase::Finish()
{
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
	SERIALIZE_FIELD(data, TargetLocation);

	return data;
}

void InvestigationBase::Deserialize(nlohmann::json& data)
{
	DESERIALIZE_FIELD(data, target);
	DESERIALIZE_FIELD(data, causer);
	DESERIALIZE_FIELD(data, sharedByNpc);
	DESERIALIZE_FIELD(data, reason);
	DESERIALIZE_FIELD(data, TargetLocation);
}
