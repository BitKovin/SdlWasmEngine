#include "WeaponFirearm.h"
#include "Animators/Animator_Akimbo.h"
#include "Animators/Animator_OneHand.h"

#include <Entities/Player/Player.hpp>

class weapon_cannon : public WeaponFirearm {
public:

    weapon_cannon() : WeaponFirearm()
    {
		params.bulletClass = "cannon_bullet";
        params.ammoType = WeaponAmmoType::CannonBullets;
        params.modelPath = "GameData/models/player/weapons/cannon/cannon.glb";
        params.texturesLocation = "GameData/models/player/weapons/cannon/cannon.glb/";
        params.modelPathTp = "GameData/models/player/weapons/cannon/cannon_tp.glb";
        params.texturesLocationTp = "GameData/models/player/weapons/pistol/pistol_tp.glb/";
        params.fireSoundEvent = "event:/Weapons/cannon/cannon_fire";
        params.useOneshotSound = false;
        params.pitchModifier = 1.0f; // Modified in Update based on Silencer
        params.baseSpread = 0.1f;
        params.spreadIncreasePerShot = 2.1f;
        params.maxActiveSpread = 5.8f;
        params.spreadDecreaseSpeed = 3;
        params.attackDelayTime = 1.4f;
        params.switchDelayTime = 0.2f;
        params.switchDelayOnAttack = 2.1f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 200.0f;
        params.range = 80.0f;
        params.fireAnimation = "fire";
        params.fireAnimInterpInTime = 0.08f;
        params.activateViolenceCrime = false;
        params.hasRecoilModelOffset = false;
        params.hasRandomRecoilStrength = false;
        params.recoilShake = CameraShake(
            0.3f,                             // interpIn
            2.3f,                              // duration
            vec3(0.0f, 0.0f, -0.1f),           // positionAmplitude
            vec3(0.0f, 0.0f, 3.4f),            // positionFrequency
            vec3(-20, 0.15f, 0.0f),             // rotationAmplitude
            vec3(-1.5f, 18.8f, 0.0f),          // rotationFrequency
            0.5f,                              // falloff
            CameraShake::ShakeType::SingleWave // shakeType
        );

        params.debuffOnHit = "QuicksilverDebuff";
        params.debuffStacksOnHit = 55.0f;

        thirdPersonAnimator = make_unique<Animator_OneHand>(this);

        SupportsOffhandWeapon = false;

    }

    void FireSingleBullet(const vec3& startLoc, const vec4& gridOffset = vec4(0)) override
    {

		WeaponFirearm::FireSingleBullet(startLoc, gridOffset);

		vec3 currentVelocity = Player::Instance->controller.GetVelocity();

        vec3 forward = Camera::Forward();
		vec3 recoilVelocity = -forward * 10.0f;

        Player::Instance->jumpDelay.AddDelay(0.3f);

        Player::Instance->controller.SetVelocity(currentVelocity + recoilVelocity);

    }

    WeaponSlotData GetDefaultData() override
    {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_cannon";
        data.slot = 3;
        return data;
    }



};

REGISTER_ENTITY(weapon_cannon, "weapon_cannon")