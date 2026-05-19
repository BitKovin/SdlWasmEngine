#pragma once
// editor/level.h
// ─────────────────────────────────────────────────────────────────────────────
// Level  –  root container: Level → Groups → Entities → Brushes
//
// ID space
// ────────
// Groups, entities, and brushes each get a unique uint32_t id from a shared
// monotonically-increasing counter (ids are never reused within a session).
// Face ids are also assigned here.
//
// Mesh maintenance
// ────────────────
// Call rebuild() once per frame (or after edits).  Only dirty brushes are
// rebuilt.  buildBrushMesh also rebuilds topology as a side effect.
// rebuild() sets csgDirty_ = true whenever any brush is rebuilt.
// Call rebuildCSG() after rebuild() to recompute Boolean subtraction.
//
// Rendering — USE csgData(), NOT brush.renderData directly
// ─────────────────────────────────────────────────────────
//   for group in level.groups():
//     if group.hidden: continue
//     for entity in group.entities:
//       for brush in entity.brushes:
//         if brush.mode == Subtractive: continue   // never drawn directly
//         const BrushCSGResult* csg = level.csgData().find(brush.id)
//         if !csg: continue
//         for face in csg->faces:
//           bind(face.material)
//           draw(face.vertices, face.indices)
// ─────────────────────────────────────────────────────────────────────────────

#include "group.h"
#include "mesh_builder.h"
#include "raycast.h"
#include "vertex_edit.h"
#include "csg.h"
#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace editor {

// ─── EditMode — which sub-object type the editor is currently manipulating ───

enum class EditMode { Brush, Face, Edge, Vertex };

// ─── Selection ────────────────────────────────────────────────────────────────
// Each field holds ids of currently-selected objects.
// brush/face selection is used in Brush and Face modes.
// edge/vertex selection is used in Edge and Vertex modes (per-brush).

struct LevelSelection {
    // Brush-mode selection
    std::unordered_set<uint32_t> brushIds;

    // Face-mode selection: brushId → set of faceIndex values
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>> faceIndices;

    // Sub-element selections are stored per-brush in BrushTopology directly
    // (TopoVertex::selected, TopoEdge::selected).

    bool empty() const noexcept { return brushIds.empty() && faceIndices.empty(); }
    void clear() { brushIds.clear(); faceIndices.clear(); }

    bool hasBrush(uint32_t id) const noexcept { return brushIds.count(id) != 0; }
    bool hasFace(uint32_t brushId, uint32_t fi) const noexcept {
        auto it = faceIndices.find(brushId);
        return it != faceIndices.end() && it->second.count(fi) != 0;
    }
};

// ─── BrushLocation — full path to a brush in the hierarchy ───────────────────

struct BrushLocation {
    uint32_t groupId  { 0 };
    uint32_t entityId { 0 };
    uint32_t brushId  { 0 };
};

// ─── Level ───────────────────────────────────────────────────────────────────

class Level {
public:
    Level();

    // ── Group management ─────────────────────────────────────────────────

    // Add a group (assigns id).  Returns the assigned id.
    uint32_t addGroup(Group g);
    void     removeGroup(uint32_t groupId);

    Group*       findGroup(uint32_t groupId);
    const Group* findGroup(uint32_t groupId) const;

    std::vector<Group>&       groups()       { return groups_; }
    const std::vector<Group>& groups() const { return groups_; }

    // ── Entity management ─────────────────────────────────────────────────

    // Add entity to a specific group.  Returns the entity id.
    uint32_t addEntity(uint32_t groupId, Entity e);
    bool     removeEntity(uint32_t entityId);

    Entity*       findEntity(uint32_t entityId);
    const Entity* findEntity(uint32_t entityId) const;

    // ── Brush management ─────────────────────────────────────────────────

    // Add brush to a specific entity (identified by id).  Returns the brush id.
    uint32_t addBrush(uint32_t entityId, Brush b);
    bool     removeBrush(uint32_t brushId);

    Brush*       findBrush(uint32_t brushId);
    const Brush* findBrush(uint32_t brushId) const;

    // Resolve the full group/entity/brush path for a brush id.
    std::optional<BrushLocation> locateBrush(uint32_t brushId) const;

    // Duplicate a brush within the same entity, offset by `delta`.
    uint32_t duplicateBrush(uint32_t brushId, glm::vec3 delta = glm::vec3(0.f));

    // ── Mesh maintenance ──────────────────────────────────────────────────

    // Rebuild all dirty brushes (call once per frame before rendering).
    // Sets csgDirty_ = true if any brush was rebuilt.
    void rebuild();

    // Recompute Boolean CSG (subtraction of subtractive brushes from additive).
    // Must be called after rebuild() whenever csgDirty() is true.
    void rebuildCSG();

    bool csgDirty() const noexcept { return csgDirty_; }

    // The CSG output — use this for rendering, not brush.renderData directly.
    const LevelCSGData& csgData() const { return csgData_; }

    // ── Edit mode ─────────────────────────────────────────────────────────

    EditMode    editMode() const noexcept { return editMode_; }
    void        setEditMode(EditMode m) noexcept;

    // ── Selection ─────────────────────────────────────────────────────────

    void selectBrush       (uint32_t brushId,    bool additive = false);
    void selectFace        (uint32_t brushId, uint32_t faceIndex, bool additive = false);
    void selectVertex      (uint32_t brushId, uint32_t vertIndex, bool additive = false);
    void selectEdge        (uint32_t brushId, uint32_t edgeIndex, bool additive = false);

    void deselectAll();

    const LevelSelection& selection() const { return selection_; }

    // ── Non-destructive transforms on selected brushes ────────────────────

    void moveSelected  (glm::vec3 delta);
    void rotateSelected(float angleDeg, glm::vec3 axis, glm::vec3 pivot);
    void scaleSelected (glm::vec3 factors, glm::vec3 pivot);
    void applyTransformSelected();

    // ── Sub-element editing ───────────────────────────────────────────────

    // Vertex / edge editing — dispatches to vertex_edit.h functions.
    // Operates on the brush identified by brushId.
    EditResult moveSelectedVerticesBy(uint32_t brushId, glm::vec3 delta);
    EditResult moveSelectedEdgesBy   (uint32_t brushId, glm::vec3 delta);
    EditResult slideEdge             (uint32_t brushId, uint32_t edgeIndex,
                                      glm::vec3 dir, float dist);

    // Face editing
    bool pushFace      (uint32_t brushId, uint32_t faceIndex, float delta);
    bool setFaceUV     (uint32_t brushId, uint32_t faceIndex, const UVProjection& uv);
    bool setFaceMaterial(uint32_t brushId, uint32_t faceIndex, const std::string& mat);

    // ── Raycasting ────────────────────────────────────────────────────────

    // Test a ray against all visible (non-hidden), non-locked brushes.
    std::optional<RayHit> raycast(const Ray& ray) const;

    // Raycast + update selection.  Respects current editMode.
    std::optional<RayHit> pick(const Ray& ray,
                                bool additive = false);

    // Vertex / edge picks (only meaningful in Vertex / Edge edit modes).
    uint32_t pickVertex(const Ray& ray, uint32_t brushId, float radius = 6.f) const;
    uint32_t pickEdge  (const Ray& ray, uint32_t brushId, float radius = 5.f) const;

    // ── World bounds ──────────────────────────────────────────────────────

    AABB worldBounds() const;

    // ── Iteration helpers ─────────────────────────────────────────────────

    // Visits every brush across the whole hierarchy.
    // hiddenGroups: if false (default) skips groups with hidden=true.
    void forEachBrush(const std::function<void(Brush&)>& fn,
                      bool includeHidden = false);
    void forEachBrush(const std::function<void(const Brush&)>& fn,
                      bool includeHidden = false) const;

    void forEachEntity(const std::function<void(Entity&)>& fn);
    void forEachEntity(const std::function<void(const Entity&)>& fn) const;

    // Visits only brushes belonging to currently-selected brush ids.
    void forEachSelectedBrush(const std::function<void(Brush&)>& fn);

private:
    std::vector<Group> groups_;
    LevelSelection     selection_;
    EditMode           editMode_ { EditMode::Brush };
    LevelCSGData       csgData_;
    bool               csgDirty_ { true };

    uint32_t nextId_ { 1 };

    uint32_t newId() noexcept { return nextId_++; }
    void     assignFaceIds(Brush& b);
};

} // namespace editor
