#include <Entity.h>
#include <Camera.h>
#include <Input.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <StaticMesh.h>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>

class EditorCore : public Entity
{
public:
    EditorCore();
    ~EditorCore();

    StaticMesh* mesh = nullptr;

    // ─── Raycast result ────────────────────────────────────────────────────────
    struct SimpleHit
    {
        bool hasHit      = false;
        vec3 hitPosition{};
        vec3 hitNormal{};
    };

    // ─── State machine ─────────────────────────────────────────────────────────
    enum class BrushState
    {
        Idle,           // Nothing active
        BuildingBrush,  // LMB held: drag footprint, Alt extrudes
        PlacingPoints,  // Click-to-place convex hull points
    };
    BrushState brushState = BrushState::Idle;

    // Plane captured on the first LMB hit — all dragging stays on this plane
    vec3 planeOrigin{};
    vec3 planeNormal = vec3(0, 1, 0);

    // 2-D footprint endpoints (world space, on the plane)
    vec3 dragStart{};
    vec3 dragCurrent{};
    bool hasDragged = false;   // distinguishes click vs drag on LMB release

    // Extrusion along planeNormal (controlled by Alt + mouse Y)
    float extrudeDepth      = 0.0f;
    float extrudeBaseMouseY = 0.0f;
    bool  altWasHeld        = false;   // used to detect the rising edge of Alt
    static constexpr float kExtrudeScale = 0.01f;  // screen-pixels → world units

    // Convex hull point cloud (PlacingPoints mode)
    std::vector<vec3> hullPoints;

    // ─── Raycasting ────────────────────────────────────────────────────────────

    // Shoot against y = 0 world plane (fallback)
    SimpleHit SimpleRayCast(vec3 start, vec3 dir)
    {
        SimpleHit result;
        if (std::abs(dir.y) < 0.0001f) return result;
        float t = -start.y / dir.y;
        if (t < 0.0f) return result;
        result.hasHit      = true;
        result.hitNormal   = vec3(0, 1, 0);
        result.hitPosition = start + dir * t;
        return result;
    }

    // Shoot against an arbitrary plane (used to lock dragging to the capture plane)
    SimpleHit RayCastPlane(vec3 origin, vec3 dir, vec3 planePoint, vec3 normal)
    {
        SimpleHit result;
        float denom = glm::dot(normal, dir);
        if (std::abs(denom) < 0.0001f) return result;
        float t = glm::dot(normal, planePoint - origin) / denom;
        if (t < 0.0f) return result;
        result.hasHit      = true;
        result.hitPosition = origin + dir * t;
        result.hitNormal   = normal;
        return result;
    }

    // ─── Geometry helpers ──────────────────────────────────────────────────────

    // Two tangent axes perpendicular to planeNormal
    void GetPlaneBasis(vec3& t1, vec3& t2) const
    {
        vec3 up = (std::abs(planeNormal.y) < 0.9f) ? vec3(0, 1, 0) : vec3(1, 0, 0);
        t1 = glm::normalize(glm::cross(planeNormal, up));
        t2 = glm::normalize(glm::cross(t1, planeNormal));
    }

    // Build the 8 corners of the current brush volume.
    //   [0..3] = base quad on the capture plane
    //   [4..7] = same quad shifted by extrudeDepth along planeNormal
    std::array<vec3, 8> GetBrushCorners() const
    {
        std::array<vec3, 8> c;
        vec3 t1, t2;
        GetPlaneBasis(t1, t2);

        vec3  d = dragCurrent - dragStart;
        float a = glm::dot(d, t1);
        float b = glm::dot(d, t2);

        c[0] = dragStart;
        c[1] = dragStart + t1 * a;
        c[2] = dragStart             + t2 * b;
        c[3] = dragStart + t1 * a   + t2 * b;

        vec3 ext = planeNormal * extrudeDepth;
        for (int i = 0; i < 4; ++i)
            c[i + 4] = c[i] + ext;

        return c;
    }

    // ─── Debug drawing ─────────────────────────────────────────────────────────

    void DrawBox(vec3 min, vec3 max)
    {
        DebugDraw::Bounds(min, max, 0.01f, 0.1f, DebugColor::Green);
    }
    void DrawPoint(vec3 p)
    {
        DebugDraw::Point(p, 0.01f, 0.05f, DebugColor::Green);
    }
    void DrawLine(vec3 s, vec3 e)
    {
        DebugDraw::Line(s, e, 0.01f, 0.1f, DebugColor::Green);
    }

    void DrawBrushVisualization()
    {
        switch (brushState)
        {
        case BrushState::BuildingBrush:
        {
            if (!hasDragged) break;

            auto c = GetBrushCorners();

            // Bottom face (on the capture plane)
            DrawLine(c[0], c[1]); DrawLine(c[1], c[3]);
            DrawLine(c[3], c[2]); DrawLine(c[2], c[0]);

            // Top face (only meaningful once extrudeDepth != 0)
            DrawLine(c[4], c[5]); DrawLine(c[5], c[7]);
            DrawLine(c[7], c[6]); DrawLine(c[6], c[4]);

            // Vertical edges connecting base to top
            DrawLine(c[0], c[4]); DrawLine(c[1], c[5]);
            DrawLine(c[2], c[6]); DrawLine(c[3], c[7]);

            // Anchor points
            DrawPoint(dragStart);
            DrawPoint(dragCurrent);
            break;
        }

        case BrushState::PlacingPoints:
        {
            for (size_t i = 0; i < hullPoints.size(); ++i)
            {
                DrawPoint(hullPoints[i]);
                if (i > 0)
                    DrawLine(hullPoints[i - 1], hullPoints[i]);
            }
            // Close the polygon preview once we have 3+ points
            if (hullPoints.size() >= 3)
                DrawLine(hullPoints.back(), hullPoints.front());
            break;
        }

        default: break;
        }
    }

    // ─── Update ────────────────────────────────────────────────────────────────

    void LateUpdate()
    {
        mesh->Position = Camera::position;
        DrawGroundGrid(Camera::position);
    }

    void Update()
    {
        Entity::Update();

        const bool lmbPressed  = Input::GetAction("click")->Pressed();
        const bool lmbHolding  = Input::GetAction("click")->Holding();
        const bool lmbReleased = Input::GetAction("click")->Released();
        const bool altHolding  = Input::GetAction("growth")->Holding();
        const bool altPressed  = !altWasHeld && altHolding;  // rising edge only

        // Camera::GetMouseRay() should return a normalized world-space ray direction
        // from the camera position through the current cursor pixel.
        vec3 rayOrigin = Camera::position;
        vec3 rayDir    = Camera::GetRayDirectionFromScreenPosition(Input::MousePos);

        switch (brushState)
        {
        // ── IDLE ───────────────────────────────────────────────────────────────
        case BrushState::Idle:
        {
            SimpleHit hit = SimpleRayCast(rayOrigin, rayDir);
            if (lmbPressed && hit.hasHit)
            {
                planeOrigin  = hit.hitPosition;
                planeNormal  = hit.hitNormal;
                dragStart    = hit.hitPosition;
                dragCurrent  = hit.hitPosition;
                extrudeDepth = 0.0f;
                hasDragged   = false;
                brushState   = BrushState::BuildingBrush;
            }
            break;
        }

        // ── BUILDING BRUSH ─────────────────────────────────────────────────────
        case BrushState::BuildingBrush:
        {
            // ① While only LMB is held (no Alt), update the 2-D footprint
            if (!altHolding)
            {
                SimpleHit ph = RayCastPlane(rayOrigin, rayDir, planeOrigin, planeNormal);
                if (ph.hasHit)
                    dragCurrent = ph.hitPosition;

                if (glm::length(dragCurrent - dragStart) > 0.05f)
                    hasDragged = true;
            }

            // ② Alt rising edge → anchor the current mouse Y as extrusion baseline
            if (altPressed && hasDragged)
                extrudeBaseMouseY = ImGui::GetIO().MousePos.y;

            // ③ While Alt is held, mouse Y controls extrusion depth along planeNormal.
            //    Moving the mouse up (negative screen Y) extrudes outward.
            if (altHolding && hasDragged)
            {
                float deltaY = ImGui::GetIO().MousePos.y - extrudeBaseMouseY;
                extrudeDepth = -deltaY * kExtrudeScale;
            }

            // ④ LMB released → decide what to do
            if (lmbReleased)
            {
                if (!hasDragged)
                {
                    // Pure click (no drag) → start / continue point-cloud mode
                    hullPoints.push_back(dragStart);
                    brushState = BrushState::PlacingPoints;
                }
                else
                {
                    // Rect was drawn (extruded or flat) → hand off to Submit
                    Submit();
                    brushState = BrushState::Idle;
                    hasDragged = false;
                }
            }
            break;
        }

        // ── PLACING POINTS ─────────────────────────────────────────────────────
        case BrushState::PlacingPoints:
        {
            SimpleHit hit = SimpleRayCast(rayOrigin, rayDir);

            if (lmbPressed && hit.hasHit)
                hullPoints.push_back(hit.hitPosition);

            // "submit" action finalises the convex mesh
            if (Input::GetAction("submit")->Released())
            {
                Submit();
                hullPoints.clear();
                brushState = BrushState::Idle;
            }
            break;
        }
        }

        altWasHeld = altHolding;

        DrawBrushVisualization();
    }

    // ─── Submit ────────────────────────────────────────────────────────────────

    void Submit()
    {
        // Box brush data available here:
        //   dragStart, dragCurrent, planeNormal, extrudeDepth
        //   GetBrushCorners() → std::array<vec3, 8>  (winding: base [0-3], top [4-7])
        //
        // Convex hull data available here:
        //   hullPoints  (std::vector<vec3>)
    }

    // ─── Ground Grid ───────────────────────────────────────────────────────────

    void DrawGroundGrid(const vec3& cameraPos)
    {
        const float gridSize = 50.0f;
        const float spacing  = 1.0f;
        const float y        = 0.0f;

        float cx = std::floor(cameraPos.x / spacing) * spacing;
        float cz = std::floor(cameraPos.z / spacing) * spacing;

        uint32_t color = DebugColor::Gray;

        for (float z = cz - gridSize; z <= cz + gridSize; z += spacing)
            DebugDraw::Line({cx - gridSize, y, z}, {cx + gridSize, y, z}, 0.0f, 0.01f, color);

        for (float x = cx - gridSize; x <= cx + gridSize; x += spacing)
            DebugDraw::Line({x, y, cz - gridSize}, {x, y, cz + gridSize}, 0.0f, 0.01f, color);
    }

private:
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
}

EditorCore::~EditorCore() {}