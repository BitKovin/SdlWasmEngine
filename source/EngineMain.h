#include <SDL2/SDL_video.h>

#include "StaticMesh.hpp"

#include "AssetRegisty.h"

#include "SoundSystem/SoundManager.hpp"

#include "Time.hpp"
#include "Camera.h"
#include "Input.h"
#include "Physics.h"

#include "Level.hpp"

#include "Entities/Player.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "skinned_model.hpp"
#include "animator.hpp"

#include "ShaderManager.h"

#include "SkeletalMesh.hpp"



class EngineMain
{
private:

	StaticMesh* mesh = nullptr;

    Texture* texture = 0;

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
        SDL_ShowCursor(SDL_ENABLE);
    }

    void initDemo()
    {
        auto sound = SoundManager::GetSoundFromPath("GameData/bass_beat.wav");

        sound.Loop = true;

        //sound.Play();

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


        Level::Current->AddEntity(new Player);


    }

    void InitInputs();



    roj::Animator animator;

    ShaderProgram* shader;

    SkeletalMesh* skm;

	void Init()
	{
        SoundManager::Initialize();

        Time::Init();

        Physics::Init();

        Level::OpenLevel();

        initDemo();

        InitInputs();

        roj::ModelLoader<roj::SkinnedMesh> modelLoader;


        modelLoader.load("GameData/dog.glb");

        Logger::Log(modelLoader.getInfoLog());

        roj::SkinnedModel* mesh = new roj::SkinnedModel(modelLoader.get());

        animator = roj::Animator(*mesh);

        skm = new SkeletalMesh();
        skm->model = mesh;

        animator.set("run");
        animator.play();


        shader = ShaderManager::GetShaderProgram("skeletal");


	}

	void MainLoop()
	{
        Time::Update();
        Input::Update();


        int x, y;
        SDL_GetWindowSize(Window, &x, &y);

        float AspectRatio = ((float)x) / ((float)y);

        Camera::AspectRatio = AspectRatio;

        if(Input::GetAction("test")->Pressed())
            animator.play();

        Camera::Update(Time::DeltaTime);

        Input::UpdateMouse();
        GameUpdate();
        Render();

	}

    bool msaa = false;

	void GameUpdate()
	{
        if (Input::GetAction("test")->Pressed())
        {
            //ToggleFullscreen(window);
            printf("framerate: %f  \n", (1 / Time::DeltaTime));

            GLint samples = 0;
            glGetIntegerv(GL_SAMPLES, &samples);
            std::cout << "Samples: " << samples << std::endl;

            msaa = !msaa;

        }

        if (Input::GetAction("fullscreen")->Pressed())
        {
            ToggleFullscreen(Window);
            printf("framerate: %f  \n", (1 / Time::DeltaTime));
        }

        animator.update(Time::DeltaTimeF);

        Level::Current->Update();

	}

	void Render()
	{

        int x, y;
        SDL_GetWindowSize(Window, &x, &y);
        glViewport(0, 0, x, y);

        

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

#if DESKTOP



        if (msaa)
        {
            glEnable(GL_MULTISAMPLE);
        }
        else
        {
            glDisable(GL_MULTISAMPLE);
        }

#endif // DESKTOP

        glEnable(GL_CULL_FACE);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        skm->boneTransforms = animator.getBoneMatrices();
        
        skm->DrawForward(Camera::finalizedView, Camera::finalizedProjection, texture);



        SDL_GL_SwapWindow(Window);

	}

};