#include <Particle/GlobalParticleSystem.hpp>
#include <FogManager.h>

class particle_volumetricFogEmitter: public ParticleEmitter
{
public:

    particle_volumetricFogEmitter()
    {
        texture = "GameData/textures/particles/smoke.png";
        SpawnRate = 0;
        Emitting = true;

        MaxParticles = 1500;
        InitialSpawnCount = 1500;

        Duration = 1000000;

        PixelShader = "fog_particle";

    }

    Particle UpdateParticle(Particle particle, float deltaTime) override
    {
       
        float distanceToCamera = distance(Camera::position, particle.position);

        float yDist = distance(Camera::position.y, particle.position.y);

        if (yDist > 12)
        {
            particle.position.y = Camera::position.y + (RandomFloat() * 2 - 0.8f) * 5.0f;
        }

        if (distanceToCamera > 40)
        {
            vec3 oldPos = particle.position;
            particle = GetNewParticle();

            float newY = particle.position.y;

            particle.position = Camera::position + -1.0f * (oldPos - Camera::position);
            particle.position.y = newY;

            return particle;
        }

        

        float fadeIn = MathHelper::MapRange(particle.lifeTime, 0, 2, 0, 1);

        float fadeOut = MathHelper::MapRange(particle.lifeTime, particle.UserValue1 - 2, particle.UserValue1, 1, 0);

        if (fadeOut == 0)
        {
            particle = GetNewParticle();
            return particle;
        }

        particle.Transparency = MathHelper::MapRange(distanceToCamera, 1, 20, 0.07, 0.1f) * fadeIn * fadeOut 
            * MathHelper::MapRange(distanceToCamera, 2, 7, 0.5, 1) * 0.6f;

        particle.Size = MathHelper::MapRange(distanceToCamera, 3, 30, 8, 12);

        return particle;
    }

    Particle GetNewParticle() override
    {
        Particle particle = ParticleEmitter::GetNewParticle();
        
        particle.Size = 3;
        particle.Transparency = 0.0f;

        particle.position = Camera::position + MathHelper::XZ(RandomPosition(35)) + vec3(0,RandomFloat()*2 - 0.8f, 0) * 4.0f;

        particle.lifeTime = 0;

        particle.rotation = RandomFloat() * 360;

        particle.UserValue1 = 4 + RandomFloat() * 4;

        particle.deathTime = 1000000;

        return particle;
    }

private:

    float RandomFloat() {
        static thread_local std::mt19937 gen(std::random_device{}());
        static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(gen);
    }

    glm::vec3 RandomPosition(float radius) {
        // Random direction on sphere
        float u = RandomFloat();
        float v = RandomFloat();
        float theta = 2.0f * glm::pi<float>() * u;
        float phi = acosf(2.0f * v - 1.0f);

        float x = sinf(phi) * cosf(theta);
        float y = sinf(phi) * sinf(theta);
        float z = cosf(phi);

        glm::vec3 dir(x, y, z);

        // Random radius with cubic root so distribution is uniform in volume
        float r = radius * cbrtf(RandomFloat());

        return dir * r;
    }


    glm::vec3 GetForwardFromRotation(const glm::vec3& eulerDegrees) const {
        glm::quat rotation = glm::quat(glm::radians(eulerDegrees));
        return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
    }
};


class VolumetricFogSystem : public GlobalParticleSystem
{
public:

    particle_volumetricFogEmitter* particle_fog;

    VolumetricFogSystem()
    {

        FogManager::Opacity = 1;

        particle_fog = new particle_volumetricFogEmitter();

        emitters.push_back(particle_fog);
    }




private:

    void SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale)
    {
        Position = position;
        Rotation = rotation;
        Scale = scale;



        particle_fog->Position = position;
        particle_fog->Rotation = rotation;
        particle_fog->Scale = scale;



        particle_fog->SpawnParticles(ceil(scale.x));

    }

};


REGISTER_ENTITY(VolumetricFogSystem, "fog")