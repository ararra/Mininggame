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
    int tile_col_index;

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
                k += 1;
                SDL_Log("coll %d ", k);

                if(!collision)
                {
                    collision_start = now;
                }
                
                collision = true;
                tile_col_index = i;
            }
        }
        
        
        // TODO: add slowdown to movement when letting go of button
        if (keys[SDL_SCANCODE_W] & !collision){ g_char.position.y -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_S] & !collision){ g_char.position.y += delta*g_char.speed; }
        if (keys[SDL_SCANCODE_A] & !collision){ g_char.position.x -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_D] & !collision){ g_char.position.x += delta*g_char.speed; }


        //TODO: Make bounceback feature initial velocity based on "pressure" (time under button press) with exponential speed loss. 
        //Needs to be done between frames right so a function with static variable right?


        if (collision && (!(keys[SDL_SCANCODE_S] || (keys[SDL_SCANCODE_A] ^ keys[SDL_SCANCODE_D])))  )
        {   
            //the constant needs to be there otherwise the colision persists for a few frames and it activates the tile collision if check above several times.
            // use the saved_result rect from the collision to create a direction for the bounce back.

            // this method not quite working
            // float saved_result_x_sum = saved_result[0].x +saved_result[1].x +saved_result[2].x;
            // float saved_result_y_sum = saved_result[0].y +saved_result[1].y +saved_result[2].y;
            // int power_y = (saved_result[0].y > g_char.position.y);
            //int power_x = (saved_result[0].x > g_char.position.x); 
            // g_char.position.y =g_char.position.y + pow(-1,power_y)*(5+saved_result[0].h);
            //g_char.position.x =g_char.position.x + pow(-1,power_x)*(5+saved_result[0].w);
            //SDL_Log("px: %d po: %d w: %f",power_x, pow(-1,power_x), saved_result[0].h);


            // vector method
            // vector from center of char to center of colision
            int char_middle_x = g_char.position.x + g_char.position.w/2;
            int char_middle_y = g_char.position.y + g_char.position.h/2; 
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
                
                SDL_Log("%d", v_x );
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


        // remove tile on "presure collision" !!!!consider the order of these if checks might be important. Do I put in functions?
        if (now - collision_start >= 500 & collision)
        {
            SDL_FRect temp = {.x = 0, .y = 0, .w =0, .h = 0}; //hide in upper left corner
            g_game.tile_position_array[tile_col_index] = temp;
            collision = false;
        }


        //TODO: create full tile set with ores randomly spread out.
        //render_tiles();


        SDL_RenderClear(g_game.renderer);



        // Draw something here...
        SDL_RenderTexture(g_game.renderer, g_game.background,NULL, NULL);
        
        //SDL_RenderTexture(g_game.renderer, g_game.basic_tile,NULL, &g_game.tile_position);

        for(int i = 0; i< AMOUNTOFTILESY*AMOUNTOFTILESX; i++)
        {
            SDL_RenderTexture(g_game.renderer, g_game.tile_texture_array[i], NULL, &g_game.tile_position_array[i]);
        }

        SDL_RenderTexture(g_game.renderer, g_char.texture,NULL, &g_char.position);


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
    g_game.gold_tile = loadTexture("assets/goldt.png", g_game.renderer);
    g_game.redonium_tile = loadTexture("assets/Redonium.png", g_game.renderer);

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
    int randomnumber;
    randomnumber = rand() % 2;
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

bool tile_colision(SDL_FRect *rect_1, SDL_FRect *rect_2)
{
    SDL_FRect result;
    return SDL_GetRectIntersectionFloat(rect_1, rect_2, &result);
}