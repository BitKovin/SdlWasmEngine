#include "../Entity.hpp"

class WorldSpawn : public Entity
{
public:

	WorldSpawn()
	{
		Static = true;
	}
	
	void FromData(EntityData data)
	{
		Entity::FromData(data);

		printf("world spawn created\n");



	}

private:

};
