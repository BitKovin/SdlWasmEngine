// NpcHumanGun.h
#pragma once

#include "NpcHumanBase.h"

class NpcHumanGun : public NpcHumanBase
{
private:
    // ── Shoot-and-scoot state ───────────────────────────────────────────────
    // repositioning: moving toward a specific, already-validated
    // repositionTarget. "Chasing" (closing distance on the target directly,
    // with no fixed destination of its own) has no matching member — it's
    // just a local flag inside AsyncUpdate, since nothing about it needs to
    // persist across frames or be saved/loaded.
    bool  repositioning         = false;
    vec3  repositionTarget      = vec3();
    float repositionElapsed     = 0.0f;
    vec3  desiredTargetLocation = vec3();
    int   shotsFired            = 0;
    int   shotsPerBurst         = 0;

    // ── Combat modifiers ──────────────────────────────────────────────────
    float accuracyModifier      = 0.0f;

    // ── Delays ────────────────────────────────────────────────────────────
    // cantAttackDelay is a general "don't call Attack() right now" gate
    // (spawn delay, run/stun animation lock, first-shot-retry throttle).
    // postBurstPauseDelay is kept entirely separate on purpose: it has
    // exactly one writer (burst completion in Attack()) and one reader
    // (the post-burst branch in AsyncUpdate), so nothing else sharing
    // cantAttackDelay can ever shorten, extend, or otherwise interfere
    // with the post-burst wait.
    Delay cantAttackDelay;
    Delay postBurstPauseDelay;

    // ── Tuning ────────────────────────────────────────────────────────────
    // Engagement distance band (NPC <-> target). idealEngageDistance is what
    // FindAttackLocation biases toward; the NPC still ranges across the
    // whole band over time (closer, then farther) rather than settling on
    // one exact distance.
    static constexpr float minEngageDistance      = 3.0f;
    static constexpr float maxEngageDistance       = 15.0f;
    static constexpr float idealEngageDistance     = 10.0f;

    // How far from its current position the NPC will search for a new
    // attack spot — bounds travel distance/time for a single reposition.
    static constexpr float repositionSearchRadius = 8.0f;

    // How close counts as "arrived" at a chosen position.
    static constexpr float arrivalRadius          = 1.0f;

    // Safety net: stop sprinting toward a spot after this long even if it
    // hasn't been reached yet (a moving obstacle, bad luck with pathing).
    // Not "smart" recovery logic — just a floor under how long a single
    // reposition is allowed to take.
    static constexpr float maxRepositionTime      = 4.0f;

    // Hold position for this long after the last shot of a burst so the
    // fire animation has time to finish before the NPC moves again.
    static constexpr float postBurstDelay         = 1.0f;

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
