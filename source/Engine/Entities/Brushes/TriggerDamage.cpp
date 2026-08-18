#include "TriggerDamage.hpp"

void TriggerDamage::OnBodyEntered(Body* body, Entity* entity)
{
	if (entity->Static == true || entity->ClassName == "Entity") return;

	if (std::find(inEntities.begin(), inEntities.end(), entity) != inEntities.end()) return;

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

	inEntities.erase(
		std::remove_if(inEntities.begin(), inEntities.end(),
			[](Entity* ent)
			{
				return Level::Current->DeletedLevelObjectAdresses.count(ent) != 0;
			}),
		inEntities.end());

	if (tickDelay.Wait()) return;

	for (auto entity : inEntities)
	{
		if (entity->Destroyed) continue;
		entity->OnDamage(tickDamage, nullptr, nullptr);
	}

	tickDelay.AddDelay(0.35f);
}
