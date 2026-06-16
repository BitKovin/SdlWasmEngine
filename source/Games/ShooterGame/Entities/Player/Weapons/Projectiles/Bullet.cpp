#include "Bullet.h"

#include <Physics.h>

#include <Particle/GlobalParticleSystem.hpp>

#include <Entities/PointLight.h>

#include "../../Player.hpp"

#include "../../../Enemy/IEnemy.h"
REGISTER_ENTITY(Bullet, "bullet")

Bullet::Bullet()
{
	DestroyOnOwnerDisconnect = true;
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

	if (isOwned)
	{

		Physics::HitResult hit = Physics::SphereTrace(oldPos, Position, 0.07f, BodyType::GroupHitTest, {}, { owner });

		if (hit.hasHit)
		{

			if (hit.entity == owner)
			{
				oldPos = Position;
				return;
			}

			if (hit.entity->HasTag(OwnerTag) == false)
			{

				TargetHit(hit);

			}



			//Logger::Log(hit.surfaceName);

			//GlobalParticleSystem::SpawnParticleAt("hit_flesh", hit.position, MathHelper::FindLookAtRotation(vec3(0), MathHelper::FastNormalize(Position - oldPos)), vec3(2.0f));



			Position = hit.position;
			trail->Position = hit.position;
			trail->StopAll();
			trail->DestroyWithDelay(0.5f);
			trail = nullptr;
			UpdateEnabled = false;
			Destroy();

			return;
		}
	}
	else
	{
		Physics::HitResult hit = Physics::SphereTrace(oldPos, Position, 0.1f, BodyType::World);

		if (hit.hasHit)
		{
			UpdateEnabled = false;
			Visible = false;
			Position = hit.position;
			trail->Position = hit.position;
			trail->StopAll();
			trail->DestroyWithDelay(0.5f);
			return;
		}

	}



	if (traveledDistance > MaxDistance)
	{
		Visible = false;
		UpdateEnabled = false;
		Destroy();
		trail->Position = Position;// hit.position;
		trail->StopAll();
		trail->DestroyWithDelay(0.5f);
		trail = nullptr;
	}

	oldPos = Position;

}

void Bullet::TargetHit(Physics::HitResult hit)
{

	if (isOwned == false) return;

	hit.entity->OnPointDamage(Damage, hit.position, MathHelper::FastNormalize(Position - oldPos), hit.hitboxName, damageCauser, this);


	IEnemy* enemy = dynamic_cast<IEnemy*>(hit.entity);

	if (enemy)
	{
		if (debuffStacks > 0 && debuffOnHit != "")
		{
			enemy->AddDebuffStacks(debuffOnHit, debuffStacks);
		}

	}

	vec3 forward = MathHelper::GetForwardVector(Rotation);
	Physics::AddImpulseAtLocation(hit.hitbody, forward * (Damage + 2) * 14.0f, hit.position);

}

void Bullet::NetSerialize(NetPacket& packet)
{

	packet.WriteVector3(Position);
	packet.WriteVector3(Rotation);

}

void Bullet::NetDeserialize(NetPacket & packet)
{

	Position = packet.ReadVector3();
	Rotation = packet.ReadVector3();

}
