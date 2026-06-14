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

#include "NetworkedEntity.h"

// ---------------------------------------------------------------------------
// NpcRPC – wire IDs for every broadcast event.
// Keep values stable; they go on the wire.
//
// Design: every peer that witnesses an event runs it locally and sends it to
// RPCTarget::Others so the remaining peers do the same.  No peer runs an
// event twice from both its own simulation and an incoming RPC.
// ---------------------------------------------------------------------------
enum class NpcRPC : uint8_t
{
    // args: float damage, string bone, vec3 point, vec3 direction
    // Carries enough data for every peer to run the full hit reaction.
    TakeDamage       = 0,

    // args: (none) – broadcast when health reaches 0 on any peer
    Death            = 1,

    // args: (none) – owner broadcasts after cooldown/range check passes
    Attack           = 2,

    // args: (none) – any peer that triggers a stun broadcasts to others
    Stun             = 3,

    // args: (none) – any peer that triggers ragdoll broadcasts to others
    StunRagdoll      = 4,

    // args: (none) – any peer that starts ragdoll recovery broadcasts to others
    ReturnFromRagdoll = 5,

    // args: NpcTarget serialised inline
    SetTarget        = 6,
};

// ---------------------------------------------------------------------------
// NpcState
// ---------------------------------------------------------------------------
enum class NpcState : uint8_t
{
    Idle      = 0,
    Chasing   = 1,
    Attacking = 2,
    Stunned   = 3,
    Fleeing   = 4,
    Dead      = 5,
};

// ---------------------------------------------------------------------------
// TargetType / NpcTarget
// ---------------------------------------------------------------------------
enum class TargetType : uint8_t
{
    None          = 0,
    Player        = 1,  // wire id = clientId (uint8_t)
    NetworkEntity = 2,  // wire id = networkId (uint64_t)
    Entity        = 3,  // wire id = entityId string (owner-only)
};

struct NpcTarget
{
    TargetType  type      = TargetType::None;
    uint8_t     clientId  = 0;
    uint64_t    networkId = 0;
    std::string entityId  = {};

    bool IsValid() const { return type != TargetType::None; }

    void Clear() { *this = NpcTarget{}; }

    static NpcTarget FromPlayer(uint8_t cid)
    { NpcTarget t; t.type = TargetType::Player; t.clientId = cid; return t; }

    static NpcTarget FromNetworkEntity(uint64_t netId)
    { NpcTarget t; t.type = TargetType::NetworkEntity; t.networkId = netId; return t; }

    static NpcTarget FromEntity(const std::string& id)
    { NpcTarget t; t.type = TargetType::Entity; t.entityId = id; return t; }

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

    void Write(NetPacket& packet) const
    {
        packet.WriteUInt8(static_cast<uint8_t>(type));
        switch (type)
        {
            case TargetType::Player:        packet.WriteUInt8(clientId);   break;
            case TargetType::NetworkEntity: packet.WriteUInt64(networkId); break;
            case TargetType::Entity:        packet.WriteString(entityId);  break;
            default: break;
        }
    }

    static NpcTarget Read(NetPacket& packet)
    {
        NpcTarget t;
        t.type = static_cast<TargetType>(packet.ReadUInt8());
        switch (t.type)
        {
            case TargetType::Player:        t.clientId  = packet.ReadUInt8();   break;
            case TargetType::NetworkEntity: t.networkId = packet.ReadUInt64();  break;
            case TargetType::Entity:        t.entityId  = packet.ReadString();  break;
            default: break;
        }
        return t;
    }
};


class NpcHumanBase : public NetworkedEntity, public IEnemy
{
protected:

    // ── Movement ──────────────────────────────────────────────────────────
    CharacterController controller;
    vec3  desiredDirection = vec3();
    vec3  movingDirection  = vec3();
    PathFollowQuery pathFollow;
    SoundPlayer*    soundPlayer = nullptr;
    float maxSpeed = 5.5f;
    float speed    = 4.0f;

    // ── State ─────────────────────────────────────────────────────────────
    NpcState state = NpcState::Idle;

    bool IsDead()      const { return state == NpcState::Dead;      }
    bool IsStunned()   const { return state == NpcState::Stunned;   }
    bool IsAttacking() const { return state == NpcState::Attacking; }
    bool IsFleeing()   const { return state == NpcState::Fleeing;   }
    bool IsIdle()      const { return state == NpcState::Idle;      }
    bool IsChasing()   const { return state == NpcState::Chasing;   }

    // Sub-states of Stunned
    bool stunnedRagdoll       = false;
    bool returningFromRagdoll = false;

    AnimationState animationStateSaveData;
    bool canBeStunRagdolled = true;

    Delay inAttackDelay;
    Delay afterAttackDelay;

    UiBilboard*  statusWidget = nullptr;
    SkeletalMesh* mesh        = nullptr;

    Delay stunnedRagdollDelay;

    Animation*     getFromRagdollAnimation           = nullptr;
    AnimationPose  ragdollPose;
    AnimationState getFromRagdollAnimationSaveState;
    vec3  ragdollPelvisWorldPos = vec3();
    float pelvisBlendTimer      = 0.0f;

    std::shared_ptr<Observer> observer;

    // ── Targeting ─────────────────────────────────────────────────────────
    NpcTarget currentTarget;
    Entity*   resolvedTarget  = nullptr;

    static constexpr float TARGET_SWITCH_COOLDOWN = 2.0f;
    float targetSwitchTimer = 0.0f;

    void ResolveTarget();
    void UpdatePerception();
    void TrySetTarget(const NpcTarget& candidate, const vec3& candidatePos);
    void ApplyTarget(const NpcTarget& newTarget);

    Delay fleeSearchDelay;
    void UpdateFleeTarget();

    // ── Animation ─────────────────────────────────────────────────────────
    // Called on ALL peers when animation events fire (no isOwned gate needed).
    virtual void ProcessAnimationEvent(AnimationEvent& event) {}

    virtual void Attack() = 0;

    // Mirror replicated state to animation on non-owning peers (idle/run).
    void UpdateNonOwnerAnimation();

    // ── Sound ─────────────────────────────────────────────────────────────
    void SetupSoundPlayer(SoundPlayer* sp);
    void PlaySoundEffect(std::string eventName);

    // ── Damage / death ─────────────────────────────────────────────────────
    // All run on every peer.  The peer that witnesses the event calls the
    // function directly, then sends the corresponding RPC to RPCTarget::Others.
    void OnPointDamage(float Damage, vec3 Point, vec3 Direction,
                       string bone = "", Entity* DamageCauser = nullptr,
                       Entity* Weapon = nullptr);

    void OnDamage(float Damage, Entity* DamageCauser = nullptr,
                  Entity* Weapon = nullptr);

    void Death();

    // ── Ragdoll ───────────────────────────────────────────────────────────
    // All run on every peer.
    void StartStunnedRagdoll();
    void UpdateStunnedReturn();
    void StartReturnFromRagdoll();
    void UpdateReturnFromRagdoll();

    // ── Update ────────────────────────────────────────────────────────────
    virtual void AsyncUpdate() = 0;

    // ── Serialization ─────────────────────────────────────────────────────
    void Serialize(json& target);
    void Deserialize(json& source);

    // ── UI ────────────────────────────────────────────────────────────────
    void UpdateStatusWidgets();
    void UpdateDebugUI();

    // ── Assets / wiring ───────────────────────────────────────────────────
    void LoadAssets();
    void FromData(EntityData data);
    void OnAction(std::string action);

    // ── Networking ────────────────────────────────────────────────────────
    void NetSerialize(NetPacket& packet)   override;
    void NetDeserialize(NetPacket& packet) override;
    void OnRPC(uint8_t rpcId, NetPacket& args) override;

public:
    NpcHumanBase();
    ~NpcHumanBase();

    void Start()   override;
    void Stun()    override;
    void Destroy() override;
};
