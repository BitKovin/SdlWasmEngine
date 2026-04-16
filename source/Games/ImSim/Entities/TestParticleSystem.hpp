#pragma once

#include <Particle/GlobalParticleSystem.hpp>

class TestParticleEmitter : public ParticleEmitter
{
public:
	
	TestParticleEmitter()
	{
		texture = "GameData/cat.png";
		InitialSpawnCount = 0;
		SpawnRate = 0;
		Duration = 100000000;
	}
	
	Particle GetNewParticle() 
	{
		Particle p = ParticleEmitter::GetNewParticle();

		p.HasCollision = true;

		p.velocity = Camera::Forward() * -3.0f;

		return p;
	}

private:

};


class TestParticleSystem : public GlobalParticleSystem
{

private:

	static TestParticleSystem* Instance;

public:

	TestParticleSystem()
	{
		emitters.push_back(new TestParticleEmitter());
	}


};

