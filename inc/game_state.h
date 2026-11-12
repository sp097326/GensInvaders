#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "game.h"

void game_state_showTitle(void);
void game_state_update(void);
void game_state_joy(u16 joy, u16 changed, u16 state);

#endif
