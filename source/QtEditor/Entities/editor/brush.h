#pragma once
// editor/brush.h
// ─────────────────────────────────────────────────────────────────────────────
// Brush  – a convex solid defined by a set of half-spaces (its faces).
//
// Render model
// ────────────
// Each Face produces its own FaceMesh (vertices + indices) stored in
// BrushRenderData.  The renderer iterates faces, binds the face material,
// and issues a draw call per face.  This is the natural model for a brush
// editor because:
//   • Every face can have a different material.
//   • Face selection highlighting can be toggled per-face without rebuilding
//     the whole brush mesh.
//   • GPU buffer size per draw call is small and predictable (convex polygon
//     fans rarely exceed ~16 triangles).
//
// The flat collision / topology mesh is reconstructed from FaceMesh data
// by the topology builder; no separate flat BrushMesh is stored.
//
// Non-destructive transform
// ─────────────────────────
// `transform` overlays on top of face definitions for interactive editing.
// All render data and raycasting honour it.  Call applyTransform() to bake.
//
// CSG mode
// ────────
// Additive = solid;  Subtractive = carves into overlapping additive brushes.
// Interpreted by the compile step; brush data is unchanged.
// ─────────────────────────────────────────────────────────────────────────────

#include "face.h"
#include "topo_types.h"
#include <vector>
#include <string>
#include <algorithm>

namespace editor {

// ─── CSG mode ────────────────────────────────────────────────────────────────

enum class BrushMode { Additive, Subtractive };

// ─── BrushVertex — one GPU-ready vertex ──────────────────────────────────────
// UV is in texel space.  Divide by (texWidth, texHeight) for normalised UVs.

struct BrushVertex {
    glm::vec3 position  { 0.f };
    glm::vec3 normal    { 0.f, 1.f, 0.f };
    glm::vec2 uv        { 0.f };
    uint32_t  faceIndex { 0 }; // which Face in Brush::faces generated this vertex
};

// ─── FaceMesh — per-face draw unit ───────────────────────────────────────────
// Vertices are in world space (non-destructive transform already baked in).
// Indices form a triangle fan.  The renderer binds `material`, then draws.

struct FaceMesh {
    std::vector<BrushVertex> vertices;
    std::vector<uint32_t>    indices;
    std::string              material; // copied from Face::material at build time
    bool                     valid { false };

    void clear() noexcept { vertices.clear(); indices.clear(); valid = false; }

    // Convenience: world-space polygon positions in vertex order.
    std::vector<glm::vec3> positions() const {
        std::vector<glm::vec3> out;
        out.reserve(vertices.size());
        for (const auto& v : vertices) out.push_back(v.position);
        return out;
    }
};

// ─── BrushRenderData — full per-face render cache ────────────────────────────

struct BrushRenderData {
    std::vector<FaceMesh> faces; // one entry per Brush::faces, same order
    AABB                  bounds;
    bool                  valid { false };

    void clear() noexcept { faces.clear(); bounds = AABB{}; valid = false; }

    size_t totalTriangles() const noexcept {
        size_t n = 0;
        for (const auto& f : faces) n += f.indices.size() / 3;
        return n;
    }
};

// ─── Brush ───────────────────────────────────────────────────────────────────

struct Brush {
    uint32_t    id   { 0 };
    std::string name;
    BrushMode   mode { BrushMode::Additive };

    std::vector<Face> faces;

    // Non-destructive interactive transform (identity = no extra transform).
    glm::mat4 transform { 1.f };

    // Cached / derived data — rebuilt when dirty == true.
    BrushRenderData renderData;   // per-face GPU-ready meshes
    BrushTopology   topology;     // vertex / edge graph for editing

    bool dirty    { true };
    bool selected { false };

    // ── Validation ────────────────────────────────────────────────────────

    bool isValid() const noexcept { return faces.size() >= 4; }

    // ── Face management ───────────────────────────────────────────────────

    size_t addFace(Face f) {
        faces.push_back(std::move(f));
        dirty = true;
        return faces.size() - 1;
    }

    bool removeFace(uint32_t faceId) {
        auto it = std::find_if(faces.begin(), faces.end(),
                               [faceId](const Face& f){ return f.id == faceId; });
        if (it == faces.end()) return false;
        faces.erase(it);
        dirty = true;
        return true;
    }

    Face* findFace(uint32_t faceId) {
        auto it = std::find_if(faces.begin(), faces.end(),
                               [faceId](const Face& f){ return f.id == faceId; });
        return it != faces.end() ? &(*it) : nullptr;
    }
    const Face* findFace(uint32_t faceId) const {
        auto it = std::find_if(faces.begin(), faces.end(),
                               [faceId](const Face& f){ return f.id == faceId; });
        return it != faces.end() ? &(*it) : nullptr;
    }

    void markDirty() noexcept { dirty = true; }

    // ── Non-destructive transform helpers ────────────────────────────────

    void translateBy(glm::vec3 delta) noexcept {
        transform = glm::translate(transform, delta); dirty = true;
    }

    void rotateBy(float angleDeg, glm::vec3 axis,
                  glm::vec3 pivot = glm::vec3(0.f)) noexcept {
        glm::mat4 t2o = glm::translate(glm::mat4(1.f), -pivot);
        glm::mat4 frm = glm::translate(glm::mat4(1.f),  pivot);
        glm::mat4 rot = glm::rotate(glm::mat4(1.f), glm::radians(angleDeg), axis);
        transform = frm * rot * t2o * transform;
        dirty = true;
    }

    void scaleBy(glm::vec3 factors, glm::vec3 pivot = glm::vec3(0.f)) noexcept {
        glm::mat4 t2o = glm::translate(glm::mat4(1.f), -pivot);
        glm::mat4 frm = glm::translate(glm::mat4(1.f),  pivot);
        glm::mat4 s   = glm::scale(glm::mat4(1.f), factors);
        transform = frm * s * t2o * transform;
        dirty = true;
    }

    // ── Destructive helpers ───────────────────────────────────────────────

    void applyTransform() noexcept {
        for (Face& f : faces) f.applyTransform(transform);
        transform = glm::mat4(1.f);
        dirty = true;
    }

    bool pushFace(size_t faceIndex, float delta) noexcept {
        if (faceIndex >= faces.size()) return false;
        faces[faceIndex].pushAlongNormal(delta);
        dirty = true;
        return true;
    }

    void setMaterial(const std::string& mat) {
        for (Face& f : faces) f.material = mat;
        dirty = true;
    }

    // ── Convenience accessors ─────────────────────────────────────────────

    AABB      bounds()       const noexcept { return renderData.bounds; }
    glm::vec3 boundsCenter() const noexcept { return renderData.bounds.center(); }
};

} // namespace editor
