#pragma once
#include "WeaponBase.h"
#include "../Player.hpp"
#include <Animation.h>
#include <SoundSystem/FmodEventInstance.h>
#include "../../Enemy/IEnemy.h"
#include <ParticleSystems/particle_system_meleeTrail.hpp>

#include <Entities/Npc/NpcGuardMelee.h>

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

	Delay blockingWindow;

	Delay forceBlockInput;

	Delay parrySpamWindow;
	Delay blockStartDelay;
	Delay counterWindow; // how long the counter-attack opportunity stays open

	particle_system_meleeTrail* trail_r = nullptr;
	particle_system_meleeTrail* trail_l = nullptr;

	// Attack state
	int  attackSide = 0;     // 0 = right next, 1 = left next
	bool pendingAttack = false;
	bool soundToggle = false;
	bool counterAvailable = false; // set true after a successful parry

	bool pendingCounterAttack = false;

	// Block state
	bool isBlocking = false;

	const float Damage = 25.0f;

	SoundPlayer* fireSoundPlayer = nullptr;
	SoundPlayer* fireSoundPlayer2 = nullptr;
	SoundPlayer* hitSoundPlayer = nullptr;

	// -----------------------------------------------------------------------
	// Ultimate mode
	// -----------------------------------------------------------------------

	bool ultimateActive = false;
	const float UltimateSpeedMultiplier = 1.65f;

	bool IsUltimateMode() const { return ultimateActive; }

	// Single source of truth — multiply anim speed, divide delays by this value
	float GetAttackSpeedScale() const
	{
		return IsUltimateMode() ? UltimateSpeedMultiplier : 1.0f;
	}

	// -----------------------------------------------------------------------

	weapon_twinsword()
	{
		LateUpdateWhenPaused = true;
		startTrailDelay.AddDelay(100000000.0f); // inert until StartAttack arms it
		SupportsOffhandWeapon = false;

		thirdPersonModelPath = "GameData/models/player/weapons/twinsword/twinsword_tp.glb";
		weaponHandlingType = 2;
	}

	// -----------------------------------------------------------------------
	// Lifecycle
	// -----------------------------------------------------------------------

	void Start() override
	{
		fireSoundPlayer = SoundPlayer::Create("event:/Weapons/knife/knife_attack");
		fireSoundPlayer2 = SoundPlayer::Create("event:/Weapons/knife/knife_attack");
		hitSoundPlayer = SoundPlayer::Create("");// ("event:/Weapons/knife/knife_hit");

		SwitchDelay.AddDelay(0.35f);
	}

	void LoadAssets() override
	{

		Weapon::LoadAssets();

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

		attackDelay.AddDelay(0.7f);
		SwitchDelay.AddDelay(0.4f);
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

	void StopTrails()
	{
		auto stopOne = [](particle_system_meleeTrail*& t)
			{
				if (t == nullptr) return;
				t->DestroyWithDelay(2.0f);
				t = nullptr;
			};
		stopOne(trail_r);
		stopOne(trail_l);
	}

	// spawnRight / spawnLeft let the caller choose which swords get a trail.
	// Normal attacks pass one true flag; counter-attacks pass both.
	void StartNewTrails(bool spawnRight, bool spawnLeft)
	{
		MathHelper::Transform weaponTransform = MathHelper::Transform();
		weaponTransform.Position = Position;
		weaponTransform.Rotation = Rotation;
		mat4 weaponMatrix = weaponTransform.ToMatrixEuler();
		mat4 inverseWeaponMatrix = glm::inverse(weaponMatrix);

		if (spawnRight)
		{
			trail_r = new particle_system_meleeTrail();
			vec3 trailStart = viewmodel_r->GetBoneMatrixWorld("trailR_start")[3];
			vec3 trailEnd = viewmodel_r->GetBoneMatrixWorld("trailR_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_r->SetTrailTransform(trailStart, trailEnd);
			trail_r->Start();
			Level::Current->AddEntity(trail_r);
			trail_r->LoadAssetsIfNeeded();
		}

		if (spawnLeft)
		{
			trail_l = new particle_system_meleeTrail();
			vec3 trailStart = viewmodel_l->GetBoneMatrixWorld("trailL_start")[3];
			vec3 trailEnd = viewmodel_l->GetBoneMatrixWorld("trailL_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_l->SetTrailTransform(trailStart, trailEnd);
			trail_l->Start();
			Level::Current->AddEntity(trail_l);
			trail_l->LoadAssetsIfNeeded();
		}
	}

	void UpdateTrail()
	{
		// Spawn trails once the start delay has elapsed.
		// pendingTrailRight/Left were set by StartAttack before arming the delay.
		if (!startTrailDelay.Wait())
		{
			StartNewTrails(pendingTrailRight, pendingTrailLeft);
			startTrailDelay.AddDelay(100000000.0f); // re-arm as inert
		}

		// Tell active trails to stop emitting once the hit window is over
		if (!pendingAttackEndDelay.Wait())
		{
			if (trail_r != nullptr) trail_r->StopAll();
			if (trail_l != nullptr) trail_l->StopAll();
		}

		MathHelper::Transform weaponTransform = MathHelper::Transform();
		weaponTransform.Position = Position;
		weaponTransform.Rotation = Rotation;
		mat4 weaponMatrix = weaponTransform.ToMatrixEuler();
		mat4 inverseWeaponMatrix = glm::inverse(weaponMatrix);

		// Feed bone endpoints every frame — null checks mean only active trails update
		if (trail_r != nullptr)
		{
			vec3 trailStart = viewmodel_r->GetBoneMatrixWorld("trailR_start")[3];
			vec3 trailEnd = viewmodel_r->GetBoneMatrixWorld("trailR_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_r->SetTrailTransform(trailStart, trailEnd);
			trail_r->RelativeTransform = weaponMatrix; // anchor to camera space so it follows the viewmodel
		}

		if (trail_l != nullptr)
		{
			vec3 trailStart = viewmodel_l->GetBoneMatrixWorld("trailL_start")[3];
			vec3 trailEnd = viewmodel_l->GetBoneMatrixWorld("trailL_end")[3];

			trailStart = vec3(inverseWeaponMatrix * vec4(trailStart, 1.0f));
			trailEnd = vec3(inverseWeaponMatrix * vec4(trailEnd, 1.0f));

			trail_l->SetTrailTransform(trailStart, trailEnd);
			trail_l->RelativeTransform = weaponMatrix; // anchor to camera space so it follows the viewmodel
		}
	}

	void WarnAboutAttack()
	{
		auto hit = Physics::SphereTrace(
			Camera::position,
			Camera::position + MathHelper::GetForwardVector(Camera::rotation) * 1.5f,
			0.5f,
			BodyType::GroupHitTest,
			{ Player::Instance->LeadBody },
			{ Player::Instance }
		);

		if (hit.hasHit && hit.entity != nullptr)
		{
			if (auto* enemy = dynamic_cast<NpcGuardMelee*>(hit.entity))
			{
				enemy->WarnAboutAttack(owner);
			}
		}
	}

	// -----------------------------------------------------------------------
	// Attack
	// -----------------------------------------------------------------------

	// Remembered between StartAttack and the delayed StartNewTrails call
	bool pendingTrailRight = false;
	bool pendingTrailLeft = false;

	void StartAttack()
	{
		if (counterAvailable)
			pendingCounterAttack = true;

		if (attackDelay.Wait() || isBlocking) return;

		const float s = GetAttackSpeedScale();

		if (counterAvailable)
		{
			pendingCounterAttack = false;

			// Counter-attack: both swords swing — spawn a trail on each
			counterAvailable = false;

			Time::AddTimeScaleEffect(0.3, 0.1, true, "weapon", 0.3f, 0.1f);

			PlayBoth("attack2_counter", false, 0.1f);
			viewmodel_l->SetAnimationTime(0.1f);
			viewmodel_r->SetAnimationTime(0.1f);

			pendingTrailRight = true;
			pendingTrailLeft = true;

			attackDelay.AddDelay(0.5f / s);
			pendingAttackStartDelay.AddDelay(0.15f / s);
			pendingAttackEndDelay.AddDelay(0.45f / s);

			Camera::AddCameraShake(CameraShake(
				1.5f, 1.5f, vec3(0), vec3(0),
				vec3(-7, 0, 0), vec3(10, 10, 0),
				1.0f, CameraShake::ShakeType::SingleWave
			));
		}
		else
		{
			// Normal attack: only the active sword swings — one trail
			const bool isRight = (attackSide == 0);

			PlayBoth(isRight ? "attack2_r" : "attack2_l", false, 0.1f);

			pendingTrailRight = isRight;
			pendingTrailLeft = !isRight;

			attackDelay.AddDelay(0.5f / s);
			pendingAttackStartDelay.AddDelay(0.15f / s);
			pendingAttackEndDelay.AddDelay(0.5f / s);

			Camera::AddCameraShake(CameraShake(
				1.0f, 1.0f, vec3(0), vec3(0),
				isRight ? vec3(-5, 5, 0) : vec3(-5, -5, 0),
				vec3(7, 7, 0),
				1.0f, CameraShake::ShakeType::SingleWave
			));

			attackSide = isRight ? 1 : 0; // alternate for next hit
		}

		// Kill the previous trails and queue new ones
		StopTrails();
		startTrailDelay.AddDelay(0.15f / s);

		soundToggle ? fireSoundPlayer->Play() : fireSoundPlayer2->Play();
		soundToggle = !soundToggle;

		pendingAttack = true;
		reAttackDelay.AddDelay(0.55f / s);

		WarnAboutAttack();
	}

	// Called every frame while the hit window is open.
	// pendingAttack is only cleared on a successful entity hit.
	void PerformAttack()
	{
		auto hit = Physics::SphereTrace(
			Camera::position,
			Camera::position + MathHelper::GetForwardVector(Camera::rotation) * 1.3f,
			0.4f,
			BodyType::GroupHitTest,
			{ Player::Instance->LeadBody },
			{ Player::Instance }
		);

		if (!hit.hasHit) return;

		if (hit.entity != nullptr)
		{
			float damangeToDeal = Damage;

			if (viewmodel_r->GetAnimationName() == "attack2_counter")
			{
				damangeToDeal *= 3.0f;
			}

			float ultimateFinalDamageMultiplier = 2.0f / GetAttackSpeedScale();

			if (IsUltimateMode())
				damangeToDeal *= ultimateFinalDamageMultiplier;

			bool isEnemy = dynamic_cast<IEnemy*>(hit.entity) && hit.entity->Health > 0;

			if (isEnemy)
			{
				Player::Instance->Heal(damangeToDeal * 0.20f
				* IsUltimateMode() ? 1.5f : 1.0f);
			}

			hit.entity->OnPointDamage(
				damangeToDeal,
				hit.position,
				MathHelper::GetForwardVector(Camera::rotation),
				"spine_03",
				owner,
				this
			);

			Physics::AddImpulseAtLocation(hit.hitbody, MathHelper::GetForwardVector(Camera::rotation) * (Damage + 2) * 14.0f, hit.position);

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
		if (viewmodel_r->GetAnimationName() == "block_start" && viewmodel_r->GetAnimationTime() < 0.2f) return;

		// Arm the spam window so the next block press can't instantly parry
		parrySpamWindow.AddDelay(0.3f);

		isBlocking = false;

		if (viewmodel_r->GetAnimationName() == "block_start")
		{
			PlayBoth("block_stop", false, 0.1f);

			attackDelay.AddDelay(0.3f);
		}
	}

	// Called by the engine when an enemy attack lands during the parry window
	void OnParried() override
	{
		SoundPlayer::PlayOneshot("event:/Weapons/cane/cane_parry", 1.0f, 1.0f, false);
		Time::AddTimeScaleEffect(0.3, 0.1, true, "parry", 0.24f, 0.1f);
		EndBlock();

		isBlocking = false;

		forceBlockInput.AddDelay(0.5f);

		PlayBoth("parry", false, 0.05f);

		attackDelay.AddDelay(0.2f);

		// Open the counter-attack opportunity
		counterAvailable = true;
		counterWindow.AddDelay(1.1f);
	}

	// -----------------------------------------------------------------------
	// Update
	// -----------------------------------------------------------------------

	void Update() override
	{
		if (counterWindow.Wait() == false)
			pendingCounterAttack = false;


		// Primary attack / counter
		if ((Input::GetAction("attack")->PressedBuffered(0.3f) || pendingCounterAttack))
			StartAttack();

		// Block: hold to block, release to lower guard
		if (Input::GetAction("attack2")->PressedBuffered(0.1f) && !isBlocking)
			StartBlock();

		if (!Input::GetAction("attack2")->Holding() && isBlocking)
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

		if (blockAnimPlaying)
			blockingWindow.AddDelay(0.1f);

		Parrying = blockAnimPlaying && !parrySpamWindow.Wait() || blockingWindow.Wait();
		Blocking = (isBlocking && !blockStartDelay.Wait()) || parrySpamWindow.Wait();
	}

	void AsyncUpdate() override
	{
		const float speed = GetAttackSpeedScale();
		viewmodel_r->Update(speed);
		viewmodel_l->Update(speed);

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