#pragma once

#include <Physics.h>
#include <Entity.h>

class PickupBase : public Entity
{
public:
	
	void Start();

	Body* triggerBody = nullptr;

	void OnBodyEntered(Body* body, Entity* entity) override;

	virtual void OnPickup(class Player* player) {}

private:

};

