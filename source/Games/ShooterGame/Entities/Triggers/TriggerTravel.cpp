#include <Entities/Brushes/TriggerBase.hpp>
#include <LevelTraversalSystem.h>

class TriggerTravel : public TriggerBase
{
public:
	TriggerTravel(): TriggerBase() {}
	
	std::string targetLevel = "";

	std::string targetSpawnPoint = "";

	void FromData(EntityData data) override
	{

		TriggerBase::FromData(data);

		targetLevel = data.GetPropertyString("targetLevel");
		targetSpawnPoint = data.GetPropertyString("targetPlayerStart");

	}

	void OnBodyEntered(Body* body, Entity* entity)override
	{

		if (entity->HasTag("player"))
		{
			if (Time::GameTime - entity->SpawnTime > 2)
			{
				LevelTraversalSystem::TravelToLevel(targetLevel, entity, targetSpawnPoint);
			}
		}

	}

private:

};

REGISTER_ENTITY(TriggerTravel, "triggerTravel")