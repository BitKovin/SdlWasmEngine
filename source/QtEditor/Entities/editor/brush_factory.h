#pragma once
// editor/brush_factory.h
// ─────────────────────────────────────────────────────────────────────────────
// Convenience functions for constructing common brush shapes.
// All functions return a ready-to-add Brush with computed planes and
// initialised UV axes.  Pass the result to Level::addBrush().
// ─────────────────────────────────────────────────────────────────────────────

#include "brush.h"
#include <string>
#include <cmath>

namespace editor {
namespace BrushFactory {

// ── Helpers ───────────────────────────────────────────────────────────────────

// Create a Face from three points and auto-initialise its UV axes and material.
inline Face makeFace(glm::vec3 a, glm::vec3 b, glm::vec3 c,
                     const std::string& material = "default")
{
    Face f;
    f.points = { a, b, c };
    f.computePlane();
    f.uv.initFromNormal(f.plane.normal);
    f.uv.scale = materialUvScale(material);
    f.material = material;
    return f;
}

// ── Box / Cuboid ───────────────────────────────────────────────────────────────
//
// Creates a 6-faced axis-aligned box from two opposite corner points.
// Face winding verified so each normal points outward.
//
//   Corner layout:
//       3───2   7───6
//       │   │   │   │
//       0───1   4───5
//       z=min   z=max

inline Brush makeCuboid(glm::vec3 mins, glm::vec3 maxs,
                         const std::string& material = "default")
{
    float x0 = mins.x, y0 = mins.y, z0 = mins.z;
    float x1 = maxs.x, y1 = maxs.y, z1 = maxs.z;

    // Eight corners
    glm::vec3 c[8] = {
        { x0, y0, z0 }, // 0: ---
        { x1, y0, z0 }, // 1: +--
        { x1, y1, z0 }, // 2: ++-
        { x0, y1, z0 }, // 3: -+-
        { x0, y0, z1 }, // 4: --+
        { x1, y0, z1 }, // 5: +-+
        { x1, y1, z1 }, // 6: +++
        { x0, y1, z1 }, // 7: -++
    };

    Brush b;
    b.name = "cuboid";

    // Each triple is ordered so cross(B-A, C-A) = outward normal.
    // Verified by hand (see math_utils.h comments).
    b.addFace(makeFace(c[1], c[2], c[6], material)); // +X  (normal = +1,0,0)
    b.addFace(makeFace(c[0], c[4], c[7], material)); // -X  (normal = -1,0,0)
    b.addFace(makeFace(c[3], c[7], c[6], material)); // +Y  (normal = 0,+1,0)
    b.addFace(makeFace(c[0], c[1], c[5], material)); // -Y  (normal = 0,-1,0)
    b.addFace(makeFace(c[4], c[5], c[6], material)); // +Z  (normal = 0,0,+1)
    b.addFace(makeFace(c[0], c[3], c[2], material)); // -Z  (normal = 0,0,-1)

    return b;
}

// Unit cube centred at the origin
inline Brush makeUnitCube(const std::string& material = "default") {
    return makeCuboid({ -0.5f, -0.5f, -0.5f },
                      {  0.5f,  0.5f,  0.5f }, material);
}

// ── Wedge (right-angle ramp) ───────────────────────────────────────────────────
//
// A 5-faced wedge:
//   bottom  (-Y), front (-Z), back (+Z), left (-X), slanted top (right face
//   between c[1],c[2],c[6],c[5] is replaced by the slope c[1],c[5],c[7],c[3]).
//
//   The slope goes from the full height at the back (+Z) to zero at the front (-Z).
//
//        7───3
//       /   /
//      4   0     (front, height=0)
//      │   │
//      5   1     (back, full height)
//       \ /
//        6───2   ← virtual bottom corners

inline Brush makeWedge(glm::vec3 mins, glm::vec3 maxs,
                        const std::string& material = "default")
{
    float x0 = mins.x, y0 = mins.y, z0 = mins.z;
    float x1 = maxs.x, y1 = maxs.y, z1 = maxs.z;

    glm::vec3 c[8] = {
        { x0, y0, z0 }, { x1, y0, z0 },   // 0,1  front bottom
        { x1, y1, z0 }, { x0, y1, z0 },   // 2,3  front top  (same as bottom when ramp hits 0)
        { x0, y0, z1 }, { x1, y0, z1 },   // 4,5  back  bottom
        { x1, y1, z1 }, { x0, y1, z1 },   // 6,7  back  top
    };

    Brush b;
    b.name = "wedge";

    b.addFace(makeFace(c[0], c[1], c[5], material)); // bottom  -Y
    b.addFace(makeFace(c[0], c[4], c[7], material)); // left    -X
    b.addFace(makeFace(c[1], c[2], c[6], material)); // right   +X
    b.addFace(makeFace(c[4], c[5], c[6], material)); // back    +Z

    // Slanted top: c[1](front-bottom), c[5](back-bottom), c[7](back-top), c[3](front-top at min y1)
    // Use first three non-collinear points to define the plane
    b.addFace(makeFace(c[1], c[7], c[3], material)); // slope

    return b;
}

// ── Pyramid ────────────────────────────────────────────────────────────────────

inline Brush makePyramid(glm::vec3 base_mins, glm::vec3 base_maxs, float height,
                          const std::string& material = "default")
{
    float x0 = base_mins.x, z0 = base_mins.z;
    float x1 = base_maxs.x, z1 = base_maxs.z;
    float y0 = base_mins.y;
    float yTop = y0 + height;

    glm::vec3 apex = { (x0 + x1) * 0.5f, yTop, (z0 + z1) * 0.5f };
    glm::vec3 bl   = { x0, y0, z0 };
    glm::vec3 br   = { x1, y0, z0 };
    glm::vec3 tr   = { x1, y0, z1 };
    glm::vec3 tl   = { x0, y0, z1 };

    Brush b;
    b.name = "pyramid";

    b.addFace(makeFace(bl, tl, tr,   material)); // base (pointing -Y)
    b.addFace(makeFace(bl, br, apex, material)); // front
    b.addFace(makeFace(br, tr, apex, material)); // right
    b.addFace(makeFace(tr, tl, apex, material)); // back
    b.addFace(makeFace(tl, bl, apex, material)); // left

    return b;
}

} // namespace BrushFactory
} // namespace editor
