#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

class CannonBulletTrailEmitter : public RibbonEmitter
{
public:

	CannonBulletTrailEmitter() : RibbonEmitter()
	{
		texture = "GameData/textures/particles/trail.png";
		SpawnRate = 100;
		Emitting = true;
		InitialSpawnCount = 1;
		SimpleRibbon = true;
		ParticleLighting = false;
		PixelShader = "fs_unlit";
	}

	Particle UpdateParticle(Particle particle, float deltaTime) override
	{

		particle.Transparency = mix(1.0, 1.0, (particle.deathTime - particle.lifeTime) / particle.deathTime);
		particle.Size = mix(0.025f, 0.055f, (particle.deathTime - particle.lifeTime) / particle.deathTime) * 5.0f;
		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();

		particle.Size = 0.03f;
		particle.Color = vec4(0.36, 0.03, 0.03, 0.95);
		particle.Transparency = 0.9;
		particle.deathTime = 0.8f;

		return particle;
	}

private:

};

class CannonBulletTrail : public ParticleSystem
{
public:
	CannonBulletTrail()
	{
		emitters.push_back(new CannonBulletTrailEmitter());
	}


private:

};

REGISTER_ENTITY(CannonBulletTrail, "cannon_bullet_trail")
