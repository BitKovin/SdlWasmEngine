#pragma once
// editor/vertex_edit.h
// ─────────────────────────────────────────────────────────────────────────────
// Non-trivial brush editing at the vertex and edge level.
//
// Why vertex editing is hard
// ──────────────────────────
// A brush stores faces as PLANES (each defined by 3 points).  Vertices are
// DERIVED — they're the intersection of 3+ planes.  Moving vertex V means
// updating the planes of all faces that share it so they now pass through
// V_new instead of V_old.
//
// Strategy (per face that owns the vertex):
//   1. Retrieve the face's current world-space polygon (from the mesh).
//   2. Replace V_old with V_new in that polygon.
//   3. Refit the face plane from the updated polygon:
//        normal  = normalised cross-product of two polygon edges (for stability
//                  we pick the pair with the largest cross-product magnitude).
//        dist    = dot(normal, any polygon point)
//   4. Select 3 non-collinear points from the new polygon and write them back
//      into face.points (so the face can be serialised correctly).
//   5. Mark the brush dirty.
//
// After all owning faces are updated, call buildBrushMesh() + buildTopology()
// to regenerate the cached data.
//
// Edge editing is decomposed into two vertex moves (both endpoints) subject
// to a shared constraint that preserves the edge's direction or allows a
// perpendicular slide.
//
// Validity
// ────────
// After every structural edit we run a convexity check.  If any face becomes
// degenerate (< 3 vertices remain after intersection) or the brush collapses
// to zero volume, the edit is rejected and the brush is rolled back.
// ─────────────────────────────────────────────────────────────────────────────

#include "topology.h"
#include "mesh_builder.h"

namespace editor {

// ─── VertexEdit result ───────────────────────────────────────────────────────

enum class EditResult {
    Ok,
    Rejected,   // edit would collapse the brush; data is unchanged
    Invalid,    // bad arguments (index out of range, etc.)
};

// ─── Vertex operations ───────────────────────────────────────────────────────

// Move topology vertex `vertIndex` (index into brush.topology.vertices) to
// `newPosition` in world space.
//
// This updates the plane definition of every face that owns the vertex,
// rebuilds the mesh, and rebuilds the topology.
//
// Returns Rejected if the move would collapse any face of the brush.
EditResult moveVertex(Brush& brush, uint32_t vertIndex, glm::vec3 newPosition);

// Move topology vertex `vertIndex` by `delta` (relative move).
// Convenience wrapper around moveVertex.
EditResult moveVertexBy(Brush& brush, uint32_t vertIndex, glm::vec3 delta);

// Move all currently-selected vertices by `delta`.
// Applies all moves simultaneously (reads old positions before writing any).
EditResult moveSelectedVerticesBy(Brush& brush, glm::vec3 delta);

// ─── Edge operations ─────────────────────────────────────────────────────────

// Slide edge `edgeIndex` along a `direction` by `distance` world units.
// direction should be perpendicular to the edge itself (e.g. the face normal
// of one of the two adjacent faces) but any direction is accepted; the
// result is simply clamped to be valid.
//
// Internally this calls moveVertex for both endpoints simultaneously.
EditResult slideEdge(Brush& brush, uint32_t edgeIndex,
                      glm::vec3 direction, float distance);

// Move both endpoints of edge `edgeIndex` by the same `delta`.
// Unlike slideEdge, this allows arbitrary translation (including along the
// edge axis) which can shear adjacent faces.
EditResult moveEdgeBy(Brush& brush, uint32_t edgeIndex, glm::vec3 delta);

// Move all currently-selected edges by `delta`.
// Deduplicates vertices that appear in multiple selected edges.
EditResult moveSelectedEdgesBy(Brush& brush, glm::vec3 delta);

// ─── Lower-level helpers (exposed for tooling / unit tests) ──────────────────

// Re-fit the plane of `face` from the world-space positions in `polygon`
// (must be convex, CCW when viewed from outside the brush).
// Writes back face.plane and face.points.
// Returns false if the polygon is degenerate (< 3 non-collinear points).
bool refitFacePlane(Face& face, const std::vector<glm::vec3>& polygon);

// Check that all faces of `brush` still produce valid (non-empty) polygons
// after hypothetical face-plane changes.  Used for rejection testing.
bool isBrushConvexAndValid(const Brush& brush);

} // namespace editor
