#pragma once
// editor/topo_types.h
// ─────────────────────────────────────────────────────────────────────────────
// Pure-data topology structs that can be included by brush.h without creating
// a circular dependency (topology builder functions live in topology.h, which
// includes brush.h, not this file).
// ─────────────────────────────────────────────────────────────────────────────

#include "math_utils.h"
#include <vector>
#include <cstdint>

namespace editor {

// ─── TopoVertex ──────────────────────────────────────────────────────────────

struct TopoVertex {
    uint32_t  id       { 0 };
    glm::vec3 position { 0.f };
    bool      selected { false };

    std::vector<uint32_t> edgeIndices; // indices into BrushTopology::edges
    std::vector<uint32_t> faceIndices; // indices into Brush::faces
};

// ─── TopoEdge ────────────────────────────────────────────────────────────────

struct TopoEdge {
    uint32_t id       { 0 };
    bool     selected { false };

    uint32_t vertA { 0 }; // indices into BrushTopology::vertices
    uint32_t vertB { 0 };

    uint32_t faceA { 0 }; // indices into Brush::faces (exactly 2 for convex)
    uint32_t faceB { 0 };

    glm::vec3 direction(const std::vector<TopoVertex>& v) const noexcept {
        return v[vertB].position - v[vertA].position;
    }
    glm::vec3 midpoint(const std::vector<TopoVertex>& v) const noexcept {
        return (v[vertA].position + v[vertB].position) * 0.5f;
    }
    float length(const std::vector<TopoVertex>& v) const noexcept {
        return glm::length(direction(v));
    }
};

// ─── FaceTopo ────────────────────────────────────────────────────────────────

struct FaceTopo {
    uint32_t             faceIndex  { 0 };
    std::vector<uint32_t> vertIndices; // ordered, same winding as face polygon
    std::vector<uint32_t> edgeIndices;
};

// ─── BrushTopology ───────────────────────────────────────────────────────────

struct BrushTopology {
    std::vector<TopoVertex> vertices;
    std::vector<TopoEdge>   edges;
    std::vector<FaceTopo>   faces;   // one per Brush::faces entry
    bool                    valid { false };

    void clear() noexcept { vertices.clear(); edges.clear(); faces.clear(); valid = false; }

    // ── Lookups ───────────────────────────────────────────────────────────

    TopoVertex*       findVertex(uint32_t id) noexcept {
        for (auto& v : vertices) if (v.id == id) return &v; return nullptr; }
    const TopoVertex* findVertex(uint32_t id) const noexcept {
        for (const auto& v : vertices) if (v.id == id) return &v; return nullptr; }

    TopoEdge*         findEdge(uint32_t id) noexcept {
        for (auto& e : edges) if (e.id == id) return &e; return nullptr; }
    const TopoEdge*   findEdge(uint32_t id) const noexcept {
        for (const auto& e : edges) if (e.id == id) return &e; return nullptr; }

    // ── Selection helpers ─────────────────────────────────────────────────

    void clearVertexSelection() noexcept { for (auto& v : vertices) v.selected = false; }
    void clearEdgeSelection()   noexcept { for (auto& e : edges)    e.selected = false; }
    void clearAll()             noexcept { clearVertexSelection(); clearEdgeSelection(); }

    std::vector<uint32_t> selectedVertexIndices() const {
        std::vector<uint32_t> out;
        for (size_t i = 0; i < vertices.size(); ++i)
            if (vertices[i].selected) out.push_back(static_cast<uint32_t>(i));
        return out;
    }
    std::vector<uint32_t> selectedEdgeIndices() const {
        std::vector<uint32_t> out;
        for (size_t i = 0; i < edges.size(); ++i)
            if (edges[i].selected) out.push_back(static_cast<uint32_t>(i));
        return out;
    }

    AABB vertexBounds() const noexcept {
        AABB b;
        for (const auto& v : vertices) b.expand(v.position);
        return b;
    }
};

} // namespace editor
