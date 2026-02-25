// NpcHumanBase.h
#pragma once

#include <Entity.h>
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
#include <Animation.h> // Assuming header for Animation

class NpcHumanBase : public Entity, public IEnemy
{
protected:
    vec3 desiredDirection = vec3();
    vec3 movingDirection = vec3();

    PathFollowQuery pathFollow;

    SoundPlayer* soundPlayer = nullptr;

    float maxSpeed = 5.5f; // Average between gun and axe
    float speed = 4;

    bool dead = false;

    bool fleeing = false;
    Delay fleeSearchDelay = Delay();

    AnimationState animationStateSaveData;

    Delay inAttackDelay;

    bool stuned = false;

    UiBilboard* statusWidget = nullptr;

    SkeletalMesh* mesh = nullptr;

    bool stunnedRagdoll = false;
    Delay stunnedRagdollDelay;
    bool returningFromRagdoll = false;

    Animation* getFromRagdollAnimation = nullptr;
    AnimationPose ragdollPose;
    AnimationState getFromRagdollAnimationSaveState;

    void UpdateFleeTarget();

    void SetupSoundPlayer(SoundPlayer* soundPlayer);

    void PlaySoundEffect(std::string eventName);

    virtual void ProcessAnimationEvent(AnimationEvent& event) = 0;

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

public:
    NpcHumanBase();

    ~NpcHumanBase();

    void Start() override;

    void Stun() override;

    void Destroy() override;
};
