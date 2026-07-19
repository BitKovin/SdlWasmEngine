#pragma once

#include "glm.h"
#include <string>

class Player;

// Snapshot of everything PlayerRepresentation needs to render a player.
// Intentionally contains model paths rather than weapon indices so the
// representation layer stays completely decoupled from the weapon registry.
struct PlayerState
{
    vec3  position{};
    vec3  rotation{};
    vec3  cameraRotation{};
    vec3  velocity{};

    bool crouching = false;

    float playerHeight  = 0.0f;

    uint8_t weaponRHandlingType = 0; //0 default with left hand holding, 1 for left hand resting, 2 for 2 separate hands holding 2 copies of weapon

    // Resolved third-person model paths. Empty string == no weapon equipped.
    std::string weaponRModelPath;
    std::string weaponLModelPath;

    // Build a PlayerState directly from a locally-controlled Player object.
    // Resolves model paths straight from the weapon pointers, so no registry
    // look-up is needed.
    static PlayerState FromPlayerPtr(Player* player);
};
