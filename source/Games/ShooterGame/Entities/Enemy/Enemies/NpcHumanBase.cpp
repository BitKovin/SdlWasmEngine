// NpcHumanBase.cpp
#include "NpcHumanBase.h"
#include <RandomHelper.h>
#include <World/WorldOrientationManager.h>
#include <Systems/ScoreSystem/ScoreSystem.h>
#include <Entities/Player/Player.hpp>
#include <AiPerception/AiPerceptionSystem.h>
#include <Network/NetworkManager.h>

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

NpcHumanBase::NpcHumanBase()
{
    mesh = new SkeletalMesh(this);
    mesh->CastDetailShadows = true;
    mesh->GravityAlignedRotation = true;
    mesh->MeshCustomShaderParams["rim_color"] = vec4(1.5f, 1.2f, 0.0f, 1.0f);
    mesh->MeshCustomShaderParams["rim_pow"]   = vec4(1.3f, 0.0f, 0.0f, 0.0f);
    Drawables.push_back(mesh);

    statusWidget = new UiBilboard(this);
    statusWidget->GravityAlignedRotation = true;
    Drawables.push_back(statusWidget);

    auto debuffs = make_shared<UiNpcStatus>(this);
    statusWidget->ViewportSize  = ivec2(1024, 256);
    statusWidget->PixelPerMeter = 1024.0f;
    statusWidget->ContentBox->AddChild(debuffs);

    SaveGame = true;
    Health    = 130;
    MaxHealth = 130;
    mesh->UpdatePoseOnlyWhenRendered = true;
    Tags.push_back("enemy");
    Poise = 100.f;

    getFromRagdollAnimation = new Animation(this);

    CanMigrateOwner          = true;
    DestroyOnOwnerDisconnect = false;
}

NpcHumanBase::~NpcHumanBase()
{
    if (getFromRagdollAnimation)
        delete getFromRagdollAnimation;
}

// ---------------------------------------------------------------------------
// Start
// ---------------------------------------------------------------------------

void NpcHumanBase::Start()
{
    mesh->Position = Position - vec3(0, 1, 0);
    mesh->Rotation = Rotation;

    controller.Init(this, Position, 0.5f, 2.0f, 5);
    controller.movementQuality = CharacterControllerMovementQuality::NpcGeneric;

    desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
    if (glm::length(desiredDirection) < 0.001f)
        desiredDirection = vec3(0, 0, 1);
    movingDirection = desiredDirection;

    soundPlayer = SoundPlayer::Create();
    SetupSoundPlayer(soundPlayer);

    if (isOwned)
    {
        observer = AiPerceptionSystem::CreateObserver(
            Position, MathHelper::GetForwardVector(mesh->Rotation), 150);
    }
}

// ---------------------------------------------------------------------------
// Targeting
// ---------------------------------------------------------------------------

// ResolveTarget – called every frame on all peers.
// Uses EntityHandle::Resolve(true) so that NetworkedEntities with localProxy
// (e.g. RemotePlayer) return the proxied local entity (e.g. Player::Instance).
void NpcHumanBase::ResolveTarget()
{
    resolvedTarget = currentTarget.Resolve();
}

// UpdatePerception – owner only.
// Scans visible + heard stimuli tagged "player" and calls TrySetTarget
// with the closest candidate.
//
// Key detail: perception targets are registered by entities in the level.
// RemotePlayer registers a target; on the owning peer RemotePlayer also
// sets localProxy = Player::Instance, so EntityHandle::Resolve() returns
// the actual Player.  This means a client-owned NPC correctly perceives
// both the local Player (via its RemotePlayer's localProxy) and sees remote
// players directly as NetworkedEntities.
void NpcHumanBase::UpdatePerception()
{
    if (currentTarget.IsValid())
        DebugDraw::Line(Position, currentTarget.Resolve()->Position);

    if (!isOwned) return;

    // Create observer lazily so it works after an ownership transfer.
    if (observer == nullptr)
    {
        observer = AiPerceptionSystem::CreateObserver(
            Position, MathHelper::GetForwardVector(mesh->Rotation), 150);
    }

    observer->forward  = MathHelper::GetForwardVector(mesh->Rotation);
    observer->position = Position + vec3(0, 0.7f, 0);

    if (targetSwitchTimer > 0.0f)
        targetSwitchTimer = std::max(0.0f, targetSwitchTimer - Time::DeltaTimeF);

    // If the current target no longer resolves (player disconnected / entity
    // destroyed), clear it and return the NPC to server ownership.
    if (currentTarget.IsValid() && resolvedTarget == nullptr)
    {
        currentTarget.Clear();
        targetSwitchTimer = 0.0f;
        NetPacket args(PacketType::RPC);
        EntityHandle{}.Write(args);
        SendRPC(static_cast<uint8_t>(NpcRPC::SetTarget), args, RPCTarget::Others);
        SetOwner(0);
        return;
    }

    // ── Build deduplicated candidate list ─────────────────────────────────
    // Key: EntityHandle (by networkId or entityId).
    // Sight position beats heard position for the same handle.
    struct Candidate { EntityHandle handle; vec3 worldPos; bool fromSight; };
    std::vector<Candidate> candidates;

    auto findOrAdd = [&](const EntityHandle& h, const vec3& pos, bool fromSight)
    {
        for (auto& c : candidates)
        {
            if (c.handle == h)
            {
                if (fromSight && !c.fromSight) { c.worldPos = pos; c.fromSight = true; }
                return;
            }
        }
        candidates.push_back({ h, pos, fromSight });
    };

    // Visible targets
    for (auto& pt : observer->visibleTargets)
    {
        if (!pt->HasTag("player")) continue;

        // Find the entity that registered this perception target.
        // pt->ownerId is a string entity Id.
        Entity* e = Level::Current->FindEntityWithId(pt->ownerId);

        if (!e) continue;

        auto* ne = dynamic_cast<NetworkedEntity*>(e);
        if (ne)
            findOrAdd(EntityHandle::FromNetworked(ne->networkId), e->Position, true);
        else
            findOrAdd(EntityHandle::FromEntity(e), e->Position, true);
    }

    // Heard sounds – wake the NPC even while Idle.
    if (!IsDead() && !IsStunned())
    {
        for (auto& s : observer->heardSounds)
        {
            // Try to find the causer entity.
            Entity* causer = Level::Current->FindEntityWithId(s.causerId);

            // Also check Player::Instance which may not be in LevelObjects.
            if (!causer && Player::Instance && Player::Instance->Id == s.causerId)
                causer = Player::Instance;

            if (!causer || !causer->HasTag("player")) continue;

            auto* ne = dynamic_cast<NetworkedEntity*>(causer);
            if (ne)
                findOrAdd(EntityHandle::FromNetworked(ne->networkId), s.position, false);
            else
                findOrAdd(EntityHandle::FromEntity(causer), s.position, false);
        }
    }

    if (candidates.empty()) return;

    // Pick closest candidate.
    float      bestDist = std::numeric_limits<float>::max();
    Candidate* best     = nullptr;
    for (auto& c : candidates)
    {
        float d = glm::distance(Position, c.worldPos);
        if (d < bestDist) { bestDist = d; best = &c; }
    }

    if (best) TrySetTarget(best->handle, best->worldPos);
}

void NpcHumanBase::TrySetTarget(const EntityHandle& candidate, const vec3& candidatePos)
{
    if (candidate == currentTarget) return;

    // Only switch in interruptible states.
    if (state != NpcState::Idle    &&
        state != NpcState::Chasing &&
        state != NpcState::Fleeing)
        return;

    // No current target – accept immediately.
    if (!currentTarget.IsValid())
    {
        ApplyTarget(candidate);
        return;
    }

    if (targetSwitchTimer > 0.0f) return;

    float currentDist   = resolvedTarget
                        ? glm::distance(Position, resolvedTarget->Position)
                        : std::numeric_limits<float>::max();
    float candidateDist = glm::distance(Position, candidatePos);

    if (candidateDist < currentDist)
        ApplyTarget(candidate);
}

// ApplyTarget – owner only.
// Commits the new target, broadcasts SetTarget RPC to others FIRST so the
// new owner already knows the target when OwnerChange arrives, then
// transfers ownership to the peer responsible for the target entity.
void NpcHumanBase::ApplyTarget(const EntityHandle& newTarget)
{
    currentTarget     = newTarget;
    targetSwitchTimer = TARGET_SWITCH_COOLDOWN;

    // Resolve immediately so this peer's AI sees the target this frame.
    ResolveTarget();

    // Broadcast target to others before SetOwner.
    NetPacket args(PacketType::RPC);
    newTarget.Write(args);
    SendRPC(static_cast<uint8_t>(NpcRPC::SetTarget), args, RPCTarget::Others);

    // Transfer ownership: use the handle's owner peer id.
    // For a non-networked entity this returns 0 (server).
    SetOwner(newTarget.GetOwnerPeerId());
}

// ---------------------------------------------------------------------------
// Sound
// ---------------------------------------------------------------------------

void NpcHumanBase::SetupSoundPlayer(SoundPlayer* sp)
{
    sp->MaxDistance = 100;
    sp->Volume      = 1.0;
    sp->MinDistance = 0.5;
}

void NpcHumanBase::PlaySoundEffect(std::string eventName)
{
    if (!soundPlayer) return;
    soundPlayer->SetSound(FmodEventInstance::Create(eventName));
    soundPlayer->Play();
}

// ---------------------------------------------------------------------------
// Damage / death
// ---------------------------------------------------------------------------

void NpcHumanBase::OnPointDamage(float Damage, vec3 Point, vec3 Direction,
                                  string bone, Entity* DamageCauser, Entity* Weapon)
{
    if (IsDead()) return;

    Damage *= mesh->GetHitboxDamageMultiplier(bone);

    // Effects – run on every peer.
    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point,
        MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage / 10.0f));
    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point,
        MathHelper::FindLookAtRotation(Direction, vec3(0)), vec3(Damage / 10.0f));
    SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1,
                              Damage / 20.0f, false, Point);

    // Leg hit → ragdoll on every peer.
    if (canBeStunRagdolled)
    {
        if (bone == "calf_l" || bone == "calf_r" ||
            bone == "thigh_l" || bone == "thigh_r")
        {
            StartStunnedRagdoll();
        }
    }

    OnDamage(Damage, DamageCauser, Weapon);

    // Broadcast to others – post-multiplier damage, bone for ragdoll check.
    NetPacket args(PacketType::RPC);
    args.WriteFloat(Damage);
    args.WriteString(bone);
    args.WriteVector3(Point);
    args.WriteVector3(Direction);
    args.WriteBool(DamageCauser != nullptr);
    SendRPC(static_cast<uint8_t>(NpcRPC::TakeDamage), args, RPCTarget::Others);
}

void NpcHumanBase::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    if (IsDead()) return;

    Damage = ModifyIncomingDamage(Damage);

    // Score only on the peer that has a local player.
    if (Player::Instance != nullptr)
        ScoreSystem::Instance().addScore(std::min(Damage, Health));

    Health -= Damage;

    if (DamageCauser != nullptr)
    {
        controller.SetVelocity(controller.GetVelocity() / 2.0f);
        speed /= 2.0f;
        PlaySoundEffect("event:/NPC/Enemy1/Enemy1Damage");
    }

    if (Health <= 0)
    {
        NetPacket deathArgs(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Death), deathArgs, RPCTarget::Others);
        Death();
    }
}

void NpcHumanBase::Death()
{
    if (IsDead()) return;

    state = NpcState::Dead;

    mesh->StartRagdoll();
    mesh->SetAnimationPaused(true);
    controller.SetVelocity(vec3(0));

    PlaySoundEffect("event:/NPC/Enemy1/Enemy1Death");

    GetDebuffsList().clear();
    UpdateStatusWidgets();

    controller.Destroy();

    // Game-logic callbacks only on the local player's peer.
    if (Player::Instance != nullptr)
    {
        ScoreSystem::Instance().addScore(MaxHealth * 0.5f);
        CallActionOnEntityWithId(OwnerId, "despawned");
    }

    if (soundPlayer)
    {
        soundPlayer->DestroyWithDelay(3);
        soundPlayer = nullptr;
    }
}

// Stun – owner originates it and broadcasts to others.
// Non-owners receive it via RPC and call Stun() directly (which won't
// re-broadcast since isOwned is false).
void NpcHumanBase::Stun()
{
    if (IsDead())    return;
    if (IsStunned()) return;

    state = NpcState::Stunned;
    mesh->PlayAnimation("stun");
    mesh->PullRootMotion();
    PlaySoundEffect("event:/NPC/Enemy1/Enemy1Stun");

    if (isOwned)
    {
        NetPacket args(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Stun), args, RPCTarget::Others);
    }
}

// ---------------------------------------------------------------------------
// Ragdoll
// ---------------------------------------------------------------------------

void NpcHumanBase::StartStunnedRagdoll()
{
    if (IsDead()) return;

    if (!stunnedRagdoll)
    {
        mesh->RagdollPoseFollowStrength = 0.5f;
        mesh->StartRagdoll();
        stunnedRagdoll = true;

        // Only the owner broadcasts – it's the authoritative hit detector.
        if (isOwned)
        {
            NetPacket args(PacketType::RPC);
            SendRPC(static_cast<uint8_t>(NpcRPC::StunRagdoll), args, RPCTarget::Others);
        }
    }

    stunnedRagdollDelay.AddDelay(2.0f);
}

void NpcHumanBase::UpdateStunnedReturn()
{
    if (!stunnedRagdoll) return;

    Body* pelvisBody = mesh->FindHitboxByName("pelvis");
    vec3  pelvisPos  = FromPhysics(pelvisBody->GetPosition());

    controller.SetVelocity(vec3(0, controller.GetVelocity().y, 0));
    Position = pelvisPos + vec3(0, 1.5f, 0);
    controller.SetPosition(Position);
    controller.SetVelocity(vec3());

    if (stunnedRagdollDelay.Wait()) return;

    bool hitsGround = Physics::LineTrace(
        pelvisPos, pelvisPos - vec3(0, 0.5f, 0), BodyType::World).hasHit;

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
    if (!mesh->InRagdoll) return;

    mesh->UpdateHitboxes();

    vec3 pelvisPos = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Position;
    vec3 spinePos  = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("spine_03")).Position;

    ragdollPose = mesh->GetAnimationPose();

    bool onFront = MathHelper::GetUpVector(
        MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Rotation).y > 0;

    getFromRagdollAnimation->PlayAnimation(onFront ? "front" : "back", false, 0);

    auto pelvisTransformWorld = MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis"));
    Position = pelvisTransformWorld.Position + vec3(0, 1.0f, 0);
    controller.SetPosition(Position);

    float oldRot = mesh->Rotation.y;

    if (onFront)
        mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(pelvisPos, spinePos).y, 0);
    else
        mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(spinePos, pelvisPos).y, 0);

    movingDirection = MathHelper::GetForwardVector(mesh->Rotation);

    auto pelvisTransform = MathHelper::DecomposeMatrix(ragdollPose.boneTransforms["pelvis"]);
    pelvisTransform.Position = vec3(0, 3.0f, 0);
    pelvisTransform.RotationQuaternion =
        MathHelper::GetRotationQuaternion(vec3(0, oldRot - mesh->Rotation.y, 0))
        * pelvisTransform.RotationQuaternion;
    ragdollPose.boneTransforms["pelvis"] = pelvisTransform.ToMatrix();

    ragdollPelvisWorldPos = pelvisPos;
    pelvisBlendTimer      = 0.0f;

    mesh->StopRagdoll();
    mesh->PasteAnimationPose(ragdollPose);

    returningFromRagdoll = true;
    movingDirection      = MathHelper::GetForwardVector(mesh->Rotation);
    desiredDirection     = movingDirection;

    mesh->StopAnimation();

    // Only the owner decides when to start recovery (via UpdateStunnedReturn).
    if (isOwned)
    {
        NetPacket args(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::ReturnFromRagdoll), args, RPCTarget::Others);
    }
}

void NpcHumanBase::UpdateReturnFromRagdoll()
{
    if (stunnedRagdollDelay.Wait()) return;
    if (!returningFromRagdoll) return;

    controller.SetVelocity(vec3(0, controller.GetVelocity().y, 0));

    getFromRagdollAnimation->Update(ModifyAnimationSpeed(1.0f));

    auto meshPose = mesh->GetAnimationPose();

    float blendInTime  = 0.5f;
    float blendOutTime = 0.7f;

    float lerpProgressFromStart =
        1.0f - ((blendInTime - getFromRagdollAnimation->GetAnimationTime()) / blendInTime);
    lerpProgressFromStart = saturate(lerpProgressFromStart);

    auto animationPose = getFromRagdollAnimation->GetAnimationPose();
    auto newPose = AnimationPose::Lerp(animationPose, ragdollPose, 1.0f - lerpProgressFromStart);

    float lerpProgressFromEnd =
        ((blendOutTime - (getFromRagdollAnimation->GetAnimationDuration()
                          - getFromRagdollAnimation->GetAnimationTime())) / blendOutTime);
    newPose = AnimationPose::Lerp(meshPose, newPose, 1.0f - lerpProgressFromEnd);

    mesh->PasteAnimationPose(newPose);

    if (pelvisBlendTimer < 0.5f)
    {
        pelvisBlendTimer += Time::DeltaTimeF;
        float t = saturate(pelvisBlendTimer / 0.5f);
        vec3 animPelvisWorldPos =
            MathHelper::DecomposeMatrix(mesh->GetBoneMatrixWorld("pelvis")).Position;
        vec3 targetPelvisWorldPos = mix(ragdollPelvisWorldPos, animPelvisWorldPos, t);
        mesh->Position += targetPelvisWorldPos - animPelvisWorldPos;
    }

    if (!getFromRagdollAnimation->IsAnimationPlaying())
    {
        returningFromRagdoll = false;
        stunnedRagdoll       = false;
        if (state == NpcState::Stunned)
            state = NpcState::Idle;
        mesh->PlayAnimation("run", true, 0.5f);
        speed = maxSpeed;
    }
}

// ---------------------------------------------------------------------------
// Flee
// ---------------------------------------------------------------------------

void NpcHumanBase::UpdateFleeTarget()
{
    if (!resolvedTarget) return;
    if (!fleeSearchDelay.Wait())
    {
        auto path = NavigationSystem::FindFleePath(Position, resolvedTarget->Position);
        if (!path.empty())
        {
            pathFollow.UpdateStartAndTarget(Position, path[path.size() - 1]);
            pathFollow.TryPerform();
        }
        fleeSearchDelay.AddDelay(0.2f);
    }
}

// ---------------------------------------------------------------------------
// Non-owner animation mirror
// ---------------------------------------------------------------------------

void NpcHumanBase::UpdateNonOwnerAnimation()
{
    if (IsDead() || IsStunned() || stunnedRagdoll || returningFromRagdoll) return;
    if (IsAttacking()) return;

    auto animName = mesh->GetAnimationName();

    if (!resolvedTarget || IsIdle())
    {
        if (animName != "idle") mesh->PlayAnimation("idle", true, 0.5f);
    }
    else if (speed > 0.1f)
    {
        if (animName != "run") mesh->PlayAnimation("run", true, 0.5f);
    }
    else
    {
        if (animName != "idle") mesh->PlayAnimation("idle", true, 0.5f);
    }
}

// ---------------------------------------------------------------------------
// Network – snapshot
// ---------------------------------------------------------------------------

void NpcHumanBase::NetSerialize(NetPacket& packet)
{
    packet.WriteVector3(Position);
    packet.WriteVector3(mesh->Rotation);
    packet.WriteVector3(movingDirection);
    packet.WriteFloat(speed);
    packet.WriteFloat(Health);
    packet.WriteUInt8(static_cast<uint8_t>(state));
    packet.WriteBool(stunnedRagdoll);
    packet.WriteBool(returningFromRagdoll);
    currentTarget.Write(packet);
    packet.WriteFloat(targetSwitchTimer);
}

void NpcHumanBase::NetDeserialize(NetPacket& packet)
{
    vec3     remotePos     = packet.ReadVector3();
    vec3     remoteRot     = packet.ReadVector3();
    vec3     remoteMoveDir = packet.ReadVector3();
    float    remoteSpeed   = packet.ReadFloat();
    float    remoteHealth  = packet.ReadFloat();
    NpcState remoteState   = static_cast<NpcState>(packet.ReadUInt8());
    bool     remoteStunRag = packet.ReadBool();
    bool     remoteReturn  = packet.ReadBool();
    EntityHandle remoteTarget      = EntityHandle::Read(packet);
    float        remoteSwitchTimer = packet.ReadFloat();

    // Owner discards snapshots from the previous owner (in-flight after transfer).
    if (isOwned) return;

    Position = mix(Position, remotePos, 0.3f);
    controller.SetPosition(Position);
    mesh->Rotation  = remoteRot;
    movingDirection = remoteMoveDir;
    speed           = remoteSpeed;
    Health          = remoteHealth;

    if (remoteTarget != currentTarget)
    {
        currentTarget     = remoteTarget;
        targetSwitchTimer = remoteSwitchTimer;
    }
    else
    {
        targetSwitchTimer = remoteSwitchTimer;
    }

    // Death is the only state transition the snapshot triggers directly –
    // it's needed for late joiners who missed the Death RPC.
    if (remoteState == NpcState::Dead && !IsDead())
    {
        Death();
        return;
    }

    if (!IsDead())
    {
        // Don't overwrite transient ragdoll/stun states with a stale value.
        bool inTransientState = stunnedRagdoll || returningFromRagdoll || IsStunned();
        if (!inTransientState)
            state = remoteState;

        if (remoteStunRag && !stunnedRagdoll)
            StartStunnedRagdoll();

        if (remoteReturn && !returningFromRagdoll)
            StartReturnFromRagdoll();
    }
}

// ---------------------------------------------------------------------------
// Network – RPCs
// ---------------------------------------------------------------------------

void NpcHumanBase::OnRPC(uint8_t rpcId, NetPacket& args)
{
    switch (static_cast<NpcRPC>(rpcId))
    {
        case NpcRPC::TakeDamage:
        {
            float  damage    = args.ReadFloat();
            string bone      = args.ReadString();
            vec3   point     = args.ReadVector3();
            vec3   direction = args.ReadVector3();
            bool   hasCauser = args.ReadBool();

            if (IsDead()) break;

            // damage is already post-multiplier – do NOT re-multiply.
            GlobalParticleSystem::SpawnParticleAt("hit_flesh", point,
                MathHelper::FindLookAtRotation(vec3(0), direction), vec3(damage / 10.0f));
            GlobalParticleSystem::SpawnParticleAt("hit_flesh", point,
                MathHelper::FindLookAtRotation(direction, vec3(0)), vec3(damage / 10.0f));
            SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1,
                                      damage / 20.0f, false, point);

            if (canBeStunRagdolled &&
                (bone == "calf_l" || bone == "calf_r" ||
                 bone == "thigh_l" || bone == "thigh_r"))
            {
                StartStunnedRagdoll();
            }

            OnDamage(damage, hasCauser ? this : nullptr);
            break;
        }

        case NpcRPC::Death:
            Death();
            break;

        case NpcRPC::Attack:
            if (!isOwned) Attack();
            break;

        case NpcRPC::Stun:
            Stun();
            break;

        case NpcRPC::StunRagdoll:
            StartStunnedRagdoll();
            break;

        case NpcRPC::ReturnFromRagdoll:
            StartReturnFromRagdoll();
            break;

        case NpcRPC::SetTarget:
        {
            EntityHandle t = EntityHandle::Read(args);
            currentTarget     = t;
            targetSwitchTimer = TARGET_SWITCH_COOLDOWN;
            break;
        }

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Save-game serialization
// ---------------------------------------------------------------------------

void NpcHumanBase::Serialize(json& target)
{
    Entity::Serialize(target);

    animationStateSaveData           = mesh->GetAnimationState();
    Rotation                         = mesh->Rotation;
    getFromRagdollAnimationSaveState = getFromRagdollAnimation->GetAnimationState();

    uint8_t stateVal = static_cast<uint8_t>(state);
    SERIALIZE_FIELD(target, Rotation);
    SERIALIZE_FIELD(target, desiredDirection);
    SERIALIZE_FIELD(target, movingDirection);
    SERIALIZE_FIELD(target, speed);
    SERIALIZE_FIELD(target, stateVal);
    SERIALIZE_FIELD(target, animationStateSaveData);
    SERIALIZE_FIELD(target, stunnedRagdoll);
    SERIALIZE_FIELD(target, returningFromRagdoll);
    SERIALIZE_FIELD(target, inAttackDelay);

    auto debuffsJson = SerializeDebuffs();
    SERIALIZE_FIELD(target, debuffsJson);

    SERIALIZE_FIELD(target, stunnedRagdollDelay);
    SERIALIZE_FIELD(target, ragdollPose);
    SERIALIZE_FIELD(target, getFromRagdollAnimationSaveState);
    SERIALIZE_FIELD(target, ragdollPelvisWorldPos);
    SERIALIZE_FIELD(target, pelvisBlendTimer);

    // Serialize EntityHandle
    bool   handleNetworked = currentTarget.isNetworked;
    uint64_t handleNetId   = currentTarget.networkId;
    string   handleEntId   = currentTarget.entityId;
    SERIALIZE_FIELD(target, handleNetworked);
    SERIALIZE_FIELD(target, handleNetId);
    SERIALIZE_FIELD(target, handleEntId);
}

void NpcHumanBase::Deserialize(json& source)
{
    Entity::Deserialize(source);

    DESERIALIZE_FIELD(source, Rotation);
    DESERIALIZE_FIELD(source, desiredDirection);
    DESERIALIZE_FIELD(source, movingDirection);
    DESERIALIZE_FIELD(source, speed);

    uint8_t stateVal = 0;
    DESERIALIZE_FIELD(source, stateVal);
    state = static_cast<NpcState>(stateVal);

    DESERIALIZE_FIELD(source, animationStateSaveData);
    DESERIALIZE_FIELD(source, stunnedRagdoll);
    DESERIALIZE_FIELD(source, returningFromRagdoll);
    DESERIALIZE_FIELD(source, inAttackDelay);

    json debuffsJson;
    DESERIALIZE_FIELD(source, debuffsJson);
    DeserializeDebuffs(debuffsJson);

    DESERIALIZE_FIELD(source, stunnedRagdollDelay);
    DESERIALIZE_FIELD(source, ragdollPose);
    DESERIALIZE_FIELD(source, getFromRagdollAnimationSaveState);

    controller.SetPosition(Position);

    if (IsDead())
    {
        controller.Destroy();
        if (soundPlayer) { soundPlayer->Destroy(); soundPlayer = nullptr; }
    }

    mesh->Rotation = Rotation;
    mesh->SetAnimationState(animationStateSaveData);
    mesh->Update(0);
    mesh->PullRootMotion();

    if (returningFromRagdoll)
        getFromRagdollAnimation->SetAnimationState(getFromRagdollAnimationSaveState);

    DESERIALIZE_FIELD(source, ragdollPelvisWorldPos);
    DESERIALIZE_FIELD(source, pelvisBlendTimer);

    bool   handleNetworked = false;
    uint64_t handleNetId   = 0;
    string   handleEntId   = {};
    DESERIALIZE_FIELD(source, handleNetworked);
    DESERIALIZE_FIELD(source, handleNetId);
    DESERIALIZE_FIELD(source, handleEntId);
    currentTarget.isNetworked = handleNetworked;
    currentTarget.networkId   = handleNetId;
    currentTarget.entityId    = handleEntId;
}

// ---------------------------------------------------------------------------
// Misc
// ---------------------------------------------------------------------------

void NpcHumanBase::UpdateStatusWidgets()
{
    statusWidget->Position = mesh->Position + WorldOrientationManager::GetUpVector() * 2.0f;
    statusWidget->TwoSided = true;
    statusWidget->Update();
    statusWidget->Visible  = !IsDead() && widgetVisible;
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

void NpcHumanBase::FromData(EntityData data) { Entity::FromData(data); }

void NpcHumanBase::OnAction(std::string action)
{
    if (action == "triggerOnPlayer" && Player::Instance != nullptr)
    {
        // Trigger volumes fire on the server. Player::Instance here is the
        // server's local player. Find its RemotePlayer NetworkedEntity so we
        // can build a proper networked handle.
        auto* ne = dynamic_cast<NetworkedEntity*>(Player::Instance);
        if (ne)
            ApplyTarget(EntityHandle::FromNetworked(ne->networkId));
        else
        {
            // Non-networked local player – use entity string Id (server-only).
            ApplyTarget(EntityHandle::FromEntity(Player::Instance));
        }
    }
}
