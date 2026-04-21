#include "TriggerBase.hpp"

void TriggerBase::Start()
{
	LeadBody->SetIsSensor(true);
	DestroyDrawables();
}

bool TriggerBase::CanBeTriggered(Entity* player)
{
	return true;
}

void TriggerBase::FromData(EntityData data)
{
	Entity::FromData(data);

	targetName = data.GetPropertyString("target", targetName);
	onEnterAction = data.GetPropertyString("onEnterAction", onEnterAction);
	onExitAction = data.GetPropertyString("onExitAction", onExitAction);

}

void TriggerBase::OnBodyEntered(Body* body, Entity* entity)
{
	if (CanBeTriggered(entity) == false)return;
	if (entity->HasTag("player"))
	{
		CallActionOnEveryEntityWithName(targetName, onEnterAction);
	}

}

void TriggerBase::OnBodyExited(Body* body, Entity* entity)
{
	if (CanBeTriggered(entity) == false)return;
	if (entity->HasTag("player"))
	{
		CallActionOnEveryEntityWithName(targetName, onExitAction);
	}

}

REGISTER_ENTITY(TriggerBase, "trigger")
