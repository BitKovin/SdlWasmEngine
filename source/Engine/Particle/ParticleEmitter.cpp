#include "ParticleEmitter.h"

#include "../MathHelper.hpp"
#include "../ShaderManager.h"
#include "../FrustrumCull.hpp"
#include "../BoundingSphere.hpp"
#include "../Renderer/Renderer.h"
#include "../BSP/Quake3Bsp.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
bgfx::VertexBufferHandle ParticleEmitter::s_billboardVbh   = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle  ParticleEmitter::s_billboardIbh   = BGFX_INVALID_HANDLE;
bgfx::VertexLayout       ParticleEmitter::s_billboardLayout = {};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
mat4 GetWorldMatrix(const Particle& particle)
{
    return translate(particle.position) * MathHelper::GetRotationMatrix(particle.globalRotation) * scale(vec3(particle.Size));
}

// ---------------------------------------------------------------------------
// Spawn / Update
// ---------------------------------------------------------------------------
void ParticleEmitter::SpawnParticles(int num)
{
    std::lock_guard<std::recursive_mutex> lock(particlesMutex);
    for (int i = 0; i < num; i++) {
        Particle particle = GetNewParticle();
        particle.deathTime *= i + 1;
        AddParticle(particle);
    }
}

void ParticleEmitter::Update(float deltaTime)
{
    if (destroyed)
        return;

    std::lock_guard<std::recursive_mutex> lock(particlesMutex);

    elapsedTime += deltaTime;
    if (elapsedTime > Duration)
        Emitting = false;

    const float spawnInterval = (SpawnRate > 0.0f) ? (1.0f / SpawnRate) : 0.0f;

    if (SpawnRate > 0.0f && Emitting && spawnInterval > 0.0f)
    {
        while (elapsedTime >= spawnInterval)
        {
            Particles.push_back(GetNewParticle());
            elapsedTime -= spawnInterval;
        }
    }

    for (auto& p : Particles)
        p.lifeTime += deltaTime;

    Particles.erase(
        std::remove_if(Particles.begin(), Particles.end(),
            [](const Particle& p) { return p.lifeTime >= p.deathTime; }),
        Particles.end());

    if (MaxParticles > 0 && Particles.size() > MaxParticles)
    {
        const size_t excess = Particles.size() - MaxParticles;
        Particles.erase(
            Particles.begin(),
            Particles.begin() + static_cast<std::ptrdiff_t>(excess));
    }

    for (auto& p : Particles)
        p = UpdateParticle(p, deltaTime);

    if (!Emitting && Particles.empty())
        destroyed = true;
}

// ---------------------------------------------------------------------------
// InitBilboardVaoIfNeeded
// ---------------------------------------------------------------------------
void ParticleEmitter::InitBilboardVaoIfNeeded()
{
    if (bgfx::isValid(s_billboardVbh)) return;

    // Exact match to shader $input — this is what the official bgfx example does
    s_billboardLayout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    struct BillboardVertex {
        float x, y, z;
        float u, v;
    };

    BillboardVertex vertices[4] = {
        {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f, 1.0f},
        { 0.5f,  0.5f, 0.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f, 0.0f, 0.0f, 0.0f}
    };

    uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

    s_billboardVbh = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), s_billboardLayout);
    s_billboardIbh = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
}

void ParticleEmitter::DestroyBillboardVao()
{

	if (bgfx::isValid(s_billboardVbh))
    {
        bgfx::destroy(s_billboardVbh);
        s_billboardVbh = BGFX_INVALID_HANDLE;
    }

	if (bgfx::isValid(s_billboardIbh))
    {
        bgfx::destroy(s_billboardIbh);
        s_billboardIbh = BGFX_INVALID_HANDLE;
    }

}

// ---------------------------------------------------------------------------
// DrawForward
// ---------------------------------------------------------------------------
void ParticleEmitter::DrawForward(mat4x4 view, mat4x4 projection)
{
    if (instances.empty()) return;

    InitBilboardVaoIfNeeded();

    if (!bgfx::isValid(s_billboardVbh) || !bgfx::isValid(s_billboardIbh)) return;

    // Resolve texture
    if (savedTextureName != texture)
    {
        savedTexture = AssetRegistry::GetTextureFromFile(texture);
        savedTextureName = texture;
    }

    auto startState = BgfxStateManager::GetState();

    BgfxStateManager::SetWriteDepth(false);
    BgfxStateManager::SetCull(BgfxStateManager::Cull::None);

    Shader* shader = ShaderManager::GetShaderProgram("vs_instanced_billboard", PixelShader);
    if (shader == nullptr) return;

    // -----------------------------------------------------------------------
    // Instance buffer handling – exactly as in the bgfx instancing example
    // -----------------------------------------------------------------------
    const uint32_t totalInstances = static_cast<uint32_t>(instances.size());
    const uint16_t instanceStride = static_cast<uint16_t>(sizeof(InstanceData));

    // Query how many instances we can fit in the transient instance buffer
    uint32_t drawInstances = bgfx::getAvailInstanceDataBuffer(totalInstances, instanceStride);
    if (drawInstances == 0)
        return; // nothing to draw this frame

    // Allocate the transient instance buffer
    bgfx::InstanceDataBuffer idb;
    bgfx::allocInstanceDataBuffer(&idb, drawInstances, instanceStride);

    // Copy instance data (only the first drawInstances entries)
    memcpy(idb.data, instances.data(), drawInstances * instanceStride);

    // Optional: report if we couldn't draw all instances (for debugging)
    // uint32_t missing = totalInstances - drawInstances;
    // if (missing > 0) DBG("ParticleEmitter: dropped %u instances", missing);
    // -----------------------------------------------------------------------

    shader->UseProgram();

    shader->SetUniform("view", view);
    shader->SetUniform("projection", projection);
    shader->SetUniform("is_decal", isDecal);          // only is_decal needed
    shader->SetUniform("isViewmodel", false);

    Renderer::SetSurfaceShaderUniforms(shader);

    shader->SetTexture("u_texture", savedTexture);

    bgfx::setVertexBuffer(0, s_billboardVbh);
    bgfx::setIndexBuffer(s_billboardIbh);
    bgfx::setInstanceDataBuffer(&idb);

    BgfxStateManager::Apply();

    shader->Submit(ViewIdManager::GetCurrentId());

    BgfxStateManager::SetState(startState);
}

// ---------------------------------------------------------------------------
// FinalizeFrameData
// ---------------------------------------------------------------------------
void ParticleEmitter::FinalizeFrameData()
{
    {
        std::lock_guard<std::recursive_mutex> lock(particlesMutex);
        finalizedParticles = Particles;
    }

    const vec3 cameraPosition = Camera::finalizedPosition;
    const vec3 cameraRotation = Camera::finalizedRotation;
    const vec3 cameraForward  = MathHelper::GetForwardVector(cameraRotation);
    const vec3 cameraRight    = MathHelper::GetRightVector(cameraRotation);
    const vec3 cameraUp       = MathHelper::GetUpVector(cameraRotation);

    const int cameraC = Level::Current->BspData.FindClusterAtPosition(cameraPosition);

    instances.clear();

    if (DepthSorting)
    {
        std::vector<std::pair<float, InstanceData>> visible;
        visible.reserve(finalizedParticles.size());

        for (const auto& particle : finalizedParticles)
        {
            if (!Camera::frustum.IsSphereVisible(particle.position, particle.Size))
                continue;

            int targetC = Level::Current->BspData.FindClusterAtPosition(particle.position);
            if (!Level::Current->BspData.IsClusterVisible(cameraC, targetC))
                continue;

            mat4x4 world;
            if (particle.UseWorldRotation)
                world = GetWorldMatrix(particle);
            else
                world = MathHelper::CreateBillboardMatrix(
                    particle.position, cameraPosition,
                    cameraForward, cameraRight, cameraUp,
                    vec3(particle.Size), particle.rotation);

            InstanceData data{};
            data.model[0] = world[0];  // column 0
            data.model[1] = world[1];
            data.model[2] = world[2];
            data.model[3] = world[3];
            data.Color = particle.Color * vec4(GetLightForParticle(particle), 1.0f);
            data.Color.a *= particle.Transparency;

            float depth = glm::dot(cameraForward, particle.position - cameraPosition);
            visible.emplace_back(depth, std::move(data));
        }

        if (!visible.empty())
        {
            std::sort(visible.begin(), visible.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });

            instances.reserve(visible.size());
            for (auto& v : visible)
                instances.push_back(std::move(v.second));
        }
    }
    else
    {
        instances.reserve(finalizedParticles.size());
        for (const auto& particle : finalizedParticles)
        {
            if (!Camera::frustum.IsSphereVisible(particle.position, particle.Size))
                continue;

            int targetC = Level::Current->BspData.FindClusterAtPosition(particle.position);
            if (!Level::Current->BspData.IsClusterVisible(cameraC, targetC))
                continue;

            mat4x4 world;
            if (particle.UseWorldRotation)
                world = GetWorldMatrix(particle);
            else
                world = MathHelper::CreateBillboardMatrix(
                    particle.position, cameraPosition,
                    cameraForward, cameraRight, cameraUp,
                    vec3(particle.Size), particle.rotation);

            InstanceData data{};
            data.model[0] = world[0];  // column 0
            data.model[1] = world[1];
            data.model[2] = world[2];
            data.model[3] = world[3];
            data.Color = particle.Color * vec4(GetLightForParticle(particle), 1.0f);
            data.Color.a *= particle.Transparency;
            instances.push_back(std::move(data));
        }

    }
}

// ---------------------------------------------------------------------------
// GetLightForParticle
// ---------------------------------------------------------------------------
vec3 ParticleEmitter::GetLightForParticle(const Particle& particle)
{
    if (particle.UseWorldRotation == false)
    {
        auto light = Level::Current->BspData.GetLightvolColorPoint(particle.position * MAP_SCALE);
        return (light.ambientColor + light.directColor) * 1.0f;
    }

    vec3 normal = MathHelper::GetForwardVector(particle.globalRotation);

    auto light = Level::Current->BspData.GetLightvolColorPoint((particle.position + normal) * MAP_SCALE) * 1.0f;

    float dirFactor = 1.0f;

    return light.ambientColor + light.directColor * dirFactor;
}
