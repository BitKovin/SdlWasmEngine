#include "NpcHumanBase.h"
#include <RandomHelper.h>

#include <World/WorldOrientationManager.h>

#include <Systems/ScoreSystem/ScoreSystem.h>

#include <Entities/Player/Player.hpp>

#include <AiPerception/AiPerceptionSystem.h>

NpcHumanBase::NpcHumanBase()
{
    mesh = new SkeletalMesh(this);
    mesh->GravityAlignedRotation = true;
    Drawables.push_back(mesh);

    statusWidget = new UiBilboard(this);
	statusWidget->GravityAlignedRotation = true;
    Drawables.push_back(statusWidget);

    auto debuffs = make_shared<UiNpcStatus>(this);
    statusWidget->ViewportSize = ivec2(1024, 256);
    statusWidget->PixelPerMeter = 1024.0f;
    statusWidget->ContentBox->AddChild(debuffs);

    SaveGame = true;

    Health = 130;
    MaxHealth = 130;

    mesh->UpdatePoseOnlyWhenRendered = true;

    Tags.push_back("enemy");

    Poise = 100.f;

    getFromRagdollAnimation = new Animation(this);
}

NpcHumanBase::~NpcHumanBase()
{
    if (getFromRagdollAnimation) {
        delete getFromRagdollAnimation;
    }
}

void NpcHumanBase::UpdateFleeTarget()
{
    if (target == nullptr) return;

    if (fleeSearchDelay.Wait() == false)
    {
        auto path = NavigationSystem::FindFleePath(Position, target->Position);

        if (path.empty() == false)
        {
            pathFollow.UpdateStartAndTarget(Position, path[path.size() - 1]);
            pathFollow.TryPerform();
        }

        fleeSearchDelay.AddDelay(0.2f);
    }
}

void NpcHumanBase::UpdatePerception()
{

    if (observer == nullptr) return;

    observer->forward = MathHelper::GetForwardVector(mesh->Rotation);
    observer->position = Position + vec3(0,0.7f,0);

    for (auto& t : observer->visibleTargets)
    {
        if (t->HasTag("player"))
        {

            if (target)
            {
                if (t->ownerId == target->Id) continue;
            }


            SetTarget(Level::Current->FindEntityWithId(t->ownerId));

        }
    }

    for (auto& s : observer->heardSounds)
    {
        auto causer = Level::Current->FindEntityWithId(s.causerId);

        if (causer == nullptr) continue;

        if (causer->HasTag("player"))
        {
            SetTarget(causer);
        }

    }

}

void NpcHumanBase::SetupSoundPlayer(SoundPlayer* soundPlayer)
{
    soundPlayer->MaxDistance = 100;
    soundPlayer->Volume = 1.0;
    soundPlayer->MinDistance = 0.5;
}

void NpcHumanBase::PlaySoundEffect(std::string eventName)
{
    if (soundPlayer == nullptr) return;

    soundPlayer->SetSound(FmodEventInstance::Create(eventName));

    soundPlayer->Play();
}

void NpcHumanBase::Start()
{
    mesh->Position = Position - vec3(0, 1, 0);
    mesh->Rotation = Rotation;

    LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 50);

    Physics::SetGravityFactor(LeadBody, 4);

    desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
    movingDirection = desiredDirection;

    pathFollow.CalculatePathOnThread();

    soundPlayer = SoundPlayer::Create();

    SetupSoundPlayer(soundPlayer);

    observer = AiPerceptionSystem::CreateObserver(Position, MathHelper::GetForwardVector(mesh->Rotation), 150);

}

void NpcHumanBase::Stun()
{
    stuned = true;
    mesh->PlayAnimation("stun");
    mesh->PullRootMotion();

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1Stun");
}

void NpcHumanBase::StartStunnedRagdoll()
{
    if (dead) return;

    if (stunnedRagdoll == false)
    {
        mesh->RagdollPoseFollowStrength = 0.5f;
        mesh->StartRagdoll();
        stunnedRagdoll = true;
    }

    stunnedRagdollDelay.AddDelay(2);
}

void NpcHumanBase::UpdateStunnedReturn()
{
    if (stunnedRagdoll == false) return;

    Body* pelvisBody = mesh->FindHitboxByName("pelvis");

    vec3 pelvisPos = FromPhysics(pelvisBody->GetPosition());

    vec3 vel = FromPhysics(LeadBody->GetLinearVelocity());
    Physics::SetLinearVelocity(LeadBody, vec3(0, vel.y, 0));

    Position = pelvisPos + vec3(0, 1.5f, 0);
    Physics::SetBodyPosition(LeadBody, Position);
    Physics::SetLinearVelocity(LeadBody, vec3());

    if (stunnedRagdollDelay.Wait()) return;

    bool hitsGround = Physics::LineTrace(pelvisPos, pelvisPos - vec3(0, 0.5f, 0), BodyType::World).hasHit;

    if (hitsGround || pelvisBody->GetLinearVelocity().Length() < 0.1f)
    {
        StartReturnFromRagdoll();
        stunnedRagdoll = false;
    }
    else
    {
        stunnedRagdollDelay.AddDelay(0.3f + RandomHelper::RandomFloat() * 0.2f);
    }
}

void NpcHumanBase::StartReturnFromRagdoll()
{
    if (mesh->InRagdoll == false) return;

    mesh->UpdateHitboxes();

    vec3 pelvisPos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Position;
    vec3 pelvisRot = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Rotation;
    vec3 spinePos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("spine_03")).Position;

    ragdollPose = mesh->GetAnimationPose();

    bool onFront = MathHelper::GetUpVector(MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Rotation).y > 0;

    getFromRagdollAnimation->PlayAnimation(onFront ? "front" : "back", false, 0);

    auto pelvisTransformWorld = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis"));

    Position = pelvisTransformWorld.Position + vec3(0, 1.0f, 0);
    Physics::SetBodyPosition(LeadBody, Position);

    float oldRot = mesh->Rotation.y;

    if (onFront)
    {
        mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(pelvisPos, spinePos).y, 0);
    }
    else
    {
        mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(spinePos, pelvisPos).y, 0);
    }

    movingDirection = MathHelper::GetForwardVector(mesh->Rotation);

    auto pelvisTransform = MathHelper::DecomposeMatrix(ragdollPose.boneTransforms["pelvis"]);

    pelvisTransform.Position = vec3(0, 3.0f, 0);
    pelvisTransform.RotationQuaternion = MathHelper::GetRotationQuaternion(vec3(0, oldRot - mesh->Rotation.y, 0)) * pelvisTransform.RotationQuaternion;

    ragdollPose.boneTransforms["pelvis"] = pelvisTransform.ToMatrix();

    ragdollPelvisWorldPos = pelvisPos;
    pelvisBlendTimer = 0.0f;

    mesh->StopRagdoll();
    mesh->PasteAnimationPose(ragdollPose);

    returningFromRagdoll = true;

    movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
    desiredDirection = movingDirection;

    mesh->StopAnimation();
}

void NpcHumanBase::UpdateReturnFromRagdoll()
{
    if (stunnedRagdollDelay.Wait()) return;

    if (returningFromRagdoll == false) return;

    vec3 vel = FromPhysics(LeadBody->GetLinearVelocity());
    Physics::SetLinearVelocity(LeadBody, vec3(0, vel.y, 0));

    getFromRagdollAnimation->Update(ModifyAnimationSpeed(1.0f));

    auto meshPose = mesh->GetAnimationPose();

    float blendInTime = 0.5;
    float blendOutTime = 0.7f;

    float lerpProgressFromStart = 1.0f - ((blendInTime - getFromRagdollAnimation->GetAnimationTime()) / blendInTime);

    lerpProgressFromStart = saturate(lerpProgressFromStart);

    auto animationPose = getFromRagdollAnimation->GetAnimationPose();

    auto newPose = AnimationPose::Lerp(animationPose, ragdollPose, 1.0 - lerpProgressFromStart);

    float lerpProgressFromEnd = ((blendOutTime - (getFromRagdollAnimation->GetAnimationDuration() - getFromRagdollAnimation->GetAnimationTime())) / blendOutTime);

    newPose = AnimationPose::Lerp(meshPose, newPose, 1.0f - lerpProgressFromEnd);

    mesh->PasteAnimationPose(newPose);

    if (pelvisBlendTimer < 0.5f)
    {
        pelvisBlendTimer += Time::DeltaTimeF;
        float t = saturate(pelvisBlendTimer / 0.5f);

        vec3 animPelvisWorldPos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Position;

        vec3 targetPelvisWorldPos = mix(ragdollPelvisWorldPos, animPelvisWorldPos, t);

        mesh->Position += targetPelvisWorldPos - animPelvisWorldPos;
    }

    if (getFromRagdollAnimation->IsAnimationPlaying() == false)
    {
        if (returningFromRagdoll)
        {
            returningFromRagdoll = false;
            stunnedRagdoll = false;
            mesh->PlayAnimation("run", true, 0.5f);
            speed = maxSpeed;
        }
        return;
    }
}

void NpcHumanBase::Death()
{
    if (dead) return;

    mesh->StartRagdoll();
    mesh->SetAnimationPaused(true);
    Physics::SetLinearVelocity(LeadBody, vec3(0));

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1Death");

    GetDebuffsList().clear();
    UpdateStatusWidgets();

    Physics::DestroyBody(LeadBody);
    LeadBody = nullptr;

    CallActionOnEntityWithId(OwnerId, "despawned");

    dead = true;

    ScoreSystem::Instance().addScore(MaxHealth * 0.5f);

    if (soundPlayer)
    {
        soundPlayer->DestroyWithDelay(3);
        soundPlayer = nullptr;
    }
}

void NpcHumanBase::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
    Damage *= mesh->GetHitboxDamageMultiplier(bone);
    Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

    if (bone == "calf_l" || bone == "calf_r" || bone == "thigh_l" || bone == "thigh_r")
    {
        StartStunnedRagdoll();
    }

    if (mesh->GetHitboxDamageMultiplier(bone) > 1.1f && dead)
    {
        //Time::AddTimeScaleEffect(0.3f, 0.15f, true, "hit_slow");
    }

    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage / 10.0f));

    SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1, Damage / 20.0f, false, Point);
}

void NpcHumanBase::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    Damage = ModifyIncomingDamage(Damage);

    ScoreSystem::Instance().addScore(std::min(Damage, Health));

    Health -= Damage;

    if (Health <= 0)
    {
        Death();
    }

    if (DamageCauser != nullptr)
    {
        if (LeadBody)
        {
            LeadBody->SetLinearVelocity(LeadBody->GetLinearVelocity() / 2.0f);
            speed /= 2.0f;
            PlaySoundEffect("event:/NPC/Enemy1/Enemy1Damage");
        }
    }

    if (Health < 30)
    {
        //fleeing = true;
        //UpdateFleeTarget();
    }
}

void NpcHumanBase::Serialize(json& target)
{
    Entity::Serialize(target);

    animationStateSaveData = mesh->GetAnimationState();

    Rotation = mesh->Rotation;

    getFromRagdollAnimationSaveState = getFromRagdollAnimation->GetAnimationState();

    SERIALIZE_FIELD(target, Rotation);
    SERIALIZE_FIELD(target, desiredDirection);
    SERIALIZE_FIELD(target, movingDirection);
    SERIALIZE_FIELD(target, speed);
    SERIALIZE_FIELD(target, dead);
    SERIALIZE_FIELD(target, animationStateSaveData);
    SERIALIZE_FIELD(target, stuned);
    SERIALIZE_FIELD(target, fleeing);

    SERIALIZE_FIELD(target, inAttackDelay);

    auto debuffsJson = SerializeDebuffs();
    SERIALIZE_FIELD(target, debuffsJson);

    SERIALIZE_FIELD(target, stunnedRagdoll);
    SERIALIZE_FIELD(target, stunnedRagdollDelay);
    SERIALIZE_FIELD(target, returningFromRagdoll);
    SERIALIZE_FIELD(target, ragdollPose);
    SERIALIZE_FIELD(target, getFromRagdollAnimationSaveState);

    SERIALIZE_FIELD(target, ragdollPelvisWorldPos);
    SERIALIZE_FIELD(target, pelvisBlendTimer);

    std::string targetId = "";
    if (this->target)
        targetId = this->target->Id;
    SERIALIZE_FIELD(target, targetId);
}

void NpcHumanBase::Deserialize(json& source)
{
    Entity::Deserialize(source);

    DESERIALIZE_FIELD(source, Rotation);
    DESERIALIZE_FIELD(source, desiredDirection);
    DESERIALIZE_FIELD(source, movingDirection);
    DESERIALIZE_FIELD(source, speed);
    DESERIALIZE_FIELD(source, dead);
    DESERIALIZE_FIELD(source, animationStateSaveData);
    DESERIALIZE_FIELD(source, stuned);
    DESERIALIZE_FIELD(source, fleeing);

    DESERIALIZE_FIELD(source, inAttackDelay);

    json debuffsJson;
    DESERIALIZE_FIELD(source, debuffsJson);
    DeserializeDebuffs(debuffsJson);

    DESERIALIZE_FIELD(source, stunnedRagdoll);
    DESERIALIZE_FIELD(source, stunnedRagdollDelay);
    DESERIALIZE_FIELD(source, returningFromRagdoll);
    DESERIALIZE_FIELD(source, ragdollPose);
    DESERIALIZE_FIELD(source, getFromRagdollAnimationSaveState);

    Physics::SetBodyPosition(LeadBody, Position);

    if (dead)
    {
        Physics::DestroyBody(LeadBody);
        LeadBody = nullptr;

        soundPlayer->Destroy();

        soundPlayer = nullptr;
    }

    mesh->Rotation = Rotation;

    mesh->SetAnimationState(animationStateSaveData);
    mesh->Update(0);
    mesh->PullRootMotion();

    if (returningFromRagdoll) {
        getFromRagdollAnimation->SetAnimationState(getFromRagdollAnimationSaveState);
    }

    DESERIALIZE_FIELD(source, ragdollPelvisWorldPos);
    DESERIALIZE_FIELD(source, pelvisBlendTimer);

    std::string targetId = "";
    DESERIALIZE_FIELD(source, targetId);

    this->target = Level::Current->FindEntityWithId(targetId);
}

void NpcHumanBase::UpdateStatusWidgets()
{
    statusWidget->Position = mesh->Position + WorldOrientationManager::GetUpVector() * 2.0f;

    statusWidget->TwoSided = true;
    statusWidget->Update();

    statusWidget->Visible = !dead;
}

void NpcHumanBase::UpdateDebugUI()
{
    ImGui::Begin(("NpcHuman Debug: " + Id).c_str());

    ImGui::Text(("Health: " + std::to_string(Health)).c_str());

    ImGui::Text(GetDebuffsDebugInfo().c_str());

    ImGui::End();
}

void NpcHumanBase::LoadAssets()
{
    Entity::LoadAssets();

    getFromRagdollAnimation->LoadFromFile("GameData/animations/npc/standUp.glb");
}

void NpcHumanBase::Destroy()
{
    Entity::Destroy();

    mesh->ClearHitboxes();
}

void NpcHumanBase::FromData(EntityData data)
{
    Entity::FromData(data);
}

void NpcHumanBase::SetTarget(Entity* newTarget)
{
    target = newTarget;
}

void NpcHumanBase::OnAction(std::string action)
{

    if (action == "triggerOnPlayer")
    {
        SetTarget(Player::Instance);
    }

}
