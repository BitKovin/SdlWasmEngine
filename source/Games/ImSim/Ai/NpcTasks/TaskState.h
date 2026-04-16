#pragma once

#include <string>
#include <glm.h>
#include <Helpers/JsonHelper.hpp>

struct TaskState
{
    std::string TaskName = "";

    bool AllowWeapon = true; //hide weapon when executing task

    bool HasToMoveToTarget = false;
    vec3 TargetLocation = vec3(); //target location for npc to move
    float AcceptanceRadius = 0.05f;

    bool CanBeCanceled = true; //means that NPC can't do any other job while moving or DoingJob==true. Can be used to finish not interruptible animations.

    bool DoingJob = false; //technical variable. Makes task execution always return "Running" in Behavior tree. Even When not walking.

    bool HasToLookAtTarget = false;
    vec3 TargetOrientation = vec3(); //target rotation that npc should take when reaching target

    std::string TaskStage = ""; //used to store stage of task execution (moving to target, waiting for another npc, doing job, etc)

    float Timer1 = 0; // user data
    float Timer2 = 0; // user data
    float Timer3 = 0; // user data

    std::string UserData = ""; // user data

    vec3 LockPosition = vec3();
    bool HasToLockPosition = false;
    vec3 StartPosition = vec3();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        TaskState,
        AllowWeapon,
        TaskName,
        HasToMoveToTarget,
        TargetLocation,
        AcceptanceRadius,
        CanBeCanceled,
        HasToLookAtTarget,
        TargetOrientation,
        TaskStage,
        Timer1,
        Timer2,
        Timer3,
        UserData,
        HasToLockPosition,
        LockPosition,
        StartPosition
    )
};
