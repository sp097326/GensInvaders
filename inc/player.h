#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

void player_init(void);
void player_reset(void);
void player_hide(void);
void player_update(void);
void player_stop(void);
u16  player_hit(int score);

GameEntity* player_get(void);
u16 player_lives(void);
u16 player_invulnTimer(void);

#endif
