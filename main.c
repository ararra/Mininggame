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
        SDL_FRect result;
        SDL_FRect saved_result[3];
        int k = 0;
        // TODO: Generalize downward collision into all directions
        for(int i = 0; i < AMOUNTOFTILESX*AMOUNTOFTILESY; i++)
        {
            if (SDL_GetRectIntersectionFloat(&g_char.position, &g_game.tile_position_array[i], &result) & !collision)
            {
                saved_result[k] =  result;
                k += 1;
                SDL_Log("coll ");
                collision = true;
                collision_start = now;
                tile_col_index = i;
                
            }
        }
        
        // TODO: add slowdown to movement when letting go of button
        if (keys[SDL_SCANCODE_W] & !collision){ g_char.position.y -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_S] & !collision){ g_char.position.y += delta*g_char.speed; }
        if (keys[SDL_SCANCODE_A] & !collision){ g_char.position.x -= delta*g_char.speed; }
        if (keys[SDL_SCANCODE_D] & !collision){ g_char.position.x += delta*g_char.speed; }

        // issues with current collisions:
        // 1) teleports drill to the top when side collision
        // 2) 

        //pushes char back if collision and the button has been let go.
        //TODO: Make bounceback feature initial velocity based on "pressure" (time under button press) with exponential speed loss. 
        //Needs to be done between frames right so a function with static variable right?
        //this is why the side collision jump is happening
        //dont know if i should implement drilling upwards or leave it as it is
        // if (collision & (!keys[SDL_SCANCODE_S] | (!keys[SDL_SCANCODE_S] & (!keys[SDL_SCANCODE_A] | !keys[SDL_SCANCODE_D]) ))  )
        // {   
        //     //the constant needs to be there otherwise the colision persists for a few frames and it activates the tile collision if check above several times.
        //     // use the result rect from the collision to create a direction for the bounce back.
        //     g_char.position.y -= 5+delta*g_char.speed;
        //     collision = false;
        // }         
        if (collision && (!(keys[SDL_SCANCODE_S] || (keys[SDL_SCANCODE_A] ^ keys[SDL_SCANCODE_D])))  )
        {   
            //the constant needs to be there otherwise the colision persists for a few frames and it activates the tile collision if check above several times.
            // use the saved_result rect from the collision to create a direction for the bounce back.

            // this method not quite working
            float saved_result_x_sum = saved_result[0].x +saved_result[1].x +saved_result[2].x;
            float saved_result_y_sum = saved_result[0].y +saved_result[1].y +saved_result[2].y;
            int power_y = (saved_result_x_sum > g_char.position.y);
            int power_x = (saved_result_y_sum > g_char.position.x); 
            g_char.position.y =g_char.position.y + pow(-1,power_y)*(5+saved_result[0].h);
            g_char.position.x =g_char.position.x + pow(-1,power_x)*(5+saved_result[0].w);
            SDL_Log("px: %d po: %d w: %f",power_x, pow(-1,power_x), saved_result[0].h);



            //ChatGPT Method Also not working
            // int dy = saved_result.y - g_char.position.y;
            // int dx = saved_result.x - g_char.position.x;
            // if (dy != 0) {
            //     g_char.position.y += (dy > 0 ? 1 : -1) * (5 + saved_result.h);
            // }
            // if (dx != 0) {
            //     g_char.position.x += (dx > 0 ? 1 : -1) * (5 + saved_result.w);
            // }
            // SDL_Log("dx: %d, dy: %d, result.h: %f, result.w: %f", dx, dy, saved_result.h, saved_result.w);

            // Vector method



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
    int randomnumber;
    randomnumber = rand() % 2;
    for(int i = 0; i< AMOUNTOFTILESX; i++)
    {

        for(int j = 0; j < AMOUNTOFTILESY; j++)
        {
            //modulo x makes it remove around 1 in x random tiles.
            bool fill_tile_true = rand() % 5;
            if(fill_tile_true)
            {
                int k = i+j*AMOUNTOFTILESX;
                g_game.tile_position_array[k].y = 640-j*32-32;
                g_game.tile_position_array[k].x = i*32;
                g_game.tile_position_array[k].h = 32;
                g_game.tile_position_array[k].w = 32;
            }
            else
            {
                //print ore tile
            }
        }
    } 
}

bool tile_colision(SDL_FRect *rect_1, SDL_FRect *rect_2)
{
    SDL_FRect result;
    return SDL_GetRectIntersectionFloat(rect_1, rect_2, &result);
}