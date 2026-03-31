#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

class BulletTrailEmitter : public RibbonEmitter
{
public:

	BulletTrailEmitter() : RibbonEmitter()
	{
		texture = "GameData/textures/particles/trail.png";
		SpawnRate = 100;
		Emitting = true;
		InitialSpawnCount = 1;
		SimpleRibbon = true;
	}

	Particle UpdateParticle(Particle particle, float deltaTime) override
	{
		
		particle.Transparency = mix(1.0,1.0, (particle.deathTime - particle.lifeTime) / particle.deathTime);
		particle.Size = mix(0.015f, 0.055f, (particle.deathTime - particle.lifeTime) / particle.deathTime);
		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();
		
		particle.Size = 0.03f;
		particle.Color = vec4(0.4, 0, 0, 0.65);
		particle.Transparency = 0.8;
		particle.deathTime = 0.1f;

		return particle;
	}

private:

};

class BulletTrail : public ParticleSystem
{
public:
	BulletTrail()
	{
		emitters.push_back(new BulletTrailEmitter());
	}


private:

};

REGISTER_ENTITY(BulletTrail, "bullet_trail")
