#pragma once
#include "../glm.h"
#include <memory>
#include <vector>
#include <string>

class Observer;
class ObservationTarget;

class AiPerceptionSystem
{
public:
    static std::shared_ptr<Observer> CreateObserver(const glm::vec3& position, const glm::vec3& forward, float fovDeg);
    static std::shared_ptr<ObservationTarget> CreateTarget(const glm::vec3& position, const std::string& ownerId, const std::vector<std::string>& tags);

    static void RemoveObserver(const std::shared_ptr<Observer>& observer);
    static void RemoveTarget(const std::shared_ptr<ObservationTarget>& target);

    static void RemoveAll();

    static void Update(); // Main update loop — updates visibility for all observers

    static void EmitSoundAt(const glm::vec3& position, float radius, int severity, std::string causerId);

    static const std::vector<std::shared_ptr<Observer>>& GetObservers() { return observers; }
    static const std::vector<std::shared_ptr<ObservationTarget>>& GetTargets() { return targets; }

    static std::vector<std::shared_ptr<Observer>> GetObserversInRadius(const glm::vec3& position, float radius);

    static std::vector<std::shared_ptr<ObservationTarget>>
        GetTargetsInRadiusWithTagOrdered(const glm::vec3& position, float radius, const std::string& tag);

private:
    static std::vector<std::shared_ptr<Observer>> observers;
    static std::vector<std::shared_ptr<ObservationTarget>> targets;

    static inline int nextId = 0;

};
