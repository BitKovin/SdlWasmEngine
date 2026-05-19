// editor/raycast.cpp
#include "raycast.h"
#include <cmath>
#include <limits>

namespace editor {

// ─────────────────────────────────────────────────────────────────────────────
// getFacePolygon  — reads from per-face FaceMesh (world space, polygon order)
// ─────────────────────────────────────────────────────────────────────────────

std::vector<glm::vec3> getFacePolygon(const Brush& brush, uint32_t faceIndex)
{
    if (!brush.renderData.valid) return {};
    if (faceIndex >= brush.renderData.faces.size()) return {};
    return brush.renderData.faces[faceIndex].positions();
}

// ─────────────────────────────────────────────────────────────────────────────
// rayIntersectPolygon
// ─────────────────────────────────────────────────────────────────────────────

std::optional<float> rayIntersectPolygon(const Ray& ray,
                                          const glm::vec3* verts, size_t count,
                                          const Plane& plane)
{
    if (count < 3) return std::nullopt;

    auto tOpt = ray.intersectPlane(plane);
    if (!tOpt || *tOpt < 0.f) return std::nullopt;

    glm::vec3 pt = ray.at(*tOpt);
    const glm::vec3& n = plane.normal;

    for (size_t i = 0; i < count; ++i) {
        const glm::vec3& A = verts[i];
        const glm::vec3& B = verts[(i + 1) % count];
        if (glm::dot(glm::cross(B - A, pt - A), n) < -kEpsilon)
            return std::nullopt;
    }
    return *tOpt;
}

// ─────────────────────────────────────────────────────────────────────────────
// rayIntersectBrush
// ─────────────────────────────────────────────────────────────────────────────

std::optional<RayHit> rayIntersectBrush(const Ray& ray, const Brush& brush)
{
    if (!brush.renderData.valid) return std::nullopt;

    float tMin, tMax;
    if (!brush.renderData.bounds.intersectsRay(ray, tMin, tMax)) return std::nullopt;

    std::optional<RayHit> best;
    auto nFaces = static_cast<uint32_t>(brush.faces.size());

    for (uint32_t fi = 0; fi < nFaces; ++fi) {
        const FaceMesh& fm = brush.renderData.faces[fi];
        if (!fm.valid || fm.vertices.size() < 3) continue;

        // Derive plane from the world-space FaceMesh (honours non-destructive transform).
        std::vector<glm::vec3> poly = fm.positions();
        Plane meshPlane = Plane::fromPoints(poly[0], poly[1], poly[2]);

        auto tOpt = rayIntersectPolygon(ray, poly.data(), poly.size(), meshPlane);
        if (!tOpt) continue;

        if (!best || *tOpt < best->t) {
            RayHit hit;
            hit.t         = *tOpt;
            hit.position  = ray.at(*tOpt);
            hit.normal    = meshPlane.normal;
            hit.brushId   = brush.id;
            hit.faceIndex = fi;
            best = hit;
        }
    }
    return best;
}

// ─────────────────────────────────────────────────────────────────────────────
// rayIntersectBrushes
// ─────────────────────────────────────────────────────────────────────────────

std::optional<RayHit> rayIntersectBrushes(const Ray& ray,
                                            const std::vector<Brush>& brushes)
{
    std::optional<RayHit> best;
    for (const Brush& b : brushes) {
        auto hit = rayIntersectBrush(ray, b);
        if (hit && (!best || hit->t < best->t)) best = hit;
    }
    return best;
}

} // namespace editor
