#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

class CaneTrailEmitter : public RibbonEmitter
{
public:

	CaneTrailEmitter() : RibbonEmitter()
	{
		texture = "GameData/textures/particles/trail.png";
		SpawnRate = 100;
		Emitting = true;
		InitialSpawnCount = 1;
		SimpleRibbon = true;
	}

	Particle UpdateParticle(Particle particle, float deltaTime) override
	{

		particle.Transparency = mix(0.3f, 0.6f, (particle.deathTime - particle.lifeTime) / particle.deathTime);
		particle.Size = mix(0.015f, 0.055f, (particle.deathTime - particle.lifeTime) / particle.deathTime);
		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();

		particle.Size = 0.03f;
		particle.Color = vec4(0.5, 0.3, 0.3, 0.8);
		particle.Transparency = 0.8;
		particle.deathTime = 0.25f;

		return particle;
	}

private:

};

class CaneTrail : public ParticleSystem
{
public:
	CaneTrail()
	{
		emitters.push_back(new CaneTrailEmitter());
	}


private:

};

REGISTER_ENTITY(CaneTrail, "cane_trail")
