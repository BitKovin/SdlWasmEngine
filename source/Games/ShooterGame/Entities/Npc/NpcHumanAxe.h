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


class NpcHumanAxe : public Entity
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

public:

	SkeletalMesh* mesh;



	NpcHumanAxe()
	{
		mesh = new SkeletalMesh(this);
		Drawables.push_back(mesh);

		ClassName = "npc_human_axe";
		SaveGame = true;

		Health = 120;

		mesh->UpdatePoseOnlyWhenRendered = true;

		Tags.push_back("enemy");

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

	void Stun(Entity* DamageCauser, Entity* Weapon);

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

	void LoadAssets();


};