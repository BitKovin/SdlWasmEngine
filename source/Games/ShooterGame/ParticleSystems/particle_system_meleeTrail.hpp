#include <Particle/GlobalParticleSystem.hpp>
#include "TrailEmitter.h"

// ---------------------------------------------------------------------------
// particle_meleeTrail
//
// A TrailEmitter that draws a short-lived red melee-swing ribbon.
// pos2 is the secondary anchor point set each frame by the owning system
// (used as particle.position2 so the trail knows the blade's other end).
// ---------------------------------------------------------------------------
class particle_meleeTrail : public TrailEmitter
{
public:
    vec3 pos2;

    particle_meleeTrail()
    {
        texture             = "GameData/textures/particles/trail.png";
        InitialSpawnCount   = 2;
        SpawnRate           = 70;
        Emitting            = true;
        IsViewmodel           = false;
    }


    Particle UpdateParticle(Particle particle, float deltaTime) override
    {
        particle = TrailEmitter::UpdateParticle(particle, deltaTime);

        // Keep scale in [0, 1] — mirrors C# Math.Clamp(particle.Scale, 0, 1).
        particle.Size = glm::clamp(particle.Size, 0.0f, 1.0f);

        return particle;
    }

    Particle GetNewParticle() override
    {
        Particle particle = TrailEmitter::GetNewParticle();

        // Store the blade's secondary tip so the trail renderer can orient itself.
        // C#: particle.position2 = pos2
        particle.position2 = pos2;

        particle.BouncePower  = 0.1f;
        particle.Transparency = 1.0f;
        particle.deathTime    = 0.1f;

        // C#: new Vector4(1, 0.2f, 0.2f, 0.4f) / 1.5f
        particle.Color = glm::vec4(1.0f, 0.2f, 0.2f, 0.4f) / 1.5f;

        return particle;
    }
};

// ---------------------------------------------------------------------------
// particle_system_meleeTrail
//
// GlobalParticleSystem wrapper.  Position2 is the blade's secondary tip;
// it is forwarded to the emitter every async tick so freshly spawned
// particles always receive the current tip position.
// ---------------------------------------------------------------------------
class particle_system_meleeTrail : public ParticleSystem
{
public:
    vec3 Position2;

    particle_system_meleeTrail()
    {
        particle_MeleeTrail = new particle_meleeTrail();
        emitters.push_back(particle_MeleeTrail);
    }

    void AsyncUpdate() override
    {
        // Mirror C# AsyncUpdate: push Position2 into the emitter each frame
        // before the base class ticks it, so GetNewParticle sees the latest value.
        particle_MeleeTrail->pos2 = Position2;

        ParticleSystem::AsyncUpdate();
    }

private:
    particle_meleeTrail* particle_MeleeTrail = nullptr;
};

REGISTER_ENTITY(particle_system_meleeTrail, "meleeTrail")
