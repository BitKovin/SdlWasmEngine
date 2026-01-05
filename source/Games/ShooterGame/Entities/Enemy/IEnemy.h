#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

#include "Debuff.h"

#include "DebuffFactory.h"
#include <json.hpp>

class IEnemy
{
public:
    virtual ~IEnemy() = default;


    void AddDebuffStacks(const std::string& debuffName, float stacks);

    void UpdateDebuffs(float deltaTime);

    bool HasDebuff(const std::string& name) const;
    int GetDebuffStacks(const std::string& name) const;

    // ---- Modifier pipelines ----
    float ModifyIncomingDamage(float baseDamage);
    float ModifyOutgoingDamage(float baseDamage);
    float ModifyAnimationSpeed(float baseSpeed);
    float ModifyMovementSpeed(float baseSpeed);

	float GetDebuffStacksTotal(const std::string& debuffName) const;

	virtual float GetPoise() const { return Poise; }
    virtual void Stun() {};

    float Poise = 20;

    void AddDebuff(const std::shared_ptr<Debuff>& debuff);

    nlohmann::json SerializeDebuffs() const
    {
        nlohmann::json j = nlohmann::json::array();
        for (auto& d : debuffs_)
        {
            j.push_back(d->Serialize());
        }
        return j;
    }

    void DeserializeDebuffs(const nlohmann::json& j)
    {
        debuffs_.clear();

        for (auto& item : j)
        {
            if (!item.contains("name")) continue;

            std::string name = item["name"].get<std::string>();
            auto debuff = DebuffFactory::Instance().CreateDebuff(name);
            if (!debuff) continue;

            debuff->Deserialize(item);
            AddDebuff(debuff);
        }
    }


protected:
    std::vector<std::shared_ptr<Debuff>> debuffs_;



private:
    void SortDebuffs();
};
