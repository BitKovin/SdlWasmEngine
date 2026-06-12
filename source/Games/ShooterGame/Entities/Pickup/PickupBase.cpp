#include "PickupBase.h"

#include <Entities/Player/Player.hpp>
#include <Network/NetworkManager.h>

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

	NetPacket args;

	SendRPC(RPC_PICKED_UP, args, RPCTarget::All);

}

void PickupBase::OnRPC(uint8_t rpcId, NetPacket& args)
{

	if (rpcId == RPC_PICKED_UP)
	{
		Destroy();

		SoundPlayer::PlayOneshot(pickupSound, 1);

		if (NetworkManager::IsServer())
		{
			CallActionOnEveryEntityWithName(target, pickupEvent);
		}

	}

}

void PickupBase::NetSerialize(NetPacket& packet)
{



}

void PickupBase::NetDeserialize(NetPacket & packet)
{
}
