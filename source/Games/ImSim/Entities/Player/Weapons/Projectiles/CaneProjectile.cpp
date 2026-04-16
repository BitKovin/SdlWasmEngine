#include "CaneProjectile.h"

#include "../../Player.hpp"

REGISTER_ENTITY(CaneProjectile, "caneProjectile")



void CaneProjectile::DamageEntity()
{

	Entity* hitedEntity = Level::Current->FindEntityWithId(hitEntityId);

	if (hitedEntity == nullptr)
	{	
		return;
	}

	vec3 forward = MathHelper::GetForwardVector(Rotation);

	hitedEntity->OnPointDamage(200, Position, forward * -1.0f, "", Player::Instance, this);

	auto hit = Physics::SphereTraceForEntity({ hitedEntity }, Position - forward * 0.5f, Position + forward, 0.05f, BodyType::HitBox);

	if (hit.hasHit == false)
	{
		Physics::SphereTraceForEntity({hitedEntity}, Position - forward * 0.5f, Position + forward, 0.3f, BodyType::HitBox);
	}

	if (hit.hasHit)
	{

		bodyToPush = hit.hitbody;
		impulseToApply = forward * (Damage) * 12.0f;

		Physics::AddImpulse(bodyToPush, impulseToApply*0.04f);

	}


}

void CaneProjectile::Update()
{

	float travelDistance = Speed * Time::DeltaTimeF;

	traveledDistance += travelDistance;

	vec3 forward = MathHelper::GetForwardVector(Rotation);


	trail->Position = Position;
	mesh->Position = Position;
	mesh->Rotation = Rotation;

	if (hited)
	{

		Entity* hitedEntity = Level::Current->FindEntityWithId(hitEntityId);

		if (hitedEntity == nullptr)
		{
			Destroy();
			return;
		}
		
		inEnemy = false;

		if (hitedEntity->HasTag("enemy"))
		{

			if (movingTo == false)
			{
				Rotation.y = MathHelper::FindLookAtRotation(hitedEntity->Position, Player::Instance->Position).y;


				Position = hitedEntity->Position + MathHelper::RotateVector(relativeOffset, vec3(0, Rotation.y, 0));

				Rotation.y += 180;
			}
			else
			{
				Position = hitedEntity->Position + MathHelper::RotateVector(relativeOffset, vec3(0, Rotation.y, 0));
			}


			if (hitedEntity->HasTag("enemy") && hitedEntity->Health <= 0)
			{
				Destroy();
				return;
			}
			else
			{
				inEnemy = true;
			}

		}



		


		trail->Position = Position;
		mesh->Position = Position;
		mesh->Rotation = Rotation - vec3(0,5,0);

		return;

	}

	Position += MathHelper::GetForwardVector(Rotation) * travelDistance;

	auto hit = Physics::SphereTrace(oldPos, Position, 0.1f, BodyType::GroupHitTest | BodyType::WorldSkybox, {},{owner});

	if (hit.hasHit)
	{

		if (Physics::GetBodyData(hit.hitbody)->group == BodyType::WorldSkybox)
		{
			Destroy();
			return;
		}

		hit.entity->OnPointDamage(5, hit.position, MathHelper::FastNormalize(Position - oldPos), hit.hitboxName, this, this);

		
		trail->Position = hit.position;
		mesh->Position = hit.position;
		Position = hit.position;

		hited = true;

		hitEntityId = hit.entity->Id;

		relativeOffset = Position - hit.entity->Position;

		relativeOffset = MathHelper::RotateVector(relativeOffset, vec3(0, -1 * MathHelper::FindLookAtRotation(Position, Player::Instance->Position).y,0));

		Speed = 0;

		return;
	}

	if (traveledDistance > MaxDistance)
	{
		trail->Position = hit.position;
		trail->StopAll();
		trail = nullptr;

		Destroy();

	}

	oldPos = Position;

}