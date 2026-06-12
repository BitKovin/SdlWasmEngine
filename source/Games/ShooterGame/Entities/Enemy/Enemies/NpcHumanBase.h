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
// RPC IDs shared between NpcHumanBase and its subclasses.
// Keep these stable – they go on the wire.
// ---------------------------------------------------------------------------
namespace NpcRPC {
    constexpr uint8_t TakeDamage = 0; // float damage
    constexpr uint8_t Death      = 1; // no args
    constexpr uint8_t Attack     = 2; // no args  (broadcast so all clients play anim)
    constexpr uint8_t Stun       = 3; // no args
}

class NpcHumanBase : public NetworkedEntity, public IEnemy
{
protected:

	CharacterController controller;

    vec3 desiredDirection = vec3();
    vec3 movingDirection = vec3();

    PathFollowQuery pathFollow;

    SoundPlayer* soundPlayer = nullptr;

    float maxSpeed = 5.5f;
    float speed = 4;

    bool dead = false;

    bool fleeing = false;
    Delay fleeSearchDelay = Delay();

    AnimationState animationStateSaveData;

	bool canBeStunRagdolled = true;

    Delay inAttackDelay;
    Delay afterAttackDelay;

    bool stuned = false;

    UiBilboard* statusWidget = nullptr;

    SkeletalMesh* mesh = nullptr;

    bool stunnedRagdoll = false;
    Delay stunnedRagdollDelay;
    bool returningFromRagdoll = false;

    Animation* getFromRagdollAnimation = nullptr;
    AnimationPose ragdollPose;
    AnimationState getFromRagdollAnimationSaveState;

    vec3 ragdollPelvisWorldPos = vec3();
    float pelvisBlendTimer = 0.0f;

    std::shared_ptr<Observer> observer;

    Entity* target = nullptr;

    void UpdateFleeTarget();

    void UpdatePerception();

    void SetupSoundPlayer(SoundPlayer* soundPlayer);

    void PlaySoundEffect(std::string eventName);

    virtual void ProcessAnimationEvent(AnimationEvent& event) {};

    virtual void Attack() = 0;

    void Death();

    void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

    void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);

    void StartStunnedRagdoll();
    void UpdateStunnedReturn();

    void StartReturnFromRagdoll();
    void UpdateReturnFromRagdoll();

    virtual void AsyncUpdate() = 0;

    void Serialize(json& target);

    void Deserialize(json& source);

    void UpdateStatusWidgets();

    void UpdateDebugUI();

    void LoadAssets();

    void FromData(EntityData data);

    void SetTarget(Entity* newTarget);

    void OnAction(std::string action);

    // ── Replication ──────────────────────────────────────────────────────
    // Snapshot sent every network tick from the owning peer.
    void NetSerialize(NetPacket& packet) override;
    void NetDeserialize(NetPacket& packet) override;

    // Incoming RPC dispatch.
    void OnRPC(uint8_t rpcId, NetPacket& args) override;

    // Helpers called on non-owning peers when an RPC arrives.
    void Net_ApplyDamage(float damage);
    void Net_ApplyDeath();
    void Net_ApplyAttack();
    void Net_ApplyStun();

public:
    NpcHumanBase();

    ~NpcHumanBase();

    void Start() override;

    void Stun() override;

    void Destroy() override;
};
