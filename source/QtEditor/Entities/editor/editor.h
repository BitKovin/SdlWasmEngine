#pragma once
// editor/editor.h
// ─────────────────────────────────────────────────────────────────────────────
// Single include that pulls in the entire editor core.
//
// ══════════════════════════════════════════════════════════════════════════════
// QUICK REFERENCE
// ══════════════════════════════════════════════════════════════════════════════
//
// DATA HIERARCHY
// ──────────────
//   Level
//     └─ groups: vector<Group>
//          └─ entities: vector<Entity>
//               ├─ properties: map<string,string>   // classname, origin, …
//               └─ brushes: vector<Brush>
//                    ├─ faces: vector<Face>
//                    │    ├─ plane (Plane)
//                    │    └─ uv   (UVProjection)
//                    ├─ renderData: BrushRenderData
//                    │    └─ faces: vector<FaceMesh>   ← one per Brush::faces
//                    │         ├─ vertices: vector<BrushVertex>
//                    │         ├─ indices:  vector<uint32_t>
//                    │         └─ material: string
//                    └─ topology: BrushTopology
//                         ├─ vertices: vector<TopoVertex>
//                         └─ edges:    vector<TopoEdge>
//
// TYPICAL FRAME LOOP
// ──────────────────
//   level.rebuild();          // rebuilds dirty brushes (mesh + topology)
//
//   for (const Group& g : level.groups()) {
//     if (g.hidden) continue;
//     for (const Entity& e : g.entities) {
//       for (const Brush& b : e.brushes) {
//         for (size_t fi = 0; fi < b.renderData.faces.size(); ++fi) {
//           const FaceMesh& fm = b.renderData.faces[fi];
//           if (!fm.valid) continue;
//           bindMaterial(fm.material);
//           uploadAndDraw(fm.vertices, fm.indices);
//         }
//       }
//     }
//   }
//
// CREATING A BRUSH
// ────────────────
//   Brush box = BrushFactory::makeCuboid({0,0,0}, {64,64,64});
//   uint32_t brushId = level.addBrush(entityId, std::move(box));
//   level.rebuild();
//
// ADDING AN ENTITY (brush entity with a subtractive brush)
// ─────────────────────────────────────────────────────────
//   Entity door;
//   door.setClassname("func_door");
//   door.set("targetname", "door_01");
//   door.set("speed", "100");
//
//   Brush hole = BrushFactory::makeCuboid({32,0,0}, {96,128,16});
//   hole.mode = BrushMode::Subtractive;
//   door.addBrush(std::move(hole));
//
//   uint32_t eid = level.addEntity(groupId, std::move(door));
//
// CREATING A PREFAB GROUP
// ────────────────────────
//   Group g;
//   g.name         = "door_prefab_01";
//   g.prefabSource = "prefabs/door_wooden.pfb";
//   g.prefabLinked = true;
//   // … populate g.entities …
//   uint32_t gid = level.addGroup(std::move(g));
//
// PICKING / SELECTION
// ────────────────────
//   level.setEditMode(EditMode::Face);
//   auto hit = level.pick(ray, /*additive=*/false);
//   if (hit) {
//     level.setFaceMaterial(hit->brushId, hit->faceIndex, "brick");
//   }
//
// VERTEX EDITING
// ──────────────
//   level.setEditMode(EditMode::Vertex);
//   auto hit = level.pick(ray);                        // brush-level hit first
//   if (hit) {
//     uint32_t vi = level.pickVertex(ray, hit->brushId);
//     level.selectVertex(hit->brushId, vi);
//     auto result = level.moveSelectedVerticesBy(hit->brushId, {4,0,0});
//     if (result == EditResult::Rejected) { /* undo */ }
//   }
//
// EDGE EDITING
// ─────────────
//   level.setEditMode(EditMode::Edge);
//   auto hit = level.pick(ray);
//   if (hit) {
//     uint32_t ei = level.pickEdge(ray, hit->brushId);
//     // Slide the edge along the face normal
//     const Brush* b = level.findBrush(hit->brushId);
//     glm::vec3 n = b->faces[b->topology.edges[ei].faceA].plane.normal;
//     level.slideEdge(hit->brushId, ei, n, 8.f);
//   }
//
// ══════════════════════════════════════════════════════════════════════════════

#include "math_utils.h"
#include "face.h"
#include "topo_types.h"
#include "brush.h"
#include "brush_factory.h"
#include "brush_builder.h"
#include "mesh_builder.h"
#include "topology.h"
#include "raycast.h"
#include "vertex_edit.h"
#include "csg.h"
#include "entity.h"
#include "group.h"
#include "level.h"
