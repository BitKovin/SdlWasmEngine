#pragma once

#include <unordered_map>
#include <json.hpp>

class PowerUpManager
{
public:
    enum class PowerUpType
    {
        Invincibility,
        TripleDamage,
        Akimbo
    };

    // Fixed duration (seconds) for each power up type.
    static const std::unordered_map<PowerUpType, float> Durations;

    // Activating a power up simply resets its time remaining to full duration.
    void ActivatePowerUp(PowerUpType type)
    {
        auto it = Durations.find(type);
        m_TimeRemaining[type] = (it != Durations.end()) ? it->second : 0.0f;
    }

    bool IsPowerUpActive(PowerUpType type) const
    {
        auto it = m_TimeRemaining.find(type);
        return it != m_TimeRemaining.end() && it->second > 0.0f;
    }

    void Update(float dt)
    {
        for (auto& [type, timeRemaining] : m_TimeRemaining)
        {
            if (timeRemaining > 0.0f)
            {
                timeRemaining -= dt;
                if (timeRemaining < 0.0f)
                    timeRemaining = 0.0f;
            }
        }
    }

    friend void to_json(nlohmann::json& j, const PowerUpManager& manager);
    friend void from_json(const nlohmann::json& j, PowerUpManager& manager);

private:
    // type -> time remaining. Presence in the map with value > 0 means active.
    std::unordered_map<PowerUpType, float> m_TimeRemaining;
};

// String mapping used by nlohmann for enum <-> json conversion (also used manually below).
NLOHMANN_JSON_SERIALIZE_ENUM(PowerUpManager::PowerUpType,
    {
        { PowerUpManager::PowerUpType::Invincibility, "Invincibility" },
        { PowerUpManager::PowerUpType::TripleDamage,  "TripleDamage" },
        { PowerUpManager::PowerUpType::Akimbo,        "Akimbo" },
    })

    inline const std::unordered_map<PowerUpManager::PowerUpType, float> PowerUpManager::Durations =
{
    { PowerUpManager::PowerUpType::Invincibility, 5.0f },
    { PowerUpManager::PowerUpType::TripleDamage,  8.0f },
    { PowerUpManager::PowerUpType::Akimbo,        10.0f },
};

inline void to_json(nlohmann::json& j, const PowerUpManager& manager)
{
    j = nlohmann::json::object();
    for (const auto& [type, timeRemaining] : manager.m_TimeRemaining)
    {
        if (timeRemaining > 0.0f)
        {
            const std::string key = nlohmann::json(type).get<std::string>();
            j[key] = timeRemaining;
        }
    }
}

inline void from_json(const nlohmann::json& j, PowerUpManager& manager)
{
    manager.m_TimeRemaining.clear();
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        const PowerUpManager::PowerUpType type =
            nlohmann::json(it.key()).get<PowerUpManager::PowerUpType>();
        manager.m_TimeRemaining[type] = it.value().get<float>();
    }
}