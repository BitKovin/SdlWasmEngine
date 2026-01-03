#pragma once

#include <Entity.h>
#include <MathHelper.hpp>
#include <Camera.h>
#include <SkeletalMesh.hpp>
#include <AssetRegistry.h>

#include <Navigation/PathFollowQuery.h>

#include <SoundSystem/SoundManager.hpp>
#include <Entities/SoundPlayer.h>
#include <Delay.hpp>

#include "Animators/NpcAnimatorBase.h"

#include <BehaviourTree/BehaviorTree.h>
#include <BehaviourTree/BehaviorTreeEditor.h>

#include <AiPerception/AiPerceptionSystem.h>
#include <AiPerception/Observer.h>
#include <AiPerception/ObservationTarget.h>

#include "../../Ai/NpcTasks/TaskState.h"

#include <unordered_map>
#include <set>
#include <limits>

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

enum class Crime
{

	WeaponFire, //aka violentCrime tag
	Group_Attack,
	WeaponFireSound,
	NearBody,
	WeaponHolding,
	Group_Arrest,
	Trespassing,
	Group_Follow,
	None
};

// Per-target memory structure used to track multiple targets
struct TargetInfo
{
	std::string id;
	bool follow = false;
	vec3 lastSeenPosition = vec3();
	Delay stopUpdateLastSeenPositionDelay = Delay();
	float lastSeenTime = -1;
	bool sees = false;
	bool underArrest = false;
	bool attack = false;
	float underArrestExpire = 5.0f;
	bool attackInRange = false;
	Crime currentCrime = Crime::None;
	float detection_progress = 0.0f;

	void Serialize(json& target) const
	{
		SERIALIZE_FIELD(target, id);
		SERIALIZE_FIELD(target, follow);
		SERIALIZE_FIELD(target, lastSeenPosition);
		SERIALIZE_FIELD(target, stopUpdateLastSeenPositionDelay);
		SERIALIZE_FIELD(target, lastSeenTime);
		SERIALIZE_FIELD(target, sees);
		SERIALIZE_FIELD(target, underArrest);
		SERIALIZE_FIELD(target, attack);
		SERIALIZE_FIELD(target, underArrestExpire);
		SERIALIZE_FIELD(target, attackInRange);
		SERIALIZE_FIELD(target, currentCrime);
		SERIALIZE_FIELD(target, detection_progress);
	}

	void Deserialize(const json& source)
	{
		DESERIALIZE_FIELD(source, id);
		DESERIALIZE_FIELD(source, follow);
		DESERIALIZE_FIELD(source, lastSeenPosition);
		DESERIALIZE_FIELD(source, stopUpdateLastSeenPositionDelay);
		DESERIALIZE_FIELD(source, lastSeenTime);
		DESERIALIZE_FIELD(source, sees);
		DESERIALIZE_FIELD(source, underArrest);
		DESERIALIZE_FIELD(source, attack);
		DESERIALIZE_FIELD(source, underArrestExpire);
		DESERIALIZE_FIELD(source, attackInRange);
		DESERIALIZE_FIELD(source, currentCrime);
		DESERIALIZE_FIELD(source, detection_progress);
	}
};


class NpcBase : public Entity
{

protected:

	vec3 desiredDirection = vec3();
	vec3 movingDirection = vec3();

	SoundPlayer* VoiceSoundPlayer = nullptr;


	float maxSpeed = 4;
	float speed = 2.0;

	bool dead = false;

	AnimationState animationStateSaveData;

	Animation* getFromRagdollAnimation = nullptr;
	AnimationPose ragdollPose;

	AnimationState getFromRagdollAnimationSaveState;

	vec3 desiredTargetLocation = vec3();

	BehaviorTree behaviorTree;
	BehaviorTreeEditor editor;

	std::string btSaveState = "";

	bool btEditorEnabled = false;

	Delay tickIntervalDelay;

	std::string fractionTag = "citizen";

	float attackRange = 20;
	float attackDesiredRange = 10;

	std::shared_ptr<Observer> observer;

	// Primary-target copies (kept for compatibility with existing code)
	bool target_follow = false;
	std::string target_id = "";
	vec3 target_lastSeenPosition = vec3();
	Delay target_stopUpdateLastSeenPositionDelay = Delay();
	float target_lastSeenTime = -1;
	bool target_sees = false;
	bool target_underArrest = false;
	bool target_attack = false;
	float target_underArrestExpire = 5.0f;
	bool target_attackInRange = false;

	// Multi-target memory and tag lists
	std::unordered_map<std::string, TargetInfo> knownTargets;
	std::set<hashed_string> neutralTags = { "player" }; // default neutral tags
	std::set<hashed_string> hostileTags = {}; // tags that are always considered hostile
	float forgetTime = 6000000.0f; // seconds to forget unseen targets

	bool report_to_guard = false;
	bool found_guard = false;
	std::string closestGuard = "";

	Delay findGuardCooldown = Delay();

	InvestigationReason currentInvestigation = InvestigationReason::None;
	vec3 investigation_target = vec3();
	std::string investigation_targetId = "";

	bool investigation_changed = false;

	bool needToInvestigateBody = false;

	std::shared_ptr<ObservationTarget> observationTarget;

	const std::string vo_base_event_path = "event:/Character/Guards/Guard1/";

	Crime currentCrime = Crime::None;

	vec3 flee_target = vec3();

	StaticMesh* weaponMesh = nullptr;

	float detection_progress = 0.0f;

	bool needHelpStunned = false;

	bool stunnedRagdoll = false;
	Delay stunnedRagdollDelay;
	bool returningFromRagdoll = false;

	Delay attackDelay;
	Delay attackPositionUpdateDelay;

	vec3 attackPosition = vec3();

	Delay movementLockDelay = Delay();

	vec3 spineRotation = vec3();

	std::string modelPath = "GameData/models/npc/base.glb";

	std::string defaultTask = "np_0_0";

	std::string scheduledTask = "";
	std::string DesiredTask = "defaultTask";

private:

	bool has_observed_crime = false;

	float GetDetectionSpeed(Crime crime) const;

	std::vector<NpcBase*> shareKnowlageWith;

	short knowlageSharedThisFrame = 0;

public:

	bool DoingTask = true;
	bool DoingTaskOld = true;

	NpcAnimatorBase animator = 0;

	bool isGuard = true;

	static inline Delay globalPhraceDelay;

	SkeletalMesh* mesh = nullptr;

	TaskState taskState;

	vec3 desiredLookVector = vec3(1);

	PathFollowQuery pathFollow;

	std::string SimulationId;

	NpcBase();


	~NpcBase();

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 40;
		soundPlayer->Volume = 1.0;
		soundPlayer->MinDistance = 0.5;
	}

	void DoInterpolatedAnimationUpdate();

	void ProcessAnimationEvent(AnimationEvent& event);

	void FromData(EntityData data);

	void Start();

	void Death();

	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	bool isStunned();

	void StartStunnedRagdoll();
	void UpdateStunnedReturn();

	void StartReturnFromRagdoll();
	void UpdateReturnFromRagdoll();


	void Destroy();

	void PlayPhrace(std::string name);

	void Update();

	void AsyncUpdate();
	void UpdateWeaponMesh();

	void LateUpdate();

	void UpdateDoorUpdate();

	void UpdateBT();
	void UpdateObserver();
	void UpdateObservationTarget();

	void UpdateTargetFollow();

	void UpdateTargetAttack();

	class NpcSimulationState* GetSimulationStateRef();

	bool CheckAttackLOS(vec3 location, vec3 targetLocation);

	//input are positions of attacker and target without vertical offset for fire point
	bool CheckAttackLocation(vec3 location, vec3 targetLocation);

	vec3 FindAttackLocation();

	void Serialize(json& target);
	void Deserialize(json& source);

	void UpdateDebugUI();

	void FindClosestGuard();

	void TryStartInvestigation(InvestigationReason reason, vec3 target, string causer, bool sharedByNpc = false);

	bool TryCommitCrime(Crime crime, std::string offender, vec3 pos);

	void FinishInvestigation();

	void PrepareToStartMovement();
	void StopMovement();
	void MoveTo(const vec3& target, float acceptanceRadius);

	// Overload: stop follow for a specific target id (new)
	void StopTargetFollow();
	void StopTargetFollow(const std::string& id);

	void BodyInvestigated();

	void Task_TargetReached();
	void Task_DoStationaryJob();

	void Task_Doint();
	void Task_NotDoing();

	TaskState& GetTaskStateRef();

	void StartTask(const std::string& taskName);
	void StopTask();
	void UpdateTask();

protected:

	void UpdateAnimations(bool forceFullUpdate = false);

	void LoadAssets();

	void ShareTargetKnowlageWith(NpcBase* anotherNpc);

	void ShareTargetKnowlageWithFinal(NpcBase* anotherNpc);

	// helper to select primary (existing) target from knownTargets
	void SelectPrimaryAndCopy();

	bool IsNeutral(const std::shared_ptr<ObservationTarget>& target) const;
	bool IsHostile(const std::shared_ptr<ObservationTarget>& target) const;

	void MoveToScheduledTask();

	void UpdateScheduledTask();

private:

};