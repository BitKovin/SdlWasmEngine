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
    //
    // postBurstPauseDelay and burstIntervalDelay both start together when a
    // burst finishes (in Attack()), but they gate two different things and
    // are kept separate on purpose:
    //   - postBurstPauseDelay only holds the NPC in place long enough for
    //     the fire animation to finish (postBurstHoldTime) before a
    //     reposition is allowed to begin (the post-burst branch in
    //     AsyncUpdate). It does NOT enforce the inter-burst gap.
    //   - burstIntervalDelay IS the minBurstInterval floor: it's read at
    //     the start of the next burst (shotsFired == 0, in Attack()) and
    //     simply refuses to fire the first shot until it clears. A quick
    //     reposition just means standing in "aim" for whatever's left of
    //     the window instead of firing the instant it arrives; a slow
    //     reposition has already cleared it by the time it arrives.
    // Neither has any other writer or reader, so nothing else sharing
    // cantAttackDelay can shorten, extend, or otherwise interfere with
    // either one.
    Delay cantAttackDelay;
    Delay postBurstPauseDelay;
    Delay burstIntervalDelay;

    // ── Tuning ────────────────────────────────────────────────────────────
    // Engagement distance band (NPC <-> target). idealEngageDistance is what
    // FindAttackLocation biases toward; the NPC still ranges across the
    // whole band over time (closer, then farther) rather than settling on
    // one exact distance.
    static constexpr float minEngageDistance      = 3.0f;
    static constexpr float maxEngageDistance       = 10.0f;
    static constexpr float idealEngageDistance     = 7.0f;

    // How far from its current position the NPC will search for a new
    // attack spot — bounds travel distance/time for a single reposition.
    static constexpr float repositionSearchRadius = 11.0f;

    // How close counts as "arrived" at a chosen position.
    static constexpr float arrivalRadius          = 1.0f;

    // Safety net: stop sprinting toward a spot after this long even if it
    // hasn't been reached yet (a moving obstacle, bad luck with pathing).
    // Not "smart" recovery logic — just a floor under how long a single
    // reposition is allowed to take.
    static constexpr float maxRepositionTime      = 4.0f;

    // How long to stand still after the last shot of a burst before a
    // reposition is allowed to begin — just long enough for the fire
    // animation to finish. Deliberately short and independent of
    // minBurstInterval below: this only delays the START of a reposition,
    // never the burst that follows it.
    static constexpr float postBurstHoldTime      = 0.5f;

    // Minimum time between the last shot of one burst and the first shot of
    // the next — a fixed safe window for the player, guaranteed regardless
    // of whether a reposition happens to eat up part of it (a fast/instant
    // reposition doesn't shorten it; a slow one just adds on top). Enforced
    // by burstIntervalDelay at the start of the next burst rather than by
    // holding the NPC still up front — it's free to start repositioning as
    // soon as postBurstHoldTime clears, and only waits in place afterward
    // if it arrives with time still left on this floor.
    static constexpr float minBurstInterval       = 3.0f;

    // Hard cap on actually firing: with LOS but beyond this, the NPC won't
    // shoot at all, however far outside maxEngageDistance it's willing to
    // reposition or hold. Between maxEngageDistance and maxAttackDistance,
    // it'll still fire — from the current spot if nothing closer is
    // reachable (see FindAttackLocation's fallback) — just from farther
    // than ideal. Distinct from maxTravelDistance: this gates whether a
    // shot is taken at all, not how far the NPC is willing to walk.
    // Placeholder value — tune to the weapon's actual effective range.
    static constexpr float maxAttackDistance      = 30.0f;

    // How far the NPC is willing to travel to close distance on a target it
    // can't yet get a shot at. Beyond this, a direct chase isn't worth it —
    // see the "no shot, too far / unreachable" branch in AsyncUpdate, which
    // holds position and just keeps facing the target instead.
    static constexpr float maxTravelDistance      = 60.0f;

    // How much longer a path is allowed to be than the straight-line
    // distance it covers before it counts as "too big a detour" and gets
    // rejected. Shared by FindAttackLocation (picking a reposition spot)
    // and CanReachDirectly (deciding whether a direct chase is worth it).
    static constexpr float maxDetourRatio         = 1.75f;

    // Heuristic jump/drop/ladder/teleport-link detector: a path segment
    // whose vertical rise/fall exceeds this AND exceeds its own horizontal
    // movement is treated as a special link rather than a normal walk, and
    // rejected. Shared by FindAttackLocation and CanReachDirectly — this is
    // what stops the NPC from running down a ledge to reach a target.
    static constexpr float maxLedgeStepDelta      = 0.5f;

    // ── Helpers ───────────────────────────────────────────────────────────
    bool  LineOfSightCheck    (Entity* targetEntity);
    bool  AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity);
    bool  CheckAttackLOS      (vec3 location, vec3 targetLocation);
    bool  CheckAttackLocation (vec3 location, vec3 targetLocation);
    vec3  FindAttackLocation  ();
    bool  CanReachDirectly    (vec3 destination);

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
