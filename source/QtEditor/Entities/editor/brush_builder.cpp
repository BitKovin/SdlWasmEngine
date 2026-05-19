// editor/brush_builder.cpp
// ─────────────────────────────────────────────────────────────────────────────
// Incremental 3-D convex hull → unique outward planes → Brush.
//
// Algorithm (all O(n²) or better; point counts are tiny in brush editing):
//
//  Phase 1 – Seed tetrahedron
//    Find 4 non-coplanar points: furthest pair, furthest from that line,
//    furthest from that plane.  Winding is corrected so every face normal
//    points away from the hull centroid (which equals the centroid of ALL
//    input points — always inside the convex hull).
//
//  Phase 2 – Incremental insertion
//    For each remaining point P:
//      • Classify all live faces as visible (normal points toward P) or hidden.
//      • If no face is visible, P is inside the hull — skip.
//      • Collect horizon edges: unordered edges shared by exactly one visible face.
//      • Delete all visible faces.
//      • Add one new face per horizon edge, connecting it to P.
//        Winding is corrected via centroid check.
//
//  Phase 3 – Plane deduplication
//    Coplanar triangles (same normal direction AND same plane offset) are
//    merged into a single Plane entry.  This gives clean brush faces rather
//    than a pile of triangles.
//
//  Phase 4 – Brush assembly
//    For each unique plane, collect input points lying on it, pick the best
//    non-collinear triple for Face::points (for serialization), init UVs.
// ─────────────────────────────────────────────────────────────────────────────

#include "brush_builder.h"
#include "tiny_csg_backend.h"
#include <map>
#include <cmath>
#include <algorithm>
#include <limits>

namespace editor {

// ─────────────────────────────────────────────────────────────────────────────
// Internal convex hull types
// ─────────────────────────────────────────────────────────────────────────────

struct CHFace {
    int  v[3];         // indices into the input points array
    bool dead = false; // removed during incremental insertion
};

// Outward normal for `f`, oriented away from `interior`.
// Defined here so we recompute it on demand — cheap for small face counts.
static glm::vec3 outwardNormal(const std::vector<glm::vec3>& pts,
                               const CHFace& f,
                               glm::vec3 interior)
{
    glm::vec3 n = glm::cross(pts[f.v[1]] - pts[f.v[0]],
                             pts[f.v[2]] - pts[f.v[0]]);
    if (glm::dot(n, pts[f.v[0]] - interior) < 0.f) n = -n;
    float len = glm::length(n);
    return (len > kEpsilon) ? (n / len) : glm::vec3(0.f, 1.f, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Phase 1–2: incremental convex hull → triangle soup
// ─────────────────────────────────────────────────────────────────────────────

static std::vector<CHFace> buildConvexHullFaces(const std::vector<glm::vec3>& pts)
{
    const int n = static_cast<int>(pts.size());
    if (n < 4) return {};

    // Centroid of ALL input points — guaranteed to lie strictly inside the
    // convex hull, and valid before we know which 4 seed points to pick.
    // (Using only the seed tetrahedron centroid was the original bug: those
    //  indices weren't even declared yet when the old line executed.)
    glm::vec3 interior(0.f);
    for (const auto& p : pts) interior += p;
    interior /= static_cast<float>(n);

    // ── Seed: find 4 non-coplanar points ─────────────────────────────────

    // i0: anchor (just use index 0)
    int i0 = 0;

    // i1: furthest from i0
    int   i1 = -1;
    float maxD = 0.f;
    for (int i = 1; i < n; ++i) {
        float d = glm::length(pts[i] - pts[i0]);
        if (d > maxD) { maxD = d; i1 = i; }
    }
    if (i1 < 0 || maxD < kEpsilon) return {};

    // i2: furthest from the line i0–i1
    int i2 = -1;
    maxD = 0.f;
    glm::vec3 e01 = glm::normalize(pts[i1] - pts[i0]);
    for (int i = 0; i < n; ++i) {
        if (i == i0 || i == i1) continue;
        glm::vec3 v = pts[i] - pts[i0];
        float d = glm::length(v - glm::dot(v, e01) * e01); // rejection distance
        if (d > maxD) { maxD = d; i2 = i; }
    }
    if (i2 < 0 || maxD < kEpsilon) return {};

    // i3: furthest from the plane i0–i1–i2 (max |signed distance|)
    int i3 = -1;
    maxD = 0.f;
    glm::vec3 n012 = glm::normalize(glm::cross(pts[i1] - pts[i0],
                                               pts[i2] - pts[i0]));
    for (int i = 0; i < n; ++i) {
        if (i == i0 || i == i1 || i == i2) continue;
        float d = std::abs(glm::dot(pts[i] - pts[i0], n012));
        if (d > maxD) { maxD = d; i3 = i; }
    }
    if (i3 < 0 || maxD < kEpsilon) return {}; // all points coplanar

    // ── Build initial tetrahedron (4 outward-wound faces) ─────────────────

    std::vector<CHFace> faces;
    faces.reserve(static_cast<size_t>(n) * 2 + 8);

    // Adds face (a,b,c) and fixes winding so normal points away from interior.
    auto addFace = [&](int a, int b, int c) {
        CHFace f;
        f.v[0] = a; f.v[1] = b; f.v[2] = c;
        // Check winding; swap b↔c if normal points inward.
        glm::vec3 nn = glm::cross(pts[b] - pts[a], pts[c] - pts[a]);
        if (glm::dot(nn, pts[a] - interior) < 0.f) std::swap(f.v[1], f.v[2]);
        faces.push_back(f);
    };

    addFace(i0, i1, i2);
    addFace(i0, i1, i3);
    addFace(i0, i2, i3);
    addFace(i1, i2, i3);

    // ── Phase 2: add remaining points one by one ──────────────────────────

    for (int i = 0; i < n; ++i) {
        if (i == i0 || i == i1 || i == i2 || i == i3) continue;

        // Classify faces: visible = outward normal points toward pts[i].
        const int nf = static_cast<int>(faces.size());
        bool anyVisible = false;

        // We use a small inline vector instead of std::vector<bool> for perf.
        std::vector<uint8_t> vis(nf, 0);
        for (int fi = 0; fi < nf; ++fi) {
            if (faces[fi].dead) continue;
            glm::vec3 fn = outwardNormal(pts, faces[fi], interior);
            if (glm::dot(fn, pts[i] - pts[faces[fi].v[0]]) > kEpsilon) {
                vis[fi]    = 1;
                anyVisible = true;
            }
        }
        if (!anyVisible) continue; // pts[i] is inside the current hull

        // Count how many visible faces each unordered edge appears in.
        // Horizon edges have count == 1 (the other side is a hidden face).
        std::map<std::pair<int,int>, int> edgeCnt;
        for (int fi = 0; fi < nf; ++fi) {
            if (faces[fi].dead || !vis[fi]) continue;
            for (int e = 0; e < 3; ++e) {
                int a = faces[fi].v[e], b = faces[fi].v[(e + 1) % 3];
                edgeCnt[{ std::min(a, b), std::max(a, b) }]++;
            }
        }

        // Remove visible faces.
        for (int fi = 0; fi < nf; ++fi)
            if (vis[fi]) faces[fi].dead = true;

        // Connect each horizon edge to the new point.
        for (const auto& [edge, cnt] : edgeCnt) {
            if (cnt == 1) // horizon
                addFace(edge.first, edge.second, i);
        }
    }

    return faces;
}

// ─────────────────────────────────────────────────────────────────────────────
// Phase 3: extract unique outward-facing planes (merge coplanar triangles)
// ─────────────────────────────────────────────────────────────────────────────

static std::vector<Plane> extractUniquePlanes(const std::vector<glm::vec3>& pts,
                                              const std::vector<CHFace>& faces,
                                              glm::vec3 interior)
{
    std::vector<Plane> planes;

    for (const CHFace& f : faces) {
        if (f.dead) continue;

        glm::vec3 n    = outwardNormal(pts, f, interior);
        float     dist = glm::dot(n, pts[f.v[0]]);

        // Merge with an existing plane if the normals are parallel and
        // the plane offsets match (i.e. the same half-space).
        bool duplicate = false;
        for (const Plane& existing : planes) {
            bool sameDir  = (std::abs(glm::dot(existing.normal, n) - 1.f) < 0.001f);
            bool sameDist = (std::abs(existing.dist - dist) < kEpsilon * 20.f);
            if (sameDir && sameDist) { duplicate = true; break; }
        }
        if (!duplicate)
            planes.push_back({ n, dist });
    }

    return planes;
}

// ─────────────────────────────────────────────────────────────────────────────
// Phase 4: Brush assembly
// ─────────────────────────────────────────────────────────────────────────────

// From a set of points known to lie on a plane, pick the best non-collinear
// triple (maximises |cross(B-A, C-A)|) and return it wound CCW w.r.t. `normal`.
// Returns false if no non-collinear triple exists.
static bool bestTriple(const std::vector<glm::vec3>& onPlane,
                       glm::vec3 normal,
                       std::array<glm::vec3, 3>& out)
{
    float bestCross = 0.f;
    int   bi = 0, bj = 1, bk = 2;
    const int m = static_cast<int>(onPlane.size());
    for (int ii = 0; ii < m;     ++ii)
        for (int jj = ii + 1; jj < m; ++jj)
            for (int kk = jj + 1; kk < m; ++kk) {
                float c = glm::length(glm::cross(onPlane[jj] - onPlane[ii],
                                                 onPlane[kk] - onPlane[ii]));
                if (c > bestCross) { bestCross = c; bi = ii; bj = jj; bk = kk; }
            }
    if (bestCross < kEpsilon) return false; // all points collinear

    out = { onPlane[bi], onPlane[bj], onPlane[bk] };

    // Ensure CCW winding matches `normal` (so Face::computePlane() reproduces it).
    glm::vec3 testN = glm::cross(out[1] - out[0], out[2] - out[0]);
    if (glm::dot(testN, normal) < 0.f) std::swap(out[1], out[2]);

    return true;
}

// Fallback: derive 3 points on the plane from its normal + basis vectors.
// Used when fewer than 3 input points lie on a plane (shouldn't happen for a
// well-formed convex hull, but we handle it defensively).
static std::array<glm::vec3, 3> fallbackTriple(const Plane& plane)
{
    glm::vec3 up = (std::abs(plane.normal.y) < 0.9f)
    ? glm::vec3(0.f, 1.f, 0.f)
    : glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 u = glm::normalize(glm::cross(up, plane.normal));
    glm::vec3 v = glm::cross(plane.normal, u);
    glm::vec3 c = plane.normal * plane.dist;
    return { c, c + u, c + v };
}

// ─────────────────────────────────────────────────────────────────────────────
// buildBrushFromPoints  — public API
// ─────────────────────────────────────────────────────────────────────────────

Brush buildBrushFromPoints(const std::vector<glm::vec3>& points,
                           const std::string& material)
{
    Brush result;

    if (points.size() < 4) return result; // can't form a volume

    // Centroid (needed for outwardNormal throughout).
    glm::vec3 interior(0.f);
    for (const auto& p : points) interior += p;
    interior /= static_cast<float>(points.size());

    // Phase 1–2: incremental hull triangulation.
    std::vector<CHFace> hullFaces = buildConvexHullFaces(points);
    if (hullFaces.empty()) return result;

    // Phase 3: unique outward planes (coplanar triangles merged).
    std::vector<Plane> planes = extractUniquePlanes(points, hullFaces, interior);
    if (planes.size() < 4) return result; // degenerate — shouldn't happen

    // Phase 4: let tiny_csg generate the actual face polygons and validate them.
    csg::world_t world;
    world.set_void_volume(tiny_csg_backend::kAirVolume);

    csg::brush_t* tinyBrush = world.add();
    std::vector<csg::plane_t> tinyPlanes;
    tinyPlanes.reserve(planes.size());
    for (const Plane& plane : planes)
        tinyPlanes.push_back(tiny_csg_backend::toTinyPlane(plane));

    tinyBrush->set_planes(tinyPlanes);
    tinyBrush->set_volume_operation(csg::make_fill_operation(tiny_csg_backend::kSolidVolume));
    tinyBrush->set_time(0);

    world.rebuild();

    const auto& tinyFaces = tinyBrush->get_faces();
    uint32_t faceId = 1;
    result.faces.reserve(tinyFaces.size());

    for (const auto& tinyFace : tinyFaces) {
        if (!tinyFace.plane) continue;

        std::vector<glm::vec3> poly = tiny_csg_backend::polygonFromFace(tinyFace);
        if (poly.size() < 3) continue;

        Face f;
        f.id       = faceId++;
        f.plane    = tiny_csg_backend::fromTinyPlane(*tinyFace.plane);
        f.material = material;
        f.uv.initFromNormal(f.plane.normal);
        f.uv.scale = materialUvScale(material);

        f.points[0] = poly[0];
        f.points[1] = poly[1];
        f.points[2] = poly[2];

        result.faces.push_back(std::move(f));
    }

    return result;
}

} // namespace editor
