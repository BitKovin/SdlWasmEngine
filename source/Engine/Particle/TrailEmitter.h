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

    int GetPrimitiveCount() const { return primitiveCount; }

    // Parallel: builds vertex/index data from finalizedParticles (see RibbonEmitter::PreFinalize).
    void PreFinalize() override;

    // Main thread: resolves the texture, commits verts/idxs onto trailCommand.
    void FinalizeFrameData();

    void CollectDrawCommands(std::vector<IDrawCommand*>& outCommands) override
    {
        outCommands.push_back(&trailCommand);
    }

    bool IsCameraVisible() { return true; }

private:
    void GenerateIndices(std::vector<uint32_t>& dst, int n);

    // Pure CPU geometry build - formerly RenderRibbon, split out of DrawForward.
    bool BuildRibbonGeometry(const std::vector<Particle>& particles);

    TrailDrawCommand trailCommand;

    std::vector<VertexData>  verts;
    std::vector<uint32_t>    idxs;

    int primitiveCount = 0;
};
