#include "../Debuff.h"
#include "../DebuffFactory.h"
#include "../IEnemy.h"
#include <Entity.h>

class PoiseBreakDebuff : public Debuff
{
public:
    PoiseBreakDebuff()
        : Debuff("PoiseBreakDebuff",
            3.0f,       // expires if no new stacks for 3s
            DebuffStage::PreClamp,
            0,
            999,        // max stacks effectively unlimited
            true, "GameData/textures/ui/debuffs/stun.png")       // refresh duration on stack add
    {
		uiColor = vec3(0.522f, 0.518f, 0.329f);
    }

    // Override to handle stack addition
    void AddStacks(float amount) override
    {

		Debuff::AddStacks(amount);

		if (target_ == nullptr) return;

        if (target_->HasDebuff("DisbalanceDebuff"))
        {

            Entity* targetEnt = dynamic_cast<Entity*>(target_);
            if (targetEnt)
            {
                targetEnt->OnDamage(amount / 3.0f); // deal some damage instead
            }

        }


        if (target_->HasDebuff("PoiseBreakImmune"))
        {

			stacks_ = -1;
            return;
        }

        remainingTime_ = 3.0f; // refresh duration
        // Trigger if threshold reached
        if (stacks_ >= target_->GetPoise())
        {
            if (target_) // optional: store pointer to IEnemy when applied
            {
                target_->Stun();
                target_->AddDebuffStacks("DisbalanceDebuff", 1);
                target_->AddDebuff(std::make_shared<Debuff>(
                    "PoiseBreakImmune", 1.5f, DebuffStage::PreClamp, 0, 1, false, ""
                ));
            }
            remainingTime_ = 1.0f;
			duration_ = 1.0f;
            stacks_ = -1.0f;
        }
    }

    float GetProgress() const override
    {

        if (stacks_ < 0)
            return 1;

        return static_cast<float>(stacks_) / static_cast<float>(target_ ? target_->GetPoise() : 1);
    }

    void OnApply(IEnemy& target) override { target_ = &target; }

private:
    IEnemy* target_ = nullptr; // stored to call Stun & Disbalance
};

REGISTER_DEBUFF(PoiseBreakDebuff)