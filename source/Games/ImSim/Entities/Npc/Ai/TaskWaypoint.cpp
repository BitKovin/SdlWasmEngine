// TaskWaypoint.cpp
#include "TaskWaypoint.h"
#include "../NpcBase.h"

void TaskWaypoint::FromData(EntityData data)
{
    TaskPoint::FromData(data); // Loads "target" as nextTask
    waitTime = data.GetPropertyFloat("waitTime", waitTime);
    acceptanceRadius = data.GetPropertyFloat("acceptanceRadius", acceptanceRadius);
    if (waitTime < 0) waitTime = 0;
}

void TaskWaypoint::OnNpcTargetReached(NpcBase* npc)
{
    TaskPoint::OnNpcTargetReached(npc);
    auto& s = npc->GetTaskStateRef();
    if (waitTime < 0.01f) {
        FinishTask(npc);
    }
    else {
        s.TaskStage = "waiting";
        s.Timer1 = 0.0f;
        s.DoingJob = true;
        s.HasToLookAtTarget = true;
        s.TargetOrientation = Rotation;
    }
}

void TaskWaypoint::NpcEntered(NpcBase* npc)
{
    TaskPoint::NpcEntered(npc);
    if (currentNpc != npc) return;
    MoveNpcTo(npc, Position, acceptanceRadius);
    npc->GetTaskStateRef().TaskStage = "moving";
}

void TaskWaypoint::NpcUpdate(NpcBase* npc)
{
    TaskPoint::NpcUpdate(npc);
    auto& s = npc->GetTaskStateRef();
    if (s.TaskStage == "waiting")
    {
        s.Timer1 += Time::DeltaTimeF;
        if (s.Timer1 >= waitTime)
            FinishTask(npc);
    }
}

void TaskWaypoint::NpcInterrupted(NpcBase* npc) { TaskPoint::NpcInterrupted(npc); }
void TaskWaypoint::NpcReturned(NpcBase* npc)
{
    TaskPoint::NpcReturned(npc);
    MoveNpcTo(npc, Position, acceptanceRadius);
    npc->GetTaskStateRef().TaskStage = "moving";
}