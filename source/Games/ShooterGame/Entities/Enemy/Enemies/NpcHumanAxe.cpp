// NpcHumanAxe.cpp
#include "NpcHumanAxe.h"

REGISTER_ENTITY(NpcHumanAxe, "npc_human_axe")

NpcHumanAxe::NpcHumanAxe()
{
    ClassName = "npc_human_axe";
    maxSpeed = 5.0f;

	pathFollow.allowPartialPath = true;

    Health = 80;
    MaxHealth = 80;
}

void NpcHumanAxe::ProcessAnimationEvent(AnimationEvent& event)
{

    if (event.eventName == "attack_start")
    {
        attackingDamage = true;
    }

    if (event.eventName == "attack_end")
    {
        mesh->PlayAnimation("run", true, 0.5f);
        attacking = false;
        attackingDamage = false;
    }
    if (event.eventName == "stun_end")
    {
        stuned = false;
        mesh->PlayAnimation("run", true, 0.5f);
    }
}

void NpcHumanAxe::Stun()
{
    NpcHumanBase::Stun();
    attacking = false;
    attackingDamage = false;
}

void NpcHumanAxe::Attack()
{
    if (inAttackDelay.Wait()) return;

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");

    inAttackDelay.AddDelay(1.5f);
    mesh->PlayAnimation("attack");
    mesh->PullRootMotion();
    attacking = true;
}

void NpcHumanAxe::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    NpcHumanBase::OnDamage(Damage, DamageCauser, Weapon);

    if (attacking)
    {
        //Stun(DamageCauser, Weapon);
    }
}

void NpcHumanAxe::UpdateAttackDamage()
{
    if (attackingDamage == false) return;

    auto hit = Physics::SphereTrace(Position, MathHelper::GetForwardVector(mesh->Rotation) * 1.2f + Position, 0.45f, BodyType::World | BodyType::CharacterCapsule, {}, {this});

    if (hit.hasHit)
    {
        if (hit.entity->HasTag("player"))
        {
            if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
            {
                OnPointDamage(20, hit.shapePosition, MathHelper::FastNormalize(Position - hit.shapePosition), "", this, this);
                Physics::AddImpulse(mesh->FindHitboxByName("spine_02"), MathHelper::GetForwardVector(mesh->Rotation) * -500.0f);
                AddDebuffStacks("PoiseBreakDebuff", 100);
                attackingDamage = false;
                return;
            }
            else
            {
                hit.entity->OnPointDamage(15, hit.shapePosition, MathHelper::FastNormalize(hit.shapePosition - Position), "", this, this);
                attackingDamage = false;
            }
        }
    }
}

void NpcHumanAxe::AsyncUpdate()
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

    auto animEvents = mesh->PullAnimationEvents();

    for (auto& event : animEvents)
    {
        ProcessAnimationEvent(event);
    }

    mesh->Position = Position - vec3(0, 1, 0);

    auto rootMotion = mesh->PullRootMotion();

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

    soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);

    soundPlayer->Velocity = controller.GetVelocity();

    if (dead || stuned || stunnedRagdoll || returningFromRagdoll) return;

    UpdateAttackDamage();
    if (dead) return;

    // No target: idle in place
    if (target == nullptr)
    {
        if (mesh->GetAnimationName() != "idle")
            mesh->PlayAnimation("idle", true, 0.5f);

        vec3 vel = controller.GetVelocity();
        controller.SetVelocity(vec3(0, vel.y, 0));
        return;
    }
    else if(mesh->GetAnimationName() == "idle")
    {      
        mesh->PlayAnimation("run", true, 0.5f);
    }

    if (attacking)
    {
        speed = 2;
        return;
    }

    vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

    if (distance(target->Position, Position) < 1.5f
        && dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.93)
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
    {
        desiredDirection = normalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
    }

    speed += Time::DeltaTimeF * 6.5;

    speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

    movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime * 10);

    movingDirection = MathHelper::FastNormalize(movingDirection);

    vec3 vel = controller.GetVelocity();
    controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));

    mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
}

void NpcHumanAxe::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);

    SERIALIZE_FIELD(target, attacking);
    SERIALIZE_FIELD(target, attackingDamage);
}

void NpcHumanAxe::Deserialize(json& source)
{
    NpcHumanBase::Deserialize(source);

    DESERIALIZE_FIELD(source, attacking);
    DESERIALIZE_FIELD(source, attackingDamage);
}

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
