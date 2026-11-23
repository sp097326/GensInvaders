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
/* Column bitmask (bit c = column c) of the ships row `row` has on `wave`.
 * 0 = row unused. Waves 11-19: rows 0-1 full + staggered row 2;
 * wave 21+: all three rows full. */
u16  enemies_rowMaskForWave(u16 wave, u16 row);

GameEntity* enemies_at(u16 i);
u16 enemies_left(void);
void enemies_decLeft(void);

#endif
