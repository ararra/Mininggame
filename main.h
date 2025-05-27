#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


///Defining structs
typedef struct Game
{
    SDL_Window *window;
    SDL_Renderer *renderer;


}Game;


typedef struct Weapon
{
    int damage;
    int max_ammo;
    int ammo;

}Weapon;

typedef struct Character
{
    SDL_Rect position;
    SDL_Texture* texture;

    int max_health;
    int health;
    
    int armor;

    int speed;

    int max_gas;
    int gas;

    int bomb_capacity;
    int amount_bombs;

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
