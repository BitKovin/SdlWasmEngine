// editor/BrushManipulation.cpp
#include "BrushManipulation.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace editor {

// ─── Internal helpers ────────────────────────────────────────────────────────

static constexpr float kEps = 1e-5f;

static float SnapScalar(float v, float grid)
{
    return std::round(v / grid) * grid;
}

/// Build a stable orthonormal tangent/bitangent pair for any unit normal.
/// Uses world-Z as the "up" helper unless the normal is nearly parallel to it.
static void BuildTangentFrame(const glm::vec3& n, glm::vec3& t, glm::vec3& b)
{
    glm::vec3 helper = (std::abs(n.z) < 0.9f) ? glm::vec3(0, 0, 1)
                                                : glm::vec3(1, 0, 0);
    t = glm::normalize(glm::cross(helper, n));
    b = glm::cross(n, t);   // n and t are orthonormal → b is already unit
}

/// Intersect ray with plane (dot(n, p) = d).  Returns false if parallel.
static bool RayPlane(const Ray&       ray,
                     const glm::vec3& normal,
                     float            d,
                     float&           outT)
{
    float denom = glm::dot(normal, ray.direction);
    if (std::abs(denom) < kEps) return false;
    outT = (d - glm::dot(normal, ray.origin)) / denom;
    return outT >= 0.f;
}


// ─── SnapToGrid / SnapToPlaneGrid ─────────────────────────────────────────────

glm::vec3 SnapToGrid(const glm::vec3& worldPos, float gridSize)
{
    return { SnapScalar(worldPos.x, gridSize),
             SnapScalar(worldPos.y, gridSize),
             SnapScalar(worldPos.z, gridSize) };
}

// Fix for vertical-surface snapping
// ──────────────────────────────────
// Naïve per-axis rounding quantises the depth component (along the face
// normal), which is constrained by the plane equation — rounding it freely
// pulls the point off the surface.
//
// Solution: decompose the point into (depth, u, v) in the face's tangent
// frame, snap only u and v, then reconstruct on the plane.

glm::vec3 SnapToPlaneGrid(const glm::vec3& worldPos,
                          const Plane&     facePlane,
                          float            gridSize)
{
    const glm::vec3& n = facePlane.normal;

    // 1. Project worldPos back onto the plane (clamp depth to plane).
    //    plane equation: dot(n, p) = dist
    float depth   = glm::dot(n, worldPos) - facePlane.dist;
    glm::vec3 onPlane = worldPos - depth * n;   // closest point on plane

    // 2. Express onPlane relative to the plane origin (n * dist).
    glm::vec3 planeOrigin = n * facePlane.dist;
    glm::vec3 local       = onPlane - planeOrigin;

    // 3. Build tangent frame and project local offset into (u, v).
    glm::vec3 t, b;
    BuildTangentFrame(n, t, b);
    float u = glm::dot(local, t);
    float v = glm::dot(local, b);

    // 4. Snap u and v — depth stays exactly on the plane.
    u = SnapScalar(u, gridSize);
    v = SnapScalar(v, gridSize);

    return planeOrigin + t * u + b * v;
}


// ─── All-face raycast ─────────────────────────────────────────────────────────

std::vector<FaceRayHit> RaycastAllFacesOfBrush(const Ray&   ray,
                                                const Brush& brush)
{
    std::vector<FaceRayHit> hits;

    if (!brush.renderData.valid) return hits;

    for (size_t i = 0; i < brush.faces.size(); ++i)
    {
        // renderData.faces is parallel to faces — same indices.
        if (i >= brush.renderData.faces.size()) break;
        const FaceMesh& mesh = brush.renderData.faces[i];
        if (!mesh.valid || mesh.vertices.empty()) continue;

        const Face& face = brush.faces[i];

        // Get world-space polygon positions from the render cache.
        std::vector<glm::vec3> poly = mesh.positions();
        if (poly.size() < 3) continue;

        // rayIntersectPolygon from raycast.h (handles back-faces too — it
        // tests both sides because we pass the face's stored plane directly).
        std::optional<float> t = rayIntersectPolygon(
            ray, poly.data(), poly.size(), face.plane);

        if (!t.has_value()) continue;

        glm::vec3 hitPos = ray.origin + ray.direction * (*t);
        hits.push_back({ *t, brush.id, static_cast<uint32_t>(i), hitPos });
    }

    std::sort(hits.begin(), hits.end(),
              [](const FaceRayHit& a, const FaceRayHit& b){ return a.t < b.t; });
    return hits;
}

std::vector<FaceRayHit> RaycastAllFaces(const Ray& ray, const Level& level)
{
    std::vector<FaceRayHit> allHits;

    level.forEachBrush([&](const Brush& brush)
    {
        // Skip subtractive brushes for picking (they are never drawn directly).
        if (brush.mode == BrushMode::Subtractive) return;

        auto hits = RaycastAllFacesOfBrush(ray, brush);
        allHits.insert(allHits.end(), hits.begin(), hits.end());
    });

    std::sort(allHits.begin(), allHits.end(),
              [](const FaceRayHit& a, const FaceRayHit& b){ return a.t < b.t; });
    return allHits;
}


// ─── GrowFaceSelection ───────────────────────────────────────────────────────
//
// For each selected face, find every adjacent face on the same brush
// (sharing ≥ 2 vertices within kEps) whose normal is within `angleTolerance`
// degrees of the selection's average normal.  Add matching faces, then
// re-select them all via Level::selectFace.

void GrowFaceSelection(Level& level, float angleTolerance)
{
    const LevelSelection& sel = level.selection();
    if (sel.faceIndices.empty()) return;

    // Helper: true if two face polygons share at least 2 vertices.
    auto sharesEdge = [](const std::vector<glm::vec3>& polyA,
                         const std::vector<glm::vec3>& polyB) -> bool
    {
        int shared = 0;
        for (const glm::vec3& va : polyA)
            for (const glm::vec3& vb : polyB)
                if (glm::length(va - vb) < kEps) { ++shared; if (shared >= 2) return true; }
        return false;
    };

    float cosThresh = std::cos(glm::radians(angleTolerance));

    // For each brush that has selected faces:
    for (auto& [brushId, selectedFaceSet] : sel.faceIndices)
    {
        Brush* brush = level.findBrush(brushId);
        if (!brush || !brush->renderData.valid) continue;

        // Compute average normal of selected faces on this brush.
        glm::vec3 avgNormal(0.f);
        for (uint32_t fi : selectedFaceSet)
        {
            if (fi < brush->faces.size())
                avgNormal += brush->faces[fi].plane.normal;
        }
        if (glm::length(avgNormal) < kEps) continue;
        avgNormal = glm::normalize(avgNormal);

        // Cache polygons for selected faces.
        std::vector<std::pair<uint32_t, std::vector<glm::vec3>>> selPolys;
        for (uint32_t fi : selectedFaceSet)
        {
            if (fi < brush->renderData.faces.size()
                && brush->renderData.faces[fi].valid)
            {
                selPolys.push_back({ fi, brush->renderData.faces[fi].positions() });
            }
        }

        // Collect new faces to add (avoid duplicates).
        std::vector<uint32_t> toAdd;

        for (size_t ci = 0; ci < brush->faces.size(); ++ci)
        {
            uint32_t cfi = static_cast<uint32_t>(ci);
            if (selectedFaceSet.count(cfi)) continue;                  // already selected
            if (ci >= brush->renderData.faces.size()) continue;
            if (!brush->renderData.faces[ci].valid) continue;

            const Face& candidate = brush->faces[ci];

            // Normal similarity.
            if (glm::dot(candidate.plane.normal, avgNormal) < cosThresh) continue;

            // Adjacency: must share an edge with at least one selected face.
            std::vector<glm::vec3> candPoly = brush->renderData.faces[ci].positions();
            bool adjacent = false;
            for (auto& [sfi, sPoly] : selPolys)
            {
                if (sharesEdge(sPoly, candPoly)) { adjacent = true; break; }
            }
            if (!adjacent) continue;

            toAdd.push_back(cfi);
        }

        // Re-select everything (keep existing, add new).
        // First selection call uses additive=false (re-enters face mode);
        // all further use additive=true.
        bool first = true;
        for (uint32_t fi : selectedFaceSet)
        {
            level.selectFace(brushId, fi, !first);
            first = false;
        }
        for (uint32_t fi : toAdd)
        {
            level.selectFace(brushId, fi, true);
        }
    }
}


// ─── BrushManipulator — private helpers ──────────────────────────────────────

void BrushManipulator::BeginBrushDrag(const glm::vec3& hitPos,
                                       const glm::vec3& camPos)
{
    m_drag              = {};
    m_drag.type         = ManipDragType::MoveBrush;
    m_drag.active       = true;
    m_drag.origin       = hitPos;
    m_drag.cameraPos    = camPos;
    // Drag plane faces the camera: normal points from hitPos toward camera.
    m_drag.planeNormal  = glm::normalize(camPos - hitPos);
    m_drag.snappedTotal = glm::vec3(0.f);
}

void BrushManipulator::BeginFaceDrag(uint32_t         brushId,
                                      uint32_t         fi,
                                      const glm::vec3& hitPos,
                                      const glm::vec3& camPos,
                                      const Level&     level)
{
    m_drag              = {};
    m_drag.type         = ManipDragType::MoveFace;
    m_drag.active       = true;
    m_drag.origin       = hitPos;
    m_drag.cameraPos    = camPos;
    m_drag.faceBrushId  = brushId;
    m_drag.faceIdx      = fi;
    m_drag.snappedTotal = glm::vec3(0.f);

    const Brush* brush = level.findBrush(brushId);
    if (brush && fi < brush->faces.size())
        m_drag.faceNormal = brush->faces[fi].plane.normal;

    // Project along face normal → drag plane normal IS the face normal.
    m_drag.planeNormal = m_drag.faceNormal;
}


// ─── OnMouseDown ─────────────────────────────────────────────────────────────

void BrushManipulator::OnMouseDown(const Ray& viewRay, Level& level,
                                    bool altHeld, bool additive)
{
    m_drag = {};

    // Camera (ray origin) position — needed to build the drag plane.
    const glm::vec3& camPos = viewRay.origin;

    // ── ALT: face grow or initial face pick ───────────────────────────────
    if (altHeld)
    {
        const LevelSelection& sel = level.selection();

        if (!sel.faceIndices.empty())
        {
            // Grow existing face selection one ring outward.
            GrowFaceSelection(level);
            return;
        }

        // No faces selected yet — pick the front face under the cursor.
        auto topHit = level.raycast(viewRay);
        if (topHit)
        {
            level.selectBrush(topHit->brushId, additive);
            level.selectFace(topHit->brushId, topHit->faceIndex, additive);
            level.setEditMode(EditMode::Face);
        }
        return;
    }

    // ── Normal click: pick brush or face ──────────────────────────────────

    const LevelSelection& sel = level.selection();

    // Use the standard raycast for the frontmost hit first.
    auto topHit = level.raycast(viewRay);

    if (!topHit)
    {
        // Clicked empty space.
        if (!additive) level.deselectAll();
        return;
    }

    // ── Clicking on an ALREADY-SELECTED brush → face cycle ───────────────
    if (level.editMode() == EditMode::Face
        && sel.hasBrush(topHit->brushId))
    {
        // Gather all face hits for this brush (including occluded ones).
        const Brush* brush = level.findBrush(topHit->brushId);
        if (brush)
        {
            std::vector<FaceRayHit> faceHits =
                RaycastAllFacesOfBrush(viewRay, *brush);

            if (!faceHits.empty())
            {
                // Advance cycle index when clicking the same brush again.
                if (topHit->brushId != m_cycleBrushId)
                {
                    m_cycleBrushId = topHit->brushId;
                    m_cycleIndex   = 0;
                }
                else
                {
                    m_cycleIndex = (m_cycleIndex + 1)
                                   % static_cast<int>(faceHits.size());
                }

                const FaceRayHit& picked = faceHits[m_cycleIndex];

                if (additive)
                {
                    // Toggle: deselect if already selected, otherwise add.
                    if (sel.hasFace(picked.brushId, picked.faceIndex))
                    {
                        // Re-select all except this one (Level has no
                        // individual-deselect, so rebuild the set).
                        bool first = true;
                        for (auto& [bid, fiset] : sel.faceIndices)
                            for (uint32_t fi : fiset)
                                if (!(bid == picked.brushId
                                      && fi == picked.faceIndex))
                                {
                                    level.selectFace(bid, fi, !first);
                                    first = false;
                                }
                        return;
                    }
                    level.selectFace(picked.brushId, picked.faceIndex, true);
                }
                else
                {
                    level.selectFace(picked.brushId, picked.faceIndex, false);
                }

                BeginFaceDrag(picked.brushId, picked.faceIndex,
                              picked.position, camPos, level);
                return;
            }
        }
    }

    // ── Standard brush selection + drag start ────────────────────────────
    m_cycleBrushId = topHit->brushId;
    m_cycleIndex   = 0;

    if (level.editMode() == EditMode::Face)
    {
        // In face mode, clicking an unselected brush enters it.
        level.selectBrush(topHit->brushId, additive);
        level.selectFace(topHit->brushId, topHit->faceIndex, additive);
    }
    else
    {
        level.selectBrush(topHit->brushId, additive);
    }

    BeginBrushDrag(topHit->position, camPos);
}


// ─── OnMouseDrag ─────────────────────────────────────────────────────────────

void BrushManipulator::OnMouseDrag(const Ray& viewRay, Level& level)
{
    if (!m_drag.active) return;

    if (m_drag.type == ManipDragType::MoveBrush)
    {
        // Project viewRay onto the camera-facing drag plane.
        float planeD = glm::dot(m_drag.planeNormal, m_drag.origin);
        float t;
        if (!RayPlane(viewRay, m_drag.planeNormal, planeD, t)) return;

        glm::vec3 currentPos = viewRay.origin + viewRay.direction * t;
        glm::vec3 rawDelta   = currentPos - m_drag.origin;

        // Snap all three axes to the grid independently for brush movement.
        glm::vec3 snapped = SnapToGrid(rawDelta, gridSize);

        // Emit only whole-grid steps to avoid jitter.
        glm::vec3 step = snapped - m_drag.snappedTotal;
        if (glm::length(step) < kEps) return;

        level.moveSelected(step);
        m_drag.snappedTotal = snapped;
    }
    else if (m_drag.type == ManipDragType::MoveFace)
    {
        // Project viewRay onto the face plane (normal = face normal).
        float planeD = glm::dot(m_drag.faceNormal, m_drag.origin);
        float t;
        if (!RayPlane(viewRay, m_drag.faceNormal, planeD, t)) return;

        glm::vec3 currentPos = viewRay.origin + viewRay.direction * t;

        // Measure displacement along the face normal only.
        float rawPush   = glm::dot(currentPos - m_drag.origin, m_drag.faceNormal);
        float snapped   = SnapScalar(rawPush, gridSize);
        float prevTotal = glm::dot(m_drag.snappedTotal, m_drag.faceNormal);
        float step      = snapped - prevTotal;
        if (std::abs(step) < kEps) return;

        // Push every selected face along its own normal by the same scalar.
        // (If only one face is selected this is precise; for multi-face
        //  selections each face moves by the same world-unit amount.)
        const LevelSelection& sel = level.selection();
        for (auto& [bid, fiset] : sel.faceIndices)
            for (uint32_t fi : fiset)
                level.pushFace(bid, fi, step);

        m_drag.snappedTotal = m_drag.faceNormal * snapped;
    }
}


// ─── OnMouseUp ────────────────────────────────────────────────────────────────

void BrushManipulator::OnMouseUp()
{
    m_drag = {};
}


// ─── SnapForDraw ──────────────────────────────────────────────────────────────

glm::vec3 BrushManipulator::SnapForDraw(const glm::vec3& rawHit,
                                          const Plane&     activeFacePlane) const
{
    // A face is "vertical" when its normal lies mostly in the XY plane
    // (i.e. |normal.z| is small → the face stands upright).
    // For vertical faces, use plane-aware snapping so Z doesn't drift.
    const float verticalness = std::abs(activeFacePlane.normal.z);
    if (verticalness < 0.1f)
        return SnapToPlaneGrid(rawHit, activeFacePlane, gridSize);

    // Horizontal floor/ceiling → standard axis-aligned snap is fine.
    return SnapToGrid(rawHit, gridSize);
}

} // namespace editor
