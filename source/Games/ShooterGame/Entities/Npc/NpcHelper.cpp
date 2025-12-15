#include "NpcHelper.h"


bool NpcHelper::CheckParry(vec3 npcForward, Entity* entity)
{

	Player* player = dynamic_cast<Player*>(entity);

	if (player == nullptr)
		return false;

	bool parrying = false;

	if (player->currentOffhandWeapon)
	{
		if(player->currentOffhandWeapon->Parrying)
			parrying = true;
	}

	if (parrying == false)
		return false;

	vec3 toTarget = glm::normalize(entity->Position - npcForward);

	vec3 targetForward = glm::normalize(MathHelper::GetForwardVector(player->cameraRotation));

	float dotProduct = glm::dot(npcForward, targetForward);

	bool res = dotProduct < -0.7f;

	if (res)
	{
		player->currentOffhandWeapon->OnParried();
	}

	return res;

}
