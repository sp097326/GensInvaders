#include "hud.h"
#include "difficulty.h"

static char hud_string[40] = "";

void hud_updateScore(int score, u16 enemiesLeft, u16 lives) {
    sprintf(hud_string, "SCORE: %d - LEFT: %d - SHIPS: %d", score, enemiesLeft, lives);
    VDP_clearText(0,0,40);
    VDP_drawText(hud_string, 0,0);
    VDP_clearText(0,1,40);
    sprintf(hud_string, "DIFF: %s", difficulty_name());
    VDP_drawText(hud_string, 0,1);
}

void hud_clear(void) {
    VDP_clearText(0, 0, 40);
    VDP_clearText(0, 1, 40);
}

void hud_drawCentered(const char* text, u16 y) {
    u16 len = strlen(text);
    VDP_drawText(text, (40 - len) / 2, y);
}

void hud_clearOverlay(void) {
    u16 row;
    for (row = 10; row <= 18; row++) {
        VDP_clearText(0, row, 40);
    }
}
