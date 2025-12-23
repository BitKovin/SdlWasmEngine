#pragma once

#include <Entity.h>

#include <Particle/ParticleSystem.hpp>

class Bullet : public Entity
{

private:

	vec3 oldPos;
	float traveledDistance = 0;

public:


	std::string OwnerTag = "player";

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



};