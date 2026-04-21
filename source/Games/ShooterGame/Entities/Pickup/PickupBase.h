#pragma once

#include <Physics.h>
#include <Entity.h>

class PickupBase : public Entity
{
public:
	
	std::string target;
	std::string pickupEvent;

	void Start();

	Body* triggerBody = nullptr;

	void OnBodyEntered(Body* body, Entity* entity) override;

	void FromData(EntityData data) override;

	virtual void OnPickup(class Player* player);

private:

};

