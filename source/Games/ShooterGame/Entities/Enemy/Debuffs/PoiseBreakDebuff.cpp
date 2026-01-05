#include "../Debuff.h"
#include "../DebuffFactory.h"
#include "../IEnemy.h"

class PoiseBreakDebuff : public Debuff
{
public:
    PoiseBreakDebuff()
        : Debuff("PoiseBreakDebuff",
            3.0f,       // expires if no new stacks for 3s
            DebuffStage::PreClamp,
            0,
            999,        // max stacks effectively unlimited
            true)       // refresh duration on stack add
    {
    }

    // Override to handle stack addition
    void AddStacks(float amount) override
    {

		Debuff::AddStacks(amount);

        remainingTime_ = 3.0f; // refresh duration

        if (target_->HasDebuff("PoiseBreakImmune"))
        {
			stacks_ = 0;
            return;
        }

        // Trigger if threshold reached
        if (stacks_ >= target_->GetPoise())
        {
            if (target_) // optional: store pointer to IEnemy when applied
            {
                target_->Stun();
                target_->AddDebuffStacks("DisbalanceDebuff", 1);
                target_->AddDebuff(std::make_shared<Debuff>(
                    "PoiseBreakImmune", 2.0f, DebuffStage::PreClamp, 0, 1, false
                ));
            }
            remainingTime_ = 0.0f;
            stacks_ = 0.0f;
        }
    }

    void OnApply(IEnemy& target) override { target_ = &target; }

private:
    IEnemy* target_ = nullptr; // stored to call Stun & Disbalance
};

REGISTER_DEBUFF(PoiseBreakDebuff)