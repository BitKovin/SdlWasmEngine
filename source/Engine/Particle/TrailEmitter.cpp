#include "TrailEmitter.h"
#include "../MathHelper.hpp"
#include "../Renderer/Renderer.h"
#include "../ShaderManager.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

// Constructor / Destructor
TrailEmitter::TrailEmitter()
{
    DepthSorting = false;
    ParticleCulling = false;
}

TrailEmitter::~TrailEmitter()
{}

// GenerateIndices
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

// BuildRibbonGeometry (formerly RenderRibbon)
bool TrailEmitter::BuildRibbonGeometry(const std::vector<Particle>& inParticles)
{
    primitiveCount = 0;
    verts.clear();
    idxs.clear();

    if (inParticles.size() < 2 || destroyed)
        return false;

    std::vector<Particle> particles = inParticles;
    if (SimpleRibbon)
        particles = { inParticles.front(), inParticles.back() };

    const int      n = static_cast<int>(particles.size());
    const uint32_t vCount = static_cast<uint32_t>(n * 2);
    const uint32_t idxCount = static_cast<uint32_t>((n - 1) * 6);

    verts.resize(vCount);
    GenerateIndices(idxs, n);

    // Pre-compute arc-length at each knot
    std::vector<float> arcLen(n, 0.0f);
    for (int i = 1; i < n; ++i)
        arcLen[i] = arcLen[i - 1] + glm::length(particles[i].position - particles[i - 1].position);

    const float totalLen = arcLen[n - 1];
    const float invLen = (totalLen > 0.0f) ? 1.0f / totalLen : 1.0f;

    for (int i = 0; i < n; ++i)
    {
        const Particle& p = particles[i];
        const vec3      P = p.position;

        const vec3 forward = MathHelper::GetForwardVector(p.globalRotation);

        const float half = p.Size * 0.5f;

        vec3 p1 = P + forward * half;
        vec3 p2 = P - forward * half;

        p1 = MathHelper::TransformVector(p1, RelativeTransform);
        p2 = MathHelper::TransformVector(p2, RelativeTransform);

        const float u = arcLen[i] * invLen;
        vec3  light = GetLightForParticle(p);

        if (ParticleLighting == false)
            light = vec3(1.0f);

        const vec4  color = p.Color * vec4(light, 1.0f) * vec4(1.0f, 1.0f, 1.0f, p.Transparency);
        const vec3  nrm = vec3(0.0f, 1.0f, 0.0f);

        const int b = i * 2;

        verts[b + 0].Position = p1;
        verts[b + 0].Normal = nrm;
        verts[b + 0].TextureCoordinate = vec2(u, 0.0f);
        verts[b + 0].Color = color;

        verts[b + 1].Position = p2;
        verts[b + 1].Normal = nrm;
        verts[b + 1].TextureCoordinate = vec2(u, 1.0f);
        verts[b + 1].Color = color;
    }

    primitiveCount = static_cast<int>(idxCount) / 3;

    return true;
}

// PreFinalize — runs in parallel, same reasoning as RibbonEmitter.
void TrailEmitter::PreFinalize()
{
    {
        std::lock_guard<std::recursive_mutex> lock(particlesMutex);
        finalizedParticles = Particles;
    }

    BuildRibbonGeometry(finalizedParticles);
}

// FinalizeFrameData — main thread.
void TrailEmitter::FinalizeFrameData()
{
    if (savedTextureName != texture)
    {
        savedTexture = AssetRegistry::GetTextureFromFile(texture);
        savedTextureName = texture;
    }

    trailCommand.Vertices = verts;
    trailCommand.Indices = idxs;
    trailCommand.ResolvedTexture = savedTexture;
    trailCommand.PixelShader = PixelShader;
    trailCommand.BlendMode = BlendMode;

    SyncCommandFlags(trailCommand);
}
