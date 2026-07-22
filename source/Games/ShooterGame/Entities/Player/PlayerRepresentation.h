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
//
// Mirrors specifically
//   A planar reflection matrix has a negative determinant (it flips
//   handedness), which a plain rotation can't represent and quat_cast can't
//   decompose correctly. ApplyState() detects this case (determinant < 0)
//   and:
//     - builds the rotation by reflecting two basis vectors and re-deriving
//       the third via cross product, so it stays a proper, quat_cast-safe
//       rotation;
//     - flips mesh->Scale.x (and the weapon meshes') to -1, which is what
//       actually makes the geometry itself look mirrored rather than just
//       rotated to face the wrong way.
//   This assumes the character rig's left/right symmetry axis is local X.
//   If your asset uses a different axis, change the flipped component in
//   Update() / AsyncUpdate() to match.
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

    void SetPlayerWalkPhase(float phase);
    float GetPlayerWalkPhase();

    bool OnlyShadows = false;

private:
    PlayerState currentState;

    // Cached each ApplyState() from transformModifier's determinant. True
    // when transformModifier is an odd reflection (a mirror), as opposed to
    // a pure rotation/translation (e.g. a portal).
    bool isMirrored = false;


    SkeletalMesh* mesh = nullptr;
    SkeletalMesh* weaponR = nullptr;
    SkeletalMesh* weaponL = nullptr;
    Animation* weaponAnimation = nullptr;
    PlayerBodyAnimator* animator = nullptr;

    // Apply transformModifier to the raw position / rotation from currentState.
    vec3 GetTransformedPosition() const;
    vec3 GetTransformedRotation() const;

    AnimationPose ApplyWeaponAnimation(AnimationPose pose);
    void          UpdateWeaponMeshes();
};