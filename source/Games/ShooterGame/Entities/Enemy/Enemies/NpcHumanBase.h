// NpcHumanBase.h
#pragma once

#include <Input.h>
#include <MathHelper.hpp>
#include <Camera.h>
#include <SkeletalMesh.hpp>
#include <AssetRegistry.h>

#include <Navigation/PathFollowQuery.h>

#include <SoundSystem/SoundManager.hpp>
#include <Entities/SoundPlayer.h>
#include <Delay.hpp>

#include "../IEnemy.h"

#include <UI/WorldSpace/UiBilboard.h>
#include <UI/UiVideo.hpp>
#include "../../../UI/Enemy/UiNpcDebuffs.h"
#include <Particle/GlobalParticleSystem.hpp>
#include <SoundSystem/FmodEventInstance.h>
#include <Navigation/Navigation.hpp>
#include "NpcHelper.h"
#include <Animation.h>

#include <Character/CharacterController.h>

#include <AiPerception/Observer.h>

// NetworkedEntity replaces Entity as the base so we get replication plumbing.
#include "NetworkedEntity.h"

// ---------------------------------------------------------------------------
// NpcRPC – wire IDs for every one-shot event sent between peers.
// Keep values stable; they go on the wire.
// ---------------------------------------------------------------------------
enum class NpcRPC : uint8_t
{
    TakeDamage  = 0,  // args: float damage
    Death       = 1,  // args: (none)
    Attack      = 2,  // args: (none)  – broadcast so all peers play the animation
    Stun        = 3,  // args: (none)
    SetTarget   = 4,  // args: NpcTarget serialised inline (see NpcTarget::Write/Read)
};

// ---------------------------------------------------------------------------
// NpcState – mutually-exclusive high-level NPC state.
// Stored as a uint8_t on the wire.
// ---------------------------------------------------------------------------
enum class NpcState : uint8_t
{
    Idle     = 0,
    Chasing  = 1,
    Attacking= 2,
    Stunned  = 3,
    Fleeing  = 4,
    Dead     = 5,
};

// ---------------------------------------------------------------------------
// TargetType – how the target entity is identified.
//
//   None           – no target.
//
//   Player         – the local "Player" singleton on each peer.
//                    Wire id = the owning peer/connection id (uint8_t clientId).
//                    On the owner and on the client whose id matches: resolve
//                    via Player::Instance.
//                    On other peers: resolve via RemotePlayer whose networkId
//                    client-field matches clientId.
//
//   NetworkEntity  – any NetworkedEntity other than a player.
//                    Wire id = networkId (uint64_t).
//                    Resolved via NetworkManager / Level lookup by networkId.
//
//   Entity         – non-networked level entity, identified by string entity Id.
//                    Only meaningful on the server/owner; not useful on clients
//                    because entity IDs are not replicated.
//                    Included for completeness (trigger volumes, scripted actors).
// ---------------------------------------------------------------------------
enum class TargetType : uint8_t
{
    None          = 0,
    Player        = 1,
    NetworkEntity = 2,
    Entity        = 3,
};

// ---------------------------------------------------------------------------
// NpcTarget – compact targeting descriptor.
//
// Contains everything needed to:
//   • identify the target uniquely over the network
//   • resolve it to an actual Entity* on any peer
//   • serialize / deserialize itself into a NetPacket
// ---------------------------------------------------------------------------
struct NpcTarget
{
    TargetType type      = TargetType::None;
    uint8_t    clientId  = 0;          // used when type == Player
    uint64_t   networkId = 0;          // used when type == NetworkEntity
    std::string entityId = {};         // used when type == Entity (owner-only)

    // ── Helpers ──────────────────────────────────────────────────────────

    bool IsValid() const { return type != TargetType::None; }

    void Clear()
    {
        type      = TargetType::None;
        clientId  = 0;
        networkId = 0;
        entityId  = {};
    }

    // Build a Player target from the connection id that owns that player.
    static NpcTarget FromPlayer(uint8_t ownerClientId)
    {
        NpcTarget t;
        t.type     = TargetType::Player;
        t.clientId = ownerClientId;
        return t;
    }

    // Build a NetworkEntity target from a NetworkedEntity's networkId.
    static NpcTarget FromNetworkEntity(uint64_t netId)
    {
        NpcTarget t;
        t.type      = TargetType::NetworkEntity;
        t.networkId = netId;
        return t;
    }

    // Build an Entity target from a string entity id (server/owner only).
    static NpcTarget FromEntity(const std::string& id)
    {
        NpcTarget t;
        t.type     = TargetType::Entity;
        t.entityId = id;
        return t;
    }

    // Equality – two descriptors refer to the same target iff both type and
    // the relevant id field match.
    bool operator==(const NpcTarget& o) const
    {
        if (type != o.type) return false;
        switch (type)
        {
            case TargetType::None:          return true;
            case TargetType::Player:        return clientId  == o.clientId;
            case TargetType::NetworkEntity: return networkId == o.networkId;
            case TargetType::Entity:        return entityId  == o.entityId;
        }
        return false;
    }
    bool operator!=(const NpcTarget& o) const { return !(*this == o); }

    // ── Wire serialization ────────────────────────────────────────────────

    void Write(NetPacket& packet) const
    {
        packet.WriteUInt8(static_cast<uint8_t>(type));
        switch (type)
        {
            case TargetType::Player:
                packet.WriteUInt8(clientId);
                break;
            case TargetType::NetworkEntity:
                packet.WriteUInt64(networkId);
                break;
            case TargetType::Entity:
                packet.WriteString(entityId);
                break;
            default:
                break;
        }
    }

    static NpcTarget Read(NetPacket& packet)
    {
        NpcTarget t;
        t.type = static_cast<TargetType>(packet.ReadUInt8());
        switch (t.type)
        {
            case TargetType::Player:
                t.clientId = packet.ReadUInt8();
                break;
            case TargetType::NetworkEntity:
                t.networkId = packet.ReadUInt64();
                break;
            case TargetType::Entity:
                t.entityId = packet.ReadString();
                break;
            default:
                break;
        }
        return t;
    }
};


class NpcHumanBase : public NetworkedEntity, public IEnemy
{
protected:

    // ── Character movement ────────────────────────────────────────────────
    CharacterController controller;

    vec3 desiredDirection = vec3();
    vec3 movingDirection  = vec3();

    PathFollowQuery pathFollow;

    SoundPlayer* soundPlayer = nullptr;

    float maxSpeed = 5.5f;
    float speed    = 4.0f;

    // ── State ─────────────────────────────────────────────────────────────
    // Single enum replaces the scattered bool flags (dead, stuned,
    // stunnedRagdoll, returningFromRagdoll, fleeing, attacking).
    // Substates that coexist (e.g. stunnedRagdoll while Stunned) are kept
    // as separate bools below because they represent a sub-phase of Stunned,
    // not a top-level state.
    NpcState state = NpcState::Idle;

    // Convenience helpers – read-only accessors keep call-sites readable.
    bool IsDead()              const { return state == NpcState::Dead;      }
    bool IsStunned()           const { return state == NpcState::Stunned;   }
    bool IsAttacking()         const { return state == NpcState::Attacking; }
    bool IsFleeing()           const { return state == NpcState::Fleeing;   }
    bool IsIdle()              const { return state == NpcState::Idle;      }
    bool IsChasing()           const { return state == NpcState::Chasing;   }

    // Sub-states of Stunned
    bool stunnedRagdoll        = false;
    bool returningFromRagdoll  = false;

    AnimationState animationStateSaveData;
    bool canBeStunRagdolled = true;

    Delay inAttackDelay;
    Delay afterAttackDelay;

    UiBilboard* statusWidget = nullptr;

    SkeletalMesh* mesh = nullptr;

    Delay stunnedRagdollDelay;

    Animation* getFromRagdollAnimation = nullptr;
    AnimationPose ragdollPose;
    AnimationState getFromRagdollAnimationSaveState;

    vec3  ragdollPelvisWorldPos = vec3();
    float pelvisBlendTimer      = 0.0f;

    std::shared_ptr<Observer> observer;

    // ── Targeting ─────────────────────────────────────────────────────────
    // currentTarget is the authoritative descriptor, synced via NetSerialize
    // and the SetTarget RPC.
    NpcTarget currentTarget;

    // Resolved entity pointer – rebuilt locally every frame from currentTarget.
    // Never serialized; always derived.
    Entity* resolvedTarget = nullptr;

    // Minimum seconds the owner must wait before switching to a different target.
    // Synced in NetSerialize so clients can display the correct target highlight.
    static constexpr float TARGET_SWITCH_COOLDOWN = 2.0f;
    float targetSwitchTimer = 0.0f;   // counts down; 0 means we may switch

    // Resolve currentTarget → resolvedTarget on all peers.
    // Call once per frame before any logic that reads resolvedTarget.
    void ResolveTarget();

    // Owner-only: scan perception results, pick the closest valid player,
    // and call TrySetTarget if it should replace the current one.
    void UpdatePerception();

    // Owner-only: given a candidate NpcTarget, switch only if the cooldown
    // has elapsed AND the candidate is strictly closer than the current target.
    void TrySetTarget(const NpcTarget& candidate, const vec3& candidatePos);

    // Owner-only: unconditionally apply a new target, reset the switch timer,
    // and broadcast the change to all peers via SetTarget RPC.
    void ApplyTarget(const NpcTarget& newTarget);

    // Called on non-owning peers when the SetTarget RPC arrives.
    void Net_ApplySetTarget(const NpcTarget& newTarget);

    // Flee helpers
    void UpdateFleeTarget();

    // ── Sound ─────────────────────────────────────────────────────────────
    void SetupSoundPlayer(SoundPlayer* sp);
    void PlaySoundEffect(std::string eventName);

    // ── Animation ─────────────────────────────────────────────────────────
    virtual void ProcessAnimationEvent(AnimationEvent& event) {}

    virtual void Attack() = 0;

    // ── Damage / death ────────────────────────────────────────────────────
    void Death();

    void OnPointDamage(float Damage, vec3 Point, vec3 Direction,
                       string bone = "", Entity* DamageCauser = nullptr,
                       Entity* Weapon = nullptr);

    void OnDamage(float Damage, Entity* DamageCauser = nullptr,
                  Entity* Weapon = nullptr);

    // ── Ragdoll recovery ──────────────────────────────────────────────────
    void StartStunnedRagdoll();
    void UpdateStunnedReturn();
    void StartReturnFromRagdoll();
    void UpdateReturnFromRagdoll();

    // ── Overridable update ────────────────────────────────────────────────
    virtual void AsyncUpdate() = 0;

    // ── Save-game serialization ───────────────────────────────────────────
    void Serialize(json& target);
    void Deserialize(json& source);

    // ── UI ────────────────────────────────────────────────────────────────
    void UpdateStatusWidgets();
    void UpdateDebugUI();

    // ── Asset loading ─────────────────────────────────────────────────────
    void LoadAssets();

    // ── Entity wiring ─────────────────────────────────────────────────────
    void FromData(EntityData data);
    void OnAction(std::string action);

    // ── Network replication ───────────────────────────────────────────────
    // Snapshot – called every network tick on the owning peer.
    void NetSerialize(NetPacket& packet) override;
    void NetDeserialize(NetPacket& packet) override;

    // RPC dispatch.
    void OnRPC(uint8_t rpcId, NetPacket& args) override;

    // RPC handlers called on non-owning peers.
    void Net_ApplyDamage(float damage);
    void Net_ApplyDeath();
    void Net_ApplyAttack();
    void Net_ApplyStun();

public:
    NpcHumanBase();
    ~NpcHumanBase();

    void Start()    override;
    void Stun()     override;
    void Destroy()  override;
};
