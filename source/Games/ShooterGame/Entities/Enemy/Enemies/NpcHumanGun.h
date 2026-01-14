#pragma once

#include <Entity.h>
#include <Input.h>
#include <MathHelper.hpp>
#include <Camera.h>
#include <SkeletalMesh.hpp>
#include <AssetRegistry.h>

#include <Navigation/PathFollowQuery.h>

#include <SoundSystem/SoundManager.hpp>
#include <Entities/SoundPlayer.h>
#include <Delay.hpp>

#include "../IEnemy.h"

#include <UI/WorldSpace/UiBilboard.h>
#include <UI/UiVideo.hpp>
#include "../../../UI/Enemy/UiNpcDebuffs.h"



class NpcHumanGun : public Entity, public IEnemy
{

private:

	vec3 desiredDirection = vec3();
	vec3 movingDirection = vec3();

	PathFollowQuery pathFollow;

	SoundPlayer* soundPlayer = nullptr;

	float maxSpeed = 5.3f;
	float speed = 4;

	bool dead = false;

	bool fleeing = false;
	Delay fleeSearchDelay = Delay();

	AnimationState animationStateSaveData;

	Delay inAttackDelay;
	Delay cantAttackDelay;

	Delay stopMovingDelay;

	bool stuned = false;

	float accuracyModifier = 0;

	void UpdateFleeTarget();

	UiBilboard* statusWidget = nullptr;

	vec3 desiredTargetLocation = vec3();

public:

	SkeletalMesh* mesh = nullptr;



	NpcHumanGun()
	{
		mesh = new SkeletalMesh(this);
		Drawables.push_back(mesh);

		statusWidget = new UiBilboard(this);
		Drawables.push_back(statusWidget);

		auto debuffs = make_shared<UiNpcStatus>(this);
		statusWidget->ViewportSize = ivec2(1024, 256);
		statusWidget->PixelPerMeter = 1024.0f;
		//debuffs->Target = this;

		statusWidget->ContentBox->AddChild(debuffs);

		ClassName = "npc_human_axe";
		SaveGame = true;

		Health = 130;
		MaxHealth = 130;

		mesh->UpdatePoseOnlyWhenRendered = true;

		Tags.push_back("enemy");

		Poise = 100.f;

	}


	~NpcHumanGun()
	{

	}

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 100;
		soundPlayer->Volume = 1.0;
		soundPlayer->MinDistance = 0.5;
	}

	void PlaySoundEffect(std::string eventName);

	void ProcessAnimationEvent(AnimationEvent& event);

	void Start();

	void Stun() override;

	void Attack();

	void Death();

	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	bool LineOfSightCheck(Entity* targetEntity);

	bool AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity);

	void Destroy()
	{
		Entity::Destroy();

		mesh->ClearHitboxes();


	}

	void AsyncUpdate();

	void Serialize(json& target);
	void Deserialize(json& source);

	void UpdateStatusWidgets();

	void UpdateDebugUI();

protected:

	void LoadAssets();


};