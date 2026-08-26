#include "WeaponFirearm.h"
#include "Animators/Animator_Rifle.h"

#include <UI/UiProgressBar.hpp>
#include <EngineMain.h>

class OverheatProgressBar : public UiProgressBar
{
public:

	float overheat = 0.0f;
	bool boosted = false;

    OverheatProgressBar()
    {
		size = vec2(200, 20);
		origin = vec2(0.5f, 0.5f);
        pivot = vec2(0.5f, 0);
		position = vec2(0, 50);
    }

    void Update() override
    {

		UiProgressBar::Update();

		color = boosted ? vec4(1, 0.2f, 0.5f, 1) : vec4(0.5f, 0.5f, 1, 1);
		Progress = overheat;
    }

private:

};


class weapon_mpsd : public WeaponFirearm {
public:

    float overheat = 0.0f;
    bool boosted = false;
    Delay overheatCooldown = Delay();

    static constexpr float BASE_ATTACK_DELAY = 0.13f;
    static constexpr float BOOSTED_ATTACK_DELAY = BASE_ATTACK_DELAY * 0.5f;
    static constexpr float OVERHEAT_DRAIN_RATE = 0.6f; // how fast it drains while boosted

	std::shared_ptr<OverheatProgressBar> progressBar;

    weapon_mpsd() : WeaponFirearm() {
        params.ammoType = WeaponAmmoType::PistolBullets;
        params.modelPath = "GameData/models/player/weapons/stg44/stg44.glb";
        params.fireSoundEvent = "event:/Weapons/pistol/pistol_fire";
        params.modelPathTp = "GameData/models/player/weapons/stg44/stg44_tp.glb";
        params.texturesLocationTp = "GameData/models/player/weapons/stg44/stg44_tp.glb/";
        params.useOneshotSound = true;
        params.pitchModifier = 1.2f;
        params.fireVolume = 0.4f;

        //params.baseSpread = 0.15f;
        //params.spreadIncreasePerShot = 0.05f;

        params.baseSpread = 0.02f;
        params.spreadIncreasePerShot = 0.01f;
        params.maxActiveSpread = 0.7f;
        params.velocitySpreadDivisor = 15.5f;
        params.attackDelayTime = BASE_ATTACK_DELAY;
        params.switchDelayTime = 0.35f;
        params.switchDelayOnAttack = 0.09f;
        params.weaponOffset = vec3(0.0, 0.00, -0.0);
        params.bulletSpeed = 200.0f;
        params.bulletDamage = 15.0f;
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
            0.13f,
            0.5f,
            vec3(0.0f, 0.0f, -0.1f),
            vec3(0.0f, 0.0f, 3.4f),
            vec3(-2, 0.6f, 0.0f),
            vec3(-1.0f, 6.f, 0.0f),
            0.5f,
            CameraShake::ShakeType::SingleWave
        );
        Illegal = true;

        thirdPersonAnimator = make_unique<Animator_Rifle>(this);

        weaponHandlingType = 0;

		SupportsOffhandWeapon = false;

    }

    void Start() override
    {
        // Add the overheat progress bar to the player's UI
        progressBar = make_shared<OverheatProgressBar>();
        EngineMain::MainInstance->Viewport.AddChild(progressBar);

        WeaponFirearm::Start();

	}

    void Destroy() override
    {
        WeaponFirearm::Destroy();
        
        if(progressBar)
            progressBar->RemoveFromParent();

	}

    void Update() override
    {
        WeaponFirearm::Update();

		progressBar->overheat = overheat;
		progressBar->boosted = boosted;

        if (boosted)
        {
            // Drain overheat while in boosted state (regardless of cooldown)
            overheat -= OVERHEAT_DRAIN_RATE * Time::DeltaTimeF;

            if (overheat <= 0.0f)
            {
                // Overheat fully drained — exit boost and trigger reload
                overheat = 0.0f;
                boosted = false;
                params.attackDelayTime = BASE_ATTACK_DELAY;

                attackDelay.AddDelay(1.0f);
                SwitchDelay.AddDelay(1.0f);
                viewmodel->PlayAnimation("reload", false, 0.2f);
                viewmodelLeft->PlayAnimation("reload", false, 0.2f);
            }
        }
        else
        {
            // Passive cooldown when not shooting and not boosted
            if (overheatCooldown.Wait() == false)
            {
                overheat -= Time::DeltaTimeF * OVERHEAT_DRAIN_RATE;
                if (overheat < 0.0f)
                    overheat = 0.0f;
            }
        }
    }

    void FireSingleBullet(const vec3& startLoc, const vec4& gridOffset = vec4(0)) override
    {
        WeaponFirearm::FireSingleBullet(startLoc, gridOffset);

        if (boosted)
            return; // already in boost phase, don't accumulate further

        overheatCooldown.AddDelay(BASE_ATTACK_DELAY);
        overheat += 0.05f;

        if (overheat >= 1.0f)
        {
            overheat = 1.0f;
            boosted = true;
            params.attackDelayTime = BOOSTED_ATTACK_DELAY;
        }
    }

    WeaponSlotData GetDefaultData() override {
        WeaponSlotData data = WeaponFirearm::GetDefaultData();
        data.className = "weapon_mpsd";
        data.slot = 3;
        data.AmmoType = WeaponAmmoType::PistolBullets;
        data.startAmmo = 34;
        return data;
    }
};

REGISTER_ENTITY(weapon_mpsd, "weapon_mpsd")