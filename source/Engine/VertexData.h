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
    glm::vec2 TextureCoordinate2 = glm::vec2();
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
            .add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Float)   // a_indices (bone indices as float)
            .add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float)   // a_weight
            .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)   // a_textureCoordinates2
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)   // a_color0
            .add(bgfx::Attrib::TexCoord2, 2, bgfx::AttribType::Float)   // a_texcoord1  (ShadowMapCoords)
            .end();
        return layout;
    }
};

struct InstanceData{
    glm::vec4 model[4];     // exactly i_data0..i_data3 (columns, shader expects this order)
    glm::vec4 Color;        // i_data4

    // Allocates a bgfx instance buffer and returns a typed pointer to fill.
    static InstanceData* Alloc(bgfx::InstanceDataBuffer& idb, uint32_t count) {
        assert(bgfx::getAvailInstanceDataBuffer(count, sizeof(InstanceData)) == count
            && "Not enough instance buffer space");

        bgfx::allocInstanceDataBuffer(&idb, count, sizeof(InstanceData));
        return reinterpret_cast<InstanceData*>(idb.data);
    }
};

static_assert(sizeof(InstanceData) == 80,
    "InstanceData must be exactly 80 bytes (4×vec4 + vec4) — no padding allowed on any platform!");