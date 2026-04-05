#include "PickupBase.h"

#include <Entities/Player/Player.hpp>

void PickupBase::Start()
{

	triggerBody = Physics::CreateBoxBody(this, Position, vec3(0.5f), 0, true, BodyType::World, BodyType::CharacterCapsule);
	triggerBody->SetIsSensor(true);
}

void PickupBase::OnBodyEntered(Body* body, Entity* entity)
{

	Entity::OnBodyEntered(body, entity);

	Player* player = dynamic_cast<Player*>(entity);

	if (player)
	{
		OnPickup(player);
		Destroy();
	}

}
