#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <genesis.h>
#include "config.h"

/* Difficulty is data-driven: tweak the table in difficulty.c, not the
 * movement or fire code. */

typedef enum
{
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD,
    DIFF_COUNT
} Difficulty;

typedef struct
{
    u16 stepDown;           /* pixels the formation drops on each edge bounce; 0 = no descent */
    u16 maxEnemyBullets;    /* cap of enemy bullets on screen at once */
    u16 shotsPerVolley;     /* distinct enemies that fire together when the interval hits */
    u16 fireChance;         /* 0-10; the volley fires if (random() % 10) < fireChance */
    u16 shotInterval;       /* frames between volley attempts */
    u16 enemySpeed;         /* horizontal pixels per frame (also the |enemyDir| reset) */
    u16 bulletSpeed;        /* downward speed of each enemy bullet */
    u8  rowsOpposite;       /* 1 = each row has its own direction and bounces alone.
                             * On in NORMAL and HARD; set EASY to 1 to enable it there too. */
    u16 shieldDurationSec;  /* wall-clock seconds the shield stays up */
    u16 shieldRechargeSec;  /* wall-clock seconds before it can be used again */
} DifficultyConfig;

void difficulty_init(void);
Difficulty difficulty_current(void);
void difficulty_set(Difficulty d);
void difficulty_cycle(s16 dir);
const DifficultyConfig* difficulty_config(void);
const char* difficulty_name(void);

/* Wave 1 uses the table as-is. Later waves scale a runtime copy (see
 * difficulty.c). beginMatch resets to wave 1; nextWave advances it. */
void difficulty_beginMatch(void);
void difficulty_nextWave(void);
u16  difficulty_wave(void);

#endif
