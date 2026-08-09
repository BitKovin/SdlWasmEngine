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

	trail = (ParticleSystem*)Spawn(trailTypeName);
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

}

void Bullet::AsyncUpdate() {
	// Only the physics trace happens here. This runs in parallel across all
	// entities, so nothing below may read/write other entities' state.

	// 1. LineTrace for world hits (0 radius so it doesn't catch on wall corners)
	auto worldHit = Physics::LineTrace(oldPos, Position, BodyType::World);

	// 2. SphereTrace for entities (larger hitbox for forgiving gameplay)
	// We use GroupHitTest for entities, keeping the owner exclusion if owned.
	auto entityHit = isOwned
		? Physics::SphereTrace(oldPos, Position, 0.07f, BodyType::GroupHitTest & ~BodyType::World, {}, { owner })
		: Physics::SphereTrace(oldPos, Position, 0.1f, BodyType::GroupHitTest & ~BodyType::World);

	// 3. Resolve the closest hit
	// NOTE: Change '.fraction' to '.distance' or '.time' if your engine's hit struct uses a different name.
	if (worldHit.hasHit && entityHit.hasHit)
	{
		asyncHit = (worldHit.fraction < entityHit.fraction) ? worldHit : entityHit;
	}
	else
	{
		// Fallback to whichever one actually registered a hit (if any)
		asyncHit = worldHit.hasHit ? worldHit : entityHit;
	}
}

void Bullet::LateUpdate()
{

	if (isOwned)
	{

		if (asyncHit.hasHit)
		{

			if (asyncHit.entity == owner)
			{
				oldPos = Position;
				return;
			}

			if (asyncHit.entity->HasTag(OwnerTag) == false)
			{

				TargetHit(asyncHit);

			}



			//Logger::Log(hit.surfaceName);

			//GlobalParticleSystem::SpawnParticleAt("hit_flesh", hit.position, MathHelper::FindLookAtRotation(vec3(0), MathHelper::FastNormalize(Position - oldPos)), vec3(2.0f));



			Position = asyncHit.position;
			trail->Position = asyncHit.position;
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
		if (asyncHit.hasHit)
		{
			UpdateEnabled = false;
			Visible = false;
			Position = asyncHit.position;
			trail->Position = asyncHit.position;
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

	Logger::Log("Hit surface: " + to_string(hit.surfaceMaterialType) + "   on entity: " + hit.entity->ClassName);

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
