#include "InvestigationBase.h"
#include "../NpcBase.h"

#include <cmath>

void InvestigationBase::Start(InvestigationReason reason, vec3 target, std::string causer, bool sharedByNpc, bool restarted)
{
	this->target = target;
	this->causer = causer;
	this->sharedByNpc = sharedByNpc;
	this->reason = reason;
	this->restarted = restarted;
	TargetLocation = target;

	// -----------------------------------------------------------------------
	// Orient phase setup.
	//
	// Rotation duration is computed from the actual angle the NPC must sweep,
	// so movementLockDelay and orientTimer expire at the exact same moment.
	//
	//   orientTimer = rotateDuration + OrientWaitTime()
	//
	// If either virtual returns 0 the corresponding stage is skipped.
	// -----------------------------------------------------------------------
	const float rotSpeed = OrientRotationSpeed();
	const float waitTime = OrientWaitTime();

	if (rotSpeed > 0.0f || waitTime > 0.0f)
	{
		float rotateDuration = 0.0f;

		if (rotSpeed > 0.0f)
		{
			vec3 toTarget = MathHelper::XZ(target - owner->Position);
			vec3 curDir = MathHelper::XZ(owner->movingDirection);

			if (length(toTarget) > 0.001f && length(curDir) > 0.001f)
			{
				float currentAngle = std::atan2(curDir.z, curDir.x);
				float targetAngle = std::atan2(toTarget.z, toTarget.x);
				float diff = targetAngle - currentAngle;

				// Wrap to shortest path [-π, π]
				while (diff > glm::pi<float>()) diff -= 2.0f * glm::pi<float>();
				while (diff < -glm::pi<float>()) diff += 2.0f * glm::pi<float>();

				rotateDuration = glm::degrees(std::abs(diff)) / rotSpeed;
			}
			// If curDir is near-zero (NPC has no facing yet) rotateDuration
			// stays 0 and Update() will snap to target on the first frame.
		}

		orientTimer = rotateDuration + waitTime;

		if (orientTimer > 0.0f)
			owner->movementLockDelay.AddDelay(orientTimer);
	}

	OnStart();
}

void InvestigationBase::Update(float deltaTime)
{
	if (orientTimer <= 0.0f)
		return;

	// Check whether the causer is currently visible. If so, continuously track
	// them for the remainder of the orient phase — the NPC noticed something
	// suspicious but won't chase it (avoids falling for an obvious trap).
	bool seesCauser = false;

	if (StopWhenSeeCauser())
	{
		for (auto& t : owner->observer->visibleTargets)
		{
			if (t->ownerId == causer)
			{
				target = t->position;
				seesCauser = true;
				break;
			}
		}
	}

	// Resolve the direction to rotate toward this frame.
	// When the causer is visible we always use their live position; otherwise
	// we use the fixed investigation target.
	vec3 toTarget = MathHelper::XZ(target - owner->Position);

	if (length(toTarget) > 0.001f)
	{
		vec3 curDir = MathHelper::XZ(owner->movingDirection);

		if (length(curDir) < 0.001f)
		{
			// No current facing — snap straight to target direction.
			owner->movingDirection = normalize(toTarget);
		}
		else if (seesCauser || orientTimer > OrientWaitTime())
		{
			// -------------------------------------------------------------------
			// Rotation stage (or live causer tracking):
			//
			// When seesCauser is true this runs every frame for the full
			// remaining duration so the NPC keeps their gaze on the causer
			// without ever advancing toward them.
			//
			// When seesCauser is false this only runs while the timer is above
			// the wait threshold; once in the wait stage the NPC already faces
			// the target and we just let the timer expire.
			// -------------------------------------------------------------------
			float currentAngle = std::atan2(curDir.z, curDir.x);
			float targetAngle = std::atan2(toTarget.z, toTarget.x);
			float diff = targetAngle - currentAngle;

			while (diff > glm::pi<float>()) diff -= 2.0f * glm::pi<float>();
			while (diff < -glm::pi<float>()) diff += 2.0f * glm::pi<float>();

			float maxDelta = glm::radians(OrientRotationSpeed()) * deltaTime;
			float step = (diff >= 0.0f ? 1.0f : -1.0f) * std::min(std::abs(diff), maxDelta);

			float newAngle = currentAngle + step;
			owner->movingDirection = vec3(std::cos(newAngle), 0.0f, std::sin(newAngle));
		}
		// Normal wait stage (seesCauser == false, orientTimer <= OrientWaitTime()):
		// NPC already faces the fixed target — just count down.
	}

	orientTimer -= deltaTime;

	if (orientTimer <= 0 && seesCauser)
	{
		Finish();
	}

}

void InvestigationBase::ReachedTarget()
{
	Finish();
}

void InvestigationBase::Finish()
{
	owner->movementLockDelay.AddDelay(StayInPlaceAfterFinishTime);
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
	SERIALIZE_FIELD(data, orientTimer);

	return data;
}

void InvestigationBase::Deserialize(nlohmann::json& data)
{
	DESERIALIZE_FIELD(data, target);
	DESERIALIZE_FIELD(data, causer);
	DESERIALIZE_FIELD(data, sharedByNpc);
	DESERIALIZE_FIELD(data, reason);
	DESERIALIZE_FIELD(data, TargetLocation);
	DESERIALIZE_FIELD(data, orientTimer);
}