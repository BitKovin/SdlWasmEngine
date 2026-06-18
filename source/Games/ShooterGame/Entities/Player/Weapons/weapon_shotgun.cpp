#include "WeaponFirearm.h"

class weapon_shotgun : public WeaponFirearm {
public:
    weapon_shotgun() : WeaponFirearm() 
    {
        params.ammoType = WeaponAmmoType::ShotgunShells;
        params.modelPath = "GameData/models/player/weapons/shotgun/shotgun.glb";
        params.modelPathTp = "GameData/models/player/weapons/shotgun/shotgun_tp.glb";
        params.texturesLocation = "GameData/models/player/weapons/shotgun/shotgun.glb/";
        params.fireSoundEvent = "GameData/sounds/weapons/shotgun/shotgun_fire2.wav";
        params.useOneshotSound = false;
        params.baseSpread = 2.2f * 0.7f;
        params.hasActiveSpread = false;
        params.attackDelayTime = 1.0f; // Note: Added in PerformAttack, but set here
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.2f;
        params.weaponOffset = vec3(0.013, 0.013, -0.096);
        params.bulletSpeed = 200.0f; // Assumed same
        params.bulletDamage = 60.0f / 21.0f;
        params.range = 70.0f;
        params.fireAnimation = "attack";
        params.fireAnimInterpInTime = 0.15f; // No speed specified, assume default
        params.notifyNpcs = true;
        params.activateViolenceCrime = false;
        params.hasRecoilModelOffset = false;
        params.hasRandomRecoilStrength = false;
        params.recoilShake = CameraShake(
            0.13f,                            // interpIn
            1.2f,                             // duration
            vec3(0.0f, 0.0f, -0.2f),          // positionAmplitude
            vec3(0.0f, 0.0f, 6.4f),           // positionFrequency
            vec3(-8, 0.15f, 0.0f),            // rotationAmplitude
            vec3(-5.0f, 28.8f, 0.0f),         // rotationFrequency
            1.2f,                             // falloff
            CameraShake::ShakeType::SingleWave// shakeType
        );
        params.spreadType = "grid";
        params.gridSpreadSize = 4.0f * 0.7f;
        params.gridStep = 2.0f * 0.7f;
        params.gridMaxLength = 4.7f * 0.7f;

        params.debuffOnHit = "PoiseBreakDebuff";
		params.debuffStacksOnHit = 100 / 21.0f;

    }

    void Update() override {
        WeaponFirearm::Update();
    }

    void PerformAttack() override {
        if (attackDelay.Wait()) return;
        WeaponFirearm::PerformAttack();
    }

    void LateUpdate() override 
    {
        WeaponFirearm::LateUpdate();

		const float perspectiveScale = 0.82f;

        viewmodel->Scale.z = perspectiveScale;
		viewmodelLeft->Scale.z = perspectiveScale;
		arms->Scale.z = perspectiveScale;
		armsLeft->Scale.z = perspectiveScale;
	}

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_shotgun";
        data.slot = 1;
        return data;
    }
};

REGISTER_ENTITY(weapon_shotgun, "weapon_shotgun")