#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <SDL3_ttf/SDL_ttf.h>

# define AMOUNTOFTILESX 25
# define AMOUNTOFTILESY 10
///Defining structs
typedef struct Game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    
    SDL_Texture *basic_tile;
    SDL_Texture *gold_tile;
    SDL_Texture *redonium_tile;
    
    SDL_Texture *healthbar_empty;
    SDL_Texture *healthbar_filled;

    SDL_Texture *store;
    SDL_FRect store_position;

    SDL_Texture *upgrade_store;
    SDL_FRect upgrade_store_position;
    
    SDL_Texture *gas_empty;
    SDL_Texture *gas_filled;
    
    SDL_FRect tile_position;
    //this array needs to be a linked list, maybe not needed since I dont need to remove it just set it to 0.
    SDL_FRect tile_position_array[AMOUNTOFTILESY*AMOUNTOFTILESX];
    SDL_Texture *tile_texture_array[AMOUNTOFTILESY*AMOUNTOFTILESX];


    TTF_Font* font;
}Game;


typedef struct Weapon
{
    int damage;
    int max_ammo;
    int ammo;

}Weapon;

typedef struct Character
{
    SDL_FRect position;
    SDL_Texture* texture;

    int max_health;
    int health;

    int armor;

    int speed;

    int max_gas;
    int gas;

    int bomb_capacity;
    int amount_bombs;

    int money;

    // from lower to upper:
    // coal, iron, copper, gold, emerald, diamond, mythril, titanium
    int bag_capacity;
    int ores[8];

    Weapon gun;

}Char;

typedef struct Enemies
{
    SDL_Rect position[5];
    SDL_Texture* texture[5];
    // This needs to be able to consider different types of enemies, was thinking union but will look at later.
}Enemies;

//declaring global variables
Game g_game;
Char g_char;




void initialize_game();
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);


void render_tiles();


void fill_tiles_array();

bool tile_colision(SDL_FRect *rect_1, SDL_FRect *rect_2);


void load_in_assets();
void define_inital_variables();

void rendering_screen();
void movement_and_collision();
