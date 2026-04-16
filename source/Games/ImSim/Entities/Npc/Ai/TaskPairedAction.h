#pragma once
#include "TaskPoint.h"

class TaskPairedAction : public TaskPoint
{
protected:
    std::string partnerName;
    TaskPairedAction* partner = nullptr;
    std::string anim;
    float duration = 0.0f;
    float waitTimeout = 10.0f;
    float acceptanceRadius = 0.3f;

public:
    virtual void FromData(EntityData data) override;

    void OnNpcTargetReached(NpcBase* npc) override;
    void NpcEntered(NpcBase* npc) override;
    void NpcUpdate(NpcBase* npc) override;
    void NpcInterrupted(NpcBase* npc) override;
    void NpcReturned(NpcBase* npc) override;
};