#pragma once

#include "NpcHumanBase.h"

class NpcHumanGun : public NpcHumanBase
{
private:
    Delay cantAttackDelay;

    float accuracyModifier = 0;

    float attackDesiredRange = 30;

    vec3 desiredTargetLocation = vec3();

    // Shoot-and-scoot state
    int shotsFired = 0;
    int shotsPerBurst = 0;
    bool repositioning = false;
    vec3 repositionTarget = vec3();

    bool CheckAttackLOS(vec3 location, vec3 targetLocation);
    bool CheckAttackLocation(vec3 location, vec3 targetLocation);

    bool LineOfSightCheck(Entity* targetEntity);

    bool AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity);

protected:
    void ProcessAnimationEvent(AnimationEvent& event) override;

    void Attack() override;

    void LoadAssets() override;

    vec3 FindAttackLocation();

public:
    NpcHumanGun();

    void AsyncUpdate() override;

    void Serialize(json& target) override;

    void Deserialize(json& source) override;
};