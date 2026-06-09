#pragma once
#include "WeaponBase.h"
#include "../Player.hpp"
#include <Animation.h>
#include <SoundSystem/FmodEventInstance.h>
#include "../../Enemy/IEnemy.h"
#include <ParticleSystems/particle_system_meleeTrail.hpp>

class weapon_twinsword : public Weapon
{
public:

	// Right-hand sword + corresponding arms
	SkeletalMesh* viewmodel_r = nullptr;
	SkeletalMesh* arms_r = nullptr;

	// Left-hand sword + corresponding arms
	SkeletalMesh* viewmodel_l = nullptr;
	SkeletalMesh* arms_l = nullptr;

	vec3 weaponOffset = vec3(0.0f);

	Delay attackDelay;
	Delay pendingAttackStartDelay;
	Delay pendingAttackEndDelay;
	Delay reAttackDelay;
	Delay startTrailDelay;

	Delay parrySpamWindow;
	Delay blockStartDelay;
	Delay counterWindow; // how long the counter-attack opportunity stays open

	particle_system_meleeTrail* trail = nullptr;
	SkeletalMesh* trailViewmodel = nullptr; // which viewmodel the active trail tracks

	// Attack state
	int  attackSide = 0;     // 0 = right next, 1 = left next
	bool pendingAttack = false;
	bool soundToggle = false;
	bool counterAvailable = false; // set true after a successful parry

	// Block state
	bool isBlocking = false;

	const float Damage = 15.0f;

	SoundPlayer* fireSoundPlayer = nullptr;
	SoundPlayer* fireSoundPlayer2 = nullptr;
	SoundPlayer* hitSoundPlayer = nullptr;

	weapon_twinsword()
	{
		LateUpdateWhenPaused = true;
		startTrailDelay.AddDelay(100000000.0f); // inert until StartAttack arms it
		SupportsOffhandWeapon = false;
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

		viewmodel_r = new SkeletalMesh(owner);
		arms_r = new SkeletalMesh(owner);
		viewmodel_l = new SkeletalMesh(owner);
		arms_l = new SkeletalMesh(owner);

		for (auto* vm : { viewmodel_r, viewmodel_l })
		{
			vm->GravityAlignedRotation = true;
			vm->LoadFromFile("GameData/models/player/weapons/twinsword/twinsword.glb");
			vm->TexturesLocation = "GameData/models/player/weapons/twinsword/twinsword.glb/";
			vm->PlayAnimation("draw", false, 0.0f);
			vm->PreloadAssets();
			vm->IsViewmodel = true;
			Drawables.push_back(vm);
		}

		for (auto* a : { arms_r, arms_l })
		{
			a->GravityAlignedRotation = true;
			a->LoadFromFile(ArmsModelPath);
			a->IsViewmodel = true;
			Drawables.push_back(a);
		}

		trailViewmodel = viewmodel_r;
	}

	// -----------------------------------------------------------------------
	// Helpers
	// -----------------------------------------------------------------------

	// Play an animation on both viewmodels simultaneously (draw, block, parry, etc.)
	void PlayBoth(const std::string& anim, bool loop, float blend)
	{
		viewmodel_r->PlayAnimation(anim, loop, blend);
		viewmodel_l->PlayAnimation(anim, loop, blend);
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
		if (trailViewmodel == nullptr) return;

		trail = new particle_system_meleeTrail();
		vec3 trailStart = trailViewmodel->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = trailViewmodel->GetBoneMatrixWorld("trail_end")[3];

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

		if (trail == nullptr || trailViewmodel == nullptr) return;

		// Feed the two blade endpoints from the active sword's animated skeleton
		vec3 trailStart = trailViewmodel->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = trailViewmodel->GetBoneMatrixWorld("trail_end")[3];

		trail->SetTrailTransform(trailStart, trailEnd);
	}

	// -----------------------------------------------------------------------
	// Attack
	// -----------------------------------------------------------------------

	void StartAttack()
	{
		if (attackDelay.Wait() || isBlocking) return;

		if (counterAvailable)
		{
			// Counter-attack immediately following a successful parry
			counterAvailable = false;

			PlayBoth("attack_counter", false, 0.1f);
			trailViewmodel = viewmodel_r; // anchor trail to right sword for counter

			attackDelay.AddDelay(0.5f);
			pendingAttackStartDelay.AddDelay(0.1f);
			pendingAttackEndDelay.AddDelay(0.45f);

			Camera::AddCameraShake(CameraShake(
				1.5f, 1.5f, vec3(0), vec3(0),
				vec3(-7, 0, 0), vec3(10, 10, 0),
				1.0f, CameraShake::ShakeType::SingleWave
			));
		}
		else
		{
			// Reset to right if the re-attack window has expired


			const bool  isRight = (attackSide == 0);
			auto* const activeVm = isRight ? viewmodel_r : viewmodel_l;

			PlayBoth(isRight ? "attack2_r" : "attack2_l", false, 0.1f);

			//activeVm->PlayAnimation(isRight ? "attack2_r" : "attack2_l", false, 0.1f);
			trailViewmodel = activeVm;

			attackDelay.AddDelay(0.5f);
			pendingAttackStartDelay.AddDelay(0.1f);
			pendingAttackEndDelay.AddDelay(0.3f);

			Camera::AddCameraShake(CameraShake(
				1.0f, 1.0f, vec3(0), vec3(0),
				isRight ? vec3(-5, -5, 0) : vec3(-5, 5, 0),
				vec3(7, 7, 0),
				1.0f, CameraShake::ShakeType::SingleWave
			));

			attackSide = isRight ? 1 : 0; // alternate for next hit
		}

		// Kill the previous trail and queue a new one
		StopTrail();
		startTrailDelay.AddDelay(0.15f);

		soundToggle ? fireSoundPlayer->Play() : fireSoundPlayer2->Play();
		soundToggle = !soundToggle;

		pendingAttack = true;
		reAttackDelay.AddDelay(0.55f);
	}

	// Called every frame while the hit window is open.
	// pendingAttack is only cleared on a successful entity hit.
	void PerformAttack()
	{
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
			hit.entity->OnPointDamage(
				Damage,
				hit.position,
				MathHelper::GetForwardVector(Camera::rotation),
				"spine_03",
				owner,
				this
			);
			pendingAttack = false; // stop re-tracing once an entity is hit
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
		if (counterWindow.Wait()) return;

		isBlocking = true;
		pendingAttack = false; // cancel any in-flight swing

		PlayBoth("block_start", false, 0.1f);

		blockStartDelay.AddDelay(0.2f);

		// Parry window is animation-time driven (see Update).
		// Spam check is handled via parrySpamWindow set in EndBlock.
	}

	void EndBlock()
	{
		// Arm the spam window so the next block press can't instantly parry
		parrySpamWindow.AddDelay(0.4f);

		isBlocking = false;

		PlayBoth("block_stop", false, 0.1f);

		attackDelay.AddDelay(0.3f);
	}

	// Called by the engine when an enemy attack lands during the parry window
	void OnParried() override
	{
		SoundPlayer::PlayOneshot("event:/Weapons/knife/knife_attack", 2.0f, 1.0f, false);

		EndBlock();

		PlayBoth("parry", false, 0.05f);

		// Open the counter-attack opportunity
		counterAvailable = true;
		counterWindow.AddDelay(0.5f);
	}

	// -----------------------------------------------------------------------
	// Update
	// -----------------------------------------------------------------------

	void Update() override
	{
		// Primary attack / counter
		if (Input::GetAction("attack")->PressedBuffered() && !isBlocking)
			StartAttack();

		// Block: hold to block, release to lower guard
		if (Input::GetAction("block")->PressedBuffered(0.1f) && !isBlocking)
			StartBlock();

		if (!Input::GetAction("block")->Holding() && isBlocking)
			EndBlock();

		// Resolve hit each frame while inside the valid window:
		//   - pendingAttackStartDelay elapsed  → blade has reached the hit zone
		//   - pendingAttackEndDelay still alive → blade hasn't passed through yet
		if (pendingAttack && !pendingAttackStartDelay.Wait() && pendingAttackEndDelay.Wait())
			PerformAttack();

		// Expire the counter opportunity when its timer runs out
		if (!counterWindow.Wait())
			counterAvailable = false;

		// Parry window: active while block_start animation is still playing.
		// Uses animation time so the window is exactly as wide as the startup anim.
		// Anti-spam: disabled for 0.4s after each block release.
		bool blockAnimPlaying =
			isBlocking &&
			viewmodel_r->GetAnimationTime() < viewmodel_r->GetAnimationDuration();

		Parrying = blockAnimPlaying && !parrySpamWindow.Wait();
		Blocking = (isBlocking && !blockStartDelay.Wait()) || parrySpamWindow.Wait();
	}

	void AsyncUpdate() override
	{
		viewmodel_r->Update();
		viewmodel_l->Update();

		const float hide = 1.0f;

		// arms_r: driven by viewmodel_r, right arm shown — hide left clavicle
		{
			auto pose = viewmodel_r->GetAnimationPose();
			auto bone = pose.GetBoneTransform("clavicle_l");
			bone.Rotation += vec3(120, 0, 0) * hide;
			bone.Scale *= mix(vec3(1.0f), vec3(0.0f), hide);
			pose.SetBoneTransformEuler("clavicle_l", bone);
			viewmodel_r->PasteAnimationPose(pose);
			arms_r->PasteAnimationPose(pose);
		}

		// arms_l: driven by viewmodel_l, left arm shown — hide right clavicle
		{
			auto pose = viewmodel_l->GetAnimationPose();
			auto bone = pose.GetBoneTransform("clavicle_r");
			bone.Rotation += vec3(120, 0, 0) * hide;
			bone.Scale *= mix(vec3(1.0f), vec3(0.0f), hide);
			pose.SetBoneTransformEuler("clavicle_r", bone);
			viewmodel_l->PasteAnimationPose(pose);
			arms_l->PasteAnimationPose(pose);
		}
	}

	void LateUpdate() override
	{
		const vec3 pos = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		const vec3 rot = Rotation;

		viewmodel_r->Position = pos;
		viewmodel_r->Rotation = rot;
		arms_r->Position = pos;
		arms_r->Rotation = rot;

		viewmodel_l->Position = pos;
		viewmodel_l->Rotation = rot;
		arms_l->Position = pos;
		arms_l->Rotation = rot;

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
		data.className = "weapon_twinsword";
		return data;
	}
};

REGISTER_ENTITY(weapon_twinsword, "weapon_twinsword")