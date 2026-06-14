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

void TriggerBase::DoEnterAction()
{

	CallActionOnEveryEntityWithName(targetName, onEnterAction);

}

void TriggerBase::DoExitAction()
{

	CallActionOnEveryEntityWithName(targetName, onExitAction);

}

void TriggerBase::OnRPC(uint8_t rpcId, NetPacket& args)
{

	if (Destroyed) return;

	RPCIndex i = (RPCIndex)rpcId;

	switch (i)
	{
	case TriggerBase::RPCIndex::None:
		break;
	case TriggerBase::RPCIndex::Enter:
		DoEnterAction();
		break;
	case TriggerBase::RPCIndex::Exit:
		DoExitAction();
		break;
	default:
		break;
	}


}

void TriggerBase::OnBodyEntered(Body* body, Entity* entity)
{
	if (CanBeTriggered(entity) == false)return;
	if (entity->HasTag("player"))
	{

		NetPacket args;

		SendRPC((uint8_t)RPCIndex::Enter, args, RPCTarget::Server);
	}

}

void TriggerBase::OnBodyExited(Body* body, Entity* entity)
{
	if (CanBeTriggered(entity) == false)return;
	if (entity->HasTag("player"))
	{
		NetPacket args;
		SendRPC((uint8_t)RPCIndex::Exit, args, RPCTarget::Server);
	}

}

REGISTER_ENTITY(TriggerBase, "trigger")
