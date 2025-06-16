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
        
        SDL_FRect collision_result_array[4];
        int k = 0;
        handle_collision_detection(&collision, &collision_start, now, tile_col_index, collision_result_array, &k);

        handle_store_interaction();

        handle_movement(keys, collision, delta);

        if(g_char.gas == 0 || g_char.health == 0) {
            handle_death_reset();
        }

        handle_collision_response(keys, &collision, tile_col_index, collision_result_array, k, collision_start, now);

        //Render only 60 fps
        SDL_Delay(16.67);
        rendering_screen();
    }
    
    shutdown();

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
    if (TTF_Init() < 0) {
        printf("TTF_Init error: %s\n", SDL_GetError());
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

        
        // SDL_RenderTexture(g_game.renderer, g_game.basic_tile,NULL, &g_game.tile_position);
        for(int i = 0; i< AMOUNTOFTILESY*AMOUNTOFTILESX; i++)
        {
            SDL_RenderTexture(g_game.renderer, g_game.tile_texture_array[i], NULL, &g_game.tile_position_array[i]);
        }

        float health_percentage = ((float)g_char.health / g_char.max_health) * 100.0f;

        SDL_FRect healthbar_pos = {.x = 20, .y = 20 ,.w = 100, .h = 20};
        SDL_FRect healthbar_left = {.x = 0, .y = 0 ,.w = health_percentage, .h = 20};
        SDL_FRect healthbar_left_pos = {.x = 20, .y = 20 ,.w = health_percentage, .h = 20};

        SDL_RenderTexture(g_game.renderer, g_game.healthbar_empty, NULL, &healthbar_pos);
        SDL_RenderTexture(g_game.renderer, g_game.healthbar_filled, &healthbar_left, &healthbar_left_pos);

        float gas_percentage = ((float)g_char.gas / g_char.max_gas) * 100.0f;

        SDL_FRect gasbar_pos = {.x = 20, .y = 60 ,.w = 100, .h = 20};
        SDL_FRect gasbar_left = {.x = 0, .y = 0 ,.w = gas_percentage, .h = 20};
        SDL_FRect gasbar_left_pos = {.x = 20, .y = 60 ,.w = gas_percentage, .h = 20};

        SDL_RenderTexture(g_game.renderer, g_game.gas_empty, NULL, &gasbar_pos);
        SDL_RenderTexture(g_game.renderer, g_game.gas_filled, &gasbar_left, &gasbar_left_pos);

        
        SDL_RenderTexture(g_game.renderer, g_game.store, NULL, &g_game.store_position);
        SDL_RenderTexture(g_game.renderer, g_game.upgrade_store, NULL, &g_game.upgrade_store_position);

        SDL_Color white_color = {255, 255, 255, 255}; // White

        char money_string[3] = "0";
        SDL_itoa(g_char.money ,money_string, 10);

        SDL_Surface* surface = TTF_RenderText_Blended(g_game.font, money_string, sizeof(money_string), white_color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(g_game.renderer, surface);

        SDL_FRect destRect = {20, 100, surface->w, surface->h};
        SDL_DestroySurface(surface);
        SDL_RenderTexture(g_game.renderer, texture, NULL, &destRect);
        
        SDL_Surface* surface_mon = TTF_RenderText_Blended(g_game.font, "money", sizeof("money"), white_color);
        SDL_Texture* texture_mon = SDL_CreateTextureFromSurface(g_game.renderer, surface_mon);

        SDL_FRect dest_rect_mon = {50, 100, surface->w, surface->h};
        SDL_DestroySurface(surface_mon);
        SDL_RenderTexture(g_game.renderer, texture_mon, NULL, &dest_rect_mon);


        for (int i = 0; i < sizeof(g_char.ores)/sizeof(g_char.ores[0]); i++)
        {
            char string[3] = "0";
            SDL_itoa(g_char.ores[i], string, 10);
            SDL_Surface* surface = TTF_RenderText_Blended(g_game.font, string, sizeof(string), white_color);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(g_game.renderer, surface);
            
            SDL_FRect dest_rect = {150+i*30, 15, surface->w, surface->h};
            SDL_DestroySurface(surface);
            SDL_RenderTexture(g_game.renderer, texture, NULL, &dest_rect);
        }
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
            SDL_Log("ss %d", fill_tile_basic);
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
    g_game.gas_empty = loadTexture("assets/Healthbar.png", g_game.renderer);
    g_game.gas_filled = loadTexture("assets/Gasbar.png", g_game.renderer);
    g_game.upgrade_store = loadTexture("assets/Upgrade House.png", g_game.renderer);

    g_game.font = TTF_OpenFont("assets/Roboto/Roboto-VariableFont_wdth,wght.ttf", 24);
    if (!g_game.font) {
        printf("Failed to load font: %s\n", SDL_GetError());
        return exit(EXIT_FAILURE);
    }

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
    g_char.max_gas = 1000;
    g_char.gas = 1000;
    g_char.money = 0;

    g_char.bag_capacity = 5;
    for(int i = 0 ; i < sizeof(g_char.ores)/sizeof(g_char.ores[0]); i++)
    {
        g_char.ores[i] = 0;
    }

    SDL_FRect temp3 = {.x = 100, .y = 256 ,.w = 64, .h = 64};
    g_game.store_position = temp3;

    SDL_FRect temp4 = {.x = 500, .y = 256 ,.w = 64, .h = 64};
    g_game.upgrade_store_position = temp4;

}

void handle_collision_detection(bool *collision, Uint64 *collision_start, Uint64 now, int tile_col_index[4], SDL_FRect collision_result_array[4], int *k) {
    SDL_FRect temp_collision_result;
    *k = 0;
    
    for(int i = 0; i < AMOUNTOFTILESX*AMOUNTOFTILESY; i++) {
        if (SDL_GetRectIntersectionFloat(&g_char.position, &g_game.tile_position_array[i], &temp_collision_result)) {
            collision_result_array[*k] = temp_collision_result;
            
            if(!*collision) {
                *collision_start = now;
            }
            
            *collision = true;
            tile_col_index[*k] = i;
            *k += 1;
        }
    }
}

void handle_store_interaction() {
    SDL_FRect temp_collision_result;
    
    if(SDL_GetRectIntersectionFloat(&g_char.position, &g_game.store_position, &temp_collision_result)) {
        int money_sum = 0;
        for(int i = 0; i < 8; i++) {
            money_sum += (i+1)*g_char.ores[i];
            g_char.ores[i] = 0;
        }

        g_char.money += money_sum;
        g_char.gas = g_char.max_gas;
        g_char.health = g_char.max_health;
    }
        // TODO: Add upgrade store interaction
    if(SDL_GetRectIntersectionFloat(&g_char.position, &g_game.upgrade_store_position, &temp_collision_result)) {

        // Get average center


        int char_middle_x = g_char.position.x + g_char.position.w/2;
        int char_middle_y = g_char.position.y + g_char.position.h/2; 

        int upg_store_middle_x = g_game.upgrade_store_position.x + g_game.upgrade_store_position.w/2;
        int upg_store_middle_y = g_game.upgrade_store_position.y + g_game.upgrade_store_position.h/2;

        int v_x = char_middle_x - upg_store_middle_x;
        int v_y = char_middle_y - upg_store_middle_y;
        
        // Apply push-back (with safety checks)
        if (v_x != 0) {
            g_char.position.x += (v_x > 0) ? 2 : -2;
        }
        if (v_y != 0) {
            g_char.position.y += (v_y > 0) ? 2 : -2;
        }
    
    }
}

void handle_movement(const bool *keys, bool collision, float delta) {
    if(keys[SDL_SCANCODE_W] && !collision && g_char.gas > 0) { 
        g_char.position.y -= delta*g_char.speed; 
    }
    if(keys[SDL_SCANCODE_S] && !collision && g_char.gas > 0) {
        g_char.position.y += delta*g_char.speed; 
    }
    if(keys[SDL_SCANCODE_A] && !collision && g_char.gas > 0) {
        g_char.position.x -= delta*g_char.speed;
    }
    if(keys[SDL_SCANCODE_D] && !collision && g_char.gas > 0) { 
        g_char.position.x += delta*g_char.speed; 
    }
    if(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_W]) {
        g_char.gas -= 1;
    }
}

void handle_death_reset() {
    SDL_FRect temp2 = {.w = 32, .h = 32, .x = 392, .y = 200};
    g_char.position = temp2;
    g_char.gas = g_char.max_gas;
    g_char.health = g_char.max_health;
    
    for(int i = 0; i < 8; i++) {
        g_char.ores[i] = 0;
    }
    
    if (g_char.money <= 10 && g_char.money > 1) {
        g_char.money -= 2;
    } else if(g_char.money > 10){
        g_char.money -= 4;
    }
    
    SDL_Log("You died, all ores gone.");
}

void handle_collision_response(const bool *keys, bool *collision, int tile_col_index[4], SDL_FRect collision_result_array[4], int k, Uint64 collision_start, Uint64 now) {
    int char_middle_x = g_char.position.x + g_char.position.w/2;
    int char_middle_y = g_char.position.y + g_char.position.h/2; 
    
    // Handle bounceback when not pressing movement keys
    if(*collision && (!(keys[SDL_SCANCODE_S] || (keys[SDL_SCANCODE_A] ^ keys[SDL_SCANCODE_D])))) {  
        int coll_middle_x = 0;
        int coll_middle_y = 0;

        // Calculate center of all collisions
        for(int i = 0; i < k; i++) {
            coll_middle_x += collision_result_array[i].x + collision_result_array[i].w/2;
            coll_middle_y += collision_result_array[i].y + collision_result_array[i].h/2;
        }

        // Get average center
        if(k > 0) {
            coll_middle_x /= k;
            coll_middle_y /= k;
            
            // Calculate push-back vector
            int v_x = char_middle_x - coll_middle_x;
            int v_y = char_middle_y - coll_middle_y;
            
            // Apply push-back (with safety checks)
            if (v_x != 0) {
                g_char.position.x += (v_x > 0) ? 2 : -2;
            }
            if (v_y != 0) {
                g_char.position.y += (v_y > 0) ? 2 : -2;
            }
        }
        
        *collision = false;
    } 
    
    // Handle tile removal on "pressure collision"
    if(now - collision_start >= 500 && *collision) {   
        SDL_FRect temp = {.x = -100, .y = -100, .w = 0, .h = 0}; //hide offscreen
        g_game.tile_position_array[tile_col_index[0]] = temp;

        int total_ores = 0;
        for(int i = 0; i < 8; i++) {
            total_ores += g_char.ores[i];
        }
        
        if(total_ores < g_char.bag_capacity) {
            if(g_game.tile_texture_array[tile_col_index[0]] == g_game.redonium_tile) {
                g_char.ores[1] += 1;
                SDL_Log("Redtile +1");
            }
            if(g_game.tile_texture_array[tile_col_index[0]] == g_game.gold_tile) {
                g_char.ores[0] += 1;
                SDL_Log("gold tile +1");
            }
        } else if(g_game.tile_texture_array[tile_col_index[0]] != g_game.basic_tile) {
            SDL_Log("Bag is full, Item deleted!");
        }
        
        *collision = false;
    }
}

void shutdown()
{
    SDL_DestroyTexture(g_game.background);
    SDL_DestroyTexture(g_game.basic_tile);
    SDL_DestroyTexture(g_game.gold_tile);
    SDL_DestroyTexture(g_game.redonium_tile);
    SDL_DestroyTexture(g_game.healthbar_empty);
    SDL_DestroyTexture(g_game.healthbar_filled);
    SDL_DestroyTexture(g_game.store);
    SDL_DestroyTexture(g_game.upgrade_store);
    SDL_DestroyTexture(g_game.gas_empty);
    SDL_DestroyTexture(g_game.gas_filled);
    TTF_CloseFont(g_game.font);
    
    SDL_DestroyTexture(g_char.texture);




    SDL_DestroyRenderer(g_game.renderer);
    SDL_DestroyWindow(g_game.window);
    SDL_Quit();
}