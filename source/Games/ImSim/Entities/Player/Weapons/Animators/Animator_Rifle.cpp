#include "Animator_Rifle.h"

void Animator_Rifle::LoadAssets()
{

    idleAnimation = AddAnimation("GameData/models/player/weapons/mpsd/mpsd_tp.glb", "idle");
    aimAnimation = AddAnimation("GameData/models/player/weapons/mpsd/mpsd_tp.glb", "aim");

}

AnimationPose Animator_Rifle::ProcessResultPose()
{
    return CalculateWeaponPose();
}

AnimationPose Animator_Rifle::CalculateWeaponPose()
{

    auto aimPose = aimAnimation->GetAnimationPose();

    mat4 rightHandMat = AnimationPose::GetModelSpaceTransform("weapon_r", aimAnimation->GetRootNode(), aimPose);
    mat4 leftHandMat = AnimationPose::GetModelSpaceTransform("hand_l", aimAnimation->GetRootNode(), aimPose);
    //leftHandMat = translate(vec3(0,0,0.1f)) * leftHandMat;
    mat4 relativeLeftHandMat = inverse(rightHandMat) * leftHandMat;

    auto resultPose = AnimationPose::Lerp(CalculateIdlePose(), CalculateAimPose(), weaponAim);

    rightHandMat = AnimationPose::GetModelSpaceTransform("weapon_r", aimAnimation->GetRootNode(), resultPose);
    leftHandMat = rightHandMat * relativeLeftHandMat;
    MathHelper::Transform leftHandTransform = MathHelper::DecomposeMatrix(leftHandMat);

    resultPose = AnimationPose::ApplyFABRIK(
        "upperarm_l",
        "hand_l",
        aimAnimation->GetRootNode(),
        resultPose,
        leftHandTransform.Position,
        leftHandTransform.RotationQuaternion
    );

    return resultPose;
}

AnimationPose Animator_Rifle::CalculateIdlePose()
{

    auto weaponPose = idleAnimation->GetAnimationPose();

    AnimationPose appliedWeaponPose = AnimationPose::LayeredLerp(
        "spine_01",
        aimAnimation->GetRootNode(),
        inPose,
        weaponPose,
        1.0,
        1.0f
    );

    

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.3,
        0.9f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.3,
        0.9f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "hand_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.0,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "hand_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        0.0,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "head",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        1.0,
        1.0f
    );

    return appliedWeaponPose;
}

AnimationPose Animator_Rifle::CalculateAimPose()
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
        0.7f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_l",
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
        weaponPoseBase,
        modelSpaceBlending,
        1.0f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        0.3f
    );

    appliedWeaponPose = AnimationPose::LayeredLerp(
        "clavicle_r",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        0.3f
    );


    appliedWeaponPose = AnimationPose::LayeredLerp(
        "upperarm_l",
        aimAnimation->GetRootNode(),
        appliedWeaponPose,
        weaponPose,
        modelSpaceBlending,
        0.85f
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
        weaponPose,
        modelSpaceBlending,
        0.5f
    );

    return appliedWeaponPose;

}
