// editor/mesh_builder.cpp
#include "mesh_builder.h"
#include "topology.h"
#include "tiny_csg_backend.h"

#include <algorithm>
#include <cmath>

namespace editor {

// Build a BrushRenderData cache by asking tiny_csg to generate the face
// polygons and triangulations for the brush's planes. The old data types stay
// intact; the geometry work is delegated to the backend.

static void appendTriangulatedFragment(
    FaceMesh& fm,
    const std::vector<csg::triangle_t>& tris,
    bool flip)
{
    const uint32_t base = static_cast<uint32_t>(fm.vertices.size());

    for (const auto& tri : tris) {
        if (flip) {
            fm.indices.push_back(base + static_cast<uint32_t>(tri.i));
            fm.indices.push_back(base + static_cast<uint32_t>(tri.k));
            fm.indices.push_back(base + static_cast<uint32_t>(tri.j));
        } else {
            fm.indices.push_back(base + static_cast<uint32_t>(tri.i));
            fm.indices.push_back(base + static_cast<uint32_t>(tri.j));
            fm.indices.push_back(base + static_cast<uint32_t>(tri.k));
        }
    }
}

void buildBrushMesh(Brush& brush)
{
    brush.renderData.clear();
    brush.dirty = false;

    if (!brush.isValid()) return;

    // Work on a locally transformed copy so we don't mutate brush.faces.
    const bool hasTransform = (brush.transform != glm::mat4(1.f));
    std::vector<Face> wsFaces = brush.faces;
    if (hasTransform) {
        for (Face& f : wsFaces)
            f.applyTransform(brush.transform);
    }

    csg::world_t world;
    world.set_void_volume(tiny_csg_backend::kAirVolume);

    csg::brush_t* tinyBrush = world.add();

    std::vector<csg::plane_t> tinyPlanes;
    tinyPlanes.reserve(wsFaces.size());
    for (const Face& face : wsFaces)
        tinyPlanes.push_back(tiny_csg_backend::toTinyPlane(face));

    tinyBrush->set_planes(tinyPlanes);
    tinyBrush->set_volume_operation(
        csg::make_fill_operation(tiny_csg_backend::kSolidVolume));
    tinyBrush->set_time(0);

    world.rebuild();

    const auto& tinyFaces = tinyBrush->get_faces();
    brush.renderData.faces.resize(brush.faces.size());

    const size_t faceCount = std::min(tinyFaces.size(), wsFaces.size());
    for (size_t fi = 0; fi < faceCount; ++fi) {
        const auto& tinyFace = tinyFaces[fi];
        FaceMesh& fm = brush.renderData.faces[fi];
        fm.clear();
        fm.material = brush.faces[fi].material;

        if (!tinyFace.plane) continue;

        const glm::vec3 faceNormal = tinyFace.plane->normal;

        for (const auto& fragment : tinyFace.fragments) {
            if (!tiny_csg_backend::isVisibleFragment(fragment)) continue;
            if (fragment.vertices.size() < 3) continue;

            const bool flip = tiny_csg_backend::shouldFlipFragment(fragment);
            const glm::vec3 normal = flip ? -faceNormal : faceNormal;

            const uint32_t base = static_cast<uint32_t>(fm.vertices.size());

            for (const auto& v : fragment.vertices) {
                BrushVertex bv;
                bv.position  = v.position;
                bv.normal    = normal;

                UVProjection uv = wsFaces[fi].uv;
                uv.scale = materialUvScale(wsFaces[fi].material);
                bv.uv = uv.project(v.position);

                bv.faceIndex = static_cast<uint32_t>(fi);
                fm.vertices.push_back(bv);
                brush.renderData.bounds.expand(v.position);
            }

            // Fan triangulation for polygon fragments.
            for (size_t i = 1; i + 1 < fragment.vertices.size(); ++i) {
                if (flip) {
                    fm.indices.push_back(base + 0);
                    fm.indices.push_back(base + static_cast<uint32_t>(i + 1));
                    fm.indices.push_back(base + static_cast<uint32_t>(i));
                } else {
                    fm.indices.push_back(base + 0);
                    fm.indices.push_back(base + static_cast<uint32_t>(i));
                    fm.indices.push_back(base + static_cast<uint32_t>(i + 1));
                }
            }
        }

        fm.valid = !fm.vertices.empty();
    }

    brush.renderData.valid = true;

    // Rebuild topology from the fresh per-face meshes.
    buildTopology(brush);
}

} // namespace editor
