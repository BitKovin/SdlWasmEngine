#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

class TargetLostInvestigation : public CustomTask
{

private:

	NpcBase* npcRef = nullptr;

	void CheckNpcRef(BehaviorTreeContext& context)
	{
		if (npcRef == nullptr)
		{
			npcRef = dynamic_cast<NpcBase*>((Entity*)(context.owner));

			if (npcRef == nullptr)
			{
				Logger::Log("task node used with wrong Entity type");
			}

		}
	}

public:
	TargetLostInvestigation() : CustomTask("Targe tLost Investigation", "TargetLostInvestigation")
	{

	}


	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckNpcRef(context);

		if (npcRef)
		{

			npcRef->TargetLost();

		}

		FinishExecution();

	}



};

REGISTER_BT_NODE(TargetLostInvestigation);