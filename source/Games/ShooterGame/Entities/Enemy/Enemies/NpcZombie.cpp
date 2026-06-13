// NpcZombie.cpp
#include "NpcHumanBase.h"

// ---------------------------------------------------------------------------
// NpcZombie
//
// Slow melee enemy. Uses time-based attack transitions instead of animation
// events (the zombie rig has no event tracks). No separate .h needed.
// ---------------------------------------------------------------------------

class NpcZombie : public NpcHumanBase
{
private:
    bool attackDamageDealt = false;

    static constexpr float AttackDamageDelay  = 0.7f;   // seconds into animation to deal damage
    static constexpr float AttackEndThreshold = 0.15f;  // seconds before animation end to transition out

    void UpdateAttackDamage();
    void UpdateAttackState();

protected:
    void Attack()     override;
    void LoadAssets() override;

public:
    NpcZombie();

    void Stun() override;

    void OnDamage(float Damage, Entity* DamageCauser = nullptr,
                  Entity* Weapon = nullptr) override;

    void AsyncUpdate() override;

    void Serialize  (json& target) override;
    void Deserialize(json& source) override;

    void NetSerialize  (NetPacket& packet) override;
    void NetDeserialize(NetPacket& packet) override;
};

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

NpcZombie::NpcZombie()
{
    ClassName = "npc_zombie";
    maxSpeed  = 1.5f;

    canBeStunRagdolled = false;

    Health    = 100;
    MaxHealth = 100;
}

// ---------------------------------------------------------------------------
// Stun
// ---------------------------------------------------------------------------

void NpcZombie::Stun()
{
    NpcHumanBase::Stun();  // sets state = Stunned
    attackDamageDealt = false;
}

// ---------------------------------------------------------------------------
// Attack
// ---------------------------------------------------------------------------

void NpcZombie::Attack()
{
    // Owner checks cooldown and broadcasts; non-owners just play the animation.
    if (isOwned)
    {
        if (inAttackDelay.Wait()) return;
        inAttackDelay.AddDelay(1.5f);

        state = NpcState::Attacking;

        NetPacket attackArgs(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Attack), attackArgs, RPCTarget::All);
    }

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");

    mesh->PlayAnimation("attack", false, 0.3f);
    mesh->PullRootMotion();
    attackDamageDealt = false;

    if (!isOwned)
        state = NpcState::Attacking;
}

// ---------------------------------------------------------------------------
// Damage
// ---------------------------------------------------------------------------

void NpcZombie::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    NpcHumanBase::OnDamage(Damage, DamageCauser, Weapon);
}

// ---------------------------------------------------------------------------
// Hit detection – fires once when animation clock passes AttackDamageDelay.
// Owner only; result is authoritative.
// ---------------------------------------------------------------------------

void NpcZombie::UpdateAttackDamage()
{
    if (!isOwned)          return;
    if (attackDamageDealt) return;
    if (mesh->GetAnimationTime() < AttackDamageDelay) return;

    auto hit = Physics::SphereTrace(
        Position,
        MathHelper::GetForwardVector(mesh->Rotation) * 1.2f + Position,
        0.45f,
        BodyType::World | BodyType::CharacterCapsule,
        { controller.body });

    // No contact yet — keep checking each frame so parry timing can land.
    if (!hit.hasHit || !hit.entity->HasTag("player")) return;

    attackDamageDealt = true;

    if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
    {
        OnPointDamage(20, hit.shapePosition,
                      MathHelper::FastNormalize(Position - hit.shapePosition),
                      "", this, this);
        Physics::AddImpulse(mesh->FindHitboxByName("spine_02"),
                            MathHelper::GetForwardVector(mesh->Rotation) * -500.0f);
        AddDebuffStacks("PoiseBreakDebuff", 100);
    }
    else
    {
        hit.entity->OnPointDamage(15, hit.shapePosition,
                                  MathHelper::FastNormalize(hit.shapePosition - Position),
                                  "", this, this);
    }
}

// ---------------------------------------------------------------------------
// Attack state – watches animation time to transition out without events.
// ---------------------------------------------------------------------------

void NpcZombie::UpdateAttackState()
{
    float dur = mesh->GetAnimationDuration();
    if (dur > 0.0f && mesh->GetAnimationTime() >= dur - AttackEndThreshold)
    {
        state             = NpcState::Chasing;
        attackDamageDealt = false;
        mesh->PlayAnimation("run", true, 0.5f);
    }
}

// ---------------------------------------------------------------------------
// Main update
// ---------------------------------------------------------------------------

void NpcZombie::AsyncUpdate()
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

    UpdateStatusWidgets();
    UpdateDebuffs(Time::DeltaTimeF);

    if (IsDead()) return;

    mesh->Update(ModifyAnimationSpeed(1.0f));

    mesh->Position = Position - vec3(0, 1, 0);

    // Zombie only applies root motion during stun animations, not during attack.
    // Owner-gate it for the same reason as NpcHumanAxe.
    auto rootMotion = mesh->PullRootMotion();
    if (isOwned && (IsStunned() || stunnedRagdoll))
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

    // Time-based stun end (zombie rig has no stun_end animation event).
    if (IsStunned() && !stunnedRagdoll && !returningFromRagdoll)
    {
        float dur = mesh->GetAnimationDuration();
        if (dur > 0.0f && mesh->GetAnimationTime() >= dur - AttackEndThreshold)
        {
            state = NpcState::Idle;
            mesh->PlayAnimation("run", true, 0.5f);
        }
    }

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
        // Attack phase: deal damage at fixed time, then transition out.
        if (IsAttacking())
        {
            UpdateAttackDamage();
            if (IsDead()) return;

            if (IsAttacking()) // parry may have called Stun() and cleared it
                UpdateAttackState();

            if (IsAttacking()) // still in attack window — hold in place
            {
                vec3 vel = controller.GetVelocity();
                controller.SetVelocity(vec3(0, vel.y, 0));
                return;
            }
        }

        if (IsDead()) return;

        // No target: idle.
        if (resolvedTarget == nullptr)
        {
            if (mesh->GetAnimationName() != "idle")
                mesh->PlayAnimation("idle", true, 0.5f);

            state = NpcState::Idle;

            vec3 vel = controller.GetVelocity();
            controller.SetVelocity(vec3(0, vel.y, 0));
            return;
        }

        if (mesh->GetAnimationName() == "idle")
            mesh->PlayAnimation("run", true, 0.5f);

        if (state == NpcState::Idle)
            state = NpcState::Chasing;

        vec3 lookAtDir = MathHelper::FastNormalize(resolvedTarget->Position - Position);

        if (glm::distance(resolvedTarget->Position, Position) < 1.1f &&
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

void NpcZombie::NetSerialize(NetPacket& packet)
{
    NpcHumanBase::NetSerialize(packet);
    packet.WriteBool(attackDamageDealt);
}

void NpcZombie::NetDeserialize(NetPacket& packet)
{
    NpcHumanBase::NetDeserialize(packet);
    attackDamageDealt = packet.ReadBool();
}

// ---------------------------------------------------------------------------
// Save-game serialization
// ---------------------------------------------------------------------------

void NpcZombie::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);
    SERIALIZE_FIELD(target, attackDamageDealt);
}

void NpcZombie::Deserialize(json& source)
{
    NpcHumanBase::Deserialize(source);
    DESERIALIZE_FIELD(source, attackDamageDealt);
}

// ---------------------------------------------------------------------------
// Asset loading
// ---------------------------------------------------------------------------

void NpcZombie::LoadAssets()
{
    NpcHumanBase::LoadAssets();

    SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

    mesh->TexturesLocation = "GameData/models/enemies/zombie/zombie.glb/";
    mesh->LoadFromFile("GameData/models/enemies/zombie/zombie.glb");
    mesh->PreloadAssets();
    mesh->CreateHitboxes(this);
    mesh->PlayAnimation("run", true);
    mesh->SetLooped(true);
}

REGISTER_ENTITY(NpcZombie, "npc_zombie")
