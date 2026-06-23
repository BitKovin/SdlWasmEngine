#include "PlayerRepresentation.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// ─── Lifecycle ────────────────────────────────────────────────────────────────

PlayerRepresentation::PlayerRepresentation()
{
    ClassName = "playerRepresentation";
    Tags = { "player" };

    // Weapon meshes are created up-front so LoadAssets() can configure their
    // hide lists before any model is loaded.
    weaponR = new SkeletalMesh(this);
    Drawables.push_back(weaponR);

    weaponL = new SkeletalMesh(this);
    Drawables.push_back(weaponL);
}

PlayerRepresentation::~PlayerRepresentation()
{
    // SkeletalMesh / Animation pointers in Drawables are deleted by
    // Entity::~Entity → DestroyDrawables(). Only animator is manual.
    delete animator;
}

void PlayerRepresentation::LoadAssets()
{
    mesh = new SkeletalMesh(this);
    Drawables.push_back(mesh);

    // Static hide lists: each weapon mesh hides the half belonging to the
    // other hand so they don't bleed through. Akimbo overrides this at runtime.
    weaponR->MeshHideList = { "w_l" };
    weaponL->MeshHideList = { "w_r" };
    weaponL->TwoSided = true;
    weaponR->TwoSided = true;
    weaponR->GravityAlignedRotation = true;
    weaponL->GravityAlignedRotation = true;

    // Weapon animation clip — kept in Drawables for lifetime management only;
    // it is never submitted to the renderer directly.
    weaponAnimation = new Animation(this);
    weaponAnimation->LoadFromFile("GameData/animations/player/tp_weapons.glb");
    Drawables.push_back(weaponAnimation);
    weaponAnimation->PlayAnimation("weapon_rl", true, 0);

    mesh->LoadFromFile("GameData/models/player/body/player_body.glb");
    mesh->GravityAlignedRotation = true;
    mesh->DepthPrePath = false;
    mesh->Masked = true;
    mesh->PreloadAssets();
    mesh->TwoSided = true;

    animator = new PlayerBodyAnimator(this);
    animator->LoadAssetsIfNeeded();
}

// ─── State ────────────────────────────────────────────────────────────────────

void PlayerRepresentation::ApplyState(const PlayerState& state)
{
    currentState = state;

    // A planar reflection has a negative determinant; a pure rotation or
    // translation (identity, portal) doesn't. Cached here because both
    // GetTransformedRotation() and Update() need to agree on it, and
    // transformModifier is expected to already be final by the time
    // ApplyState() runs (callers set it just before calling this).
    isMirrored = glm::determinant(glm::mat3(transformModifier)) < 0.0f;

    // Update the entity's world position/rotation so the level system
    // (AI queries, culling, etc.) sees the post-transform location.
    Position = GetTransformedPosition();
    Rotation = GetTransformedRotation();
}

// ─── Update ───────────────────────────────────────────────────────────────────

void PlayerRepresentation::Update()
{
    if (!mesh) return;

    // Offset the mesh origin downward by half the capsule height so the
    // skeleton root ends up at ground level.
    mesh->Position = Position - vec3(0, currentState.playerHeight / 2.0f, 0);
    mesh->Rotation = Rotation;

    // Flips the geometry's handedness so it visually mirrors rather than
    // just facing a mirrored direction. Assumes the rig's left/right
    // symmetry axis is local X -- adjust the component here if your model
    // uses a different axis.
    mesh->Scale = isMirrored ? vec3(-1, 1, 1) : vec3(1, 1, 1);
}

void PlayerRepresentation::AsyncUpdate()
{
    // Skip all GPU-facing work if nothing is on screen or assets aren't ready.
    if (!Visible || !animator) return;

    UpdateWeaponMeshes();

    animator->movementSpeed = length(MathHelper::XZ(currentState.velocity));
    animator->Update();

    AnimationPose pose = animator->GetResultPose();
    pose = ApplyWeaponAnimation(pose);

    mesh->PasteAnimationPose(pose);

    if (weaponR->Visible)
        weaponR->PasteAnimationPose(pose);
    if (weaponL->Visible)
        weaponL->PasteAnimationPose(pose);

    // Weapon meshes share the body mesh's world transform.
    weaponR->Position = weaponL->Position = mesh->Position;
    weaponR->Rotation = weaponL->Rotation = mesh->Rotation;
    weaponR->Scale = weaponL->Scale = mesh->Scale;
}

// ─── Transform modifier ───────────────────────────────────────────────────────

vec3 PlayerRepresentation::GetTransformedPosition() const
{
    return vec3(transformModifier * glm::vec4(currentState.position, 1.0f));
}

vec3 PlayerRepresentation::GetTransformedRotation() const
{
    // Combine the modifier's rotation with the player's current Euler rotation.
    // For the identity case this degenerates to the player's rotation, but we
    // always go through the matrix path so mirrors / portals work correctly.
    glm::mat3 playerRotMat = glm::mat3(MathHelper::GetRotationMatrix(currentState.rotation));
    glm::mat3 modRotMat = glm::mat3(transformModifier);
    glm::mat3 combined;

    if (isMirrored)
    {
        // modRotMat includes an odd number of reflections, so naively
        // multiplying gives a det=-1 matrix that glm::quat_cast cannot
        // represent correctly (it assumes a proper rotation). Instead,
        // reflect two of the player's basis vectors through the mirror and
        // re-derive the third with a cross product -- that keeps the result
        // a proper, quat_cast-safe rotation. The actual handedness flip
        // (what makes the mesh *look* mirrored, not just turned around) is
        // applied separately via Scale in Update()/AsyncUpdate().
        glm::vec3 up = modRotMat * playerRotMat[1];
        glm::vec3 forward = modRotMat * playerRotMat[2];
        glm::vec3 right = glm::cross(up, forward);

        combined = glm::mat3(right, up, forward);
    }
    else
    {
        combined = modRotMat * playerRotMat;
    }

    return MathHelper::ToYawPitchRoll(glm::quat_cast(combined));
}

// ─── Animation ────────────────────────────────────────────────────────────────

AnimationPose PlayerRepresentation::ApplyWeaponAnimation(AnimationPose pose)
{
    const bool hasR = !currentState.weaponRModelPath.empty();
    const bool hasL = !currentState.weaponLModelPath.empty();

    if (!hasR && !hasL)
        return pose;

    // Layer the weapon animation onto the upper spine so legs remain
    // driven by the locomotion animator.
    AnimationPose outPose = AnimationPose::LayeredLerp(
        "spine_03",
        weaponAnimation->GetRootNode(),
        pose,
        weaponAnimation->GetAnimationPose(),
        1, 1);

    // Tilt the mid-spine to follow camera pitch, giving the impression the
    // character is aiming at the crosshair.
    outPose.boneTransforms["spine_02"] = outPose.boneTransforms["spine_02"]
        * MathHelper::GetRotationMatrix(
            vec3(0, 0, currentState.cameraRotation.x * -0.75f));

    return outPose;
}

// ─── Weapon meshes ────────────────────────────────────────────────────────────

void PlayerRepresentation::UpdateWeaponMeshes()
{
    // Reload only when the path actually changes, not every frame.
    if (weaponR->filePath != currentState.weaponRModelPath)
    {
        weaponR->LoadFromFile(currentState.weaponRModelPath);
        weaponR->TexturesLocation = currentState.weaponRModelPath + "/";
    }

    if (weaponL->filePath != currentState.weaponLModelPath)
    {
        weaponL->LoadFromFile(currentState.weaponLModelPath);
        weaponL->TexturesLocation = currentState.weaponLModelPath + "/";
        // Left-hand slot always hides the right-weapon geometry.
        weaponL->MeshHideList = { "w_r" };
    }

    // Akimbo shows both mesh halves on the right-hand weapon; otherwise the
    // left half ("w_l") stays hidden to avoid T-posing the second grip.
    if (currentState.weaponRAkimbo)
    {
        weaponR->MeshHideList.clear();
    }
    else if (weaponR->MeshHideList.empty() || weaponR->MeshHideList.count("w_l") == 0)
    {
        weaponR->MeshHideList = { "w_l" };
    }

    weaponR->Visible = !currentState.weaponRModelPath.empty();
    weaponL->Visible = !currentState.weaponLModelPath.empty();
}