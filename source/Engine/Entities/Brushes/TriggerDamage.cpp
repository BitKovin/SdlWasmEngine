#include "TriggerDamage.hpp"

void TriggerDamage::OnBodyEntered(Body* body, Entity* entity)
{
	inEntities.push_back(entity);

	entity->OnDamage(enterDamage, nullptr, nullptr);
}

void TriggerDamage::OnBodyExited(Body * body, Entity * entity)
{
	inEntities.erase(std::remove(inEntities.begin(), inEntities.end(), entity), inEntities.end());
}

void TriggerDamage::Update()
{

	TriggerBase::Update();

	for (auto ent : inEntities)
	{
		if(Level::Current->DeletedLevelObjectAdresses.count(ent))
		{
			inEntities.erase(std::remove(inEntities.begin(), inEntities.end(), ent), inEntities.end());
		}
	}

	

	if (tickDelay.Wait()) return;

	for (auto entity : inEntities)
	{
		if (entity->Destroyed) continue;
		entity->OnDamage(tickDamage, nullptr, nullptr);
	}

	tickDelay.AddDelay(0.35f);

}
