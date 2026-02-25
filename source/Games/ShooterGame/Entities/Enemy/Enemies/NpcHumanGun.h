#pragma once

#include "NpcHumanBase.h"

class NpcHumanGun : public NpcHumanBase
{
private:
    Delay cantAttackDelay;
    Delay stopMovingDelay;

    float accuracyModifier = 0;

    vec3 desiredTargetLocation = vec3();

    bool LineOfSightCheck(Entity* targetEntity);

    bool AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity);

protected:
    void ProcessAnimationEvent(AnimationEvent& event) override;

    void Attack() override;

    void LoadAssets() override;

public:
    NpcHumanGun();

    void AsyncUpdate() override;

    void Serialize(json& target) override;

    void Deserialize(json& source) override;
};
