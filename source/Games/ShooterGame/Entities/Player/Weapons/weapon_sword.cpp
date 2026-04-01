#pragma once
#include "WeaponBase.h"
#include "../Player.hpp"
#include <Animation.h>
#include <SoundSystem/FmodEventInstance.h>
#include "../../Enemy/IEnemy.h"
#include <ParticleSystems/particle_system_meleeTrail.hpp>

class weapon_sword : public Weapon
{
public:

	SkeletalMesh* viewmodel = nullptr;
	SkeletalMesh* arms = nullptr;

	vec3 weaponOffset = vec3(0.0f);

	Delay attackDelay;
	Delay pendingAttackStartDelay;
	Delay pendingAttackEndDelay;
	Delay reAttackDelay;
	Delay startTrailDelay;

	Delay parryWindow;
	Delay parrySpamWindow;

	particle_system_meleeTrail* trail = nullptr;

	// Combo state
	int attack = -1;
	bool hadHit = false;
	bool pendingAttack = false;
	bool soundToggle = false;

	// Block state
	bool isBlocking = false;

	const float Damage = 30.0f;

	SoundPlayer* fireSoundPlayer = nullptr;
	SoundPlayer* fireSoundPlayer2 = nullptr;
	SoundPlayer* hitSoundPlayer = nullptr;

	Delay blockStartDelay;

	weapon_sword()
	{
		LateUpdateWhenPaused = true;
		startTrailDelay.AddDelay(100000000.0f); // start inert — only arms after StartAttack sets it
	}

	// -----------------------------------------------------------------------
	// Lifecycle
	// -----------------------------------------------------------------------

	void Start() override
	{
		fireSoundPlayer = SoundPlayer::Create("event:/Weapons/knife/knife_attack");
		fireSoundPlayer2 = SoundPlayer::Create("event:/Weapons/knife/knife_attack");
		hitSoundPlayer = SoundPlayer::Create("event:/Weapons/knife/knife_hit");

		SwitchDelay.AddDelay(0.35f);
	}

	void LoadAssets() override
	{
		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");

		viewmodel = new SkeletalMesh(owner);
		arms = new SkeletalMesh(owner);

		viewmodel->GravityAlignedRotation = true;
		arms->GravityAlignedRotation = true;

		viewmodel->LoadFromFile("GameData/models/player/weapons/sword/sword.glb");
		viewmodel->TexturesLocation = "GameData/models/player/weapons/sword/sword.glb/";
		viewmodel->PlayAnimation("draw", false, 0.0f);
		viewmodel->PreloadAssets();
		viewmodel->IsViewmodel = true;
		Drawables.push_back(viewmodel);

		arms->LoadFromFile(ArmsModelPath);
		arms->IsViewmodel = true;
		Drawables.push_back(arms);
	}

	// -----------------------------------------------------------------------
	// Trail
	// -----------------------------------------------------------------------

	void StopTrail()
	{
		if (trail == nullptr) return;
		trail->DestroyWithDelay(2.0f);
		trail = nullptr;
	}

	void StartNewTrail()
	{
		trail = new particle_system_meleeTrail();
		vec3 trailStart = viewmodel->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = viewmodel->GetBoneMatrixWorld("trail_end")[3];

		trail->SetTrailTransform(trailStart, trailEnd);
		trail->Start();
		Level::Current->AddEntity(trail);
		trail->LoadAssetsIfNeeded();
	}

	void UpdateTrail()
	{
		// Spawn the trail once the start delay has elapsed
		if (!startTrailDelay.Wait())
		{
			StartNewTrail();
			startTrailDelay.AddDelay(100000000.0f); // re-arm as inert
		}

		// Tell the trail to stop emitting once the hit window is over
		if (!pendingAttackEndDelay.Wait() && trail != nullptr)
			trail->StopAll();

		if (trail == nullptr) return;

		// Anchor the trail to camera space so it follows the viewmodel correctly
		//trail->RelativeTransform = Camera::GetMatrix();

		// Feed the two blade endpoints from the animated skeleton
		vec3 trailStart = viewmodel->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = viewmodel->GetBoneMatrixWorld("trail_end")[3];

		trail->SetTrailTransform(trailStart, trailEnd);
	}

	// -----------------------------------------------------------------------
	// Attack combo
	// -----------------------------------------------------------------------

	void StartAttack()
	{
		if (attackDelay.Wait() || isBlocking) return;

		// Advance or reset the combo counter
		if (reAttackDelay.Wait())
			attack++;
		else
			if (attack >= 0) attack = 0;

		if (attack > 2) attack = 0;

		// Finisher only available if the previous swing actually landed
		if (attack == 2 && !hadHit) attack = 0;

		// Play the correct animation for each combo step
		if (attack == -1)
		{
			viewmodel->PlayAnimation("attack_start", false, 0.0f);
			attackDelay.AddDelay(0.4f);
			pendingAttackStartDelay.AddDelay(0.0f);
			Camera::AddCameraShake(CameraShake(1.0f, 1.0f, vec3(0), vec3(0), vec3(-5, -5, 0), vec3(7, 7, 0), 1.0f, CameraShake::ShakeType::SingleWave));
		}
		else if (attack == 0)
		{
			viewmodel->PlayAnimation("attack", false, 0.1f);
			attackDelay.AddDelay(0.4f);
			pendingAttackStartDelay.AddDelay(0.15f);
			Camera::AddCameraShake(CameraShake(1.0f, 1.0f, vec3(0), vec3(0), vec3(-5, -5, 0), vec3(7, 7, 0), 1.0f, CameraShake::ShakeType::SingleWave));
		}
		else if (attack == 1)
		{
			viewmodel->PlayAnimation("attack2", false, 0.1f);
			attackDelay.AddDelay(0.4f);
			pendingAttackStartDelay.AddDelay(0.15f);
			Camera::AddCameraShake(CameraShake(1.0f, 1.0f, vec3(0), vec3(0), vec3(-5, 5, 0), vec3(7, 7, 0), 1.0f, CameraShake::ShakeType::SingleWave));
		}
		else if (attack == 2)
		{
			viewmodel->PlayAnimation("attack_finish", false, 0.1f);
			attackDelay.AddDelay(0.5f);
			pendingAttackStartDelay.AddDelay(0.15f);
		}

		// Hit-window closes after this delay — attack must land before then
		pendingAttackEndDelay.AddDelay(0.35f);

		// Kill the previous trail immediately and start a new one after a short
		// delay so it begins at the moment the blade actually starts moving
		StopTrail();
		startTrailDelay.AddDelay(0.19f);

		soundToggle ? fireSoundPlayer->Play() : fireSoundPlayer2->Play();
		soundToggle = !soundToggle;

		pendingAttack = true;
		reAttackDelay.AddDelay(0.55f);
	}

	// Called every frame while the hit window is open.
	// pendingAttack is only cleared on a successful entity hit, matching the
	// original C# behaviour (keeps re-tracing until something is touched).
	void PerformAttack()
	{
		hadHit = false;

		auto hit = Physics::SphereTrace(
			Camera::position,
			Camera::position + MathHelper::GetForwardVector(Camera::rotation) * 1.3f,
			0.3f,
			BodyType::GroupHitTest,
			{ Player::Instance->LeadBody },
			{ Player::Instance }
		);

		if (!hit.hasHit) return;

		if (hit.entity != nullptr)
		{
			hit.entity->OnPointDamage(Damage, hit.position, MathHelper::GetForwardVector(Camera::rotation), hit.hitboxName, Player::Instance, this);
			hadHit = true;
			pendingAttack = false; // stop re-tracing once an entity is hit
		}

		auto bodyData = Physics::GetBodyData(hit.hitbody);
		if (bodyData != nullptr)
		{
			/*
			GlobalParticleSystem::EmitAt(
				"hit_" + bodyData->Surface,
				hit.position,
				MathHelper::FindLookAtRotation(vec3(0), hit.normal),
				vec3(10, 10, 10)
			);
			*/
		}

		hitSoundPlayer->Position = hit.position;
		hitSoundPlayer->Play();
	}

	// -----------------------------------------------------------------------
	// Block / parry
	// -----------------------------------------------------------------------

	void StartBlock()
	{

		if (attackDelay.Wait()) return;

		isBlocking = true;
		pendingAttack = false;  // cancel any in-flight swing

		viewmodel->PlayAnimation("block_start", false, 0.1f);

		blockStartDelay.AddDelay(0.2f);

		if(parrySpamWindow.Wait() == false)
			parryWindow.AddDelay(0.5);

	}

	void EndBlock()
	{

		parrySpamWindow.AddDelay(0.4);

		isBlocking = false;

		viewmodel->PlayAnimation("block_end", false, 0.1f);

		attackDelay.AddDelay(0.3f);
	}

	// Called by the engine when an enemy attack lands during a parry window
	void OnParried() override
	{
		//Time::AddTimeScaleEffect(0.3f, 0.1f, true, "parry", 0.02f, 0.1f);
		SoundPlayer::PlayOneshot("event:/Weapons/knife/knife_attack", 2.0f, 1.0f, false);

		// Snap to a stable mid-block frame so the animation doesn't pop
		//viewmodel->SetAnimationTime(0.1f);
	}

	// -----------------------------------------------------------------------
	// Update
	// -----------------------------------------------------------------------

	void Update() override
	{
		// Primary attack
		if (Input::GetAction("attack")->PressedBuffered() && !isBlocking)
			StartAttack();

		// Block: hold to block, release to lower guard
		if (Input::GetAction("block")->Holding() && !isBlocking)
			StartBlock();
			
		if (Input::GetAction("block")->Holding() == false && isBlocking)
			EndBlock();

		// Resolve hit each frame while inside the valid window:
		//   - start delay elapsed   → swing has reached the hit zone
		//   - end delay still alive → swing hasn't passed through yet
		if (pendingAttack && !pendingAttackStartDelay.Wait() && pendingAttackEndDelay.Wait())
			PerformAttack();

		Parrying = parryWindow.Wait();
		Blocking = (isBlocking && blockStartDelay.Wait() == false) || parrySpamWindow.Wait();
	}

	void AsyncUpdate() override
	{
		viewmodel->Update();

		float hide = 1;

		auto pose = viewmodel->GetAnimationPose();
		auto leftHandPose = pose.GetBoneTransform("clavicle_l");
		leftHandPose.Rotation += vec3(120, 0, 0) * hide;

		leftHandPose.Scale *= mix(vec3(1.0f), vec3(0.0f), hide);

		pose.SetBoneTransformEuler("clavicle_l", leftHandPose);
		arms->PasteAnimationPose(pose);
	}

	void LateUpdate() override
	{
		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;

		UpdateTrail();
	}

	// -----------------------------------------------------------------------
	// Misc
	// -----------------------------------------------------------------------

	// Prevent weapon switching mid-combo
	bool CanChangeSlot() override
	{
		return !reAttackDelay.Wait();
	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_sword";
		return data;
	}
};

REGISTER_ENTITY(weapon_sword, "weapon_sword")