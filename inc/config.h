#ifndef CONFIG_H
#define CONFIG_H

/* Shared gameplay constants for Gens Invaders.
 * Tunables that change by difficulty live in difficultyTable, not here. */

/* Enemy formation: two rows of 7, moving as one group (or opposite on
 * NORMAL/HARD). Each 16x16 ship uses 4 hardware sprites (2 per scanline).
 * Rows are spaced so they never share a scanline (20-sprite/line limit). */
#define ENEMY_ROWS 2
#define ENEMIES_PER_ROW 7
#define MAX_ENEMIES (ENEMY_ROWS * ENEMIES_PER_ROW)
#define ENEMY_COL_SPACING 32
#define ENEMY_ROW_SPACING 24    /* 16 px sprite + 8 px gap; rows do not overlap */
#define ENEMY_START_Y 32

#define LEFT_EDGE 0
#define RIGHT_EDGE 320
#define BOTTOM_EDGE 224

#define ANIM_STRAIGHT 0
#define ANIM_MOVE 1

#define MAX_BULLETS 6           /* player bullet pool (also the RapidFire cap) */
#define MAX_PLAYER_BULLETS 3
#define MAX_ENEMY_BULLETS 4     /* dedicated enemy bullet pool (HARD can use all 4) */

#define SHOT_INTERVAL 120       /* NORMAL default; the table's shotInterval is what the game uses */

#define INTRO_MUSIC 66
#define SFX_LASER 64
#define SFX_XPLOSION 65
#define SFX_SHIELD_ON 67
#define SFX_SHIELD_READY 68
#define SFX_ENEMY_EXP 69

#define POWERUP_DURATION 100
#define POWER_ON 1
#define POWER_OFF 0

/* Shield (button C). Duration and recharge (wall-clock seconds) live in
 * the difficulty table. powerups converts them to frames with
 * IS_PAL_SYSTEM (x60 NTSC, x50 PAL) so the real time is the same on both. */
#define SHIELD_SPRITE_SIZE   32
#define SHIELD_OFFSET        8   /* (32-16)/2, centers the bubble on the ship */

#define PLAYER_START_X 152
#define PLAYER_START_Y 192

#define ENEMY_STEP_DOWN 8       /* NORMAL step; HARD uses twice this, EASY uses 0 */
#define MAX_LIVES 3
#define RESPAWN_DELAY 90        /* frames the player stays gone after being hit (~1.5 s) */
#define INVULN_FRAMES 120       /* frames of blinking invulnerability after respawn (~2 s) */
#define EXPLOSION_DURATION 48   /* frames the death explosion plays (~0.8 s at 60 Hz) */
#define EXPLOSION_ANIM_FRAMES 6
#define EXPLOSION_SHAKE 2       /* horizontal pixels of starfield shake at the start */
#define MAX_ENEMY_EXPLOSIONS 6  /* simultaneous enemy bursts; extra spawns reuse the oldest */
#define EXPLOSION_ENEMY_DURATION 24  /* ~0.4 s at 60 Hz, faster than the player burst */
#define EXPLOSION_ENEMY_ANIM_FRAMES 6

#define BG_TILES_W 40           /* visible width of the plane in tiles (320 px / 8) */
#define BG_TILES_H 32           /* plane height in tiles (256 px, matches the scroll wrap) */

#endif
