#ifndef HUD_H
#define HUD_H

#include <genesis.h>

#define SHIELD_HUD_READY     0
#define SHIELD_HUD_ACTIVE    1
#define SHIELD_HUD_RECHARGE  2
#define SHIELD_BAR_STEPS     64  /* 8 tiles x 8 pixels */

void hud_init(void);
/* Record score / LEFT / lives / wave / difficulty. The VDP write happens
 * once per frame in hud_flush, and only for the fields that changed. */
void hud_updateScore(int score, u16 enemiesLeft, u16 lives);
/* Apply pending HUD changes. Call once per frame from the main loop,
 * after game_state_update and before SPR_update. */
void hud_flush(void);
/* The tilemap was wiped (title screen). Drop cached tiles so the next
 * in-game flush redraws from scratch; do not paint over the title. */
void hud_invalidate(void);
/* level is 0..SHIELD_BAR_STEPS (0 = empty, 64 = full). blink != 0 flashes the
 * ready bar (used for a short pulse when the recharge finishes). */
void hud_updateShieldBar(u16 state, u16 level, u16 blink);
/* RapidFire timer on the bottom-right of row 27: "RF" plus an 8-tile
 * red bar. Hidden (active == 0) clears that corner. level is 0..SHIELD_BAR_STEPS. */
void hud_updateRapidBar(u16 active, u16 level);
void hud_hideRapidBar(void);
/* Boost (button A) readiness on row 27, cols 18-28: "IMP" plus an 8-tile
 * bar, same colours as the shield bar (state: SHIELD_HUD_READY green,
 * SHIELD_HUD_ACTIVE blue while dashing, SHIELD_HUD_RECHARGE red filling). */
void hud_updateBoostBar(u16 state, u16 level, u16 blink);
/* Mothership HP on row 2: "JEFE" plus a 16-tile bar. Hidden when active==0. */
void hud_updateBossBar(u16 active, u16 hp, u16 hpMax);
void hud_hideBossBar(void);
void hud_clear(void);
void hud_drawCentered(const char* text, u16 y);
void hud_clearOverlay(void);
/* Solid "+1 NAVE" (or "+n NAVE") on plane A, bottom row, next to the
 * life icons, for ~1 s. The newly awarded ship icon(s) blink. Tick only
 * while PLAYING. */
void hud_showExtraLife(u16 granted);
void hud_updateExtraLife(void);
void hud_hideExtraLife(void);
/* "+N" points in the same slot for the same ~1 s (no icon blink). */
void hud_showBonus(u16 points);

#endif
