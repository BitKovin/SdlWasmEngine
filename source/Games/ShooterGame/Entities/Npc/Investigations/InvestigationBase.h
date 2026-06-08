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
	LookAt,
	None
};

class NpcBase;

class InvestigationBase
{
public:
	InvestigationBase(NpcBase* owner) { this->owner = owner; }
	virtual ~InvestigationBase() = default;

	void Start(InvestigationReason reason, vec3 target, std::string causer, bool sharedByNpc, bool restarted);

	virtual void OnStart() {}
	virtual void Update(float deltaTime);
	virtual bool Alerted() { return false; }
	virtual void ReachedTarget();
	virtual void Finish();

	virtual nlohmann::json Serialize();
	virtual void Deserialize(nlohmann::json& data);

	// -----------------------------------------------------------------------
	// Orient phase — before walking the NPC turns to face the investigation
	// target, then holds that pose briefly.  Override either value (> 0) in
	// a subclass to enable; both default to 0 (feature disabled).
	//
	//   OrientRotationSpeed  degrees per second (0 = disabled)
	//   OrientWaitTime       seconds to stand still after rotation completes
	//
	// The two values are independent: you can rotate without waiting, wait
	// without rotating (instant snap + hold), or combine both.
	// -----------------------------------------------------------------------
	virtual float OrientRotationSpeed() const { return 180.0f; }
	virtual float OrientWaitTime()      const { return 0.75f; }
	virtual bool StopWhenSeeCauser() const { return false; }


	vec3 target{};
	std::string causer = "";
	bool sharedByNpc = false;
	InvestigationReason reason = InvestigationReason::None;
	NpcBase* owner = nullptr;
	bool restarted = false; // true if this investigation is a restart of a previous one

	vec3  TargetLocation{};
	float DelayAfterReach = 1;

	float orientTimer = 0.0f;

protected:
	// Single countdown that encodes both stages:
	//   orientTimer > OrientWaitTime()  →  still rotating
	//   orientTimer in (0, OrientWaitTime()]  →  waiting (already facing target)
	//   orientTimer <= 0                →  orient phase complete


	float StayInPlaceAfterFinishTime = 1.0f; // how long the NPC stays in place after finishing investigation before it can start moving again

};
