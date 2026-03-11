#pragma once

#include "ParticleEmitter.h"
#include "../VertexData.h"
#include "../Camera.h"
#include "../glm.h"
#include <vector>

class RibbonEmitter : public ParticleEmitter {
public:
    RibbonEmitter();
    ~RibbonEmitter();

    bool SimpleRibbon = false;

    void RenderRibbon(const std::vector<Particle>& particles);

    int GetPrimitiveCount() const { return primitiveCount; }

    void FinalizeFrameData();

    void DrawForward(mat4x4 view, mat4x4 projection);

    bool IsCameraVisible() { return true; }



private:
    void GenerateIndices(std::vector<int>& dst, int count);


    std::vector<VertexData> verts;
    std::vector<int>     idxs;


    int lastCount = 0;
    int primitiveCount = 0;
};
