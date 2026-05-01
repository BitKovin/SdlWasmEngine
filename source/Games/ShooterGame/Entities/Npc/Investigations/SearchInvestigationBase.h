#pragma once
#include "AlertedInvestigationBase.h"
#include <vector>

class SearchInvestigationBase : public AlertedInvestigationBase
{
public:
    SearchInvestigationBase(NpcBase* owner) : AlertedInvestigationBase(owner) {}

    void Update(float deltaTime) override;
    void ReachedTarget() override;

    nlohmann::json Serialize() override;
    void Deserialize(nlohmann::json& data) override;

protected:
    // How long the NPC searches before giving up (seconds).
    virtual float SearchDuration()       const { return 20.0f; }

    // Radius around the event origin within which candidates are generated.
    virtual float SearchRadius()         const { return 6.0f; }

    // How many random candidates to try per waypoint before settling on
    // the best available one (or falling back to a partial path endpoint).
    virtual int   MaxCandidateAttempts() const { return 12; }

    // A path is considered too indirect and rejected if its walked length
    // exceeds the straight-line distance multiplied by this factor.
    virtual float MaxPathLengthRatio()   const { return 2.2f; }

    // Probability [0-1] of pausing at each waypoint to look around.
    virtual float WaitChance()           const { return 0.5f; }

    // Random wait duration range (seconds).
    virtual float MinWaitTime()          const { return 1.5f; }
    virtual float MaxWaitTime()          const { return 3.5f; }

protected:
    enum class SearchPhase { Inactive, Walking, Waiting };

    SearchPhase searchPhase   = SearchPhase::Inactive;
    float       searchTimer   = 0.0f; // counts up
    float       waitTimer     = 0.0f;
    vec3        searchOrigin{};

    // Visited sector angles so the NPC sweeps the whole area.
    std::vector<float> visitedAngles;

    void  BeginSearch();
    void  AdvanceToNextPoint();
    bool  FindNextSearchPoint();
    float PickSearchAngle() const;

    static float PathLength(const std::vector<vec3>& path);
};
