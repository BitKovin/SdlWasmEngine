#include "Animator_OneHand.h"

void Animator_OneHand::LoadAssets()
{

    idleAnimation = AddAnimation("GameData/models/player/weapons/pistol/pistol_tp.glb", "idle");
    aimAnimation = AddAnimation("GameData/models/player/weapons/pistol/pistol_tp.glb", "aim");

}

AnimationPose Animator_OneHand::ProcessResultPose()
{
    return CalculateWeaponPose();
}

AnimationPose Animator_OneHand::CalculateWeaponPose()
{
    return AnimationPose::Lerp(CalculateIdlePose(), CalculateAimPose(), weaponAim);
}

AnimationPose Animator_OneHand::CalculateIdlePose()
{

    auto weaponPose = idleAnimation->GetAnimationPose();

    AnimationPose appliedWeaponPose = AnimationPose::LayeredLerp(
        "spine_02",
        aimAnimation->GetRootNode(),
        inPose,
        weaponPose,
        1,
        0.3f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "spine_03",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        1,
        1.0f
    );


    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.3,
        0.9f
    );

    appliedWeaponPose = AnimationPose::Lerp(inPose, appliedWeaponPose, 0.7f);

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        inPose,
        1.0,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.0,
        0.5f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "upperarm_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.1,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "hand_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.0,
        1.0f
    );


    return appliedWeaponPose;
}

AnimationPose Animator_OneHand::CalculateAimPose()
{

    auto weaponPose = aimAnimation->GetAnimationPose();
    auto weaponPoseBase = weaponPose;

    weaponPose.boneTransforms["pelvis"] = MathHelper::GetRotationMatrix(vec3(Camera::rotation.x - 10, 0, 0)) * weaponPose.boneTransforms["pelvis"]; //rotating pelvis. Causes whole model to

    float modelSpaceBlending = 1;

    AnimationPose appliedWeaponPose = AnimationPose::LayeredLerp(
        "spine_02",
        aimAnimation->GetRootNode(),
        inPose,
        weaponPose,
        modelSpaceBlending,
        0.3f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "spine_03",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        0.5f
    );


    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPoseBase,
        modelSpaceBlending,
        1.0f
    );


    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        0.75f
    );


    appliedWeaponPose = AnimationPose::LayeredLerp(
        "upperarm_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        0.85f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "hand_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "hand_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "head",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPoseBase,
        modelSpaceBlending,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "head",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        0.5f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        inPose,
        1.0,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        1.0,
        0.5f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "upperarm_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        1.0,
        0.5f
    );

    return appliedWeaponPose;

}
