#include <SDL2/SDL_video.h>

#include "StaticMesh.hpp"

#include "AssetRegisty.h"

#include "SoundSystem/SoundManager.hpp"

#include "Time.hpp"
#include "Camera.h"
#include "Input.h"

class EngineMain
{
private:

	StaticMesh* mesh = nullptr;

    GLuint texture = 0;

public:

	SDL_Window* Window = nullptr;

	static EngineMain* MainInstance;

	EngineMain(SDL_Window* window)
	{
		Window = window;
	}
	~EngineMain()
	{

	}


    void ToggleFullscreen(SDL_Window* Window)
    {
        Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
        SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
        SDL_ShowCursor(IsFullscreen);
    }

	void Init()
	{

        SoundManager::Initialize();

        Time::Init();

        auto sound = SoundManager::GetSoundFromPath("GameData/bass_beat.wav");

        sound.Loop = true;

        sound.Play();

        // Fullscreen quad vertices using proper texture coordinates
        std::vector<VertexData> vertices;
        vertices.reserve(4);  // Pre-allocate memory

        // Top Right
        VertexData v1;
        v1.Position = glm::vec3(1.0f, 1.0f, 0.0f);
        v1.TextureCoordinate = glm::vec2(1.0f, 0.0f);
        v1.Color = glm::vec4(1.0f);
        vertices.push_back(v1);

        // Bottom Right
        VertexData v2;
        v2.Position = glm::vec3(1.0f, -1.0f, 0.0f);
        v2.TextureCoordinate = glm::vec2(1.0f, 1.0f);
        v2.Color = glm::vec4(1.0f);
        vertices.push_back(v2);

        // Bottom Left
        VertexData v3;
        v3.Position = glm::vec3(-1.0f, -1.0f, 0.0f);
        v3.TextureCoordinate = glm::vec2(0.0f, 1.0f);
        v3.Color = glm::vec4(1.0f);
        vertices.push_back(v3);

        // Top Left
        VertexData v4;
        v4.Position = glm::vec3(-1.0f, 1.0f, 0.0f);
        v4.TextureCoordinate = glm::vec2(0.0f, 0.0f);
        v4.Color = glm::vec4(1.0f);
        vertices.push_back(v4);

        std::vector<GLuint> indices = { 0, 1, 2, 0, 2, 3 };

        // Create buffers
        auto vb = new VertexBuffer(vertices, VertexData::Declaration());
        auto ib = new IndexBuffer(indices);
        auto vao = new VertexArrayObject(*vb, *ib);

        mesh = new StaticMesh();

        mesh->vao = vao;


        texture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

	}

	void MainLoop()
	{
        Time::Update();
        Input::Update();

        int x, y;
        SDL_GetWindowSize(Window, &x, &y);

        float AspectRatio = ((float)x) / ((float)y);

        Camera::AspectRatio = AspectRatio;

        Camera::Update(Time::DeltaTime);

        GameUpdate();
        Render();

	}

	void GameUpdate()
	{
        if (Input::GetAction("test")->Pressed())
        {
            //ToggleFullscreen(window);
            printf("framerate: %f  \n", (1 / Time::DeltaTime));
        }

        if (Input::GetAction("fullscreen")->Pressed())
        {
            ToggleFullscreen(Window);
            printf("framerate: %f  \n", (1 / Time::DeltaTime));
        }
	}

	void Render()
	{
        int x, y;
        SDL_GetWindowSize(Window, &x, &y);
        glViewport(0, 0, x, y);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearDepthf(0);

        mesh->DrawForward(Camera::finalizedView, Camera::finalizedProjection, texture);

        SDL_GL_SwapWindow(Window);

	}

};