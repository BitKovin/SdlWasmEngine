#pragma once

#include <vector>

#include "EObject.hpp"
#include "Helpers/JsonHelper.hpp"
#include "Helpers/JsonMacros.hpp"

#include "LevelObject.hpp"

#include "IDrawMesh.h"

#include "glm.h"

#include "Physics.h"

#include "RegisterLevelObject.h"

#include "MapData.h"

#include "Level.hpp"

using namespace std;

class Entity : public LevelObject
{
public:

	vec3 Position = vec3();

	vec3 Rotation = vec3();

	vec3 Scale = vec3(1);

	vector<IDrawMesh*> Drawables;

	Body* LeadBody = nullptr;

	vector<Body*> Bodies;

	bool Unique = false;

	bool SaveGame = false;

	string ClassName = "Entity";
	string Id = "";

	string Name = "";

	float Health = 1;
	float MaxHealth = 100;

	bool AssetsLoaded = false;

	bool ConvexCollision = false;

	string OwnerId = "";

	vector<string> Tags;

	Delay destroyDelay = Delay(1000000000);

	BodyType DefaultBrushGroup = BodyType::WorldOpaque;
	BodyType DefaultBrushCollisionMask = BodyType::GroupCollisionTest;

	double SpawnTime = 0;

	vector<Body*> contactList;

	Entity()
	{
		
	}
	virtual ~Entity() 
	{
		DestroyDrawables();
	}


	void UpdatePhysics()
	{
		if (LeadBody)
		{
			Position = FromPhysics(LeadBody->GetPosition());
			Rotation = MathHelper::ToYawPitchRoll(FromPhysics(LeadBody->GetRotation()));
		}
	}

	void Start()
	{
		SpawnTime = Time::GameTime;
	}

	virtual void FromData(EntityData data)
	{

		Name = data.GetPropertyString("targetname");

		Position = data.GetPropertyVectorPosition("origin");

		Unique = data.GetPropertyBool("unique", Unique);

	}

	virtual void Finalize()
	{

	}

	virtual LightVolPointData GetLightVolData(bool wallCheck);

	virtual void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		Health -= Damage;
	}

	virtual void Teleport(vec3 position);

	virtual void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		OnDamage(Damage, DamageCauser, Weapon);
	}

	vector<IDrawMesh*> GetDrawMeshes()
	{
		return Drawables;
	}

	void DestroyDrawables()
	{
		for (IDrawMesh* mesh : Drawables)
		{
			delete(mesh);
		}

		Drawables.clear();
	}

	void FinalLevelRemove();

	void UpdateDestroyDelay()
	{
		if (destroyDelay.Wait() == false)
		{
			Destroy();
		}
	}

	virtual void DestroyWithDelay(float delay)
	{
		destroyDelay.AddDelay(delay);
	}

	virtual void Destroy();

	void Serialize(json& target);

	void Deserialize(json& source);

	void LoadAssetsIfNeeded()
	{
		if (AssetsLoaded == false)
		{
			LoadAssets();
			AssetsLoaded = true;

			if (Level::Current->IsEntityTypeLoaded(ClassName) == false)
			{
				Level::Current->AddLoadedEntityType(ClassName);
			}

		}
	}

	bool HasTag(string tag)
	{

		for (auto t : Tags)
		{
			if (tag == t)
			{
				return true;
			}
		}

		return false;
	}

	virtual void OnBodyEntered(Body* body, Entity* entity) {}
	virtual void OnBodyExited(Body* body, Entity* entity) {}

	virtual void OnAction(string action) {}

	static Entity* Spawn(std::string technicalName);

	static void CallActionOnEveryEntityWithName(std::string name, std::string action)
	{

		if (name == "") return;

		auto entities = Level::Current->FindAllEntitiesWithName(name);

		for (Entity* entity : entities)
		{
			entity->OnAction(action);
		}

	}

	static void CallActionOnEntityWithId(std::string id, std::string action)
	{

		if (id == "") return;

		auto entity = Level::Current->FindEntityWithId(id);

		if (entity != nullptr)
		{
			entity->OnAction(action);
		}



	}

	static void PreloadEntityType(std::string technicalName);

protected:

	void OnDispose()
	{
		Destroy();
	}

	virtual void LoadAssets()
	{

	}

private:

};