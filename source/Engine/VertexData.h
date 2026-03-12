#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <cstdint>  // For uint8_t

struct VertexData {
    glm::vec3 Position = glm::vec3();
    glm::vec3 Normal = glm::vec3();
    glm::vec2 TextureCoordinate = glm::vec2();
    glm::vec3 Tangent = glm::vec3();
    glm::vec3 BiTangent = glm::vec3();
    float     BlendIndices[4] = { 0, 0, 0, 0 };
    glm::vec4 BlendWeights = glm::vec4();
    glm::vec3 SmoothNormal = glm::vec3();
    glm::vec4 Color = glm::vec4(1.0f);
    glm::vec2 ShadowMapCoords = glm::vec2(0.0f);

    static bgfx::VertexLayout Declaration() {
        bgfx::VertexLayout layout;
        layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)   // a_position
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)   // a_normal
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)   // a_texcoord0  (UV)
            .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)   // (unused in shader, but keeps stride)
            .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)   // (unused in shader, but keeps stride)
            .add(bgfx::Attrib::TexCoord3, 4, bgfx::AttribType::Float)   // a_indices (bone indices as float)
            .add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float)   // a_weight
            .add(bgfx::Attrib::TexCoord1, 3, bgfx::AttribType::Float)   // a_smoothNormal
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)   // a_color0
            .add(bgfx::Attrib::TexCoord2, 2, bgfx::AttribType::Float)   // a_texcoord1  (ShadowMapCoords)
            .end();
        return layout;
    }
};

struct InstanceData {
    glm::mat4 ModelMatrix;  // i_data0..i_data3 (4x vec4)
    glm::vec4 Color;        // i_data4          (1x vec4)

    // Allocates a bgfx instance buffer and returns a typed pointer to fill.
    // Usage:
    //   bgfx::InstanceDataBuffer idb;
    //   InstanceData* instances = InstanceData::Alloc(idb, count);
    //   instances[0] = { matrix, color };
    //   bgfx::setInstanceDataBuffer(&idb);
    static InstanceData* Alloc(bgfx::InstanceDataBuffer& idb, uint32_t count) {
        assert(bgfx::getAvailInstanceDataBuffer(count, sizeof(InstanceData)) == count
            && "Not enough instance buffer space");

        bgfx::allocInstanceDataBuffer(&idb, count, sizeof(InstanceData));
        return reinterpret_cast<InstanceData*>(idb.data);
    }
};

static_assert(sizeof(InstanceData) % 16 == 0,
    "bgfx requires instance data stride to be a multiple of 16 bytes");