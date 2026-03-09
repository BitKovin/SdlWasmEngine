// weapon_sniper.cpp (modified)
#include "WeaponFirearm.h"
#include "Animators/Animator_Rifle.h"

class weapon_sniper : public WeaponFirearm {
public:
    bool Silencer = false;
    float aimProgress = 0.0f;
    vec3 weaponAimOffset = vec3(0.062707, 0.033706, -0.0f);

    weapon_sniper() : WeaponFirearm() {
        params.modelPath = "GameData/models/player/weapons/sniper/sniper.glb";
        params.texturesLocation = "GameData/models/player/weapons/sniper/sniper.glb/";
        params.modelPathTp = "GameData/models/player/weapons/sniper/sniper_tp.glb";
        params.texturesLocationTp = "GameData/models/player/weapons/sniper/sniper_tp.glb/";
        params.fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
        params.useOneshotSound = false;
        params.pitchModifier = 1.0f; // Modified in Update based on Silencer
        params.baseSpread = 0.0f;
        params.hasActiveSpread = false;
        params.attackDelayTime = 1.3f;
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.5f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 70.0f;
        params.range = 80.0f;
        params.fireAnimation = "fire";
        params.fireAnimInterpInTime = 0.08f;
        params.notifyNpcs = true;
        params.activateViolenceCrime = true;
        params.hasRecoilModelOffset = false;
        params.hasRandomRecoilStrength = false;
        params.recoilShake = CameraShake(
            0.13f,                             // interpIn
            0.5f,                              // duration
            vec3(0.0f, 0.0f, -0.1f),           // positionAmplitude
            vec3(0.0f, 0.0f, 3.4f),            // positionFrequency
            vec3(-4, 0.15f, 0.0f),             // rotationAmplitude
            vec3(-2.0f, 18.8f, 0.0f),          // rotationFrequency
            0.5f,                              // falloff
            CameraShake::ShakeType::SingleWave // shakeType
        );
        bobScale = 0.0f;

        thirdPersonAnimator = make_unique<Animator_Rifle>(this);

    }

    void Update() override {
        if (Input::GetAction("attack2")->Holding() && CanAttack()) {
            aimProgress += Time::DeltaTimeF * 3.0f;
        }
        else {
            aimProgress -= Time::DeltaTimeF * 3.0f;
        }

        if (aimProgress > weaponAim)
        {
            weaponAim = aimProgress;
        }

        aimProgress = glm::clamp(aimProgress, 0.0f, 1.0f);
        bobScale = 1.0f - aimProgress;

        params.weaponOffset = mix(vec3(0.0, 0.00, 0.0), weaponAimOffset, aimProgress);
        Camera::FOV = mix(65.0f, 10.0f, aimProgress);


        ForceFirstPerson = aimProgress > 0.9;


        if (aimProgress > 0.01f) SwitchDelay.AddDelay(0.1f);

        params.pitchModifier = Silencer ? 2.8f : 1.0f;

        WeaponFirearm::Update();
    }

    void UpdateDebugUI() override {
        ImGui::Begin("Sniper Weapon Debug");
        ImGui::Checkbox("Silencer", &Silencer);
        ImGui::DragFloat3("Weapon Offset", &params.weaponOffset.x, 0.01f);
        ImGui::End();
    }

    void LateUpdate() override {
        params.weaponOffset.z = mix(0.0f, 0.02f, aimProgress);

        WeaponFirearm::LateUpdate();

        viewmodel->Scale.z = mix(1.0f, 0.02f, aimProgress);
        arms->Scale = viewmodel->Scale;

        viewmodel->MeshHideList.clear();
        if (Silencer) {
            viewmodel->MeshHideList.insert("muzzle");
        }
        else {
            viewmodel->MeshHideList.insert("silencer");
        }
    }

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_sniper";
        data.slot = 3;
        return data;
    }
};

REGISTER_ENTITY(weapon_sniper, "weapon_sniper")