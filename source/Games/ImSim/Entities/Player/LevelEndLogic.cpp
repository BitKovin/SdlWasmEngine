#include <Entity.h>

class LevelEndLogic : public Entity
{
public:
	
	std::string nextLevel = "";

	void FromData(EntityData data) override
	{
		Entity::FromData(data);
		
		nextLevel = data.GetPropertyString("nextLevel", "");

	}

	void OnAction(string action) override
	{
		Spawn("level_end_camera");
	}

private:

};

REGISTER_ENTITY(LevelEndLogic, "logic_level_end")