#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

class WeaponSmokeEmitter : public RibbonEmitter
{
public:


	WeaponSmokeEmitter() : RibbonEmitter()
	{
		texture = "GameData/textures/particles/trail.png";
		SpawnRate = 20;
		Emitting = true;
		InitialSpawnCount = 1;
		SimpleRibbon = false;
		IsViewmodel = true;
	}

	float easeOutCubic(float val)
	{
		return 1.0f - powf(1.0f - val, 2.0f);
	}

	Particle UpdateParticle(Particle particle, float deltaTime) override
	{

		float deathTime = particle.deathTime - 0.1f;

		particle.Transparency = mix(0.1, 1.0, (deathTime - particle.lifeTime) / deathTime);
		particle.Size = mix(0.055f, 0.025f, (deathTime - particle.lifeTime) / deathTime);

		float t = particle.lifeTime / deathTime;
		float eased = easeOutCubic(t);

		float verticalSpeed = mix(0.8f, 0.25f, eased);

		particle.position += vec3(0, verticalSpeed, 0) * Time::DeltaTimeF;

		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();

		float systemTransparency = MathHelper::MapRange(emitterTime, 0.5, 1.8f, 1, 0);

		particle.Size = 0.03f;
		particle.Color = vec4(0.8, 0.8, 0.8, 0.2 * systemTransparency);

		particle.Transparency = 0.8;
		particle.deathTime = 1.4f;

		return particle;
	}



	void Update(float deltaTime)
	{

		RibbonEmitter::Update(deltaTime);

		if (emitterTime > 1.8f) { Emitting = false; }
			
		if (emitterTime < 1.85f)
		{
			if (Particles.size() > 0)
			{
				Particles[Particles.size() - 1].position = Position;
			}
		}


	}

private:

};

class WeaponSmoke : public ParticleSystem
{
public:
	WeaponSmoke()
	{
		emitters.push_back(new WeaponSmokeEmitter());
	}


private:

};

REGISTER_ENTITY(WeaponSmoke, "weapon_smoke")
