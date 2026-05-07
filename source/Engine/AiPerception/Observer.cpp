#include "Observer.h"
#include "ObservationTarget.h"
#include "../glm.h"
#include <cmath>
#include "../Physics.h"
#include "../DebugDraw.hpp"

#include <Level.hpp>

void Observer::UpdateVisibility(const std::vector<std::shared_ptr<ObservationTarget>>& allTargets)
{
    visibleTargets.clear();

    for (auto& target : allTargets)
    {

        if (target->active == false) continue;

        if (searchForTriggeredNpc == false)
        {
            if (target->isTriggeredNpc && target->npc)
            {
                continue;
            }
        }

        glm::vec3 toTarget = target->position - position;

        float dist = glm::length(toTarget) / target->noticeMaxDistanceMultiplier;

        if (dist < 0.001f || dist > maxDistance)
            continue; // Skip if same position or too far

        toTarget = glm::normalize(toTarget);
        float angle = glm::degrees(glm::angle(forward, toTarget));

        if (angle <= fovDeg * 0.5f)
        {

			int observerCluster = Level::Current->BspData.FindClusterAtPosition(position);
			int targetCluster = Level::Current->BspData.FindClusterAtPosition(target->position);

            if (Level::Current->BspData.IsClusterVisible(observerCluster, targetCluster) == false) continue;

            auto hit = Physics::LineTrace(position, target->position, BodyType::WorldOpaque | BodyType::MainBody);

            if (hit.hasHit == false)
            {
                visibleTargets.push_back(target);
            }


        }
    }
}
