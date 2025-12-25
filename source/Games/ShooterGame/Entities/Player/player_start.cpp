#include <Entity.h>
#include <LevelTraversalSystem.h>
#include "Player.hpp"

#include <glm.h>

class player_start : public Entity
{
public:
	player_start(){}

	EntityData storedData;

	float angle = 0;

	void FromData(EntityData data) override
	{
		Entity::FromData(data);

		std::string spawnPointName = LevelTraversalSystem::DesiredSpawnPointName;

		angle = data.GetPropertyFloat("angle") + 90;

		if (spawnPointName == Name)
		{
			Entity* player = LevelObjectFactory::instance().create("player");

			player->Name = "player";

			Level::Current->AddEntity(player);
			player->Position = Position;
			player->Rotation.y = angle;
			player->LoadAssetsIfNeeded();
			player->Start();

			if (LevelTraversalSystem::Traveling)
			{
				player->Deserialize(LevelTraversalSystem::PlayerSerializedData);
			}



			
			Player* p = dynamic_cast<Player*>(player);
			p->Rotation.y = angle;
			p->cameraRotation.y = angle;
			p->cameraRotation.x = 0;
			p->cameraRotation.z = 0;


			p->Teleport(Position);

		}

	}

	void Start() override
	{
		Entity::Start();

		
	}

private:

};

REGISTER_ENTITY(player_start, "info_player_start")