#include "../Debuff.h"
#include "../DebuffFactory.h"
#include "../IEnemy.h"

class QuicksilverDebuff : public Debuff
{
public:
    QuicksilverDebuff()
        : Debuff("QuicksilverDebuff",
            3.0f,       // expires if no new stacks for 3s
            DebuffStage::Multiply,
            0,
            100,        // max stacks effectively unlimited
            true)       // refresh duration on stack add
    {
    }

    float ModifyIncomingDamage(IEnemy& target, float value) 
    { 

		if (stacks_ >= 100)
        {
			return value * 1.5f;
        }

        return value; 
    }
    float ModifyOutgoingDamage(IEnemy& target, float value) { return value; }
    float ModifyAnimationSpeed(IEnemy& target, float value) { return value; }
    float ModifyMovementSpeed(IEnemy& target, float value) { return value; }

    // Override to handle stack addition
    void AddStacks(float amount) override
    {

        Debuff::AddStacks(amount);

        if (stacks_ == 100)
        {
            remainingTime_ = 10.0f;
			duration_ = 10.0f;
        }
        
    }

    void OnApply(IEnemy& target) override { target_ = &target; }

private:
    IEnemy* target_ = nullptr; // stored to call Stun & Disbalance


};

REGISTER_DEBUFF(QuicksilverDebuff)