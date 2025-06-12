#pragma once
#include "main.h"

typedef struct linkedlist
{
    SDL_FRect tile_position;
    linkedlist* next;
}linkedlist;

void remove_node()