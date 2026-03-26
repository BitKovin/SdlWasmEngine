#pragma once
#include "../glm.h"
#include <vector>
#include <memory>
#include <string>
#include "../utility/hashed_string.hpp"

class Entity;

class ObservationTarget;

struct HeardSound
{
    glm::vec3   position;   // World position where the sound was emitted
    float       radius;     // Radius the sound was emitted with
    int         severity;   // AI logic value: higher = more alarming
    std::string causerId;
};

class Observer
{
public:
    glm::vec3 position;
    glm::vec3 forward;
    float fovDeg;
    float maxDistance = 60;

    hashed_string owner;

    Entity* ownerPtr = nullptr;

    bool searchForTriggeredNpc = true;

    int id = 0;

    std::vector<std::shared_ptr<ObservationTarget>> visibleTargets;

    // Sounds readable by AI this frame. Populated at the start of Update() from
    // pendingSounds, so every entry was emitted during the *previous* frame.
    std::vector<HeardSound> heardSounds;

    // Staging buffer filled by AiPerceptionSystem::EmitSoundAt(). Promoted to
    // heardSounds at the start of the next Update() call — do not read directly.
    std::vector<HeardSound> pendingSounds;

    Observer(const glm::vec3& pos, const glm::vec3& fwd, float fov)
        : position(pos), forward(glm::normalize(fwd)), fovDeg(fov) {}

    void UpdateVisibility(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets);
};