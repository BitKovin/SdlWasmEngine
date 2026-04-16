#include <Entity.h>
#include "Player/Player.hpp"

class RelativeTeleporter : public Entity
{
public:

	vec3 start;
	vec3 end;

	string startName = "";
	string endName = "";

	RelativeTeleporter();
	~RelativeTeleporter();

	void Teleport();

	void FromData(EntityData data)
	{

		Entity::FromData(data);

		startName = data.GetPropertyString("start");
		endName = data.GetPropertyString("end");


	}

	void Start()
	{

		Entity* startEnt = Level::Current->FindEntityWithName(startName);
		Entity* endEnt = Level::Current->FindEntityWithName(endName);

		if (startEnt == nullptr)
		{

			Logger::Log(Id + "did not found start entity");

			return;
		}
		if (endEnt == nullptr)
		{

			Logger::Log(Id + "did not found end entity");

			return;

		}

		start = startEnt->Position;
		end = endEnt->Position;

	}

	void OnAction(std::string action)
	{

		Entity::OnAction(action);

		if(action == "teleport")
			Teleport();

	}

private:

};

RelativeTeleporter::RelativeTeleporter()
{
}

RelativeTeleporter::~RelativeTeleporter()
{
}

void RelativeTeleporter::Teleport()
{

	vec3 offset = end - start;

	Player* player = Player::Instance;

	if (player == nullptr) return;

	player->Teleport(player->Position + offset);

}


REGISTER_ENTITY(RelativeTeleporter, "relativeTeleporter")