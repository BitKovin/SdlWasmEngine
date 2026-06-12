#pragma once

#include "../../Entity.h"
#include "../../Physics.h"
#include <Network/NetworkedEntity.h>

class TriggerBase : public NetworkedEntity
{
public:

	enum class RPCIndex : uint8_t
	{

		None = 0,
		Enter = 1,
		Exit = 2

	};

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

	virtual void DoEnterAction();
	virtual void DoExitAction();

	void OnRPC(uint8_t rpcId, NetPacket& args);

	void OnBodyEntered(Body* body, Entity* entity);
	void OnBodyExited(Body* body, Entity* entity);

private:

};