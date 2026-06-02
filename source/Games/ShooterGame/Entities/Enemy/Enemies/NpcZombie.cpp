#include "NpcHumanBase.h"

class NpcZombie : public NpcHumanBase
{
private:
    bool attacking = false;
    bool attackDamageDealt = false;

    static constexpr float AttackDamageDelay = 0.7f;  // seconds into animation to deal damage
    static constexpr float AttackEndThreshold = 0.15f; // seconds before animation end to transition out

    void UpdateAttackDamage();
    void UpdateAttackState();

protected:
    void Attack() override;
    void LoadAssets() override;

public:
    NpcZombie();

    void Stun() override;

    void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr) override;

    void AsyncUpdate() override;

    void Serialize(json& target) override;
    void Deserialize(json& source) override;
};


NpcZombie::NpcZombie()
{
    ClassName = "npc_zombie";
    maxSpeed = 1.5f;

	canBeStunRagdolled = false;

    Health = 100;
    MaxHealth = 100;
}

void NpcZombie::Stun()
{
    NpcHumanBase::Stun();
    attacking = false;
    attackDamageDealt = false;
}

void NpcZombie::Attack()
{
    if (inAttackDelay.Wait()) return;

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");

    inAttackDelay.AddDelay(1.5f);
    mesh->PlayAnimation("attack",false,0.3);
    mesh->PullRootMotion(); // discard any initial root motion snapshot
    attacking = true;
    attackDamageDealt = false;
}

void NpcZombie::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    NpcHumanBase::OnDamage(Damage, DamageCauser, Weapon);
}

// Deals damage exactly once when the animation clock passes AttackDamageDelay
void NpcZombie::UpdateAttackDamage()
{
    if (attackDamageDealt) return;
    if (mesh->GetAnimationTime() < AttackDamageDelay) return;

    auto hit = Physics::SphereTrace(
        Position,
        MathHelper::GetForwardVector(mesh->Rotation) * 1.2f + Position,
        0.45f,
        BodyType::World | BodyType::CharacterCapsule,
        { controller.body });

    // No contact yet — keep checking each frame so parry timing can land
    if (!hit.hasHit || !hit.entity->HasTag("player")) return;

    // Commit only once we've actually resolved contact
    attackDamageDealt = true;

    if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
    {
        OnPointDamage(20, hit.shapePosition,
            MathHelper::FastNormalize(Position - hit.shapePosition), "", this, this);
        Physics::AddImpulse(mesh->FindHitboxByName("spine_02"),
            MathHelper::GetForwardVector(mesh->Rotation) * -500.0f);
        AddDebuffStacks("PoiseBreakDebuff", 100);
    }
    else
    {
        hit.entity->OnPointDamage(15, hit.shapePosition,
            MathHelper::FastNormalize(hit.shapePosition - Position), "", this, this);
    }
}

// Watches animation time to transition out of attack without events
void NpcZombie::UpdateAttackState()
{
    float dur = mesh->GetAnimationDuration();
    if (dur > 0.0f && mesh->GetAnimationTime() >= dur - AttackEndThreshold)
    {
        attacking = false;
        attackDamageDealt = false;
        mesh->PlayAnimation("run", true, 0.5f);
    }
}

void NpcZombie::AsyncUpdate()
{
    if (dead)
    {
        mesh->UpdateHitboxes();
        UpdateStatusWidgets();
        return;
    }

    controller.Update(Time::DeltaTimeF);
    Position = controller.GetPosition();

    UpdatePerception();
    UpdateStatusWidgets();
    UpdateDebuffs(Time::DeltaTimeF);

    if (dead) return;

    mesh->Update(ModifyAnimationSpeed(1.0f));

    mesh->Position = Position - vec3(0, 1, 0);

    // Root motion applied only during stun animations
    auto rootMotion = mesh->PullRootMotion();
    if (stuned || stunnedRagdoll)
    {
        Position += rootMotion.Position;
        controller.SetPosition(Position);
        if (rootMotion.Position != vec3())
            controller.SetVelocity(vec3(0, controller.GetVelocity().y, 0));

        if (rootMotion.Rotation != vec3())
        {
            mesh->Rotation += rootMotion.Rotation;
            movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
        }
    }

    UpdateStunnedReturn();
    UpdateReturnFromRagdoll();

    // Time-based stun end (replaces stun_end animation event)
    if (stuned && !stunnedRagdoll && !returningFromRagdoll)
    {
        float dur = mesh->GetAnimationDuration();
        if (dur > 0.0f && mesh->GetAnimationTime() >= dur - AttackEndThreshold)
        {
            stuned = false;
            mesh->PlayAnimation("run", true, 0.5f);
        }
    }

    mesh->UpdateHitboxes();

    soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);
    soundPlayer->Velocity = controller.GetVelocity();

    if (dead || stuned || stunnedRagdoll || returningFromRagdoll) return;

    // Attack state: deal damage at fixed time, stand still, then transition out
    if (attacking)
    {
        UpdateAttackDamage();
        if (dead)return;
        if (attacking) // parry may have cleared it via Stun()
            UpdateAttackState();

        if (attacking) // still attacking after state update — hold in place
        {
            vec3 vel = controller.GetVelocity();
            controller.SetVelocity(vec3(0, vel.y, 0));
            return;
        }
    }

    if (dead) return;

    // No target: idle
    if (target == nullptr)
    {
        if (mesh->GetAnimationName() != "idle")
            mesh->PlayAnimation("idle", true, 0.5f);

        vec3 vel = controller.GetVelocity();
        controller.SetVelocity(vec3(0, vel.y, 0));
        return;
    }
    else if (mesh->GetAnimationName() == "idle")
    {
        mesh->PlayAnimation("run", true, 0.5f);
    }

    vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

    if (distance(target->Position, Position) < 1.1f
        && dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.93f)
    {
        Attack();
    }

    if (fleeing)
    {
        UpdateFleeTarget();
    }
    else
    {
        pathFollow.UpdateStartAndTarget(Position, target->Position);
        pathFollow.TryPerform();
    }

    if (pathFollow.FoundTarget)
        desiredDirection = normalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));

    speed += Time::DeltaTimeF * 6.5f;
    speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

    movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime * 10.0);
    movingDirection = MathHelper::FastNormalize(movingDirection);

    vec3 vel = controller.GetVelocity();
    controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));

    mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
}

void NpcZombie::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);
    SERIALIZE_FIELD(target, attacking);
    SERIALIZE_FIELD(target, attackDamageDealt);
}

void NpcZombie::Deserialize(json& source)
{
    NpcHumanBase::Deserialize(source);
    DESERIALIZE_FIELD(source, attacking);
    DESERIALIZE_FIELD(source, attackDamageDealt);
}

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