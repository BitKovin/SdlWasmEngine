#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

class LockTask : public CustomTask
{

private:


public:
	LockTask() : CustomTask("Lock", "LockTask")
	{
		Variables["lock"] = false;
		Variables["inverse"] = false;
	}

	void CheckCondition(BehaviorTreeContext& context)
	{
		bool condition = GetVariable<bool>("lock", false);
		if (GetVariable<bool>("inverse", false))
			condition = !condition;

		NodeStatus childStatus = NodeStatus::Success;

		if (children_.empty() == false)
		{
			childStatus = children_[0]->Execute(context);
		}

		if (condition == false)
		{

			if (childStatus == NodeStatus::Success)
			{
				FinishExecution(true);
			}
			else if (childStatus == NodeStatus::Failure)
			{
				FinishExecution(false);
			}



		}
			
	}

	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckCondition(context);

	}

	void Tick(BehaviorTreeContext& context)
	{

		CheckCondition(context);

	}

};

REGISTER_BT_NODE(LockTask);