#pragma once

#include <Network/NetworkedEntity.h>

#include <Particle/ParticleSystem.hpp>

#include <Physics.h>

class Bullet : public NetworkedEntity
{

protected:

	vec3 oldPos;
	float traveledDistance = 0;

	// Result of the trace fired in AsyncUpdate, consumed in LateUpdate.
	Physics::HitResult asyncHit;

public:

	Entity* damageCauser = nullptr;

	std::string trailTypeName = "bullet_trail";

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

		PreloadEntityType(trailTypeName);
	}

	void Start();

	void Update();
	void AsyncUpdate();
	void LateUpdate();

	virtual void TargetHit(Physics::HitResult hit);

	void NetSerialize(NetPacket& packet);
	void NetDeserialize(NetPacket& packet);

};
