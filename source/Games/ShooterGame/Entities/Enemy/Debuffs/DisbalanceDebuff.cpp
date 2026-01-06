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
            true, "GameData/textures/ui/debuffs/disb.png")                  // refresh duration on reapply
    {
		uiColor = vec3(0.2f, 0.2f, 0.2f);
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