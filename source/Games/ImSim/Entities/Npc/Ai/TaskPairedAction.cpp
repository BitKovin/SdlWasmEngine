#include "TaskPairedAction.h"
#include "../NpcBase.h"

void TaskPairedAction::FromData(EntityData data)
{
    TaskPoint::FromData(data); // Loads "target" as nextTask
    partnerName = data.GetPropertyString("partner");
    acceptanceRadius = data.GetPropertyFloat("acceptanceRadius", acceptanceRadius);
    waitTimeout = data.GetPropertyFloat("waitTimeout", waitTimeout);
    partner = dynamic_cast<TaskPairedAction*>(FindTaskByName(partnerName));
    if (!partner && !partnerName.empty())
        Logger::Log("Partner not found: " + partnerName);

	AllowMultipleNpcs = false;

}

void TaskPairedAction::OnNpcTargetReached(NpcBase* npc)
{
    TaskPoint::OnNpcTargetReached(npc);
    auto& s = npc->GetTaskStateRef();
    s.TaskStage = "waiting";
    s.HasToLookAtTarget = true;
    s.TargetOrientation = Rotation;
    s.AllowWeapon = false;
    s.Timer2 = 0.0f;
}

void TaskPairedAction::NpcEntered(NpcBase* npc)
{
    TaskPoint::NpcEntered(npc);
    if (currentNpc != npc) return;
    MoveNpcTo(npc, Position, acceptanceRadius);
    npc->GetTaskStateRef().TaskStage = "moving";
}

void TaskPairedAction::NpcUpdate(NpcBase* npc)
{
    TaskPoint::NpcUpdate(npc);
    auto& s = npc->GetTaskStateRef();

    if (s.TaskStage == "waiting")
    {
        s.Timer2 += Time::DeltaTimeF;
        if (s.Timer2 >= waitTimeout)
        {
            FinishTask(npc);
            return;
        }

        if (partner && partner->GetCurrentNpc())
        {
            auto& ps = partner->GetCurrentNpc()->GetTaskStateRef();
            if (ps.TaskStage == "waiting" || ps.TaskStage == "acting")
            {
                PlayTaskAnimation(npc, anim, true);
                s.TaskStage = "acting";
                s.DoingJob = true;
                s.Timer1 = 0.0f;
            }
        }
    }
    else if (s.TaskStage == "acting")
    {
        s.Timer1 += Time::DeltaTimeF;
        if (s.Timer1 >= duration)
        {
            StopTaskAnimation(npc);
            FinishTask(npc);
        }
    }
}

void TaskPairedAction::NpcInterrupted(NpcBase* npc)
{
    TaskPoint::NpcInterrupted(npc);
    auto& s = npc->GetTaskStateRef();
    s.DoingJob = false;
    s.AllowWeapon = true;

    if (s.TaskStage == "acting" && partner)
    {
        auto other = partner->GetCurrentNpc();
        if (other && other->GetTaskStateRef().TaskStage == "acting")
        {
            StopTaskAnimation(other);
            other->GetTaskStateRef().DoingJob = false;
            other->GetTaskStateRef().AllowWeapon = true;
        }
    }
}

void TaskPairedAction::NpcReturned(NpcBase* npc)
{
    TaskPoint::NpcReturned(npc);
    MoveNpcTo(npc, Position, acceptanceRadius);
    npc->GetTaskStateRef().TaskStage = "moving";
}