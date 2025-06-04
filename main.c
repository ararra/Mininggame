#include "main.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    initialize_game();

    bool running = true;
    SDL_Event event;
    Uint64 now = SDL_GetTicks();
    Uint64 last = now;
    float delta;



    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        last = now;
        now = SDL_GetTicks();
        delta = (now-last)/1000.0f;

        const bool *keys = SDL_GetKeyboardState(NULL);
        

        bool collision = false;
        for(int i = 0; i < AMOUNTOFTILESX*AMOUNTOFTILESY; i++)
        {
            if (tile_colision(&g_char.position, &g_game.tile_position_array[i]))
            {
                printf("coll");
                collision = true;
            }
        }
        
        
        if (keys[SDL_SCANCODE_W] & !collision){ g_char.position.y -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_S] & !collision){ g_char.position.y += delta*g_char.speed; }
        if (keys[SDL_SCANCODE_A] & !collision){ g_char.position.x -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_D] & !collision){ g_char.position.x += delta*g_char.speed; }
        //pushes char back if collision.
        if (collision){g_char.position.y -= delta*g_char.speed;}
       

        // TODO:check collision detection and and remove tile on "presure collision"
        if (true){;}


        //TODO: create full tile set with ores randomly spread out.
        //render_tiles();


        SDL_RenderClear(g_game.renderer);



        // Draw something here...
        SDL_RenderTexture(g_game.renderer, g_game.background,NULL, NULL);
        SDL_RenderTexture(g_game.renderer, g_char.texture,NULL, &g_char.position);
        
        //SDL_RenderTexture(g_game.renderer, g_game.basic_tile,NULL, &g_game.tile_position);

        for(int i = 0; i< AMOUNTOFTILESY*AMOUNTOFTILESX; i++)
        {
            SDL_RenderTexture(g_game.renderer, g_game.basic_tile,NULL, &g_game.tile_position_array[i]);
        }



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

    g_game.window = SDL_CreateWindow("SDL3 Boilerplate", 800, 640, SDL_WINDOW_RESIZABLE);
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
    
    //Loading in assets
    g_game.background =  loadTexture("assets/Background.png", g_game.renderer);
    g_char.texture = loadTexture("assets/ship.png", g_game.renderer);
    g_game.basic_tile = loadTexture("assets/Dirt.png", g_game.renderer);
    
    SDL_FRect temp = {.w = 32, .h = 32, .x =392, .y = 400};
    g_game.tile_position = temp;

    
    //Declare structs
    SDL_FRect temp2 = {.w = 32, .h = 32, .x =392, .y = 200};
    g_char.position = temp2;
    g_char.speed = 200.0f;


    fill_tiles_array();

}


SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("IMG_Load failed: %s", SDL_GetError()); // Make sure SDL_GetError() works with IMG_Load()
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (!texture) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        return NULL;
    }

    return texture;
}


void render_tiles()
{
    for(int i = 1; i<= AMOUNTOFTILESX; i++)
    {
        SDL_RenderTexture(g_game.renderer, g_game.basic_tile,NULL, &g_game.tile_position_array[i-1]);
    }
}

void fill_tiles_array()
{
    float setprob = 0.5;
    for(int i = 0; i< AMOUNTOFTILESX; i++)
    {

        for(int j = 0; j < AMOUNTOFTILESY; j++)
        {
            
            int k = i+j*AMOUNTOFTILESX;
            g_game.tile_position_array[k].y = 640-j*32-32;
            g_game.tile_position_array[k].x = i*32;
            g_game.tile_position_array[k].h = 32;
            g_game.tile_position_array[k].w = 32;

        }
    } 
}

bool tile_colision(SDL_FRect *rect_1, SDL_FRect *rect_2)
{
    SDL_FRect result;
    return SDL_GetRectIntersectionFloat(rect_1, rect_2, &result);
}