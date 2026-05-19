#pragma once
// editor/mesh_builder.h
// ─────────────────────────────────────────────────────────────────────────────
// Builds per-face FaceMesh data from a Brush's face definitions.
//
// Algorithm (per face F):
//   1. Start with a huge quad in F's plane.
//   2. Sutherland-Hodgman clip against every other face's plane.
//   3. Fan-triangulate the surviving polygon.
//   4. Compute UVs from F's UVProjection.
//
// The non-destructive Brush::transform is honoured: the builder works on a
// locally-transformed copy of the faces so renderData comes out in world space
// without permanently modifying face data.
// ─────────────────────────────────────────────────────────────────────────────

#include "brush.h"

namespace editor {

// ── Main entry point ─────────────────────────────────────────────────────────

// Build (or rebuild) brush.renderData and brush.topology from brush.faces
// plus brush.transform.  Clears brush.dirty regardless of outcome.
// brush.renderData.valid is set to true only if at least one face succeeded.
void buildBrushMesh(Brush& brush);

// ── Lower-level utilities (useful for editor tools / unit tests) ──────────────


} // namespace editor
