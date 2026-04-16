#pragma once

#include <Entity.h>

#include <Particle/ParticleSystem.hpp>

#include <Physics.h>

class Bullet : public Entity
{

protected:

	vec3 oldPos;
	float traveledDistance = 0;

public:

	Entity* damageCauser = nullptr;

	std::string OwnerTag = "player";

	std::string debuffOnHit = "";
	float debuffStacks = 0;

	float Damage = 15;
	float Speed = 100;
	float MaxDistance = 100;

	Entity* owner = nullptr;

	ParticleSystem* trail;

	Bullet();
	~Bullet();

	void LoadAssets()
	{

		PreloadEntityType("bullet_trail");
	}

	void Start();

	void Update();

	virtual void TargetHit(Physics::HitResult hit);

};