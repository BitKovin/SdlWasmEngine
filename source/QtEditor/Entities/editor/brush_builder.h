#pragma once
// editor/brush_builder.h
// ─────────────────────────────────────────────────────────────────────────────
// Single entry point for all brush creation.
//
// Both creation workflows in the editor reduce to the same operation:
//   "given a set of 3-D points, produce the convex Brush that wraps them."
//
//   Box drag     → GetBrushCorners() → 8 corners  → buildBrushFromPoints
//   Polygon hull → hullPoints×2 extruded          → buildBrushFromPoints
//
// The function computes the 3-D convex hull of the input points, extracts
// one unique outward-facing half-space per hull face (coplanar triangles are
// merged into a single plane), and returns a ready-to-use Brush with
// Face::plane, Face::points, and Face::uv pre-filled.
//
// The returned Brush has no ID (assigned by Level::addBrush) and is NOT yet
// inserted into the level — the caller owns it and decides when to commit.
// ─────────────────────────────────────────────────────────────────────────────

#include "brush.h"
#include <vector>
#include <string>

namespace editor {

// ─── buildBrushFromPoints ────────────────────────────────────────────────────
//
// Requirements:
//   • At least 4 non-coplanar points.
//   • Points inside the convex hull are silently ignored.
//
// Returns an invalid Brush (isValid() == false) if the point set is degenerate
// (fewer than 4 points, all coplanar, or zero volume).
//
// Usage:
//   // Box drag
//   auto corners = GetBrushCorners();   // std::array<vec3,8>
//   std::vector<glm::vec3> pts(corners.begin(), corners.end());
//   editor::Brush b = buildBrushFromPoints(pts, "brick");
//
//   // Convex hull from clicked points + extrusion
//   std::vector<glm::vec3> pts;
//   for (const auto& p : hullPoints)  pts.push_back(p);
//   for (const auto& p : hullPoints)  pts.push_back(p + planeNormal * extrudeDepth);
//   editor::Brush b = buildBrushFromPoints(pts, "concrete");

Brush buildBrushFromPoints(const std::vector<glm::vec3>& points,
                            const std::string& material = "default");

} // namespace editor
