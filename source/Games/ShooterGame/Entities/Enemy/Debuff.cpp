#include "Debuff.h"
#include "IEnemy.h"

Debuff::Debuff(std::string name,
    float durationSeconds,
    DebuffStage stage,
    int priority,
    float maxStacks,
    bool refreshOnReapply, std::string icon)
    : name_(std::move(name))
    , duration_(durationSeconds)
    , remainingTime_(durationSeconds)
    , stage_(stage)
    , priority_(priority)
    , maxStacks_(maxStacks)
    , stacks_(1)
    , refreshOnReapply_(refreshOnReapply)
	, iconPath(std::move(icon))
{
}

void Debuff::Update(IEnemy&, float deltaTime)
{
    remainingTime_ -= deltaTime;
}

bool Debuff::OnReapply(IEnemy&, const std::shared_ptr<Debuff>&)
{
    if (stacks_ < maxStacks_)
    {
        ++stacks_;
        if (refreshOnReapply_)
            remainingTime_ = duration_;
        return true;
    }

    if (refreshOnReapply_)
    {
        remainingTime_ = duration_;
        return true;
    }

    return false;
}
