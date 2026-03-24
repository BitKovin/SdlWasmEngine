#pragma once

#include "glm.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <numeric>
#include <functional>

#include <algorithm>

#include "VertexData.h"

using namespace std;

class MeshUtils
{
public:
    struct PositionVerticesIndices
    {
        vector<vec3>     vertices;
        vector<uint32_t> indices;
    };

    struct VerticesIndices
    {
        vector<VertexData> vertices;
        vector<uint32_t>   indices;
    };

    /// Merge a bunch of meshes together
    static PositionVerticesIndices MergeMeshes(const vector<PositionVerticesIndices>& meshes)
    {
        PositionVerticesIndices merged;
        uint32_t vertexOffset = 0;
        for (auto const& mesh : meshes)
        {
            merged.vertices.insert(merged.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
            for (auto idx : mesh.indices)
                merged.indices.push_back(idx + vertexOffset);
            vertexOffset += uint32_t(mesh.vertices.size());
        }
        return merged;
    }

    static VerticesIndices MergeMeshes(const vector<VerticesIndices>& meshes)
    {
        MeshUtils::VerticesIndices mergedMesh;

        // Reserve memory to avoid multiple reallocations, improving performance.
        size_t totalVertexCount = 0;
        size_t totalIndexCount = 0;
        for (const auto& mesh : meshes) {
            totalVertexCount += mesh.vertices.size();
            totalIndexCount += mesh.indices.size();
        }
        mergedMesh.vertices.reserve(totalVertexCount);
        mergedMesh.indices.reserve(totalIndexCount);

        uint32_t baseVertexIndex = 0;

        for (const auto& mesh : meshes)
        {
            // 1. Append the new vertices to the merged list.
            mergedMesh.vertices.insert(mergedMesh.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

            // 2. Append the new indices, but add the baseVertexIndex to each one.
            for (uint32_t index : mesh.indices)
            {
                mergedMesh.indices.push_back(index + baseVertexIndex);
            }

            // 3. Update the base index for the next mesh.
            baseVertexIndex += static_cast<uint32_t>(mesh.vertices.size());
        }

        return mergedMesh;
    }

    // --------------------------------------------------
    // RemoveDegenerates: weld very-close vertices & drop degenerate triangles
    // --------------------------------------------------

    static PositionVerticesIndices RemoveDegenerates(const PositionVerticesIndices& mesh,
        float distanceThreshold,
        float areaEpsilon)
    {
        if (mesh.vertices.empty())
            return PositionVerticesIndices{ {}, {} };

        // --- Merge close vertices via union-find ---
        vector<size_t> parent(mesh.vertices.size());
        iota(parent.begin(), parent.end(), 0);
        function<size_t(size_t)> findRoot = [&](size_t i) -> size_t {
            return parent[i] == i ? i : (parent[i] = findRoot(parent[i]));
            };
        auto unionSets = [&](size_t a, size_t b) {
            size_t ra = findRoot(a), rb = findRoot(b);
            if (ra != rb) parent[ra] = rb;
            };

        // Spatial sort by X
        vector<size_t> sorted(mesh.vertices.size());
        iota(sorted.begin(), sorted.end(), 0);
        std::sort(sorted.begin(), sorted.end(), [&](size_t a, size_t b) {
            return mesh.vertices[a].x < mesh.vertices[b].x;
            });

        for (size_t i = 0; i < sorted.size(); ++i)
        {
            size_t ai = sorted[i];
            for (size_t j = i + 1; j < sorted.size(); ++j)
            {
                size_t bi = sorted[j];
                if (mesh.vertices[bi].x > mesh.vertices[ai].x + distanceThreshold)
                    break;
                if (length(mesh.vertices[ai] - mesh.vertices[bi]) < distanceThreshold)
                    unionSets(ai, bi);
            }
        }

        // Build new vertex groups
        vector<size_t> root(mesh.vertices.size());
        for (size_t i = 0; i < root.size(); ++i)
            root[i] = findRoot(i);

        unordered_map<size_t, vector<size_t>> groups;
        for (size_t i = 0; i < root.size(); ++i)
            groups[root[i]].push_back(i);

        vector<vec3> newVerts;
        newVerts.reserve(groups.size());
        vector<size_t> oldToNew(mesh.vertices.size());
        for (auto& kv : groups)
        {
            vec3 avg(0.0f);
            for (auto idx : kv.second)
                avg += mesh.vertices[idx];
            avg /= float(kv.second.size());
            size_t newIdx = newVerts.size();
            newVerts.push_back(avg);
            for (auto idx : kv.second)
                oldToNew[idx] = newIdx;
        }

        // Remove tiny/degenerate triangles
        vector<uint32_t> newIdxs;
        for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
        {
            size_t i0 = oldToNew[mesh.indices[i]];
            size_t i1 = oldToNew[mesh.indices[i + 1]];
            size_t i2 = oldToNew[mesh.indices[i + 2]];
            if (i0 == i1 || i1 == i2 || i2 == i0)
                continue;
            vec3 A = newVerts[i0];
            vec3 B = newVerts[i1];
            vec3 C = newVerts[i2];
            float area = 0.5f * length(cross(B - A, C - A));
            if (area > areaEpsilon)
            {
                newIdxs.push_back(uint32_t(i0));
                newIdxs.push_back(uint32_t(i1));
                newIdxs.push_back(uint32_t(i2));
            }
        }

        return PositionVerticesIndices{ newVerts, newIdxs };
    }

    static PositionVerticesIndices ExpandHorizontalTriangles(const PositionVerticesIndices& mesh, float radius);

    // --------------------------------------------------
    // MergeCoplanarRegions: flood-fill + re-triangulate planar patches
    // --------------------------------------------------

    struct Edge {
        uint32_t a, b;

        // Constructor normalizes so that a < b
        Edge(uint32_t v0, uint32_t v1) {
            if (v0 < v1) {
                a = v0;
                b = v1;
            }
            else {
                a = v1;
                b = v0;
            }
        }

        // Default constructor (for unordered_map compatibility)
        Edge() : a(0), b(0) {}
    };


    struct EdgeHash {
        size_t operator()(Edge const& e) const noexcept {
            // Combine a and b into a single 64-bit key: (a << 32) | b
            uint64_t key = (static_cast<uint64_t>(e.a) << 32) | e.b;
            return std::hash<uint64_t>()(key);
        }
    };

    struct EdgeEq {
        bool operator()(Edge const& e1, Edge const& e2) const noexcept {
            return e1.a == e2.a && e1.b == e2.b;
        }
    };

    // You can tweak this based on the precision of your mesh.
    static constexpr float POSITION_EPSILON = 1e-4f;

    struct Vec3Hash {
        size_t operator()(glm::vec3 const& p) const noexcept {
            // Quantize position to a discrete grid
            int64_t xi = static_cast<int64_t>(std::floor(p.x / POSITION_EPSILON));
            int64_t yi = static_cast<int64_t>(std::floor(p.y / POSITION_EPSILON));
            int64_t zi = static_cast<int64_t>(std::floor(p.z / POSITION_EPSILON));

            // Combine into a 64-bit key: we assume each coordinate fits in 21 bits
            // (so: |xi|,|yi|,|zi| < 2^20). You can adjust bit-shifts if your coordinate range is bigger.
            uint64_t key =
                ((uint64_t)(xi & 0x1FFFFF) << 42) |
                ((uint64_t)(yi & 0x1FFFFF) << 21) |
                ((uint64_t)(zi & 0x1FFFFF));

            return std::hash<uint64_t>()(key);
        }
    };

    // Compare two vec3’s “equal” if they’re within POSITION_EPSILON in each axis
    struct Vec3Eq {
        bool operator()(glm::vec3 const& a, glm::vec3 const& b) const noexcept {
            return (std::fabs(a.x - b.x) <= POSITION_EPSILON
                && std::fabs(a.y - b.y) <= POSITION_EPSILON
                && std::fabs(a.z - b.z) <= POSITION_EPSILON);
        }
    };

    static std::pair<std::vector<glm::vec3>, std::vector<uint32_t>>
        WeldVertices(
            std::vector<glm::vec3> const& inVertices,
            std::vector<uint32_t>  const& inIndices);

    // ---------------------------------------------------------------------
// Check if point P lies on the segment AB in 3D, within a given epsilon.
// Returns true if: 
//   1) P is colinear with A and B (|cross(AP, AB)| / |AB| < epsilon), 
//   2) the projection of P lies between A and B (dot(AP, AB) ≥ 0 and dot(BP, BA) ≥ 0).
// ---------------------------------------------------------------------
    static bool IsPointOnSegment3D(const vec3& P, const vec3& A, const vec3& B, float epsilon);

    // ---------------------------------------------------------------------
    // Split a single triangle (at index triIndex) by inserting an existing
    // vertexIndex (which lies on one of its edges). We assume that vertexIndex
    // indeed lies on exactly one edge of that triangle. This function modifies
    // mesh.indices in place by replacing the original triangle with two new ones.
    // 
    // Input: 
    //   mesh           : mesh to modify
    //   triIndex       : index into mesh.indices / 3 (i.e., which triangle to split)
    //   edgeV0, edgeV1 : the two endpoint indices of the edge on which vertexIndex lies
    //   vertexIndex    : the index of the vertex that lies on the edge (in mesh.vertices)
    // 
    // Process: Replace triangle [i0, i1, i2] with two triangles that include vertexIndex.
    // ---------------------------------------------------------------------
    static void SplitTriangleAtVertex(PositionVerticesIndices& mesh,
        uint32_t triIndex,
        uint32_t edgeV0,
        uint32_t edgeV1,
        uint32_t vertexIndex);

    // ---------------------------------------------------------------------
    // Resolve T-junctions for an entire mesh. For every triangle edge,
    // if any other vertex in the mesh lies on that edge (within epsilon),
    // split that triangle so that the vertex becomes an explicit corner.
    // 
    // Input:
    //   mesh    : mesh to modify in place (will change mesh.indices and possibly reuse existing vertices)
    //   epsilon : distance tolerance for “on-edge” test (e.g. 1e-4f)
    // 
    // Process:
    //   Repeatedly scan all triangles. For each triangle T and each of its three edges (vA,vB),
    //   look for any vertex V (not equal to vA or vB) in mesh.vertices such that
    //   IsPointOnSegment3D(mesh.vertices[V], mesh.vertices[vA], mesh.vertices[vB], epsilon) == true.
    //   If found, immediately call SplitTriangleAtVertex(...) and restart scanning from first triangle.
    //   Continue until no more splits are needed.
    // ---------------------------------------------------------------------
    static void ResolveTJunctions(PositionVerticesIndices& mesh, float epsilon);

    static PositionVerticesIndices MergeCoplanarRegions(const PositionVerticesIndices& mesh, float normalTolerance = 0.99f);



private:
    // Simple boundary chaining: pick a start and walk matching edges
    static vector<uint32_t> ChainBoundary(const vector<Edge>& edges);
};
