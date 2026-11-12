#ifndef HUD_H
#define HUD_H

#include <genesis.h>

void hud_updateScore(int score, u16 enemiesLeft, u16 lives);
void hud_clear(void);
void hud_drawCentered(const char* text, u16 y);
void hud_clearOverlay(void);

#endif
