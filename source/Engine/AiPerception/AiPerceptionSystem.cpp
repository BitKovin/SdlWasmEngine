#include "AiPerceptionSystem.h"
#include "Observer.h"
#include "ObservationTarget.h"
#include <algorithm>
#include <utility>
#include "../EngineMain.h"
#include <tracy/tracy/Tracy.hpp>
#include <Level.hpp>

std::vector<std::shared_ptr<Observer>> AiPerceptionSystem::observers;
std::vector<std::shared_ptr<ObservationTarget>> AiPerceptionSystem::targets;

std::shared_ptr<Observer> AiPerceptionSystem::CreateObserver(const glm::vec3& position, const glm::vec3& forward, float fovDeg)
{
    auto observer = std::make_shared<Observer>(position, forward, fovDeg);
    observer->id = nextId;
    nextId++;
    observers.push_back(observer);
    return observer;
}

std::shared_ptr<ObservationTarget> AiPerceptionSystem::CreateTarget(const glm::vec3& position, const std::string& ownerId, const std::vector<std::string>& tags)
{
    auto target = std::make_shared<ObservationTarget>(position, tags);
    target->ownerId = ownerId;
    targets.push_back(target);
    return target;
}

void AiPerceptionSystem::RemoveObserver(const std::shared_ptr<Observer>& observer)
{
    if (observer == nullptr) return;
    observer->ownerPtr = nullptr;

    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void AiPerceptionSystem::RemoveTarget(const std::shared_ptr<ObservationTarget>& target)
{
    if (target == nullptr) return;
    targets.erase(std::remove(targets.begin(), targets.end(), target), targets.end());
}

void AiPerceptionSystem::RemoveAll()
{
    observers.clear();
    targets.clear();
}

void AiPerceptionSystem::Update()
{
    const int frameDistrib = 4;

    std::vector<std::function<void()>> updateJobs;
    updateJobs.reserve(observers.size() / 3); // Pre-allocate for efficiency

    for (auto& observer : observers)
    {
        // Promote sounds emitted last frame into heardSounds so AI can read them this frame,
        // then clear the staging buffer ready for new EmitSoundAt() calls.
        observer->heardSounds = std::move(observer->pendingSounds);
        observer->pendingSounds.clear();

        if (observer->id % frameDistrib == EngineMain::MainInstance->frame % frameDistrib)
        {

            updateJobs.emplace_back([observer]()
                {
                    ZoneScoped;
                    std::string zoneName = "Observer " + observer->owner.str();
                    ZoneName(zoneName.c_str(), zoneName.size());
                    observer->UpdateVisibility(targets);
				});
        }
    }

    if (!updateJobs.empty()) {
        Level::Current->asyncUpdateThreadPool->QueueJobs(std::move(updateJobs));
    }

    Level::Current->asyncUpdateThreadPool->WaitForFinish();

}

void AiPerceptionSystem::EmitSoundAt(const glm::vec3& position, float radius, int severity, std::string causerId)
{
    if (observers.empty() || radius <= 0.0f)
        return;

    const float radiusSq = radius * radius;

    for (auto& observer : observers)
    {
        const glm::vec3 delta = observer->position - position;
        const float distSq = glm::dot(delta, delta);

        if (distSq <= radiusSq)
        {
            observer->pendingSounds.push_back({ position, radius, severity, causerId });
        }
    }
}

std::vector<std::shared_ptr<Observer>> AiPerceptionSystem::GetObserversInRadius(const glm::vec3& position, float radius)
{
    std::vector<std::shared_ptr<Observer>> result;
    if (observers.empty() || radius <= 0.0f)
        return result;

    float radiusSq = radius * radius;

    for (auto& obs : observers)
    {
        float distSq = glm::length2(obs->position - position);
        if (distSq <= radiusSq)
        {
            result.push_back(obs);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ObservationTarget>>
AiPerceptionSystem::GetTargetsInRadiusWithTagOrdered(const glm::vec3& position, float radius, const std::string& tag)
{
    std::vector<std::shared_ptr<ObservationTarget>> result;

    if (targets.empty() || radius <= 0.0f)
        return result;

    const float radiusSq = radius * radius;

    // Collect candidate pairs (distSq, index) to avoid bumping shared_ptr refcounts
    std::vector<std::pair<float, size_t>> candidates;
    candidates.reserve(targets.size());

    for (size_t i = 0; i < targets.size(); ++i)
    {
        const auto& t = targets[i];
        if (!t) continue;                         // defensive: skip null shared_ptr
        if (!t->HasTag(tag)) continue;           // tag test (O(1) average, unordered_set)

        const glm::vec3 delta = t->position - position;
        const float distSq = glm::dot(delta, delta); // same as glm::length2(delta)

        if (distSq <= radiusSq)
            candidates.emplace_back(distSq, i);
    }

    if (candidates.empty())
        return result;

    // Sort candidates by distance (ascending)
    std::sort(candidates.begin(), candidates.end(),
        [](const std::pair<float, size_t>& a, const std::pair<float, size_t>& b) {
            return a.first < b.first;
        });

    // Build result copying shared_ptrs only for selected (and ordered) targets
    result.reserve(candidates.size());
    for (const auto& p : candidates)
        result.push_back(targets[p.second]);

    return result;
}