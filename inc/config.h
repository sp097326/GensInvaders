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

/* Charged shot (hold B ~1 s, release). Tap-release still fires a normal 8x8. */
#define CHARGE_FRAMES_NTSC 60
#define CHARGE_FRAMES_PAL  50
#define CHARGED_BULLET_SIZE 24  /* 3x the 8x8 shot */
#define CHARGED_BULLET_SPEED -4
#define MAX_ENEMY_BULLETS 4     /* dedicated enemy bullet pool (HARD can use all 4) */

#define SHOT_INTERVAL 120       /* NORMAL default; the table's shotInterval is what the game uses */

#define INTRO_MUSIC 66
#define SFX_LASER 64
#define SFX_XPLOSION 65
#define SFX_SHIELD_ON 67
#define SFX_SHIELD_READY 68
#define SFX_SHIELD_HUM 71    /* 2 s loop; plays on CH1 for the shield duration */
#define SFX_ENEMY_EXP 69
#define SFX_1UP 70
#define SFX_CHARGE_READY 72  /* short lock-on ding, CH4 (below 1UP priority) */
#define SFX_CHARGE_SHOT 73   /* heavier shot, CH3 like the laser */
#define SFX_RAPID_PICK 74    /* capsule pickup, CH4 (below 1UP, never CH1) */
#define SFX_BOSS_HIT 75      /* short core tick, CH4, rate-limited */
#define SFX_BOSS_DIE 76      /* heavier boom on death, CH2 */

/* Mothership boss: first appears on wave 10, then every 10 waves.
 * HP starts at 500 and doubles each return, capped so the bar and u16 stay safe. */
#define BOSS_WAVE_EVERY      10
#define BOSS_HP_BASE        500
#define BOSS_HP_CAP        8000  /* wave 50+: 500, 1000, 2000, 4000, 8000 */
#define BOSS_CHARGED_MULT     3  /* charged bolt deals 3x a normal hit */
#define BOSS_W               64
#define BOSS_H               24
#define BOSS_CORE_W          24  /* ~1/3 of the 64 px hull, forgiving center */
#define BOSS_CORE_H          24  /* full body height of the core */
#define BOSS_CORE_OX         20  /* (64-24)/2 */
#define BOSS_CORE_OY          0
#define BOSS_Y_MIN           28  /* just below the JEFE bar (row 2) */
#define BOSS_Y_MAX           (112 - BOSS_H)  /* 88: hull stays in the top half */
#define BOSS_HOME_Y          ((BOSS_Y_MIN + BOSS_Y_MAX) / 2)  /* 58, roam center */
#define BOSS_BAR_STEPS      128  /* 16 tiles x 8 px */

#define POWER_ON 1
#define POWER_OFF 0

/* RapidFire capsule: 10% drop, at most one on screen, pity after 10
 * kills without a drop. Duration (seconds) lives in the difficulty table. */
#define RAPID_DROP_MOD     10  /* 1 in 10 */
#define RAPID_DROP_PITY    10  /* guaranteed drop on the 10th dry kill */
#define CAPSULE_SPEED       1  /* pixels per frame, downward */

/* Shield (button C). Duration and recharge (wall-clock seconds) live in
 * the difficulty table. powerups converts them to frames with
 * IS_PAL_SYSTEM (x60 NTSC, x50 PAL) so the real time is the same on both. */
#define SHIELD_SPRITE_SIZE   32
#define SHIELD_OFFSET        8   /* (32-16)/2, centers the bubble on the ship */

#define PLAYER_START_X 152
#define PLAYER_START_Y 192

#define ENEMY_STEP_DOWN 8       /* NORMAL step; HARD uses twice this, EASY uses 0 */
#define MAX_LIVES 3              /* ships at the start of a match */
#define LIVES_CAP 9              /* up to 9 miniature ships on the HUD; 1UPs stop adding here */
#define EXTRA_LIFE_SCORE 1000    /* a ship is awarded every 1000 points, all difficulties */
#define EXTRA_LIFE_BANNER_NTSC 60  /* ~1 s for the "+1 NAVE" text */
#define EXTRA_LIFE_BANNER_PAL  50
#define RESPAWN_DELAY 90        /* frames the player stays gone after being hit (~1.5 s) */
#define INVULN_FRAMES 120       /* frames of blinking invulnerability after respawn (~2 s) */
#define EXPLOSION_DURATION 48   /* frames the death explosion plays (~0.8 s at 60 Hz) */
#define EXPLOSION_ANIM_FRAMES 6
#define EXPLOSION_SHAKE 2       /* horizontal pixels of starfield shake at the start */
#define MAX_ENEMY_EXPLOSIONS 6  /* simultaneous enemy bursts; extra spawns reuse the oldest */
#define EXPLOSION_ENEMY_DURATION 24  /* ~0.4 s at 60 Hz, faster than the player burst */
#define EXPLOSION_ENEMY_ANIM_FRAMES 6

/* Intermission after a wave is cleared (~2.5 s on both systems):
 * starfield warp, engine flame, then the next formation slides in.
 * The OLEADA N banner stays up for the whole transition. */
#define WAVE_WARP_FRAMES_NTSC 150
#define WAVE_WARP_FRAMES_PAL  125
#define WAVE_WARP_ACCEL_NTSC   60  /* first second: stars and ship ramp up */
#define WAVE_WARP_ACCEL_PAL    50
#define WAVE_WARP_ARRIVE_NTSC  75  /* last 1.25 s: formation slides in */
#define WAVE_WARP_ARRIVE_PAL   62
#define STAR_SPEED_NORMAL       2
#define STAR_SPEED_WARP        14
#define WAVE_WARP_LIFT          8  /* pixels the ship rises during the boost */
#define WAVE_WARP_ENTER        80  /* both rows start fully above the screen */
/* Kept as aliases so older comments still read; the warp timer is what is used. */
#define WAVE_BANNER_FRAMES_NTSC WAVE_WARP_FRAMES_NTSC
#define WAVE_BANNER_FRAMES_PAL  WAVE_WARP_FRAMES_PAL

#define BG_TILES_W 40           /* visible width of the plane in tiles (320 px / 8) */
#define BG_TILES_H 32           /* plane height in tiles (256 px, matches the scroll wrap) */

#endif
