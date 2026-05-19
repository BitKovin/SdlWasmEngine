// editor/level.cpp
#include "level.h"
#include <algorithm>

namespace editor {

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

Level::Level()
{
    // Always create a default "world" group containing a worldspawn entity.
    // This mirrors TrenchBroom / Quake convention.
    Group world;
    world.name = "world";

    Entity worldspawn;
    worldspawn.setClassname("worldspawn");

    world.entities.push_back(std::move(worldspawn));
    addGroup(std::move(world));
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

void Level::assignFaceIds(Brush& b) {
    for (Face& f : b.faces)
        if (f.id == 0) f.id = newId();
}

// ─────────────────────────────────────────────────────────────────────────────
// Group management
// ─────────────────────────────────────────────────────────────────────────────

uint32_t Level::addGroup(Group g) {
    g.id = newId();
    for (Entity& e : g.entities) {
        if (e.id == 0) e.id = newId();
        for (Brush& b : e.brushes) {
            if (b.id == 0) b.id = newId();
            assignFaceIds(b);
            b.dirty = true;
        }
    }
    groups_.push_back(std::move(g));
    return groups_.back().id;
}

void Level::removeGroup(uint32_t groupId) {
    groups_.erase(
        std::remove_if(groups_.begin(), groups_.end(),
                       [groupId](const Group& g){ return g.id == groupId; }),
        groups_.end());
}

Group* Level::findGroup(uint32_t groupId) {
    for (auto& g : groups_) if (g.id == groupId) return &g;
    return nullptr;
}
const Group* Level::findGroup(uint32_t groupId) const {
    for (const auto& g : groups_) if (g.id == groupId) return &g;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Entity management
// ─────────────────────────────────────────────────────────────────────────────

uint32_t Level::addEntity(uint32_t groupId, Entity e) {
    Group* g = findGroup(groupId);
    if (!g) return 0;
    e.id = newId();
    for (Brush& b : e.brushes) {
        if (b.id == 0) b.id = newId();
        assignFaceIds(b);
        b.dirty = true;
    }
    g->entities.push_back(std::move(e));
    return g->entities.back().id;
}

bool Level::removeEntity(uint32_t entityId) {
    for (Group& g : groups_) {
        auto it = std::find_if(g.entities.begin(), g.entities.end(),
                               [entityId](const Entity& e){ return e.id == entityId; });
        if (it != g.entities.end()) { g.entities.erase(it); return true; }
    }
    return false;
}

Entity* Level::findEntity(uint32_t entityId) {
    for (Group& g : groups_)
        for (Entity& e : g.entities)
            if (e.id == entityId) return &e;
    return nullptr;
}
const Entity* Level::findEntity(uint32_t entityId) const {
    for (const Group& g : groups_)
        for (const Entity& e : g.entities)
            if (e.id == entityId) return &e;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush management
// ─────────────────────────────────────────────────────────────────────────────

uint32_t Level::addBrush(uint32_t entityId, Brush b) {
    Entity* e = findEntity(entityId);
    if (!e) return 0;
    b.id = newId();
    assignFaceIds(b);
    b.dirty = true;
    e->brushes.push_back(std::move(b));
    return e->brushes.back().id;
}

bool Level::removeBrush(uint32_t brushId) {
    for (Group& g : groups_)
        for (Entity& e : g.entities)
            if (e.removeBrush(brushId)) return true;
    return false;
}

Brush* Level::findBrush(uint32_t brushId) {
    for (Group& g : groups_)
        for (Entity& e : g.entities)
            for (Brush& b : e.brushes)
                if (b.id == brushId) return &b;
    return nullptr;
}
const Brush* Level::findBrush(uint32_t brushId) const {
    for (const Group& g : groups_)
        for (const Entity& e : g.entities)
            for (const Brush& b : e.brushes)
                if (b.id == brushId) return &b;
    return nullptr;
}

std::optional<BrushLocation> Level::locateBrush(uint32_t brushId) const {
    for (const Group& g : groups_)
        for (const Entity& e : g.entities)
            for (const Brush& b : e.brushes)
                if (b.id == brushId)
                    return BrushLocation{ g.id, e.id, b.id };
    return std::nullopt;
}

uint32_t Level::duplicateBrush(uint32_t brushId, glm::vec3 delta) {
    auto loc = locateBrush(brushId);
    if (!loc) return 0;
    const Brush* src = findBrush(brushId);
    if (!src) return 0;
    Brush copy = *src;
    copy.translateBy(delta);
    return addBrush(loc->entityId, std::move(copy));
}

// ─────────────────────────────────────────────────────────────────────────────
// Mesh maintenance
// ─────────────────────────────────────────────────────────────────────────────

void Level::rebuild() {
    forEachBrush([this](Brush& b) {
        if (b.dirty) {
            buildBrushMesh(b);
            csgDirty_ = true;
        }
    }, /*includeHidden=*/true);
}

void Level::rebuildCSG()
{
    if (!csgDirty_) return;

    std::vector<const Brush*> additive;
    std::vector<const Brush*> subtractive;

    forEachBrush([&](const Brush& b) {
        if (!b.renderData.valid) return;
        if (b.mode == BrushMode::Additive)
            additive.push_back(&b);
        else
            subtractive.push_back(&b);
    }, /*includeHidden=*/false);

    csgData_  = buildLevelCSG(additive, subtractive);
    csgDirty_ = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Edit mode
// ─────────────────────────────────────────────────────────────────────────────

void Level::setEditMode(EditMode m) noexcept {
    if (m == editMode_) return;
    deselectAll();
    editMode_ = m;
}

// ─────────────────────────────────────────────────────────────────────────────
// Selection
// ─────────────────────────────────────────────────────────────────────────────

static void clearBrushSubSelection(Brush* b) {
    if (!b) return;
    b->selected = false;
    for (Face& f : b->faces) f.selected = false;
    b->topology.clearAll();
}

void Level::deselectAll() {
    forEachBrush([](Brush& b){ clearBrushSubSelection(&b); }, true);
    for (Group& g : groups_) g.selected = false;
    for (Group& g : groups_) for (Entity& e : g.entities) e.selected = false;
    selection_.clear();
}

void Level::selectBrush(uint32_t brushId, bool additive) {
    if (!additive) deselectAll();
    Brush* b = findBrush(brushId);
    if (!b) return;
    b->selected = true;
    selection_.brushIds.insert(brushId);
}

void Level::selectFace(uint32_t brushId, uint32_t faceIndex, bool additive) {
    if (!additive) deselectAll();
    Brush* b = findBrush(brushId);
    if (!b || faceIndex >= b->faces.size()) return;
    b->faces[faceIndex].selected = true;
    selection_.faceIndices[brushId].insert(faceIndex);
}

void Level::selectVertex(uint32_t brushId, uint32_t vertIndex, bool additive) {
    if (!additive) deselectAll();
    Brush* b = findBrush(brushId);
    if (!b || !b->topology.valid || vertIndex >= b->topology.vertices.size()) return;
    b->topology.vertices[vertIndex].selected = true;
}

void Level::selectEdge(uint32_t brushId, uint32_t edgeIndex, bool additive) {
    if (!additive) deselectAll();
    Brush* b = findBrush(brushId);
    if (!b || !b->topology.valid || edgeIndex >= b->topology.edges.size()) return;
    b->topology.edges[edgeIndex].selected = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Non-destructive transforms on selection
// ─────────────────────────────────────────────────────────────────────────────

void Level::forEachSelectedBrush(const std::function<void(Brush&)>& fn) {
    for (uint32_t id : selection_.brushIds)
        if (Brush* b = findBrush(id)) fn(*b);
}

void Level::moveSelected(glm::vec3 delta) {
    forEachSelectedBrush([&](Brush& b){ b.translateBy(delta); });
}
void Level::rotateSelected(float angleDeg, glm::vec3 axis, glm::vec3 pivot) {
    forEachSelectedBrush([&](Brush& b){ b.rotateBy(angleDeg, axis, pivot); });
}
void Level::scaleSelected(glm::vec3 factors, glm::vec3 pivot) {
    forEachSelectedBrush([&](Brush& b){ b.scaleBy(factors, pivot); });
}
void Level::applyTransformSelected() {
    forEachSelectedBrush([](Brush& b){ b.applyTransform(); });
}

// ─────────────────────────────────────────────────────────────────────────────
// Sub-element editing
// ─────────────────────────────────────────────────────────────────────────────

EditResult Level::moveSelectedVerticesBy(uint32_t brushId, glm::vec3 delta) {
    Brush* b = findBrush(brushId);
    if (!b) return EditResult::Invalid;
    return editor::moveSelectedVerticesBy(*b, delta);
}

EditResult Level::moveSelectedEdgesBy(uint32_t brushId, glm::vec3 delta) {
    Brush* b = findBrush(brushId);
    if (!b) return EditResult::Invalid;
    return editor::moveSelectedEdgesBy(*b, delta);
}

EditResult Level::slideEdge(uint32_t brushId, uint32_t edgeIndex,
                              glm::vec3 dir, float dist) {
    Brush* b = findBrush(brushId);
    if (!b) return EditResult::Invalid;
    return editor::slideEdge(*b, edgeIndex, dir, dist);
}

bool Level::pushFace(uint32_t brushId, uint32_t faceIndex, float delta) {
    Brush* b = findBrush(brushId);
    return b ? b->pushFace(faceIndex, delta) : false;
}

bool Level::setFaceUV(uint32_t brushId, uint32_t faceIndex,
                       const UVProjection& uv) {
    Brush* b = findBrush(brushId);
    if (!b || faceIndex >= b->faces.size()) return false;
    b->faces[faceIndex].uv = uv;
    b->dirty = true;
    return true;
}

bool Level::setFaceMaterial(uint32_t brushId, uint32_t faceIndex,
                              const std::string& mat) {
    Brush* b = findBrush(brushId);
    if (!b) return false;
    if (faceIndex == ~0u) {
        b->setMaterial(mat);
    } else {
        if (faceIndex >= b->faces.size()) return false;
        b->faces[faceIndex].material = mat;
        b->dirty = true; // FaceMesh::material must also update
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Raycasting
// ─────────────────────────────────────────────────────────────────────────────

std::optional<RayHit> Level::raycast(const Ray& ray) const {
    std::optional<RayHit> best;
    for (const Group& g : groups_) {
        if (g.hidden) continue;
        for (const Entity& e : g.entities) {
            for (const Brush& b : e.brushes) {
                auto hit = rayIntersectBrush(ray, b);
                if (hit && (!best || hit->t < best->t)) best = hit;
            }
        }
    }
    return best;
}

std::optional<RayHit> Level::pick(const Ray& ray, bool additive) {
    auto hit = raycast(ray);
    if (!hit) { if (!additive) deselectAll(); return std::nullopt; }

    switch (editMode_) {
    case EditMode::Brush:
        selectBrush(hit->brushId, additive);
        break;
    case EditMode::Face:
        selectFace(hit->brushId, hit->faceIndex, additive);
        break;
    case EditMode::Vertex: {
        // After a face hit, find the nearest topology vertex on that brush.
        uint32_t vi = pickVertex(ray, hit->brushId);
        if (vi != ~0u) selectVertex(hit->brushId, vi, additive);
        break;
    }
    case EditMode::Edge: {
        uint32_t ei = pickEdge(ray, hit->brushId);
        if (ei != ~0u) selectEdge(hit->brushId, ei, additive);
        break;
    }
    }
    return hit;
}

uint32_t Level::pickVertex(const Ray& ray, uint32_t brushId, float radius) const {
    const Brush* b = findBrush(brushId);
    if (!b) return ~0u;
    return rayPickVertex(ray, *b, radius);
}

uint32_t Level::pickEdge(const Ray& ray, uint32_t brushId, float radius) const {
    const Brush* b = findBrush(brushId);
    if (!b) return ~0u;
    return rayPickEdge(ray, *b, radius);
}

// ─────────────────────────────────────────────────────────────────────────────
// Bounds & iteration
// ─────────────────────────────────────────────────────────────────────────────

AABB Level::worldBounds() const {
    AABB b;
    for (const Group& g : groups_) b.expand(g.bounds());
    return b;
}

void Level::forEachBrush(const std::function<void(Brush&)>& fn,
                          bool includeHidden) {
    for (Group& g : groups_) {
        if (!includeHidden && g.hidden) continue;
        g.forEachBrush(fn);
    }
}
void Level::forEachBrush(const std::function<void(const Brush&)>& fn,
                          bool includeHidden) const {
    for (const Group& g : groups_) {
        if (!includeHidden && g.hidden) continue;
        g.forEachBrush(fn);
    }
}

void Level::forEachEntity(const std::function<void(Entity&)>& fn) {
    for (Group& g : groups_) for (Entity& e : g.entities) fn(e);
}
void Level::forEachEntity(const std::function<void(const Entity&)>& fn) const {
    for (const Group& g : groups_) for (const Entity& e : g.entities) fn(e);
}

} // namespace editor
