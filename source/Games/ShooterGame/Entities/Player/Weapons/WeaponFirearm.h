#pragma once

#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>
#include "Animators/WeaponAnimator.h"

#include <string>

struct FirearmParams {
	std::string modelPath = "";
	std::string texturesLocation = "";
	std::string modelPathTp = "";
	std::string texturesLocationTp = "";
	std::string fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
	std::string fireAnimation = "fire";
	std::string drawAnimation = "draw";
	std::string boneMuzzle = "muzzle";
	float fireVolume = 0.5f;
	bool fireSoundIs2D = true;
	bool useOneshotSound = false;
	float pitchModifier = 1.0f;
	float baseSpread = 0.1f;
	float spreadIncreasePerShot = 0.1f;
	float spreadDecreaseSpeed = 2.0f;
	float maxActiveSpread = 1.2f;
	float velocitySpreadDivisor = 17.0f;
	float attackDelayTime = 0.3f;
	float switchDelayTime = 0.35f;
	float switchDelayOnAttack = 0.2f;
	vec3 weaponOffset = vec3(0.0f, 0.0f, 0.0f);
	float bulletSpeed = 200.0f;
	float bulletDamage = 20.0f;
	int bulletsPerShot = 1;
	float range = 80.0f;
	float muzzleMix = 0.6f;
	float muzzleForwardOffset = 0.1f;
	float fireAnimInterpInTime = 0.08f;
	bool hasActiveSpread = true;
	bool notifyNpcs = false;
	float npcNotifyRadius = 20.0f;
	bool activateViolenceCrime = false;
	float violenceCrimeDelay = 0.3f;
	bool hasRecoilModelOffset = false;
	float recoilModelTarget = 0.0f;
	float recoilModelInterpIn = 2.0f;
	float recoilModelInterpOut = 7.0f;
	bool hasRandomRecoilStrength = false;
	CameraShake recoilShake = CameraShake(
		0.13f, 0.5f,
		vec3(0.0f, 0.0f, -0.1f),
		vec3(0.0f, 0.0f, 3.4f),
		vec3(-3.0f, 0.15f, 0.0f),
		vec3(-1.5f, 18.8f, 0.0f),
		0.5f,
		CameraShake::ShakeType::SingleWave
	);
	bool lateUpdateWhenPaused = true;
	std::string spreadType = "random";
	float gridSpreadSize = 4.0f;
	float gridStep = 2.0f;
	float gridMaxLength = 4.7f;

	std::string debuffOnHit = "";
	float debuffStacksOnHit = 0.0f;

};

class WeaponFirearm : public Weapon {
public:
	FirearmParams params;

	// Right-hand
	SkeletalMesh* viewmodel = nullptr;
	SkeletalMesh* arms = nullptr;

	// Left-hand (always loaded)
	SkeletalMesh* viewmodelLeft = nullptr;
	SkeletalMesh* armsLeft = nullptr;

	Delay attackDelay;
	SoundPlayer* fireSoundPlayer = nullptr;
	float activeSpread = 0.0f;
	float recoilModelOffset = 0.0f;

	SkeletalMesh* thirdPersonModel = nullptr;
	float weaponAim = 0.0f;
	float oldWeaponAim = 0.0f;
	AnimationPose lastAppliedPose;

	std::unique_ptr<WeaponAnimator> thirdPersonAnimator;

	bool fireLeftNext = false;

	// Runtime akimbo state
	bool akimbo = false;
	bool akimboPrev = false;
	bool alternateFire = true;

	WeaponFirearm(const FirearmParams& initialParams = FirearmParams());

	void Start() override;
	void LoadAssets() override;
	void Update() override;
	virtual void PerformAttack();
	virtual void FireSingleBullet(const vec3& startLoc, const vec4& gridOffset = vec4(0));
	void NotifyNpcs();
	void AsyncUpdate() override;
	void LateUpdate() override;
	WeaponSlotData GetDefaultData() override;

	virtual AnimationPose ApplyWeaponAnimation(AnimationPose thirdPersonPose);

	// Runtime akimbo functions
	void SetAkimbo(bool enabled);

protected:

	float akimboDistanceProgress = 0.0f;

};
