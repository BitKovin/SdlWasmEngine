// editor/vertex_edit.cpp
#include "vertex_edit.h"
#include "raycast.h"        // getFacePolygon
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <limits>

namespace editor {

// ─────────────────────────────────────────────────────────────────────────────
// posNear — must be defined before applyVertexMoves which calls it
// ─────────────────────────────────────────────────────────────────────────────

static bool posNear(glm::vec3 a, glm::vec3 b, float eps = kEpsilon * 10.f) noexcept {
    return glm::length(a - b) < eps;
}

// ─────────────────────────────────────────────────────────────────────────────
// refitFacePlane
// ─────────────────────────────────────────────────────────────────────────────

bool refitFacePlane(Face& face, const std::vector<glm::vec3>& polygon)
{
    if (polygon.size() < 3) return false;

    glm::vec3 bestNormal(0.f);
    float     bestLen2 = 0.f;
    size_t    bestI    = 0;

    for (size_t i = 0; i < polygon.size(); ++i) {
        const glm::vec3& a = polygon[i];
        const glm::vec3& b = polygon[(i + 1) % polygon.size()];
        const glm::vec3& c = polygon[(i + 2) % polygon.size()];

        glm::vec3 cross = glm::cross(b - a, c - a);
        float     len2  = glm::dot(cross, cross);

        if (len2 > bestLen2) {
            bestLen2   = len2;
            bestNormal = cross;
            bestI      = i;
        }
    }

    if (bestLen2 < kEpsilon * kEpsilon) return false;

    bestNormal = glm::normalize(bestNormal);

    face.plane.normal = bestNormal;
    face.plane.dist   = glm::dot(bestNormal, polygon[0]);

    face.points[0] = polygon[bestI];
    face.points[1] = polygon[(bestI + 1) % polygon.size()];
    face.points[2] = polygon[(bestI + 2) % polygon.size()];

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// isBrushConvexAndValid
// ─────────────────────────────────────────────────────────────────────────────

bool isBrushConvexAndValid(const Brush& brush)
{
    if (brush.faces.size() < 4) return false;

    for (size_t fi = 0; fi < brush.faces.size(); ++fi) {
        std::vector<glm::vec3> poly =
            getFacePolygon(brush, static_cast<uint32_t>(fi));
        if (poly.size() < 3) return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// applyVertexMoves
// ─────────────────────────────────────────────────────────────────────────────

typedef std::pair<uint32_t, glm::vec3> VertexMove;

static EditResult applyVertexMoves(
    Brush& brush,
    const std::vector<VertexMove>& moves)
{
    if (!brush.topology.valid) return EditResult::Invalid;
    if (moves.empty())         return EditResult::Ok;

    // faceIndex → updated polygon
    std::unordered_map<uint32_t, std::vector<glm::vec3>> updatedPolygons;

    // Pass 1: for each moved vertex, update the polygons of all owning faces.
    for (size_t mi = 0; mi < moves.size(); ++mi) {
        uint32_t          vertIdx = moves[mi].first;
        const glm::vec3&  newPos  = moves[mi].second;

        if (vertIdx >= static_cast<uint32_t>(brush.topology.vertices.size()))
            return EditResult::Invalid;

        const glm::vec3& oldPos = brush.topology.vertices[vertIdx].position;

        const std::vector<uint32_t>& owningFaces =
            brush.topology.vertices[vertIdx].faceIndices;

        for (size_t fi = 0; fi < owningFaces.size(); ++fi) {
            uint32_t faceIdx = owningFaces[fi];

            if (updatedPolygons.find(faceIdx) == updatedPolygons.end())
                updatedPolygons[faceIdx] = getFacePolygon(brush, faceIdx);

            std::vector<glm::vec3>& poly = updatedPolygons[faceIdx];
            for (size_t pi = 0; pi < poly.size(); ++pi)
                if (posNear(poly[pi], oldPos))
                    poly[pi] = newPos;
        }
    }

    // Pass 2: apply remaining moves to already-seeded polygons
    // (handles the case where two vertices on the same face both move).
    for (auto it = updatedPolygons.begin(); it != updatedPolygons.end(); ++it) {
        std::vector<glm::vec3>& poly = it->second;
        for (size_t mi = 0; mi < moves.size(); ++mi) {
            const glm::vec3& oldPos =
                brush.topology.vertices[moves[mi].first].position;
            const glm::vec3& newPos = moves[mi].second;
            for (size_t pi = 0; pi < poly.size(); ++pi)
                if (posNear(poly[pi], oldPos))
                    poly[pi] = newPos;
        }
    }

    // Trial brush — check validity before committing.
    Brush trial = brush;

    for (auto it = updatedPolygons.begin(); it != updatedPolygons.end(); ++it) {
        if (!refitFacePlane(trial.faces[it->first], it->second))
            return EditResult::Rejected;
    }

    if (!isBrushConvexAndValid(trial))
        return EditResult::Rejected;

    // Commit.
    for (auto it = updatedPolygons.begin(); it != updatedPolygons.end(); ++it)
        refitFacePlane(brush.faces[it->first], it->second);

    buildBrushMesh(brush);
    buildTopology(brush);

    return EditResult::Ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// moveVertex / moveVertexBy
// ─────────────────────────────────────────────────────────────────────────────

EditResult moveVertex(Brush& brush, uint32_t vertIndex, glm::vec3 newPosition)
{
    std::vector<VertexMove> moves;
    moves.push_back(VertexMove(vertIndex, newPosition));
    return applyVertexMoves(brush, moves);
}

EditResult moveVertexBy(Brush& brush, uint32_t vertIndex, glm::vec3 delta)
{
    if (!brush.topology.valid) return EditResult::Invalid;
    if (vertIndex >= static_cast<uint32_t>(brush.topology.vertices.size()))
        return EditResult::Invalid;
    glm::vec3 newPos = brush.topology.vertices[vertIndex].position + delta;
    return moveVertex(brush, vertIndex, newPos);
}

EditResult moveSelectedVerticesBy(Brush& brush, glm::vec3 delta)
{
    if (!brush.topology.valid) return EditResult::Invalid;

    std::vector<VertexMove> moves;
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(brush.topology.vertices.size()); ++i)
    {
        if (brush.topology.vertices[i].selected) {
            glm::vec3 newPos = brush.topology.vertices[i].position + delta;
            moves.push_back(VertexMove(i, newPos));
        }
    }
    if (moves.empty()) return EditResult::Ok;
    return applyVertexMoves(brush, moves);
}

// ─────────────────────────────────────────────────────────────────────────────
// slideEdge / moveEdgeBy / moveSelectedEdgesBy
// ─────────────────────────────────────────────────────────────────────────────

EditResult slideEdge(Brush& brush, uint32_t edgeIndex,
                      glm::vec3 direction, float distance)
{
    return moveEdgeBy(brush, edgeIndex, glm::normalize(direction) * distance);
}

EditResult moveEdgeBy(Brush& brush, uint32_t edgeIndex, glm::vec3 delta)
{
    if (!brush.topology.valid) return EditResult::Invalid;
    if (edgeIndex >= static_cast<uint32_t>(brush.topology.edges.size()))
        return EditResult::Invalid;

    const TopoEdge& edge = brush.topology.edges[edgeIndex];

    glm::vec3 posA = brush.topology.vertices[edge.vertA].position + delta;
    glm::vec3 posB = brush.topology.vertices[edge.vertB].position + delta;

    std::vector<VertexMove> moves;
    moves.push_back(VertexMove(edge.vertA, posA));
    moves.push_back(VertexMove(edge.vertB, posB));
    return applyVertexMoves(brush, moves);
}

EditResult moveSelectedEdgesBy(Brush& brush, glm::vec3 delta)
{
    if (!brush.topology.valid) return EditResult::Invalid;

    std::vector<VertexMove> moves;

    auto alreadyQueued = [&](uint32_t vi) -> bool {
        for (size_t i = 0; i < moves.size(); ++i)
            if (moves[i].first == vi) return true;
        return false;
    };

    for (uint32_t ei = 0;
         ei < static_cast<uint32_t>(brush.topology.edges.size()); ++ei)
    {
        const TopoEdge& e = brush.topology.edges[ei];
        if (!e.selected) continue;

        if (!alreadyQueued(e.vertA)) {
            glm::vec3 newPos = brush.topology.vertices[e.vertA].position + delta;
            moves.push_back(VertexMove(e.vertA, newPos));
        }
        if (!alreadyQueued(e.vertB)) {
            glm::vec3 newPos = brush.topology.vertices[e.vertB].position + delta;
            moves.push_back(VertexMove(e.vertB, newPos));
        }
    }

    if (moves.empty()) return EditResult::Ok;
    return applyVertexMoves(brush, moves);
}

} // namespace editor
