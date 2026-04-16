#pragma once

#include <Entity.h>

#include <Particle/ParticleSystem.hpp>

class CaneProjectile : public Entity
{

private:

	vec3 oldPos;
	float traveledDistance = 0;

public:

	Entity* owner = nullptr;

	float Damage = 15;
	float Speed = 100;
	float MaxDistance = 100;

	bool hited = false;

	ParticleSystem* trail = nullptr;

	StaticMesh* mesh = nullptr;

	string hitEntityId = "";

	vec3 relativeOffset = vec3(0,0,0);


	const Body* bodyToPush = nullptr;
	vec3 impulseToApply = vec3();

	bool movingTo = false;

	bool inEnemy = false;

	CaneProjectile()
	{

		SaveGame = true;
		ClassName = "caneProjectile";

		mesh = new StaticMesh(this);
		Drawables.push_back(mesh);

		Name = "caneProjectile";

	}
	~CaneProjectile(){}

	void LoadAssets()
	{

		PreloadEntityType("cane_trail");
		mesh->LoadFromFile("GameData/models/player/weapons/cane/cane.obj");
		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/models/player/weapons/cane/cane.png");

	}

	void Start()
	{
		oldPos = Position;

		trail = (ParticleSystem*)Spawn("cane_trail");
		trail->Position = Position;
		trail->Rotation = Rotation;
		trail->Start();

	}

	void Destroy()
	{

		Entity::Destroy();

		if (trail != nullptr)
		{
			trail->StopAll();
			trail = nullptr;
		}

	}

	void DamageEntity();

	void Serialize(json& target)
	{

		Entity::Serialize(target);

		SERIALIZE_FIELD(target, oldPos)
		SERIALIZE_FIELD(target, traveledDistance)
		SERIALIZE_FIELD(target, hited)
		SERIALIZE_FIELD(target, hitEntityId)
		SERIALIZE_FIELD(target, relativeOffset)

	}

	void Deserialize(json& source)
	{

		Entity::Deserialize(source);

		DESERIALIZE_FIELD(source, oldPos)
		DESERIALIZE_FIELD(source, traveledDistance)
		DESERIALIZE_FIELD(source, hited)
		DESERIALIZE_FIELD(source, hitEntityId)
		DESERIALIZE_FIELD(source, relativeOffset)

	}

	void Update();



};