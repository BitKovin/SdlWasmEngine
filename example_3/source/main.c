
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#if DESKTOP
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL_opengles2.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>
#include <emscripten.h>
#endif 

#define STB_TRUETYPE_IMPLEMENTATION
#include "3rd_party/stb_truetype.h"
#include "3rd_party/json.h"



#include "prodefs.h"  //imports the type names i like to use
#include "util.h"  //simple utility functions
#include "m_math.h"  // some math functions and structs
#include "platform.h"  //has a controller struct for input handling

SDL_Window *window;

u32 current_time = 0;
u32 last_time_updated = 0;
GameInput input[2] = {0};
GameInput *new_input;
GameInput *old_input;
GameControllerInput* new_controller;
GameControllerInput* old_controller;
u32 loop_time = 0;


Mix_Music *music = NULL;
Mix_Chunk *yummy_snd = NULL;

#define CAM_WIDTH 800
#define CAM_HEIGHT 600


#include "assets.h"
#include "sprite.h"
#include "entity.h"
#include "render.h"
#include "hog.h" //game core


#include "assets.c"
#include "sprite.c"
#include "render.c"
#include "entity.c"
#include "entity_sim.c"
#include "hog.c"


void
process_key_press(GameButtonState* new_button, GameButtonState* old_button, b32 ended_down)
{
    new_button->ended_down = ended_down;
    if(!old_button->ended_down && ended_down)
    {
        new_button->pressed = true;
    }
}




void desktop_render_loop()
{  
    
    SDL_Event event;
    int quit = 0;
    //timeBeginPeriod(1);
    while (!quit)
    {
        // Handle events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                quit = 1;
        }
        
        
        *new_input = (GameInput){0};
        
        new_controller = &new_input->controller;
        old_controller = &old_input->controller;
        
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        
        process_key_press(&new_controller->move_left, &old_controller->move_left, currentKeyStates[SDL_SCANCODE_LEFT]);
        process_key_press(&new_controller->move_right, &old_controller->move_right, currentKeyStates[SDL_SCANCODE_RIGHT]);
        process_key_press(&new_controller->move_up, &old_controller->move_up, currentKeyStates[SDL_SCANCODE_UP]);
        process_key_press(&new_controller->move_down, &old_controller->move_down, currentKeyStates[SDL_SCANCODE_DOWN]);
        process_key_press(&new_controller->action_right, &old_controller->action_right, currentKeyStates[SDL_SCANCODE_C]);
        process_key_press(&new_controller->start, &old_controller->start, currentKeyStates[SDL_SCANCODE_Z]);
        
        
        current_time = SDL_GetTicks();
        new_input->elapsed_seconds = ((f32)current_time - (f32)last_time_updated)/1000.0f;
        
        //printf("curr: %d last: %d elsaped: %f \n",current_time, last_time_updated, new_input->elapsed_seconds);
        hog_update(new_input);
        
        SDL_GL_SwapWindow(window);
        
        GameInput *temp = new_input;
        new_input = old_input;
        old_input = temp;
        
        
        
        last_time_updated = current_time;
        local_persist i32 sleep_time;
        loop_time = SDL_GetTicks() - last_time_updated;
        sleep_time = (1000 / 60) -(loop_time); 
        if(sleep_time > 0)
        {
            
            //printf("loop_time %d, sleep_time: %d\n",loop_time, sleep_time);
            SDL_Delay(sleep_time);
        }
    }
}


void emscripten_render_loop()
{
    
    
    
    
    
    
    *new_input = (GameInput){0};
    
    new_controller = &new_input->controller;
    old_controller = &old_input->controller;
    
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    
    process_key_press(&new_controller->move_left, &old_controller->move_left, currentKeyStates[SDL_SCANCODE_LEFT]);
    process_key_press(&new_controller->move_right, &old_controller->move_right, currentKeyStates[SDL_SCANCODE_RIGHT]);
    process_key_press(&new_controller->move_up, &old_controller->move_up, currentKeyStates[SDL_SCANCODE_UP]);
    process_key_press(&new_controller->move_down, &old_controller->move_down, currentKeyStates[SDL_SCANCODE_DOWN]);
    process_key_press(&new_controller->action_right, &old_controller->action_right, currentKeyStates[SDL_SCANCODE_C]);
    process_key_press(&new_controller->start, &old_controller->start, currentKeyStates[SDL_SCANCODE_Z]);
    
    
    current_time = SDL_GetTicks();
    new_input->elapsed_seconds = ((f32)current_time - (f32)last_time_updated)/1000.0f;
    
    //printf("curr: %d last: %d elsaped: %f \n",current_time, last_time_updated, new_input->elapsed_seconds);
    hog_update(new_input);
    
    SDL_GL_SwapWindow(window);
    
    GameInput *temp = new_input;
    new_input = old_input;
    old_input = temp;
    
    
    
    last_time_updated = current_time;
    local_persist i32 sleep_time;
    loop_time = SDL_GetTicks() - last_time_updated;
    sleep_time = (1000 / 60) -(loop_time); 
    if(sleep_time > 0)
    {
        
        //printf("loop_time %d, sleep_time: %d\n",loop_time, sleep_time);
        SDL_Delay(sleep_time);
    }
}



void play_music() {
    Mix_PlayMusic(music, -1);
}




int main() {
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    
    Mix_Init(MIX_INIT_OGG);
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        printf("Failed to initialize SDL_mixer: %s\n", Mix_GetError());
        return 1;
    }
    
    //Load sound
    music = Mix_LoadMUS("assets/bass_beat.ogg");
    if (!music) {
        printf("Failed to load music: %s\n", Mix_GetError());
        return 1;
    }
    yummy_snd = Mix_LoadWAV("assets/yummy.ogg");
    
    play_music();
    // Create SDL window
    
    window = SDL_CreateWindow("Happy Hog", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CAM_WIDTH, CAM_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Surface* icon = IMG_Load("assets/mini_hog.ico");
    SDL_SetWindowIcon(window, icon);
    
#if DESKTOP
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#else
    // Create OpenGL ES context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        fprintf(stderr, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        
    }
#if DESKTOP
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glewError));
        
    }
#endif
    
    
    init_gl();
    
    
    
    
    new_input = &input[0];
    old_input = &input[1];
    new_controller = &new_input->controller;
    old_controller = &old_input->controller;
    
    
    current_time = SDL_GetTicks();
    last_time_updated = SDL_GetTicks();
    
    
    //start game
    hog_init();
    // Run main loop
#if DESKTOP
    desktop_render_loop();
#else
    emscripten_set_main_loop(emscripten_render_loop, 0, 1);
#endif
    hog_clean_up();
    
    Mix_HaltChannel(-1);
    Mix_HaltMusic();
    Mix_FreeChunk(yummy_snd);
    Mix_FreeMusic(music);
    
    // Clean up
    SDL_GL_DeleteContext(glContext);
    
    SDL_DestroyWindow(window);
    SDL_FreeSurface(icon);
    
    SDL_Quit();
    
    return 0;
}

