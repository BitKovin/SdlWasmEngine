#pragma once
// editor/csg.h
// ─────────────────────────────────────────────────────────────────────────────
// Constructive Solid Geometry — Boolean subtraction of brush volumes.
//
// Only two modes exist:
//   Additive    – solid geometry rendered and collided against.
//   Subtractive – carves a hole in every overlapping additive brush.
//
// The operation is computed at level scope (not per-brush) because a single
// subtractive brush can overlap many additive brushes simultaneously.
//
// Output
// ──────
// buildLevelCSG() produces one BrushCSGResult per additive brush.  Each
// result holds the final FaceMesh list after all subtractions have been
// applied.  The render loop uses these instead of Brush::renderData directly.
//
// Subtractive brushes produce no BrushCSGResult — they only affect additives.
//
// Rebuild triggers
// ────────────────
// CSG must be rebuilt whenever:
//   • Any brush is added, removed, or structurally edited (dirty flag).
//   • A brush's mode changes (additive ↔ subtractive).
//   • A brush is moved (transform baked or pending).
// Level::rebuild() sets csgDirty_ automatically; call rebuildCSG() after.
// ─────────────────────────────────────────────────────────────────────────────

#include "brush.h"
#include <vector>
#include <unordered_map>

namespace editor {

// ─── CSGFace — one renderable face produced by CSG ───────────────────────────
// Mirrors FaceMesh but carries the source brush/face indices for picking.

enum class FaceRole {
    SolidSurface,     // normal visible surface
    CapSurface,       // cap created by subtraction
    EditorOnlySurface // subtractive brush mesh itself
};

struct CSGFace {
    std::vector<BrushVertex> vertices;
    std::vector<uint32_t>    indices;
    std::string              material;
    uint32_t                 sourceBrushId  { 0 };
    uint32_t                 sourceFaceIdx  { 0 };
    bool                     isCap         { false }; // true = came from a subtractive brush
    bool                     valid         { false };

    void clear() noexcept { vertices.clear(); indices.clear(); valid = false; }

    std::vector<glm::vec3> positions() const {
        std::vector<glm::vec3> out;
        out.reserve(vertices.size());
        for (const auto& v : vertices) out.push_back(v.position);
        return out;
    }
};

// ─── BrushCSGResult — final geometry for one additive brush ──────────────────

struct BrushCSGResult {
    uint32_t             brushId { 0 };
    std::vector<CSGFace> faces;          // after all subtractions
    AABB                 bounds;
    bool                 valid  { false };

    void clear() noexcept { faces.clear(); bounds = AABB{}; valid = false; }
};

// ─── LevelCSGData — full level CSG output ────────────────────────────────────

struct LevelCSGData {
    // brushId → result  (only additive brushes appear here)
    std::unordered_map<uint32_t, BrushCSGResult> results;
    bool valid { false };

    void clear() noexcept { results.clear(); valid = false; }

    const BrushCSGResult* find(uint32_t brushId) const {
        auto it = results.find(brushId);
        return (it != results.end()) ? &it->second : nullptr;
    }
};

// ─── Entry point ─────────────────────────────────────────────────────────────

// Collect all brushes from the level hierarchy and compute CSG.
// `additiveBrushes`    – all brushes with mode == Additive (already mesh-built)
// `subtractiveBrushes` – all brushes with mode == Subtractive (already mesh-built)
LevelCSGData buildLevelCSG(
    const std::vector<const Brush*>& additiveBrushes,
    const std::vector<const Brush*>& subtractiveBrushes);

// ─── Lower-level helpers (exposed for testing / tooling) ─────────────────────

// Split `poly` by `plane` into front (outside, dist > 0) and back (inside, dist <= 0).
// Points on the plane (within kEpsilon) go to both sides.
void splitPolygon(const std::vector<glm::vec3>& poly,
                  const Plane& plane,
                  std::vector<glm::vec3>& front,
                  std::vector<glm::vec3>& back);

// Clip `poly` against the outside of `subFaces` starting at `planeIdx`.
// Emits surviving (outside) fragments into `output`.
// Fragments that pass all planes are fully inside the subtractive brush → discarded.
void clipPolyOutsideBrush(const std::vector<glm::vec3>& poly,
                           const std::vector<Face>& subFaces,
                           size_t planeIdx,
                           std::vector<std::vector<glm::vec3>>& output);

// Check whether two AABBs overlap (used for early rejection before polygon clipping).
bool aabbsOverlap(const AABB& a, const AABB& b) noexcept;

} // namespace editor
