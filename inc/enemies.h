#ifndef ENEMIES_H
#define ENEMIES_H

#include "game.h"

void enemies_init(void);
void enemies_reset(void);
void enemies_hideAll(void);
u16  enemies_update(void);
void enemies_setRowPalettes(void);

GameEntity* enemies_at(u16 i);
u16 enemies_left(void);
void enemies_decLeft(void);

#endif
