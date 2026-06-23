#pragma once

#include "Entity.h"
#include "PlayerBodyAnimator.h"
#include "PlayerState.h"
#include <SkeletalMesh.hpp>
#include <Animation.h>
#include <glm/glm.hpp>

// Non-networked entity that renders the third-person appearance of a player.
//
// Lifecycle
//   Spawned and destroyed by RemotePlayer. State is pushed every frame via
//   ApplyState() before Update() runs.
//
// Transform modifier
//   transformModifier is a 4x4 matrix applied to the incoming position and
//   rotation before anything is placed in the world. Defaults to identity
//   (player rendered at their actual location). Set it to a reflection or
//   portal matrix to implement mirrors, looping rooms, etc.
//
//   A mirror system can create its own PlayerRepresentation independently
//   and feed it state from the same source (RemotePlayer or a local Player*
//   via PlayerState::FromPlayerPtr), then set this matrix to the reflection
//   transform.
class PlayerRepresentation : public Entity
{
public:
    PlayerRepresentation();
    ~PlayerRepresentation();

    // Identity = no transform; set to reflection/portal matrix for mirrors etc.
    glm::mat4 transformModifier = glm::mat4(1.0f);

    // Push a new state snapshot. Call this every frame (or whenever state
    // changes) before Update() executes.
    void ApplyState(const PlayerState& state);

    void LoadAssets() override;
    void Update()     override;
    void AsyncUpdate() override;

private:
    PlayerState currentState;

    SkeletalMesh*       mesh           = nullptr;
    SkeletalMesh*       weaponR        = nullptr;
    SkeletalMesh*       weaponL        = nullptr;
    Animation*          weaponAnimation = nullptr;
    PlayerBodyAnimator* animator        = nullptr;

    // Apply transformModifier to the raw position / rotation from currentState.
    vec3 GetTransformedPosition() const;
    vec3 GetTransformedRotation() const;

    AnimationPose ApplyWeaponAnimation(AnimationPose pose);
    void          UpdateWeaponMeshes();
};
