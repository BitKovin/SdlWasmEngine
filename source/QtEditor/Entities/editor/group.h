#pragma once
// editor/group.h
// ─────────────────────────────────────────────────────────────────────────────
// Group  –  a named container of entities used for organisation and prefabbing.
//
// Usage examples
// ──────────────
//
// World geometry:
//   Group("world") { Entity(classname="worldspawn") { Brush... } }
//
// Door prefab:
//   Group("door_01", prefabSource="prefabs/door_wooden.pfb") {
//     Entity(classname="func_door", properties={...})   // point or brush entity
//     Entity(classname="brush_clip") {                  // brush entity
//       Brush(mode=Subtractive)  // cuts hole in wall
//     }
//   }
//
// Prefab groups loaded from an external file are initially marked
// `prefabLinked = true`.  After the user edits them in the level, they
// become unlinked (editing is free; re-link is a deliberate action).
//
// Visibility / lock
// ─────────────────
// hidden → not rendered, not included in raycasts
// locked → rendered, but picking / editing is rejected
// ─────────────────────────────────────────────────────────────────────────────

#include "entity.h"
#include <string>
#include <algorithm>
#include <functional>

namespace editor {

struct Group {
    uint32_t    id       { 0 };
    std::string name;
    bool        selected { false };
    bool        hidden   { false };
    bool        locked   { false };

    // ── Prefab linkage ────────────────────────────────────────────────────
    // Empty string = not a prefab instance.
    // Non-empty    = path to the source prefab file (relative to project root).
    std::string prefabSource;
    bool        prefabLinked { false }; // true = still in sync with file

    bool isPrefabInstance() const noexcept { return !prefabSource.empty(); }

    // Mark the group as locally edited (prefab link broken).
    void breakPrefabLink() noexcept { prefabLinked = false; }

    // ── Entities ──────────────────────────────────────────────────────────

    std::vector<Entity> entities;

    size_t addEntity(Entity e) {
        entities.push_back(std::move(e));
        return entities.size() - 1;
    }

    bool removeEntity(uint32_t entityId) {
        auto it = std::find_if(entities.begin(), entities.end(),
                               [entityId](const Entity& e){ return e.id == entityId; });
        if (it == entities.end()) return false;
        entities.erase(it);
        return true;
    }

    Entity* findEntity(uint32_t entityId) {
        auto it = std::find_if(entities.begin(), entities.end(),
                               [entityId](const Entity& e){ return e.id == entityId; });
        return it != entities.end() ? &(*it) : nullptr;
    }
    const Entity* findEntity(uint32_t entityId) const {
        auto it = std::find_if(entities.begin(), entities.end(),
                               [entityId](const Entity& e){ return e.id == entityId; });
        return it != entities.end() ? &(*it) : nullptr;
    }

    // ── Convenience traversal ─────────────────────────────────────────────

    // Call fn(brush) for every brush in every entity in this group.
    void forEachBrush(const std::function<void(Brush&)>& fn) {
        for (Entity& e : entities)
            for (Brush& b : e.brushes) fn(b);
    }
    void forEachBrush(const std::function<void(const Brush&)>& fn) const {
        for (const Entity& e : entities)
            for (const Brush& b : e.brushes) fn(b);
    }

    // Mark every brush in this group dirty.
    void markAllDirty() noexcept {
        for (Entity& e : entities) e.markAllDirty();
    }

    // Union of all entity bounds.
    AABB bounds() const noexcept {
        AABB b;
        for (const Entity& e : entities) b.expand(e.bounds());
        return b;
    }
};

} // namespace editor
