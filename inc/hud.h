#ifndef HUD_H
#define HUD_H

#include <genesis.h>

#define SHIELD_HUD_READY     0
#define SHIELD_HUD_ACTIVE    1
#define SHIELD_HUD_RECHARGE  2
#define SHIELD_BAR_STEPS     64  /* 8 tiles x 8 pixels */

void hud_init(void);
void hud_updateScore(int score, u16 enemiesLeft, u16 lives);
/* level is 0..SHIELD_BAR_STEPS (0 = empty, 64 = full). blink != 0 flashes the
 * ready bar (used for a short pulse when the recharge finishes). */
void hud_updateShieldBar(u16 state, u16 level, u16 blink);
void hud_clear(void);
void hud_drawCentered(const char* text, u16 y);
void hud_clearOverlay(void);
/* Solid "+1 NAVE" (or "+n NAVE") on plane A, row 2, for ~1 s. The newly
 * awarded ship icon(s) blink. Tick only while PLAYING. */
void hud_showExtraLife(u16 granted);
void hud_updateExtraLife(void);
void hud_hideExtraLife(void);

#endif
