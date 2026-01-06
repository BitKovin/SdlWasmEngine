#include "../Debuff.h"
#include "../DebuffFactory.h"
#include "../IEnemy.h"
#include <Entity.h>

class QuicksilverDebuff : public Debuff
{
public:
    QuicksilverDebuff()
        : Debuff("QuicksilverDebuff",
            3.0f,       // expires if no new stacks for 3s
            DebuffStage::Multiply,
            0,
            100,        // max stacks effectively unlimited
            true, "GameData/textures/ui/debuffs/qs.png")       // refresh duration on stack add
    {
        uiColor = vec3(0.682, 0.89, 0.773);
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

    void OnTick(IEnemy& target, float deltaTime) override
    {
        if (stacks_ >= 100)
        {
			Entity* ent = dynamic_cast<Entity*>(&target);

            if (ent)
            {
				ent->OnDamage(deltaTime*2, nullptr, nullptr);
			}
        }
	}

    float GetProgress() const override
    {
        return static_cast<float>(stacks_) / 100.0f;
	}

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
    IEnemy* target_ = nullptr;


};

REGISTER_DEBUFF(QuicksilverDebuff)