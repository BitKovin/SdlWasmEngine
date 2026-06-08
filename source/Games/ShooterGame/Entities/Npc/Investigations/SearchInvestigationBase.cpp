#include "SearchInvestigationBase.h"
#include "../NpcBase.h"

#include <glm.h>
#include <cstdlib>
#include <cmath>
#include <limits>

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

nlohmann::json SearchInvestigationBase::Serialize()
{
    nlohmann::json data = AlertedInvestigationBase::Serialize();

    data["searchPhase"]   = static_cast<int>(searchPhase);
    data["searchTimer"]   = searchTimer;
    data["waitTimer"]     = waitTimer;
    SERIALIZE_FIELD(data, searchOrigin);
    data["visitedAngles"] = visitedAngles;

    return data;
}

void SearchInvestigationBase::Deserialize(nlohmann::json& data)
{
    AlertedInvestigationBase::Deserialize(data);

    searchPhase   = static_cast<SearchPhase>(data["searchPhase"].get<int>());
    searchTimer   = data["searchTimer"].get<float>();
    waitTimer     = data["waitTimer"].get<float>();
    DESERIALIZE_FIELD(data, searchOrigin);
    visitedAngles = data["visitedAngles"].get<std::vector<float>>();
}

// ---------------------------------------------------------------------------
// Public overrides
// ---------------------------------------------------------------------------

void SearchInvestigationBase::Update(float deltaTime)
{
    // Run the base orient phase first — it must execute even while
    // searchPhase == Inactive so the NPC turns before it starts walking.
    InvestigationBase::Update(deltaTime);

    if (searchPhase == SearchPhase::Inactive)
        return;

    searchTimer += deltaTime;

    if (searchTimer >= SearchDuration())
    {
        Finish();
        return;
    }

    if (searchPhase == SearchPhase::Waiting)
    {
        waitTimer -= deltaTime;
        if (waitTimer <= 0.0f)
            AdvanceToNextPoint();
    }
}

void SearchInvestigationBase::ReachedTarget()
{

    if (owner->npcType != NpcType::Guard && owner->report_to_guard)
    {
        Finish();
    }

    if (searchPhase == SearchPhase::Inactive)
    {
        BeginSearch();
        return;
    }

    // Randomly pause at this waypoint to look around before moving on.
    float roll = static_cast<float>(rand()) / RAND_MAX;
    if (roll < WaitChance())
    {
        searchPhase = SearchPhase::Waiting;
        float range = MaxWaitTime() - MinWaitTime();
        waitTimer   = MinWaitTime() + (static_cast<float>(rand()) / RAND_MAX) * range;
    }
    else
    {
        AdvanceToNextPoint();
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void SearchInvestigationBase::BeginSearch()
{
    searchOrigin = target;
    searchTimer  = 0.0f;
    AdvanceToNextPoint();
}

void SearchInvestigationBase::AdvanceToNextPoint()
{
    if (!FindNextSearchPoint())
        Finish();
    else
        searchPhase = SearchPhase::Walking;
}

bool SearchInvestigationBase::FindNextSearchPoint()
{
    const float radius   = SearchRadius();
    const float twoPi    = 2.0f * glm::pi<float>();
    const float maxRatio = MaxPathLengthRatio();
    const float npcY     = owner->Position.y;

    bool  haveBest     = false;
    vec3  bestPoint{};
    float bestScore    = -std::numeric_limits<float>::infinity();

    bool  haveFallback = false;
    vec3  fallbackPoint{};

    for (int attempt = 0; attempt < MaxCandidateAttempts(); ++attempt)
    {
        // Spread attempts evenly around the circle, perturbed randomly.
        float baseAngle    = PickSearchAngle();
        float spread       = twoPi / static_cast<float>(MaxCandidateAttempts());
        float perturbation = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread;
        float angle        = baseAngle + perturbation
                             + static_cast<float>(attempt) * (twoPi / MaxCandidateAttempts());

        // Slight inner bias: more points closer to the origin, fewer at the edge.
        float distNorm = 0.6f + (static_cast<float>(rand()) / RAND_MAX) * 0.4f;
        float dist     = distNorm * radius;

        vec3 candidate = searchOrigin + vec3(
            std::cos(angle) * dist,
            0.0f,
            std::sin(angle) * dist);

        bool outReached = false;

        std::vector<vec3> path = NavigationSystem::FindSimplePath(
            owner->Position, candidate,
            /*acceptanceRadius=*/0.5f,
            &outReached,
            /*allowPartialPath=*/false);

        if (outReached && !path.empty())
        {
            float straightDist = glm::distance(owner->Position, candidate);
            float pathLen      = PathLength(path);

            if (straightDist < 0.001f || pathLen / straightDist > maxRatio)
                continue;

            // Score: angular coverage + directness + small random component.
            float angularCoverage = 1.0f;
            for (float visited : visitedAngles)
            {
                float diff = std::abs(angle - visited);
                while (diff >  glm::pi<float>()) diff -= twoPi;
                while (diff < -glm::pi<float>()) diff += twoPi;
                diff = std::abs(diff);
                angularCoverage -= std::exp(-diff * diff / (2.0f * 0.6f * 0.6f));
            }

            float directnessScore = 1.0f - ((pathLen / straightDist) - 1.0f) / (maxRatio - 1.0f);
            directnessScore = glm::clamp(directnessScore, 0.0f, 1.0f);

            float randScore = (static_cast<float>(rand()) / RAND_MAX) * 0.2f;
            float score     = angularCoverage * 2.0f + directnessScore + randScore;

            if (!haveBest || score > bestScore)
            {
                haveBest  = true;
                bestPoint = candidate;
                bestScore = score;
            }
        }
        else if (!haveFallback)
        {
            // Exact point unreachable – try partial path, use its endpoint.
            std::vector<vec3> partialPath = NavigationSystem::FindSimplePath(
                owner->Position, candidate,
                /*acceptanceRadius=*/0.5f,
                &outReached,
                /*allowPartialPath=*/true);

            if (!partialPath.empty())
            {
                vec3 partialEnd = partialPath.back();
                partialEnd.y   = npcY;

                if (glm::distance(owner->Position, partialEnd) > 1.0f)
                {
                    haveFallback  = true;
                    fallbackPoint = partialEnd;
                }
            }
        }
    }

    if (haveBest)
    {
        vec3 toPoint = bestPoint - searchOrigin;
        visitedAngles.push_back(std::atan2(toPoint.z, toPoint.x));
        TargetLocation = bestPoint;
        return true;
    }

    if (haveFallback)
    {
        TargetLocation = fallbackPoint;
        return true;
    }

    return false;
}

float SearchInvestigationBase::PickSearchAngle() const
{
    if (visitedAngles.empty())
        return (static_cast<float>(rand()) / RAND_MAX) * 2.0f * glm::pi<float>();

    const int   steps = 36;
    const float twoPi = 2.0f * glm::pi<float>();
    float bestAngle   = 0.0f;
    float bestMinDist = -1.0f;

    for (int i = 0; i < steps; ++i)
    {
        float testAngle = (static_cast<float>(i) / steps) * twoPi;
        float minDist   = std::numeric_limits<float>::max();

        for (float visited : visitedAngles)
        {
            float diff = std::abs(testAngle - visited);
            while (diff >  glm::pi<float>()) diff -= twoPi;
            while (diff < -glm::pi<float>()) diff += twoPi;
            minDist = std::min(minDist, std::abs(diff));
        }

        if (minDist > bestMinDist)
        {
            bestMinDist = minDist;
            bestAngle   = testAngle;
        }
    }

    return bestAngle;
}

float SearchInvestigationBase::PathLength(const std::vector<vec3>& path)
{
    float length = 0.0f;
    for (size_t i = 1; i < path.size(); ++i)
        length += glm::distance(path[i - 1], path[i]);
    return length;
}
