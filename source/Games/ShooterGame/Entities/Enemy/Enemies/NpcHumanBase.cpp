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
    movingDirection  = desiredDirection;

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

void NpcHumanBase::ResolveTarget()
{
    switch (currentTarget.type)
    {
        case TargetType::None:
            resolvedTarget = nullptr;
            return;

        case TargetType::Player:
            if (Player::Instance != nullptr &&
                NetworkManager::GetLocalPeerId() == currentTarget.clientId)
            {
                resolvedTarget = Player::Instance;
                return;
            }
            resolvedTarget = nullptr;
            for (auto& e : Level::Current->LevelObjects)
            {
                auto* ne = dynamic_cast<NetworkedEntity*>(e);
                if (!ne || !ne->HasTag("player") || ne->isOwned) continue;
                if (NetworkIdGetClientId(ne->networkId) == currentTarget.clientId)
                {
                    resolvedTarget = ne;
                    return;
                }
            }
            return;

        case TargetType::NetworkEntity:
            resolvedTarget = nullptr;
            for (auto& e : Level::Current->LevelObjects)
            {
                auto* ne = dynamic_cast<NetworkedEntity*>(e);
                if (ne && ne->networkId == currentTarget.networkId)
                {
                    resolvedTarget = ne;
                    return;
                }
            }
            return;

        case TargetType::Entity:
            // Entity string IDs not replicated; only the owner can resolve them.
            resolvedTarget = isOwned
                ? Level::Current->FindEntityWithId(currentTarget.entityId)
                : nullptr;
            return;
    }
}

// UpdatePerception – owner only.
// Collects all visible + heard player stimuli, picks the closest, calls
// TrySetTarget.  Heard sounds wake an Idle NPC even without line-of-sight.
void NpcHumanBase::UpdatePerception()
{
    if (!isOwned) return;

    // Create the observer lazily so it works after an ownership transfer.
    if (observer == nullptr)
    {
        observer = AiPerceptionSystem::CreateObserver(
            Position, MathHelper::GetForwardVector(mesh->Rotation), 150);
    }

    observer->forward  = MathHelper::GetForwardVector(mesh->Rotation);
    observer->position = Position + vec3(0, 0.7f, 0);

    if (targetSwitchTimer > 0.0f)
        targetSwitchTimer = std::max(0.0f, targetSwitchTimer - Time::DeltaTimeF);

    // If the current target descriptor no longer resolves (player disconnected),
    // clear it and return to server so the NPC idles until it perceives again.
    if (currentTarget.IsValid() && resolvedTarget == nullptr)
    {
        currentTarget.Clear();
        targetSwitchTimer = 0.0f;
        NetPacket args(PacketType::RPC);
        NpcTarget{}.Write(args);
        SendRPC(static_cast<uint8_t>(NpcRPC::SetTarget), args, RPCTarget::Others);
        SetOwner(0);
        return;
    }

    // ── Collect candidates, deduplicated by NpcTarget descriptor ──────────
    struct Candidate { NpcTarget target; vec3 worldPos; bool fromSight; };
    std::vector<Candidate> candidates;

    auto findOrAdd = [&](const NpcTarget& t, const vec3& pos, bool fromSight)
    {
        for (auto& c : candidates)
        {
            if (c.target == t)
            {
                if (fromSight && !c.fromSight) { c.worldPos = pos; c.fromSight = true; }
                return;
            }
        }
        candidates.push_back({ t, pos, fromSight });
    };

    // Visible targets
    for (auto& pt : observer->visibleTargets)
    {
        if (!pt->HasTag("player")) continue;
        Entity* e = Level::Current->FindEntityWithId(pt->ownerId);
        if (!e) continue;

        auto* ne = dynamic_cast<NetworkedEntity*>(e);
        if (ne)
            findOrAdd(NpcTarget::FromPlayer(NetworkIdGetClientId(ne->networkId)),
                      e->Position, true);
        else
            findOrAdd(NpcTarget::FromEntity(e->Id), e->Position, true);
    }

    // Heard sounds – valid in all non-dead/non-stunned states including Idle.
    if (!IsDead() && !IsStunned())
    {
        for (auto& s : observer->heardSounds)
        {
            Entity* causer = nullptr;
            if (Player::Instance && Player::Instance->Id == s.causerId)
                causer = Player::Instance;
            else
                causer = Level::Current->FindEntityWithId(s.causerId);

            if (!causer || !causer->HasTag("player")) continue;

            auto* ne = dynamic_cast<NetworkedEntity*>(causer);
            if (ne)
                findOrAdd(NpcTarget::FromPlayer(NetworkIdGetClientId(ne->networkId)),
                          s.position, false);
            else if (causer == Player::Instance)
                findOrAdd(NpcTarget::FromPlayer(NetworkManager::GetLocalPeerId()),
                          s.position, false);
        }
    }

    if (candidates.empty()) return;

    // Pick closest
    float      bestDist = std::numeric_limits<float>::max();
    Candidate* best     = nullptr;
    for (auto& c : candidates)
    {
        float d = glm::distance(Position, c.worldPos);
        if (d < bestDist) { bestDist = d; best = &c; }
    }

    if (best) TrySetTarget(best->target, best->worldPos);
}

void NpcHumanBase::TrySetTarget(const NpcTarget& candidate, const vec3& candidatePos)
{
    if (candidate == currentTarget) return;

    if (state != NpcState::Idle    &&
        state != NpcState::Chasing &&
        state != NpcState::Fleeing)
        return;

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

void NpcHumanBase::ApplyTarget(const NpcTarget& newTarget)
{
    currentTarget     = newTarget;
    targetSwitchTimer = TARGET_SWITCH_COOLDOWN;

    ResolveTarget();

    // Broadcast to others first so the new owner already has the target
    // when the OwnerChange packet arrives.
    NetPacket args(PacketType::RPC);
    newTarget.Write(args);
    SendRPC(static_cast<uint8_t>(NpcRPC::SetTarget), args, RPCTarget::Others);

    switch (newTarget.type)
    {
        case TargetType::Player:
            SetOwner(newTarget.clientId);
            break;
        case TargetType::NetworkEntity:
            for (auto& e : Level::Current->LevelObjects)
            {
                auto* ne = dynamic_cast<NetworkedEntity*>(e);
                if (ne && ne->networkId == newTarget.networkId)
                { SetOwner(ne->networkOwner); break; }
            }
            break;
        case TargetType::Entity:
        case TargetType::None:
        default:
            SetOwner(0);
            break;
    }
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
//
// Design: every peer that witnesses an event runs the full reaction locally,
// then sends the event to RPCTarget::Others so every other peer does the same.
// No peer runs an event twice.
// ---------------------------------------------------------------------------

// OnPointDamage – runs on every peer that witnesses a hit.
// Applies damage multiplier, hit effects (particles, sound, ragdoll), updates
// health, then broadcasts to all other peers so they react identically.
void NpcHumanBase::OnPointDamage(float Damage, vec3 Point, vec3 Direction,
                                  string bone, Entity* DamageCauser, Entity* Weapon)
{
    if (IsDead()) return;

    Damage *= mesh->GetHitboxDamageMultiplier(bone);

    // Hit effects – run on every peer.
    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point,
        MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage / 10.0f));
    GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point,
        MathHelper::FindLookAtRotation(Direction, vec3(0)), vec3(Damage / 10.0f));
    SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1,
                              Damage / 20.0f, false, Point);

    // Leg hit → ragdoll – runs on every peer.
    if (canBeStunRagdolled)
    {
        if (bone == "calf_l" || bone == "calf_r" ||
            bone == "thigh_l" || bone == "thigh_r")
        {
            StartStunnedRagdoll();
        }
    }

    // Apply health and check for death.
    OnDamage(Damage, DamageCauser, Weapon);

    // Broadcast to other peers so they run the same reaction.
    // We send the pre-multiplier damage value so the receiver applies the
    // multiplier too from the bone name (consistent with how this peer did it).
    // Actually we send post-multiplier damage directly to keep it simple —
    // bone is still sent for the ragdoll check on other peers.
    NetPacket args(PacketType::RPC);
    args.WriteFloat(Damage);
    args.WriteString(bone);
    args.WriteVector3(Point);
    args.WriteVector3(Direction);
    SendRPC(static_cast<uint8_t>(NpcRPC::TakeDamage), args, RPCTarget::Others);
}

// OnDamage – applies health reduction and triggers death if needed.
// Runs on every peer independently (called from OnPointDamage).
void NpcHumanBase::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    if (IsDead()) return;

    Damage = ModifyIncomingDamage(Damage);

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
        // Broadcast death to other peers before calling locally so all peers
        // enter dead state as close to simultaneously as possible.
        NetPacket deathArgs(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::Death), deathArgs, RPCTarget::Others);
        Death();
    }
}

// Death – runs on every peer.  Idempotent (guarded by IsDead()).
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

// Stun – runs on every peer.
// The peer that triggers the stun calls this directly, then broadcasts to
// RPCTarget::Others so every other peer does the same.
void NpcHumanBase::Stun()
{
    if (IsDead())    return;
    if (IsStunned()) return;  // Already stunned – don't re-trigger.

    state = NpcState::Stunned;
    mesh->PlayAnimation("stun");
    mesh->PullRootMotion();
    PlaySoundEffect("event:/NPC/Enemy1/Enemy1Stun");

    NetPacket args(PacketType::RPC);
    SendRPC(static_cast<uint8_t>(NpcRPC::Stun), args, RPCTarget::Others);
}

// ---------------------------------------------------------------------------
// Ragdoll
// ---------------------------------------------------------------------------

// StartStunnedRagdoll – runs on every peer.
void NpcHumanBase::StartStunnedRagdoll()
{
    if (IsDead()) return;

    if (!stunnedRagdoll)
    {
        mesh->RagdollPoseFollowStrength = 0.5f;
        mesh->StartRagdoll();
        stunnedRagdoll = true;

        NetPacket args(PacketType::RPC);
        SendRPC(static_cast<uint8_t>(NpcRPC::StunRagdoll), args, RPCTarget::Others);
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

// StartReturnFromRagdoll – runs on every peer.
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

    // Broadcast so other peers start recovery in sync.
    NetPacket args(PacketType::RPC);
    SendRPC(static_cast<uint8_t>(NpcRPC::ReturnFromRagdoll), args, RPCTarget::Others);
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
    if (IsAttacking()) return;  // Attack RPC started the animation; let it run.

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
//
// Snapshot is now lean: transform, health, state, target.
// It is NOT used to trigger events (stun, death, ragdoll) – those are RPCs.
// The snapshot is a position/health correction and a late-join catch-up only.
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
    NpcTarget remoteTarget      = NpcTarget::Read(packet);
    float     remoteSwitchTimer = packet.ReadFloat();

    // Owner discards snapshots from the previous owner (in-flight after transfer).
    if (isOwned) return;

    // ── Position / movement ───────────────────────────────────────────────
    // Lerp for smooth interpolation on non-owners.
    Position = mix(Position, remotePos, 0.3f);
    controller.SetPosition(Position);
    mesh->Rotation  = remoteRot;
    movingDirection = remoteMoveDir;
    speed           = remoteSpeed;

    // ── Health ─────────────────────────────────────────────────────────────
    // Accept the authoritative value without triggering damage reactions here.
    // Reactions come from RPCs; the snapshot just corrects drift.
    Health = remoteHealth;

    // ── Targeting ──────────────────────────────────────────────────────────
    if (remoteTarget != currentTarget)
    {
        currentTarget     = remoteTarget;
        targetSwitchTimer = remoteSwitchTimer;
    }
    else
    {
        targetSwitchTimer = remoteSwitchTimer;
    }

    // ── State (late-join / RPC packet loss recovery only) ─────────────────
    // If a peer joins mid-game or loses an RPC packet, the snapshot brings
    // it in sync.  We set state directly without calling the event functions
    // (no sounds, no animations triggered here) to avoid duplicating effects
    // that the peer already saw, or producing them out of context.
    // The one exception is Death – a dead NPC must enter ragdoll/cleanup.
    if (remoteState == NpcState::Dead && !IsDead())
    {
        Death();
        return;
    }

    if (!IsDead())
    {
        state          = remoteState;
        stunnedRagdoll = remoteStunRag;

        if (remoteReturn && !returningFromRagdoll)
            StartReturnFromRagdoll();
    }
}

// ---------------------------------------------------------------------------
// Network – RPCs
//
// Each RPC handler simply calls the same local function the sender already
// called.  No isOwned gate here; every peer runs the same reaction.
// ---------------------------------------------------------------------------

void NpcHumanBase::OnRPC(uint8_t rpcId, NetPacket& args)
{
    switch (static_cast<NpcRPC>(rpcId))
    {
        case NpcRPC::TakeDamage:
        {
            // Reconstruct the hit and run the full reaction locally.
            // Note: damage here is already post-multiplier (multiplied by sender).
            // We skip the multiplier and the bone-ragdoll check is still done
            // using the bone name so the ragdoll fires on this peer too.
            float  damage    = args.ReadFloat();
            string bone      = args.ReadString();
            vec3   point     = args.ReadVector3();
            vec3   direction = args.ReadVector3();

            if (IsDead()) break;

            // Hit effects – damage is already post-multiplied by the sender,
            // but we re-apply the bone multiplier since each peer's mesh may
            // differ slightly.  Keeping this consistent with OnPointDamage.
            damage *= mesh->GetHitboxDamageMultiplier(bone);

            GlobalParticleSystem::SpawnParticleAt("hit_flesh", point,
                MathHelper::FindLookAtRotation(vec3(0), direction), vec3(damage / 10.0f));
            GlobalParticleSystem::SpawnParticleAt("hit_flesh", point,
                MathHelper::FindLookAtRotation(direction, vec3(0)), vec3(damage / 10.0f));
            SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1,
                                      damage / 20.0f, false, point);

            // Ragdoll check
            if (canBeStunRagdolled &&
                (bone == "calf_l" || bone == "calf_r" ||
                 bone == "thigh_l" || bone == "thigh_r"))
            {
                StartStunnedRagdoll();
            }

            // Health + death
            OnDamage(damage);
            break;
        }

        case NpcRPC::Death:
            Death();
            break;

        case NpcRPC::Attack:
            // Owner already started the attack locally; non-owners play anim.
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
            NpcTarget t = NpcTarget::Read(args);
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

    uint8_t targetType = 0;
    DESERIALIZE_FIELD(source, targetType);
    currentTarget.type = static_cast<TargetType>(targetType);
    DESERIALIZE_FIELD(source, currentTarget.clientId);
    DESERIALIZE_FIELD(source, currentTarget.networkId);
    DESERIALIZE_FIELD(source, currentTarget.entityId);
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
        auto* ne    = dynamic_cast<NetworkedEntity*>(Player::Instance);
        uint8_t cid = ne ? NetworkIdGetClientId(ne->networkId)
                         : NetworkManager::GetLocalPeerId();
        ApplyTarget(NpcTarget::FromPlayer(cid));
    }
}
