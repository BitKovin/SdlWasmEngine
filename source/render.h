#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "Shader.hpp"

#include "AssetRegisty.h"

#include "ShaderManager.h"

#include "Time.hpp"
#include "Input.h"

#include "Camera.h"

#include "VertexData.h"



SDL_GLContext glContext;
ShaderProgram* shader_program;
GLuint texture;
GLuint vbo;


void update_screen_size(int w, int h)
{
    glViewport(0, 0, w, h);
    SDL_SetWindowSize(window, w, h);
}

// Global objects
VertexArrayObject* vao = nullptr;
VertexBuffer* vb = nullptr;
IndexBuffer* ib = nullptr;

void init_gl() {
    shader_program = ShaderManager::GetShaderProgram();

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
    vb = new VertexBuffer(vertices, VertexData::Declaration());
    ib = new IndexBuffer(indices);
    vao = new VertexArrayObject(*vb, *ib);

    // Texture setup
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load your actual texture data here
    // unsigned char data[] = {...};
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void load_image(const char* filename)
{
    
    SDL_Surface *surface = IMG_Load(filename);
    if (!surface) {
        printf("Error loading image: %s\n", SDL_GetError());
    }
    SDL_Surface *converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted_surface->w, converted_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, converted_surface->pixels);
    
    
    // Set texture parameters (we need all these or these won't work)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    SDL_FreeSurface(converted_surface); 
    SDL_FreeSurface(surface);
    
}

void ToggleFullscreen(SDL_Window* Window) 
{
    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
    SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
    SDL_ShowCursor(IsFullscreen);
}

void Draw() {
    int x, y;
    SDL_GetWindowSize(window, &x, &y);
    glViewport(0, 0, x, y);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(0);

    shader_program->UseProgram();

    // Set texture uniform
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader_program->program, "u_texture"), 0);

    // Draw using VAO
    vao->Bind();
    glDrawElements(GL_TRIANGLES, ib->GetIndexCount(), GL_UNSIGNED_INT, 0);
    VertexArrayObject::Unbind();

    SDL_GL_SwapWindow(window);
}

void main_loop()
{
    Time::Update();
    Input::Update();

    if (Input::GetAction("test")->Pressed())
    {
        //ToggleFullscreen(window);
        printf("framerate: %f  \n", (1 / Time::DeltaTime));
    }

    if (Input::GetAction("fullscreen")->Pressed())
    {
        ToggleFullscreen(window);
        printf("framerate: %f  \n", (1 / Time::DeltaTime));
    }

    Camera::Update(Time::DeltaTime);

    Draw();

}

void desktop_render_loop()
{
    
    SDL_Event event;
    int quit = 0;   

    while (!quit) {

        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = 1;
        }

        main_loop();
        
    }
}


void emscripten_render_loop()
{
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                update_screen_size(event.window.data1, event.window.data2);
            }
            break;
        default:
            break;
        }
    }

    main_loop();
    
}
