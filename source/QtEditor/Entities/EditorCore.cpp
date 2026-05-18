#include <Entity.h>
#include <Camera.h>
#include <Input.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <StaticMesh.h>

class EditorCore : public Entity
{
public:
    EditorCore();
    ~EditorCore();

    StaticMesh* mesh = nullptr;

    void LateUpdate()
    {
        mesh->Position = Camera::position;
        DrawGroundGrid(Camera::position);
    }

    void Update()
    {
        Entity::Update();


    }

    void UpdateDebugUI() override
    {



    }


    void DrawGroundGrid(const vec3& cameraPos)
    {
        const float gridSize   = 50.0f;  // half-extent from camera
        const float spacing    = 1.0f;   // grid cell size
        const float y          = 0.0f;

        const float half = gridSize;

        // Snap grid center to camera so it follows movement smoothly
        float centerX = std::floor(cameraPos.x / spacing) * spacing;
        float centerZ = std::floor(cameraPos.z / spacing) * spacing;

        // Very subtle color (assuming DebugColor::Gray exists or similar)
        uint32_t color = DebugColor::Gray;

        // Lines parallel to X axis (varying Z)
        for (float z = centerZ - half; z <= centerZ + half; z += spacing)
        {
            vec3 start(centerX - half, y, z);
            vec3 end  (centerX + half, y, z);

            DebugDraw::Line(start, end, 0.0f, 0.01f, color);
        }

        // Lines parallel to Z axis (varying X)
        for (float x = centerX - half; x <= centerX + half; x += spacing)
        {
            vec3 start(x, y, centerZ - half);
            vec3 end  (x, y, centerZ + half);

            DebugDraw::Line(start, end, 0.0f, 0.01f, color);
        }
    }

private:

};

REGISTER_ENTITY(EditorCore, "editorCore")

EditorCore::EditorCore()
{

    Camera::position = vec3(1.0f,1.0f,1.0f);


    mesh = new StaticMesh(this);
    mesh->LoadFromFile("GameData/models/cube.obj");
    mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");
    mesh->Scale = vec3(1000);
    mesh->TwoSided = true;
    Drawables.push_back(mesh);

}

EditorCore::~EditorCore()
{
}