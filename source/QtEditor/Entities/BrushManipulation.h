#pragma once
// editor/BrushManipulation.h
// ─────────────────────────────────────────────────────────────────────────────
// Brush / face selection and drag manipulation for the editor.
//
// Depends only on project headers; all types are from the editor namespace.
// Include this in your EditorCore / viewport input handler.
// ─────────────────────────────────────────────────────────────────────────────

#include "editor/level.h"      // Level, LevelSelection, EditMode
#include "editor/raycast.h"    // Ray, RayHit, rayIntersectPolygon
// brush.h, face.h, math_utils.h are pulled in transitively via level.h

#include <vector>
#include <optional>
#include <cstdint>

namespace editor {

// ─── Grid snapping ────────────────────────────────────────────────────────────
//
// The standard SnapToGrid rounds X, Y, and Z independently.  On a vertical
// surface the component along the face normal is plane-constrained and must
// NOT be rounded freely — doing so pulls vertices off the face plane.
// Use SnapToPlaneGrid whenever you have a face normal available.

/// Snap `worldPos` to the grid inside the plane defined by `facePlane`.
/// Only the two in-plane axes are rounded; depth along the normal is clamped
/// back to the plane, not quantised.
glm::vec3 SnapToPlaneGrid(const glm::vec3& worldPos,
                          const Plane&     facePlane,
                          float            gridSize);

/// Simple axis-aligned snap (X/Y/Z each rounded independently).
/// Only correct for perfectly horizontal floors/ceilings.
glm::vec3 SnapToGrid(const glm::vec3& worldPos, float gridSize);


// ─── All-face raycast ─────────────────────────────────────────────────────────
//
// The standard raycast returns only the closest front-facing hit.  We also
// need to reach occluded and back-facing faces (TrenchBroom-style cycling).

struct FaceRayHit {
    float     t         { std::numeric_limits<float>::max() };
    uint32_t  brushId   { 0 };
    uint32_t  faceIndex { 0 };     // index into Brush::faces / renderData.faces
    glm::vec3 position  { 0.f };
};

/// Intersect the ray against every face of `brush` (back-faces included).
/// Requires brush.renderData to be valid (call Level::rebuild() first).
/// Returns hits sorted front-to-back.
std::vector<FaceRayHit> RaycastAllFacesOfBrush(const Ray&   ray,
                                                const Brush& brush);

/// Same, but across all non-hidden, non-locked brushes in the level.
std::vector<FaceRayHit> RaycastAllFaces(const Ray& ray, const Level& level);


// ─── Face selection grow ──────────────────────────────────────────────────────
//
// Alt + click when faces are already selected: flood-fills the selection
// one ring outward along faces with similar normals (≤ angleTolerance apart).
// This matches TrenchBroom's "select touching / grow" behaviour.

void GrowFaceSelection(Level& level, float angleTolerance = 45.f);


// ─── BrushManipulator ────────────────────────────────────────────────────────
//
// Usage (call from your viewport mouse handler):
//
//   BrushManipulator manip;
//
//   void onLMBDown(Ray r, bool alt, bool shift) {
//       manip.OnMouseDown(r, level, alt, shift);
//   }
//   void onMouseMove(Ray r) {
//       if (lmbHeld) manip.OnMouseDrag(r, level);
//   }
//   void onLMBUp() { manip.OnMouseUp(); }
//
//   // In your draw-brush code, replace plain SnapToGrid with:
//   glm::vec3 snapped = manip.SnapForDraw(rawHit, activeFacePlane);

enum class ManipDragType { None, MoveBrush, MoveFace };

struct ManipDragState {
    ManipDragType type   { ManipDragType::None };
    bool          active { false };

    // World-space drag origin (brush or face hit point at LMB-down).
    glm::vec3 origin     {};
    // Camera position at LMB-down — used to build the camera-facing drag plane.
    glm::vec3 cameraPos  {};
    // Normal of the projection plane (perpendicular to camera, through origin).
    glm::vec3 planeNormal{};

    // Snapped total delta applied so far (to detect whole-grid steps).
    glm::vec3 snappedTotal{};

    // Face-drag: which face, its outward normal (for push direction).
    uint32_t  faceBrushId { 0 };
    uint32_t  faceIdx     { 0 };
    glm::vec3 faceNormal  {};
};

class BrushManipulator {
public:
    float gridSize { 16.f };

    explicit BrushManipulator(float gs = 16.f) : gridSize(gs) {}

    // ── Input events ─────────────────────────────────────────────────────

    /// LMB press.  `viewRay` is the un-projected pick ray.
    /// altHeld   = Alt key (face grow / face-pick mode).
    /// additive  = Shift key (additive selection).
    void OnMouseDown(const Ray& viewRay, Level& level,
                     bool altHeld, bool additive);

    /// Mouse moved while LMB is held.
    void OnMouseDrag(const Ray& viewRay, Level& level);

    /// LMB release — finalises and resets drag.
    void OnMouseUp();

    // ── Draw-brush snap ──────────────────────────────────────────────────

    /// Use this instead of SnapToGrid() in your brush-draw path.
    /// On vertical faces it snaps in the face's tangent plane; on
    /// horizontal faces it falls back to standard axis-aligned snap.
    glm::vec3 SnapForDraw(const glm::vec3& rawHit,
                          const Plane&     activeFacePlane) const;

    bool isDragging() const noexcept { return m_drag.active; }

private:
    ManipDragState m_drag;

    // Face-cycle state (for picking occluded faces on repeated clicks).
    uint32_t m_cycleBrushId { 0 };
    int      m_cycleIndex   { 0 };

    void BeginBrushDrag(const glm::vec3& hitPos, const glm::vec3& camPos);
    void BeginFaceDrag (uint32_t brushId, uint32_t fi,
                        const glm::vec3& hitPos, const glm::vec3& camPos,
                        const Level& level);
};

} // namespace editor
