// NpcHumanBase.cpp
#include "NpcHumanBase.h"
#include <RandomHelper.h>

#include <World/WorldOrientationManager.h>
#include <Systems/ScoreSystem/ScoreSystem.h>
#include <Entities/Player/Player.hpp>
#include <AiPerception/AiPerceptionSystem.h>

// ---------------------------------------------------------------------------
// We assume the project provides a way to look up entities by networkId.
// If the API is different, adjust the two calls in ResolveTarget().
//   Level::Current->FindNetworkedEntityById(uint64_t networkId)
//   Level::Current->FindRemotePlayerByClientId(uint8_t clientId)
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

    CanMigrateOwner = true;

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
    movingDirection  = desiredDirection;

    soundPlayer = SoundPlayer::Create();
    SetupSoundPlayer(soundPlayer);

    // Perception observer is only needed on the owning peer.
    if (isOwned)
    {
        observer = AiPerceptionSystem::CreateObserver(
            Position, MathHelper::GetForwardVector(mesh->Rotation), 150);
    }
}

// ---------------------------------------------------------------------------
// Targeting
// ---------------------------------------------------------------------------

// ResolveTarget rebuilds resolvedTarget from currentTarget on every peer.
//
// Player targets: the local Player::Instance is used when this peer's clientId
// matches the target's clientId.  Otherwise we look for a RemotePlayer whose
// networkId client field matches.  RemotePlayer entities that are owned by this
// peer do NOT have a perception target set (they represent our own avatar on
// remote machines), so we guard with clientId comparison instead of relying on
// the perception system.
void NpcHumanBase::ResolveTarget()
{
    switch (currentTarget.type)
    {
        case TargetType::None:
            resolvedTarget = nullptr;
            break;

        case TargetType::Player:
        {
            // Is the target the local player on this peer?
            if (Player::Instance != nullptr &&
                networkOwner == currentTarget.clientId)
            {
                // We are the peer that owns this player – use the singleton.
                resolvedTarget = Player::Instance;
            }
            else
            {
                // Find the RemotePlayer entity whose networkId was packed with
                // the matching clientId.  RemotePlayer entities owned by us
                // exist on the server/host side but have isOwned=true, so we
                // skip them (the owner sees them as the local Player singleton).
                resolvedTarget = nullptr;
                for (auto& e : Level::Current->LevelObjects)
                {
                    auto* ne = dynamic_cast<NetworkedEntity*>(e);
                    if (ne == nullptr) continue;
                    if (!ne->HasTag("player")) continue;
                    if (ne->isOwned) continue; // skip our own player entity

                    if (NetworkIdGetClientId(ne->networkId) == currentTarget.clientId)
                    {
                        resolvedTarget = ne;
                        break;
                    }
                }
            }
            break;
        }

        case TargetType::NetworkEntity:
        {
            resolvedTarget = nullptr;
            for (auto& e : Level::Current->LevelObjects)
            {
                auto* ne = dynamic_cast<NetworkedEntity*>(e);
                if (ne && ne->networkId == currentTarget.networkId)
                {
                    resolvedTarget = ne;
                    break;
                }
            }
            break;
        }

        case TargetType::Entity:
        {
            // Entity IDs are not replicated; this path is only meaningful
            // on the owning/server peer.
            resolvedTarget = isOwned
                ? Level::Current->FindEntityWithId(currentTarget.entityId)
                : nullptr;
            break;
        }
    }
}

// UpdatePerception – owner only.
// Collects every player-tagged perception target visible this frame,
// finds which one is closest, then calls TrySetTarget.
void NpcHumanBase::UpdatePerception()
{
    if (!isOwned || observer == nullptr) return;

    observer->forward  = MathHelper::GetForwardVector(mesh->Rotation);
    observer->position = Position + vec3(0, 0.7f, 0);

    // Advance the switch cooldown timer.
    if (targetSwitchTimer > 0.0f)
        targetSwitchTimer = std::max(0.0f, targetSwitchTimer - Time::DeltaTimeF);

    // ── Collect all candidate player targets from perception ──────────────
    // For each visible perception target tagged "player" we build an NpcTarget
    // and note the world position so we can pick the closest.
    struct Candidate
    {
        NpcTarget target;
        vec3      worldPos;
    };
    std::vector<Candidate> candidates;

    for (auto& pt : observer->visibleTargets)
    {
        if (!pt->HasTag("player")) continue;

        // pt->ownerId is the entity Id of the entity that registered this
        // perception target (typically the player or remote player entity).
        Entity* e = Level::Current->FindEntityWithId(pt->ownerId);
        if (e == nullptr) continue;

        // Determine how to encode this target.
        auto* ne = dynamic_cast<NetworkedEntity*>(e);
        if (ne != nullptr)
        {
            uint8_t ownerClient = NetworkIdGetClientId(ne->networkId);

            if (ne->HasTag("player"))
            {
                // Use Player encoding so the target resolves to
                // Player::Instance on the matching peer.
                candidates.push_back({ NpcTarget::FromPlayer(ownerClient),
                                       e->Position });
            }
            else
            {
                candidates.push_back({ NpcTarget::FromNetworkEntity(ne->networkId),
                                       e->Position });
            }
        }
        else
        {
            // Non-networked entity (e.g. server-only trigger actor) –
            // encode by entity Id; only meaningful while we are the owner.
            candidates.push_back({ NpcTarget::FromEntity(e->Id), e->Position });
        }
    }

    // Also react to heard sounds pointing to a player entity.
    for (auto& s : observer->heardSounds)
    {
        Entity* causer = Level::Current->FindEntityWithId(s.causerId);
        if (causer == nullptr || !causer->HasTag("player")) continue;

        auto* ne = dynamic_cast<NetworkedEntity*>(causer);
        if (ne != nullptr)
        {
            candidates.push_back({ NpcTarget::FromPlayer(NetworkIdGetClientId(ne->networkId)),
                                   causer->Position });
        }
    }

    if (candidates.empty()) return;

    // ── Pick closest candidate ────────────────────────────────────────────
    float    bestDist = std::numeric_limits<float>::max();
    Candidate* best   = nullptr;

    for (auto& c : candidates)
    {
        float d = glm::distance(Position, c.worldPos);
        if (d < bestDist)
        {
            bestDist = d;
            best     = &c;
        }
    }

    if (best != nullptr)
        TrySetTarget(best->target, best->worldPos);
}

// TrySetTarget – owner only.
// Switch to a new target only when:
//   (a) we currently have no target, OR
//   (b) the cooldown has elapsed AND the candidate is strictly closer than
//       the current resolved target.
void NpcHumanBase::TrySetTarget(const NpcTarget& candidate, const vec3& candidatePos)
{
    // Same target – nothing to do.
    if (candidate == currentTarget) return;

    // No current target → accept immediately regardless of cooldown.
    if (!currentTarget.IsValid())
    {
        ApplyTarget(candidate);
        return;
    }

    // Cooldown still active → keep current target.
    if (targetSwitchTimer > 0.0f) return;

    // Compare distances; only switch if the candidate is closer.
    float currentDist = resolvedTarget
        ? glm::distance(Position, resolvedTarget->Position)
        : std::numeric_limits<float>::max();

    float candidateDist = glm::distance(Position, candidatePos);

    if (candidateDist < currentDist)
        ApplyTarget(candidate);
}

// ApplyTarget – owner only.
// Stores the new descriptor, resets the switch cooldown, and broadcasts via RPC.
void NpcHumanBase::ApplyTarget(const NpcTarget& newTarget)
{
    currentTarget     = newTarget;
    targetSwitchTimer = TARGET_SWITCH_COOLDOWN;

    // Immediately resolve so the owning peer's AI loop sees it this frame.
    ResolveTarget();

    // Broadcast to all non-owning peers so their resolvedTarget stays in sync.
    NetPacket args(PacketType::RPC);
    newTarget.Write(args);
    SendRPC(static_cast<uint8_t>(NpcRPC::SetTarget), args, RPCTarget::All);
}

// Net_ApplySetTarget – non-owning peers only.
void NpcHumanBase::Net_ApplySetTarget(const NpcTarget& newTarget)
{
    currentTarget     = newTarget;
    targetSwitchTimer = TARGET_SWITCH_COOLDOWN;
    // resolvedTarget will be rebuilt next ResolveTarget() call.
}

// ---------------------------------------------------------------------------
// Sound helpers
// ---------------------------------------------------------------------------

void NpcHumanBase::SetupSoundPlayer(SoundPlayer* sp)
{
    sp->MaxDistance = 100;
    sp->Volume      = 1.0;
    sp->MinDistance = 0.5;
}

void NpcHumanBase::PlaySoundEffect(std::string eventName)
{
    if (soundPlayer == nullptr) return;
    soundPlayer->SetSound(FmodEventInstance::Create(eventName));
    soundPlayer->Play();
}

// ---------------------------------------------------------------------------
// Combat helpers
// ---------------------------------------------------------------------------

void NpcHumanBase::Stun()
{
    state = NpcState::Stunned;
    mesh->PlayAnimation("stun");
    mesh->PullRootMotion();
    PlaySoundEffect("event:/NPC/Enemy1/Enemy1Stun");
}

void NpcHumanBase::StartStunnedRagdoll()
{
    if (IsDead()) return;

    if (!stunnedRagdoll)
    {
        mesh->RagdollPoseFollowStrength = 0.5f;
        mesh->StartRagdoll();
        stunnedRagdoll = true;
    }

    stunnedRagdollDelay.AddDelay(2.0f);
}

void NpcHumanBase::UpdateStunnedReturn()
{
    if (!stunnedRagdoll) return;

    Body* pelvisBody = mesh->FindHitboxByName("pelvis");
    vec3  pelvisPos  = FromPhysics(pelvisBody->GetPosition());

    vec3 vel = controller.GetVelocity();
    controller.SetVelocity(vec3(0, vel.y, 0));

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

    movingDirection  = MathHelper::GetForwardVector(mesh->Rotation);
    desiredDirection = movingDirection;

    mesh->StopAnimation();
}

void NpcHumanBase::UpdateReturnFromRagdoll()
{
    if (stunnedRagdollDelay.Wait()) return;
    if (!returningFromRagdoll) return;

    vec3 vel = controller.GetVelocity();
    controller.SetVelocity(vec3(0, vel.y, 0));

    getFromRagdollAnimation->Update(ModifyAnimationSpeed(1.0f));

    auto  meshPose = mesh->GetAnimationPose();

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
        if (returningFromRagdoll)
        {
            returningFromRagdoll = false;
            stunnedRagdoll       = false;
            // Transition back to Idle; subclass/AI loop will set Chasing as
            // soon as a target is confirmed.
            if (state == NpcState::Stunned)
                state = NpcState::Idle;
            mesh->PlayAnimation("run", true, 0.5f);
            speed = maxSpeed;
        }
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

    CallActionOnEntityWithId(OwnerId, "despawned");

    ScoreSystem::Instance().addScore(MaxHealth * 0.5f);

    if (soundPlayer)
    {
        soundPlayer->DestroyWithDelay(3);
        soundPlayer = nullptr;
    }
}

void NpcHumanBase::OnPointDamage(float Damage, vec3 Point, vec3 Direction,
                                  string bone, Entity* DamageCauser, Entity* Weapon)
{
    if (!isOwned) return;

    Damage *= mesh->GetHitboxDamageMultiplier(bone);
    Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

    if (canBeStunRagdolled)
    {
        if (bone == "calf_l" || bone == "calf_r" ||
            bone == "thigh_l" || bone == "thigh_r")
        {
            StartStunnedRagdoll();
        }
    }

    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point,
        MathHelper::FindLookAtRotation(vec3(0), Direction),
        vec3(Damage / 10.0f));
    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point,
        MathHelper::FindLookAtRotation(Direction, vec3(0)),
        vec3(Damage / 10.0f));

    SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1,
                              Damage / 20.0f, false, Point);
}

void NpcHumanBase::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    // ---------------------------------------------------------------------------
    // Damage flow:
    //   Player-side  → calls OnPointDamage/OnDamage on its local NPC copy,
    //                  then sends TakeDamage RPC to the owner.
    //   Owner        → applies final damage authoritatively; if fatal, broadcasts
    //                  Death RPC so all peers enter dead state reliably even when
    //                  a snapshot is lost.
    //   Non-owner    → no-op; health arrives via snapshot (Health field in
    //                  NetSerialize).  Death is also covered by the Death RPC.
    // ---------------------------------------------------------------------------
    if (!isOwned) return;

    Damage = ModifyIncomingDamage(Damage);

    ScoreSystem::Instance().addScore(std::min(Damage, Health));

    Health -= Damage;

    if (Health <= 0)
    {
        NetPacket deathArgs(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Death), deathArgs, RPCTarget::All);
        Death();
    }

    if (DamageCauser != nullptr)
    {
        controller.SetVelocity(controller.GetVelocity() / 2.0f);
        speed /= 2.0f;
        PlaySoundEffect("event:/NPC/Enemy1/Enemy1Damage");
    }
}

// ---------------------------------------------------------------------------
// Flee
// ---------------------------------------------------------------------------

void NpcHumanBase::UpdateFleeTarget()
{
    if (resolvedTarget == nullptr) return;

    return;
    /*
    if (!fleeSearchDelay.Wait())
    {
        auto path = NavigationSystem::FindFleePath(Position, resolvedTarget->Position);

        if (!path.empty())
        {
            pathFollow.UpdateStartAndTarget(Position, path[path.size() - 1]);
            pathFollow.TryPerform();
        }

        fleeSearchDelay.AddDelay(0.2f);
    }*/
}

// ---------------------------------------------------------------------------
// Network replication – snapshot
// ---------------------------------------------------------------------------

void NpcHumanBase::NetSerialize(NetPacket& packet)
{
    // ── Transform & movement ─────────────────────────────────────────────
    packet.WriteVector3(Position);
    packet.WriteVector3(mesh->Rotation);
    packet.WriteVector3(movingDirection);
    packet.WriteFloat(speed);

    // ── Vital stats ───────────────────────────────────────────────────────
    packet.WriteFloat(Health);

    // ── State ─────────────────────────────────────────────────────────────
    packet.WriteUInt8(static_cast<uint8_t>(state));
    packet.WriteBool(stunnedRagdoll);
    packet.WriteBool(returningFromRagdoll);

    // ── Targeting ─────────────────────────────────────────────────────────
    currentTarget.Write(packet);
    packet.WriteFloat(targetSwitchTimer);
}

void NpcHumanBase::NetDeserialize(NetPacket& packet)
{
    // ── Transform & movement ─────────────────────────────────────────────
    vec3  remotePos     = packet.ReadVector3();
    vec3  remoteRot     = packet.ReadVector3();
    vec3  remoteMoveDir = packet.ReadVector3();
    float remoteSpeed   = packet.ReadFloat();

    // ── Vital stats ───────────────────────────────────────────────────────
    float remoteHealth = packet.ReadFloat();

    // ── State ─────────────────────────────────────────────────────────────
    NpcState remoteState    = static_cast<NpcState>(packet.ReadUInt8());
    bool     remoteStunRag  = packet.ReadBool();
    bool     remoteReturn   = packet.ReadBool();

    // ── Targeting ─────────────────────────────────────────────────────────
    NpcTarget remoteTarget = NpcTarget::Read(packet);
    float     remoteSwitchTimer = packet.ReadFloat();

    // ── Apply transform ───────────────────────────────────────────────────
    Position = mix(Position, remotePos, 0.3f);
    controller.SetPosition(Position);

    mesh->Rotation  = remoteRot;
    movingDirection = remoteMoveDir;
    speed           = remoteSpeed;

    // ── Apply health ──────────────────────────────────────────────────────
    Health = remoteHealth;

    // ── Apply targeting ───────────────────────────────────────────────────
    // Only update the descriptor; the RPC already handled the instant
    // notification.  The snapshot is the authoritative fallback.
    if (remoteTarget != currentTarget)
    {
        currentTarget     = remoteTarget;
        targetSwitchTimer = remoteSwitchTimer;
    }
    else
    {
        targetSwitchTimer = remoteSwitchTimer;
    }

    // ── Apply state transitions ───────────────────────────────────────────
    // State transitions are one-way driven by the RPC; here we only handle
    // cases where the snapshot arrives but the RPC was lost (failsafe).
    if (remoteState == NpcState::Dead && !IsDead())
    {
        Death();
    }
    if (remoteState == NpcState::Stunned && !IsStunned())
    {
        Stun();
    }
    if (remoteStunRag && !stunnedRagdoll)
    {
        StartStunnedRagdoll();
    }
    if (remoteReturn && !returningFromRagdoll)
    {
        StartReturnFromRagdoll();
    }
}

// ---------------------------------------------------------------------------
// Network replication – RPCs
// ---------------------------------------------------------------------------

void NpcHumanBase::OnRPC(uint8_t rpcId, NetPacket& args)
{
    switch (static_cast<NpcRPC>(rpcId))
    {
        case NpcRPC::TakeDamage:
            Net_ApplyDamage(args.ReadFloat());
            break;

        case NpcRPC::Death:
            Net_ApplyDeath();
            break;

        case NpcRPC::Attack:
            Net_ApplyAttack();
            break;

        case NpcRPC::Stun:
            Net_ApplyStun();
            break;

        case NpcRPC::SetTarget:
            Net_ApplySetTarget(NpcTarget::Read(args));
            break;

        default:
            break;
    }
}

// TakeDamage RPC – owner applies damage authoritatively.
// Non-owners do nothing; health arrives via the snapshot.
void NpcHumanBase::Net_ApplyDamage(float damage)
{
    if (!isOwned) return;

    Health -= damage;

    if (Health <= 0)
    {
        NetPacket deathArgs(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Death), deathArgs, RPCTarget::All);
        Death();
    }
}

// Death RPC – reliable safety net so clients always enter dead state even if
// a snapshot carrying state==Dead is dropped.
void NpcHumanBase::Net_ApplyDeath()
{
    Death();   // Death() is idempotent (guarded by IsDead()).
}

// Attack RPC – broadcast so every non-owning peer plays the animation in sync.
void NpcHumanBase::Net_ApplyAttack()
{
    if (!isOwned)
        Attack();
}

// Stun RPC
void NpcHumanBase::Net_ApplyStun()
{
    if (!isOwned)
        Stun();
}

// ---------------------------------------------------------------------------
// Save-game serialization (unchanged behaviour, updated field names)
// ---------------------------------------------------------------------------

void NpcHumanBase::Serialize(json& target)
{
    Entity::Serialize(target);

    animationStateSaveData = mesh->GetAnimationState();
    Rotation = mesh->Rotation;
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

    // Serialize the target descriptor (type + id fields).
    uint8_t targetType = static_cast<uint8_t>(currentTarget.type);
    SERIALIZE_FIELD(target, targetType);
    SERIALIZE_FIELD(target, currentTarget.clientId);
    SERIALIZE_FIELD(target, currentTarget.networkId);
    SERIALIZE_FIELD(target, currentTarget.entityId);
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
        if (soundPlayer)
        {
            soundPlayer->Destroy();
            soundPlayer = nullptr;
        }
    }

    mesh->Rotation = Rotation;
    mesh->SetAnimationState(animationStateSaveData);
    mesh->Update(0);
    mesh->PullRootMotion();

    if (returningFromRagdoll)
        getFromRagdollAnimation->SetAnimationState(getFromRagdollAnimationSaveState);

    DESERIALIZE_FIELD(source, ragdollPelvisWorldPos);
    DESERIALIZE_FIELD(source, pelvisBlendTimer);

    // Restore target descriptor.
    uint8_t targetType = 0;
    DESERIALIZE_FIELD(source, targetType);
    currentTarget.type = static_cast<TargetType>(targetType);
    DESERIALIZE_FIELD(source, currentTarget.clientId);
    DESERIALIZE_FIELD(source, currentTarget.networkId);
    DESERIALIZE_FIELD(source, currentTarget.entityId);

    // resolvedTarget will be rebuilt next frame in AsyncUpdate → ResolveTarget().
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

void NpcHumanBase::FromData(EntityData data)
{
    Entity::FromData(data);
}

void NpcHumanBase::OnAction(std::string action)
{
    if (action == "triggerOnPlayer")
    {
        // Trigger volumes fire on the server/owner, where Player::Instance is
        // the authoritative local player.  Encode as a Player target using the
        // local peer's network owner id.
        if (Player::Instance != nullptr)
        {
            auto* ne = dynamic_cast<NetworkedEntity*>(Player::Instance);
            uint8_t cid = ne ? NetworkIdGetClientId(ne->networkId) : networkOwner;
            ApplyTarget(NpcTarget::FromPlayer(cid));
        }
    }
}
