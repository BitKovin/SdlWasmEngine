// NpcHumanAxe.cpp
#include "NpcHumanAxe.h"

REGISTER_ENTITY(NpcHumanAxe, "npc_human_axe")

NpcHumanAxe::NpcHumanAxe()
{
    ClassName = "npc_human_axe";
    maxSpeed  = 5.0f;

    pathFollow.allowPartialPath = true;

    Health    = 80;
    MaxHealth = 80;
}

// ---------------------------------------------------------------------------
// Animation events
// ---------------------------------------------------------------------------

void NpcHumanAxe::ProcessAnimationEvent(AnimationEvent& event)
{
    if (event.eventName == "attack_start")
    {
        attackingDamage = true;
    }

    if (event.eventName == "attack_end")
    {
        mesh->PlayAnimation("run", true, 0.5f);
        state           = NpcState::Chasing;
        attackingDamage = false;
    }

    if (event.eventName == "stun_end")
    {
        state = NpcState::Idle;
        mesh->PlayAnimation("run", true, 0.5f);
    }
}

// ---------------------------------------------------------------------------
// Stun
// ---------------------------------------------------------------------------

void NpcHumanAxe::Stun()
{
    NpcHumanBase::Stun();
    attackingDamage = false;
}

// ---------------------------------------------------------------------------
// Attack
// ---------------------------------------------------------------------------

void NpcHumanAxe::Attack()
{
    if (isOwned)
    {
        if (inAttackDelay.Wait()) return;
        inAttackDelay.AddDelay(1.5f);

        state = NpcState::Attacking;

        NetPacket attackArgs(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Attack), attackArgs, RPCTarget::All);
    }

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");

    mesh->PlayAnimation("attack");
    mesh->PullRootMotion();

    if (!isOwned)
        state = NpcState::Attacking;
}

// ---------------------------------------------------------------------------
// Damage
// ---------------------------------------------------------------------------

void NpcHumanAxe::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    NpcHumanBase::OnDamage(Damage, DamageCauser, Weapon);
}

// ---------------------------------------------------------------------------
// Hit detection (owner only)
// ---------------------------------------------------------------------------

void NpcHumanAxe::UpdateAttackDamage()
{
    if (!isOwned)         return;
    if (!attackingDamage) return;

    auto hit = Physics::SphereTrace(
        Position,
        MathHelper::GetForwardVector(mesh->Rotation) * 1.2f + Position,
        0.45f,
        BodyType::World | BodyType::CharacterCapsule,
        {}, {this});

    if (!hit.hasHit) return;

    if (hit.entity->HasTag("player"))
    {
        if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
        {
            OnPointDamage(20, hit.shapePosition,
                          MathHelper::FastNormalize(Position - hit.shapePosition),
                          "", this, this);
            Physics::AddImpulse(mesh->FindHitboxByName("spine_02"),
                                MathHelper::GetForwardVector(mesh->Rotation) * -500.0f);
            AddDebuffStacks("PoiseBreakDebuff", 100);
            attackingDamage = false;
            return;
        }
        else
        {
            hit.entity->OnPointDamage(15, hit.shapePosition,
                                      MathHelper::FastNormalize(hit.shapePosition - Position),
                                      "", this, this);
            attackingDamage = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Main update
// ---------------------------------------------------------------------------

void NpcHumanAxe::AsyncUpdate()
{
    if (IsDead())
    {
        mesh->UpdateHitboxes();
        UpdateStatusWidgets();
        return;
    }

    controller.Update(Time::DeltaTimeF);
    Position = controller.GetPosition();

    // Rebuild the resolved entity pointer every frame from the synced descriptor.
    ResolveTarget();

    // Perception runs only on the owner; it may call ApplyTarget which sends
    // the SetTarget RPC to keep non-owners in sync.
    if (isOwned)
        UpdatePerception();

    UpdateStatusWidgets();
    UpdateDebuffs(Time::DeltaTimeF);

    if (IsDead()) return;

    mesh->Update(ModifyAnimationSpeed(1.0f));

    auto animEvents = mesh->PullAnimationEvents();
    for (auto& ev : animEvents)
        ProcessAnimationEvent(ev);

    mesh->Position = Position - vec3(0, 1, 0);

    auto rootMotion = mesh->PullRootMotion();

    if (isOwned)
    {
        Position += rootMotion.Position;
        controller.SetPosition(Position);

        if (rootMotion.Position != vec3())
            controller.SetVelocity(vec3(0, controller.GetVelocity().y, 0));

        if (rootMotion.Rotation != vec3())
        {
            mesh->Rotation  += rootMotion.Rotation;
            movingDirection  = MathHelper::GetForwardVector(mesh->Rotation);
        }
    }

    UpdateStunnedReturn();
    UpdateReturnFromRagdoll();

    mesh->UpdateHitboxes();

    // soundPlayer can be nulled mid-frame by Death(), so guard here.
    if (soundPlayer)
    {
        soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);
        soundPlayer->Velocity = controller.GetVelocity();
    }

    if (IsDead() || IsStunned() || stunnedRagdoll || returningFromRagdoll) return;

    // ── Owner AI ──────────────────────────────────────────────────────────
    if (isOwned)
    {
        UpdateAttackDamage();
        if (IsDead()) return;

        if (resolvedTarget == nullptr)
        {
            // No target – idle in place.
            // Use the animation name as the gate, exactly as the original did,
            // to avoid fighting with state set elsewhere (e.g. after a stun).
            if (mesh->GetAnimationName() != "idle")
                mesh->PlayAnimation("idle", true, 0.5f);

            state = NpcState::Idle;

            vec3 vel = controller.GetVelocity();
            controller.SetVelocity(vec3(0, vel.y, 0));
            return;
        }

        // We have a target. If the mesh is still on "idle", kick off running.
        if (mesh->GetAnimationName() == "idle")
            mesh->PlayAnimation("run", true, 0.5f);

        if (state == NpcState::Idle)
            state = NpcState::Chasing;

        // Don't interrupt an ongoing attack.
        if (IsAttacking())
        {
            speed = 2.0f;
            return;
        }

        vec3 calculatedTargetPos = resolvedTarget->Position;

        if (resolvedTarget->Position.y < Position.y - 0.1f &&
            resolvedTarget->Position.y > Position.y - 1.5f)
        {
            calculatedTargetPos.y = Position.y;
        }

        vec3 lookAtDir = MathHelper::FastNormalize(calculatedTargetPos - Position);

        if (glm::distance(calculatedTargetPos, Position) < 1.5f &&
            glm::dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.93f)
        {
            Attack();
        }

        if (IsFleeing())
        {
            UpdateFleeTarget();
        }
        else
        {
            pathFollow.UpdateStartAndTarget(Position, resolvedTarget->Position);
            pathFollow.TryPerform();
        }

        if (pathFollow.FoundTarget)
        {
            desiredDirection = glm::normalize(
                MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
        }

        speed += Time::DeltaTimeF * 6.5f;
        speed  = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

        movingDirection = glm::mix(movingDirection, desiredDirection,
                                   (double)Time::DeltaTime * 10.0);
        movingDirection = MathHelper::FastNormalize(movingDirection);

        vec3 vel = controller.GetVelocity();
        controller.SetVelocity(vec3(movingDirection.x * speed,
                                    vel.y,
                                    movingDirection.z * speed));

        mesh->Rotation = vec3(0,
            MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
    }
    else
    {
        // Non-owner: mirror animation from replicated state and drive capsule.
        UpdateNonOwnerAnimation();

        vec3 vel = controller.GetVelocity();
        controller.SetVelocity(vec3(movingDirection.x * speed,
                                    vel.y,
                                    movingDirection.z * speed));
        mesh->Rotation = vec3(0,
            MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
    }
}

// ---------------------------------------------------------------------------
// Replication – axe-specific fields appended after the base snapshot
// ---------------------------------------------------------------------------

void NpcHumanAxe::NetSerialize(NetPacket& packet)
{
    NpcHumanBase::NetSerialize(packet);
    packet.WriteBool(attackingDamage);
}

void NpcHumanAxe::NetDeserialize(NetPacket& packet)
{
    NpcHumanBase::NetDeserialize(packet);
    attackingDamage = packet.ReadBool();
}

// ---------------------------------------------------------------------------
// Save-game serialization
// ---------------------------------------------------------------------------

void NpcHumanAxe::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);
    SERIALIZE_FIELD(target, attackingDamage);
}

void NpcHumanAxe::Deserialize(json& source)
{
    NpcHumanBase::Deserialize(source);
    DESERIALIZE_FIELD(source, attackingDamage);
}

// ---------------------------------------------------------------------------
// Asset loading
// ---------------------------------------------------------------------------

void NpcHumanAxe::LoadAssets()
{
    NpcHumanBase::LoadAssets();

    SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

    mesh->TexturesLocation = "GameData/models/enemies/humanAxe/humanAxe.glb/";
    mesh->LoadFromFile("GameData/models/enemies/humanAxe/humanAxe.glb");
    mesh->PreloadAssets();
    mesh->CreateHitboxes(this);
    mesh->PlayAnimation("run", true);
    mesh->SetLooped(true);
}
