#include <BehaviourTree/CustomTask.h>
#include <BehaviourTree/NodeFactory.h>

#include "../../Entities/Npc/NpcBase.h"

class MoveToEntity : public CustomTask
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

	int numTicks = 2;

public:
	MoveToEntity() : CustomTask ("Move To Entity", "MoveToEntity")
	{
		Variables["target"] = "";
		Variables["acceptance radius"] = 1.0f;
		Variables["pretty path"] = false;
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

			std::string target = GetVariable<std::string>("target");

			Entity* targetEntity = nullptr;

			if (target.starts_with("$"))
			{
				targetEntity = Level::Current->FindEntityWithId(target);
			}
			else
			{
				targetEntity = Level::Current->FindEntityWithName(target);
			}

			if (targetEntity == nullptr)
			{
				FinishExecution(false);
				return;
			}

		}

		numTicks = 2;

	}

	void Tick(BehaviorTreeContext& context)
	{

		CheckNpcRef(context);


		std::string target = GetVariable<std::string>("target");

		Entity* targetEntity = nullptr;

		if (target.starts_with("$"))
		{
			targetEntity = Level::Current->FindEntityWithId(target);
		}
		else
		{
			targetEntity = Level::Current->FindEntityWithName(target);
		}

		if (targetEntity == nullptr)
		{
			FinishExecution(false);
			return;
		}

		if (numTicks)
		{
			npcRef->MoveTo(targetEntity->Position, GetVariable<float>("acceptance radius"));

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


		npcRef->MoveTo(targetEntity->Position, GetVariable<float>("acceptance radius"));

	}

};

REGISTER_BT_NODE(MoveToEntity);