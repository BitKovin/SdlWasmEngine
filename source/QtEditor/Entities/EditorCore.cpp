#include <Entity.h>
#include <Camera.h>
#include <Input.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <StaticMesh.h>
#include <vector>
#include <array>
#include <optional>
#include <algorithm>
#include <cmath>
#include "EditorExternalData.h"
#include "editor/editor.h"
#include <BoundingBox.hpp>

class EditorCore : public Entity
{
public:
    EditorCore();
    ~EditorCore();

    StaticMesh* mesh = nullptr;

    // ─── Editor level ──────────────────────────────────────────────────────────
    editor::Level level_;
    uint32_t      worldEntityId_ { 0 };

    // Pending brush — rebuilt every frame from input state, never in level_
    // until Submit() is called.
    std::optional<editor::Brush> pendingBrush_;

    // Last selection hit — stored so the UI can display info about it.
    std::optional<editor::RayHit> lastHit_;

    // ─── Floor-plane raycast (for brush placement) ─────────────────────────────

    struct SimpleHit { bool hasHit = false; vec3 hitPosition{}; vec3 hitNormal{}; };

    SimpleHit SimpleRayCast(vec3 start, vec3 dir)
    {
        SimpleHit r;
        if (std::abs(dir.y) < 0.0001f) return r;
        float t = -start.y / dir.y;
        if (t < 0.0f) return r;
        r.hasHit = true; r.hitNormal = vec3(0,1,0);
        r.hitPosition = start + dir * t;
        return r;
    }

    SimpleHit RayCastPlane(vec3 origin, vec3 dir, vec3 planePoint, vec3 normal)
    {
        SimpleHit r;
        float denom = glm::dot(normal, dir);
        if (std::abs(denom) < 0.0001f) return r;
        float t = glm::dot(normal, planePoint - origin) / denom;
        if (t < 0.0f) return r;
        r.hasHit = true; r.hitPosition = origin + dir * t; r.hitNormal = normal;
        return r;
    }

    // ─── Brush-draw state machine ──────────────────────────────────────────────
    enum class BrushState { Idle, BuildingBrush, PlacingPoints };
    BrushState brushState = BrushState::Idle;

    vec3  planeOrigin {};
    vec3  planeNormal  = vec3(0, 1, 0);
    vec3  dragStart    {};
    vec3  dragCurrent  {};
    bool  hasDragged   = false;

    float extrudeDepth      = 0.0f;
    bool  altWasHeld        = false;

    // World-space anchor and helper-plane normal for extrusion projection.
    // Set once on altPressed; used every frame while altHolding.
    vec3  extrudeAnchor      {};
    vec3  extrudePlaneNormal {};

    std::vector<vec3> hullPoints;

    // ─── Select Drag State ─────────────────────────────────────────────────────
    struct SelectDragState {
        bool active = false;
        bool isGrowing = false;

        vec3 hitNormal{};

        // For moving brushes
        vec3 planeOrigin{};
        vec3 planeNormal{};
        vec3 lastDragPosOnPlane{};
        vec3 lastDragPosXZ{};
        float lastDragPosY = 0.0f;

        // For growing faces / vertical movement projection
        vec3 extrudePlaneNormal{};
        vec3 extrudeAnchor{};
        float totalPushed = 0.0f;
    } selectDrag;

    // ─── Geometry helpers ──────────────────────────────────────────────────────

    void GetPlaneBasis(vec3& t1, vec3& t2) const
    {
        vec3 up = (std::abs(planeNormal.y) < 0.9f) ? vec3(0,1,0) : vec3(1,0,0);
        t1 = glm::normalize(glm::cross(planeNormal, up));
        t2 = glm::normalize(glm::cross(t1, planeNormal));
    }

    std::array<vec3, 8> GetBrushCorners() const
    {
        std::array<vec3, 8> c;
        vec3 t1, t2; GetPlaneBasis(t1, t2);
        vec3  d = dragCurrent - dragStart;
        float a = glm::dot(d, t1), b = glm::dot(d, t2);
        c[0] = dragStart;
        c[1] = dragStart + t1*a;
        c[2] = dragStart          + t2*b;
        c[3] = dragStart + t1*a   + t2*b;
        vec3 ext = planeNormal * extrudeDepth;
        for (int i = 0; i < 4; ++i) c[i+4] = c[i] + ext;
        return c;
    }

    // ─── Extrusion helpers ─────────────────────────────────────────────────────

    // Snap a world-space scalar (e.g. extrudeDepth) to the grid.
    float SnapToGridScalar(float v) const
    {
        if (!EditorExternalData::SnapToGrid) return v;
        float s = EditorExternalData::GridSpacing;
        return std::round(v / s) * s;
    }

    // Call once when Alt is first pressed.  Stores the anchor world point and
    // builds a helper plane whose normal lies perpendicular to planeNormal and
    // faces the camera as much as possible.  Intersecting the mouse ray with
    // this plane then projecting onto planeNormal gives a stable world-space
    // extrude depth that tracks the cursor correctly regardless of view angle.
    void BeginExtrude(vec3 anchor)
    {
        extrudeAnchor = anchor;

        // Direction from anchor toward the camera.
        vec3 toCamera = glm::normalize(Camera::position - anchor);

        // Remove the component along the extrusion axis (Gram-Schmidt).
        vec3 helperNormal = toCamera - planeNormal * glm::dot(toCamera, planeNormal);
        float len = glm::length(helperNormal);

        if (len < 0.001f)
        {
            // Camera is exactly along the extrusion axis — fall back to the
            // first plane tangent so the helper plane is still well-defined.
            vec3 t1, t2; GetPlaneBasis(t1, t2);
            helperNormal = t1;
        }
        else
        {
            helperNormal /= len;
        }
        extrudePlaneNormal = helperNormal;
    }

    // Call every frame while Alt is held.  Intersects the mouse ray with the
    // helper plane and projects onto planeNormal, then snaps to grid.
    void UpdateExtrude(vec3 rayOrigin, vec3 rayDir)
    {
        float denom = glm::dot(extrudePlaneNormal, rayDir);
        if (std::abs(denom) < 0.0001f) return;          // ray nearly parallel to plane
        float t = glm::dot(extrudePlaneNormal, extrudeAnchor - rayOrigin) / denom;
        if (t < 0.0f) return;                           // intersection behind camera
        vec3  worldHit = rayOrigin + rayDir * t;
        float raw      = glm::dot(worldHit - extrudeAnchor, planeNormal);
        extrudeDepth   = SnapToGridScalar(raw);
    }


    editor::Ray MakeMouseRay()
    {
        editor::Ray ray;
        ray.origin    = Camera::position;
        ray.direction = glm::normalize(
            Camera::GetRayDirectionFromScreenPosition(Input::MousePos));
        return ray;
    }

    // ─── Pending brush helpers ─────────────────────────────────────────────────

    std::vector<glm::vec3> GetCurrentBrushPoints() const
    {
        switch (brushState)
        {
        case BrushState::BuildingBrush:
            if (!hasDragged || std::abs(extrudeDepth) < 0.001f) return {};
            { auto c = GetBrushCorners();
                return std::vector<glm::vec3>(c.begin(), c.end()); }

        case BrushState::PlacingPoints:
            if (hullPoints.size() < 3 || std::abs(extrudeDepth) < 0.001f) return {};
            { std::vector<glm::vec3> pts;
                pts.reserve(hullPoints.size() * 2);
                for (const auto& p : hullPoints) pts.push_back(p);
                for (const auto& p : hullPoints) pts.push_back(p + planeNormal * extrudeDepth);
                return pts; }

        default: return {};
        }
    }

    void RebuildPendingBrush()
    {

        if(brushState == BrushState::BuildingBrush)
        {

            auto pts = GetCurrentBrushPoints();

            auto bounds = BoundingBox::FromPoints(pts);

            editor::Brush b = editor::BrushFactory::makeCuboid(bounds.Min, bounds.Max);

            if(EditorExternalData::SubtractiveBrush)
                b.mode = editor::BrushMode::Subtractive;

            if (!b.isValid()) { pendingBrush_.reset(); return; }

            editor::buildBrushMesh(b);
            pendingBrush_ = std::move(b);
        }
        else
        {
            auto pts = GetCurrentBrushPoints();
            if (pts.size() < 4) { pendingBrush_.reset(); return; }

            editor::Brush b = editor::buildBrushFromPoints(pts);
            if (!b.isValid()) { pendingBrush_.reset(); return; }

            editor::buildBrushMesh(b);
            pendingBrush_ = std::move(b);
        }


    }

    void Submit()
    {
        if (!pendingBrush_.has_value() || !pendingBrush_->isValid()) return;
        level_.addBrush(worldEntityId_, std::move(*pendingBrush_));
        level_.rebuild();
        level_.rebuildCSG();
        pendingBrush_.reset();
        hasDragged   = false;
        extrudeDepth = 0.0f;
        hullPoints.clear();
    }

    void CancelDraw()
    {
        pendingBrush_.reset();
        hasDragged   = false;
        extrudeDepth = 0.0f;
        hullPoints.clear();
        brushState = BrushState::Idle;
    }

    // ─── Debug drawing helpers ─────────────────────────────────────────────────

    void adjustToCamera(vec3& p)
    { vec3 d = glm::normalize(p - Camera::position); p -= d * 0.2f; }

    void DrawPoint(vec3 p) { adjustToCamera(p); DebugDraw::Point(p, 0.01f, 0.05f, DebugColor::Green); }
    void DrawLine(vec3 s, vec3 e)
    { adjustToCamera(s); adjustToCamera(e); DebugDraw::Line(s, e, 0.01f, 0.1f, DebugColor::Green); }

    void DrawBrushVisualization()
    {
        switch (brushState)
        {
        case BrushState::BuildingBrush:
        {
            if (!hasDragged) break;
            auto c = GetBrushCorners();
            DrawLine(c[0],c[1]); DrawLine(c[1],c[3]); DrawLine(c[3],c[2]); DrawLine(c[2],c[0]);
            DrawLine(c[4],c[5]); DrawLine(c[5],c[7]); DrawLine(c[7],c[6]); DrawLine(c[6],c[4]);
            DrawLine(c[0],c[4]); DrawLine(c[1],c[5]); DrawLine(c[2],c[6]); DrawLine(c[3],c[7]);
            DrawPoint(dragStart); DrawPoint(dragCurrent);
            break;
        }
        case BrushState::PlacingPoints:
        {
            for (size_t i = 0; i < hullPoints.size(); ++i) {
                DrawPoint(hullPoints[i]);
                if (i > 0) DrawLine(hullPoints[i-1], hullPoints[i]);
            }
            if (hullPoints.size() >= 3)
                DrawLine(hullPoints.back(), hullPoints.front());

            if (std::abs(extrudeDepth) > 0.001f && hullPoints.size() >= 3) {
                const vec3 ext = planeNormal * extrudeDepth;
                for (size_t i = 0; i < hullPoints.size(); ++i) {
                    vec3 top  = hullPoints[i] + ext;
                    vec3 topN = hullPoints[(i+1) % hullPoints.size()] + ext;
                    DrawPoint(top);
                    DrawLine(top, topN);
                    DrawLine(hullPoints[i], top);
                }
            }
            break;
        }
        default: break;
        }
    }

    // ─── Mode handlers ─────────────────────────────────────────────────────────

    // ── Select mode ───────────────────────────────────────────────────────────
    void UpdateSelectMode(bool lmbPressed, bool lmbReleased, bool lmbHolding,
                          bool altHolding, bool faceSelectHeld,
                          vec3 rayOrigin, vec3 rayDir)
    {
        if (Input::GetAction("delete")->Released())
        {
            auto selected = level_.selection();
            for (auto& id : selected.brushIds)
            {
                level_.removeBrush(id);
            }
            for (auto const& pair : selected.faceIndices)
            {
                level_.removeBrush(pair.first);
            }
            level_.deselectAll();
        }

        if (lmbReleased && selectDrag.active)
        {
            selectDrag.active = false;
            return;
        }

        if (selectDrag.active && lmbHolding)
        {
            if (selectDrag.isGrowing)
            {
                // Face Operation: Grow (extrude) along face normal
                float denom = glm::dot(selectDrag.extrudePlaneNormal, rayDir);
                if (std::abs(denom) > 0.0001f)
                {
                    float t = glm::dot(selectDrag.extrudePlaneNormal, selectDrag.extrudeAnchor - rayOrigin) / denom;
                    if (t >= 0.0f)
                    {
                        vec3 worldHit = rayOrigin + rayDir * t;
                        float raw = glm::dot(worldHit - selectDrag.extrudeAnchor, selectDrag.hitNormal);
                        float snappedDepth = SnapToGridScalar(raw);
                        float delta = snappedDepth - selectDrag.totalPushed;

                        if (std::abs(delta) > 0.001f)
                        {
                            auto selected = level_.selection();
                            for (auto const& pair : selected.faceIndices) {
                                for (uint32_t faceIdx : pair.second) {
                                    level_.pushFace(pair.first, faceIdx, delta);
                                }
                            }
                            selectDrag.totalPushed = snappedDepth;
                            level_.rebuild();
                            level_.rebuildCSG();
                        }
                    }
                }
            }
            else
            {
                // Brush Operation: Translate whole brush/face hierarchies
                if (altHolding)
                {
                    // Move vertically only (along Y axis)
                    float denom = glm::dot(selectDrag.extrudePlaneNormal, rayDir);
                    if (std::abs(denom) > 0.0001f)
                    {
                        float t = glm::dot(selectDrag.extrudePlaneNormal, selectDrag.extrudeAnchor - rayOrigin) / denom;
                        if (t >= 0.0f)
                        {
                            vec3 worldHit = rayOrigin + rayDir * t;
                            vec3 snappedHit = SnapToGridPoint(worldHit);
                            float deltaY = snappedHit.y - selectDrag.lastDragPosY;

                            if (std::abs(deltaY) > 0.001f)
                            {
                                vec3 delta = vec3(0.0f, deltaY, 0.0f);
                                auto selected = level_.selection();
                                std::vector<uint32_t> brushesToMove;

                                for (auto id : selected.brushIds) brushesToMove.push_back(id);
                                for (auto const& pair : selected.faceIndices) {
                                    if (std::find(brushesToMove.begin(), brushesToMove.end(), pair.first) == brushesToMove.end()) {
                                        brushesToMove.push_back(pair.first);
                                    }
                                }

                                for (auto id : brushesToMove) {
                                    if (auto* b = level_.findBrush(id)) {
                                        b->translateBy(delta);
                                    }
                                }

                                selectDrag.lastDragPosY = snappedHit.y;

                                // Sync horizontal anchor plane to avoid jumps when Alt is released
                                SimpleHit xzHit = RayCastPlane(rayOrigin, rayDir, selectDrag.planeOrigin, vec3(0, 1, 0));
                                if (xzHit.hasHit) {
                                    selectDrag.lastDragPosXZ = SnapToGridPoint(xzHit.hitPosition);
                                }

                                level_.rebuild();
                                level_.rebuildCSG();
                            }
                        }
                    }
                }
                else
                {
                    // Move horizontally only (along XZ plane)
                    SimpleHit hit = RayCastPlane(rayOrigin, rayDir, selectDrag.planeOrigin, vec3(0, 1, 0));
                    if (hit.hasHit)
                    {
                        vec3 currentPos = SnapToGridPoint(hit.hitPosition);
                        vec3 delta = vec3(currentPos.x - selectDrag.lastDragPosXZ.x, 0.0f, currentPos.z - selectDrag.lastDragPosXZ.z);
                        if (glm::length(delta) > 0.001f)
                        {
                            auto selected = level_.selection();
                            std::vector<uint32_t> brushesToMove;

                            for (auto id : selected.brushIds) brushesToMove.push_back(id);
                            for (auto const& pair : selected.faceIndices) {
                                if (std::find(brushesToMove.begin(), brushesToMove.end(), pair.first) == brushesToMove.end()) {
                                    brushesToMove.push_back(pair.first);
                                }
                            }

                            for (auto id : brushesToMove) {
                                if (auto* b = level_.findBrush(id)) {
                                    b->translateBy(delta);
                                }
                            }

                            selectDrag.lastDragPosXZ = currentPos;

                            // Sync vertical tracking depth anchor to avoid jumps when Alt is pressed
                            float denom = glm::dot(selectDrag.extrudePlaneNormal, rayDir);
                            if (std::abs(denom) > 0.0001f) {
                                float t = glm::dot(selectDrag.extrudePlaneNormal, selectDrag.extrudeAnchor - rayOrigin) / denom;
                                if (t >= 0.0f) {
                                    vec3 worldHit = rayOrigin + rayDir * t;
                                    selectDrag.lastDragPosY = SnapToGridPoint(worldHit).y;
                                }
                            }

                            level_.rebuild();
                            level_.rebuildCSG();
                        }
                    }
                }
            }
            return;
        }

        if (!lmbPressed) return;

        editor::Ray ray = MakeMouseRay();
        auto hit = level_.raycast(ray);

        if (!hit)
        {
            level_.deselectAll();
            lastHit_.reset();
            return;
        }

        lastHit_ = hit;

        auto selected = level_.selection();
        // Disambiguate Face operations from Brush operations to prevent input mapping conflicts
        bool isFaceOp = faceSelectHeld || (!selected.faceIndices.empty() && selected.brushIds.empty());

        if (isFaceOp)
        {
            if (altHolding)
            {
                // Face operation path: setup face extrusion selection
                bool alreadySelected = false;
                auto it = selected.faceIndices.find(hit->brushId);
                if (it != selected.faceIndices.end() && it->second.count(hit->faceIndex)) {
                    alreadySelected = true;
                }

                if (!alreadySelected) {
                    level_.selectFace(hit->brushId, hit->faceIndex, /*additive=*/false);
                }

                selectDrag.active = true;
                selectDrag.isGrowing = true;
                selectDrag.hitNormal = hit->normal;
                selectDrag.totalPushed = 0.0f;
                selectDrag.extrudeAnchor = hit->position;

                vec3 toCamera = glm::normalize(Camera::position - selectDrag.extrudeAnchor);
                vec3 helperNormal = toCamera - selectDrag.hitNormal * glm::dot(toCamera, selectDrag.hitNormal);
                float len = glm::length(helperNormal);

                if (len < 0.001f) {
                    vec3 up = (std::abs(selectDrag.hitNormal.y) < 0.9f) ? vec3(0,1,0) : vec3(1,0,0);
                    helperNormal = glm::normalize(glm::cross(selectDrag.hitNormal, up));
                } else {
                    helperNormal /= len;
                }
                selectDrag.extrudePlaneNormal = helperNormal;
            }
            else
            {
                // Normal face selection selection logic
                bool alreadySelected = false;
                auto it = selected.faceIndices.find(hit->brushId);
                if (it != selected.faceIndices.end() && it->second.count(hit->faceIndex)) {
                    alreadySelected = true;
                }

                if (!alreadySelected) {
                    level_.selectFace(hit->brushId, hit->faceIndex, /*additive=*/false);
                }

                // Setup baseline anchors for dragging its parent brush structures
                selectDrag.active = true;
                selectDrag.isGrowing = false;
                selectDrag.planeOrigin = hit->position;
                selectDrag.planeNormal = vec3(0, 1, 0);
                selectDrag.lastDragPosXZ = SnapToGridPoint(hit->position);
                selectDrag.lastDragPosY = SnapToGridPoint(hit->position).y;

                vec3 toCamera = glm::normalize(Camera::position - hit->position);
                vec3 vHelper = toCamera - vec3(0, 1, 0) * glm::dot(toCamera, vec3(0, 1, 0));
                float vLen = glm::length(vHelper);
                selectDrag.extrudePlaneNormal = (vLen < 0.001f) ? vec3(1,0,0) : vHelper / vLen;
                selectDrag.extrudeAnchor = hit->position;
            }
        }
        else
        {
            // Brush operation path: setup standard moving constraints
            bool alreadySelected = selected.brushIds.count(hit->brushId) > 0;
            if (!alreadySelected) {
                level_.selectBrush(hit->brushId, /*additive=*/false);
            }

            selectDrag.active = true;
            selectDrag.isGrowing = false;
            selectDrag.planeOrigin = hit->position;
            selectDrag.planeNormal = vec3(0, 1, 0); // Enforce XZ plane alignment for normal drag operations
            selectDrag.lastDragPosXZ = SnapToGridPoint(hit->position);
            selectDrag.lastDragPosY = SnapToGridPoint(hit->position).y;

            // Generate camera-aligned vertical helper plane reference
            vec3 toCamera = glm::normalize(Camera::position - hit->position);
            vec3 vHelper = toCamera - vec3(0, 1, 0) * glm::dot(toCamera, vec3(0, 1, 0));
            float vLen = glm::length(vHelper);
            selectDrag.extrudePlaneNormal = (vLen < 0.001f) ? vec3(1,0,0) : vHelper / vLen;
            selectDrag.extrudeAnchor = hit->position;
        }
    }

    // ── BrushDraw mode ────────────────────────────────────────────────────────
    void UpdateBrushDrawMode(bool lmbPressed, bool lmbReleased,
                             bool altHolding, bool altPressed,
                             bool escPressed,
                             vec3 rayOrigin, vec3 rayDir)
    {
        if (escPressed && brushState != BrushState::Idle)
        {
            CancelDraw();
            return;
        }

        const bool canExtrude =
            (brushState == BrushState::BuildingBrush && hasDragged) ||
            (brushState == BrushState::PlacingPoints && hullPoints.size() >= 3);

        if (altPressed && canExtrude)
        {
            vec3 anchor(0);
            if (brushState == BrushState::BuildingBrush)
            {
                anchor = (dragStart + dragCurrent) * 0.5f;
            }
            else
            {
                for (const auto& p : hullPoints) anchor += p;
                anchor /= static_cast<float>(hullPoints.size());
            }
            BeginExtrude(anchor);
        }
        if (altHolding && canExtrude)
            UpdateExtrude(rayOrigin, rayDir);

        switch (brushState)
        {
        case BrushState::Idle:
        {
            editor::Ray ray = MakeMouseRay();
            auto levelHit   = level_.raycast(ray);

            vec3 hitPos;
            vec3 hitNorm = vec3(0, 1, 0);

            if (levelHit)
            {
                hitPos  = levelHit->position;
                hitNorm = levelHit->normal;
            }
            else
            {
                SimpleHit floorHit = SimpleRayCast(rayOrigin, rayDir);
                if (!floorHit.hasHit || !lmbPressed) break;
                hitPos  = floorHit.hitPosition;
                hitNorm = floorHit.hitNormal;
            }

            if (lmbPressed)
            {
                planeOrigin  = hitPos;
                planeNormal  = hitNorm;
                dragStart    = SnapToGridPoint(hitPos);
                dragCurrent  = dragStart;
                extrudeDepth = 0.0f;
                hasDragged   = false;
                brushState   = BrushState::BuildingBrush;
            }
            break;
        }

        case BrushState::BuildingBrush:
        {
            if (!altHolding)
            {
                SimpleHit ph = RayCastPlane(rayOrigin, rayDir, planeOrigin, planeNormal);
                if (ph.hasHit)
                    dragCurrent = SnapToGridPoint(ph.hitPosition);

                if (glm::length(dragCurrent - dragStart) > 0.05f)
                    hasDragged = true;
            }

            if (lmbReleased)
            {
                if (!hasDragged)
                {
                    hullPoints.push_back(dragStart);
                    extrudeDepth = 0.0f;
                    brushState   = BrushState::PlacingPoints;
                    brushState = BrushState::Idle;
                }
                else
                {
                    Submit();
                    brushState = BrushState::Idle;
                }
            }
            break;
        }

        case BrushState::PlacingPoints:
        {
            SimpleHit hit = SimpleRayCast(rayOrigin, rayDir);
            if (lmbPressed && hit.hasHit)
                hullPoints.push_back(SnapToGridPoint(hit.hitPosition));

            if (Input::GetAction("submit")->Released())
            {
                Submit();
                brushState = BrushState::Idle;
            }
            break;
        }
        }
    }

    // ─── Update ────────────────────────────────────────────────────────────────

    void LateUpdate() { mesh->Position = Camera::position; }

    void Update()
    {
        Entity::Update();
        DrawGroundGrid(Camera::position);

        DrawLevel();

        const bool lmbPressed      = Input::GetAction("click")->Pressed();
        const bool lmbReleased     = Input::GetAction("click")->Released();
        const bool lmbHolding      = Input::GetAction("click")->Holding();
        const bool altHolding      = Input::GetAction("growth")->Holding();
        const bool altPressed      = !altWasHeld && altHolding;
        const bool escPressed      = Input::GetAction("esc")->Pressed();
        const bool faceSelectHeld  = Input::GetAction("faceSelect")->Holding();

        vec3 rayOrigin = Camera::position;
        vec3 rayDir    = Camera::GetRayDirectionFromScreenPosition(Input::MousePos);

        switch (EditorExternalData::editMode)
        {
        case EditorExternalData::EditMode::None:
            if (escPressed) level_.deselectAll();
            break;

        case EditorExternalData::EditMode::Select:
            if (brushState != BrushState::Idle) CancelDraw();
            UpdateSelectMode(lmbPressed, lmbReleased, lmbHolding, altHolding, faceSelectHeld, rayOrigin, rayDir);
            break;

        case EditorExternalData::EditMode::BrushDraw:
            UpdateBrushDrawMode(lmbPressed, lmbReleased,
                                altHolding, altPressed,
                                escPressed, rayOrigin, rayDir);
            RebuildPendingBrush();
            break;
        }

        altWasHeld = altHolding;

        level_.rebuild();
        level_.rebuildCSG();

        DrawBrushVisualization();
    }

    // ─── Ground Grid ───────────────────────────────────────────────────────────

    void DrawGroundGrid(const vec3& cameraPos)
    {
        const float gridSize = 50.0f;
        const float spacing  = EditorExternalData::GridSpacing;
        const float y        = 0.0f;
        float cx = std::floor(cameraPos.x / spacing) * spacing;
        float cz = std::floor(cameraPos.z / spacing) * spacing;
        uint32_t color = DebugColor::Gray;
        for (float z = cz-gridSize; z <= cz+gridSize; z += spacing)
            DebugDraw::Line({cx-gridSize,y,z},{cx+gridSize,y,z}, 0.0f, 0.01f, color);
        for (float x = cx-gridSize; x <= cx+gridSize; x += spacing)
            DebugDraw::Line({x,y,cz-gridSize},{x,y,cz+gridSize}, 0.0f, 0.01f, color);
    }

private:

    vec3 SnapToGridPoint(vec3 p) const
    {
        if (!EditorExternalData::SnapToGrid) return p;
        float s = EditorExternalData::GridSpacing;
        return vec3(std::round(p.x/s)*s, std::round(p.y/s)*s, std::round(p.z/s)*s);
    }

    void DrawLevel()
    {
        auto selected = level_.selection();

        for (const editor::Group& g : level_.groups()) {
            if (g.hidden) continue;
            for (const editor::Entity& e : g.entities) {
                for (const editor::Brush& b : e.brushes) {

                    auto csgBrush = level_.csgData().find(b.id);
                    if(csgBrush == nullptr) continue;

                    for(const editor::CSGFace& face : csgBrush->faces)
                    {
                        auto color = DebugColor::Orange;

                        if(b.mode == editor::BrushMode::Subtractive && face.isCap == false)
                        {
                            color = DebugColor::Yellow;
                        }

                        if(selected.hasFace(face.sourceBrushId,face.sourceFaceIdx) || selected.hasBrush(face.sourceBrushId))
                        {
                            color = DebugColor::Blue;
                        }

                        std::vector<vec3> vertexPositions;
                        for (auto& vertex : face.vertices)
                        {
                            vertexPositions.push_back(vertex.position);
                        }

                        DebugDraw::IndexedMesh(vertexPositions,face.indices, 0.01f,0.1, color);
                    }
                }
            }
        }
    }
};

REGISTER_ENTITY(EditorCore, "editorCore")

EditorCore::EditorCore()
{
    Camera::position = vec3(1.0f, 1.0f, 1.0f);
    mesh = new StaticMesh(this);
    mesh->LoadFromFile("GameData/models/cube.obj");
    mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");
    mesh->Scale = vec3(1000);
    mesh->TwoSided = true;
    Drawables.push_back(mesh);

    const auto& groups = level_.groups();
    if (!groups.empty() && !groups[0].entities.empty())
        worldEntityId_ = groups[0].entities[0].id;
}

EditorCore::~EditorCore() {}