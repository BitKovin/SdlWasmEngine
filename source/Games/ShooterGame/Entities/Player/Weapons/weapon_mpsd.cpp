#include "WeaponFirearm.h"
#include "Animators/Animator_Rifle.h"

class weapon_mpsd : public WeaponFirearm {
public:
    weapon_mpsd() : WeaponFirearm() {
        params.modelPath = "GameData/models/player/weapons/stg44/stg44.glb";
        params.fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
        params.modelPathTp = "GameData/models/player/weapons/mpsd/mpsd_tp.glb";
        params.texturesLocationTp = "GameData/models/player/weapons/mpsd/mpsd_tp.glb/";
        params.useOneshotSound = true;
        params.pitchModifier = 1.2f;
        params.fireVolume = 0.4f;
        params.baseSpread = 0.15f;
        params.spreadIncreasePerShot = 0.1f;
        params.maxActiveSpread = 1.2f;
        params.velocitySpreadDivisor = 5.5f;
        params.attackDelayTime = 0.13f;
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.09f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 20.0f;
        params.range = 50.0f;
        params.fireAnimation = "fire";
        params.drawAnimation = "reload";
        params.fireAnimInterpInTime = 0.08f;
        params.notifyNpcs = true;
        params.activateViolenceCrime = true;
        params.violenceCrimeDelay = 0.3f;
        params.hasRecoilModelOffset = true;
        params.recoilModelTarget = -2;
        params.recoilModelInterpIn = 2.0f;
        params.recoilModelInterpOut = 7.0f;
        params.hasRandomRecoilStrength = true;
        params.recoilShake = CameraShake(
            0.13f,                            // interpIn
            0.5f,                             // duration
            vec3(0.0f, 0.0f, -0.1f),         // positionAmplitude
            vec3(0.0f, 0.0f, 3.4f),          // positionFrequency
            vec3(-4, 1.15f, 0.0f),           // rotationAmplitude (modified by random)
            vec3(-2.0f, 12.8f, 0.0f),        // rotationFrequency
            0.5f,                            // falloff
            CameraShake::ShakeType::SingleWave // shakeType
        );
		Illegal = true;

        thirdPersonAnimator = make_unique<Animator_Rifle>(this);

    }

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_mpsd";
        data.slot = 2;
        return data;
    }

};

REGISTER_ENTITY(weapon_mpsd, "weapon_mpsd")