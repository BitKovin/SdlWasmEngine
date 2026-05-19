#pragma once
// editor/raycast.h
// ─────────────────────────────────────────────────────────────────────────────
// Ray intersection against brush geometry.
//
// All functions work against the *built mesh* stored in BrushMesh, so call
// buildBrushMesh() (or Level::rebuild()) before raycasting.  The mesh is in
// world space and already includes the brush's non-destructive transform.
// ─────────────────────────────────────────────────────────────────────────────

#include "brush.h"
#include <optional>
#include <vector>

namespace editor {

// ─── Hit result ──────────────────────────────────────────────────────────────

struct RayHit {
    float     t         { 0.f };               // distance along the ray
    glm::vec3 position  { 0.f };               // world-space hit point
    glm::vec3 normal    { 0.f, 1.f, 0.f };     // surface normal at hit
    uint32_t  brushId   { 0 };                 // Brush::id
    uint32_t  faceIndex { 0 };                 // index into Brush::faces

    bool operator<(const RayHit& o) const noexcept { return t < o.t; }
};

// ─── Face-polygon intersection ───────────────────────────────────────────────

// Intersect a ray against a convex polygon described by `count` vertices and
// an already-computed plane.  Returns the hit distance t, or nullopt.
// Only returns hits with t >= 0 (in front of the ray origin).
std::optional<float> rayIntersectPolygon(const Ray& ray,
                                          const glm::vec3* verts, size_t count,
                                          const Plane& plane);

// ─── Brush intersection ──────────────────────────────────────────────────────

// Intersect a ray against all faces of one brush.
// Returns the closest forward hit, or nullopt.
// Performs a quick AABB early-out before testing individual faces.
std::optional<RayHit> rayIntersectBrush(const Ray& ray, const Brush& brush);

// Intersect a ray against a list of brushes.
// Returns the globally closest hit, or nullopt.
std::optional<RayHit> rayIntersectBrushes(const Ray& ray,
                                            const std::vector<Brush>& brushes);

// ─── Utility ─────────────────────────────────────────────────────────────────

// Build the world-space polygon for face `faceIndex` from the built mesh.
// Returns an ordered list of positions (same order as the vertex buffer).
// Returns an empty vector if the face has no mesh data.
std::vector<glm::vec3> getFacePolygon(const Brush& brush, uint32_t faceIndex);

} // namespace editor
