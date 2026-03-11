#pragma once

#include <vector>
#include <mutex>
#include <algorithm>   // for std::remove_if

#include "../Physics.h"

#include "../IDrawMesh.h"

#include "../glm.h"
#include "../gl.h"

#include "../VertexData.h"

#include "../AssetRegistry.h"

struct Particle {
	glm::vec3 position = vec3();
	glm::vec3 position2 = vec3();
	glm::vec3 velocity = vec3();
	float lifeTime = 0.0f;
	float deathTime = 5.0f;  // example default lifetime
	bool Collided = false;
	bool HasCollision = false;
	float CollisionRadius = 0.5f; // example radius value
	float BouncePower = 1.0f;
	int id = 0;
	glm::vec3 globalRotation = vec3(); // you might instead use a quaternion (glm::quat) if needed

	bool UseWorldRotation = false;

	float Transparency = 1;

	float Size = 1;
	float rotation = 0;

	float UserValue1 = 0;
	float UserValue2 = 0;
	float UserValue3 = 0;
	float UserValue4 = 0;

	vec4 Color = vec4(1);

};

// -----------------------------
// ParticleEmitter class
// -----------------------------
class ParticleEmitter : public IDrawMesh
{
public:
	ParticleEmitter()
		: currentId(0), elapsedTime(0.0f), Emitting(true), destroyed(false),
		InitialSpawnCount(10), Duration(10.0f), SpawnRate(1.0f),
		Position(0.0f), Rotation(0.0f) 
	{

		Transparent = true;

	}

	// Call once to initialize particles.
	void Start()
	{
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		SpawnParticles(InitialSpawnCount);
	}

	// Spawn a given number of new particles.
	void SpawnParticles(int num);

	// Adds a particle to the main container (thread-safe using recursive mutex).
	void AddParticle(const Particle& particle)
	{
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		Particles.push_back(particle);
	}

	// Update all particles. deltaTime is the elapsed time since last call.
	void Update(float deltaTime);


	// Update a single particle.
	virtual Particle UpdateParticle(Particle particle, float deltaTime) 
	{
		glm::vec3 oldPos = particle.position;
		particle.position += particle.velocity * deltaTime;
		particle.Collided = false;

		if (!particle.HasCollision)
			return particle;


		auto hit = Physics::SphereTrace(oldPos, particle.position, particle.CollisionRadius, BodyType::World);
		if (!hit.hasHit)
			return particle;

		particle.Collided = true;
		particle.position = hit.position;
		if (glm::dot(particle.velocity, hit.normal) < 0) {
			particle.velocity = glm::reflect(particle.velocity * particle.BouncePower, hit.normal);
			particle.position = hit.position + hit.normal * (particle.CollisionRadius + 0.02f);
		}

		return particle;
	}

	// Create and return a new Particle with a unique id and initial parameters.
	virtual Particle GetNewParticle() 
	{
		// Locking here is optional if GetNewParticle() is only called within already locked sections.
		// If there's any chance of a race condition with currentId, lock here.
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		currentId++;
		Particle p;
		p.position = Position;
		p.id = currentId;
		p.globalRotation = Rotation;
		p.HasCollision = false; // default value, can be modified as needed.
		// Additional initialization (e.g., velocity) can be added here.
		return p;
	}

	void DrawForward(mat4x4 view, mat4x4 projection);

	void FinalizeFrameData();

	int MaxParticles = 0;

	// Data members
	std::vector<Particle> finalizedParticles; // finalized copy of active particles.
	std::vector<Particle> Particles;            // primary container holding current particles.
	std::recursive_mutex particlesMutex;        // recursive mutex for thread safety.

	// Other emitter state variables.
	int currentId;
	float elapsedTime;
	bool Emitting;
	bool destroyed;

	// Configuration parameters (customize as needed).
	int InitialSpawnCount;
	float Duration;       // total duration during which particles are emitted.
	float SpawnRate;      // particles per second.
	glm::vec3 Position = vec3(0);   // emitter's position.
	glm::vec3 Rotation = vec3(0);   // emitter's rotation; could be represented by a quaternion (glm::quat) if preferred.
	glm::vec3 Scale = vec3(1);

	bool isDecal;

	string texture = "";

	string PixelShader = "unlit_pixel";

	bool DepthSorting = true;

	static void InitBilboardVaoIfNeeded();

	void PreloadAssets()
	{
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		AssetRegistry::GetTextureFromFile(texture);
	}

protected:

	Texture* savedTexture = nullptr;
	string savedTextureName = "";

	static vec3 GetLightForParticle(const Particle& particle);

private:





	std::vector<InstanceData> instances;

	static void SetInstanceData(std::vector<InstanceData>& instanceData);

};