// TaskPoint.cpp
#include "TaskPoint.h"
#include "../NpcBase.h"

void TaskPoint::FromData(EntityData data)
{
    Entity::FromData(data);
    Rotation.y = data.GetPropertyFloat("angle") + 90;
    nextTask = data.GetPropertyString("target");
}

TaskState TaskPoint::GetDefaultTaskState()
{
    TaskState state;
    state.TaskName = Name;
    return state;
}

void TaskPoint::NpcEntered(NpcBase* npc)
{

    if (AllowMultipleNpcs == false)
    {
        if (currentNpc != nullptr && currentNpc != npc) 
        {

            npc->taskState = GetDefaultTaskState();
            npc->taskState.TaskStage = "_waitingForTurn_";
            npc->taskState.HasToMoveToTarget = false;

            //Logger::Log("Task already occupied: " + Name);
            return;
		}
    }

    npc->taskState = GetDefaultTaskState();
    currentNpc = npc;
}

void TaskPoint::NpcExited(NpcBase* npc)
{
    if (currentNpc == npc) currentNpc = nullptr;
}

void TaskPoint::NpcUpdate(NpcBase* npc) 
{
    TaskState& s = npc->GetTaskStateRef();

    if (s.TaskStage == "_waitingForTurn_")
    {
        NpcEntered(npc);
    }

}
void TaskPoint::NpcInterrupted(NpcBase* npc)
{
    //Logger::Log("npc interrupted task");
    npc->animator->StopTaskAnimation();
    
    TaskState& s = npc->GetTaskStateRef();
    s.HasToLockPosition = false;
    s.DoingJob = false;

}
void TaskPoint::NpcReturned(NpcBase* npc) 
{ 

    npc->PrepareToStartMovement();

    //Logger::Log("npc returned to task"); 
}

TaskPoint* TaskPoint::FindTaskByName(const std::string name)
{
    return dynamic_cast<TaskPoint*>(Level::Current->FindEntityWithName(name));
}

void TaskPoint::OnNpcTargetReached(NpcBase* npc)
{
    npc->GetTaskStateRef().HasToMoveToTarget = false;
}

void TaskPoint::MoveNpcTo(NpcBase* npc, vec3 target, float radius)
{

    auto& s = npc->GetTaskStateRef();

    if (s.HasToMoveToTarget == false)
    {
        npc->PrepareToStartMovement();
    }

    s.TargetLocation = target;
    s.HasToMoveToTarget = true;
    s.AcceptanceRadius = radius;
}

void TaskPoint::PlayTaskAnimation(NpcBase* npc, std::string anim, bool loop)
{
    npc->animator->PlayTaskAnimation(anim, loop);
}

void TaskPoint::StopTaskAnimation(NpcBase* npc)
{
    npc->animator->StopTaskAnimation();
}

void TaskPoint::NpcTryInterrupt(NpcBase* npc)
{
    NpcInterrupted(npc);
}

void TaskPoint::FinishTask(NpcBase* npc)
{
    if (!nextTask.empty()) {
        npc->StartTask(nextTask);
    }
    else {
        auto& s = npc->GetTaskStateRef();
        s.DoingJob = false;
        s.TaskStage = "done";
        NpcExited(npc);
    }
}