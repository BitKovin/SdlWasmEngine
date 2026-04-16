#pragma once
#include "TaskPoint.h"

class TaskPhasedAction : public TaskPoint
{
protected:
    std::string enterAnim, loopAnim, exitAnim;
    float enterDur = 0.0f, exitDur = 0.0f;
    float blendDur = 1.0f;
    float loopDur = 0.0f;
    float acceptanceRadius = 0.4f;

    void UpdatePositionBlend(NpcBase* npc);

    void StartLoop(NpcBase* npc);
    void StartExit(NpcBase* npc);

public:
    virtual void FromData(EntityData data) override;

    void OnNpcTargetReached(NpcBase* npc) override;
    void NpcEntered(NpcBase* npc) override;
    void NpcUpdate(NpcBase* npc) override;
    void NpcTryInterrupt(NpcBase* npc) override;
    void NpcInterrupted(NpcBase* npc) override;
    void NpcReturned(NpcBase* npc) override;
};