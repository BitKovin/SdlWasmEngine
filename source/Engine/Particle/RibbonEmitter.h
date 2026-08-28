#pragma once

#include "ParticleEmitter.h"
#include "../VertexData.h"
#include "../Camera.h"
#include "../glm.h"

#include <bgfx/bgfx.h>
#include <vector>

class RibbonEmitter : public ParticleEmitter {
public:
    RibbonEmitter();
    ~RibbonEmitter();

    bool SimpleRibbon = false;

    int GetPrimitiveCount() const { return primitiveCount; }

    // Parallel: builds vertex/index data from finalizedParticles.
    void PreFinalize() override;

    // Main thread: resolves the texture, commits verts/idxs onto ribbonCommand.
    void FinalizeFrameData();

    void CollectDrawCommands(std::vector<IDrawCommand*>& outCommands) override
    {
        outCommands.push_back(&ribbonCommand);
    }

    bool IsCameraVisible() { return true; }

private:
    void GenerateIndices(std::vector<uint32_t>& dst, int n);

    // Pure CPU geometry build - formerly RenderRibbon, split out of DrawForward.
    bool BuildRibbonGeometry(const std::vector<Particle>& particles);

    RibbonDrawCommand ribbonCommand;

    std::vector<VertexData>  verts;
    std::vector<uint32_t>    idxs;

    int primitiveCount = 0;
};
