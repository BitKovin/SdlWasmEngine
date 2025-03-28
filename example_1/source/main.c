
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#if DESKTOP
#else
#include <emscripten.h>
#endif 



SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_Rect destinationRect = {0, 0, 0, 0};

Mix_Music *music = NULL;

void play_music() {
    Mix_PlayMusic(music, 0);
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
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &destinationRect);
        SDL_RenderPresent(renderer);
    }
}


void emscripten_render_loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        if (event.type == SDL_QUIT) 
        {
#if DESKTOP
#else
            emscripten_cancel_main_loop();
#endif
            break;
        }
    }
    
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &destinationRect);
    SDL_RenderPresent(renderer);
}

int main() {
    
    // Initizlize SDL, SDL_image, and SDL_mixer
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_OGG);
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        printf("Failed to initialize SDL_mixer: %s\n", Mix_GetError());
        return 1;
    }
    
    //Load sound
    music = Mix_LoadMUS("assets/sound_test.ogg");
    if (!music) {
        printf("Failed to load music: %s\n", Mix_GetError());
        return 1;
    }
    
    //Create window
    window = SDL_CreateWindow("SDL2_image Example",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              800, 600,
                              0);
    //Create Render
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    //Load image
    SDL_Surface *surface = IMG_Load("assets/happy_hog.png");
    if (!surface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return 1;
    }
    
    destinationRect.w = surface->w;
    destinationRect.h = surface->h;
    
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return 1;
    }
    
    play_music();

#if DESKTOP
    play_music();
    
    desktop_render_loop();
    
#else
    //play sound
    //emscripten_async_call(play_music, NULL, 10);
    
    //draw image
    emscripten_set_main_loop(emscripten_render_loop, 0, 1);
#endif
    
    //Cleanup
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}
