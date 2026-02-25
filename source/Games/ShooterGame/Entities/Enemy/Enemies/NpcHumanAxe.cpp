// NpcHumanAxe.cpp
#include "NpcHumanAxe.h"
#include "../../Player/Player.hpp"

REGISTER_ENTITY(NpcHumanAxe, "npc_human_axe")

NpcHumanAxe::NpcHumanAxe()
{
    ClassName = "npc_human_axe";
    maxSpeed = 5.7f;
}

void NpcHumanAxe::ProcessAnimationEvent(AnimationEvent& event)
{
    Logger::Log(event.eventName);

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

    auto hit = Physics::SphereTrace(Position, MathHelper::GetForwardVector(mesh->Rotation) * 1.2f + Position, 0.45f, BodyType::World | BodyType::CharacterCapsule, { LeadBody });

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

    if (LeadBody != nullptr)
    {
        Physics::MoveBody(LeadBody, rootMotion.Position);

        if (rootMotion.Position != vec3())
        {
            Physics::SetLinearVelocity(LeadBody, vec3(0, LeadBody->GetLinearVelocity().GetY(), 0));
        }
    }
    else
    {
        Position += rootMotion.Position;
    }

    if (rootMotion.Rotation != vec3())
    {
        mesh->Rotation += rootMotion.Rotation;
        movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
    }

    UpdateStunnedReturn();
    UpdateReturnFromRagdoll();

    mesh->UpdateHitboxes();

    if (dead || stuned || stunnedRagdoll || returningFromRagdoll) return;

    UpdateAttackDamage();

    soundPlayer->Position = Position;

    soundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());

    Entity* target = Player::Instance;

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

    if (target)
    {
        if (fleeing)
        {
            UpdateFleeTarget();
        }
        else
        {

            pathFollow.UpdateStartAndTarget(Position, target->Position);
            pathFollow.TryPerform();

        }

    }
    if (pathFollow.FoundTarget)
    {
        desiredDirection = normalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
    }

    speed += Time::DeltaTimeF * 6.5;

    speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

    movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime * 10);

    movingDirection = MathHelper::FastNormalize(movingDirection);

    // Get the current horizontal velocity (preserving the vertical component from physics)
    vec3 currentVelocity = FromPhysics(LeadBody->GetLinearVelocity());
    vec3 currentHorizontalVel(currentVelocity.x, 0.0f, currentVelocity.z);

    // Determine the desired horizontal velocity (5.0f is the intended speed)
    vec3 desiredHorizontalVel = movingDirection * speed;

    // Calculate the change in velocity you need to achieve over the current frame
    // Using Time::DeltaTime (dt) to convert velocity difference to the required acceleration
    float dt = Time::DeltaTime;
    vec3 neededAcceleration = (desiredHorizontalVel - currentHorizontalVel) / dt;

    // Retrieve the body mass to calculate the needed force (F = m * a)
    float mass = 40;
    vec3 forceToApply = neededAcceleration * mass;

    // Only apply horizontal forces to avoid interfering with the vertical (gravity, jump, etc.)
    vec3 horizontalForce(forceToApply.x, 0.0f, forceToApply.z);


    // Apply the calculated force to the body
    LeadBody->AddForce(ToPhysics(horizontalForce));

    Physics::Activate(LeadBody);

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
