// editor/csg.cpp
// ─────────────────────────────────────────────────────────────────────────────
// CSG Boolean subtraction backed by tiny_csg.
// ─────────────────────────────────────────────────────────────────────────────

#include "csg.h"
#include "tiny_csg_backend.h"

#include <algorithm>
#include <cmath>

namespace editor {

// ─────────────────────────────────────────────────────────────────────────────
// splitPolygon
//
// Kept for compatibility with the existing public API, but the main CSG path
// no longer depends on it.
// ─────────────────────────────────────────────────────────────────────────────

void splitPolygon(const std::vector<glm::vec3>& poly,
                  const Plane& plane,
                  std::vector<glm::vec3>& front,
                  std::vector<glm::vec3>& back)
{
    front.clear();
    back.clear();
    if (poly.size() < 3) return;

    for (size_t i = 0; i < poly.size(); ++i) {
        const glm::vec3& curr = poly[i];
        const glm::vec3& next = poly[(i + 1) % poly.size()];

        float dCurr = plane.distanceTo(curr);
        float dNext = plane.distanceTo(next);

        if (dCurr > kEpsilon)        { front.push_back(curr); }
        else if (dCurr < -kEpsilon)  { back.push_back(curr);  }
        else                         { front.push_back(curr); back.push_back(curr); }

        bool currFront = (dCurr >  kEpsilon);
        bool currBack  = (dCurr < -kEpsilon);
        bool nextFront = (dNext >  kEpsilon);
        bool nextBack  = (dNext < -kEpsilon);

        if ((currFront && nextBack) || (currBack && nextFront)) {
            float t = dCurr / (dCurr - dNext);
            glm::vec3 intersect = curr + t * (next - curr);
            front.push_back(intersect);
            back.push_back(intersect);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// clipPolyOutsideBrush
// ─────────────────────────────────────────────────────────────────────────────

void clipPolyOutsideBrush(const std::vector<glm::vec3>& poly,
                          const std::vector<Face>& subFaces,
                          size_t planeIdx,
                          std::vector<std::vector<glm::vec3>>& output)
{
    if (poly.size() < 3) return;
    if (planeIdx >= subFaces.size()) return;

    std::vector<glm::vec3> front, back;
    splitPolygon(poly, subFaces[planeIdx].plane, front, back);

    if (front.size() >= 3)
        output.push_back(front);

    if (back.size() >= 3)
        clipPolyOutsideBrush(back, subFaces, planeIdx + 1, output);
}

// ─────────────────────────────────────────────────────────────────────────────
// aabbsOverlap
// ─────────────────────────────────────────────────────────────────────────────

bool aabbsOverlap(const AABB& a, const AABB& b) noexcept
{
    return a.min.x <= b.max.x && a.max.x >= b.min.x
        && a.min.y <= b.max.y && a.max.y >= b.min.y
        && a.min.z <= b.max.z && a.max.z >= b.min.z;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildLevelCSG
// ─────────────────────────────────────────────────────────────────────────────

static void clipPolyInsideBrush(
    const std::vector<glm::vec3>& poly,
    const std::vector<Face>& brushFaces,
    size_t planeIdx,
    std::vector<std::vector<glm::vec3>>& output)
{
    if (poly.size() < 3) return;
    if (planeIdx >= brushFaces.size()) {
        output.push_back(poly);
        return;
    }

    std::vector<glm::vec3> front, back;
    splitPolygon(poly, brushFaces[planeIdx].plane, front, back);

    // Inside of the brush is the back side for the current plane convention.
    if (back.size() >= 3) {
        clipPolyInsideBrush(back, brushFaces, planeIdx + 1, output);
    }
}

LevelCSGData buildLevelCSG(
    const std::vector<const Brush*>& additiveBrushes,
    const std::vector<const Brush*>& subtractiveBrushes)
{
    LevelCSGData data;

    auto emitPolygon = [&](CSGFace& out,
                           BrushCSGResult& result,
                           const std::vector<glm::vec3>& poly,
                           const glm::vec3& normal,
                           const UVProjection& uvSrc,
                           const std::string& material,
                           uint32_t sourceBrushId,
                           uint32_t sourceFaceIdx,
                           bool flipWinding,
                           bool isCap)
    {
        if (poly.size() < 3) return;

        const uint32_t base = static_cast<uint32_t>(out.vertices.size());

        UVProjection uv = uvSrc;
        uv.scale = materialUvScale(material);

        for (const glm::vec3& p : poly) {
            BrushVertex bv;
            bv.position  = p;
            bv.normal    = normal;
            bv.uv        = uv.project(p);
            bv.faceIndex = sourceFaceIdx;

            out.vertices.push_back(bv);
            result.bounds.expand(p);
        }

        for (size_t i = 1; i + 1 < poly.size(); ++i) {
            if (flipWinding) {
                out.indices.push_back(base + 0);
                out.indices.push_back(base + static_cast<uint32_t>(i + 1));
                out.indices.push_back(base + static_cast<uint32_t>(i));
            } else {
                out.indices.push_back(base + 0);
                out.indices.push_back(base + static_cast<uint32_t>(i));
                out.indices.push_back(base + static_cast<uint32_t>(i + 1));
            }
        }

        out.material      = material;
        out.sourceBrushId  = sourceBrushId;
        out.sourceFaceIdx  = sourceFaceIdx;
        out.isCap          = isCap;
        out.valid          = !out.vertices.empty() && !out.indices.empty();
    };

    auto buildBrushResult = [&](const Brush* brush,
                                const std::vector<const Brush*>& cutters) -> BrushCSGResult
    {
        BrushCSGResult result;
        if (!brush) return result;

        result.brushId = brush->id;
        result.bounds   = AABB{};

        csg::world_t world;
        world.set_void_volume(tiny_csg_backend::kAirVolume);

        csg::brush_t* solidBrush = world.add();

        std::vector<csg::plane_t> planes;
        planes.reserve(brush->faces.size());
        for (const Face& face : brush->faces) {
            planes.push_back(tiny_csg_backend::toTinyPlane(face));
        }

        solidBrush->set_planes(planes);
        solidBrush->set_volume_operation(
            csg::make_fill_operation(tiny_csg_backend::kSolidVolume));
        solidBrush->set_time(0);

        int time = 1;
        for (const Brush* cutter : cutters) {
            if (!cutter) continue;

            csg::brush_t* tinyCut = world.add();

            std::vector<csg::plane_t> cutPlanes;
            cutPlanes.reserve(cutter->faces.size());
            for (const Face& face : cutter->faces) {
                cutPlanes.push_back(tiny_csg_backend::toTinyPlane(face));
            }

            tinyCut->set_planes(cutPlanes);
            tinyCut->set_volume_operation(
                csg::make_fill_operation(tiny_csg_backend::kAirVolume));
            tinyCut->set_time(time++);
        }

        world.rebuild();

        const auto& tinyFaces = solidBrush->get_faces();
        const size_t faceCount = std::min(tinyFaces.size(), brush->faces.size());
        result.faces.reserve(faceCount);

        for (size_t fi = 0; fi < faceCount; ++fi) {
            const auto& tinyFace = tinyFaces[fi];
            if (!tinyFace.plane) continue;

            CSGFace out;
            const Face& srcFace = brush->faces[fi];
            const glm::vec3 faceNormal = tinyFace.plane->normal;

            for (const auto& fragment : tinyFace.fragments) {
                if (!tiny_csg_backend::isVisibleFragment(fragment)) continue;
                if (fragment.vertices.size() < 3) continue;

                const bool flip = tiny_csg_backend::shouldFlipFragment(fragment);
                const glm::vec3 normal = flip ? -faceNormal : faceNormal;

                std::vector<glm::vec3> poly;
                poly.reserve(fragment.vertices.size());
                for (const auto& v : fragment.vertices) {
                    poly.push_back(v.position);
                }

                emitPolygon(out, result, poly, normal, srcFace.uv, srcFace.material,
                            brush->id, static_cast<uint32_t>(fi), flip, false);
            }

            if (out.valid) {
                result.faces.push_back(std::move(out));
            }
        }

        result.valid = !result.faces.empty();
        return result;
    };

    // Additive brushes: build final cut geometry, then add cap faces from
    // subtractive brushes using the subtractive brush's own material and UVs.
    for (const Brush* addBrush : additiveBrushes) {
        if (!addBrush) continue;

        BrushCSGResult result = buildBrushResult(addBrush, subtractiveBrushes);
        if (!result.valid) {
            data.results[addBrush->id] = std::move(result);
            continue;
        }

        for (const Brush* subBrush : subtractiveBrushes) {
            if (!subBrush) continue;
            if (!aabbsOverlap(addBrush->bounds(), subBrush->bounds())) continue;
            if (subBrush->renderData.faces.empty()) continue;

            const size_t faceCount = std::min(subBrush->faces.size(),
                                              subBrush->renderData.faces.size());

            for (size_t sfi = 0; sfi < faceCount; ++sfi) {
                const FaceMesh& subFm = subBrush->renderData.faces[sfi];
                if (!subFm.valid || subFm.vertices.size() < 3) continue;

                std::vector<glm::vec3> subPoly = subFm.positions();

                std::vector<std::vector<glm::vec3>> insidePieces;
                clipPolyInsideBrush(subPoly, addBrush->faces, 0, insidePieces);

                for (const auto& piece : insidePieces) {
                    if (piece.size() < 3) continue;

                    CSGFace cap;
                    emitPolygon(cap,
                                result,
                                piece,
                                subBrush->faces[sfi].plane.normal,
                                subBrush->faces[sfi].uv,
                                subBrush->faces[sfi].material,
                                subBrush->id,
                                static_cast<uint32_t>(sfi),
                                false,
                                true);

                    if (cap.valid) {
                        result.faces.push_back(std::move(cap));
                    }
                }
            }
        }

        result.valid = !result.faces.empty();
        data.results[addBrush->id] = std::move(result);
    }

    // Subtractive brushes should also be drawable, but they do not get cut by
    // other subtractive brushes.
    for (const Brush* subBrush : subtractiveBrushes) {
        if (!subBrush) continue;
        data.results[subBrush->id] = buildBrushResult(subBrush, {});
    }

    data.valid = true;
    return data;
}

} // namespace editor
