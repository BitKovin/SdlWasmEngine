#pragma once

#include <Physics.h>
#include <Entity.h>
#include <NetworkedEntity.h>

class PickupBase : public NetworkedEntity
{
public:
	
	static inline uint8_t RPC_PICKED_UP = 0;

	std::string target;
	std::string pickupEvent;

	std::string pickupSound = "event:/Character/Player/ItemPickup";

	void Start();

	Body* triggerBody = nullptr;

	void OnBodyEntered(Body* body, Entity* entity) override;

	void FromData(EntityData data) override;

	virtual void OnPickup(class Player* player);

	void OnRPC(uint8_t rpcId, NetPacket& args);

	void NetSerialize(NetPacket& packet) override;
	void NetDeserialize(NetPacket& packet) override;

private:

};

