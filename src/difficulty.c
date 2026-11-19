#include "difficulty.h"

static Difficulty currentDiff = DIFF_NORMAL;
static u16 currentWave = 1;
static DifficultyConfig runtime;

static const DifficultyConfig difficultyTable[DIFF_COUNT] = {
    /* EASY:   no descent, 1 bullet, fires regularly every 90 frames;
     *         shield 5 s / recharge 10 s; 10 points per ship;
     *         RapidFire lasts 10 s;
     *         boss: 30 dmg, speed 1, 1 shot every 90 frames, 500 pts. */
    { 0,                   1, 1, 10,  90, 1, 5, 0, 5, 10, 10, 10, 30, 1, 90, 1, 500 },
    /* NORMAL: 8 px step, 2 shots / cap 2, every 120 frames;
     *         rowsOpposite like HARD (row 0 right, row 1 left, bounce alone);
     *         shield 3 s / recharge 20 s; 20 points per ship;
     *         RapidFire lasts 8 s;
     *         boss: 20 dmg, speed 2, 2 shots every 60 frames, 1000 pts. */
    { ENEMY_STEP_DOWN,     2, 2, 10, SHOT_INTERVAL, 1, 5, 1, 3, 20, 20, 8, 20, 2, 60, 2, 1000 },
    /* HARD:   16 px step, twice as fast sideways, volley every 60 frames,
     *         slightly faster bullets; cap 4 so consecutive volleys can overlap.
     *         rowsOpposite: row 0 goes right, row 1 goes left, each bounces alone;
     *         shield 2 s / recharge 30 s; 30 points per ship;
     *         RapidFire lasts 8 s;
     *         boss: 10 dmg, speed 3, 4 shots every 40 frames, 1500 pts. */
    { ENEMY_STEP_DOWN * 2, 4, 2, 10,  60, 2, 6, 1, 2, 30, 30, 8, 10, 3, 40, 4, 1500 }
};

/* Per-difficulty caps and increments applied on top of the table above.
 * Wave 1 is always an exact copy of the table (playable as in V0.2.45).
 * idx = effective wave - 1 (see effectiveWave below: 10-wave cycles):
 *   shotInterval     -= intervalDec each wave, floored at intervalMin
 *   enemySpeed       += 1 every speedEvery waves, capped at speedMax
 *   shotsPerVolley   += 1 every shotsEvery waves, capped at shotsMax
 *   maxEnemyBullets  += 1 every bulletsEvery waves, capped at bulletsMax
 *                      (0 every = keep the table value, then only raise to
 *                      cover a larger volley)
 *   bulletSpeed      += 1 every bulletSpeedEvery waves, capped at bulletSpeedMax
 *   stepDown         += stepInc every stepEvery waves, capped at stepMax
 *                      (stepEvery 0 leaves descent unchanged; EASY stays 0) */
typedef struct
{
    u16 intervalDec;
    u16 intervalMin;
    u16 speedEvery;
    u16 speedMax;
    u16 shotsEvery;
    u16 shotsMax;
    u16 bulletsEvery;
    u16 bulletsMax;
    u16 bulletSpeedEvery;
    u16 bulletSpeedMax;
    u16 stepEvery;
    u16 stepInc;
    u16 stepMax;
} WaveTune;

/* V0.3.69 (19/11/2026): gentler ramp so wave 9 plays roughly like the
 * old wave 6-7. Steps every 4 waves instead of 3 (HARD volley every 3
 * instead of 2), slower interval decrease, descent loosened by one wave. */
static const WaveTune waveTuneTable[DIFF_COUNT] = {
    /* EASY:   never descends; at most 2 bullets on screen.
     *         interval 90 -5/wave -> 45; speed 1 -> 3 every 4 waves;
     *         2-shot volley from wave 5; bullet 5 -> 7 every 4 waves. */
    {  5, 45, 4, 3, 4, 2, 4, 2, 4, 7, 0, 0,  0 },
    /* NORMAL: interval 120 -6/wave -> 40; speed 1 -> 3 every 4 waves;
     *         volley/cap 2 -> 4 every 4 waves; bullet 5 -> 7 every 4 waves;
     *         step 8 -> 16 (+4 every 5 waves). */
    {  6, 40, 4, 3, 4, 4, 4, 4, 4, 7, 5, 4, 16 },
    /* HARD:   interval 60 -3/wave -> 30; speed 2 -> 3 every 4 waves;
     *         volley 2 -> 4 every 3 waves (cap already 4); bullet 6 -> 7
     *         every 4 waves; step 16 -> 20 (+4 every 6 waves). */
    {  3, 30, 4, 3, 3, 4, 0, 4, 4, 7, 6, 4, 20 }
};

/* 10-wave cycles (V0.3.69). After each boss wave the formation tuning
 * drops back, then climbs again, a little harder each cycle:
 *   cycle = (wave - 1) / 10,  pos = (wave - 1) % 10 + 1
 *   effective = pos + 3 * cycle   (capped at EFFECTIVE_WAVE_MAX)
 * Wave 1-10 -> 1-10, wave 11 -> 4, 20 -> 13, 21 -> 7, 30 -> 16, 31 -> 10.
 * Only the formation tuning uses it. The boss appearance count, its HP
 * doubling and the OLEADA / TOP 5 numbers use the real wave, and the boss
 * fields of the table (damage, speed, shot interval, points) are never
 * scaled. On a boss wave the effective wave still sets bulletSpeed,
 * which the fan (+1) and the bone (-BOSS_BONE_SLOWER) are based on.
 *
 * Resulting NORMAL values (speed = px/frame sideways, shots = volley
 * size = bullet cap, interval in frames, bullet = px/frame, step = px):
 *
 *   wave  eff  speed  shots  interval  bullet  step
 *     1    1     1      2      120       5      8
 *     2    2     1      2      114       5      8
 *     3    3     1      2      108       5      8
 *     4    4     1      2      102       5      8
 *     5    5     2      3       96       6      8
 *     6    6     2      3       90       6     12
 *     7    7     2      3       84       6     12
 *     8    8     2      3       78       6     12
 *     9    9     3      4       72       7     12
 *    10   10     3      4       66       7     12   (boss: La Cookie)
 *    11    4     1      2      102       5      8
 *    12    5     2      3       96       6      8
 *
 * Before V0.3.69 wave 9 was speed 3, 4 shots, interval 40, bullet 7,
 * step 16 (everything maxed); old wave 6 was 2 / 3 / 70 / 6 / 12 and
 * old wave 7 was 3 / 4 / 60 / 7 / 12. */
#define EFFECTIVE_WAVE_MAX 40

static u16 effectiveWave(u16 wave) {
    u16 cycle;
    u16 pos;
    u16 eff;

    if (wave < 1) return 1;
    cycle = (u16)((wave - 1) / 10);
    pos = (u16)(((wave - 1) % 10) + 1);
    eff = (u16)(pos + 3 * cycle);
    if (eff > EFFECTIVE_WAVE_MAX) eff = EFFECTIVE_WAVE_MAX;
    return eff;
}

static const char* difficultyNames[DIFF_COUNT] = {
    "EASY",
    "NORMAL",
    "HARD"
};

static void applyWaveTuning(void) {
    const DifficultyConfig* base = &difficultyTable[currentDiff];
    const WaveTune* t = &waveTuneTable[currentDiff];
    u16 idx;
    u16 dec;
    u16 bump;

    runtime = *base;
    idx = (u16)(effectiveWave(currentWave) - 1);

    dec = (u16)(t->intervalDec * idx);
    if (base->shotInterval > dec) {
        runtime.shotInterval = base->shotInterval - dec;
    } else {
        runtime.shotInterval = t->intervalMin;
    }
    if (runtime.shotInterval < t->intervalMin) {
        runtime.shotInterval = t->intervalMin;
    }

    if (t->speedEvery > 0) {
        bump = idx / t->speedEvery;
        runtime.enemySpeed = base->enemySpeed + bump;
        if (runtime.enemySpeed > t->speedMax) runtime.enemySpeed = t->speedMax;
    }

    if (t->shotsEvery > 0) {
        bump = idx / t->shotsEvery;
        runtime.shotsPerVolley = base->shotsPerVolley + bump;
        if (runtime.shotsPerVolley > t->shotsMax) runtime.shotsPerVolley = t->shotsMax;
    }
    if (runtime.shotsPerVolley > MAX_ENEMY_BULLETS) {
        runtime.shotsPerVolley = MAX_ENEMY_BULLETS;
    }

    if (t->bulletsEvery > 0) {
        bump = idx / t->bulletsEvery;
        runtime.maxEnemyBullets = base->maxEnemyBullets + bump;
    }
    if (runtime.maxEnemyBullets > t->bulletsMax) {
        runtime.maxEnemyBullets = t->bulletsMax;
    }
    if (runtime.maxEnemyBullets > MAX_ENEMY_BULLETS) {
        runtime.maxEnemyBullets = MAX_ENEMY_BULLETS;
    }
    if (runtime.maxEnemyBullets < runtime.shotsPerVolley) {
        runtime.maxEnemyBullets = runtime.shotsPerVolley;
    }

    if (t->bulletSpeedEvery > 0) {
        bump = idx / t->bulletSpeedEvery;
        runtime.bulletSpeed = base->bulletSpeed + bump;
        if (runtime.bulletSpeed > t->bulletSpeedMax) {
            runtime.bulletSpeed = t->bulletSpeedMax;
        }
    }

    if (t->stepEvery > 0 && t->stepInc > 0) {
        bump = idx / t->stepEvery;
        runtime.stepDown = base->stepDown + bump * t->stepInc;
        if (runtime.stepDown > t->stepMax) runtime.stepDown = t->stepMax;
    }
}

void difficulty_init(void) {
    currentDiff = DIFF_NORMAL;
    currentWave = 1;
    applyWaveTuning();
}

Difficulty difficulty_current(void) {
    return currentDiff;
}

void difficulty_set(Difficulty d) {
    if (d < DIFF_COUNT) currentDiff = d;
    currentWave = 1;
    applyWaveTuning();
}

void difficulty_cycle(s16 dir) {
    s16 d = (s16)currentDiff + dir;
    if (d < 0) d = DIFF_COUNT - 1;
    if (d >= (s16)DIFF_COUNT) d = 0;
    currentDiff = (Difficulty)d;
    currentWave = 1;
    applyWaveTuning();
}

const DifficultyConfig* difficulty_config(void) {
    return &runtime;
}

const char* difficulty_name(void) {
    return difficultyNames[currentDiff];
}

void difficulty_beginMatch(void) {
    difficulty_beginMatchAt(1);
}

void difficulty_beginMatchAt(u16 wave) {
    if (wave < 1) wave = 1;
    if (wave > 99) wave = 99;
    currentWave = wave;
    applyWaveTuning();
}

void difficulty_nextWave(void) {
    if (currentWave < 999) currentWave++;
    applyWaveTuning();
}

u16 difficulty_wave(void) {
    return currentWave;
}
