// TaskPoint.h
#pragma once

#include <Entity.h>
#include "../../../Ai/NpcTasks/TaskState.h"

class NpcBase;

class TaskPoint : public Entity
{
public:
    void FromData(EntityData data) override;

    virtual TaskState GetDefaultTaskState();

    virtual void NpcEntered(NpcBase* npc);
    virtual void NpcExited(NpcBase* npc);
    virtual void NpcUpdate(NpcBase* npc);

    virtual void NpcInterrupted(NpcBase* npc);
    virtual void NpcReturned(NpcBase* npc);

    TaskPoint* FindTaskByName(const std::string name);

    virtual void OnNpcTargetReached(NpcBase* npc);

    void MoveNpcTo(NpcBase* npc, vec3 target, float acceptanceRadius);
    void PlayTaskAnimation(NpcBase* npc, std::string animationName, bool loop);
    void StopTaskAnimation(NpcBase* npc);

    virtual void NpcTryInterrupt(NpcBase* npc);

    NpcBase* GetCurrentNpc() const { return currentNpc; }

    virtual void FinishTask(NpcBase* npc);

protected:

	bool AllowMultipleNpcs = true;

    NpcBase* currentNpc = nullptr;
    std::string nextTask;

private:
};