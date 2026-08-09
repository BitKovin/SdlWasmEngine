#include <Particle/GlobalParticleSystem.hpp>

#include <World/WorldOrientationManager.h>

static vec3 bloodColor = vec3(0.5,0,0);

class BloodDecal : public ParticleEmitter
{
public:
	BloodDecal()
	{
		Duration = 10000000;
		InitialSpawnCount = 0;
		SpawnRate = 0;	
        texture = "GameData/textures/particles/smoke.png";

        Transparent = true;

		PixelShader = "fs_decal";

        isDecal = true;
        DepthSorting = false;
	}
	
    // Override UpdateParticle.
     Particle UpdateParticle(Particle particle, float deltaTime) override 
{
        // Call the base update first.
        particle = ParticleEmitter::UpdateParticle(particle, deltaTime);

        // Increase the scale based on lifetime.
        float incScale = particle.lifeTime / 9.0f;
        // Use glm::mix to linearly interpolate between 1 and 0.1.
        float lerpVal = glm::mix(1.0f, 0.1f, incScale * incScale);
        float maxVal = (lerpVal > 0.0f) ? lerpVal : 0.0f;

        if (particle.lifeTime < 15)
        {
            particle.Size += deltaTime * maxVal * 0.15f;
        }



        const float despawnTime = 4.0f;
        if ((particle.deathTime - particle.lifeTime - 0.1f) < despawnTime)
            particle.Transparency -= deltaTime / despawnTime;

        return particle;
    }

    // Override GetNewParticle.
     Particle GetNewParticle() override 
     {
        Particle particle = ParticleEmitter::GetNewParticle();

        particle.UseWorldRotation = true;

        // Compute the normal from the emitter's Rotation.
        glm::vec3 normal = Rotation;
        // Set globalRotation based on a "look-at" rotation.
        particle.globalRotation = MathHelper::FindLookAtRotation(glm::vec3(0.0f), normal);
        // Add a random angle (in degrees) on Z.
        float randomAngle = static_cast<float>(rand()) / (float)RAND_MAX * 360.0f;
        particle.globalRotation.z += randomAngle;

        // Slightly adjust particle position.
        particle.position += normal * 0.01f;

        particle.Size = 1;
        //particle.MaxDrawDistance = 60.0f;
        particle.BouncePower = 0.1f;

        // Generate a random deathTime in the range [0, 90] approximately.
        float randomVal = static_cast<float>(rand()) / (float)RAND_MAX;
        particle.deathTime = 60 * 30 + 30.0f * randomVal * 3.0f;

        // Set the color to a semi-transparent red.
        particle.Color = glm::vec4(bloodColor, 0.7f);

        return particle;
    }

     float GetDistanceToCamera() override
     {
         return 100000000;
     }

private:

};

class particle_hitBloodDrips : public ParticleEmitter
{
public:
    particle_hitBloodDrips()
    {
        texture = "GameData/textures/particles/blood.png";
        InitialSpawnCount = 0;
        SpawnRate = 0;
        Emitting = true;
        DepthSorting = false;
        Duration = 1000000;

    }

    Particle UpdateParticle(Particle particle, float deltaTime) override
    {
        particle.velocity -= WorldOrientationManager::GetUpVector()* 5.0f * (deltaTime / 2.0f);
        particle = ParticleEmitter::UpdateParticle(particle, deltaTime);
        particle.velocity -= WorldOrientationManager::GetUpVector() * 5.0f * (deltaTime / 2.0f);
        particle.Transparency = std::max(particle.Transparency - (deltaTime / 1.5f), 0.0f);
        return particle;
    }

    Particle GetNewParticle() override
    {
        Particle particle = ParticleEmitter::GetNewParticle();
        particle.velocity = Rotation;
        particle.Size = glm::mix(0.12f, 0.14f, RandomFloat());
        particle.Transparency = 1;
        particle.deathTime = 1.0f;
        particle.rotation = RandomFloat() * 500.0f;
        particle.Color = glm::vec4(bloodColor, 1.0f);
        return particle;
    }

private:
    float RandomFloat() const { return static_cast<float>(rand()) / (float)RAND_MAX; }
};

class particle_hitBloodPieces : public ParticleEmitter
{
public:
    particle_hitBloodDrips* drips;

    particle_hitBloodPieces()
    {
        texture = "GameData/textures/particles/blood.png";
        InitialSpawnCount = 0;
        SpawnRate = 0;
        Emitting = true;

        Duration = 1000000;

    }

    Particle UpdateParticle(Particle particle, float deltaTime) override
    {


        vec3 oldPos = particle.position;

        if (particle.lifeTime < 3.5f)
            particle.UserValue1 += deltaTime;


        float spawnInterval = 0.02f * Time::TimeScale;
        if (particle.UserValue1 > spawnInterval)
        {
            particle.UserValue1 -= spawnInterval;
            drips->Position = particle.position;
            drips->Rotation = particle.velocity / 3.0f;
            drips->SpawnParticles(1);

        }

        particle.velocity -= WorldOrientationManager::GetUpVector() * 10.0f * (deltaTime / 2.0f);
        particle = ParticleEmitter::UpdateParticle(particle, deltaTime);


        if (particle.UserValue2 < 1.9)
        {


            glm::vec3 dir = glm::normalize(particle.velocity);
            auto hit = Physics::LineTrace(
                particle.position2,
                particle.position,
                BodyType::World
            );
            if (hit.hasHit)
            {

                if (dot(hit.normal, particle.velocity) > 0)
                {
                    particle.deathTime = 0;
                    return particle;
                }

                if (RandomFloat() < ((particle.UserValue2 == 0) ? 0.5f : 0.5f))
                {

                    // Only place a decal if the surface under its *entire*
                    // footprint is solid. A single LineTrace only proves the
                    // hit point itself has geometry -- on meshes with holes
                    // (fences, grates, foliage, etc.) that's not enough, and
                    // the decal quad ends up straddling empty space.
                    if (IsSurfaceSolidForDecal(hit.position, hit.normal, DecalSize))
                    {
                        GlobalParticleSystem::SpawnParticleAt("decal_blood", hit.position, hit.normal, vec3(1));
                    }

                    if (particle.UserValue2 < 1.1)
                    {
                        particle.position = hit.position + hit.normal * (particle.CollisionRadius + 0.002f);
                        particle.velocity = reflect(particle.velocity, hit.normal) * particle.BouncePower;

                        particle.UserValue2 = 3;

                    }
                    particle.UserValue2++;

                }
            }

            particle.position2 = particle.position;

        }

        particle.velocity -= WorldOrientationManager::GetUpVector() * 10.0f * (deltaTime / 2.0f);
        particle.Transparency = std::max(particle.Transparency - (deltaTime / 3.0f), 0.0f);
        return particle;
    }

    Particle GetNewParticle() override
    {
        Particle particle = ParticleEmitter::GetNewParticle();
        particle.BouncePower = 0.5f;
        particle.CollisionRadius = 0.2f;
        particle.Size = 0.4;

        particle.position2 = Position;

        glm::vec3 randPos = RandomPosition(0.15f);
        particle.position += randPos;

        glm::vec3 randomDir = RandomPosition(1.0f);
        glm::vec3 velocity = glm::normalize(randomDir) * 1.5f;
        velocity.y += 0.8f * glm::mix(1.0f, 2.0f, RandomFloat());
        velocity += GetForwardFromRotation(Rotation) * 2.0f;
        velocity *= glm::mix(0.3f, 1.0f, RandomFloat());

        particle.position += velocity * 0.2f;

        particle.velocity = velocity;
        particle.Transparency = 1.7f;
        particle.deathTime = 3.0f;
        particle.rotation = RandomFloat() * 500.0f;
        particle.Color = glm::vec4(bloodColor, 1.0f);

        particle.UserValue3 = RandomFloat() / 3;

        particle.position2 = Position;

        return particle;
    }

private:

    // ---------------------------------------------------------------
    // Decal footprint validation
    // ---------------------------------------------------------------

    // World-space edge length of the blood decal quad spawned via
    // "decal_blood". Tune this to match the actual rendered size of
    // that decal (it's currently spawned with a scale of vec3(1), so
    // this should equal decal_blood's base world size).
    float DecalSize = 1.05f;

    // Number of probe rays per edge of the sample grid (3 = 9 rays:
    // corners, edge midpoints, center). Higher catches smaller holes
    // (thinner wires / smaller gaps) at the cost of more traces per
    // decal attempt. This only runs when a decal is about to be
    // spawned, not every frame, so it's cheap in practice.
    int DecalCheckResolution = 4; 

    // How far above/below the sampled surface point each probe ray
    // starts/ends. Needs to be big enough to punch through the local
    // surface but small enough not to poke through to geometry behind
    // thin meshes.
    float DecalProbeOffset = 0.3f;

    // Fraction of probe rays that must hit for the decal to be
    // considered valid. 1.0f = every single ray must hit (strictest,
    // what you asked for). Lower this (e.g. 0.85f) if legitimate flat
    // surfaces start rejecting decals near edges/seams too often.
    float DecalRequiredHitRatio = 0.8f;

    bool IsSurfaceSolidForDecal(const glm::vec3& hitPos, const glm::vec3& hitNormal, float decalSize) const
    {
        // Build a tangent basis on the hit plane so we can sample points
        // across the decal's footprint, not just its exact center.
        glm::vec3 refUp = WorldOrientationManager::GetUpVector();
        if (fabs(dot(refUp, hitNormal)) > 0.99f)
            refUp = glm::vec3(1.0f, 0.0f, 0.0f);

        glm::vec3 tangent = glm::normalize(glm::cross(refUp, hitNormal));
        glm::vec3 bitangent = glm::normalize(glm::cross(hitNormal, tangent));

        float halfSize = decalSize * 0.5f;
        int res = std::max(DecalCheckResolution, 1);
        int totalRays = res * res;
        int hits = 0;

        for (int x = 0; x < res; x++)
        {
            for (int y = 0; y < res; y++)
            {
                float u = (res == 1) ? 0.0f : ((float)x / (float)(res - 1) * 2.0f - 1.0f) * halfSize;
                float v = (res == 1) ? 0.0f : ((float)y / (float)(res - 1) * 2.0f - 1.0f) * halfSize;

                glm::vec3 samplePos = hitPos + tangent * u + bitangent * v;
                glm::vec3 rayStart = samplePos + hitNormal * DecalProbeOffset;
                glm::vec3 rayEnd = samplePos - hitNormal * DecalProbeOffset;

                auto probe = Physics::LineTrace(rayStart, rayEnd, BodyType::World);

                if (probe.hasHit)
                {
                    hits++;
                }
                else if (DecalRequiredHitRatio >= 1.0f)
                {
                    // Fast out: strict mode, one miss is enough to reject.
                    return false;
                }
            }
        }

        return (float)hits / (float)totalRays >= DecalRequiredHitRatio;
    }

    float RandomFloat() const { return static_cast<float>(rand()) / (float)RAND_MAX; }

    glm::vec3 RandomPosition(float radius) const {
        return glm::vec3(
            (RandomFloat() - 0.5f) * 2.0f * radius,
            (RandomFloat() - 0.5f) * 2.0f * radius,
            (RandomFloat() - 0.5f) * 2.0f * radius
        );
    }

    glm::vec3 GetForwardFromRotation(const glm::vec3& eulerDegrees) const {
        glm::quat rotation = glm::quat(glm::radians(eulerDegrees));
        return rotation * glm::vec3(0.0f, 0.0f, 1.0f);
    }
};

class BloodDecalSystem : public GlobalParticleSystem
{
public:
    
    BloodDecal* decal = nullptr;

    BloodDecalSystem()
    {

        decal = new BloodDecal();

        emitters.push_back(decal);
    }

    void SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale)
    {
        Position = position;
        Rotation = rotation;
        Scale = scale;

        for (auto emitter : emitters)
        {

            emitter->Position = position;
            emitter->Rotation = rotation;

            emitter->SpawnParticles(1);
        }
    }


private:

};

class BloodPieceSystem : public GlobalParticleSystem
{
public:

    particle_hitBloodPieces* particle_blood;
    particle_hitBloodDrips* particle_bloodDrips;

    BloodPieceSystem()
    {

        particle_blood = new particle_hitBloodPieces();
        particle_bloodDrips = new particle_hitBloodDrips();

        particle_blood->drips = particle_bloodDrips;

        emitters.push_back(particle_blood);
        emitters.push_back(particle_bloodDrips);
    }




private:

    void SpawnParticleAtInst(vec3 position, vec3 rotation, vec3 scale)
    {
        Position = position;
        Rotation = rotation;
        Scale = scale;



        particle_blood->Position = position;
        particle_blood->Rotation = rotation;
        particle_blood->Scale = scale;



        particle_blood->SpawnParticles(ceil(scale.x));

    }

};


REGISTER_ENTITY(BloodDecalSystem, "decal_blood")
REGISTER_ENTITY(BloodPieceSystem, "hit_flesh")