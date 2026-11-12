#ifndef COLLISION_H
#define COLLISION_H

#include "game.h"

#define COLLISION_NONE      0
#define COLLISION_WIN       1
#define COLLISION_GAME_OVER 2

int collideEntities(GameEntity* a, GameEntity* b);
u16 collision_update(int* score);

#endif
