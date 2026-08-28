#pragma once

#include <vector>
#include <mutex>
#include <algorithm>   // for std::remove_if

#include "../Physics.h"

#include "../IDrawMesh.h"
#include "ParticleDrawCommand.h"

#include "../glm.h"

#include <bgfx/bgfx.h>

#include "../VertexData.h"

#include "../AssetRegistry.h"

#include <BgfxStateManager.h>

struct Particle {
	glm::vec3 position = vec3();
	glm::vec3 position2 = vec3();
	glm::vec3 velocity = vec3();
	float lifeTime = 0.0f;
	float deathTime = 5.0f;
	bool Collided = false;
	bool HasCollision = false;
	float CollisionRadius = 0.5f;
	float BouncePower = 1.0f;
	int id = 0;
	glm::vec3 globalRotation = vec3();

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

class ParticleEmitter : public IDrawMesh
{
public:
	ParticleEmitter()
		: currentId(0), elapsedTime(0.0f), Emitting(true), destroyed(false),
		InitialSpawnCount(10), Duration(10.0f), SpawnRate(1.0f),
		Position(0.0f), Rotation(0.0f)
	{
		Transparent = true; // no-op now, kept for compatibility - see IDrawMesh::Transparent
		PixelShader = "fs_unlit";
	}

	mat4 RelativeTransform = glm::identity<mat4>();

	void Start()
	{
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		SpawnParticles(InitialSpawnCount);
	}

	void SpawnParticles(int num);

	void AddParticle(const Particle& particle)
	{
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		Particles.push_back(particle);
	}

	virtual void Update(float deltaTime);

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

	virtual Particle GetNewParticle()
	{
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		currentId++;
		Particle p;
		p.position = Position;
		p.id = currentId;
		p.globalRotation = Rotation;
		p.HasCollision = false;
		return p;
	}


	void SnapLastParticleToEmitterPosition() {
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		if (!Particles.empty()) {
			Particles.back().position = Position;
		}
	}

	// Parallel: commits Particles into finalizedParticles and builds preFinalizedInstances.
	void PreFinalize() override;

	// Main thread: resolves the texture and commits everything onto drawCommand.
	void FinalizeFrameData();

	void CollectDrawCommands(std::vector<IDrawCommand*>& outCommands) override
	{
		outCommands.push_back(&drawCommand);
	}

	int MaxParticles = 0;

	std::vector<Particle> finalizedParticles;
	std::vector<Particle> Particles;
	std::recursive_mutex particlesMutex;

	int currentId = -1;
	float elapsedTime = 0;
	float emitterTime = 0;
	bool Emitting = true;
	bool destroyed = false;

	int InitialSpawnCount = 0;
	float Duration = 100000;
	float SpawnRate = 0;
	glm::vec3 Position = vec3(0);
	glm::vec3 Rotation = vec3(0);
	glm::vec3 Scale = vec3(1);

	bool isDecal = false;

	string texture = "";

	string PixelShader = "fs_unlit";

	bool DepthSorting = true;

	bool ParticleLighting = true;

	bool ParticleCulling = true;

	BgfxStateManager::Blend BlendMode = BgfxStateManager::Blend::Alpha;

	static void InitBilboardVaoIfNeeded();
	static void DestroyBillboardVao();

	// Stable handles into geometry shared by every emitter - safe for ParticleDrawCommand to bind without an owner pointer.
	static bgfx::VertexBufferHandle GetBillboardVbh() { return s_billboardVbh; }
	static bgfx::IndexBufferHandle GetBillboardIbh() { return s_billboardIbh; }

	void PreloadAssets()
	{
		std::lock_guard<std::recursive_mutex> lock(particlesMutex);
		AssetRegistry::GetTextureFromFile(texture);
	}

protected:

	Texture* savedTexture = nullptr;
	string savedTextureName = "";

	vec3 GetLightForParticle(const Particle& particle);

	// Ribbon/TrailEmitter own their own command types, but still need to sync these three flags - kept available to them.
	void SyncCommandFlags(IDrawCommand& cmd) const
	{
		cmd.IsViewmodel = IsViewmodel;
		cmd.OnlyShadows = OnlyShadows;
		cmd.ReceiveDetailShadows = ReceiveDetailShadows;
	}

private:

	ParticleDrawCommand drawCommand;

	std::vector<InstanceData> preFinalizedInstances;

	// Shared billboard geometry — created once, reused by all emitters
	static bgfx::VertexBufferHandle s_billboardVbh;
	static bgfx::IndexBufferHandle  s_billboardIbh;
	static bgfx::VertexLayout       s_billboardLayout;
};
