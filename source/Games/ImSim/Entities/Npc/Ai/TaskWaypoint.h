#pragma once
#include "TaskPoint.h"

class TaskWaypoint : public TaskPoint
{
public:
    float waitTime = 0.0f;
    float acceptanceRadius = 0.3f;

    virtual void FromData(EntityData data) override;

    void OnNpcTargetReached(NpcBase* npc) override;
    void NpcEntered(NpcBase* npc) override;
    void NpcUpdate(NpcBase* npc) override;
    void NpcInterrupted(NpcBase* npc) override;
    void NpcReturned(NpcBase* npc) override;
};