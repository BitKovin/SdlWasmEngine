#pragma once
// editor/topology.h
// ─────────────────────────────────────────────────────────────────────────────
// Topology builder and ray-pick declarations.
//
// Data types (TopoVertex, TopoEdge, FaceTopo, BrushTopology) live in
// topo_types.h so that brush.h can include them without a circular dependency.
// This file only declares the free functions that operate on a Brush.
// ─────────────────────────────────────────────────────────────────────────────

#include "brush.h"      // brings in topo_types.h → BrushTopology already defined

namespace editor {

// ── Builder ───────────────────────────────────────────────────────────────────
// Derive topology from brush.renderData (must call buildBrushMesh first).
void buildTopology(Brush& brush);

// ── Ray picks ─────────────────────────────────────────────────────────────────
// Returns index into brush.topology.vertices / edges, or ~0u on miss.
uint32_t rayPickVertex(const Ray& ray, const Brush& brush, float radius = 6.f);
uint32_t rayPickEdge  (const Ray& ray, const Brush& brush, float radius = 5.f);

} // namespace editor
