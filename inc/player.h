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
/* Always ends the match after the explosion (invasion / landing). */
void player_hitFatal(int score);

GameEntity* player_get(void);
u16 player_lives(void);
u16 player_invulnTimer(void);
u16 player_isExploding(void);
/* After the score increases: grant a life for every EXTRA_LIFE_SCORE crossed,
 * up to LIVES_CAP. Thresholds still advance when the cap is hit. Returns how
 * many lives were actually added. No grant during a fatal (invasion) death. */
u16 player_checkExtraLife(int score);
/* 8x8 exhaust flame under the ship (wave-transition boost). */
void player_setWarpFlame(u16 on);


#endif
