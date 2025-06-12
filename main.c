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
    Uint64 collision_start;
    bool collision = false;
    int tile_col_index[4];

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
        
        SDL_FRect temp_collision_result;
        SDL_FRect collision_result_array[4];
        int k = 0;
        for(int i = 0; i < AMOUNTOFTILESX*AMOUNTOFTILESY; i++)
        {
            if (SDL_GetRectIntersectionFloat(&g_char.position, &g_game.tile_position_array[i], &temp_collision_result))
            {
                collision_result_array[k] = temp_collision_result;
                // SDL_Log("coll %d ", k);

                if(!collision)
                {
                    collision_start = now;
                }
                
                collision = true;
                tile_col_index[k] = i;
                k += 1;

            }
        }

        if(SDL_GetRectIntersectionFloat(&g_char.position, &g_game.store_position, &temp_collision_result))
        {
            for(int i = 0; i < 8; i++)
            {
                g_char.ores[i] = 0;
            }
            SDL_Log("Ores sold");
        }
        
        
        // TODO: add slowdown to movement when letting go of button
        if (keys[SDL_SCANCODE_W] & !collision){ g_char.position.y -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_S] & !collision){ g_char.position.y += delta*g_char.speed; }
        if (keys[SDL_SCANCODE_A] & !collision){ g_char.position.x -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_D] & !collision){ g_char.position.x += delta*g_char.speed; }

        // TODO: Move collision into separate function.
        // TODO: Make bounceback feature initial velocity based on "pressure" (time under button press) with exponential speed loss. 
        //Needs to be done between frames right so a function with static variable right?

        int char_middle_x = g_char.position.x + g_char.position.w/2;
        int char_middle_y = g_char.position.y + g_char.position.h/2; 
        if (collision && (!(keys[SDL_SCANCODE_S] || (keys[SDL_SCANCODE_A] ^ keys[SDL_SCANCODE_D])))  )
        {  
            // vector method
            // vector from center of char to center of colision
            int coll_middle_x = 0;
            int coll_middle_y = 0;

            // Calculate center of all collisions
            for(int i = 0; i < k; i++)
            {
                coll_middle_x += collision_result_array[i].x + collision_result_array[i].w/2;
                coll_middle_y += collision_result_array[i].y + collision_result_array[i].h/2;
            }

            // Get average center
            if (k > 0) {
                coll_middle_x /= k;
                coll_middle_y /= k;
                
                // Calculate push-back vector
                int v_x = char_middle_x - coll_middle_x;
                int v_y = char_middle_y - coll_middle_y;
                
                // SDL_Log("v_x position %d", v_x );
                // Apply push-back (with safety checks)
                if (v_x != 0) {
                    g_char.position.x += (v_x > 0) ? 2 : -2;
                }
                if (v_y != 0) {
                    g_char.position.y += (v_y > 0) ? 2 : -2;
                }
            }

            collision = false;

        } 
        // TODO:  remove tile with most overlap and center character into the drilled tile smoothly.
        // remove tile on "presure collision" !!!!consider the order of these if checks might be important. Do I put in functions?
        if (now - collision_start >= 500 & collision)
        {   
            // Attempt at removing the closest collision.
            // int min_ind = 0;
            // int min = abs((char_middle_x - collision_result_array[0].x + collision_result_array[0].w/2)+ (char_middle_x - collision_result_array[0].x + collision_result_array[0].w/2));
            // for(int i = 1; i<k; i++)
            // {
            //     int temp_x = abs(char_middle_x - collision_result_array[i].x + collision_result_array[i].w/2);
            //     int temp_y = abs(char_middle_x - collision_result_array[i].x + collision_result_array[i].w/2);
            //     if(abs(temp_x + temp_y) < min)
            //     {
            //         min = abs(temp_x + temp_y);
            //         min_ind = tile_col_index[i];
            //     }
            // }


            SDL_FRect temp = {.x = -100, .y = -100, .w =0, .h = 0}; //hide offscreen
            g_game.tile_position_array[tile_col_index[0]] = temp;
            collision = false;

            int total_ores = 0;
            for(int i = 0; i < 8; i++)
            {
                total_ores += g_char.ores[i];
            }
            if(total_ores < g_char.bag_capacity)
            {
                if(g_game.tile_texture_array[tile_col_index[0]] == g_game.redonium_tile )
                {
                    g_char.ores[0] += 1;
                    SDL_Log("Redtile +1");
                }
                if(g_game.tile_texture_array[tile_col_index[0]] == g_game.gold_tile)
                {
                    g_char.ores[1] += 1;
                    SDL_Log("gold tile +1");
                }
            }else if(g_game.tile_texture_array[tile_col_index[0]] != g_game.basic_tile)
            {
                SDL_Log("Bag is full, Item deleted!");
            }
            
        }

        rendering_screen();
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


    load_in_assets();
    define_inital_variables();
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


void rendering_screen()
{
        SDL_RenderClear(g_game.renderer);

        // Draw something here...
        SDL_RenderTexture(g_game.renderer, g_game.background,NULL, NULL);
        SDL_RenderTexture(g_game.renderer, g_char.texture,NULL, &g_char.position);

        
        //SDL_RenderTexture(g_game.renderer, g_game.basic_tile,NULL, &g_game.tile_position);

        for(int i = 0; i< AMOUNTOFTILESY*AMOUNTOFTILESX; i++)
        {
            SDL_RenderTexture(g_game.renderer, g_game.tile_texture_array[i], NULL, &g_game.tile_position_array[i]);
        }

        SDL_FRect healthbar_pos = {.x = 20, .y = 20 ,.w = 100, .h = 20};
        SDL_FRect healthbar_left = {.x = 0, .y = 0 ,.w = g_char.health_percentage, .h = 20};
        SDL_FRect healthbar_left_pos = {.x = 20, .y = 20 ,.w = g_char.health_percentage, .h = 20};

        SDL_RenderTexture(g_game.renderer, g_game.healthbar_empty, NULL, &healthbar_pos);
        SDL_RenderTexture(g_game.renderer, g_game.healthbar_filled, &healthbar_left, &healthbar_left_pos);

        
        SDL_RenderTexture(g_game.renderer, g_game.store, NULL, &g_game.store_position);

        SDL_RenderPresent(g_game.renderer);
}

void fill_tiles_array()
{
    for(int i = 0; i< AMOUNTOFTILESX; i++)
    {

        for(int j = 0; j < AMOUNTOFTILESY; j++)
        {
            //modulo x makes it remove around 1 in x random tiles.
            int fill_tile_basic = rand();
            int k = i+j*AMOUNTOFTILESX;
            g_game.tile_position_array[k].y = 640-j*32-32;
            g_game.tile_position_array[k].x = i*32;
            g_game.tile_position_array[k].h = 32;
            g_game.tile_position_array[k].w = 32;
            
            if(fill_tile_basic % 20 <= 15)
            {
                g_game.tile_texture_array[k] = g_game.basic_tile;    
            }
            else if (fill_tile_basic % 20 <= 18 && fill_tile_basic % 20 >= 16)
            {
                g_game.tile_texture_array[k] = g_game.gold_tile;
            }
            else if (fill_tile_basic % 20 == 19)
            {
                g_game.tile_texture_array[k] = g_game.redonium_tile;
            }
        }
    } 
}



void load_in_assets()
{
    //Loading in assets
    g_game.background =  loadTexture("assets/Background.png", g_game.renderer);
    g_char.texture = loadTexture("assets/ship.png", g_game.renderer);
    g_game.basic_tile = loadTexture("assets/Dirt.png", g_game.renderer);
    g_game.gold_tile = loadTexture("assets/goldt.png", g_game.renderer);
    g_game.redonium_tile = loadTexture("assets/Redonium.png", g_game.renderer);
    g_game.healthbar_empty = loadTexture("assets/Healthbar.png", g_game.renderer);
    g_game.healthbar_filled = loadTexture("assets/Healthbar_filled.png", g_game.renderer);
    g_game.store = loadTexture("assets/House.png", g_game.renderer);


}

void define_inital_variables()
{
    SDL_FRect temp = {.w = 32, .h = 32, .x =392, .y = 400};
    g_game.tile_position = temp;

    
    //Declare structs
    SDL_FRect temp2 = {.w = 32, .h = 32, .x =392, .y = 200};
    g_char.position = temp2;
    g_char.speed = 200.0f;
    g_char.max_health = 100;
    g_char.health = 50;
    g_char.health_percentage = g_char.max_health / 100*g_char.health; 

    g_char.bag_capacity = 5;
    for(int i = 0 ; i < sizeof(g_char.ores)/sizeof(g_char.ores[0]); i++)
    {
        g_char.ores[i] = 0;
    }

    SDL_FRect temp3 = {.x = 100, .y = 256 ,.w = 64, .h = 64};
    g_game.store_position = temp3;

}