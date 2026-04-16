#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"
#include "../../Entities/Npc/Ai/TaskPoint.h"

class SetLookTargetFromCurrentNavPoint : public CustomTask
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
	SetLookTargetFromCurrentNavPoint() : CustomTask("Set Look Target From Current NavPoint", "SetLookTargetFromCurrentNavPoint")
	{

	}

	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckNpcRef(context);

		if (npcRef && npcRef->GetTaskStateRef().HasToLookAtTarget)
		{

			npcRef->StopMovement();
			npcRef->desiredLookVector = MathHelper::GetForwardVector(npcRef->GetTaskStateRef().TargetOrientation);

		}

		FinishExecution(true);

	}



};

REGISTER_BT_NODE(SetLookTargetFromCurrentNavPoint);