#pragma once

#include "ParticleEmitter.h"
#include "../VertexData.h"
#include "../Camera.h"
#include "../glm.h"

#include <bgfx/bgfx.h>
#include <vector>

class TrailEmitter : public ParticleEmitter {
public:
    TrailEmitter();
    ~TrailEmitter();

    bool SimpleRibbon = false;

    // Builds ribbon geometry and uploads it into transient bgfx buffers.
    // Returns false if there is nothing to draw.
    bool RenderRibbon(const std::vector<Particle>& inParticles, bgfx::TransientVertexBuffer& tvb, bgfx::TransientIndexBuffer& tib);

    int GetPrimitiveCount() const { return primitiveCount; }

    void FinalizeFrameData();
    void DrawForward(mat4x4 view, mat4x4 projection);

    bool IsCameraVisible() { return true; }

    void PreFinalize() override {}

private:
    void GenerateIndices(std::vector<uint32_t>& dst, int n);

    std::vector<VertexData>  verts;
    std::vector<uint32_t>    idxs;

    int primitiveCount = 0;
};
