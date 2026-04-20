#include "IEnemy.h"

void IEnemy::AddDebuff(const std::shared_ptr<Debuff>& debuff)
{
    if (!debuff)
        return;

    for (auto& existing : debuffs_)
    {
        if (existing->GetName() == debuff->GetName())
        {
            if (existing->OnReapply(*this, debuff))
            {
                SortDebuffs();
                return;
            }
        }
    }

    debuff->OnApply(*this);
    debuffs_.push_back(debuff);
    SortDebuffs();
}

void IEnemy::AddDebuffStacks(const std::string& debuffName, float stacks)
{
    // 1. Grab a shared_ptr copy so it survives even if debuffs_.clear() is called
    std::shared_ptr<Debuff> debuffToUpdate = nullptr;
    for (auto& d : debuffs_)
    {
        if (d->GetName() == debuffName)
        {
            debuffToUpdate = d;
            break;
        }
    }

    if (debuffToUpdate)
    {
        debuffToUpdate->AddStacks(stacks);
        return;
    }

    // 2. If not found, create it as before
    auto debuff = DebuffFactory::Instance().CreateDebuff(debuffName);
    AddDebuff(debuff);
    debuff->AddStacks(stacks - 1);
}

void IEnemy::UpdateDebuffs(float deltaTime)
{
    for (auto& d : debuffs_)
    {
        d->Update(*this, deltaTime);
        d->OnTick(*this, deltaTime);
    }

    auto it = std::remove_if(debuffs_.begin(), debuffs_.end(),
        [&](const std::shared_ptr<Debuff>& d)
        {
            if (d->IsExpired())
            {
                d->OnRemove(*this);
                return true;
            }
            return false;
        });

    debuffs_.erase(it, debuffs_.end());
}

bool IEnemy::HasDebuff(const std::string& name) const
{
    for (const auto& d : debuffs_)
        if (d->GetName() == name)
            return true;
    return false;
}

int IEnemy::GetDebuffStacks(const std::string& name) const
{
    for (const auto& d : debuffs_)
        if (d->GetName() == name)
            return d->GetStacks();
    return 0;
}

float IEnemy::ModifyIncomingDamage(float baseDamage)
{
    float value = baseDamage;
    for (auto& d : debuffs_)
        value = d->ModifyIncomingDamage(*this, value);
    return value;
}

float IEnemy::ModifyOutgoingDamage(float baseDamage)
{
    float value = baseDamage;
    for (auto& d : debuffs_)
        value = d->ModifyOutgoingDamage(*this, value);
    return value;
}

float IEnemy::ModifyAnimationSpeed(float baseSpeed)
{
    float value = baseSpeed;
    for (auto& d : debuffs_)
        value = d->ModifyAnimationSpeed(*this, value);
    return value;
}

float IEnemy::ModifyMovementSpeed(float baseSpeed)
{
    float value = baseSpeed;
    for (auto& d : debuffs_)
        value = d->ModifyMovementSpeed(*this, value);
    return value;
}

float IEnemy::GetDebuffStacksTotal(const std::string& debuffName) const
{
    for (auto& d : debuffs_)
    {
		if (d->GetName() == debuffName)
        {
            return d->GetStacks();
        }
    }
    return 0;
}

std::string IEnemy::GetDebuffsDebugInfo() const
{
    std::ostringstream ss;

    for (size_t i = 0; i < debuffs_.size(); ++i)
    {
        ss << debuffs_[i]->GetDebugInfo();

        if (i + 1 < debuffs_.size())
            ss << '\n';
    }

    return ss.str();
}

void IEnemy::SortDebuffs()
{
    std::sort(debuffs_.begin(), debuffs_.end(),
        [](const std::shared_ptr<Debuff>& a,
            const std::shared_ptr<Debuff>& b)
        {
            if (a->GetStage() != b->GetStage())
                return a->GetStage() < b->GetStage();
            return a->GetPriority() < b->GetPriority();
        });
}
