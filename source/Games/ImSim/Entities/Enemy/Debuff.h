#pragma once

#include <string>
#include <memory>

#include <cstdint>
#include <json.hpp>
#include <glm.h>

#include <sstream>

enum class DebuffStage : uint8_t
{
    PreClamp = 0,   // additive, setup effects
    Multiply = 1,   // multipliers
    Override = 2,   // forced values (stun, freeze)
    PostClamp = 3   // caps, safety clamps
};

class IEnemy;

class Debuff
{
public:
    Debuff(std::string name,
        float durationSeconds,
        DebuffStage stage,
        int priority,
        float maxStacks = 1,
        bool refreshOnReapply = true,
        std::string icon = "");

    virtual ~Debuff() = default;

    // ---- Lifecycle ----
    virtual void OnApply(IEnemy& target) {}
    virtual void OnRemove(IEnemy& target) {}
    virtual void Update(IEnemy& target, float deltaTime);
    virtual void OnTick(IEnemy& target, float deltaTime) {}

    // ---- Modifiers ----
    virtual float ModifyIncomingDamage(IEnemy& target, float value) { return value; }
    virtual float ModifyOutgoingDamage(IEnemy& target, float value) { return value; }
    virtual float ModifyAnimationSpeed(IEnemy& target, float value) { return value; }
    virtual float ModifyMovementSpeed(IEnemy& target, float value) { return value; }

    // ---- Reapply / stacking ----
    virtual bool OnReapply(IEnemy& target, const std::shared_ptr<Debuff>& incoming);

    // ---- State ----
    bool IsExpired() const { return remainingTime_ <= 0.0f; }

    const std::string& GetName() const { return name_; }
    DebuffStage GetStage() const { return stage_; }
    int GetPriority() const { return priority_; }
    int GetStacks() const { return stacks_; }

    virtual float GetProgress() const
    {
        return 1;
	}

    virtual float GetTimeRel() const
    {
        if (duration_ <= 0.0f) return 0.0f;
        return remainingTime_ / duration_;
	}

    virtual void AddStacks(float amount)
    {

        if(refreshOnReapply_)
        {
            remainingTime_ = duration_;
		}

        // By default, integer stacks
        stacks_ += static_cast<int>(amount);
        if (stacks_ > maxStacks_) stacks_ = maxStacks_;
    }

    virtual nlohmann::json Serialize() const
    {
        return {
            {"name", name_},
            {"remainingTime", remainingTime_},
            {"stacks", stacks_},
            {"duration_", duration_}
        };
    }

    virtual void Deserialize(const nlohmann::json& j)
    {
        if (j.contains("remainingTime")) remainingTime_ = j["remainingTime"].get<float>();
        if (j.contains("stacks")) stacks_ = j["stacks"].get<int>();
		if (j.contains("duration_")) duration_ = j["duration_"].get<float>();
    }

    virtual std::string GetDebugInfo() const
    {
        std::ostringstream ss;
        ss << name_;

        if (stacks_ > 0)
            ss << " | stacks: " << stacks_;

        if (duration_ > 0.0f)
        {
            ss << " | time: "
                << std::fixed << std::setprecision(2)
                << remainingTime_ << " / " << duration_;
        }

        return ss.str();
    }

	std::string iconPath = "";

	float uiProgress = 0.0f;
	bool uiShowStacks = false;
	vec3 uiColor = vec3(1.0f, 1.0f, 1.0f);

protected:
    std::string name_;

    float duration_;
    float remainingTime_;

    DebuffStage stage_;
    int priority_;

    float maxStacks_;
    float stacks_;
    bool refreshOnReapply_;
};
