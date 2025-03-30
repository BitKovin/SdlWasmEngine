#include <SDL2/SDL_video.h>

#include "StaticMesh.hpp"

#include "AssetRegisty.h"

#include "SoundSystem/SoundManager.hpp"

#include "Time.hpp"
#include "Camera.h"
#include "Input.h"

#include "Level.hpp"

#include "Entities/Player.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "skinned_model.hpp"
#include "animator.hpp"

#include "ShaderManager.h"

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

    void initDemo()
    {
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


        Level::Current->AddEntity(new Player);


    }

    void InitInputs();

    roj::SkinnedModel skm;

    roj::Animator animator;

    ShaderProgram* shader;

	void Init()
	{

        SoundManager::Initialize();

        Time::Init();

        Level::OpenLevel();

        initDemo();

        InitInputs();

        roj::ModelLoader<roj::SkinnedMesh> modelLoader;


        modelLoader.load("GameData/dog.glb");

        Logger::Log(modelLoader.getInfoLog());

        skm = modelLoader.get();



        animator = roj::Animator(skm);

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

        animator.update(Time::DeltaTimeF);

        Level::Current->Update();

	}

	void Render()
	{
        int x, y;
        SDL_GetWindowSize(Window, &x, &y);
        glViewport(0, 0, x, y);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearDepthf(0);


        shader->UseProgram();

        shader->AllowMissingUniforms = false;

        //shader->SetTexture("u_texture", texture);

        vec3 pos = vec3(0, 0, 0);

        vec3 rot = vec3(0);

        mat4x4 world = scale(vec3(1)) * MathHelper::GetRotationMatrix(rot) * translate(pos);

        shader->SetUniform("view", Camera::finalizedView);
        shader->SetUniform("projection", Camera::finalizedProjection);

        shader->SetUniform("world", world);

        auto transforms = animator.getBoneMatrices();

        for (int i = 0; i < transforms.size(); ++i)
            shader->SetUniform("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        auto boneMatrices = animator.getBoneMatrices();


        for (roj::SkinnedMesh& mesh : skm)
        {
            auto& indices = mesh.indices;
            uint32_t VAO = mesh.VAO;


            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glActiveTexture(GL_TEXTURE0);
        }

        SDL_GL_SwapWindow(Window);

	}

};