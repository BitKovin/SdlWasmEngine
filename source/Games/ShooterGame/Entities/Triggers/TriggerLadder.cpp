#include <Entities/Brushes/TriggerBase.hpp>
#include <Entities/Player/Player.hpp>

#include <Helpers/StringHelper.h>

class TriggerLadder : public TriggerBase
{
public:


	bool CanBeTriggered(Entity* entity) override
	{
		return true;
	}

	void OnBodyEntered(Body* body, Entity* entity)override
	{
		
		Player* player = dynamic_cast<Player*>(entity);

		if (player != nullptr)
		{
			player->StartedTouchLadder();
		}

	}

	void OnBodyExited(Body* body, Entity* entity)override
	{
		Player* player = dynamic_cast<Player*>(entity);

		if (player != nullptr)
		{
			player->StoppedTouchLadder();
		}

	}

private:

};

REGISTER_ENTITY(TriggerLadder, "trigger_ladder")