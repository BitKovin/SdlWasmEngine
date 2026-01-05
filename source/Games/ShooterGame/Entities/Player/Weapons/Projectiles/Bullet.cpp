#include "Bullet.h"

#include <Physics.h>

#include <Particle/GlobalParticleSystem.hpp>

#include <Entities/PointLight.h>

#include "../../Player.hpp"

#include "../../../Enemy/IEnemy.h"
REGISTER_ENTITY(Bullet, "bullet")

Bullet::Bullet()
{

}

Bullet::~Bullet()
{
}

void Bullet::Start()
{

	oldPos = Position;

	trail = (ParticleSystem*)Spawn("bullet_trail");
	trail->Position = Position;
	trail->Rotation = Rotation;
	trail->Start();

	if (OwnerTag == "player")
	{
		owner = Player::Instance;
	}


}

void Bullet::Update()
{

	float travelDistance = Speed * Time::DeltaTimeF;

	traveledDistance += travelDistance;

	vec3 forward = MathHelper::GetForwardVector(Rotation);

	Position += MathHelper::GetForwardVector(Rotation) * travelDistance;

	trail->Position = Position;

	auto hit = Physics::LineTrace(oldPos, Position, BodyType::GroupHitTest, {}, {owner});


	if (hit.hasHit)
	{

		if (hit.entity == owner)
		{
			oldPos = Position;
			return;
		}

		if (hit.entity->HasTag(OwnerTag) == false)
		{

			hit.entity->OnPointDamage(Damage, hit.position, MathHelper::FastNormalize(Position - oldPos), hit.hitboxName, this, this);
			Physics::AddImpulseAtLocation(hit.hitbody, forward * (Damage + 2) * 4.5f, hit.position);

			IEnemy* enemy = dynamic_cast<IEnemy*>(hit.entity);

			if(enemy)
			{
				enemy->AddDebuffStacks("PoiseBreakDebuff", Damage);
			}

		}



		//Logger::Log(hit.surfaceName);

		//GlobalParticleSystem::SpawnParticleAt("hit_flesh", hit.position, MathHelper::FindLookAtRotation(vec3(0), MathHelper::FastNormalize(Position - oldPos)), vec3(2.0f));


		Destroy();
		trail->Position = hit.position;
		trail->StopAll();
		trail = nullptr;
		return;
	}

	if (traveledDistance > MaxDistance)
	{
		Destroy();
		trail->Position = hit.position;
		trail->StopAll();
		trail = nullptr;
	}

	oldPos = Position;

}
