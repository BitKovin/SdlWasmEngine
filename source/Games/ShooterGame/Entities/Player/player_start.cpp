#include <Entity.h>
#include <LevelTraversalSystem.h>
#include "Player.hpp"
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

			player->Deserialize(LevelTraversalSystem::PlayerSerializedData);


			
			Player* p = dynamic_cast<Player*>(player);
			p->cameraRotation.y = angle;
			p->Teleport(Position);

		}

	}

	void Start() override
	{
		Entity::Start();

		LevelTraversalSystem::DesiredSpawnPointName = "";

	}

private:

};

REGISTER_ENTITY(player_start, "info_player_start")