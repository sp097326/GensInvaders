#ifndef ENEMIES_H
#define ENEMIES_H

#include "game.h"

void enemies_init(void);
void enemies_reset(void);
/* Like reset, then shift the formation `offset` pixels up so it can slide in. */
void enemies_beginEnter(u16 offset);
/* remain goes from total down to 0; at 0 every ship is in its home slot. */
void enemies_advanceEnter(u16 remain, u16 total, u16 offset);
void enemies_hideAll(void);
u16  enemies_update(void);
void enemies_setRowPalettes(void);

GameEntity* enemies_at(u16 i);
u16 enemies_left(void);
void enemies_decLeft(void);

#endif
