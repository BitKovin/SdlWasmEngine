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

#include <Network/NetworkedEntity.h>
#include <EntityHandle.h>

// ---------------------------------------------------------------------------
// NpcRPC
// ---------------------------------------------------------------------------
enum class NpcRPC : uint8_t
{
    TakeDamage        = 0,  // float damage, string bone, vec3 point, vec3 dir, bool hasCauser
    Death             = 1,  // (none)
    Attack            = 2,  // (none)
    Stun              = 3,  // (none)
    StunRagdoll       = 4,  // (none)
    ReturnFromRagdoll = 5,  // (none)
    SetTarget         = 6,  // EntityHandle
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
// NpcHumanBase
// ---------------------------------------------------------------------------
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

    bool pendingNetDeath = false;

	bool canHear = true;

    uint32_t numUpdates = 0;

    std::string damageSoundPath = "event:/NPC/Enemy1/Enemy1Damage";
    std::string deathSoundPath = "event:/NPC/Enemy1/Enemy1Death";
    std::string stunSoundPath = "event:/NPC/Enemy1/Enemy1Stun";

    // ── State ─────────────────────────────────────────────────────────────
    NpcState state = NpcState::Idle;

    bool IsDead()      const { return state == NpcState::Dead;      }
    bool IsStunned()   const { return state == NpcState::Stunned;   }
    bool IsAttacking() const { return state == NpcState::Attacking; }
    bool IsFleeing()   const { return state == NpcState::Fleeing;   }
    bool IsIdle()      const { return state == NpcState::Idle;      }
    bool IsChasing()   const { return state == NpcState::Chasing;   }

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
    // currentTarget is the wire-stable descriptor, synced via snapshot + RPC.
    // resolvedTarget is rebuilt locally every frame – never serialised.
    EntityHandle currentTarget;
    Entity*      resolvedTarget = nullptr;

    static constexpr float TARGET_SWITCH_COOLDOWN = 2.0f;
    float targetSwitchTimer = 0.0f;

    // Rebuild resolvedTarget from currentTarget on every peer.
    void ResolveTarget();

    // Owner-only: scan perception, pick closest, call TrySetTarget.
    void UpdatePerception();

    // Owner-only: conditionally switch to a new target.
    void TrySetTarget(const EntityHandle& candidate, const vec3& candidatePos);

    // Owner-only: commit new target, broadcast RPC, transfer ownership.
    void ApplyTarget(const EntityHandle& newTarget);

    Delay fleeSearchDelay;
    void  UpdateFleeTarget();

    // ── Animation ─────────────────────────────────────────────────────────
    virtual void ProcessAnimationEvent(AnimationEvent& event) {}
    virtual void Attack() = 0;
    void UpdateNonOwnerAnimation();

    // ── Sound ─────────────────────────────────────────────────────────────
    void SetupSoundPlayer(SoundPlayer* sp);
    void PlaySoundEffect(std::string eventName);

    // ── Damage / death – run on every peer ────────────────────────────────
    void OnPointDamage(float Damage, vec3 Point, vec3 Direction,
                       string bone = "", Entity* DamageCauser = nullptr,
                       Entity* Weapon = nullptr);
    void OnDamage(float Damage, Entity* DamageCauser = nullptr,
                  Entity* Weapon = nullptr);
    void Death();

    // ── Ragdoll – run on every peer ───────────────────────────────────────
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
    void NetSerialize(NetPacket& packet)       override;
    void NetDeserialize(NetPacket& packet)     override;
    void OnRPC(uint8_t rpcId, NetPacket& args) override;

    void LateUpdate() override;

public:
    NpcHumanBase();
    ~NpcHumanBase();

    void Start()   override;
    void Stun()    override;
    void Destroy() override;
};
