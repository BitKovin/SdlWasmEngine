#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_audio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "Input.h"
#include "SoundSystem/SoundInstance.hpp"
#include "SoundSystem/SoundManager.hpp"

#if DESKTOP
#else
#include <emscripten.h>
#endif 

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;
SDL_Rect destinationRect = { 0, 0, 0, 0 };

SoundInstance sound;

void play_music() {
	
}

void desktop_render_loop() {
	SDL_Event event;
	int quit = 0;
	while (!quit) {
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

	Input::Update();

	if (Input::GetAction("forward")->Holding())
		destinationRect.y += 1;

	if (Input::GetAction("backward")->Holding())
		destinationRect.y -= 1;

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, &destinationRect);
	SDL_RenderPresent(renderer);
}

int main() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	IMG_Init(IMG_INIT_PNG);

	// Initialize OpenAL

	SoundManager::Initialize();

	sound = SoundManager::GetSoundFromPath("assets/bass_beat.wav");

	sound.Loop = true;

	sound.Play();

	// Create window
	window = SDL_CreateWindow("SDL2_image Example",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800, 600,
		0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Surface* surface = IMG_Load("assets/happy_hog.png");
	if (!surface) {
		printf("Failed to load image: %s\n", IMG_GetError());
		return 1;
	}

	destinationRect.w = surface->w;
	destinationRect.h = surface->h;

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture) 
	{
		printf("Failed to create texture: %s\n", SDL_GetError());
		return 1;
	}

	Input::AddAction("forward")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_w));
	Input::AddAction("backward")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_s));

#if DESKTOP
	desktop_render_loop();
#else
	emscripten_set_main_loop(emscripten_render_loop, 0, 1);
#endif

;

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}
