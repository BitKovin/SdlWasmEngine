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

#include "../Enemy/IEnemy.h"

#include <UI/WorldSpace/UiBilboard.h>
#include <UI/UiVideo.hpp>

class NpcHumanAxe : public Entity, public IEnemy
{

private:

	vec3 desiredDirection;
	vec3 movingDirection;

	PathFollowQuery pathFollow;

	SoundPlayer* soundPlayer = nullptr;

	float maxSpeed = 6;
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



	NpcHumanAxe()
	{
		mesh = new SkeletalMesh(this);
		Drawables.push_back(mesh);

		statusWidget = new UiBilboard(this);
		Drawables.push_back(statusWidget);

		auto image = make_shared<UiVideo>();

		image->VideoPath = "GameData/videos/meowl.mpg";
		image->size = vec2(256, 256);

		statusWidget->ContentBox->AddChild(image);

		ClassName = "npc_human_axe";
		SaveGame = true;

		Health = 120;

		mesh->UpdatePoseOnlyWhenRendered = true;

		Tags.push_back("enemy");

		Poise = 100.f;

	}


	~NpcHumanAxe()
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

	void UpdateAttackDamage();

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