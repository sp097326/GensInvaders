#ifndef EXPLOSIONS_H
#define EXPLOSIONS_H

#include "game.h"

void explosions_init(void);
void explosions_reset(void);
void explosions_hide(void);
void explosions_update(void);
/* Plays a short enemy burst at (x, y). If the pool is full, the oldest is reused.
 * Does not affect collisions or formation movement. */
void explosions_spawn(s16 x, s16 y);

#endif
