#include "NpcBase.h"

#include <SoundSystem/FmodEventInstance.h>

#include <Particle/GlobalParticleSystem.hpp>

#include <Input.h>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include "Ai/TaskPoint.h"

#include "../Player/Weapons/Projectiles/Bullet.h"
#include "../Player/Player.hpp"

#include "../Enviroment/Door.h"


#include <EngineMain.h>

#include <RandomHelper.h>

#include <limits>

#include <LevelTraversalSystem.h>
#include "../NpcSimulationManager.h"

float NpcBase::GetDetectionSpeed(Crime crime) const
{
	switch (crime) {
	case Crime::WeaponFire:
	case Crime::WeaponFireSound:
	case Crime::Group_Attack:
	case Crime::NearBody:
		return 10000.0f; // immediate
	case Crime::WeaponHolding:
		return 1.0f;
	case Crime::Trespassing:
		// slow detection for trespassing: only apply slow detection for neutral targets (player)
		return 0.63f; // ~1.5 seconds to full detection
	default:
		return 0.0f; // no detection buildup
	}
}

NpcBase::NpcBase()
{

	mesh = new SkeletalMesh(this);
	Drawables.push_back(mesh);
	weaponMesh = new StaticMesh(this);
	Drawables.push_back(weaponMesh);

	ClassName = "npc_base";
	SaveGame = true;

	Health = 100;

	mesh->UpdatePoseOnlyWhenRendered = true;

	animator = NpcAnimatorBase(this);

	Tags.push_back("npc");
	Tags.push_back(fractionTag);

	// knownTargets empty by default, neutralTags already has "player"
}

NpcBase::~NpcBase()
{
}

void NpcBase::DoInterpolatedAnimationUpdate()
{
}

void NpcBase::ProcessAnimationEvent(AnimationEvent& event)
{

	animator.UsePrecomputedFrames = false;
	UpdateAnimations();

}

void NpcBase::FromData(EntityData data)
{

	Entity::FromData(data);

	defaultTask = data.GetPropertyString("target");

}

void NpcBase::Start()
{



	//Drawables.push_back(mesh);

	desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
	movingDirection = desiredDirection;

	//pathFollow.Async = false;

	VoiceSoundPlayer = SoundPlayer::Create();
	VoiceSoundPlayer->SetSound(FmodEventInstance::Create("event:/Character/Dialogue"));
	SetupSoundPlayer(VoiceSoundPlayer);


	editor = BehaviorTreeEditor(&behaviorTree);
	editor.Init();

	behaviorTree.LoadFromFile("GameData/behaviourTrees/general.bt");
	behaviorTree.Owner = this;
	behaviorTree.Start();

	observationTarget = AiPerceptionSystem::CreateTarget(Position, Id, {});
	observationTarget->noticeMaxDistanceMultiplier = 0.3;
	observationTarget->npc = true;

	observer = AiPerceptionSystem::CreateObserver(Position + vec3(0, 0.7, 0), movingDirection, 90);
	observer->owner = Id;
	observer->ownerPtr = this;

	pathFollow.reachedTarget = true;

	StartTask(defaultTask);

	MoveToScheduledTask();

	controller = new CharacterController();
	controller->movementQuality = CharacterControllerMovementQuality::NpcGeneric;
	controller->Init(this, Position, 0.45f);

	mesh->Position = Position - vec3(0, controller->height / 2.0f, 0);
	mesh->Rotation = Rotation;

}


void NpcBase::Death()
{

	if (dead) return;

	needToInvestigateBody = true;

	//mesh->ClearHitboxes();
	DoInterpolatedAnimationUpdate();
	mesh->StartRagdoll();
	mesh->RagdollPoseFollowStrength = 0.0f;
	mesh->SetAnimationPaused(true);

	VoiceSoundPlayer->Stop();
	VoiceSoundPlayer->Volume = 0;



	if (controller)
	{
		delete controller;
		controller = nullptr;
	}



	dead = true;

	//Tags.clear();



	Tags.push_back("dead");

	AiPerceptionSystem::RemoveObserver(observer);
	observer = nullptr;



}

void NpcBase::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
	Damage *= mesh->GetHitboxDamageMultiplier(bone);
	Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

	if (mesh->GetHitboxDamageMultiplier(bone) > 1.1f && false)
	{
		Time::AddTimeScaleEffect(0.3f, 0.15f, true, "hit_slow");
	}

	if (bone == "calf_l" || bone == "calf_r" || Damage > 50)
	{
		StartStunnedRagdoll();
	}

	GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage / 20.0f));

}

void NpcBase::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{

	Health -= Damage;

	if (Health <= 0)
	{
		Death();
	}

	if (DamageCauser != nullptr)
	{
		if (controller)
		{
			controller->SetVelocity(controller->GetVelocity() / 2.0f);
			speed /= 2.0f;
		}
	}



}

bool NpcBase::isStunned()
{
	return returningFromRagdoll || stunnedRagdoll;
}

void NpcBase::StartStunnedRagdoll()
{

	if (dead)return;

	if (stunnedRagdoll == false)
	{
		mesh->RagdollPoseFollowStrength = 0.5f;
		DoInterpolatedAnimationUpdate();
		mesh->StartRagdoll();
		stunnedRagdoll = true;
	}

	stunnedRagdollDelay.AddDelay(2);
	needHelpStunned = true;

}

void NpcBase::UpdateStunnedReturn()
{

	if (stunnedRagdoll == false) return;



	Body* pelvisBody = mesh->FindHitboxByName("pelvis");

	vec3 pelvisPos = FromPhysics(pelvisBody->GetPosition());

	controller->SetPosition(pelvisPos + vec3(0, 0.4f, 0));

	if (stunnedRagdollDelay.Wait()) return;

	bool hitsGround = Physics::LineTrace(pelvisPos, pelvisPos - vec3(0, 0.5f, 0), BodyType::World).hasHit;

	if (hitsGround || pelvisBody->GetLinearVelocity().Length() < 0.1f)
	{

		StartReturnFromRagdoll();
		stunnedRagdoll = false;
		needHelpStunned = false;

	}
	else
	{
		stunnedRagdollDelay.AddDelay(0.3f + RandomHelper::RandomFloat() * 0.2f);
	}

}

void NpcBase::StartReturnFromRagdoll()
{

	if (mesh->InRagdoll == false) return;



	vec3 pelvisPos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Position;
	vec3 pelvisRot = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Rotation;
	vec3 spinePos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("spine_03")).Position;

	mesh->UpdateHitboxes();

	ragdollPose = mesh->GetAnimationPose();

	bool onFront = MathHelper::GetUpVector(MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Rotation).y > 0;

	getFromRagdollAnimation->PlayAnimation(onFront ? "front" : "back", false, 0);

	auto pelvisTransformWorld = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis"));


	Position = pelvisTransformWorld.Position + vec3(0, 1.0f, 0);
	controller->SetPosition(Position);

	float oldRot = mesh->Rotation.y;

	if (onFront)
	{
		mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(pelvisPos, spinePos).y, 0);
	}
	else
	{
		mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(spinePos, pelvisPos).y, 0);
	}



	auto pelvisTransform = MathHelper::DecomposeMatrix(ragdollPose.boneTransforms["pelvis"]);

	pelvisTransform.Position = vec3(0, 3.0f, 0);
	pelvisTransform.RotationQuaternion = MathHelper::GetRotationQuaternion(vec3(0, oldRot - mesh->Rotation.y, 0)) * pelvisTransform.RotationQuaternion;

	ragdollPose.boneTransforms["pelvis"] = pelvisTransform.ToMatrix();

	mesh->StopRagdoll();

	returningFromRagdoll = true;

	movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
	desiredDirection = movingDirection;


}

void NpcBase::UpdateReturnFromRagdoll()
{

	if (getFromRagdollAnimation->IsAnimationPlaying() == false)
	{
		if (returningFromRagdoll)
		{
			returningFromRagdoll = false;
			stunnedRagdoll = false;
		}
		return;
	}


	auto meshPose = mesh->GetAnimationPose();

	getFromRagdollAnimation->Update(1.0f);

	float blendInTime = 0.5;
	float blendOutTime = 0.7f;

	float lerpProgressFromStart = 1.0f - ((blendInTime - getFromRagdollAnimation->GetAnimationTime()) / blendInTime);

	lerpProgressFromStart = saturate(lerpProgressFromStart);

	auto animationPose = getFromRagdollAnimation->GetAnimationPose();

	auto newPose = AnimationPose::Lerp(animationPose, ragdollPose, 1.0 - lerpProgressFromStart);

	float lerpProgressFromEnd = ((blendOutTime - (getFromRagdollAnimation->GetAnimationDuration() - getFromRagdollAnimation->GetAnimationTime())) / blendOutTime);

	newPose = AnimationPose::Lerp(meshPose, newPose, 1.0f - lerpProgressFromEnd);

	mesh->PasteAnimationPose(newPose);

}


void NpcBase::Destroy()
{

	Entity::Destroy();

	mesh->ClearHitboxes();

	VoiceSoundPlayer->Destroy();
	AiPerceptionSystem::RemoveTarget(observationTarget);
	AiPerceptionSystem::RemoveObserver(observer);

}

void NpcBase::PlayPhrace(std::string name)
{

	if (EngineMain::MainInstance->SimulatingGameTicks) return;

	if (VoiceSoundPlayer == nullptr) return;

	if (globalPhraceDelay.Wait()) return;

	const string eventPath = vo_base_event_path + name;

	VoiceSoundPlayer->Stop();
	VoiceSoundPlayer->SetSound(FmodEventInstance::Create(eventPath));
	VoiceSoundPlayer->Play();

	globalPhraceDelay.AddDelay(1.0f);

}

void NpcBase::Update()
{
	Entity::Update();

	UpdateTask();

}

void NpcBase::AsyncUpdate()
{

	if (controller)
	{
		controller->Update(Time::DeltaTimeF);
		Position = controller->GetPosition();
	}


	pathFollow.WaitToFinish();

	if (dead == false)
	{
		UpdateObserver();
		UpdateTargetFollow();
		UpdateBT();
		UpdateTargetAttack();

		UpdateScheduledTask();

	}


	auto animEvents = mesh->PullAnimationEvents();

	for (auto& event : animEvents)
	{
		ProcessAnimationEvent(event);
	}


	if (dead)
	{
		UpdateAnimations();
		if (mesh->WasRended)
		{
			mesh->UpdateHitboxes();
		}
		else if (tickIntervalDelay.Wait() == false)
		{
			mesh->UpdateHitboxes();
		}

		weaponMesh->Visible = false;

		return;
	}

	auto headHitbox = mesh->FindHitboxByName("neck_01");

	if (headHitbox)
		VoiceSoundPlayer->Position = FromPhysics(headHitbox->GetPosition());

	VoiceSoundPlayer->Velocity = controller->GetVelocity();

	vec3 curMove = MathHelper::XZ(controller->GetVelocity());

	if (isStunned() || mesh->InRagdoll)
	{

		UpdateStunnedReturn();

		controller->SetVelocity(vec3(0,controller->GetVelocity().y, 0));

	}
	else if (movementLockDelay.Wait())
	{

		desiredDirection = MathHelper::Interp(curMove, vec3(), Time::DeltaTimeF, 5.0f);

		float gravity = controller->GetVelocity().y;

		vec3 move = desiredDirection;

		move.y = gravity;

		controller->SetVelocity(move);

	}
	else
	{

		bool lockAtTarget = ((target_attack && target_sees && target_attackInRange && isGuard) || (target_follow && target_sees && length(curMove) < 1)) && DoingTask == false;

		desiredDirection = vec3(0);

		if (pathFollow.reachedTarget == false || pathFollow.CalculatedPath == false)
		{
			pathFollow.UpdateStartAndTarget(Position, desiredTargetLocation);
			pathFollow.TryPerform();
		}



		if (pathFollow.reachedTarget == false && pathFollow.CalculatedPath)
		{

			UpdateDoorUpdate();

			vec3 dir = MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position);

			if (length(dir) > 0.001)
			{
				vec3 newMove = normalize(dir) * speed;

				vec3 curMove = MathHelper::XZ(controller->GetVelocity());

				desiredDirection = MathHelper::Interp(curMove, newMove, Time::DeltaTimeF, 5.0f);

				if (lockAtTarget == false)
				{
					movingDirection += desiredDirection * Time::DeltaTimeF * 3.0f;
				}

				desiredLookVector = movingDirection;
			}

		}
		else
		{

			vec3 curMove = MathHelper::XZ(controller->GetVelocity());

			desiredDirection = MathHelper::Interp(curMove, vec3(0), Time::DeltaTimeF, 8.0f);

			if (lockAtTarget == false)
			{
				movingDirection = MathHelper::Interp(movingDirection, desiredLookVector, Time::DeltaTimeF, 2);
			}

		}

		if (lockAtTarget)
		{
			auto targetRef = Level::Current->FindEntityWithId(target_id);

			desiredLookVector = targetRef->Position - Position;

			movingDirection = MathHelper::Interp(movingDirection, desiredLookVector, Time::DeltaTimeF, 4.0f);

		}



		if (length(movingDirection) > 1.0f)
		{
			movingDirection = normalize(movingDirection);
		}

		if (length(movingDirection) < 0.5f && length(movingDirection) > 0.01f)
		{
			movingDirection = normalize(movingDirection) * 0.3f;
		}

		float gravity = controller->GetVelocity().y;

		vec3 move = desiredDirection;


		move.y = gravity;

		controller->SetVelocity(move);


	}



	UpdateAnimations();
	UpdateReturnFromRagdoll();


	mesh->Position = Position - vec3(0, controller->height / 2.0f, 0);
	mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

	if (mesh->WasRended)
	{
		mesh->UpdateHitboxes();
	}
	else if (tickIntervalDelay.Wait() == false)
	{
		mesh->UpdateHitboxes();
	}

	UpdateWeaponMesh();

	if (tickIntervalDelay.Wait() == false)
	{

		if (EngineMain::MainInstance->SimulatingGameTicks)
		{
			tickIntervalDelay.AddDelay(RandomHelper::RandomFloat() + 0.5f);
		}
		else
		{
			tickIntervalDelay.AddDelay(0.15f + distance(Position, Camera::position) / 200.0f);
		}


	}




}
void NpcBase::UpdateWeaponMesh()
{

	weaponMesh->Visible = false;

	if (animator.weapon_holds == false && animator.weapon_ready == false && animator.weapon_aims == false) return;

	if (taskState.AllowWeapon == false && DoingTask)
	{
		return;
	}

	weaponMesh->Visible = true;

	const mat4 rotationFixMatrix = MathHelper::GetRotationMatrix(vec3(90, 0, 0));

	auto weaponTrans = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("weapon") * rotationFixMatrix);

	weaponMesh->Position = weaponTrans.Position;
	weaponMesh->Rotation = weaponTrans.Rotation;


}
void NpcBase::LateUpdate()
{
	UpdateObservationTarget();

	for (auto npc : shareKnowlageWith)
	{
		ShareTargetKnowlageWithFinal(npc);
	}
	shareKnowlageWith = std::vector<NpcBase*>();
	knowlageSharedThisFrame = 0;
}

void NpcBase::UpdateDoorUpdate()
{

	if (observer->id % 3 != (EngineMain::MainInstance->frame + 1) % 3) return;


	auto trimmedPath = MathHelper::GetPathWithLength(Position, pathFollow.CurrentPath, 3.0f);


	bool first = true;

	vec3 prevPoint = vec3();

	for (auto& point : trimmedPath)
	{
		if (first)
		{
			prevPoint = point;
			first = false;
			continue;
		}

		auto hitResult = Physics::LineTrace(prevPoint + vec3(0, 0.2f, 0), point + vec3(0, 0.5f, 0), BodyType::MainBody);

		if (hitResult.hasHit)
		{
			auto door = dynamic_cast<Door*>(hitResult.entity);
			if (door)
			{

				if (door->IsOpen == false)
				{
					door->OpenFromPosition(Position);
				}

			}
		}

		prevPoint = point;

	}


}

void NpcBase::UpdateBT()
{

	behaviorTree.GetBlackboard().SetValue("target", target_id);


	if (taskState.HasToMoveToTarget)
	{
		behaviorTree.GetBlackboard().SetValue("task_move", true);
		behaviorTree.GetBlackboard().SetValue("task_moveLocation", taskState.TargetLocation);
		behaviorTree.GetBlackboard().SetValue("task_acceptanceRadius", taskState.AcceptanceRadius);
		behaviorTree.GetBlackboard().SetValue("task_setOrientation", taskState.HasToLookAtTarget);
	}
	else
	{
		behaviorTree.GetBlackboard().SetValue("task_move", false);
		behaviorTree.GetBlackboard().SetValue("task_moveLocation", vec3(0.0f));
		behaviorTree.GetBlackboard().SetValue("task_acceptanceRadius", 0.0f);
		behaviorTree.GetBlackboard().SetValue("task_setOrientation", false);
	}
	behaviorTree.GetBlackboard().SetValue("task_canBeCanceled", taskState.CanBeCanceled);
	behaviorTree.GetBlackboard().SetValue("task_doingJob", taskState.DoingJob);

	behaviorTree.GetBlackboard().SetValue("stunned", isStunned());

	behaviorTree.GetBlackboard().SetValue("isGuard", isGuard);

	behaviorTree.GetBlackboard().SetValue("target_follow", target_follow);
	behaviorTree.GetBlackboard().SetValue("target_id", target_id);
	behaviorTree.GetBlackboard().SetValue("target_sees", target_sees);
	behaviorTree.GetBlackboard().SetValue("target_knowsLocation", target_sees || target_stopUpdateLastSeenPositionDelay.Wait());
	behaviorTree.GetBlackboard().SetValue("target_lastSeenPosition", target_lastSeenPosition);
	behaviorTree.GetBlackboard().SetValue("target_underArrest", target_underArrest);
	behaviorTree.GetBlackboard().SetValue("target_attack", target_attack);
	behaviorTree.GetBlackboard().SetValue("target_attackInRange", target_attackInRange);

	behaviorTree.GetBlackboard().SetValue("attackPosition", attackPosition);

	behaviorTree.GetBlackboard().SetValue("investigation", currentInvestigation != InvestigationReason::None);
	behaviorTree.GetBlackboard().SetValue("investigation_target", investigation_target);

	behaviorTree.GetBlackboard().SetValue("report_to_guard", report_to_guard);
	behaviorTree.GetBlackboard().SetValue("closestGuard", closestGuard);
	behaviorTree.GetBlackboard().SetValue("found_guard", found_guard);

	behaviorTree.GetBlackboard().SetValue("flee_target", flee_target);

	if (investigation_changed)
	{
		behaviorTree.GetBlackboard().SetValue("investigation", false); //stopping investigation for one frame to reset logic
		investigation_changed = false;
	}

	if (btEditorEnabled)
	{
		editor.Update(Time::DeltaTimeF / 1.0f);
	}
	else
	{
		behaviorTree.Update(Time::DeltaTimeF);
	}


}

void NpcBase::UpdateObserver()
{

	if (!observer) return;

	observer->searchForTriggeredNpc = !target_follow && !target_attack && isGuard;

	if (distance(Camera::finalizedPosition, Position) < 40)
	{

		MathHelper::Transform headTrans;

		headTrans = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("head"));

		observer->forward = MathHelper::TransformVector(vec3(0, -1, 0), headTrans.RotationQuaternion);

		observer->position = headTrans.Position - observer->forward * 0.2f;
	}
	else
	{
		observer->forward = movingDirection;

		observer->position = Position + vec3(0, 0.65, 0);
	}



	// Reset per-target sees
	for (auto& p : knownTargets)
	{
		p.second.sees = false;
	}

	bool anyCrimeObserved = false;

	// Process each visible target
	for (std::shared_ptr<ObservationTarget> target : observer->visibleTargets)
	{
		if (target->ownerId == Id) continue;


		// ensure we have entry if it's relevant (neutral or hostile) or we already remembered it
		bool neutral = IsNeutral(target);
		bool hostile = IsHostile(target);

		if (neutral || hostile || knownTargets.count(target->ownerId))
		{
			auto& info = knownTargets[target->ownerId];
			info.id = target->ownerId;

			
			info.sees = true;
			

			info.lastSeenPosition = target->position;
			info.lastSeenTime = Time::GameTime;
		}

		// skip targets we don't react to
		if (!neutral && !hostile && target->HasTag("body") == false)
			continue;

		Crime min_crime = Crime::None;
		std::string observed_offender = target->ownerId;
		vec3 observed_pos = target->position;

		// Hostile tags force high priority reaction
		if (hostile)
		{
			min_crime = Crime::Group_Attack;
			auto& info = knownTargets[target->ownerId];
			info.attack = true;
			info.underArrestExpire = -1;
		}

		if (target->HasTag("dead"))
		{
			if (knownTargets.count(target->ownerId))
			{

				auto& info = knownTargets[target->ownerId];

				knownTargets.erase(target->ownerId);
				continue;


			}
		}


		// Same detection logic as before, applied per target
		if (target->HasTag("violentCrime"))
		{
			if (min_crime > Crime::WeaponFire)
			{
				min_crime = Crime::WeaponFire;
			}
		}

		if (target->HasTag("illegal_weapon"))
		{
			if (min_crime > Crime::WeaponHolding)
			{
				min_crime = Crime::WeaponHolding;
			}
		}

		if (target->HasTag("trespassing") && target->HasTag("player"))
		{
			if (min_crime > Crime::Trespassing)
			{
				min_crime = Crime::Trespassing;
			}
		}

		//if weapon fire investigation, check for player proximity
		if (currentInvestigation == InvestigationReason::WeaponFire && target->HasTag("player") && distance(target->position, investigation_target) < 4)
		{
			if (min_crime > Crime::WeaponFireSound)
			{
				min_crime = Crime::WeaponFireSound;
			}
		}

		// Body handling - check for near suspects (applied per relevant other visible targets)
		if (target->HasTag("body"))
		{
			bool near_player = false;

			for (std::shared_ptr<ObservationTarget> target2 : observer->visibleTargets)
			{
				if (target2->npc == false)
				{
					if (target2->HasTag("player") || IsNeutral(target2))
					{
						if (distance(target->position, target2->position) < 5)
						{
							auto& suspectInfo = knownTargets[target2->ownerId];
							float speed = IsNeutral(target2) ? GetDetectionSpeed(Crime::NearBody) : 10000.0f;
							suspectInfo.detection_progress = std::min(1.0f, suspectInfo.detection_progress + speed * Time::DeltaTimeF);
							if (suspectInfo.detection_progress >= 1.0f)
							{
								TryCommitCrime(Crime::NearBody, target2->ownerId, target2->position);
							}
							near_player = true;
							anyCrimeObserved = true;
						}
					}
				}
			}

			if (!near_player)
			{
				TryStartInvestigation(InvestigationReason::Body, target->position, target->ownerId);
			}
		}

		if (target->HasTag("in_trouble"))
		{
			TryStartInvestigation(InvestigationReason::NpcInTrouble, target->position, target->ownerId);
		}

		auto& targetInfo = knownTargets[target->ownerId];
		targetInfo.lastMinCrime = min_crime;

		// commit detection buildup per-target
		if (min_crime != Crime::None)
		{
			auto& info = knownTargets[target->ownerId];

			info.lastTimeSpotedCrime = Time::GameTime;

			float speed = (neutral ? GetDetectionSpeed(min_crime) : 10000.0f);
			info.detection_progress = std::min(1.0f, info.detection_progress + speed * Time::DeltaTimeF);
			if (info.detection_progress >= 1.0f)
			{

				info.seesAndDetected = true;

				if (TryCommitCrime(min_crime, target->ownerId, target->position))
				{
					info.detection_progress = 1.0f;
				}
				else
				{
					// fallback behavior: if underArrest but not following, set follow
					if (info.underArrest && !info.follow)
					{
						info.follow = true;
						if (!info.stopUpdateLastSeenPositionDelay.Wait())
						{
							PlayPhrace("target_found");
							report_to_guard = true;
						}
					}
				}
			}
			anyCrimeObserved = true;
		}
	}

	// Per-target passive increases / decreases and forgetting
	for (auto it = knownTargets.begin(); it != knownTargets.end(); )
	{
		auto& info = it->second;

		// Passive increase: sees + underArrest but not following
		if (info.sees && info.underArrest && !info.follow)
		{
			float passive_speed = 0.33f;
			info.detection_progress = std::min(1.0f, info.detection_progress + passive_speed * Time::DeltaTimeF);
			if (info.detection_progress >= 1.0f)
			{
				info.seesAndDetected = true;
				info.follow = true;
				PlayPhrace("target_found");
				report_to_guard = true;
			}
		}
		else if(
			info.sees == false  
			|| (info.underArrest == false && info.lastMinCrime == Crime::None) //when we can forget a crime
			)
		{
			// decrease detection when appropriate
			bool shouldDecrease;

			if (!info.underArrest)
			{

				if (info.sees)
				{
					shouldDecrease = !info.follow || (info.lastMinCrime == Crime::None); //if not following or sees
				}
				else
				{
					shouldDecrease = !info.follow || (info.lastMinCrime == Crime::None && pathFollow.reachedTarget == true); //if not following or sees
				}

				
			}
			else
			{
				shouldDecrease = !info.follow && !info.sees;
			}

			if (shouldDecrease)
			{
				info.detection_progress -= 0.5f * Time::DeltaTimeF;
				info.detection_progress = std::max(0.0f, info.detection_progress);

				if (info.detection_progress <= 0.0f)
				{
					info.seesAndDetected = false;
				}

				if (info.follow && info.detection_progress <= 0.01f)
				{
					StopTargetFollow(info.id);
				}
			}
		}

		// if currently seeing and fully detected, add a short delay for lastSeen updates
		if (info.sees && info.detection_progress >= 1.0f)
		{
			info.stopUpdateLastSeenPositionDelay.AddDelay(1.0f);
		}

		// Forget unseen entries
		if (Time::GameTime - info.lastSeenTime > forgetTime)
		{
			it = knownTargets.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Select primary target and copy to old single-target fields (keeps downstream code unchanged)
	SelectPrimaryAndCopy();
}

void NpcBase::SelectPrimaryAndCopy()
{
	

	std::string bestId = "";
	Crime bestCrime = Crime::None;
	float bestDist = std::numeric_limits<float>::max();
	int bestPriority = -1; // Higher = more important


	// Find the best target based on priority
	for (const auto& pair : knownTargets)
	{
		const auto& info = pair.second;

		// Calculate priority score (higher = more important)
		int priority = 0;

		if (info.attack) {
			priority = 3; // Highest: actively attacking
		}
		else if (info.underArrest && info.follow) {
			priority = 2; // High: arresting and following
		}
		else if (info.follow) {
			priority = 1; // Medium: following but not arresting
		}
		else if (info.underArrest) {
			priority = 0; // Low: knows about crime but not following
		}
		else {
			continue; // Skip targets we're not actively dealing with
		}

		float dist = distance(Position, info.lastSeenPosition);

		// Priority comparison: higher priority wins
		if (priority > bestPriority ||
			(priority == bestPriority && info.currentCrime < bestCrime) ||
			(priority == bestPriority && info.currentCrime == bestCrime && dist < bestDist) ||
			bestId.empty())
		{
			bestPriority = priority;
			bestCrime = info.currentCrime;
			bestId = pair.first;
			bestDist = dist;
		}
	}
	


	if (bestId.empty())
	{
		// clear primary
		target_id = "";
		target_follow = false;
		target_lastSeenPosition = vec3();
		target_stopUpdateLastSeenPositionDelay = Delay();
		target_lastSeenTime = -1;
		target_sees = false;
		target_underArrest = false;
		target_attack = false;
		target_underArrestExpire = 5.0f;

		currentCrime = Crime::None;
		detection_progress = 0.0f;
	}
	else
	{
		const auto& info = knownTargets[bestId];
		target_id = bestId;
		target_follow = info.follow;
		target_lastSeenPosition = info.lastSeenPosition;
		target_stopUpdateLastSeenPositionDelay = info.stopUpdateLastSeenPositionDelay;
		target_lastSeenTime = info.lastSeenTime;
		target_sees = info.seesAndDetected;
		target_underArrest = info.underArrest;
		target_attack = info.attack;
		target_underArrestExpire = info.underArrestExpire;

		currentCrime = info.currentCrime;
		detection_progress = info.detection_progress;
	}
	
}

bool NpcBase::IsNeutral(const std::shared_ptr<ObservationTarget>& target) const
{
	for (const auto& t : target->tags)
	{
		if (neutralTags.contains(t)) return true;
	}
	return false;
}

bool NpcBase::IsHostile(const std::shared_ptr<ObservationTarget>& target) const
{
	for (const auto& t : target->tags)
	{
		if (hostileTags.count(t)) return true;
	}
	return false;
}

void NpcBase::MoveToScheduledTask()
{

	float scheduleTime = Time::GameTime;

}

void NpcBase::UpdateScheduledTask()
{

	if (SimulationId.empty()) return;

	auto SimulationStateRef = NpcSimulationManager::GetSimulationStateRef(SimulationId);


	if (scheduledTask != SimulationStateRef->currentTask)
	{

		scheduledTask = SimulationStateRef->currentTask;

		DesiredTask = scheduledTask;

		//StartTask(SimulationStateRef->currentTask);

	}

}

void NpcBase::UpdateObservationTarget()
{

	observationTarget->tags.clear();

	observationTarget->active = true; // needToInvestigateBody || (target_attack && target_follow);
	observationTarget->isTriggeredNpc = false;

	if (dead)
	{
		observationTarget->position = FromPhysics(mesh->FindHitboxByName("pelvis")->GetPosition()) + vec3(0, 0.5, 0);
	}
	else
	{
		observationTarget->position = Position + vec3(0, 0.5, 0);
	}

	if (dead)
	{
		if (needToInvestigateBody)
		{
			observationTarget->tags.insert("body");

			observationTarget->active = true;

		}
	}

	else
	{
		if (isGuard)
		{
			observationTarget->tags = { "guard" };
		}
		else
		{
			observationTarget->tags = { "civilian" };
		}

		if (target_underArrest && target_follow || needHelpStunned)
		{

			//observationTarget->tags.insert("in_trouble");
			//observationTarget->active = true;
			//observationTarget->isTriggeredNpc = true;
		}
		else
		{
			if (isGuard)
			{
				observationTarget->tags.emplace("guard_safe");
			}
		}

	}

	if (dead)
	{
		observationTarget->tags.insert("dead");
	}

	observationTarget->tags.insert(fractionTag);

}


void NpcBase::UpdateTargetFollow()
{


	if (observer)
	{

		if (target_follow || currentInvestigation == InvestigationReason::WeaponFire)
		{
			observer->fovDeg = 200;
		}
		else if (currentInvestigation <= InvestigationReason::Body)
		{
			observer->fovDeg = 170;
		}
		else
		{
			observer->fovDeg = 120;
		}

	}

	auto& info = knownTargets[target_id];

	speed = ((target_follow) || currentInvestigation <= InvestigationReason::Body) ? 5 : 2;

	if (isGuard && target_sees && target_attackInRange && target_attack)
	{
		speed = 2;
	}


	if (target_follow || report_to_guard)
	{
		FindClosestGuard();
	}

	if (target_follow == false) return;

	if (target_underArrest && observer)
	{

		if (observer->id % 3 == EngineMain::MainInstance->frame % 3)
		{
			auto observers = AiPerceptionSystem::GetObserversInRadius(observer->position, isGuard ? 13 : 6);

			for (auto ob : observers)
			{
				NpcBase* npcRef = dynamic_cast<NpcBase*>(ob->ownerPtr);

				if (npcRef == nullptr)
					npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(ob->owner));

				if (npcRef)
				{
					ShareTargetKnowlageWith(npcRef);
				}

			}
		}

	}
	else if(target_attack == false && currentCrime == Crime::Trespassing)
	{

	}

	auto targetRef = Level::Current->FindEntityWithId(target_id);

	target_attackInRange = distance(targetRef->Position, Position) < attackRange;

	if (target_attack && target_sees)
	{
		currentInvestigation = InvestigationReason::None;
	}


	if (target_underArrest && target_follow && target_sees && target_attack == false)
	{
		if (distance2(targetRef->Position, Position) < 15)
		{
			float decrease_rate = 0.5f;
			info.underArrestExpire -= decrease_rate * Time::DeltaTimeF;
		}

		if (target_underArrestExpire < 0)
		{

			info.attack = true;

			PlayPhrace("arrest_final");
		}
	}


	if (target_stopUpdateLastSeenPositionDelay.Wait())
	{
		auto targetRef = Level::Current->FindEntityWithId(target_id);

		target_lastSeenPosition = targetRef->Position;
		target_lastSeenTime = Time::GameTime;

	}

}

void NpcBase::UpdateTargetAttack()
{

	vec3 desiredSpineRotation = vec3();

	if (target_follow)
	{
		auto targetRef = Level::Current->FindEntityWithId(target_id);

		vec3 lookAtTargetVector = normalize(targetRef->Position - Position);

		vec3 localVector = MathHelper::RotateVector(lookAtTargetVector, mesh->Rotation * -1.0f);

		desiredSpineRotation = MathHelper::FindLookAtRotation(vec3(), localVector);

	}

	spineRotation = mix(spineRotation, desiredSpineRotation, Time::DeltaTimeF * 10);

	if (auto stateRef = GetSimulationStateRef())
	{
		stateRef->dead = true;
	}


	if (target_attack && target_follow && target_sees)
	{
		if (attackPositionUpdateDelay.Wait() == false)
		{

			attackPosition = FindAttackLocation();

			attackPositionUpdateDelay.AddDelay(1.5f + RandomHelper::RandomFloat() * 2);
		}
	}


	if (target_follow == false || target_sees == false || target_attack == false || target_attackInRange == false || isStunned() || DoingTask)
	{
		attackDelay.AddDelay(0.8f);
		return;
	}

	if (attackDelay.Wait()) return;

	if (length(controller->GetVelocity()) > 2.2f)
	{

		attackDelay.AddDelay(0.25f + RandomHelper::RandomFloat() * 0.1f);

		return;
	}

	vec3 targetLocation = Level::Current->FindEntityWithId(target_id)->Position;

	if (CheckAttackLOS(Position, targetLocation) == false)
	{
		attackDelay.AddDelay(0.15f + RandomHelper::RandomFloat() * 0.1f);
		return;
	}

	animator.weapon_pendingAttack = true;
	attackDelay.AddDelay(0.5f + RandomHelper::RandomFloat() * 0.1f);

	auto targetRef = Level::Current->FindEntityWithId(target_id);

	vec3 predictedTargetPosition = targetRef->Position;

	const float bulletSpeed = 50;

	if (targetRef)
	{

		Player* playerRef = dynamic_cast<Player*>(targetRef);

		if (playerRef)
		{
			predictedTargetPosition += playerRef->controller.GetVelocity() * distance(Position, targetRef->Position) / bulletSpeed;
		}
		else
		{
			if (targetRef->LeadBody)
			{
				predictedTargetPosition += FromPhysics(targetRef->LeadBody->GetLinearVelocity()) * distance(Position, targetRef->Position) / bulletSpeed;
			}
		}

		
	}

	vec3 bulletRotation = MathHelper::FindLookAtRotation(vec3(), predictedTargetPosition - Position);

	Bullet* bullet = (Bullet*)Spawn("bullet");
	bullet->LoadAssets();
	bullet->Position = weaponMesh->Position;
	bullet->Rotation = bulletRotation;
	bullet->Speed = bulletSpeed;
	bullet->OwnerTag = fractionTag;
	bullet->Damage = 5;
	bullet->owner = this;
	bullet->Start();

}

NpcSimulationState* NpcBase::GetSimulationStateRef()
{

	return NpcSimulationManager::GetSimulationStateRef(SimulationId);

}

bool NpcBase::CheckAttackLOS(vec3 location, vec3 targetLocation)
{


	if (distance(location, targetLocation) < 1.5f) return true;

	vec3 attackDir = normalize(targetLocation - location) * 1.0f;

	vec3 attackPos = location + vec3(0, 0.6f, 0) + attackDir;

	auto hit = Physics::SphereTrace(attackPos, targetLocation + vec3(0, 0.65, 0), 0.1f, BodyType::GroupHitTest, mesh->hitboxBodies);

	if (hit.hasHit && hit.entity->Id != target_id)
	{

		return false;

	}

	return true;
}

bool NpcBase::CheckAttackLocation(vec3 location, vec3 targetLocation)
{

	if (distance(location, targetLocation) > attackDesiredRange)
	{
		return false;
	}

	if (CheckAttackLOS(location, targetLocation))
		return true;



	return true;
}

vec3 NpcBase::FindAttackLocation()
{
	// (unchanged - uses target_id primary)
	// Detection parameters
	const float tooCloseThreshold = 2.0f;

	// Candidate generation parameters
	const int maxIterations = 100;
	const int normalTargetCandidates = 20;
	const int tooCloseTargetCandidates = 30;
	const float preferredMaxMove = 5.0f;
	const float absoluteMaxMove = 8.0f;

	// Ring multipliers
	struct Ring {
		float minMult;
		float maxMult;
	};
	const std::vector<Ring> rings = {
		{0.85f, 1.15f}, // Primary: optimal range
		{0.65f, 0.85f}, // Secondary: closer
		{0.40f, 0.65f}  // Tertiary: evasive close
	};

	// Normal mode angle biases (probabilities cumulative)
	const float normalLeftFlankProb = 0.35f;
	const float normalRightFlankProb = 0.35f;
	const float normalBackProb = 0.20f;
	const float normalFrontProb = 0.10f; // Remaining
	const float normalLeftFlankAngleBase = -90.0f;
	const float normalLeftFlankAngleVar = 80.0f;
	const float normalRightFlankAngleBase = 90.0f;
	const float normalRightFlankAngleVar = 80.0f;
	const float normalBackAngleBase = 180.0f;
	const float normalBackAngleVar = 120.0f;
	const float normalFrontAngleBase = 0.0f;
	const float normalFrontAngleVar = 40.0f;

	// Too close mode angle biases (probabilities cumulative)
	const float tooCloseDirectAwayProb = 0.60f;
	const float tooCloseLeftFlankProb = 0.20f;
	const float tooCloseRightFlankProb = 0.20f; // Remaining
	const float tooCloseDirectAwayAngleBase = 0.0f;
	const float tooCloseDirectAwayAngleVar = 20.0f;
	const float tooCloseLeftFlankAngleBase = 60.0f;
	const float tooCloseLeftFlankAngleVar = 60.0f;
	const float tooCloseRightFlankAngleBase = -60.0f;
	const float tooCloseRightFlankAngleVar = 60.0f;

	// Path checking parameters
	const float segmentDivisor = 2.5f;
	const int maxSegments = 10;

	// Fallback parameters
	const float fallbackDist = 4.0f;
	const float fallbackDistMult = 0.8f;

	// Scoring parameters - common
	const float distSigma = 0.15f;
	const float randomScoreMult = 0.25f;
	const float moveScoreWeight = 1.8f;
	const float circleScoreWeight = 1.1f;

	// Scoring parameters - normal
	const float normalMovePeak = 0.3f;
	const float normalMoveSigma = 0.25f;
	const float normalFlankWeight = 3.2f;
	const float normalDistWeight = 2.1f;

	// Scoring parameters - too close
	const float tooCloseMovePeak = 0.7f;
	const float tooCloseMoveSigma = 0.35f;
	const float tooCloseFlankWeight = 1.8f;
	const float tooCloseDistWeight = 3.2f;

	// Clustering penalty parameters
	const float clusterSigma = 2.0f;
	const float clusterWeight = 2.5f;

	// Scatter parameters to break perfect circle
	const float scatterRadiusMult = 0.15f;

	const vec3& currentPos = Position;
	auto targetEntity = Level::Current->FindEntityWithId(target_id);
	const vec3& targetPos = targetEntity->Position;
	const vec3& targetRot = targetEntity->Rotation;
	vec3 targetForward = MathHelper::GetForwardVector(targetRot);
	targetForward.y = 0.0f;
	targetForward = MathHelper::Normalized(targetForward);
	vec3 targetRight = MathHelper::Normalized(glm::cross(targetForward, vec3(0.0f, 1.0f, 0.0f)));

	float currentDistXZ = glm::length(MathHelper::XZ(currentPos - targetPos));
	bool isTooClose = currentDistXZ < tooCloseThreshold;

	vec3 referenceForward = targetForward;
	vec3 referenceRight = targetRight;

	int targetCandidatesLocal = normalTargetCandidates;
	if (isTooClose) {
		referenceForward = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));
		referenceRight = MathHelper::Normalized(glm::cross(referenceForward, vec3(0.0f, 1.0f, 0.0f)));
		targetCandidatesLocal = tooCloseTargetCandidates;
	}

	// Collect positions of other NPCs targeting the same target
	std::vector<vec3> otherNpcPositions;

	for (auto ob : AiPerceptionSystem::GetObserversInRadius(Position, 30))
	{
		otherNpcPositions.push_back(ob->position - vec3(0, 1.3f, 0));

	}

	std::vector<vec3> candidates;

	int iter = 0;
	const float desired = attackDesiredRange;

	bool relaxedMoveConstraint = false;
	for (const auto& ring : rings) {
		while (candidates.size() < static_cast<size_t>(targetCandidatesLocal) && iter < maxIterations) {
			++iter;

			// Biased relative angle generation (target-relative flanking or evade)
			float r = RandomHelper::RandomFloat();
			float relAngleDeg;
			if (isTooClose) {
				// Evade: 60% direct away, 20% left flank away, 20% right flank away
				if (r < tooCloseDirectAwayProb) {
					relAngleDeg = tooCloseDirectAwayAngleBase + (RandomHelper::RandomFloat() - 0.5f) * tooCloseDirectAwayAngleVar;
				}
				else if (r < tooCloseDirectAwayProb + tooCloseLeftFlankProb) {
					relAngleDeg = tooCloseLeftFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * tooCloseLeftFlankAngleVar;
				}
				else {
					relAngleDeg = tooCloseRightFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * tooCloseRightFlankAngleVar;
				}
			}
			else {
				if (r < normalLeftFlankProb) { // 35% left flank
					relAngleDeg = normalLeftFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalLeftFlankAngleVar;
				}
				else if (r < normalLeftFlankProb + normalRightFlankProb) { // 35% right flank
					relAngleDeg = normalRightFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalRightFlankAngleVar;
				}
				else if (r < normalLeftFlankProb + normalRightFlankProb + normalBackProb) { // 20% back
					relAngleDeg = normalBackAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalBackAngleVar;
				}
				else { // 10% front
					relAngleDeg = normalFrontAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalFrontAngleVar;
				}
			}

			float relAngleRad = MathHelper::ToRadians(relAngleDeg);
			vec3 dirToPos = MathHelper::Normalized(
				std::cos(relAngleRad) * referenceForward +
				std::sin(relAngleRad) * referenceRight
			);

			float distMult = ring.minMult + RandomHelper::RandomFloat() * (ring.maxMult - ring.minMult);
			float dist = desired * distMult;
			dist = std::min(dist, desired); // Ensure <= desiredRange for CheckAttackLocation
			vec3 newPos = targetPos + dirToPos * dist;

			// Add scatter to break perfect circle
			float scatterRadius = desired * scatterRadiusMult;
			vec3 scatter = RandomHelper::RandomPosition(scatterRadius);
			scatter.y = 0.0f;
			newPos += scatter;
			if (glm::length(MathHelper::XZ(newPos - targetPos)) > desired) continue;

			// Check movement distance constraint (prefer close, relax if needed)
			float moveDist = glm::distance(newPos, currentPos);
			float maxMove = relaxedMoveConstraint ? absoluteMaxMove : preferredMaxMove;
			if (moveDist > maxMove) continue;

			// Direct path check
			auto pathHit = Physics::CylinderTrace(currentPos, newPos, 0.5f, 0.8f, BodyType::World | BodyType::MainBody);
			if (pathHit.hasHit) continue;

			// Intermediate LOS checks on subdivided path
			bool pathLOSClear = true;
			vec3 diff = newPos - currentPos;
			float pathLen = glm::length(diff);
			int segments = 1 + static_cast<int>(pathLen / segmentDivisor);
			segments = std::min(maxSegments, segments);
			for (int s = 1; s < segments; ++s) {
				float t = static_cast<float>(s) / static_cast<float>(segments);
				vec3 midPos = currentPos + diff * t;
				if (!CheckAttackLocation(midPos, targetPos)) {
					pathLOSClear = false;
					break;
				}
			}
			if (!pathLOSClear) continue;

			// Final LOS check
			if (!CheckAttackLocation(newPos, targetPos)) continue;

			candidates.push_back(newPos);
		}

		// If not enough candidates after a ring, relax move constraint for subsequent rings
		if (candidates.size() < static_cast<size_t>(targetCandidatesLocal / 2)) {
			relaxedMoveConstraint = true;
		}
		if (candidates.size() >= static_cast<size_t>(targetCandidatesLocal / 2)) break;
	}

	// Ultimate fallback
	if (candidates.empty()) {
		if (isTooClose) {
			// Perpendicular escape when too close
			vec3 radialDir = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));
			vec3 perpRight = MathHelper::Normalized(glm::cross(radialDir, vec3(0.0f, 1.0f, 0.0f)));
			vec3 fallbackDirs[2] = { perpRight, -perpRight };
			float fallbackDistLocal = std::min(fallbackDist, desired * fallbackDistMult);
			for (int i = 0; i < 2; ++i) {
				vec3 fallbackPos = currentPos + fallbackDirs[i] * fallbackDistLocal;
				auto h = Physics::CylinderTrace(currentPos, fallbackPos, 0.5f, 0.8f, BodyType::World | BodyType::MainBody);
				if (!h.hasHit && CheckAttackLocation(fallbackPos, targetPos)) {
					return fallbackPos;
				}
			}
			return currentPos; // Stay if no escape
		}
		else {
			return targetPos;
		}
	}

	// Advanced scoring system
	vec3 bestPos = candidates[0];
	float bestScore = -std::numeric_limits<float>::max();

	vec3 currRelDir = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));

	float movePeak = isTooClose ? tooCloseMovePeak : normalMovePeak;
	float moveSigma = isTooClose ? tooCloseMoveSigma : normalMoveSigma;
	float flankWeight = isTooClose ? tooCloseFlankWeight : normalFlankWeight;
	float distWeight = isTooClose ? tooCloseDistWeight : normalDistWeight;

	for (const vec3& cand : candidates) {
		vec3 toTarget = MathHelper::XZ(cand - targetPos);
		float cDist = glm::length(toTarget);
		vec3 attackDir = (cDist > 0.001f) ? toTarget / cDist : vec3(0.0f, 0.0f, 1.0f);

		// Tactical alignment score: flank vs target or away alignment (higher when aligned/opposite)
		float tacticalScore = -glm::dot(attackDir, referenceForward);

		// Distance score: Gaussian peak at desired range
		float distDiff = (cDist - desired) / desired;
		float distScore = std::exp(-distDiff * distDiff / (2.0f * distSigma * distSigma));

		// Movement score: Gaussian favoring appropriate movement distance
		float moveDistScore = glm::distance(cand, currentPos);
		float moveNorm = moveDistScore / desired;
		float moveScore = std::exp(-(moveNorm - movePeak) * (moveNorm - movePeak) / (2.0f * moveSigma * moveSigma));

		// Circling score: prefer tangential movement
		vec3 moveDir = MathHelper::Normalized(MathHelper::XZ(cand - currentPos));
		float circleScore = 1.0f - std::abs(glm::dot(moveDir, currRelDir));

		// Randomness for unpredictability
		float randScore = RandomHelper::RandomFloat() * randomScoreMult;

		// Clustering penalty: penalize proximity to other NPCs' current positions
		float clusterPenalty = 0.0f;
		for (const vec3& otherPos : otherNpcPositions) {
			float d = glm::distance(cand, otherPos);
			if (d < 0.001f) continue;
			clusterPenalty += std::exp(-(d * d) / (2.0f * clusterSigma * clusterSigma));
		}

		// Weighted total score
		float score = tacticalScore * flankWeight +
			distScore * distWeight +
			moveScore * moveScoreWeight +
			circleScore * circleScoreWeight +
			randScore -
			clusterPenalty * clusterWeight;

		if (score > bestScore) {
			bestScore = score;
			bestPos = cand;
		}
	}

	return bestPos;
}

void NpcBase::UpdateAnimations(bool forceFullUpdate)
{

	//if (dead) return;


	if (isGuard)
	{
		animator.weapon_holds = target_underArrest && !isStunned();
		animator.weapon_ready = target_follow && target_underArrest && !isStunned();
		animator.weapon_aims = target_attack && animator.weapon_ready && target_attackInRange && !isStunned();

		animator.spineRotation = spineRotation;

	}
	else
	{
		animator.scared = (target_follow && target_attack) || (report_to_guard && currentInvestigation <= InvestigationReason::WeaponFire) || (report_to_guard && found_guard == false);
	}

	if (EngineMain::MainInstance->SimulatingGameTicks)
		mesh->WasRended = false;

	if (mesh->WasRended || forceFullUpdate)
	{
		animator.UpdatePose = mesh->WasRended && mesh->InRagdoll == false || forceFullUpdate;

		if (controller != nullptr)
		{
			animator.movementSpeed = length(controller->GetVelocity());
		}

		animator.Update();

		auto pose = animator.GetResultPose();

		mesh->PasteAnimationPose(pose);
	}
	if (forceFullUpdate == false)
		animator.UsePrecomputedFrames = true;

}

void NpcBase::LoadAssets()
{

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/VO.bank");

	mesh->LoadFromFile(modelPath);
	mesh->DepthPrePath = false;
	mesh->Masked = true;
	//mesh->Transparent = true;

	mesh->LoadMetaFromFile("GameData/models/npc/base.glb.skmm");
	mesh->CreateHitboxes(this);

	weaponMesh->LoadFromFile("GameData/models/weapons/glock.glb");
	weaponMesh->TexturesLocation = "GameData/models/weapons/glock.glb/";
	weaponMesh->PreloadAssets();
	weaponMesh->DepthPrePath = false; //reduce driver overhead

	getFromRagdollAnimation = new Animation(this);
	getFromRagdollAnimation->LoadFromFile("GameData/animations/npc/standUp.glb");

	animator.LoadAssetsIfNeeded();

	//mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");



}

void NpcBase::ShareTargetKnowlageWith(NpcBase* anotherNpc)
{

	if (anotherNpc->fractionTag != fractionTag && friendlyTags.count(anotherNpc->fractionTag) == 0) return;

	// Generalized sharing across knownTargets
	for (const auto& pair : knownTargets)
	{
		const std::string& tId = pair.first;
		const auto& myInfo = pair.second;

		bool hasChanges = false;

		if (anotherNpc->knownTargets.count(tId))
		{
			const auto& otherInfo = anotherNpc->knownTargets.at(tId);

			if (myInfo.underArrest && !otherInfo.underArrest) hasChanges = true;
			if (myInfo.currentCrime < otherInfo.currentCrime) 
				hasChanges = true;
			if (myInfo.underArrestExpire < otherInfo.underArrestExpire - 0.2f) hasChanges = true;
			if (myInfo.attack && !otherInfo.attack) 
				hasChanges = true;
			if (myInfo.lastSeenTime > otherInfo.lastSeenTime + 0.2f) hasChanges = true;
			if (myInfo.follow && myInfo.underArrest && !otherInfo.follow) hasChanges = true;
			if (myInfo.sees && !otherInfo.sees) hasChanges = true;
			if (myInfo.detection_progress >= 1 && otherInfo.detection_progress < 1) hasChanges = true;
		}
		else
		{
			// New target, share if active
			if (myInfo.follow || myInfo.underArrest || myInfo.attack) hasChanges = true;
		}

		if (!hasChanges) continue;

		if (distance(Position, anotherNpc->Position) > 2)
			if (Physics::LineTrace(Position, anotherNpc->Position, BodyType::WorldOpaque).hasHit) 
				continue;

		shareKnowlageWith.push_back(anotherNpc);
		knowlageSharedThisFrame++;
	}
}

void NpcBase::ShareTargetKnowlageWithFinal(NpcBase* anotherNpc)
{

	for (const auto& pair : knownTargets)
	{
		const std::string& tId = pair.first;
		const auto& myInfo = pair.second;

		if (anotherNpc->knownTargets.count(tId) == 0)
		{
			anotherNpc->knownTargets[tId] = TargetInfo();
			anotherNpc->knownTargets[tId].id = tId;
		}

		auto& otherInfo = anotherNpc->knownTargets[tId];  // creates if missing

		if (myInfo.follow && myInfo.underArrest && !otherInfo.follow)
		{
			otherInfo.follow = true;
		}

		if (myInfo.underArrest)
		{
			otherInfo.underArrest = true;
		}

		if (myInfo.underArrestExpire < otherInfo.underArrestExpire)
		{
			otherInfo.underArrestExpire = myInfo.underArrestExpire;
		}

		if (myInfo.attack)
		{
			otherInfo.attack = true;
			otherInfo.underArrestExpire = -1.0f;
			otherInfo.follow = true;
			otherInfo.underArrest = true;
		}

		if (myInfo.lastSeenTime > otherInfo.lastSeenTime)
		{
			otherInfo.lastSeenPosition = myInfo.lastSeenPosition;
			otherInfo.lastSeenTime = myInfo.lastSeenTime;
			otherInfo.stopUpdateLastSeenPositionDelay.AddDelay(0.5);
		}

		if (myInfo.currentCrime < otherInfo.currentCrime)
		{
			otherInfo.currentCrime = myInfo.currentCrime;
		}

		if (myInfo.detection_progress >= 1 && otherInfo.detection_progress < 1)
		{
			otherInfo.detection_progress = 1;
		}
	}
}

bool NpcBase::TryCommitCrime(Crime crime, std::string offender, vec3 pos)
{
	// Ensure we have memory for this offender
	auto& info = knownTargets[offender];
	info.id = offender;

	if (crime >= info.currentCrime) return false;

	if (isGuard == false)
	{
		if (crime < Crime::Group_Arrest)
		{
			info.follow = true;
			report_to_guard = true;
		}
	}


	bool wasUnderArrest = info.underArrest;
	bool wasAttack = info.attack;


	info.currentCrime = crime;

	info.stopUpdateLastSeenPositionDelay.AddDelay(0.5f);

	if (crime < Crime::Group_Attack)
	{
		info.underArrest = true;
		info.attack = true;
		info.underArrestExpire = -1.0f;
	}

	if (crime < Crime::Group_Arrest)
	{
		info.underArrest = true;
		info.follow = true;

		if (isGuard == false)
		{
			report_to_guard = true;
		}

	}

	if (crime < Crime::Group_Follow && isGuard)
	{

		if (crime == Crime::Trespassing)
		{
			PlayPhrace("shots_fired");
		}

		info.follow = true;
	}

	if (info.attack && !wasAttack)
	{
		PlayPhrace("arrest_final");
	}
	else if (info.underArrest && !wasUnderArrest)
	{
		if (crime == Crime::WeaponHolding || crime == Crime::WeaponFireSound)
		{
			PlayPhrace("weapon_surrender");
		}
		else if (crime == Crime::NearBody)
		{
			PlayPhrace("arrest");
		}
	}

	// Ensure primary copies are updated if this offender became primary
	SelectPrimaryAndCopy();

	return true;
}

void NpcBase::StopTargetFollow()
{
	StopTargetFollow(target_id);
}

void NpcBase::StopTargetFollow(const std::string& id)
{
	if (id.empty()) return;

	// if we have per-target memory, update it; otherwise fallback to clearing primary
	if (knownTargets.count(id))
	{
		auto& info = knownTargets[id];

		if (info.underArrest)
		{
			PlayPhrace("target_lost");
		}

		info.follow = false;
		info.sees = false;
		info.lastSeenPosition = vec3();
		info.currentCrime = Crime::None;
		pathFollow.reachedTarget = false;
		pathFollow.FoundTarget = true;
	}
	else
	{
		// fallback to previous behaviour
		if (target_underArrest)
		{
			PlayPhrace("target_lost");
		}
		target_follow = false;
		target_sees = false;
		target_lastSeenPosition = vec3();
		pathFollow.reachedTarget = false;
		pathFollow.FoundTarget = true;
		currentCrime = Crime::None;
	}

	// If primary was this id, clear primary copies
	if (target_id == id)
	{
		target_id = "";
		target_follow = false;
		target_sees = false;
		target_underArrest = false;
		currentCrime = Crime::None;
		detection_progress = 0.0f;
	}
}

void NpcBase::Serialize(json& target)
{

	pathFollow.WaitToFinish();

	Entity::Serialize(target);


	animationStateSaveData = mesh->GetAnimationState();

	getFromRagdollAnimationSaveState = getFromRagdollAnimation->GetAnimationState();

	Rotation = mesh->Rotation;

	SERIALIZE_FIELD(target, Rotation);
	SERIALIZE_FIELD(target, desiredDirection);
	SERIALIZE_FIELD(target, movingDirection);
	SERIALIZE_FIELD(target, desiredLookVector);
	SERIALIZE_FIELD(target, speed);
	SERIALIZE_FIELD(target, dead);
	SERIALIZE_FIELD(target, animationStateSaveData);
	SERIALIZE_FIELD(target, getFromRagdollAnimationSaveState);
	SERIALIZE_FIELD(target, ragdollPose);

	SERIALIZE_FIELD(target, pathFollow.acceptanceRadius);
	SERIALIZE_FIELD(target, pathFollow.CalculatedTargetLocation);
	SERIALIZE_FIELD(target, pathFollow.FoundTarget);
	SERIALIZE_FIELD(target, pathFollow.reachedTarget);
	SERIALIZE_FIELD(target, pathFollow.CalculatedPath);
	SERIALIZE_FIELD(target, pathFollow.CurrentPath);


	SERIALIZE_FIELD(target, taskState);

	// serialize primary copies (for compatibility)
	SERIALIZE_FIELD(target, target_follow);
	SERIALIZE_FIELD(target, target_id);
	SERIALIZE_FIELD(target, target_lastSeenPosition);
	SERIALIZE_FIELD(target, target_lastSeenTime);
	SERIALIZE_FIELD(target, target_stopUpdateLastSeenPositionDelay);
	SERIALIZE_FIELD(target, target_sees);
	SERIALIZE_FIELD(target, target_underArrest);
	SERIALIZE_FIELD(target, target_attack);
	SERIALIZE_FIELD(target, target_underArrestExpire);
	SERIALIZE_FIELD(target, target_attackInRange);

	SERIALIZE_FIELD(target, report_to_guard);
	SERIALIZE_FIELD(target, found_guard);
	SERIALIZE_FIELD(target, closestGuard);

	SERIALIZE_FIELD(target, currentInvestigation);
	SERIALIZE_FIELD(target, investigation_target);
	SERIALIZE_FIELD(target, investigation_targetId);
	SERIALIZE_FIELD(target, investigation_changed);
	SERIALIZE_FIELD(target, needToInvestigateBody);

	SERIALIZE_FIELD(target, currentCrime);

	SERIALIZE_FIELD(target, flee_target);

	// Save behavior tree state
	btSaveState = behaviorTree.SaveState().dump(0);
	SERIALIZE_FIELD(target, btSaveState);

	// primary detection progress (compat)
	SERIALIZE_FIELD(target, detection_progress);

	SERIALIZE_FIELD(target, needHelpStunned);
	SERIALIZE_FIELD(target, stunnedRagdoll);
	SERIALIZE_FIELD(target, stunnedRagdollDelay);
	SERIALIZE_FIELD(target, returningFromRagdoll);

	SERIALIZE_FIELD(target, attackDelay);
	SERIALIZE_FIELD(target, attackPositionUpdateDelay);
	SERIALIZE_FIELD(target, attackPosition);

	SERIALIZE_FIELD(target, movementLockDelay);

	SERIALIZE_FIELD(target, DoingTask);
	SERIALIZE_FIELD(target, DoingTaskOld);
	SERIALIZE_FIELD(target, DesiredTask);

	SERIALIZE_FIELD(target, scheduledTask);

	SERIALIZE_FIELD(target, SimulationId);


	AnimationState taskAnimationState;

	taskAnimationState = animator.taskAnimation->GetAnimationState();

	SERIALIZE_FIELD(target, taskAnimationState);

	// Serialize knownTargets map
	json ktJson = json::object();
	for (const auto& pair : knownTargets)
	{
		json infoJson;
		pair.second.Serialize(infoJson);
		ktJson[pair.first] = infoJson;
	}
	target["knownTargets"] = ktJson;

	std::string currentMap = Level::Current->mapName;

	SERIALIZE_FIELD(target, currentMap);

	// Optional: save tag lists and forgetTime if modified by design (left default if unchanged)
}

void NpcBase::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, Rotation);
	DESERIALIZE_FIELD(source, desiredDirection);
	DESERIALIZE_FIELD(source, movingDirection);
	DESERIALIZE_FIELD(source, desiredLookVector);
	DESERIALIZE_FIELD(source, speed);
	DESERIALIZE_FIELD(source, dead);
	DESERIALIZE_FIELD(source, animationStateSaveData);
	DESERIALIZE_FIELD(source, getFromRagdollAnimationSaveState);
	DESERIALIZE_FIELD(source, ragdollPose);

	DESERIALIZE_FIELD(source, pathFollow.acceptanceRadius);
	DESERIALIZE_FIELD(source, pathFollow.CalculatedTargetLocation);
	DESERIALIZE_FIELD(source, pathFollow.FoundTarget);
	DESERIALIZE_FIELD(source, pathFollow.reachedTarget);
	DESERIALIZE_FIELD(source, pathFollow.CalculatedPath);
	DESERIALIZE_FIELD(source, pathFollow.CurrentPath);

	DESERIALIZE_FIELD(source, taskState);

	// primary copies
	DESERIALIZE_FIELD(source, target_follow);
	DESERIALIZE_FIELD(source, target_id);
	DESERIALIZE_FIELD(source, target_lastSeenPosition);
	DESERIALIZE_FIELD(source, target_lastSeenTime);
	DESERIALIZE_FIELD(source, target_stopUpdateLastSeenPositionDelay);
	DESERIALIZE_FIELD(source, target_sees);
	DESERIALIZE_FIELD(source, target_underArrest);
	DESERIALIZE_FIELD(source, target_attack);
	DESERIALIZE_FIELD(source, target_underArrestExpire);
	DESERIALIZE_FIELD(source, target_attackInRange);

	DESERIALIZE_FIELD(source, report_to_guard);
	DESERIALIZE_FIELD(source, found_guard);
	DESERIALIZE_FIELD(source, closestGuard);

	DESERIALIZE_FIELD(source, currentInvestigation);
	DESERIALIZE_FIELD(source, investigation_target);
	DESERIALIZE_FIELD(source, investigation_targetId);
	DESERIALIZE_FIELD(source, investigation_changed);
	DESERIALIZE_FIELD(source, needToInvestigateBody);

	DESERIALIZE_FIELD(source, movementLockDelay);

	DESERIALIZE_FIELD(source, currentCrime);

	DESERIALIZE_FIELD(source, flee_target);

	DESERIALIZE_FIELD(source, btSaveState);

	DESERIALIZE_FIELD(source, scheduledTask);

	if (btSaveState.empty() == false)
	{
		behaviorTree.LoadState(json::parse(btSaveState));
	}

	// Deserialize knownTargets map if present
	if (source.contains("knownTargets"))
	{
		const json& ktJson = source["knownTargets"];
		for (auto it = ktJson.begin(); it != ktJson.end(); ++it)
		{
			TargetInfo info;
			info.Deserialize(it.value());
			knownTargets[it.key()] = info;
		}
	}

	if(HasTag("npc") == false)
		Tags.push_back("npc");

	if (HasTag(fractionTag) == false)
		Tags.push_back(fractionTag);

	controller->SetPosition(Position);



	if (dead)
	{
		
		if (controller)
		{
			delete controller;
			controller = nullptr;
		}

		/*
		DeathSoundPlayer->Destroy();
		HurtSoundPlayer->Destroy();
		StunSoundPlayer->Destroy();
		AttackSoundPlayer->Destroy();
		DeathSoundPlayer = nullptr;
		HurtSoundPlayer = nullptr;
		StunSoundPlayer = nullptr;
		AttackSoundPlayer = nullptr;
		*/

	}


	mesh->Rotation = Rotation;

	if (source.contains("animationStateSaveData"))
	{
		mesh->SetAnimationState(animationStateSaveData);
		if (getFromRagdollAnimation)
			getFromRagdollAnimation->SetAnimationState(getFromRagdollAnimationSaveState);
	}


	//mesh->Update(0);
	//mesh->PullRootMotion();

	DESERIALIZE_FIELD(source, detection_progress);

	DESERIALIZE_FIELD(source, needHelpStunned);
	DESERIALIZE_FIELD(source, stunnedRagdoll);
	DESERIALIZE_FIELD(source, stunnedRagdollDelay);
	DESERIALIZE_FIELD(source, returningFromRagdoll);

	DESERIALIZE_FIELD(source, attackDelay);
	DESERIALIZE_FIELD(source, attackPositionUpdateDelay);
	DESERIALIZE_FIELD(source, attackPosition);

	AnimationState taskAnimationState;
	DESERIALIZE_FIELD(source, taskAnimationState);

	if(animator.taskAnimation)
		animator.taskAnimation->SetAnimationState(taskAnimationState);

	DESERIALIZE_FIELD(source, DoingTask);
	DESERIALIZE_FIELD(source, DoingTaskOld);
	DESERIALIZE_FIELD(source, DesiredTask);

	DESERIALIZE_FIELD(source, SimulationId);

	// Ensure primary is consistent with knownTargets
	SelectPrimaryAndCopy();

	std::string currentMap = "";

	DESERIALIZE_FIELD(source, currentMap);

	if (currentMap != Level::Current->mapName)
	{
		taskState = TaskState();
		scheduledTask = "";
		DesiredTask = "";
		DoingTask = true;
		DoingTaskOld = true;
		target_id = "";
		target_attack = false;
		target_follow = false;
		animator.taskAnimation->StopAnimation();

		UpdateObserver();
	}

	/*
	if (LevelTraversalSystem::Traveling) //we know that we are being loaded into level and not loaded using save game.
	{

		MoveToScheduledTask();

	}
	*/
}

void NpcBase::PrepareToStartMovement()
{

	pathFollow.WaitToFinish();
	pathFollow.CalculatedPath = false;
	pathFollow.reachedTarget = false;
	pathFollow.isPerformingDelay.AddDelay(-1);

}

void NpcBase::StopMovement()
{

	pathFollow.WaitToFinish();
	pathFollow.CalculatedPath = true;
	pathFollow.reachedTarget = true;
	//desiredDirection = vec3(0);

}

void NpcBase::MoveTo(const vec3& target, float acceptanceRadius)
{
	desiredTargetLocation = target;
	pathFollow.acceptanceRadius = acceptanceRadius;

}

void NpcBase::BodyInvestigated()
{
	needToInvestigateBody = false;
}

void NpcBase::Task_TargetReached()
{

	if (taskState.HasToMoveToTarget == false) return;

	TaskPoint* taskPoint = dynamic_cast<TaskPoint*>(Level::Current->FindEntityWithName(taskState.TaskName));

	if (taskPoint == nullptr)
		return;

	taskState.HasToMoveToTarget = false;

	taskPoint->OnNpcTargetReached(this);


}

void NpcBase::Task_Doint()
{

	DoingTask = true;

}

void NpcBase::Task_NotDoing()
{
	DoingTask = false;
}

void NpcBase::Task_DoStationaryJob()
{
}

TaskState& NpcBase::GetTaskStateRef()
{
	return taskState;
}

void NpcBase::StartTask(const std::string& taskName)
{

	DesiredTask = taskName;

	StopTask();

	TaskPoint* taskPoint = dynamic_cast<TaskPoint*>(Level::Current->FindEntityWithName(taskName));

	if (taskPoint == nullptr)
		return;

	taskPoint->NpcEntered(this);

}

void NpcBase::StopTask()
{

	if (taskState.TaskName.empty()) return;

	TaskPoint* taskPoint = dynamic_cast<TaskPoint*>(Level::Current->FindEntityWithName(taskState.TaskName));

	if (taskPoint == nullptr) return;

	taskPoint->NpcExited(this);

	taskState = TaskState();

}

void NpcBase::UpdateTask()
{


	TaskPoint* taskPoint = nullptr;

	if (taskState.TaskName.empty() == false)
	{
		taskPoint = dynamic_cast<TaskPoint*>(Level::Current->FindEntityWithName(taskState.TaskName));
	}

	bool actualDoingTask = DoingTask;

	if (isStunned())
		actualDoingTask = false;


	if (actualDoingTask)
	{
		if (controller != nullptr)
		{
			if (taskState.HasToLockPosition)
			{
				//Physics::SetMotionType(LeadBody, JPH::EMotionType::Kinematic);
				controller->SetPosition(taskState.LockPosition);
				Position = taskState.LockPosition;
			}
			else
			{

			}
		}
	}

	if (taskPoint == nullptr)
	{

		DoingTaskOld = actualDoingTask;

		if (DesiredTask != "")
		{
			StartTask(DesiredTask);
		}


		return;
	}


	if (actualDoingTask != DoingTaskOld)
	{

		if (actualDoingTask)
		{
			taskPoint->NpcReturned(this);
		}
		else
		{
			taskPoint->NpcInterrupted(this);
		}

	}
	else
	{

		if (actualDoingTask)
		{

			if (target_follow || report_to_guard || currentInvestigation != InvestigationReason::None || taskState.TaskName != DesiredTask)
			{
				taskPoint->NpcTryInterrupt(this);
			}

		}

	}

	auto desTask = DesiredTask;

	if (actualDoingTask)
	{
		taskPoint->NpcUpdate(this);
	}

	DesiredTask = desTask;

	DoingTaskOld = actualDoingTask;


	if (DesiredTask != "")
	{
		if (taskState.DoingJob == false && taskState.TaskName != DesiredTask)
		{
			StartTask(DesiredTask);
		}
	}
	

}

void NpcBase::UpdateDebugUI()
{

	if (btEditorEnabled)
	{
		editor.Draw();
	}

	ImGui::Begin(Id.c_str());

	ImGui::Text("detection:");
	ImGui::SameLine();
	ImGui::ProgressBar(detection_progress);

	ImGui::DragFloat("activeragdoll strength", &mesh->RagdollPoseFollowStrength, 0.1f);
	ImGui::Checkbox("update ragdoll pose", &mesh->UpdateRagdollPose);

	ImGui::Checkbox("bt editor", &btEditorEnabled);

	ImGui::End();


	auto hit = Physics::LineTrace(Camera::position, Camera::position + Camera::Forward() * 500.0f, BodyType::CharacterCapsule, {}, {Player::Instance});

	if (hit.entity == this)
	{

		ImGui::Begin("look at npc info");

		ImGui::DragFloat("Health", &Health);

		ImGui::Checkbox("Follow", &target_follow);

		ImGui::InputText(
			"Target ID",
			&target_id
		);

		ImGui::InputFloat3(
			"Last Seen Position",
			&target_lastSeenPosition.x
		);

		ImGui::InputFloat(
			"Last Seen Time",
			&target_lastSeenTime
		);

		ImGui::Checkbox("Sees Target", &target_sees);
		ImGui::Checkbox("Under Arrest", &target_underArrest);
		ImGui::Checkbox("Attack", &target_attack);

		ImGui::InputFloat(
			"Under Arrest Expire",
			&target_underArrestExpire
		);

		ImGui::Checkbox("Attack In Range", &target_attackInRange);

		ImGui::LabelText("task", taskState.TaskName.c_str());

		ImGui::LabelText("fov", to_string(observer->fovDeg).c_str());

		ImGui::End();

	}

}

void NpcBase::FindClosestGuard()
{

	if (isGuard) return;

	if (findGuardCooldown.Wait()) return;

	found_guard = false;

	std::vector<std::shared_ptr<ObservationTarget>> npcsInRadius;

	if (report_to_guard)
		npcsInRadius = AiPerceptionSystem::GetTargetsInRadiusWithTagOrdered(Position, 150, "guard_safe");

	

	for (auto& npc : npcsInRadius)
	{

		auto& foundID = npc->ownerId;

		NpcBase* guardRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(foundID));

		if (guardRef == nullptr) continue;

		if (guardRef->hostileTags.count(fractionTag)) continue; // hostile to us

		closestGuard = npc->ownerId;
		found_guard = true;
		break;
	}


	if(found_guard == false)
	{

		if (target_follow == false && currentInvestigation == InvestigationReason::Body)
		{
			FinishInvestigation();
			return;
		}

		closestGuard = "";


		auto fleePath = NavigationSystem::FindFleePathSimple(Position, target_lastSeenPosition, target_follow ? 20 : 60, 4, speed);

		if (fleePath.size() > 0)
		{
			flee_target = fleePath[fleePath.size() - 1];
		}
		else
		{
			flee_target = normalize(Position - target_lastSeenPosition) * 10.0f;
		}

	}

	if (EngineMain::MainInstance->SimulatingGameTicks)
	{
		findGuardCooldown.AddDelay(2.3 + RandomHelper::RandomFloat());
	}
	else
	{
		findGuardCooldown.AddDelay(0.3 + RandomHelper::RandomFloat() / 3.0f);
	}



}

void NpcBase::TryStartInvestigation(InvestigationReason reason, vec3 target, string causer, bool sharedByNpc)
{

	if (target_follow && reason == InvestigationReason::WeaponFire && causer == target_id)
	{
		target_stopUpdateLastSeenPositionDelay.AddDelay(0.5f);
	}

	if (target_follow && target_underArrest == false && reason > InvestigationReason::Body)
	{
		return;
	}

	if (target_follow && target_underArrest)
	{
		return;
	}

	if (isGuard == false && reason == InvestigationReason::NpcInTrouble) return;

	if (reason >= currentInvestigation)
	{
		return;
	}

	investigation_changed = true;

	currentInvestigation = reason;
	investigation_target = target;
	investigation_targetId = causer;

	if (reason == InvestigationReason::LoudNoise || reason == InvestigationReason::Noise)
	{

		if (sharedByNpc == false)
			PlayPhrace("heard_sound");
	}

	if (reason == InvestigationReason::WeaponFire)
	{
		PlayPhrace("shots_fired");
	}

	if (reason < InvestigationReason::LoudNoise && isGuard == false)
	{
		report_to_guard = true;

		FindClosestGuard();

		if (found_guard == false)
		{

			if (currentInvestigation == InvestigationReason::Body)
			{
				FinishInvestigation();
			}
			else if (currentInvestigation == InvestigationReason::WeaponFire)
			{
				target_lastSeenPosition = investigation_target;
			}


		}


	}
}

void NpcBase::FinishInvestigation()
{

	if (currentInvestigation == InvestigationReason::NpcInTrouble)
	{
		if (investigation_targetId.empty() == false)
		{
			NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(investigation_targetId));

			if (npcRef)
			{
				npcRef->needHelpStunned = false;
			}

		}
	}

	if (isGuard)
	{
		if (currentInvestigation == InvestigationReason::Body)
		{
			if (investigation_targetId.empty() == false)
			{
				NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(investigation_targetId));

				if (npcRef)
				{
					npcRef->BodyInvestigated();
					PlayPhrace("dead_body");
				}

			}
		}
	}
	else
	{

		if (currentInvestigation == InvestigationReason::Body && found_guard == false)
		{
			if (investigation_targetId.empty() == false)
			{
				NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(investigation_targetId));

				if (npcRef)
				{
					npcRef->BodyInvestigated();
					PlayPhrace("dead_body");
				}

			}
		}

		if (found_guard)
		{
			auto guardRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(closestGuard));

			if (currentInvestigation != InvestigationReason::None)
			{

				if (guardRef)
				{
					guardRef->TryStartInvestigation(currentInvestigation, investigation_target, Id, true);
					report_to_guard = false;
				}

			}
			else
			{
				if (guardRef->target_lastSeenTime < target_lastSeenTime + 0.5f)
				{
					guardRef->target_lastSeenTime = target_lastSeenTime;
					guardRef->target_lastSeenPosition = target_lastSeenPosition;
					guardRef->TryStartInvestigation(InvestigationReason::TargetSeen, target_lastSeenPosition, Id, true);

				}
			}

		}

	}

	movementLockDelay.AddDelay(3);

	currentInvestigation = InvestigationReason::None;
	investigation_target = vec3();
	report_to_guard = false;
}

REGISTER_ENTITY(NpcBase, "npc_base")