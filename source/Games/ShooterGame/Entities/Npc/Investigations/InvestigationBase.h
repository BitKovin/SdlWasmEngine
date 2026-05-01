#pragma once

#include <string>
#include <glm.h>
#include <json.hpp>

enum class InvestigationReason
{
	TargetSeen,
	NpcInTrouble,
	WeaponFire,
	Body,
	Explosion,
	LoudNoise,
	Noise,
	None
};

class NpcBase;

class InvestigationBase
{
public:
	InvestigationBase(NpcBase* owner) { this->owner = owner; }
	virtual ~InvestigationBase() = default;

	void Start(InvestigationReason reason, vec3 target, std::string causer, bool sharedByNpc = false);

	virtual void OnStart() {}
	virtual void Update(float deltaTime) {}
	virtual bool Alerted() { return false; }
	virtual void ReachedTarget();
	virtual void Finish();

	virtual nlohmann::json Serialize();
	virtual void Deserialize(nlohmann::json& data);

	vec3 target{};
	std::string causer = "";
	bool sharedByNpc = false;
	InvestigationReason reason = InvestigationReason::None;
	NpcBase* owner = nullptr;

	vec3 TargetLocation{};
	float DelayAfterReach = 1;
};
