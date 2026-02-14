#include "TaskPhasedAction.h"
#include "../NpcBase.h"

void TaskPhasedAction::FromData(EntityData data)
{
    TaskPoint::FromData(data); // Loads "target" as nextTask
    acceptanceRadius = data.GetPropertyFloat("acceptanceRadius", acceptanceRadius);

    AllowMultipleNpcs = false;

}

void TaskPhasedAction::OnNpcTargetReached(NpcBase* npc)
{
    TaskPoint::OnNpcTargetReached(npc);
    auto& s = npc->GetTaskStateRef();
    s.AllowWeapon = false;

    s.StartPosition = npc->Position;

    if (!enterAnim.empty())
    {
        PlayTaskAnimation(npc, enterAnim, false);
        s.TaskStage = "entering";
        s.Timer1 = enterDur;
        s.CanBeCanceled = false;
        s.DoingJob = true;
    }
    else
    {
        StartLoop(npc);
    }
}

void TaskPhasedAction::NpcEntered(NpcBase* npc)
{
    TaskPoint::NpcEntered(npc);
    if (currentNpc != npc) return;
    MoveNpcTo(npc, Position, acceptanceRadius);
    auto& s = npc->GetTaskStateRef();
    s.TaskStage = "moving";
    s.AllowWeapon = true;
    s.HasToLookAtTarget = true;
    s.TargetOrientation = Rotation;
}

void TaskPhasedAction::NpcUpdate(NpcBase* npc)
{
    TaskPoint::NpcUpdate(npc);
    auto& s = npc->GetTaskStateRef();

    UpdatePositionBlend(npc);

    if (s.TaskStage == "entering")
    {
        s.Timer1 -= Time::DeltaTimeF;
        if (s.Timer1 <= 0.0f)
        {
            StopTaskAnimation(npc);
            StartLoop(npc);
        }
    }
    else if (s.TaskStage == "loop")
    {
        if (loopDur == 0.0f) return;
        s.Timer1 += Time::DeltaTimeF;
        if (s.Timer1 >= loopDur) StartExit(npc);
    }
    else if (s.TaskStage == "exiting")
    {
        s.Timer1 -= Time::DeltaTimeF;
        if (s.Timer1 <= 0.0f)
        {
            StopTaskAnimation(npc);
            s.CanBeCanceled = true;
            FinishTask(npc);
        }
    }
}

void TaskPhasedAction::NpcTryInterrupt(NpcBase* npc)
{
    if (npc->GetTaskStateRef().TaskStage == "loop")
        StartExit(npc);
}

void TaskPhasedAction::NpcInterrupted(NpcBase* npc)
{
    TaskPoint::NpcInterrupted(npc);
   
    auto& s = npc->GetTaskStateRef();
    
    s.AllowWeapon = true;
    s.HasToLockPosition = false;
    
    StopTaskAnimation(npc);

    s.TaskStage = "";
    

}

void TaskPhasedAction::NpcReturned(NpcBase* npc)
{
    TaskPoint::NpcReturned(npc);
    MoveNpcTo(npc, Position, acceptanceRadius);
    auto& s = npc->GetTaskStateRef();
    s.TaskStage = "moving";
    s.AllowWeapon = true;
    s.TargetOrientation = Rotation;
    s.HasToLookAtTarget = true;
}

void TaskPhasedAction::UpdatePositionBlend(NpcBase* npc)
{

    float blend = 0;
    
    TaskState& s = npc->GetTaskStateRef();

    if (s.TaskStage == "exiting")
    {
        blend = npc->GetTaskStateRef().Timer1 / blendDur;
    }
    else if(s.TaskStage == "entering")
    {
        blend = (enterDur - npc->GetTaskStateRef().Timer1) / blendDur;
    }
    else if (s.TaskStage == "loop")
    {
        blend = 1;
    }

    blend = glm::clamp(blend, 0.0f, 1.0f);

    s.HasToLockPosition = blend > 0.01f;


    s.LockPosition = glm::mix(s.StartPosition, Position + vec3(0, 1, 0), blend);


}

void TaskPhasedAction::StartLoop(NpcBase* npc)
{
    PlayTaskAnimation(npc, loopAnim, true);
    auto& s = npc->GetTaskStateRef();
    s.TaskStage = "loop";
    s.Timer1 = 0.0f;
    s.CanBeCanceled = false;
    s.DoingJob = true;
}

void TaskPhasedAction::StartExit(NpcBase* npc)
{
    StopTaskAnimation(npc);
    PlayTaskAnimation(npc, exitAnim, false);
    auto& s = npc->GetTaskStateRef();
    s.TaskStage = "exiting";
    s.Timer1 = exitDur;
    s.CanBeCanceled = false;
    s.DoingJob = true;
}