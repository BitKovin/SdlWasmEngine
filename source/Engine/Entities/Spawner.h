#pragma once
#include "../Entity.h"

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
	}

	void LoadAssets()
	{
		PreloadEntityType(spawnClassName);
	}

	void PerformSpawn()
	{
		Entity* entity = Spawn(spawnClassName);
		if (entity == nullptr)
		{
			Logger::Log("failed to spawn entity of type :" + spawnClassName);
			return;
		}
		entity->Position = Position;
		entity->Rotation = Rotation;
		entity->OwnerId = Id;
		entity->Start();

		if (autoTriggerOnPlayer)
		{
			entity->OnAction("triggerOnPlayer");
		}
		
		Logger::Log("calling action");

		CallActionOnEveryEntityWithName(actionTarget, onSpawnedAction);

	}

	void OnAction(string action)
	{

		Logger::Log("spawner received action " + action);

		if (action == "despawned")
		{
			CallActionOnEveryEntityWithName(actionTarget, onDespawnedAction);
			return;
		}

		PerformSpawn();

	}


private:

};