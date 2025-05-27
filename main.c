#include "main.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    initialize_game();

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(g_game.renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(g_game.renderer);

        // Draw something here...

        SDL_RenderPresent(g_game.renderer);
    }

    SDL_DestroyRenderer(g_game.renderer);
    SDL_DestroyWindow(g_game.window);
    SDL_Quit();

    return 0;
}


void initialize_game()
{
    if (SDL_Init(SDL_INIT_VIDEO ) != 1) {
        SDL_Log("Unable to initialize SDL video: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    g_game.window = SDL_CreateWindow("SDL3 Boilerplate", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!g_game.window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    g_game.renderer = SDL_CreateRenderer(g_game.window, NULL);
    if (!g_game.renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(g_game.window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

}