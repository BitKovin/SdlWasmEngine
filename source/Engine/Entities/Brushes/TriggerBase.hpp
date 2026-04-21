#pragma once

#include "../../Entity.h"
#include "../../Physics.h"

class TriggerBase : public Entity
{
public:

	string targetName = "";
	string onEnterAction = "trigger_enter";
	string onExitAction = "trigger_exit";

	TriggerBase()
	{
		ClassName = "trigger";
		SaveGame = true;
		ConvexCollision = true;
	}

	void Start();

	virtual bool CanBeTriggered(Entity* entity);

	void FromData(EntityData data);

	void OnBodyEntered(Body* body, Entity* entity);
	void OnBodyExited(Body* body, Entity* entity);

private:

};