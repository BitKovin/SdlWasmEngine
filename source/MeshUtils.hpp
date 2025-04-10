#pragma once

#include "glm.h"

#include <vector>

using namespace std;

class MeshUtils
{
public:
    struct VerticesIndices
    {
        vector<vec3> vertices;
        vector<uint32_t> indices;
    };

    static VerticesIndices MergeMeshes(vector<VerticesIndices> meshes)
    {
        VerticesIndices merged;
        // This offset represents the current vertex index in the merged result.
        uint32_t vertexOffset = 0;

        // Loop through each mesh to merge.
        for (const auto& mesh : meshes)
        {
            // Add the vertices from the current mesh to the merged list.
            merged.vertices.insert(merged.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

            // For each index in the current mesh, add the offset to maintain proper index references.
            for (const auto& index : mesh.indices)
            {
                merged.indices.push_back(index + vertexOffset);
            }

            // Update the vertexOffset for the next mesh.
            vertexOffset += static_cast<uint32_t>(mesh.vertices.size());
        }

        return merged;
    }
};
