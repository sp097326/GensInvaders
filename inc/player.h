#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

void player_init(void);
void player_reset(void);
void player_hide(void);
/* Returns 1 when a death explosion has finished and there are no lives left
 * (caller should show GAME OVER). */
u16  player_update(void);
void player_stop(void);
/* Starts the explosion; GAME OVER is deferred until player_update says so. */
u16  player_hit(int score);

GameEntity* player_get(void);
u16 player_lives(void);
u16 player_invulnTimer(void);
u16 player_isExploding(void);

#endif
