#include "TrailEmitter.h"
#include "../MathHelper.hpp"
#include "../Renderer/Renderer.h"
#include "../ShaderManager.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
TrailEmitter::TrailEmitter()
{
    DepthSorting = false;
}

TrailEmitter::~TrailEmitter()
{}

// ---------------------------------------------------------------------------
// GenerateIndices
// ---------------------------------------------------------------------------
void TrailEmitter::GenerateIndices(std::vector<uint32_t>& dst, int n)
{
    dst.resize((n - 1) * 6);
    for (int i = 0; i < n - 1; ++i)
    {
        int      b = i * 6;
        uint32_t v0 = i * 2;
        uint32_t v1 = i * 2 + 1;
        uint32_t v2 = i * 2 + 2;
        uint32_t v3 = i * 2 + 3;

        dst[b + 0] = v0; dst[b + 1] = v2; dst[b + 2] = v1;
        dst[b + 3] = v1; dst[b + 4] = v2; dst[b + 5] = v3;
    }
}

// ---------------------------------------------------------------------------
// RenderRibbon
// ---------------------------------------------------------------------------
bool TrailEmitter::RenderRibbon(const std::vector<Particle>& inParticles)
{
    primitiveCount = 0;

    if (inParticles.size() < 2 || destroyed)
        return false;

    std::vector<Particle> particles = inParticles;
    if (SimpleRibbon)
        particles = { inParticles.front(), inParticles.back() };

    const int      n = static_cast<int>(particles.size());
    const uint32_t vCount = static_cast<uint32_t>(n * 2);
    const uint32_t idxCount = static_cast<uint32_t>((n - 1) * 6);

    const bgfx::VertexLayout layout = VertexData::Declaration();

    if (bgfx::getAvailTransientVertexBuffer(vCount, layout) < vCount)   return false;
    if (bgfx::getAvailTransientIndexBuffer(idxCount, true) < idxCount) return false;

    verts.resize(vCount);
    GenerateIndices(idxs, n);

    const vec3 camPos = Camera::finalizedPosition;

    for (int i = 0; i < n; ++i)
    {
        const Particle& p = inParticles[i];
        const vec3      P = p.position;

        // Derive the particle's local forward vector from its rotation
        // quaternion (+Z column of the rotation matrix).
        // C# equivalent: particle.globalRotation.GetForwardVector()
        const vec3 forward = MathHelper::GetForwardVector(p.globalRotation);

        const float half = p.Size * 0.5f;  // C# uses particle.Scale / 2

        // "Top" and "bottom" edges of the ribbon strip at this knot.
        // C# equivalent (before RelativeMatrix — omitted, see note above):
        //   p1 = p + halfSize
        //   p2 = p - halfSize
        vec3 p1 = P + forward * half;
        vec3 p2 = P - forward * half;

        p1 = MathHelper::TransformVector(p1, RelativeTransform);
        p2 = MathHelper::TransformVector(p2, RelativeTransform);

        const float u = static_cast<float>(i) / static_cast<float>(n - 1);
        const vec3  light = GetLightForParticle(p);
        const vec4  color = p.Color * vec4(light, 1.0f) * vec4(1.0f, 1.0f, 1.0f, p.Transparency);
        const vec3  nrm = vec3(0.0f, 1.0f, 0.0f);

        const int b = i * 2;

        verts[b + 0].Position = p1;
        verts[b + 0].Normal = nrm;
        verts[b + 0].TextureCoordinate = vec2(u, 0.0f);
        verts[b + 0].Color = color;
        verts[b + 0].SmoothNormal = nrm;

        verts[b + 1].Position = p2;
        verts[b + 1].Normal = nrm;
        verts[b + 1].TextureCoordinate = vec2(u, 1.0f);
        verts[b + 1].Color = color;
        verts[b + 1].SmoothNormal = nrm;
    }

    primitiveCount = static_cast<int>(idxCount) / 3;

    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer  tib;

    bgfx::allocTransientVertexBuffer(&tvb, vCount, layout);
    bgfx::allocTransientIndexBuffer(&tib, idxCount, true);

    memcpy(tvb.data, verts.data(), vCount * sizeof(VertexData));
    memcpy(tib.data, idxs.data(), idxCount * sizeof(uint32_t));

    bgfx::setVertexBuffer(0, &tvb);
    bgfx::setIndexBuffer(&tib);

    return true;
}

// ---------------------------------------------------------------------------
// FinalizeFrameData
// ---------------------------------------------------------------------------
void TrailEmitter::FinalizeFrameData()
{
    std::lock_guard<std::recursive_mutex> lock(particlesMutex);
    finalizedParticles = Particles;
}

// ---------------------------------------------------------------------------
// DrawForward
// ---------------------------------------------------------------------------
void TrailEmitter::DrawForward(mat4x4 view, mat4x4 projection)
{
    if (finalizedParticles.size() < 2) return;

    if (savedTextureName != texture)
    {
        savedTexture = AssetRegistry::GetTextureFromFile(texture);
        savedTextureName = texture;
    }

    auto startState = BgfxStateManager::GetState();

    BgfxStateManager::SetWriteDepth(true);
    BgfxStateManager::SetCull(BgfxStateManager::Cull::None);

    Shader* shader = ShaderManager::GetShaderProgram("vs_default", PixelShader);
    if (shader == nullptr) return;

    shader->UseProgram();

    shader->SetUniform("view", view);
    shader->SetUniform("projection", projection);
    shader->SetUniform("world", glm::identity<mat4>());
    shader->SetUniform("isViewmodel", false);
    shader->SetUniform("is_particle", true);
    shader->SetUniform("is_decal", false);

    Renderer::SetSurfaceShaderUniforms(shader);

    shader->SetTexture("u_texture", savedTexture);

    if (!RenderRibbon(finalizedParticles))
    {
        BgfxStateManager::SetState(startState);
        return;
    }

    BgfxStateManager::Apply();
    shader->Submit(ViewIdManager::GetCurrentId());

    BgfxStateManager::SetState(startState);
}
