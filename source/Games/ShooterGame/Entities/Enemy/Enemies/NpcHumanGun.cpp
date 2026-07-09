// NpcHumanGun.cpp
#include "NpcHumanGun.h"
#include "../../Player/Player.hpp"
#include "../../Player/Weapons/Projectiles/Bullet.h"
#include <RandomHelper.h>

REGISTER_ENTITY(NpcHumanGun, "npc_human_gun")

NpcHumanGun::NpcHumanGun()
{
    ClassName = "npc_human_gun";
    maxSpeed = 4.2f;
    mesh->Scale = vec3(1.15f);
    Health = 90;
    MaxHealth = 90;

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
// takes it from there (after postBurstDelay, or immediately on abandon).
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

        const float bulletSpeed = 50.0f;

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

        inAttackDelay.AddDelay(ModifyAnimationSpeed(0.5f));

        shotsFired++;
        if (shotsFired >= shotsPerBurst)
        {
            shotsFired = 0;
            shotsPerBurst = 0;

            // Hold position for postBurstDelay so the fire animation has
            // time to finish before AsyncUpdate's state machine moves on
            // (see the "just finished a burst" branch there). This is a
            // dedicated timer specifically so nothing else touching
            // cantAttackDelay (the run/stun lock, the retry throttle
            // below, the spawn delay) can shorten or extend it.
            postBurstPauseDelay.AddDelay(postBurstDelay);
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
        0.3f, BodyType::GroupHitTest, {}, {this, resolvedTarget});

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

    mesh->Position = Position - vec3(0, 1, 0);

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

        // Upper bound only (no lower bound) — a target that's closer than
        // minEngageDistance still counts as "has a shot" here. That's what
        // sends it into a reposition search below rather than a "chase
        // closer" that would just walk it into the player.
        bool hasLineOfSight = LineOfSightCheck(resolvedTarget) &&
            glm::distance2(resolvedTarget->Position, Position)
                <= maxEngageDistance * maxEngageDistance;

        bool chasing = false;

        // ── State machine ────────────────────────────────────────────────
        // Priority order, evaluated fresh every frame:
        //   1. Mid-reposition   — keep moving to repositionTarget, stop the
        //                         instant we arrive.
        //   2. Mid-burst        — keep firing until it's spent.
        //   3. Post-burst pause — just finished (or about to start); hold
        //                         still until postBurstPauseDelay clears.
        //   4. Have a shot      — go find a spot nearby and move to it.
        //   5. No shot          — close the distance directly until we do.
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
            // No shot available — close the distance directly until there
            // is one. No search, no destination of its own: just walk at
            // the target.
            chasing = true;

            if (animName != "run" && !IsStunned())
                mesh->PlayAnimation("run", true, 0.6f);

            desiredDirection = lookAtDir;
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
        controller.SetVelocity(vec3(movingDirection.x * speed, vel.y,
            movingDirection.z * speed));

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
        controller.SetVelocity(vec3(movingDirection.x * speed, vel.y,
            movingDirection.z * speed));
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
    // repositionTarget and desiredTargetLocation are owner-only movement state;
    // non-owners just follow the replicated position/direction from the base.
}

void NpcHumanGun::NetDeserialize(NetPacket& packet)
{
    NpcHumanBase::NetDeserialize(packet);
    repositioning = packet.ReadBool();
    accuracyModifier = packet.ReadFloat();
}

// ---------------------------------------------------------------------------
// Save-game serialization
// ---------------------------------------------------------------------------

void NpcHumanGun::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);

    SERIALIZE_FIELD(target, cantAttackDelay);
    SERIALIZE_FIELD(target, postBurstPauseDelay);
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

vec3 NpcHumanGun::FindAttackLocation()
{
    if (resolvedTarget == nullptr)
        return Position;

    const vec3 currentPos = Position;
    const vec3 targetPos = resolvedTarget->Position;

    const int   maxAttempts = 32;
    const float minRadiusFrac = 0.6f;   // always move a noticeable amount, not 2 inches
    const float acceptanceRadius = 0.1f;
    const float pathEndTolerance = acceptanceRadius * 2.0f;
    const float maxDetourRatio = 1.75f;  // allowed path length vs straight line
    const float maxStepDelta = 0.5f;   // heuristic jump/drop link detector
    const float navSnapTolerance = 0.75f;  // how far a raw sample may be from walkable navmesh
    const float outOfRangeWeight = 1000.0f; // in-range candidates always outrank out-of-range ones

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


        // Snap to the walkable navmesh now, cheaply, rather than finding out during the
        // expensive pathfind that the raw sample wasn't usable at all.
        vec3 navPos;
        if (!NavigationSystem::ProjectPointToNavMesh(sample, glm::vec3(navSnapTolerance), navPos))
            continue;

        //DebugDraw::Point(navPos, 3);

        vec3 flatCandidate = vec3(navPos.x, 0.0f, navPos.z);
        vec3 flatTarget = vec3(targetPos.x, 0.0f, targetPos.z);
        float distToTarget = glm::distance(flatCandidate, flatTarget);

        // To make being in-range a hard requirement again (reject anything outside the
        // band instead of treating it as a fallback), uncomment:
        // if (distToTarget < minEngageDistance || distToTarget > maxEngageDistance) continue;

        float rangePenalty = 0.0f;
        if (distToTarget < minEngageDistance)
            rangePenalty = minEngageDistance - distToTarget;
        else if (distToTarget > maxEngageDistance)
            rangePenalty = distToTarget - maxEngageDistance;

        float distFromIdeal = std::abs(distToTarget - idealEngageDistance);
        float score = -(rangePenalty * outOfRangeWeight + distFromIdeal);

        candidates.push_back({ navPos, score });
    }

    // --- PHASE 2: SORT (best first) ---------------------------------------------------
    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b) { return a.score > b.score; });

    // --- PHASE 3: PATHFIND, BEST CANDIDATE FIRST --------------------------------------
    for (auto candidate : candidates)
    {
        bool reached = false;
        std::vector<vec3> path = NavigationSystem::FindSimplePath(
            currentPos, candidate.position, acceptanceRadius, &reached, /*allowPartialPath=*/false);


		if (glm::distance(candidate.position, currentPos) < 4.0f)
		{
			// Don't reposition to a spot that's too close to the current position.
			continue;
		}



        //if (glm::distance(path.back(), candidate.position) > pathEndTolerance)
            //continue;

        // Reject a path that used a jump/drop/ladder/teleport link.
        bool usedSpecialLink = false;
        for (size_t p = 1; p < path.size() && !usedSpecialLink; ++p)
        {
            float verticalDelta = std::abs(path[p].y - path[p - 1].y);
            float horizontalDelta = glm::distance(vec3(path[p].x, 0, path[p].z), vec3(path[p - 1].x, 0, path[p - 1].z));
            if (verticalDelta > maxStepDelta && verticalDelta > horizontalDelta)
                usedSpecialLink = true;
        }



        if (usedSpecialLink)
            continue;

        float pathLength = 0.0f;
        for (size_t p = 1; p < path.size(); ++p)
            pathLength += glm::distance(path[p - 1], path[p]);

        if (!CheckAttackLocation(path.back() + vec3(0, 1.2f, 0), targetPos))
        {
            continue;
        }

        float straightLineDist = glm::distance(currentPos, candidate.position);
        if (pathLength > straightLineDist * maxDetourRatio)
            continue;

        //DebugDraw::Point(path.back(), 3, 0.6f, DebugColor::Blue);

        // Best-scoring reachable candidate - done.
        return path.back() + vec3(0, 0.3f, 0);
    }

    // Fallback: nothing at all was reachable (fully cornered, or no navmesh nearby) - hold.
    //DebugDraw::Point(currentPos, 3, 0.6f);
    return currentPos;
}