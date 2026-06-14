// NpcHumanGun.cpp
#include "NpcHumanGun.h"
#include "../../Player/Player.hpp"
#include "../../Player/Weapons/Projectiles/Bullet.h"
#include <RandomHelper.h>

REGISTER_ENTITY(NpcHumanGun, "npc_human_gun")

NpcHumanGun::NpcHumanGun()
{
    ClassName = "npc_human_gun";
    maxSpeed  = 4.2f;
    mesh->Scale = vec3(1.15f);
    Health    = 80;
    MaxHealth = 80;
    pathFollow.allowPartialPath = true;

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
// ---------------------------------------------------------------------------

void NpcHumanGun::Attack()
{
    if (cantAttackDelay.Wait()) return;
    if (inAttackDelay.Wait())   return;

    // resolvedTarget is owner-only meaningful here; guard just in case.
    if (resolvedTarget == nullptr) return;

    vec3 bonePos = mesh->GetBoneMatrixWorld("weapon_muzzle")[3];

    vec3 predictedTargetPosition = resolvedTarget->Position;

    if (AttackDirectionCheck(bonePos, predictedTargetPosition, resolvedTarget) == false)
    {
        inAttackDelay.AddDelay(0.5f);
        return;
    }

    const float bulletSpeed = 50.0f;

    // Lead the target based on its velocity.
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

    // Spawn bullet on the owning peer only — it is a server-authoritative entity.
    Bullet* bullet = (Bullet*)Spawn("bullet");
    bullet->LoadAssets();
    bullet->Position = bonePos;
    bullet->Rotation = bulletRotation;
    bullet->Speed    = bulletSpeed;
    bullet->OwnerTag = "enemy";
    bullet->Damage   = 5;
    bullet->owner    = this;
    bullet->Start();

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");

    inAttackDelay.AddDelay(ModifyAnimationSpeed(0.5f));
    afterAttackDelay.AddDelay(3.0f);
    mesh->PlayAnimation("fire");
    mesh->PullRootMotion();

    // Broadcast to other peers so they play the fire animation in sync.
    NetPacket attackArgs(PacketType::RPC);
    SendRPC(static_cast<uint8_t>(NpcRPC::Attack), attackArgs, RPCTarget::Others);

    // Shoot-and-scoot: count shots; reposition once the burst is done.
    shotsFired++;
    if (shotsFired >= shotsPerBurst)
    {
        shotsFired    = 0;
        shotsPerBurst = 0;
        // Enforce a pause before the next burst whether we reposition or not.
        afterAttackDelay.AddDelay(1.5f);
        cantAttackDelay.AddDelay(0.5f);
        if (!pathFollow.reachedTarget)
        {
            repositioning    = true;
            repositionTarget = FindAttackLocation();
        }
    }
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
        0.1f, BodyType::GroupHitTest, mesh->hitboxBodies);

    if (hit.hasHit && hit.entity != resolvedTarget)
        return false;

    return true;
}

bool NpcHumanGun::CheckAttackLocation(vec3 location, vec3 targetLocation)
{
    if (glm::distance(location, targetLocation) > attackDesiredRange)
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

    // Perception and target switching are owner-only.
    if (isOwned)
        UpdatePerception();

    accuracyModifier -= Time::DeltaTimeF / 3.0f;
    accuracyModifier  = glm::clamp(accuracyModifier, 0.0f, 5.0f);

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
        mesh->Rotation  += rootMotion.Rotation;
        movingDirection  = MathHelper::GetForwardVector(mesh->Rotation);
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
        // No target: hold aim and reset attack window.
        if (resolvedTarget == nullptr)
        {
            if (mesh->GetAnimationName() != "aim")
                mesh->PlayAnimation("aim", true, 0.5f);

            afterAttackDelay.AddDelay(2.0f);

            state = NpcState::Idle;

            vec3 vel = controller.GetVelocity();
            controller.SetVelocity(vec3(0, vel.y, 0));
            return;
        }

        if (state == NpcState::Idle)
            state = NpcState::Chasing;

        auto animName = mesh->GetAnimationName();

        // Running or stunned animations lock out attacking briefly.
        if (animName == "run" || animName == "stun")
            cantAttackDelay.AddDelay(0.7f);

        desiredTargetLocation = resolvedTarget->Position;

        vec3 lookAtDir = MathHelper::FastNormalize(resolvedTarget->Position - Position);

        bool hasLineOfSight = LineOfSightCheck(resolvedTarget);

        if (glm::distance2(resolvedTarget->Position, Position)
            > attackDesiredRange * attackDesiredRange)
        {
            hasLineOfSight = false;
        }

        // ── Shoot-and-scoot state machine ──────────────────────────────────

        if (repositioning)
        {
            // REPOSITIONING phase: sprint to the chosen flank position.
            if (animName != "run" && !IsStunned())
                mesh->PlayAnimation("run", true, 0.6f);

            cantAttackDelay.AddDelay(0.5f);

            if (pathFollow.reachedTarget)
            {
                // Arrived – switch to shooting phase.
                repositioning  = false;
                shotsPerBurst  = 3 + (RandomHelper::RandomInt() % 2);
                shotsFired     = 0;

                if (animName == "run")
                    mesh->PlayAnimation("aim", true, 0.3f);
            }
            else
            {
                desiredTargetLocation = repositionTarget;
            }
        }
        else
        {
            // SHOOTING phase: stand still and fire the burst.

            // Roll burst size on first entry, but only after the inter-burst
            // pause (afterAttackDelay) has elapsed so the NPC doesn't fire again
            // immediately after exhausting a burst at its current position.
            if (shotsPerBurst == 0 && !afterAttackDelay.Wait())
            {
                shotsPerBurst = 2 + (RandomHelper::RandomInt() % 3);
                shotsFired    = 0;
            }

            if (hasLineOfSight)
                afterAttackDelay.AddDelay(3.0f);

            if (hasLineOfSight || afterAttackDelay.Wait())
            {
                if (animName == "run")
                    mesh->PlayAnimation("aim", true, 0.3f);
                else
                    desiredDirection = lookAtDir;

                if (!cantAttackDelay.Wait())
                    Attack();
            }
            else
            {
                // Lost LOS mid-burst. Only reposition if there is somewhere to go;
                // if reachedTarget is true we are already at the closest navmesh
                // point — repositioning would just loop back here immediately.
                if (!pathFollow.reachedTarget)
                {
                    if (animName != "run" && !IsStunned())
                        mesh->PlayAnimation("run", true, 0.6f);

                    cantAttackDelay.AddDelay(0.5f);
                    repositioning    = true;
                    repositionTarget = FindAttackLocation();
                    shotsFired       = 0;
                    shotsPerBurst    = 0;
                }
                else
                {
                    // Stuck at closest possible position with no LOS — hold aim.
                    if (animName == "run")
                        mesh->PlayAnimation("aim", true, 0.3f);
                }
            }
        }

        // ── End state machine ──────────────────────────────────────────────

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

        speed += Time::DeltaTimeF * 6.5f;
        speed  = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

        if ((hasLineOfSight || afterAttackDelay.Wait()) && !repositioning)
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
        // Non-owner: mirror animation from replicated state and drive capsule.
        UpdateNonOwnerAnimation();

        vec3 vel = controller.GetVelocity();
        controller.SetVelocity(vec3(movingDirection.x * speed, vel.y,
                                    movingDirection.z * speed));
        mesh->Rotation = vec3(0,
            MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
    }
}

// ---------------------------------------------------------------------------
// Replication
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
    repositioning    = packet.ReadBool();
    accuracyModifier = packet.ReadFloat();
}

// ---------------------------------------------------------------------------
// Save-game serialization
// ---------------------------------------------------------------------------

void NpcHumanGun::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);

    SERIALIZE_FIELD(target, cantAttackDelay);
    SERIALIZE_FIELD(target, desiredTargetLocation);
    SERIALIZE_FIELD(target, accuracyModifier);
    SERIALIZE_FIELD(target, repositioning);
    SERIALIZE_FIELD(target, repositionTarget);
    SERIALIZE_FIELD(target, shotsFired);
    SERIALIZE_FIELD(target, shotsPerBurst);
}

void NpcHumanGun::Deserialize(json& source)
{
    NpcHumanBase::Deserialize(source);

    DESERIALIZE_FIELD(source, cantAttackDelay);
    DESERIALIZE_FIELD(source, desiredTargetLocation);
    DESERIALIZE_FIELD(source, accuracyModifier);
    DESERIALIZE_FIELD(source, repositioning);
    DESERIALIZE_FIELD(source, repositionTarget);
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

// ---------------------------------------------------------------------------
// FindAttackLocation – tactical positioning for shoot-and-scoot.
// Unchanged logic; only reference to `target` replaced with `resolvedTarget`.
// ---------------------------------------------------------------------------

vec3 NpcHumanGun::FindAttackLocation()
{
    // Detection parameters
    const float tooCloseThreshold = 2.0f;

    // Candidate generation parameters
    const int   maxIterations           = 100;
    const int   normalTargetCandidates  = 20;
    const int   tooCloseTargetCandidates = 30;
    const float preferredMaxMove        = 5.0f;
    const float absoluteMaxMove         = 8.0f;

    // Ring multipliers
    struct Ring { float minMult; float maxMult; };
    const std::vector<Ring> rings = {
        {0.85f, 1.15f},  // Primary: optimal range
        {0.65f, 0.85f},  // Secondary: closer
        {0.40f, 0.65f},  // Tertiary: evasive close
    };

    // Normal mode angle biases (cumulative probabilities)
    const float normalLeftFlankProb       = 0.35f;
    const float normalRightFlankProb      = 0.35f;
    const float normalBackProb            = 0.20f;
    const float normalLeftFlankAngleBase  = -90.0f;
    const float normalLeftFlankAngleVar   =  80.0f;
    const float normalRightFlankAngleBase =  90.0f;
    const float normalRightFlankAngleVar  =  80.0f;
    const float normalBackAngleBase       = 180.0f;
    const float normalBackAngleVar        = 120.0f;
    const float normalFrontAngleBase      =   0.0f;
    const float normalFrontAngleVar       =  40.0f;

    // Too-close mode angle biases
    const float tooCloseDirectAwayProb       = 0.60f;
    const float tooCloseLeftFlankProb        = 0.20f;
    const float tooCloseDirectAwayAngleBase  =   0.0f;
    const float tooCloseDirectAwayAngleVar   =  20.0f;
    const float tooCloseLeftFlankAngleBase   =  60.0f;
    const float tooCloseLeftFlankAngleVar    =  60.0f;
    const float tooCloseRightFlankAngleBase  = -60.0f;
    const float tooCloseRightFlankAngleVar   =  60.0f;

    // Path checking parameters
    const float segmentDivisor = 2.5f;
    const int   maxSegments    = 10;

    // Fallback parameters
    const float fallbackDist     = 4.0f;
    const float fallbackDistMult = 0.8f;

    // Scoring parameters - common
    const float distSigma         = 0.15f;
    const float randomScoreMult   = 0.25f;
    const float moveScoreWeight   = 1.8f;
    const float circleScoreWeight = 1.1f;

    // Scoring parameters - normal
    const float normalMovePeak    = 0.3f;
    const float normalMoveSigma   = 0.25f;
    const float normalFlankWeight = 3.2f;
    const float normalDistWeight  = 2.1f;

    // Scoring parameters - too close
    const float tooCloseMovePeak    = 0.7f;
    const float tooCloseMoveSigma   = 0.35f;
    const float tooCloseFlankWeight = 1.8f;
    const float tooCloseDistWeight  = 3.2f;

    // Clustering penalty
    const float clusterSigma  = 2.0f;
    const float clusterWeight = 2.5f;

    // Scatter to break perfect circle
    const float scatterRadiusMult = 0.15f;

    const vec3& currentPos  = Position;
    const vec3& targetPos   = resolvedTarget->Position;
    const vec3& targetRot   = resolvedTarget->Rotation;
    vec3 targetForward = MathHelper::GetForwardVector(targetRot);
    targetForward.y = 0.0f;
    targetForward = MathHelper::Normalized(targetForward);
    vec3 targetRight = MathHelper::Normalized(
        glm::cross(targetForward, vec3(0.0f, 1.0f, 0.0f)));

    float currentDistXZ = glm::length(MathHelper::XZ(currentPos - targetPos));
    bool  isTooClose    = currentDistXZ < tooCloseThreshold;

    vec3 referenceForward = targetForward;
    vec3 referenceRight   = targetRight;

    int targetCandidatesLocal = normalTargetCandidates;
    if (isTooClose)
    {
        referenceForward      = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));
        referenceRight        = MathHelper::Normalized(
            glm::cross(referenceForward, vec3(0.0f, 1.0f, 0.0f)));
        targetCandidatesLocal = tooCloseTargetCandidates;
    }

    std::vector<vec3> otherNpcPositions;
    std::vector<vec3> candidates;

    int   iter                  = 0;
    const float desired         = attackDesiredRange;
    bool  relaxedMoveConstraint = false;

    for (const auto& ring : rings)
    {
        while (candidates.size() < static_cast<size_t>(targetCandidatesLocal)
               && iter < maxIterations)
        {
            ++iter;

            float r = RandomHelper::RandomFloat();
            float relAngleDeg;

            if (isTooClose)
            {
                if (r < tooCloseDirectAwayProb)
                    relAngleDeg = tooCloseDirectAwayAngleBase
                                  + (RandomHelper::RandomFloat() - 0.5f) * tooCloseDirectAwayAngleVar;
                else if (r < tooCloseDirectAwayProb + tooCloseLeftFlankProb)
                    relAngleDeg = tooCloseLeftFlankAngleBase
                                  + (RandomHelper::RandomFloat() - 0.5f) * tooCloseLeftFlankAngleVar;
                else
                    relAngleDeg = tooCloseRightFlankAngleBase
                                  + (RandomHelper::RandomFloat() - 0.5f) * tooCloseRightFlankAngleVar;
            }
            else
            {
                if (r < normalLeftFlankProb)
                    relAngleDeg = normalLeftFlankAngleBase
                                  + (RandomHelper::RandomFloat() - 0.5f) * normalLeftFlankAngleVar;
                else if (r < normalLeftFlankProb + normalRightFlankProb)
                    relAngleDeg = normalRightFlankAngleBase
                                  + (RandomHelper::RandomFloat() - 0.5f) * normalRightFlankAngleVar;
                else if (r < normalLeftFlankProb + normalRightFlankProb + normalBackProb)
                    relAngleDeg = normalBackAngleBase
                                  + (RandomHelper::RandomFloat() - 0.5f) * normalBackAngleVar;
                else
                    relAngleDeg = normalFrontAngleBase
                                  + (RandomHelper::RandomFloat() - 0.5f) * normalFrontAngleVar;
            }

            float relAngleRad = MathHelper::ToRadians(relAngleDeg);
            vec3 dirToPos = MathHelper::Normalized(
                std::cos(relAngleRad) * referenceForward +
                std::sin(relAngleRad) * referenceRight);

            float distMult = ring.minMult
                             + RandomHelper::RandomFloat() * (ring.maxMult - ring.minMult);
            float dist = std::min(desired * distMult, desired);
            vec3 newPos = targetPos + dirToPos * dist;

            float scatterRadius = desired * scatterRadiusMult;
            vec3 scatter = RandomHelper::RandomPosition(scatterRadius);
            scatter.y = 0.0f;
            newPos += scatter;

            if (glm::length(MathHelper::XZ(newPos - targetPos)) > desired) continue;

            float moveDist = glm::distance(newPos, currentPos);
            float maxMove  = relaxedMoveConstraint ? absoluteMaxMove : preferredMaxMove;
            if (moveDist > maxMove) continue;

            auto pathHit = Physics::CylinderTrace(
                currentPos, newPos, 0.5f, 0.8f,
                BodyType::World | BodyType::MainBody);
            if (pathHit.hasHit) continue;

            bool pathLOSClear = true;
            vec3 diff    = newPos - currentPos;
            float pathLen = glm::length(diff);
            int segments  = std::min(maxSegments,
                                     1 + static_cast<int>(pathLen / segmentDivisor));
            for (int s = 1; s < segments; ++s)
            {
                float t      = static_cast<float>(s) / static_cast<float>(segments);
                vec3  midPos = currentPos + diff * t;
                if (!CheckAttackLocation(midPos, targetPos))
                {
                    pathLOSClear = false;
                    break;
                }
            }
            if (!pathLOSClear) continue;

            if (!CheckAttackLocation(newPos, targetPos)) continue;

            candidates.push_back(newPos);
        }

        if (candidates.size() < static_cast<size_t>(targetCandidatesLocal / 2))
            relaxedMoveConstraint = true;
        if (candidates.size() >= static_cast<size_t>(targetCandidatesLocal / 2))
            break;
    }

    // Ultimate fallback
    if (candidates.empty())
    {
        if (isTooClose)
        {
            vec3 radialDir = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));
            vec3 perpRight = MathHelper::Normalized(
                glm::cross(radialDir, vec3(0.0f, 1.0f, 0.0f)));
            vec3 fallbackDirs[2] = { perpRight, -perpRight };
            float fallbackDistLocal = std::min(fallbackDist, desired * fallbackDistMult);
            for (int i = 0; i < 2; ++i)
            {
                vec3 fallbackPos = currentPos + fallbackDirs[i] * fallbackDistLocal;
                auto h = Physics::CylinderTrace(currentPos, fallbackPos, 0.5f, 0.8f,
                                                BodyType::World | BodyType::MainBody);
                if (!h.hasHit && CheckAttackLocation(fallbackPos, targetPos))
                    return fallbackPos;
            }
            return currentPos;
        }
        else
        {
            return targetPos;
        }
    }

    // Advanced scoring
    vec3  bestPos   = candidates[0];
    float bestScore = -std::numeric_limits<float>::max();

    vec3 currRelDir = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));

    float movePeak   = isTooClose ? tooCloseMovePeak   : normalMovePeak;
    float moveSigma  = isTooClose ? tooCloseMoveSigma  : normalMoveSigma;
    float flankWeight = isTooClose ? tooCloseFlankWeight : normalFlankWeight;
    float distWeight  = isTooClose ? tooCloseDistWeight  : normalDistWeight;

    for (const vec3& cand : candidates)
    {
        vec3  toTarget  = MathHelper::XZ(cand - targetPos);
        float cDist     = glm::length(toTarget);
        vec3  attackDir = (cDist > 0.001f) ? toTarget / cDist : vec3(0.0f, 0.0f, 1.0f);

        float tacticalScore = -glm::dot(attackDir, referenceForward);

        float distDiff  = (cDist - desired) / desired;
        float distScore = std::exp(-distDiff * distDiff / (2.0f * distSigma * distSigma));

        float moveDistScore = glm::distance(cand, currentPos);
        float moveNorm      = moveDistScore / desired;
        float moveScore     = std::exp(-(moveNorm - movePeak) * (moveNorm - movePeak)
                                       / (2.0f * moveSigma * moveSigma));

        vec3  moveDir    = MathHelper::Normalized(MathHelper::XZ(cand - currentPos));
        float circleScore = 1.0f - std::abs(glm::dot(moveDir, currRelDir));

        float randScore  = RandomHelper::RandomFloat() * randomScoreMult;

        float clusterPenalty = 0.0f;
        for (const vec3& otherPos : otherNpcPositions)
        {
            float d = glm::distance(cand, otherPos);
            if (d < 0.001f) continue;
            clusterPenalty += std::exp(-(d * d) / (2.0f * clusterSigma * clusterSigma));
        }

        float score = tacticalScore  * flankWeight
                    + distScore      * distWeight
                    + moveScore      * moveScoreWeight
                    + circleScore    * circleScoreWeight
                    + randScore
                    - clusterPenalty * clusterWeight;

        if (score > bestScore)
        {
            bestScore = score;
            bestPos   = cand;
        }
    }

    return bestPos;
}
