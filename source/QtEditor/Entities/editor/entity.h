#pragma once
// editor/entity.h
// ─────────────────────────────────────────────────────────────────────────────
// Entity  –  the basic gameplay/logic object in the level.
//
// Design
// ──────
// Entities follow the Quake / GoldSrc / idTech convention:
//
//   • Every property is a (key, value) string pair.
//     - "classname"  identifies the entity type (e.g. "func_door", "light")
//     - "origin"     world-space position as "x y z" for point entities
//     - "angles"     Euler angles as "pitch yaw roll"
//     - Any engine- or game-specific key is simply stored verbatim.
//
//   • Point entities have no brushes (isPointEntity() == true).
//     They exist at the origin stored in their properties.
//
//   • Brush entities own one or more brushes (isBrushEntity() == true).
//     Each brush can independently be Additive or Subtractive.
//     Example: func_door (additive brush = door geometry) + a subtractive
//              brush entity that cuts the doorway in the surrounding wall.
//
// Ownership
// ─────────
// Brushes are stored by value.  The Entity is responsible for their lifetime.
// IDs are assigned by Level when the entity is added to a Group.
// ─────────────────────────────────────────────────────────────────────────────

#include "brush.h"
#include <map>
#include <string>
#include <optional>
#include <sstream>

namespace editor {

struct Entity {
    uint32_t    id       { 0 };
    bool        selected { false };

    // ── Property bag ──────────────────────────────────────────────────────
    // All data lives here; classname/origin/angles are just conventional keys.
    std::map<std::string, std::string> properties;

    // ── Brushes (empty = point entity) ────────────────────────────────────
    std::vector<Brush> brushes;

    // ── Property helpers ──────────────────────────────────────────────────

    // Get a property, returning `defaultVal` if the key is absent.
    std::string get(const std::string& key,
                    const std::string& defaultVal = "") const
    {
        auto it = properties.find(key);
        return (it != properties.end()) ? it->second : defaultVal;
    }

    void set(const std::string& key, const std::string& value) {
        properties[key] = value;
    }

    bool has(const std::string& key) const {
        return properties.count(key) != 0;
    }

    void unset(const std::string& key) {
        properties.erase(key);
    }

    // ── Typed convenience getters ─────────────────────────────────────────

    std::string classname() const { return get("classname", "worldspawn"); }
    void setClassname(const std::string& cn) { set("classname", cn); }

    // Parse "x y z" origin string into a vec3.  Returns nullopt on failure.
    std::optional<glm::vec3> originVec3() const {
        auto s = get("origin");
        if (s.empty()) return std::nullopt;
        std::istringstream ss(s);
        float x, y, z;
        if (!(ss >> x >> y >> z)) return std::nullopt;
        return glm::vec3(x, y, z);
    }

    // Write a vec3 back to the "origin" property.
    void setOrigin(glm::vec3 o) {
        std::ostringstream ss;
        ss << o.x << " " << o.y << " " << o.z;
        set("origin", ss.str());
    }

    // Parse "pitch yaw roll" angles string.
    std::optional<glm::vec3> anglesVec3() const {
        auto s = get("angles");
        if (s.empty()) return std::nullopt;
        std::istringstream ss(s);
        float p, y, r;
        if (!(ss >> p >> y >> r)) return std::nullopt;
        return glm::vec3(p, y, r);
    }

    void setAngles(glm::vec3 pyr) {
        std::ostringstream ss;
        ss << pyr.x << " " << pyr.y << " " << pyr.z;
        set("angles", ss.str());
    }

    // ── Entity type queries ───────────────────────────────────────────────

    bool isPointEntity() const noexcept { return brushes.empty(); }
    bool isBrushEntity() const noexcept { return !brushes.empty(); }

    // ── Brush management ─────────────────────────────────────────────────

    // Add a brush, returns its index within this entity.
    size_t addBrush(Brush b) {
        brushes.push_back(std::move(b));
        return brushes.size() - 1;
    }

    bool removeBrush(uint32_t brushId) {
        auto it = std::find_if(brushes.begin(), brushes.end(),
                               [brushId](const Brush& b){ return b.id == brushId; });
        if (it == brushes.end()) return false;
        brushes.erase(it);
        return true;
    }

    Brush* findBrush(uint32_t brushId) {
        auto it = std::find_if(brushes.begin(), brushes.end(),
                               [brushId](const Brush& b){ return b.id == brushId; });
        return it != brushes.end() ? &(*it) : nullptr;
    }
    const Brush* findBrush(uint32_t brushId) const {
        auto it = std::find_if(brushes.begin(), brushes.end(),
                               [brushId](const Brush& b){ return b.id == brushId; });
        return it != brushes.end() ? &(*it) : nullptr;
    }

    // ── Bounds (union of all brush bounds) ────────────────────────────────

    AABB bounds() const noexcept {
        AABB b;
        for (const Brush& br : brushes)
            if (br.renderData.valid) b.expand(br.renderData.bounds);
        return b;
    }

    // Mark all brushes dirty (e.g. after a property-driven transform).
    void markAllDirty() noexcept {
        for (Brush& b : brushes) b.markDirty();
    }
};

} // namespace editor
