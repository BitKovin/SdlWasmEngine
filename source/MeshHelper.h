#pragma once
#include <vector>

#include "Shader.hpp"
#include "ShaderManager.h"

#include "VertexData.h"


// Helper function to create and add vertices
void AddVertex(std::vector<VertexData>& vertices,
    const glm::vec3& position,
    const glm::vec3& normal,
    const glm::vec3& tangent,
    const glm::vec3& bitangent,
    const glm::vec2& texCoord) 
{
    VertexData v;
    v.Position = position;
    v.Normal = normal;
    v.TextureCoordinate = texCoord;
    v.Tangent = tangent;
    v.BiTangent = bitangent;
    v.BlendIndices = glm::vec4(0);
    v.BlendWeights = glm::vec4(1, 0, 0, 0);
    v.SmoothNormal = normal;
    v.Color = glm::vec4(1.0f);
    vertices.push_back(v);
}

std::vector<VertexData> GenerateCubeVertices(float size = 1.0f) 
{
    std::vector<VertexData> vertices;
    const float halfSize = size * 0.5f;

    // Helper lambda now takes const references to arrays
    auto AddFace = [&](const glm::vec3& normal,
        const glm::vec3& tangent,
        const glm::vec3& bitangent,
        const glm::vec3(&positions)[4],
        const glm::vec2(&texCoords)[4]) {
            // Triangle 1
            AddVertex(vertices, positions[0], normal, tangent, bitangent, texCoords[0]);
            AddVertex(vertices, positions[1], normal, tangent, bitangent, texCoords[1]);
            AddVertex(vertices, positions[2], normal, tangent, bitangent, texCoords[2]);

            // Triangle 2
            AddVertex(vertices, positions[0], normal, tangent, bitangent, texCoords[0]);
            AddVertex(vertices, positions[2], normal, tangent, bitangent, texCoords[2]);
            AddVertex(vertices, positions[3], normal, tangent, bitangent, texCoords[3]);
        };

    // Front face
    {
        const glm::vec3 positions[4] = {
            {-halfSize, -halfSize, halfSize},
            {halfSize, -halfSize, halfSize},
            {halfSize, halfSize, halfSize},
            {-halfSize, halfSize, halfSize}
        };
        const glm::vec2 texCoords[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
        AddFace({ 0,0,1 }, { 1,0,0 }, { 0,1,0 }, positions, texCoords);
    }

    // Back face
    {
        const glm::vec3 positions[4] = {
            {halfSize, -halfSize, -halfSize},
            {-halfSize, -halfSize, -halfSize},
            {-halfSize, halfSize, -halfSize},
            {halfSize, halfSize, -halfSize}
        };
        const glm::vec2 texCoords[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
        AddFace({ 0,0,-1 }, { -1,0,0 }, { 0,1,0 }, positions, texCoords);
    }

    // Left face
    {
        const glm::vec3 positions[4] = {
            {-halfSize, -halfSize, -halfSize},
            {-halfSize, -halfSize, halfSize},
            {-halfSize, halfSize, halfSize},
            {-halfSize, halfSize, -halfSize}
        };
        const glm::vec2 texCoords[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
        AddFace({ -1,0,0 }, { 0,0,1 }, { 0,1,0 }, positions, texCoords);
    }

    // Right face
    {
        const glm::vec3 positions[4] = {
            {halfSize, -halfSize, halfSize},
            {halfSize, -halfSize, -halfSize},
            {halfSize, halfSize, -halfSize},
            {halfSize, halfSize, halfSize}
        };
        const glm::vec2 texCoords[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
        AddFace({ 1,0,0 }, { 0,0,-1 }, { 0,1,0 }, positions, texCoords);
    }

    // Top face
    {
        const glm::vec3 positions[4] = {
            {-halfSize, halfSize, halfSize},
            {halfSize, halfSize, halfSize},
            {halfSize, halfSize, -halfSize},
            {-halfSize, halfSize, -halfSize}
        };
        const glm::vec2 texCoords[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
        AddFace({ 0,1,0 }, { 1,0,0 }, { 0,0,1 }, positions, texCoords);
    }

    // Bottom face
    {
        const glm::vec3 positions[4] = {
            {-halfSize, -halfSize, -halfSize},
            {halfSize, -halfSize, -halfSize},
            {halfSize, -halfSize, halfSize},
            {-halfSize, -halfSize, halfSize}
        };
        const glm::vec2 texCoords[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
        AddFace({ 0,-1,0 }, { 1,0,0 }, { 0,0,-1 }, positions, texCoords);
    }

    return vertices;
}
