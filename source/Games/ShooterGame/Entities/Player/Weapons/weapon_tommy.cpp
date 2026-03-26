#include "WeaponFirearm.h"

class weapon_tommy : public WeaponFirearm {
public:
    weapon_tommy() : WeaponFirearm() {
        params.modelPath = "GameData/models/player/weapons/tommy/tommy.glb";
        params.texturesLocation = "GameData/models/player/weapons/tommy/tommy.glb/";
        params.fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
        params.useOneshotSound = true;
        params.pitchModifier = 1.2f;
        params.fireVolume = 0.4f;
        params.baseSpread = 1.0f;
        params.hasActiveSpread = false;
        params.attackDelayTime = 0.15f;
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.09f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 8.0f;
        params.range = 80.0f;
        params.fireAnimation = "fire";
        params.fireAnimInterpInTime = 0.08f;
        params.activateViolenceCrime = false;
        params.hasRecoilModelOffset = false;
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
    }

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_tommy";
        data.slot = 2;
        return data;
    }
};

REGISTER_ENTITY(weapon_tommy, "weapon_tommy")