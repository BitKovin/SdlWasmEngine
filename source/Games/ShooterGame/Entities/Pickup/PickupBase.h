#pragma once

#include <Physics.h>
#include <Entity.h>
#include <Network/NetworkedEntity.h>

class PickupBase : public NetworkedEntity
{
public:
	
	vec3 areaSize = vec3(0.5f);

	bool NetworkReplicated = false;

	bool pickedUp = false;

	bool eventFired = false;

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

	void Serialize(json& target) override
	{

		SERIALIZE_FIELD(target, pickedUp);
	}

	void Deserialize(json& source) override
	{

		DESERIALIZE_FIELD(source, pickedUp);
		if (pickedUp)
		{
			Visible = false;
		}
	}

private:

};

