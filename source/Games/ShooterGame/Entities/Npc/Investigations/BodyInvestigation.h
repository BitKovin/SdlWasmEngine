#pragma once
#include "SearchInvestigationBase.h"

class BodyInvestigation : public SearchInvestigationBase
{
public:
    BodyInvestigation(NpcBase* owner) : SearchInvestigationBase(owner) {}
    void Finish() override;

protected:
    void OnNoGuardFound() override;

    float SearchDuration() const override { return 20.0f; }
    float SearchRadius()   const override { return 5.0f; }
    float WaitChance()     const override { return 0.6f; }  // cautious, stops to look around more

private:
    void InvestigateCauserBody();
};
