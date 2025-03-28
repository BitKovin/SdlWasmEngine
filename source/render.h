#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "Time.hpp"
#include "Input.h"

const char* vertex_shader_source =
"attribute vec2 position;\n"
"attribute vec2 texcoord;\n"
"varying vec2 v_texcoord;\n"
"void main() {\n"
"    v_texcoord = texcoord;\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"}\n";

const char* fragment_shader_source =
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"void main() {\n"
"    gl_FragColor = texture2D(tex, v_texcoord);\n"
"}\n";



SDL_GLContext glContext;
GLuint shader_program;
GLuint texture;
GLuint vbo;


void update_screen_size(int w, int h)
{
    glViewport(0, 0, w, h);
    SDL_SetWindowSize(window, w, h);
}

void init_gl()
{
    
    // Compile shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    
    // Link shaders
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    // Set up quad vertices
    GLfloat vertices[] = {
        // Positions    // Texture Coords
        1.0f,  1.0f,   1.0f, 0.0f, // Top Right
        1.0f, -1.0f,   1.0f, 1.0f, // Bottom Right
        -1.0f, -1.0f,   0.0f, 1.0f, // Bottom Left
        1.0f,  1.0f,   1.0f, 0.0f, // Top Right
        -1.0f, -1.0f,   0.0f, 1.0f, // Bottom Left
        -1.0f,  1.0f,   0.0f, 0.0f  // Top Left
    };
    
    // Create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Specify vertex attributes
    GLint posAttrib = glGetAttribLocation(shader_program, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    
    GLint texAttrib = glGetAttribLocation(shader_program, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    

    Input::AddAction("test")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_w));

    update_screen_size(512, 512);

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

void main_loop()
{
    Time::Update();
    Input::Update();


    if (Input::GetAction("test")->Pressed())
    {
        ToggleFullscreen(window);
    }

    int x, y;
    SDL_GetWindowSize(window, &x, &y);
    glViewport(0, 0, x, y);

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);


    // Use shader program
    glUseProgram(shader_program);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // Draw the quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Present the renderer
    SDL_GL_SwapWindow(window);
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
