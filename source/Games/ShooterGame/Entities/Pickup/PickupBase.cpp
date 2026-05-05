#include "PickupBase.h"

#include <Entities/Player/Player.hpp>

void PickupBase::Start()
{

	triggerBody = Physics::CreateBoxBody(this, Position + vec3(0,0.0f,0), vec3(0.5f), 0, true, BodyType::MainBody, BodyType::CharacterCapsule);
	triggerBody->SetIsSensor(true);
	LeadBody = triggerBody;
}

void PickupBase::OnBodyEntered(Body* body, Entity* entity)
{

	Entity::OnBodyEntered(body, entity);

	Player* player = dynamic_cast<Player*>(entity);

	if (player)
	{
		OnPickup(player);
	}

}

void PickupBase::FromData(EntityData data)
{

	Entity::FromData(data);

	target = data.GetPropertyString("target");
	pickupEvent = data.GetPropertyString("onPickupEvent");


}

void PickupBase::OnPickup(Player* player)
{

	CallActionOnEveryEntityWithName(target, pickupEvent);

}
