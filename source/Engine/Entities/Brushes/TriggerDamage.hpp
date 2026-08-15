#include "TriggerBase.hpp"

class TriggerDamage : public TriggerBase
{
public:

	TriggerDamage() : TriggerBase()
	{
		ClassName = "trigger_damage";
	}

	float enterDamage = 10.0f;
	float tickDamage = 10.0f;

	Delay tickDelay = Delay(0.35f);

	std::vector<Entity*> inEntities;

	void OnBodyEntered(Body* body, Entity* entity);
	void OnBodyExited(Body* body, Entity* entity);

	void Update();

	void FromData(EntityData data)
	{
		TriggerBase::FromData(data);
		
		enterDamage = data.GetPropertyFloat("enterDamage", enterDamage);

		tickDamage = data.GetPropertyFloat("tickDamage", tickDamage);

	}

private:

};