// NpcHumanGun.h
#pragma once

#include "NpcHumanBase.h"

class NpcHumanGun : public NpcHumanBase
{
private:
    // ── Shoot-and-scoot state ─────────────────────────────────────────────
    bool  repositioning      = false;
    vec3  repositionTarget   = vec3();
    float repositionElapsed = 0.0f;
    vec3  desiredTargetLocation = vec3();
    int   shotsFired         = 0;
    int   shotsPerBurst      = 0;

    // ── Combat modifiers ──────────────────────────────────────────────────
    float accuracyModifier   = 0.0f;

    // ── Delays ────────────────────────────────────────────────────────────
    Delay cantAttackDelay;

    static constexpr float attackDesiredRange = 34.0f;

    // ── Helpers ───────────────────────────────────────────────────────────
    bool  LineOfSightCheck    (Entity* targetEntity);
    bool  AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity);
    bool  CheckAttackLOS      (vec3 location, vec3 targetLocation);
    bool  CheckAttackLocation (vec3 location, vec3 targetLocation);
    vec3  FindAttackLocation  ();

protected:
    void ProcessAnimationEvent(AnimationEvent& event) override;
    void Attack()     override;
    void LoadAssets() override;

public:
    NpcHumanGun();

    void AsyncUpdate() override;

    void Serialize  (json& target) override;
    void Deserialize(json& source) override;

    // ── Replication ───────────────────────────────────────────────────────
    void NetSerialize  (NetPacket& packet) override;
    void NetDeserialize(NetPacket& packet) override;
};
