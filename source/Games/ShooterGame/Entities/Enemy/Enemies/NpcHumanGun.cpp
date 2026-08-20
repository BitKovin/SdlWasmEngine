// NpcHumanGun.cpp
#include "NpcHumanGun.h"
#include "../../Player/Player.hpp"
#include "../../Player/Weapons/Projectiles/Bullet.h"
#include <RandomHelper.h>

REGISTER_ENTITY(NpcHumanGun, "npc_human_gun")

NpcHumanGun::NpcHumanGun()
{
    ClassName = "npc_human_gun";
    maxSpeed = 5.7f;
    mesh->Scale = vec3(1.15f);
    Health = 110;
    MaxHealth = 110;

    // No partial paths, for chasing or repositioning alike. A partial path
    // can route the NPC toward a ledge or gap it can't actually finish
    // crossing and have it walk off the end once the navmesh runs out —
    // every destination we send it to is expected to be fully, directly
    // reachable; if it isn't, we simply don't move there.
    pathFollow.allowPartialPath = false;

    // Prevent firing the instant the NPC spawns / gets ownership.
    cantAttackDelay.AddDelay(0.5f);
}

// ---------------------------------------------------------------------------
// Animation events
// ---------------------------------------------------------------------------

void NpcHumanGun::ProcessAnimationEvent(AnimationEvent& event)
{
    if (event.eventName == "stun_end")
    {
        state = NpcState::Idle;
        mesh->PlayAnimation("run", true, 0.5f);
    }
}

// ---------------------------------------------------------------------------
// Attack – fire a bullet toward the predicted target position.
// Bullet spawning is owner-only (authoritative). The animation is broadcast
// via RPC so all peers see the fire anim in sync.
//
// This only decides whether to keep firing or abandon the burst. It never
// picks a new position itself — shotsPerBurst == 0 is the sole signal the
// state machine in AsyncUpdate needs to know a burst isn't running, and it
// takes it from there (after minBurstInterval, or immediately on abandon).
// Keeping "what happens next" in one place is what makes it possible to
// reason about the whole loop.
// ---------------------------------------------------------------------------

void NpcHumanGun::Attack()
{
    if (isOwned)
    {
        if (cantAttackDelay.Wait()) return;
        if (inAttackDelay.Wait())   return;
        if (resolvedTarget == nullptr) return;

        // Enforce the minBurstInterval floor. If we got here (reposition
        // finished, or wasn't needed) faster than the window since the
        // last shot, just wait — shotsPerBurst is already > 0 so
        // AsyncUpdate's mid-burst branch keeps us standing in "aim" and
        // keeps calling Attack() each frame — rather than opening the next
        // burst the instant we arrive.
        if (shotsFired == 0 && burstIntervalDelay.Wait()) return;

        vec3 bonePos = mesh->GetBoneMatrixWorld("weapon_muzzle")[3];
        vec3 predictedTargetPosition = resolvedTarget->Position;

        // Only verify the actual bullet path on the first shot of a burst.
        // Once that first shot is confirmed clear, the rest of the burst
        // fires without re-checking — path/LOS only gates the start of a
        // burst. If even the first shot can't get a clear path, abandon the
        // burst; the state machine picks a new spot on its next pass.
        if (shotsFired == 0 &&
            !AttackDirectionCheck(bonePos, predictedTargetPosition, resolvedTarget))
        {
            shotsPerBurst = 0;
            cantAttackDelay.AddDelay(0.3f);
            return;
        }

        const float bulletSpeed = 40.0f;

        Player* playerRef = dynamic_cast<Player*>(resolvedTarget);
        if (playerRef)
        {
            predictedTargetPosition +=
                playerRef->controller.GetVelocity()
                * glm::distance(Position, resolvedTarget->Position) / bulletSpeed;
        }
        else if (resolvedTarget->LeadBody)
        {
            predictedTargetPosition +=
                FromPhysics(resolvedTarget->LeadBody->GetLinearVelocity())
                * glm::distance(Position, resolvedTarget->Position) / bulletSpeed;
        }

        predictedTargetPosition +=
            RandomHelper::RandomPosition(
                glm::distance(resolvedTarget->Position, Position) / 20.0f)
            * (accuracyModifier + 1.0f);

        vec3 bulletRotation = MathHelper::FindLookAtRotation(bonePos, predictedTargetPosition);

        Bullet* bullet = (Bullet*)Spawn("bullet");
        bullet->LoadAssets();
        bullet->Position = bonePos;
        bullet->Rotation = bulletRotation;
        bullet->Speed = bulletSpeed;
        bullet->OwnerTag = "enemy";
        bullet->Damage = 5;
        bullet->owner = this;
        bullet->Start();
        bullet->trail->Scale = vec3(1.5f);

        inAttackDelay.AddDelay(ModifyAnimationSpeed(0.5f));

        shotsFired++;
        if (shotsFired >= shotsPerBurst)
        {
            shotsFired = 0;
            shotsPerBurst = 0;

            // Two independent timers start here:
            //  - postBurstPauseDelay (short) just lets the fire animation
            //    finish before AsyncUpdate's state machine considers moving
            //    on to a reposition (see the "just finished a burst" branch
            //    there).
            //  - burstIntervalDelay (minBurstInterval) is the actual
            //    inter-burst floor, checked back at the top of this
            //    function on the next burst's first shot — so a quick
            //    reposition just means standing in "aim" for whatever's
            //    left of the window, and a slow one has already cleared it
            //    by the time the NPC arrives.
            postBurstPauseDelay.AddDelay(postBurstHoldTime);
            burstIntervalDelay.AddDelay(minBurstInterval);
        }

        // Broadcast animation to others.
        NetPacket attackArgs(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Attack), attackArgs, RPCTarget::Others);
    }

    // Animation and sound run on every peer.
    PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");
    mesh->PlayAnimation("fire");
    mesh->PullRootMotion();
}

// ---------------------------------------------------------------------------
// LOS / direction helpers
// ---------------------------------------------------------------------------

bool NpcHumanGun::LineOfSightCheck(Entity* targetEntity)
{
    auto hit = Physics::SphereTrace(
        Position + vec3(0, 0.4f, 0),
        targetEntity->Position + vec3(0, 0.3f, 0),
        0.4f,
        BodyType::World,
        {}, { this, targetEntity });

    return hit.hasHit == false;
}

bool NpcHumanGun::AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity)
{
    auto hit = Physics::SphereTrace(
        start, target, 0.2f,
        BodyType::GroupHitTest,
        {}, { this, targetEntity });

    return hit.hasHit == false;
}

bool NpcHumanGun::CheckAttackLOS(vec3 location, vec3 targetLocation)
{
    if (glm::distance(location, targetLocation) < 1.5f) return true;

    vec3 attackDir = glm::normalize(targetLocation - location);
    vec3 attackPos = location + vec3(0, 0.6f, 0) + attackDir;

    auto hit = Physics::SphereTrace(
        attackPos, targetLocation + vec3(0, 0.65f, 0),
        0.3f, BodyType::GroupHitTest, {}, { this, resolvedTarget });

    if (hit.hasHit && hit.entity != resolvedTarget)
        return false;

    return true;
}

bool NpcHumanGun::CheckAttackLocation(vec3 location, vec3 targetLocation)
{
    float dist = glm::distance(location, targetLocation);
    if (dist < minEngageDistance || dist > maxEngageDistance)
        return false;

    return CheckAttackLOS(location, targetLocation);
}


// ---------------------------------------------------------------------------
// Main update
// ---------------------------------------------------------------------------

void NpcHumanGun::AsyncUpdate()
{
    if (IsDead())
    {
        mesh->UpdateHitboxes();
        UpdateStatusWidgets();
        return;
    }

    controller.Update(Time::DeltaTimeF);
    Position = controller.GetPosition();
    speed = glm::length(MathHelper::XZ(controller.GetVelocity()));

    // Rebuild resolved target pointer every frame.
    ResolveTarget();

    UpdatePerception();

    accuracyModifier -= Time::DeltaTimeF / 3.0f;
    accuracyModifier = glm::clamp(accuracyModifier, 0.0f, 5.0f);

    UpdateStatusWidgets();
    UpdateDebuffs(Time::DeltaTimeF);

    if (IsDead()) return;

    mesh->Update(ModifyAnimationSpeed(1.0f));

    auto animEvents = mesh->PullAnimationEvents();
    for (auto& event : animEvents)
        ProcessAnimationEvent(event);

    mesh->Position = Position - vec3(0, 1, 0) + controller.GetSmoothOffset();

    auto rootMotion = mesh->PullRootMotion();

    // Root motion applies on ALL peers for smooth movement.
    Position += rootMotion.Position;
    controller.SetPosition(Position);
    if (rootMotion.Position != vec3())
        controller.SetVelocity(vec3(0, controller.GetVelocity().y, 0));
    if (rootMotion.Rotation != vec3())
    {
        mesh->Rotation += rootMotion.Rotation;
        movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
    }

    UpdateStunnedReturn();
    UpdateReturnFromRagdoll();

    mesh->UpdateHitboxes();

    // soundPlayer can be nulled mid-frame by Death().
    if (soundPlayer)
    {
        soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);
        soundPlayer->Velocity = controller.GetVelocity();
    }

    if (IsDead() || IsStunned() || stunnedRagdoll || returningFromRagdoll) return;

    // ── Owner AI ──────────────────────────────────────────────────────────
    if (isOwned)
    {
        // No target: hold aim, fully reset combat state.
        if (resolvedTarget == nullptr)
        {
            if (mesh->GetAnimationName() != "aim")
                mesh->PlayAnimation("aim", true, 0.5f);

            state = NpcState::Idle;
            repositioning = false;
            shotsFired = 0;
            shotsPerBurst = 0;

            vec3 vel = controller.GetVelocity();
            controller.SetVelocity(vec3(0, vel.y, 0));
            return;
        }

        if (state == NpcState::Idle)
            state = NpcState::Chasing;

        auto animName = mesh->GetAnimationName();

        // Running or stunned animations lock out attacking briefly, so a
        // burst never starts mid-transition.
        if (animName == "run" || animName == "stun")
            cantAttackDelay.AddDelay(0.7f);

        desiredTargetLocation = resolvedTarget->Position;
        vec3 lookAtDir = MathHelper::FastNormalize(resolvedTarget->Position - Position);

        // Upper bound only (no lower bound) — a target closer than
        // minEngageDistance still counts as "has a shot" here; that's what
        // sends it into a reposition search below rather than a "chase
        // closer" that would just walk it into the player. The bound is
        // maxAttackDistance, not maxEngageDistance: the NPC will fire from
        // farther than its preferred band (holding and shooting from the
        // current spot if nothing closer is reachable — see
        // FindAttackLocation's fallback), it just won't fire past
        // maxAttackDistance at all, no matter how clear the sightline.
        bool hasLineOfSight = LineOfSightCheck(resolvedTarget) &&
            glm::distance2(resolvedTarget->Position, Position)
            <= maxAttackDistance * maxAttackDistance;

        // A direct chase is only worth taking if the target isn't too far
        // to bother walking to at all, and there's a sane, fully-connected
        // route — no ledge/drop link, no absurd detour. CanReachDirectly is
        // only evaluated when it can actually matter (short-circuited by
        // the distance check) since it costs a pathfind.
        bool withinTravelRange = glm::distance2(resolvedTarget->Position, Position)
            <= maxTravelDistance * maxTravelDistance;

        bool chasing = false;

        // ── State machine ────────────────────────────────────────────────
        // Priority order, evaluated fresh every frame:
        //   1. Mid-reposition   — keep moving to repositionTarget, stop the
        //                         instant we arrive.
        //   2. Mid-burst        — keep firing until it's spent (Attack()
        //                         itself still withholds the very first
        //                         shot until burstIntervalDelay clears).
        //   3. Post-burst pause — just finished; hold still just long
        //                         enough for the fire animation to finish
        //                         (postBurstPauseDelay / postBurstHoldTime)
        //                         before considering a reposition.
        //   4. Have a shot      — go find a spot nearby and move to it (or
        //                         hold and shoot from here if nothing
        //                         nearby qualifies — see FindAttackLocation).
        //   5. No shot          — try FindAttackLocation anyway: it
        //                         validates LOS and path/detour per
        //                         candidate on its own, so it can still
        //                         find a spot that clears LOS even though
        //                         we don't have one right now. If that
        //                         comes up empty:
        //   5a.  reachable        — close the distance directly until we do.
        //   5b.  unreachable      — hold and face the target instead of
        //                           blindly running at it.
        // Deliberately not "smart": no flanking, no threat evaluation, just
        // a reachable, correctly-ranged, visible spot.

        if (repositioning)
        {
            if (animName != "run" && !IsStunned())
                mesh->PlayAnimation("run", true, 0.6f);

            desiredTargetLocation = repositionTarget;
            repositionElapsed += Time::DeltaTimeF;



            bool arrived = false;

            //DebugDraw::Point(desiredTargetLocation, 1.0f, 0.5f, DebugColor::Green);

            if (glm::distance(Position, repositionTarget) < 3)
            {



                if (CheckAttackLocation(Position, resolvedTarget->Position))
                {
                    arrived = glm::distance(Position, repositionTarget) < arrivalRadius || pathFollow.reachedTarget;
                }
                else
                {
                    arrived = pathFollow.reachedTarget;
                }
            }
            bool timedOut = repositionElapsed > maxRepositionTime;

            if (arrived || timedOut)
            {
                repositioning = false;
                shotsPerBurst = 3;
                shotsFired = 0;

                if (animName == "run")
                    mesh->PlayAnimation("aim", true, 0.3f);
            }
        }
        else if (shotsPerBurst > 0)
        {
            if (animName == "run")
                mesh->PlayAnimation("aim", true, 0.3f);
            else
                desiredDirection = lookAtDir;

            // Fire it out to the end regardless of visibility — LOS/path
            // were already confirmed before this burst started, either
            // below or in Attack()'s first-shot check.
            if (!cantAttackDelay.Wait())
                Attack();
        }
        else if (postBurstPauseDelay.Wait())
        {
            // Just finished a burst. Hold still and let the fire animation
            // finish before doing anything else.
            desiredDirection = lookAtDir;
        }
        else if (hasLineOfSight)
        {
            // Have a shot and nothing else in progress — find a spot
            // nearby (closer, farther, or about the same — see
            // FindAttackLocation) and move to it.
            repositioning = true;
            repositionElapsed = 0.0f;
            repositionTarget = FindAttackLocation();

            desiredTargetLocation = repositionTarget;
            pathFollow.reachedTarget = false;
        }
        else
        {
            // No shot from here — either LOS is blocked outright, or the
            // only route out would be an unreasonable detour (see
            // CanReachDirectly / FindAttackLocation's shared
            // maxDetourRatio check). Before falling back to a blind chase
            // or just holding, check whether some other spot within
            // repositionSearchRadius would actually give us a shot.
            // FindAttackLocation validates LOS and path/detour on each
            // candidate itself, so it works fine here without current
            // LOS — this is the same search branch 4 above uses, just
            // reached from "no shot" instead of "have a shot".
            vec3 vantagePoint = FindAttackLocation();
            bool foundVantagePoint = glm::distance(vantagePoint, Position) > arrivalRadius;

            if (foundVantagePoint)
            {
                // Found a reachable, validated spot — reposition there like
                // any other. Once repositioning is true, branch 1 takes
                // over next frame, so this search is paid for once per
                // "lost the shot" event, not every single frame.
                repositioning = true;
                repositionElapsed = 0.0f;
                repositionTarget = vantagePoint;

                desiredTargetLocation = repositionTarget;
                pathFollow.reachedTarget = false;
            }
            else if (withinTravelRange && CanReachDirectly(resolvedTarget->Position))
            {
                // No vantage point found nearby, but the target is close
                // enough to be worth closing the distance on, and there's a
                // normal, fully-connected route there. No search, no
                // destination of its own: just walk at the target.
                chasing = true;

                if (animName != "run" && !IsStunned())
                    mesh->PlayAnimation("run", true, 0.6f);

                desiredDirection = lookAtDir;
            }
            else
            {
                // Nothing worked — the target is too far to be worth the
                // trip, there's no sane direct route, and no reachable spot
                // nearby clears LOS either. Hold position and face the
                // target rather than running blindly at it — a real path
                // may open up later, or LOS may arrive and send us into a
                // proper reposition next pass.
                if (animName != "aim")
                    mesh->PlayAnimation("aim", true, 0.3f);

                desiredDirection = lookAtDir;
            }
        }

        // ── End state machine ────────────────────────────────────────────

        if (IsFleeing())
        {
            UpdateFleeTarget();
        }
        else
        {
            pathFollow.WaitToFinish();
            pathFollow.UpdateStartAndTarget(Position, desiredTargetLocation);
            pathFollow.TryPerform();
        }

        if (pathFollow.FoundTarget && animName == "run")
        {
            desiredDirection = glm::normalize(
                MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
        }

        speed += Time::DeltaTimeF * 4.5f;
        speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

        // Repositioning and chasing are the only phases meant to move the
        // NPC (that's what drives the "run" animation above). Every other
        // phase — holding aim, mid-burst, the post-burst pause — is fully
        // stationary.
        if (!repositioning && !chasing)
            speed = 0.0f;

        movingDirection = glm::mix(movingDirection, desiredDirection,
            (double)Time::DeltaTime * 10.0);
        movingDirection = MathHelper::FastNormalize(movingDirection);



        vec3 vel = controller.GetVelocity();
        if (controller.onGround)
        {
            controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));
        }

        mesh->Rotation = vec3(0,
            MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
    }
    else
    {
        // Non-owner: mirror animation from replicated state.
        // Gun NPC has an extra "aim" state when standing still with a target.
        if (!IsDead() && !IsStunned() && !stunnedRagdoll && !returningFromRagdoll && !IsAttacking())
        {
            auto animName = mesh->GetAnimationName();

            if ((mesh->IsAnimationPlaying() && animName == "fire" && mesh->GetAnimationTime() < 0.5) == false)
            {


                if (!resolvedTarget || IsIdle())
                {
                    if (animName != "aim") mesh->PlayAnimation("aim", true, 0.5f);
                }
                else if (speed > 0.1f)
                {
                    if (animName != "run") mesh->PlayAnimation("run", true, 0.5f);
                }
                else
                {
                    if (animName != "aim") mesh->PlayAnimation("aim", true, 0.3f);
                }

            }
        }

        vec3 vel = controller.GetVelocity();
        if (controller.onGround)
        {
            controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));
        }
        mesh->Rotation = vec3(0,
            MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
    }
}
// ---------------------------------------------------------------------------

void NpcHumanGun::NetSerialize(NetPacket& packet)
{
    NpcHumanBase::NetSerialize(packet);
    packet.WriteBool(repositioning);
    packet.WriteFloat(accuracyModifier);

    // repositionTarget and the burst counters are read directly by the
    // owner's AI decision branch every frame. A target switch can hand
    // ownership to any peer at any time -- not just on burst/reposition
    // boundaries, since NpcHumanGun never enters NpcState::Attacking and
    // TrySetTarget's state check therefore never blocks a switch mid-burst
    // or mid-reposition -- so whichever peer becomes the new owner needs
    // this already cached locally the instant it takes over. These used to
    // be treated as "owner-only movement state" and only went into the
    // save-game Serialize, so a freshly-owning peer fell back to defaults:
    // reposition toward (0,0,0), and any in-progress burst silently
    // treated as already finished. desiredTargetLocation itself still
    // doesn't need to be sent -- it's recomputed every frame from either
    // resolvedTarget->Position or repositionTarget, both of which are now
    // covered.
    packet.WriteVector3(repositionTarget);
    packet.WriteFloat(repositionElapsed);
    packet.WriteUInt8(static_cast<uint8_t>(shotsFired));
    packet.WriteUInt8(static_cast<uint8_t>(shotsPerBurst));
}

void NpcHumanGun::NetDeserialize(NetPacket& packet)
{
    NpcHumanBase::NetDeserialize(packet);

    bool    remoteRepositioning = packet.ReadBool();
    float   remoteAccuracyMod = packet.ReadFloat();
    vec3    remoteRepoTarget = packet.ReadVector3();
    float   remoteRepoElapsed = packet.ReadFloat();
    uint8_t remoteShotsFired = packet.ReadUInt8();
    uint8_t remoteShotsPerBurst = packet.ReadUInt8();

    // Same rule as the base class: discard snapshots that were still in
    // flight from the previous owner and arrive after this peer has
    // already taken ownership -- otherwise a stale mid-reposition/
    // mid-burst snapshot could stomp the state this peer is now
    // authoritatively driving. (The old code applied repositioning/
    // accuracyModifier unconditionally here, ignoring that guard.)
    if (isOwned) return;

    repositioning = remoteRepositioning;
    accuracyModifier = remoteAccuracyMod;
    repositionTarget = remoteRepoTarget;
    repositionElapsed = remoteRepoElapsed;
    shotsFired = remoteShotsFired;
    shotsPerBurst = remoteShotsPerBurst;
}

// ---------------------------------------------------------------------------
// Save-game serialization
// ---------------------------------------------------------------------------

void NpcHumanGun::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);

    SERIALIZE_FIELD(target, cantAttackDelay);
    SERIALIZE_FIELD(target, postBurstPauseDelay);
    SERIALIZE_FIELD(target, burstIntervalDelay);
    SERIALIZE_FIELD(target, desiredTargetLocation);
    SERIALIZE_FIELD(target, accuracyModifier);
    SERIALIZE_FIELD(target, repositioning);
    SERIALIZE_FIELD(target, repositionTarget);
    SERIALIZE_FIELD(target, repositionElapsed);
    SERIALIZE_FIELD(target, shotsFired);
    SERIALIZE_FIELD(target, shotsPerBurst);
}

void NpcHumanGun::Deserialize(json& source)
{
    NpcHumanBase::Deserialize(source);

    DESERIALIZE_FIELD(source, cantAttackDelay);
    DESERIALIZE_FIELD(source, postBurstPauseDelay);
    DESERIALIZE_FIELD(source, burstIntervalDelay);
    DESERIALIZE_FIELD(source, desiredTargetLocation);
    DESERIALIZE_FIELD(source, accuracyModifier);
    DESERIALIZE_FIELD(source, repositioning);
    DESERIALIZE_FIELD(source, repositionTarget);
    DESERIALIZE_FIELD(source, repositionElapsed);
    DESERIALIZE_FIELD(source, shotsFired);
    DESERIALIZE_FIELD(source, shotsPerBurst);
}

// ---------------------------------------------------------------------------
// Asset loading
// ---------------------------------------------------------------------------

void NpcHumanGun::LoadAssets()
{
    NpcHumanBase::LoadAssets();

    SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

    mesh->TexturesLocation = "GameData/models/enemies/humanGun/humanGun.glb/";
    mesh->LoadFromFile("GameData/models/enemies/humanGun/humanGun.glb");
    mesh->PreloadAssets();
    mesh->CreateHitboxes(this);
    mesh->PlayAnimation("run", true);
    mesh->SetLooped(true);
}



namespace
{
    struct Candidate
    {
        vec3  position; // navmesh-projected, walkable position
        float score = 0.0f;
    };
}

vec3 NpcHumanGun::FindAttackLocation() {
    if (resolvedTarget == nullptr)
        return Position;

    const vec3 currentPos = Position;
    const vec3 targetPos = resolvedTarget->Position;

    if (glm::distance(currentPos, targetPos) - repositionSearchRadius > maxEngageDistance)
        return currentPos;

    const int   maxAttempts = 32;
    const float minRadiusFrac = 0.2f;
    const float acceptanceRadius = 0.1f;
    const float navSnapTolerance = 0.75f;
    const float outOfRangeWeight = 1000.0f;

    std::vector<Candidate> candidates;
    candidates.reserve(maxAttempts);

    // --- PHASE 1: GENERATE + SCORE ---------------------------------------------------
    for (int i = 0; i < maxAttempts; ++i)
    {
        float angleRad = MathHelper::ToRadians(RandomHelper::RandomFloat() * 360.0f);
        float radius = glm::mix(repositionSearchRadius * minRadiusFrac,
            repositionSearchRadius,
            RandomHelper::RandomFloat());

        vec3 sample = currentPos + vec3(std::cos(angleRad) * radius, 0.0f, std::sin(angleRad) * radius);
        sample.y = currentPos.y;

        vec3 navPos;
        if (!NavigationSystem::ProjectPointToNavMesh(sample, glm::vec3(navSnapTolerance), navPos))
            continue;

        vec3 flatCandidate = vec3(navPos.x, 0.0f, navPos.z);
        vec3 flatTarget = vec3(targetPos.x, 0.0f, targetPos.z);
        float distToTarget = glm::distance(flatCandidate, flatTarget);

        float rangePenalty = 0.0f;
        if (distToTarget < minEngageDistance)
            rangePenalty = minEngageDistance - distToTarget;
        else if (distToTarget > maxEngageDistance)
            rangePenalty = distToTarget - maxEngageDistance;

        float distFromIdeal = std::abs(distToTarget - idealEngageDistance);
        float score = -(rangePenalty * outOfRangeWeight + distFromIdeal);

        candidates.push_back({ navPos, score });
    }

    // --- PHASE 2: SORT (best first based on initial engagement distance) -------------
    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b) { return a.score > b.score; });

    // --- PHASE 3: PATHFIND AND EVALUATE ALL VIABLE CANDIDATES ------------------------
    bool foundViableCandidate = false;
    vec3 bestCandidatePos = currentPos;
    float bestPathScore = std::numeric_limits<float>::max(); // Lower delta from 6m is better
    const float targetPathLength = 6.0f;

    for (auto candidate : candidates)
    {
        bool reached = false;
        std::vector<vec3> path = NavigationSystem::FindSimplePath(
            currentPos, candidate.position, acceptanceRadius, &reached, /*allowPartialPath=*/false);

        // Not actually reachable — skip
        if (!reached || path.empty())
            continue;

        // Don't reposition to a spot that's too close to the current position
        if (glm::distance(candidate.position, currentPos) < 3.0f)
            continue;

        // Reject a path that used a jump/drop/ladder/teleport link 
        bool usedSpecialLink = false;
        for (size_t p = 1; p < path.size() && !usedSpecialLink; ++p)
        {
            float verticalDelta = std::abs(path[p].y - path[p - 1].y);
            float horizontalDelta = glm::distance(vec3(path[p].x, 0, path[p].z), vec3(path[p - 1].x, 0, path[p - 1].z));
            if (verticalDelta > maxLedgeStepDelta && verticalDelta > horizontalDelta)
                usedSpecialLink = true;
        }

        if (usedSpecialLink)
            continue;

        // Calculate actual path length
        float pathLength = 0.0f;
        for (size_t p = 1; p < path.size(); ++p)
            pathLength += glm::distance(path[p - 1], path[p]);

        // Check line of sight / attack validity
        if (!CheckAttackLocation(path.back() + vec3(0, 1.2f, 0), targetPos))
            continue;

        // Check detour constraints
        float straightLineDist = glm::distance(currentPos, candidate.position);
        if (pathLength > straightLineDist * maxDetourRatio)
            continue;

        // --- NEW LOGIC: Track the candidate closest to 6 meters ---
        float currentDeltaFrom6m = std::abs(pathLength - targetPathLength);
        if (!foundViableCandidate || currentDeltaFrom6m < bestPathScore)
        {
            foundViableCandidate = true;
            bestPathScore = currentDeltaFrom6m;
            bestCandidatePos = path.back() + vec3(0, 0.3f, 0);
        }
    }

    // If we found at least one viable candidate, return the one closest to 6m path length
    if (foundViableCandidate)
        return bestCandidatePos;

    // Fallback: hold position
    return currentPos;
}

// ---------------------------------------------------------------------------
// Direct-chase reachability check
// ---------------------------------------------------------------------------
// Used only by the "no shot yet, close the distance" branch in AsyncUpdate.
// Holds a direct chase to the same standard FindAttackLocation holds a
// reposition to: fully connected (no partial path), no jump/drop/ladder
// link, and not an unreasonable detour versus the straight-line distance.
// If any of that fails, the caller holds position instead of walking
// blindly at the target's raw position — which is what used to send the NPC
// running down ledges it had no real path down to.
// ---------------------------------------------------------------------------

bool NpcHumanGun::CanReachDirectly(vec3 destination)
{
    const float acceptanceRadius = 0.1f;

    bool reached = false;
    std::vector<vec3> path = NavigationSystem::FindSimplePath(
        Position, destination, acceptanceRadius, &reached, /*allowPartialPath=*/false);

    if (!reached || path.empty())
        return false;

    // Reject a path that used a jump/drop/ladder/teleport link — same
    // heuristic FindAttackLocation uses.
    for (size_t p = 1; p < path.size(); ++p)
    {
        float verticalDelta = std::abs(path[p].y - path[p - 1].y);
        float horizontalDelta = glm::distance(
            vec3(path[p].x, 0, path[p].z), vec3(path[p - 1].x, 0, path[p - 1].z));
        if (verticalDelta > maxLedgeStepDelta && verticalDelta > horizontalDelta)
            return false;
    }

    float pathLength = 0.0f;
    for (size_t p = 1; p < path.size(); ++p)
        pathLength += glm::distance(path[p - 1], path[p]);

    float straightLineDist = glm::distance(Position, destination);
    if (pathLength > straightLineDist * maxDetourRatio)
        return false;

    return true;
}