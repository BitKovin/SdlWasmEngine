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
#include "../../../UI/Enemy/UiNpcDebuffs.h"
#include <UI/WorldSpace/UiBilboard.h>
#include <Character/CharacterController.h>

class TestNpc : public Entity, public IEnemy
{

private:

	vec3 desiredDirection;
	vec3 movingDirection;

	PathFollowQuery pathFollow;

	SoundPlayer* soundPlayer = nullptr;

	CharacterController controller;

	float maxSpeed = 6.5f;
	float speed = 4;

	bool dead = false;

	bool fleeing = false;
	Delay fleeSearchDelay = Delay();

	AnimationState animationStateSaveData;

	Delay inAttackDelay;

	bool attacking = false;
	bool stuned = false;

	bool attackingDamage = false;

	void UpdateFleeTarget();

	UiBilboard* statusWidget = nullptr;

public:

	SkeletalMesh* mesh;



	TestNpc()
	{
		mesh = new SkeletalMesh(this);
		Drawables.push_back(mesh);

		ClassName = "testnpc";
		SaveGame = true;

		Health = 80;
		MaxHealth = 80;


		Tags.push_back("enemy");


		statusWidget = new UiBilboard(this);
		Drawables.push_back(statusWidget);

		auto debuffs = make_shared<UiNpcStatus>(this);
		statusWidget->ViewportSize = ivec2(1024, 256);
		statusWidget->PixelPerMeter = 1024.0f;
		//debuffs->Target = this;

		statusWidget->ContentBox->AddChild(debuffs);

	}


	~TestNpc()
	{

	}

	void SetupSoundPlayer(SoundPlayer* soundPlayer)
	{
		soundPlayer->MaxDistance = 100;
		soundPlayer->Volume = 1.0;
		soundPlayer->MinDistance = 0.5;
	}

	void ProcessAnimationEvent(AnimationEvent& event);

	void Start();

	void Stun();

	void Attack();

	void Death();

	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

	void UpdateAttackDamage();

	void Destroy()
	{
		Entity::Destroy();

		mesh->ClearHitboxes();


	}

	void AsyncUpdate();

	void Serialize(json& target);
	void Deserialize(json& source);

protected:

	void UpdateStatusWidgets();

	void LoadAssets();

	void PlaySoundEffect(std::string eventName);


};