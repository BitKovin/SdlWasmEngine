#pragma once
// editor/tiny_csg_backend.h
// ─────────────────────────────────────────────────────────────────────────────
// Small adapter layer that maps the editor's old data types onto tiny_csg.
// This keeps the existing Brush / Face / BrushRenderData / CSG result types,
// while moving the geometry work into the tiny_csg backend.
// ─────────────────────────────────────────────────────────────────────────────

#include "face.h"
#include "brush.h"
#include "csg.h"
#include "csg.hpp"

#include <array>
#include <cmath>
#include <vector>

namespace editor::tiny_csg_backend {

constexpr csg::volume_t kAirVolume   = 0;
constexpr csg::volume_t kSolidVolume = 1;

inline csg::plane_t toTinyPlane(const Plane& plane) noexcept {
    return { glm::normalize(plane.normal), -plane.dist };
}

inline Plane fromTinyPlane(const csg::plane_t& plane) noexcept {
    glm::vec3 normal = glm::normalize(plane.normal);
    return { normal, -plane.offset };
}

inline csg::plane_t toTinyPlane(const Face& face) noexcept {
    return toTinyPlane(face.plane);
}

inline csg::plane_t makeTinyPlane(const glm::vec3& point,
                                  const glm::vec3& normal) noexcept {
    glm::vec3 n = glm::normalize(normal);
    return { n, -glm::dot(point, n) };
}

inline std::vector<glm::vec3> polygonFromFace(const csg::face_t& face) {
    std::vector<glm::vec3> poly;
    if (!face.vertices.empty()) {
        poly.reserve(face.vertices.size());
        for (const auto& v : face.vertices) poly.push_back(v.position);
        if (poly.size() >= 3) return poly;
    }

    for (const auto& fragment : face.fragments) {
        if (fragment.vertices.size() < 3) continue;
        poly.clear();
        poly.reserve(fragment.vertices.size());
        for (const auto& v : fragment.vertices) poly.push_back(v.position);
        if (poly.size() >= 3) return poly;
    }
    return {};
}

inline std::vector<glm::vec3> polygonFromFragment(const csg::fragment_t& fragment) {
    std::vector<glm::vec3> poly;
    poly.reserve(fragment.vertices.size());
    for (const auto& v : fragment.vertices) poly.push_back(v.position);
    return poly;
}

inline bool isVisibleFragment(const csg::fragment_t& fragment) noexcept {
    return fragment.back_volume != fragment.front_volume;
}

inline bool shouldFlipFragment(const csg::fragment_t& fragment) noexcept {
    return fragment.back_volume == kAirVolume;
}

inline glm::vec3 fragmentNormal(const csg::face_t& face,
                                const csg::fragment_t& fragment) noexcept {
    glm::vec3 normal = face.plane ? face.plane->normal : glm::vec3(0.f, 1.f, 0.f);
    return shouldFlipFragment(fragment) ? -normal : normal;
}

inline std::array<glm::vec3, 3> fallbackTriangle(const Plane& plane) noexcept {
    glm::vec3 up = (std::abs(plane.normal.y) < 0.9f)
                 ? glm::vec3(0.f, 1.f, 0.f)
                 : glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 u = glm::normalize(glm::cross(up, plane.normal));
    glm::vec3 v = glm::cross(plane.normal, u);
    glm::vec3 c = plane.normal * plane.dist;
    return { c, c + u, c + v };
}

} // namespace editor::tiny_csg_backend
