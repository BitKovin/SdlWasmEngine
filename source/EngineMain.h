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
#include "Entities/TestCube.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "skinned_model.hpp"
#include "animator.hpp"

#include "ShaderManager.h"

#include "SkeletalMesh.hpp"

#include "ThreadPool.h"

#include <future>

#include <thread>

#include "ImGuiEngineImpl.h"

#include "UI/UiImage.hpp"

#include "UI/UiViewport.hpp"
#include "UI/UiText.hpp"

class EngineMain
{
private:

	StaticMesh* mesh = nullptr;

    Texture* texture = 0;

public:

	SDL_Window* Window = nullptr;

	static EngineMain* MainInstance;

    static UiViewport Viewport;

	EngineMain(SDL_Window* window)
	{
		Window = window;
	}
	~EngineMain()
	{

	}

    ThreadPool MainThreadPool;

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


        texture = AssetRegistry::GetTextureFromFile("GameData/cat.png");


        auto player = new Player();

        Level::Current->AddEntity(player);

        player->Position = vec3(0,3,0);

        player->Start();

        Level::Current->AddEntity(new TestCube(vec3(2, 3, 1)));
        Level::Current->AddEntity(new TestCube(vec3(2, 4, 1)));
        Level::Current->AddEntity(new TestCube(vec3(2, 4, 0.5)));
        Level::Current->AddEntity(new TestCube(vec3(2, 5, 1)));
        Level::Current->AddEntity(new TestCube(vec3(1, 5, 1)));

    }

    void InitInputs();



    roj::Animator animator;

    ShaderProgram* shader;

    SkeletalMesh* skm;

    Body* body0;

    std::shared_ptr<UiImage> img;

    std::shared_ptr<UiText> text;

	void Init()
	{

        MainThreadPool.Start();

        SoundManager::Initialize();

        Time::Init();

        Physics::Init();

        UiRenderer::Init();

        Level::OpenLevel();

        initDemo();

        InitInputs();

        skm = new SkeletalMesh();
        skm->LoadFromFile("GameData/cube.obj");
        skm->ColorTexture = texture;

        skm->Size = vec3(30,0.2f,30);


        animator = roj::Animator(skm->model);


        body0 = Physics::CreateBoxBody(nullptr, skm->Position, skm->Size, 10, true, BodyType::World);



        animator.set("run");
        animator.play();

        animator.update(0.01);

        //auto pose = animator.GetBonePoseArray();

        shader = ShaderManager::GetShaderProgram("skeletal");

        img = make_shared<UiImage>();

        img->position = vec2(100,100);
        img->size = vec2(100);

        text = make_shared<UiText>();

        text->position = vec2(0, 0);

        text->origin = vec2(0, 1);
        text->pivot = vec2(0, 1);

        text->text = "text text";

        Viewport.AddChild(img);
        Viewport.AddChild(text);

	}

    // Toggle asynchronous GameUpdate.
    bool asyncGameUpdate = true;

    // Store the future of the async update.
    std::future<void> gameUpdateFuture;

    // Main game loop.
    void MainLoop() {


        ImStartFrame();

        // Wait for game update here
        Time::Update();
        Input::Update();

        Level::Current->FinalizeFrame();
        Viewport.FinalizeChildren();

        int x, y;
        SDL_GetWindowSize(Window, &x, &y);


        float AspectRatio = static_cast<float>(x) / static_cast<float>(y);
        Camera::AspectRatio = AspectRatio;

        if (Input::GetAction("test")->Pressed())
            animator.play();

        Camera::Update(Time::DeltaTime);
        Input::UpdateMouse();

        // Start GameUpdate here, either asynchronously or synchronously.
        if (asyncGameUpdate) {
            // Optionally, check if a previous async GameUpdate is still running.

            // Launch GameUpdate asynchronously.
            gameUpdateFuture = std::async(std::launch::async, &EngineMain::GameUpdate, this);
        }
        else {
            // Run GameUpdate on the main thread.
            GameUpdate();
        }

        Render();


        if (asyncGameUpdate)
        {
            if (gameUpdateFuture.valid()) {
                // If it's not done yet, wait (or you could choose to skip/warn).
                if (gameUpdateFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                    gameUpdateFuture.wait();
                }
            }
        }


        if (Input::GetAction("fullscreen")->Pressed())
        {
            ToggleFullscreen(Window);
        }

    }

    bool msaa = false;

    static void myFunction() {
        std::cout << "Hello from the async task!" << std::endl;
        // Simulate some work with a sleep.
        std::this_thread::sleep_for(50ms);
        std::cout << "Async task finished." << std::endl;
    }

	void GameUpdate()
	{

        Physics::Simulate();

        Level::Current->UpdatePhysics();

        Level::Current->Update();

        if (Input::GetAction("test")->Pressed())
        {
            //ToggleFullscreen(window);
            printf("framerate: %f  \n", (1 / Time::DeltaTime));

            GLint samples = 0;
            glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &samples);
            std::cout << "Samples: " << samples << std::endl;

            Input::LockCursor = !Input::LockCursor;

            msaa = !msaa;

        }

        skm->Position = FromPhysics(body0->GetPosition());
        skm->Rotation = MathHelper::ToYawPitchRoll(FromPhysics(body0->GetRotation()));


		animator.update(Time::DeltaTimeF);

	}

	void Render()
	{

        skm->FinalizeFrameData();

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
        
        skm->DrawForward(Camera::finalizedView, Camera::finalizedProjection);

        //printf("renderin %i meshes\n", Level::Current->VissibleRenderList.size());

        for (IDrawMesh* mesh : Level::Current->VissibleRenderList)
        {
            mesh->DrawForward(Camera::finalizedView, Camera::finalizedProjection);
        }

        bool showdemo = true;

        ImGui::ShowDemoWindow(&showdemo);

        glDisable(GL_DEPTH_TEST);

        Viewport.Update();

        Viewport.Draw();

        RenderImGui();

        SDL_GL_SwapWindow(Window);

	}

};