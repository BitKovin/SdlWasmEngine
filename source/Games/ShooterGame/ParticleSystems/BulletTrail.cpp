#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

class BulletTrailEmitter : public RibbonEmitter
{
public:

	float trailScale = 1.0f;
	float lifeTimeMultiplier = 1.0f;
	
	BulletTrailEmitter() : RibbonEmitter()
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
		
		particle.Transparency = mix(1.0,1.0, (particle.deathTime - particle.lifeTime) / particle.deathTime);
		particle.Size = mix(0.015f, 0.055f, (particle.deathTime - particle.lifeTime) / particle.deathTime) * trailScale;
		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();
		
		particle.Size = 0.03f;
		particle.Color = vec4(0.16, 0.03, 0.03, 0.65);
		particle.Transparency = 0.8;
		particle.deathTime = 0.1f * lifeTimeMultiplier;

		return particle;
	}

private:

};

class BulletTrail : public ParticleSystem
{
public:

	BulletTrailEmitter* emitter = nullptr;

	BulletTrail()
	{
		emitter = new BulletTrailEmitter();
		emitters.push_back(emitter);
	}


	void Update() override
	{
		ParticleSystem::Update();

		emitter->trailScale = Scale.x;
		emitter->lifeTimeMultiplier = Scale.y;

	}

private:

};

REGISTER_ENTITY(BulletTrail, "bullet_trail")
