#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"
#include "../../Entities/Npc/Ai/TaskPoint.h"

class Task_NotDoing : public CustomTask
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
	Task_NotDoing() : CustomTask("Task Not Doing", "Task_NotDoing")
	{

	}

	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckNpcRef(context);

		if (npcRef)
		{

			npcRef->Task_NotDoing();

		}

		FinishExecution(true);

	}



};

REGISTER_BT_NODE(Task_NotDoing);