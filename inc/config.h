#ifndef CONFIG_H
#define CONFIG_H

/* Shared gameplay constants for Gens Invaders.
 * Tunables that change by difficulty live in difficultyTable, not here. */

/* Enemy formation: two rows of 7, moving as one group (or opposite on
 * NORMAL/HARD). Each 16x16 ship uses 4 hardware sprites (2 per scanline).
 * Rows are spaced so they never share a scanline (20-sprite/line limit). */
/* ENEMY_ROWS is the most rows any wave can use (all sprites are created
 * at boot). Which rows/columns a wave actually fills comes from
 * enemies_rowMaskForWave() in enemies.c: waves 11-19 add a staggered
 * third row (columns 0, 2, 4, 6); every other wave uses 2 full rows. */
#define ENEMY_ROWS 3
#define ENEMIES_PER_ROW 7
#define MAX_ENEMIES (ENEMY_ROWS * ENEMIES_PER_ROW)
#define ENEMY_ROW_FULL      0x7F  /* columns 0-6 */
#define ENEMY_ROW_STAGGER   0x55  /* columns 0, 2, 4, 6 */
#define STAGGER_ROW_FIRST_WAVE 11
#define STAGGER_ROW_LAST_WAVE  19
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
 * HP starts at 350 (500 until V0.3.69) and doubles each return, capped so
 * the bar and u16 stay safe. */
#define BOSS_WAVE_EVERY      10
#define BOSS_HP_BASE        350
#define BOSS_HP_CAP        8000  /* 350, 700, 1400, 2800, 5600, then 8000 from wave 60 */
#define BOSS_CHARGED_MULT     3  /* charged bolt deals 3x a normal hit */
#define BOSS_W               64
#define BOSS_H               56
#define BOSS_CORE_W          24  /* body between the two rockets */
#define BOSS_CORE_H          40  /* body below the ears, above the feet */
#define BOSS_CORE_OX         20  /* (64-24)/2 */
#define BOSS_CORE_OY          8
#define BOSS_Y_MIN           28  /* just below the JEFE bar (row 2) */
#define BOSS_Y_MAX           (112 - BOSS_H)  /* 56: sprite stays in the top half */
#define BOSS_HOME_Y          ((BOSS_Y_MIN + BOSS_Y_MAX) / 2)  /* 42, roam center */
#define BOSS_BAR_STEPS      128  /* 16 tiles x 8 px */
/* Second boss attack: a 3-bullet fan aimed toward the player.
 * Above 50% HP it is a random pick (1 in BOSS_FAN_CHANCE attacks);
 * at 50% HP or less it alternates with the bone. */
#define BOSS_FAN_CHANCE       4
#define BOSS_FAN_SPREAD       2  /* px/frame sideways between fan bullets */
#define BOSS_FAN_AIM_ZONE    32  /* player offset (px) that tilts the fan */
/* Main boss attack (19/11/2026): one 48x16 bone falls straight down from
 * the belly, centred (x = boss.x + 8), where the old 8x8 volley spread.
 * The hitbox is smaller than the art (40x8, centred) so a graze does not
 * kill. It falls BOSS_BONE_SLOWER (2) px/frame slower than the enemy
 * bullets of the current wave (min 1): 3-5 px/frame for bullets at 5-7
 * (Alberto's tuning after playtesting, 19/11/2026).
 * At most 2 on screen: with one, a fast boss timer would skip attacks
 * while the previous bone is still falling. The shield does NOT stop it
 * (Alberto's call): only dodging works. Player shots ignore it. */
#define BOSS_BONE_W          48
#define BOSS_BONE_H          16
#define BOSS_BONE_HIT_OX      4  /* (48-40)/2 */
#define BOSS_BONE_HIT_OY      4  /* (16-8)/2 */
#define BOSS_BONE_HIT_W      40
#define BOSS_BONE_HIT_H       8
#define BOSS_BONE_SLOWER      2  /* px/frame slower than bulletSpeed (was 1 until V0.3.68) */
#define MAX_BOSS_BONES        2
/* Bone colours live in PAL1 slots 7-9: space.bmp only uses 0-6, nothing
 * else draws with 7-15, and PAL1 is never tinted by the boss hit flash
 * (that only touches PAL2 colours 1-3). imgbone.png is indexed 7-9. */
#define BOSS_BONE_PAL_FIRST   7
#define BOSS_BONE_PAL_COUNT   3

/* Second boss (23/11/2026): the turtle wizard, only on wave 20 (the 2nd
 * boss appearance). Waves 10, 30, 40... keep La Cookie unchanged. Same HP,
 * points, damage, speed and fire timer as any 2nd appearance (700 HP).
 * res/tiles/imgboss2.png: 64x64, 1 frame, own 16-colour palette in PAL2
 * (indices 1-3 = shell and limb greens, tinted red by the hit flash).
 * If a 2nd frame is added (8 8 tiles per frame), boss.c animates it. */
/* V0.3.73 (Alberto): the order is swapped. Odd appearances (waves 10, 30,
 * 50...) are the turtle, even ones (20, 40, 60...) La Cookie. HP still
 * comes from the appearance number: turtle 350 on wave 10, Cookie 700 on
 * wave 20, turtle 1400 on wave 30... (the notes above that say "wave 20"
 * describe V0.3.71-72). */
#define BOSS2_ON_ODD          1   /* 1 = turtle on odd appearances */
#define BOSS2_W              64
#define BOSS2_H              64
#define BOSS2_CORE_OX        20  /* shell centre: x 20-43 */
#define BOSS2_CORE_OY         8  /* y 8-43, inside the shell */
#define BOSS2_CORE_W         24
#define BOSS2_CORE_H         36
#define BOSS2_Y_MAX          (112 - BOSS2_H)  /* 48: unused by the patrol, kept for the kind table */
#define BOSS2_HOME_Y         32  /* fixed patrol height (just below the JEFE bar) */
#define BOSS2_FAN_OX         28  /* fan from the head (bullet centred at x 32) */
#define BOSS2_FAN_OY         48
#define BOSS2_STAR_OX        50  /* star centred on the wand tip (58, 28) */
#define BOSS2_STAR_OY        20
/* Turtle movement (V0.3.72, Alberto): no Lissajous and no bob. It patrols
 * left-right at a constant bossSpeed px/frame (1/2/3) at BOSS2_HOME_Y and,
 * at a random interval, dives straight down from its current column across
 * the screen, leaves through the bottom, re-enters from the top and slides
 * back down to its home height. Body contact costs a life (player_hit); the
 * shield does NOT stop it (like the bone and the star) unless
 * BOSS2_DIVE_SHIELD_BLOCKS is 1. No stars or fan while warning/diving. */
/* V0.3.74: intervals ~20% shorter (were 240-420 / 180-360 / 150-270). */
#define BOSS2_DIVE_MIN_EASY     192  /* frames between dives: 3.2-5.6 s */
#define BOSS2_DIVE_MAX_EASY     336
#define BOSS2_DIVE_MIN_NORMAL   144  /* 2.4-4.8 s */
#define BOSS2_DIVE_MAX_NORMAL   288
#define BOSS2_DIVE_MIN_HARD     120  /* 2-3.6 s */
#define BOSS2_DIVE_MAX_HARD     216
#define BOSS2_PATROL_EXTRA        1  /* patrol px/frame = bossSpeed + 1: 2/3/4 (was 1/2/3) */
/* Turtle fan (V0.3.75): the 3 bullets fall at this % of the normal fan
 * speed (bulletSpeed + 1), min 1 px/frame. bulletSpeed is already capped
 * at 7 on boss waves (10, 30, 50...) on every difficulty, so the fan goes
 * from 8 to 5 px/frame (37.5% slower). Sideways speed (BOSS_FAN_SPREAD)
 * is unchanged, so the fan opens a bit wider. La Cookie keeps 100%. */
#define BOSS2_FAN_SPEED_PCT      65
#define BOSS2_DIVE_SPEED_BASE     5  /* dive px/frame = base + bossSpeed: 6/7/8 (was 5/6/7, 3/4/5) */
#define BOSS2_RETURN_SPEED        2  /* px/frame from above the screen back home */
#define BOSS2_WARN_FRAMES        20  /* telegraph: stops, shakes 1 px, blinks yellow (was 36) */
/* After leaving the bottom it re-enters from the top at a random x
 * (0..RIGHT_EDGE - BOSS2_W), not the column it dived from. */
#define BOSS2_DIVE_SHIELD_BLOCKS  0  /* 1 = an active shield makes the dive harmless */
/* Body contact box for the dive (shell + head, limbs and wand excluded). */
#define BOSS2_BODY_OX        18
#define BOSS2_BODY_OY         8
#define BOSS2_BODY_W         28
#define BOSS2_BODY_H         42
#define BOSS2_ANIM_SHIFT      4  /* 2nd frame (when added): swap every 16 frames */
/* Turtle main attack: a 16x16 star from the wand instead of the bone.
 * Same mechanics as the bone: falls straight down BOSS_STAR_SLOWER px/frame
 * slower than the wave's enemy bullets (min 1), at most 2 on screen, the
 * shield does NOT stop it, player shots ignore it. Hitbox 10x10 centred. */
#define BOSS_STAR_W          16
#define BOSS_STAR_H          16
#define BOSS_STAR_HIT_OX      3
#define BOSS_STAR_HIT_OY      3
#define BOSS_STAR_HIT_W      10
#define BOSS_STAR_HIT_H      10
#define BOSS_STAR_SLOWER      BOSS_BONE_SLOWER
#define MAX_BOSS_STARS        2
/* Star colours in PAL1 slots 10-11 (right after the bone's 7-9). */
#define BOSS_STAR_PAL_FIRST  10
#define BOSS_STAR_PAL_COUNT   2

#define POWER_ON 1
#define POWER_OFF 0

/* RapidFire capsule: 10% drop, at most one on screen, pity after 10
 * kills without a drop. Duration (seconds) lives in the difficulty table. */
#define RAPID_DROP_MOD     10  /* 1 in 10 */
#define RAPID_DROP_PITY    10  /* guaranteed drop on the 10th dry kill */
#define CAPSULE_SPEED       1  /* pixels per frame, downward */
/* Capsule types (V0.4.79, Alberto's idea). When an enemy kill drops a
 * capsule (same 10% + pity + one on screen), its type is rolled with these
 * weights (1/1/1 = 1/3 each; 0 disables a type, at least one must be > 0):
 *   R (orange/white, PAL1)  RapidFire, as before
 *   E (blue/white, PAL1)    refills the shield (ESC) recharge at once
 *   I (green/white, PAL3)   refills the boost (IMP) recharge at once
 * Caught while the shield / dash is in use, the refill is queued: it is
 * ready again the moment that use ends (no recharge). Boss capsules are
 * always R. One sprite: the type is the animation row of imgcapsule.bmp. */
#define CAPSULE_WEIGHT_RAPID   1
#define CAPSULE_WEIGHT_SHIELD  1
#define CAPSULE_WEIGHT_BOOST   1
/* Boss reward (V0.4.78): a defeated boss always drops a capsule at its
 * centre (no 10% roll, replaces any capsule on screen) instead of
 * granting RapidFire on the spot. The spawn is clamped to the screen and
 * to Y <= BOSS_CAPSULE_Y_MAX, so a turtle killed mid-dive (at ship
 * height or off-screen) still drops it in the top half, >= 1 s of fall
 * above the ship. It survives the warp into the next wave. */
#define BOSS_CAPSULE_Y_MIN  BOSS_Y_MIN
#define BOSS_CAPSULE_Y_MAX  112

/* Shield (button C). Duration and recharge (wall-clock seconds) live in
 * the difficulty table. powerups converts them to frames with
 * IS_PAL_SYSTEM (x60 NTSC, x50 PAL) so the real time is the same on both. */
/* Boost / dash (V0.3.76, button A + LEFT or RIGHT). The ship dashes
 * BOOST_FRAMES x BOOST_SPEED = 48 px (3 ship widths) in ~0.13 s, clamped
 * to the screen edges, then recharges for the difficulty's shield recharge
 * time (shieldRechargeSec: 10 / 20 / 30 s on EASY / NORMAL / HARD; its own
 * timer, independent of the shield). A alone does nothing. No
 * invulnerability: the dash only moves you out of the way. Blocked during
 * the between-wave warp.
 * Trail (V0.4.77, Alberto found the single flickering afterimage hard to
 * see): 2 ghost ships drawn every frame where the ship was 2 and 4 frames
 * ago (12 and 24 px behind at full speed), in a blue ghost palette (PAL0
 * colours 1-4, unused by the font), lingering BOOST_TRAIL_LINGER frames
 * after the dash so they slide back into the ship. */
#define BOOST_SPEED           6   /* px/frame while dashing (normal move is 2) */
#define BOOST_FRAMES          8   /* 8 x 6 = 48 px */
#define BOOST_GHOSTS          2   /* trail sprites (1 hardware sprite each) */
#define BOOST_GHOST_LAG1      2   /* frames behind: 2 x 6 = 12 px */
#define BOOST_GHOST_LAG2      4   /* frames behind: 4 x 6 = 24 px */
#define BOOST_TRAIL_LINGER    6   /* frames the trail stays after the dash */
#define BOOST_HIST            8   /* position history (power of 2, > LAG2) */
#define BOOST_READY_BLINK    32   /* HUD bar blink frames when recharged */

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
#define WAVE_WARP_ENTER        80  /* boss slide-in offset (unchanged) */
/* Formation slide-in offset: every row (up to ENEMY_ROWS) starts fully
 * above the screen. 32 + 3 * 24 = 104, so the third row (home y 80)
 * starts at y -24. */
#define ENEMY_WARP_ENTER       (ENEMY_START_Y + ENEMY_ROWS * ENEMY_ROW_SPACING)
/* Kept as aliases so older comments still read; the warp timer is what is used. */
#define WAVE_BANNER_FRAMES_NTSC WAVE_WARP_FRAMES_NTSC
#define WAVE_BANNER_FRAMES_PAL  WAVE_WARP_FRAMES_PAL

#define BG_TILES_W 40           /* visible width of the plane in tiles (320 px / 8) */
#define BG_TILES_H 32           /* plane height in tiles (256 px, matches the scroll wrap) */

#endif
