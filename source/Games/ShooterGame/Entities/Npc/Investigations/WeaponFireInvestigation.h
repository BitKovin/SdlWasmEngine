#pragma once
#include "SearchInvestigationBase.h"

class WeaponFireInvestigation : public SearchInvestigationBase
{
public:
    WeaponFireInvestigation(NpcBase* owner) : SearchInvestigationBase(owner) {}
    void OnStart() override;

    void Update(float deltaTime) override;

protected:
    void OnNoGuardFound() override;

    float SearchDuration() const override { return 25.0f; }
    float SearchRadius()   const override { return 8.0f; }
};
