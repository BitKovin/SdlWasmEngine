#pragma once

#include <string>
#include <unordered_map>
#include <utility>
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

    // Exposed only so nlohmann::adl_serializer<PowerUpManager> can (de)serialize us.
    // We deliberately avoid free-standing to_json/from_json + NLOHMANN_JSON_SERIALIZE_ENUM
    // here: on MSVC those are found via ADL and can become ambiguous with nlohmann's own
    // internal to_json/from_json customization objects once another type in the same
    // translation unit also serializes (e.g. Player.hpp). Explicit adl_serializer
    // specialization sidesteps that lookup entirely.
    const std::unordered_map<PowerUpType, float>& GetActiveTimers() const { return m_TimeRemaining; }
    void SetActiveTimers(std::unordered_map<PowerUpType, float> timers) { m_TimeRemaining = std::move(timers); }

    static const char* ToString(PowerUpType type)
    {
        switch (type)
        {
        case PowerUpType::Invincibility: return "Invincibility";
        case PowerUpType::TripleDamage:  return "TripleDamage";
        case PowerUpType::Akimbo:        return "Akimbo";
        }
        return "Unknown";
    }

    static PowerUpType FromString(const std::string& str)
    {
        if (str == "Invincibility") return PowerUpType::Invincibility;
        if (str == "TripleDamage")  return PowerUpType::TripleDamage;
        if (str == "Akimbo")        return PowerUpType::Akimbo;
        return PowerUpType::Invincibility; // fallback for unknown/corrupt data
    }

private:
    // type -> time remaining. Presence in the map with value > 0 means active.
    std::unordered_map<PowerUpType, float> m_TimeRemaining;
};

inline const std::unordered_map<PowerUpManager::PowerUpType, float> PowerUpManager::Durations =
{
    { PowerUpManager::PowerUpType::Invincibility, 5.0f },
    { PowerUpManager::PowerUpType::TripleDamage,  8.0f },
    { PowerUpManager::PowerUpType::Akimbo,        10.0f },
};

namespace nlohmann
{
    template <>
    struct adl_serializer<PowerUpManager>
    {
        static void to_json(json& j, const PowerUpManager& manager)
        {
            j = json::object();
            for (const auto& [type, timeRemaining] : manager.GetActiveTimers())
            {
                if (timeRemaining > 0.0f)
                    j[PowerUpManager::ToString(type)] = timeRemaining;
            }
        }

        static void from_json(const json& j, PowerUpManager& manager)
        {
            std::unordered_map<PowerUpManager::PowerUpType, float> timers;
            for (auto it = j.begin(); it != j.end(); ++it)
                timers[PowerUpManager::FromString(it.key())] = it.value().get<float>();
            manager.SetActiveTimers(std::move(timers));
        }
    };
}