#include "../Debuff.h"
#include "../DebuffFactory.h"

class DisbalanceDebuff : public Debuff
{
public:
    DisbalanceDebuff()
        : Debuff("DisbalanceDebuff",
            10.0f,                  // natural duration
            DebuffStage::Multiply,
            15,                    // priority
            1,                     // max stacks
            true)                  // refresh duration on reapply
    {
    }

    float ModifyMovementSpeed(IEnemy&, float value) override
    {
        return value * 0.75f;
    }

    float ModifyAnimationSpeed(IEnemy&, float value) override
    {
        return value * 0.8f;
    }

    float ModifyOutgoingDamage(IEnemy&, float value) override
    {
        return value * 0.7f;
    }
};

REGISTER_DEBUFF(DisbalanceDebuff)