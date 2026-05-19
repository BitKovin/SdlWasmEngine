#pragma once
// editor/face.h
// ─────────────────────────────────────────────────────────────────────────────
// UVProjection  – Valve 220-style texture axes stored explicitly as world-space
//                 vectors.  This avoids ambiguity from standard axis-aligned
//                 projection and makes rotations/transforms unambiguous.
//
// Face          – One planar face of a convex brush.
//                 Defined by three non-collinear points (the Quake .map
//                 convention) so it can be serialized and exactly reconstructed.
// ─────────────────────────────────────────────────────────────────────────────

#include "math_utils.h"
#include <string>
#include <array>
#include <cmath>

namespace editor {

// ─── UVProjection ────────────────────────────────────────────────────────────
//
// For any world-space vertex position P, the texel-space UV is:
//
//   u = dot(P, uAxis) / scale.x + offset.x
//   v = dot(P, vAxis) / scale.y + offset.y
//
// To get normalized [0,1] UVs for the GPU, divide by texture dimensions:
//   uNorm = u / texWidth,   vNorm = v / texHeight
//
// scale   – world units per texel (default 1 → 1 texel per 1 world unit).
//           Increase to zoom in (fewer texels per unit); decrease to zoom out.
// offset  – translation in texels (integer multiples pan by whole pixels).
// rotation – logical angle in degrees; stored for serialization and UI display.
//            The actual axes (uAxis, vAxis) are authoritative; this field is
//            recalculated when axes change via rotateAxes().

struct UVProjection {
    glm::vec3 uAxis    { 1.f,  0.f,  0.f };
    glm::vec3 vAxis    { 0.f, -1.f,  0.f }; // -Y so texture V goes "down"
    glm::vec2 offset   { 0.f };
    glm::vec2 scale    { 1.f };
    float     rotation { 0.f };             // degrees, informational

    // ── Projection ────────────────────────────────────────────────────────

    glm::vec2 project(glm::vec3 worldPos) const noexcept {
        return {
            glm::dot(worldPos, uAxis) / scale.x + offset.x,
            glm::dot(worldPos, vAxis) / scale.y + offset.y
        };
    }

    // ── Initialisation ────────────────────────────────────────────────────

    // Set uAxis/vAxis from a face normal using the same axis-aligned projection
    // rules as id Software's Quake tools ("standard" projection).
    // This is a reasonable default; users can then fine-tune rotation/offset.
    void initFromNormal(glm::vec3 n) noexcept {
        float ax = std::fabs(n.x), ay = std::fabs(n.y), az = std::fabs(n.z);
        if (ax >= ay && ax >= az) {
            // X-dominant face (left/right wall)
            uAxis = { 0.f,  1.f,  0.f };
            vAxis = { 0.f,  0.f, -1.f };
        } else if (az >= ax && az >= ay) {
            // Z-dominant face (front/back wall)
            uAxis = { 1.f,  0.f,  0.f };
            vAxis = { 0.f, -1.f,  0.f };
        } else {
            // Y-dominant face (floor/ceiling)
            uAxis = { 1.f,  0.f,  0.f };
            vAxis = { 0.f,  0.f, -1.f };
        }
        rotation = 0.f;
    }

    // Rotate both axes around the face normal by `degrees`.
    // Updates the `rotation` field too.
    void rotateAxes(glm::vec3 faceNormal, float degrees) noexcept {
        float    rad = glm::radians(degrees);
        glm::mat4 r  = glm::rotate(glm::mat4(1.f), rad, faceNormal);
        uAxis    = glm::normalize(glm::vec3(r * glm::vec4(uAxis, 0.f)));
        vAxis    = glm::normalize(glm::vec3(r * glm::vec4(vAxis, 0.f)));
        rotation = degrees;
    }

    // ── Comparison ────────────────────────────────────────────────────────

    bool operator==(const UVProjection& o) const noexcept {
        return uAxis == o.uAxis && vAxis == o.vAxis
            && offset == o.offset && scale == o.scale
            && rotation == o.rotation;
    }
};


// Placeholder material lookup used by the automatic UV projection path.
// The intended contract is "how many world meters correspond to one UV unit".
// For now we default to 1 UV unit per meter in both axes.
inline glm::vec2 materialUvScale(const std::string&) noexcept {
    return { 1.f, 1.f };
}

// ─── Face ────────────────────────────────────────────────────────────────────
//
// A brush face is the intersection of an infinite half-space with the other
// faces of the brush.  The three `points` uniquely identify the bounding plane
// and survive serialization perfectly (no floating-point drift from normals).
//
// The derived `plane` member must be kept in sync: call computePlane() after
// editing any of the three points.

struct Face {
    // Three non-collinear defining points, ordered CCW when viewed from outside
    // the brush (i.e. from the direction the normal points toward).
    std::array<glm::vec3, 3> points {{
        { 0.f, 0.f, 0.f },
        { 1.f, 0.f, 0.f },
        { 0.f, 1.f, 0.f }
    }};

    // Derived from points; always call computePlane() after editing points.
    Plane plane;

    // Texture / UV mapping
    UVProjection uv;

    // Material / texture name (maps to asset key in your resource system)
    std::string material { "default" };

    // ── Editor metadata ───────────────────────────────────────────────────

    uint32_t id       { 0 };
    bool     selected { false };

    // ── Maintenance ───────────────────────────────────────────────────────

    // Recompute plane from the three defining points.
    // Must be called after any edit to `points`.
    void computePlane() noexcept {
        plane = Plane::fromPoints(points[0], points[1], points[2]);
    }

    // Apply a 4×4 transform matrix to all three points and recompute the plane.
    // Also transforms the UV axes using the inverse-transpose (normal matrix).
    // This PERMANENTLY bakes the transform into the face definition.
    void applyTransform(const glm::mat4& m) noexcept {
        for (auto& p : points)
            p = glm::vec3(m * glm::vec4(p, 1.f));
        computePlane();

        // Texture axes transform as surface normals: use the inverse-transpose.
        glm::mat3 nm = glm::mat3(glm::transpose(glm::inverse(m)));
        uv.uAxis = glm::normalize(nm * uv.uAxis);
        uv.vAxis = glm::normalize(nm * uv.vAxis);
    }

    // Move the face's plane along its normal by `delta` world units
    // without changing the texture alignment.
    // (Useful for pushing/pulling a face in the editor.)
    void pushAlongNormal(float delta) noexcept {
        glm::vec3 move = plane.normal * delta;
        for (auto& p : points) p += move;
        plane.dist += delta;  // equivalent to recomputing, but faster
    }
};

} // namespace editor
