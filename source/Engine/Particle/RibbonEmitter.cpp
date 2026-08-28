#include "RibbonEmitter.h"
#include "../MathHelper.hpp"
#include "../Renderer/Renderer.h"
#include "../ShaderManager.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

// Constructor / Destructor
RibbonEmitter::RibbonEmitter()
{
    DepthSorting = false;
    ParticleCulling = false;
    PixelShader = "fs_unlit";
}

RibbonEmitter::~RibbonEmitter()
{
}

// GenerateIndices
void RibbonEmitter::GenerateIndices(std::vector<uint32_t>& dst, int n)
{
    dst.resize((n - 1) * 6);
    for (int i = 0; i < n - 1; ++i)
    {
        int      b  = i * 6;
        uint32_t v0 = i * 2;
        uint32_t v1 = i * 2 + 1;
        uint32_t v2 = i * 2 + 2;
        uint32_t v3 = i * 2 + 3;

        dst[b + 0] = v0; dst[b + 1] = v2; dst[b + 2] = v1;
        dst[b + 3] = v1; dst[b + 4] = v2; dst[b + 5] = v3;
    }
}

// BuildRibbonGeometry (formerly RenderRibbon - see RibbonDrawCommand::DrawForward for the transient-buffer half that used to live here)
bool RibbonEmitter::BuildRibbonGeometry(const std::vector<Particle>& inParticles)
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

    const vec3 camPos = Camera::finalizedPosition;

    for (int i = 0; i < n; ++i)
    {
        const Particle& p = particles[i];
        const vec3      P = p.position;

        vec3 dir = (i < n - 1)
            ? glm::normalize(P - particles[i + 1].position)
            : glm::normalize(particles[i - 1].position - P);

        vec3 camFwd = glm::normalize(P - camPos);
        vec3 perp = glm::normalize(glm::cross(dir, camFwd));

        const float half = p.Size * 0.5f;
        const int   b = i * 2;
        const float u = arcLen[i] * invLen;  // arc-length U
        vec3  light = vec3(1.0f);

        if(ParticleLighting)
            light = GetLightForParticle(p);

        const vec4  color = p.Color * vec4(light, 1.0f) * vec4(1.0f, 1.0f, 1.0f, p.Transparency);
        const vec3  nrm = vec3(0.0f, 1.0f, 0.0f);

        verts[b + 0].Position = P + perp * half;
        verts[b + 0].Normal = nrm;
        verts[b + 0].TextureCoordinate = vec2(u, 0.0f);
        verts[b + 0].Color = color;

        verts[b + 1].Position = P - perp * half;
        verts[b + 1].Normal = nrm;
        verts[b + 1].TextureCoordinate = vec2(u, 1.0f);
        verts[b + 1].Color = color;
    }

    primitiveCount = static_cast<int>(idxCount) / 3;

    return true;
}

// PreFinalize - parallel; finalizedParticles commit moved here to match ParticleEmitter's own pattern.
void RibbonEmitter::PreFinalize()
{
    {
        std::lock_guard<std::recursive_mutex> lock(particlesMutex);
        finalizedParticles = Particles;
    }

    BuildRibbonGeometry(finalizedParticles);
}

// FinalizeFrameData - main thread: resolve texture, commit verts/idxs onto ribbonCommand.
void RibbonEmitter::FinalizeFrameData()
{
    if (savedTextureName != texture)
    {
        savedTexture = AssetRegistry::GetTextureFromFile(texture);
        savedTextureName = texture;
    }

    ribbonCommand.Vertices = verts;
    ribbonCommand.Indices = idxs;
    ribbonCommand.ResolvedTexture = savedTexture;
    ribbonCommand.PixelShader = PixelShader;
    ribbonCommand.BlendMode = BlendMode;

    SyncCommandFlags(ribbonCommand);
}
