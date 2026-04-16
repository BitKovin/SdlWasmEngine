#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

#include "../../Ai/NpcTasks/TaskState.h"

class NpcTaskDoJob : public CustomTask
{

private:

	NpcBase* npcRef = nullptr;

	int numTicks = 2;

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
	NpcTaskDoJob() : CustomTask("Npc Task Do Job", "NpcTaskDoJob")
	{

	}



	void OnStop(BehaviorTreeContext& context) override
	{

		CustomTask::OnStop(context);

		CheckNpcRef(context);

		npcRef->pathFollow.reachedTarget = true;
		npcRef->StopMovement();

	}

	void OnStart(BehaviorTreeContext& context) override
	{

		CustomTask::OnStart(context);

		CheckNpcRef(context);

		if (npcRef)
		{
			vec3 target = GetVariable<vec3>("target");
			npcRef->MoveTo(target, GetVariable<float>("acceptance radius"));
			npcRef->PrepareToStartMovement();
			npcRef->pathFollow.reachedTarget = false;

			numTicks = 2;

		}

	}

	void Tick(BehaviorTreeContext& context)
	{

		CheckNpcRef(context);

		vec3 target = GetVariable<vec3>("target");

		if (numTicks)
		{
			npcRef->MoveTo(target, GetVariable<float>("acceptance radius"));

			npcRef->pathFollow.reachedTarget = false;
			npcRef->PrepareToStartMovement();

			numTicks--;
			return;
		}

		if (npcRef->pathFollow.reachedTarget && npcRef->pathFollow.CalculatedPath)
		{
			FinishExecution(true);
			return;
		}


		npcRef->MoveTo(target, GetVariable<float>("acceptance radius"));

	}

};

REGISTER_BT_NODE(NpcTaskDoJob);