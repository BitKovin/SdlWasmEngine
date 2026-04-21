#pragma once

#include "TriggerBase.hpp"

class TriggerOnce : public TriggerBase
{
public:

	TriggerOnce() : TriggerBase()
	{
		ClassName = "triggerOnce";
	}
	
	void OnBodyEntered(Body* body, Entity* entity)
	{
		if (CanBeTriggered(entity) == false)return;
		TriggerBase::OnBodyEntered(body, entity);

		if (entity->HasTag("player"))
		{
			Destroy();
		}

	}

private:

};