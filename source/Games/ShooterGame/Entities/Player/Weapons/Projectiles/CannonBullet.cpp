#include "Bullet.h"

#include <Entities/Explosion.h>

class CannonBullet : public Bullet
{

public:

	vector<Entity*> hitEntities;

	void Start() override
	{
		Bullet::Start();
		hitEntities.push_back(owner);
	}

	void Update() override
	{
		float travelDistance = Speed * Time::DeltaTimeF;

		traveledDistance += travelDistance;

		vec3 forward = MathHelper::GetForwardVector(Rotation);

		Position += MathHelper::GetForwardVector(Rotation) * travelDistance;

		trail->Position = Position;

		auto hit = Physics::LineTrace(oldPos, Position, BodyType::GroupHitTest, {}, hitEntities);


		if (hit.hasHit)
		{

			hitEntities.push_back(hit.entity);

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


			if (Physics::GetBodyData(hit.hitbody)->group & BodyType::World)
			{
				Explosion* explosion = new Explosion();
				explosion->Position = hit.position + hit.normal * 0.5f;
				explosion->Radius = 3.0f;
				explosion->minDamage = Damage * 0.5f;
				explosion->MaxDamage = Damage;
				explosion->minPlayerDamage = 40;
				explosion->MaxPlayerDamage = 60;
				explosion->Lifetime = 2.9f;
				explosion->LoadAssetsIfNeeded();
				explosion->damageCauser = damageCauser;

				Level::Current->AddEntity(explosion);

				explosion->Start();

				Destroy();
				trail->Position = hit.position;
				trail->StopAll();
				trail = nullptr;

			}


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

	void TargetHit(Physics::HitResult hit) override
	{
		Bullet::TargetHit(hit);
		
	}

	void LoadAssets() override
	{
		Bullet::LoadAssets();
		PreloadEntityType("explosion");
	}

};

REGISTER_ENTITY(CannonBullet, "cannon_bullet")