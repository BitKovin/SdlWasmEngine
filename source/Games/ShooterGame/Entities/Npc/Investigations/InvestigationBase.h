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

	void Start(InvestigationReason reason, vec3 target, std::string causer, bool sharedByNpc = false);

	virtual void OnStart();

	virtual void Update(float deltaTime);

	virtual bool Alerted();

	virtual void ReachedTarget();

	virtual void Finish();

	virtual nlohmann::json Serialize();
	virtual void Deserialize(nlohmann::json& data);

	vec3 target{};
	std::string causer = "";
	bool sharedByNpc = false;
	InvestigationReason reason;
	NpcBase* owner = nullptr;

	vec3 TargetLocation; //where npc will be moving
	float DelayAfterReach = 1; //how much npc will wait after reaching target location

private:

};

