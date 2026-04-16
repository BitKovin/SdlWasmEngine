#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

class StopTargetFollow : public CustomTask
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
	StopTargetFollow() : CustomTask("Stop Target Follow", "StopTargetFollow")
	{

	}


	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckNpcRef(context);

		if (npcRef)
		{

			npcRef->StopTargetFollow();

		}

		FinishExecution();

	}

	

};

REGISTER_BT_NODE(StopTargetFollow);