#pragma once
#include "../Entity.h"
#include <Logger.hpp>

class Spawner : public Entity
{
public:

	string spawnClassName = "testnpc";
	string onSpawnedAction = "spawned";
	string onDespawnedAction = "despawned";
	string actionTarget = "";

	bool autoTriggerOnPlayer = false;

	Spawner()
	{

	}

	void FromData(EntityData data)
	{
		Entity::FromData(data);

		spawnClassName = data.GetPropertyString("className", spawnClassName);
		onSpawnedAction = data.GetPropertyString("onSpawned", onSpawnedAction);
		onDespawnedAction = data.GetPropertyString("onDespawned", onDespawnedAction);
		actionTarget = data.GetPropertyString("target", actionTarget);
		autoTriggerOnPlayer = data.GetPropertyBool("autoTrigger");
	}

	void LoadAssets()
	{
		PreloadEntityType(spawnClassName);
	}

	void PerformSpawn()
	{
		Entity* entity = LevelObjectFactory::instance().create(spawnClassName);
		if (entity == nullptr)
		{
			Logger::Log("failed to spawn entity of type :" + spawnClassName);
			return;
		}
		entity->Position = Position;
		entity->Rotation = Rotation;
		entity->OwnerId = Id;



		DebugDraw::Line(Position, MathHelper::GetForwardVector(Rotation) + Position, 10, 0);
		DebugDraw::Line(Position, vec3(0,1.0f,0) + Position, 10, 0);

		Level::Current->AddEntity(entity);

		entity->Start();

		if (autoTriggerOnPlayer)
		{
			entity->OnAction("triggerOnPlayer");
		}
		

		CallActionOnEveryEntityWithName(actionTarget, onSpawnedAction);

	}

	void OnAction(string action)
	{

		if (action == "despawned")
		{
			CallActionOnEveryEntityWithName(actionTarget, onDespawnedAction);
			return;
		}

		PerformSpawn();

	}


private:

};