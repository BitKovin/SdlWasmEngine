#pragma once
// editor/math_utils.h
// ─────────────────────────────────────────────────────────────────────────────
// Core math primitives: Plane, Ray, AABB.
// All header-only; no .cpp needed.
// ─────────────────────────────────────────────────────────────────────────────

#include <glm.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <cmath>
#include <limits>
#include <algorithm>

namespace editor {

// ─── Constants ────────────────────────────────────────────────────────────────

// Tolerance used for plane-side classification and polygon clipping.
constexpr float kEpsilon = 1e-4f;

// Half-size of the large initial polygon used during brush mesh generation.
// Should be larger than any expected level geometry.
constexpr float kHalfWorldSize = 131072.0f;

// ─── Plane ───────────────────────────────────────────────────────────────────
// Represented as  dot(normal, P) = dist  (the "Hessian normal form").
// normal is always unit-length.
// dist == glm::dot(normal, any_point_on_plane).

enum class PlaneSide { Front, Back, On };

struct Plane {
    glm::vec3 normal { 0.f, 1.f, 0.f };
    float     dist   { 0.f };

    // ── Construction ──────────────────────────────────────────────────────

    // Build from three points ordered CCW when viewed from the "outside".
    // The cross product of (B-A) × (C-A) gives the outward normal.
    static Plane fromPoints(glm::vec3 a, glm::vec3 b, glm::vec3 c) noexcept {
        Plane p;
        p.normal = glm::normalize(glm::cross(b - a, c - a));
        p.dist   = glm::dot(p.normal, a);
        return p;
    }

    // Build from a known unit normal and one point on the plane.
    static Plane fromNormalPoint(glm::vec3 normal, glm::vec3 point) noexcept {
        return { glm::normalize(normal), glm::dot(glm::normalize(normal), point) };
    }

    // ── Queries ───────────────────────────────────────────────────────────

    // Signed distance from the plane to a point (positive = front/outside).
    float distanceTo(glm::vec3 pt) const noexcept {
        return glm::dot(normal, pt) - dist;
    }

    PlaneSide classify(glm::vec3 pt) const noexcept {
        float d = distanceTo(pt);
        if (d >  kEpsilon) return PlaneSide::Front;
        if (d < -kEpsilon) return PlaneSide::Back;
        return PlaneSide::On;
    }

    // ── Three-plane intersection ──────────────────────────────────────────
    // Solves the 3×3 linear system via Cramer's rule.
    // Returns nullopt when the determinant is (near-)zero (parallel planes).
    std::optional<glm::vec3> intersect3(const Plane& b, const Plane& c) const noexcept {
        glm::vec3 n1 = normal, n2 = b.normal, n3 = c.normal;
        float det = glm::dot(n1, glm::cross(n2, n3));
        if (std::fabs(det) < kEpsilon) return std::nullopt;
        return (  dist * glm::cross(n2, n3)
               + b.dist * glm::cross(n3, n1)
               + c.dist * glm::cross(n1, n2)) / det;
    }

    // Return the plane with reversed orientation.
    Plane flipped() const noexcept { return { -normal, -dist }; }
};

// ─── Ray ─────────────────────────────────────────────────────────────────────

struct Ray {
    glm::vec3 origin    { 0.f };
    glm::vec3 direction { 0.f, 0.f, -1.f }; // must be unit-length

    glm::vec3 at(float t) const noexcept { return origin + direction * t; }

    // Distance t along the ray to plane intersection.
    // Returns nullopt when the ray is (nearly) parallel to the plane.
    // Note: t < 0 means the intersection is behind the ray origin.
    std::optional<float> intersectPlane(const Plane& plane) const noexcept {
        float denom = glm::dot(plane.normal, direction);
        if (std::fabs(denom) < kEpsilon) return std::nullopt;
        return (plane.dist - glm::dot(plane.normal, origin)) / denom;
    }
};

// ─── AABB ────────────────────────────────────────────────────────────────────

struct AABB {
    glm::vec3 min {  std::numeric_limits<float>::max()    };
    glm::vec3 max {  std::numeric_limits<float>::lowest() };

    bool valid() const noexcept { return min.x <= max.x; }

    void expand(glm::vec3 p) noexcept {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }
    void expand(const AABB& o) noexcept {
        if (o.valid()) { expand(o.min); expand(o.max); }
    }

    glm::vec3 center()  const noexcept { return (min + max) * 0.5f; }
    glm::vec3 extents() const noexcept { return (max - min) * 0.5f; }
    glm::vec3 size()    const noexcept { return max - min; }

    bool contains(glm::vec3 p) const noexcept {
        return p.x >= min.x && p.x <= max.x
            && p.y >= min.y && p.y <= max.y
            && p.z >= min.z && p.z <= max.z;
    }

    // Slab method (Amy Williams et al.).
    // tMin / tMax are set to the entry/exit distances along the ray.
    // Returns false if there is no intersection (or the box is behind the ray).
    bool intersectsRay(const Ray& ray, float& tMin, float& tMax) const noexcept {
        tMin = std::numeric_limits<float>::lowest();
        tMax = std::numeric_limits<float>::max();
        for (int i = 0; i < 3; ++i) {
            // Guard against zero direction component
            float invD = (std::fabs(ray.direction[i]) > 1e-10f)
                       ? 1.f / ray.direction[i]
                       : std::numeric_limits<float>::max();
            float t0 = (min[i] - ray.origin[i]) * invD;
            float t1 = (max[i] - ray.origin[i]) * invD;
            if (invD < 0.f) std::swap(t0, t1);
            tMin = std::max(tMin, t0);
            tMax = std::min(tMax, t1);
            if (tMax < tMin) return false;
        }
        return tMax >= 0.f; // at least part of the box is in front of the ray
    }
};

} // namespace editor
