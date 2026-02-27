// NpcHumanGun.cpp
#include "NpcHumanGun.h"
#include "../../Player/Player.hpp"
#include "../../Player/Weapons/Projectiles/Bullet.h"
#include <RandomHelper.h>

REGISTER_ENTITY(NpcHumanGun, "npc_human_gun")

NpcHumanGun::NpcHumanGun()
{
    ClassName = "npc_human_gun";
    maxSpeed = 5.3f;
    mesh->Scale = vec3(1.15f);
}

void NpcHumanGun::ProcessAnimationEvent(AnimationEvent& event)
{
    if (event.eventName == "stun_end")
    {
        stuned = false;
        mesh->PlayAnimation("run", true, 0.5f);
    }
}

void NpcHumanGun::Attack()
{
    if (cantAttackDelay.Wait()) return;
    if (inAttackDelay.Wait()) return;

    vec3 bonePos = mesh->GetBoneMatrixWorld("weapon_muzzle")[3];

    Entity* targetRef = Player::Instance;

    vec3 predictedTargetPosition = targetRef->Position;

    if (AttackDirectionCheck(bonePos, predictedTargetPosition, targetRef) == false)
    {
        inAttackDelay.AddDelay(0.5f);
    }

    const float bulletSpeed = 50;

    if (targetRef)
    {
        Player* playerRef = dynamic_cast<Player*>(targetRef);

        if (playerRef)
        {
            predictedTargetPosition += playerRef->controller.GetVelocity() * distance(Position, targetRef->Position) / bulletSpeed;
        }
        else
        {
            if (targetRef->LeadBody)
            {
                predictedTargetPosition += FromPhysics(targetRef->LeadBody->GetLinearVelocity()) * distance(Position, targetRef->Position) / bulletSpeed;
            }
        }
    }

    predictedTargetPosition += RandomHelper::RandomPosition(distance(targetRef->Position, Position) / 20.0f) * (accuracyModifier + 1.0f);

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

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");

    inAttackDelay.AddDelay(ModifyAnimationSpeed(0.5f));
    mesh->PlayAnimation("fire");
    mesh->PullRootMotion();
}

bool NpcHumanGun::LineOfSightCheck(Entity* targetEntity)
{
    auto hit = Physics::SphereTrace(Position + vec3(0, 0.4f, 0), targetEntity->Position + vec3(0, 0.3f, 0), 0.4,
        BodyType::World,
        {  }, { this, targetEntity });

    return hit.hasHit == false;
}

bool NpcHumanGun::AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity)
{
    auto hit = Physics::SphereTrace(start, target, 0.2,
        BodyType::GroupHitTest,
        {  }, { this, targetEntity });

    return hit.hasHit == false;
}


void NpcHumanGun::AsyncUpdate()
{
    if (dead)
    {
        mesh->UpdateHitboxes();

        UpdateStatusWidgets();

        return;
    }

    accuracyModifier -= Time::DeltaTimeF / 3.0f;
    accuracyModifier = glm::clamp(accuracyModifier, 0.0f, 5.0f);

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

    soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);

    soundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());

    if (dead || stuned || stunnedRagdoll || returningFromRagdoll) return;


    auto animName = mesh->GetAnimationName();

    if (animName == "run" || animName == "stun")
    {
        cantAttackDelay.AddDelay(0.7f);
    }

    Entity* target = Player::Instance;

    desiredTargetLocation = target->Position;

    vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

    bool hasLineOfSight = LineOfSightCheck(target);

    float attackDistance = 16;

    if (distance2(target->Position, Position) > attackDistance * attackDistance)
    {
        hasLineOfSight = false;
    }

    if (hasLineOfSight)
    {
        if (stopMovingDelay.Wait() == false)
        {

            if (animName == "run")
            {
                mesh->PlayAnimation("aim", true, 0.3f);

            }
            else
            {
                desiredDirection = lookAtDir;
            }

            if (cantAttackDelay.Wait() == false)
            {
                Attack();
            }
        }
        else
        {
            cantAttackDelay.AddDelay(0.5f);
        }
    }
    else
    {

        if (animName != "run" && stuned == false)
        {
            mesh->PlayAnimation("run", true, 0.6f);
        }

        cantAttackDelay.AddDelay(0.5f);

        stopMovingDelay.AddDelay(RandomHelper::RandomFloat() * -0.5f + 0.5f);

    }



    if (target)
    {
        if (fleeing)
        {
            UpdateFleeTarget();
        }
        else
        {
            pathFollow.WaitToFinish();
            pathFollow.UpdateStartAndTarget(Position, desiredTargetLocation);
            pathFollow.TryPerform();

        }

    }
    if (pathFollow.FoundTarget && animName == "run")
    {
        desiredDirection = normalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
    }


    speed += Time::DeltaTimeF * 6.5;

    speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

    if ((hasLineOfSight || inAttackDelay.Wait()) && stopMovingDelay.Wait() == false)
    {
        speed = 0;
    }

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

void NpcHumanGun::Serialize(json& target)
{
    NpcHumanBase::Serialize(target);

    SERIALIZE_FIELD(target, cantAttackDelay);
    SERIALIZE_FIELD(target, desiredTargetLocation);
    SERIALIZE_FIELD(target, accuracyModifier);
}

void NpcHumanGun::Deserialize(json& source)
{
    NpcHumanBase::Deserialize(source);

    DESERIALIZE_FIELD(source, cantAttackDelay);
    DESERIALIZE_FIELD(source, desiredTargetLocation);
    DESERIALIZE_FIELD(source, accuracyModifier);
}

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
