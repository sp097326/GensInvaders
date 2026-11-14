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
 * idx = wave - 1:
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

static const WaveTune waveTuneTable[DIFF_COUNT] = {
    /* EASY:   gentler ramp; never descends; at most 2 bullets on screen.
     *         interval 90 -> 45; speed 1 -> 3 every 3 waves; 2-shot volley
     *         from wave 4; bullet 5 -> 7. */
    { 10, 45, 3, 3, 3, 2, 3, 2, 3, 7, 0, 0,  0 },
    /* NORMAL: interval 120 -> 40; speed 1 -> 3 every 3 waves; volley/cap
     *         grow to 4; bullet 5 -> 7; step 8 -> 16 (+4 every 4 waves). */
    { 10, 40, 3, 3, 3, 4, 3, 4, 3, 7, 4, 4, 16 },
    /* HARD:   interval 60 -> 30; speed 2 -> 3 every 3 waves; volley 2 -> 4
     *         every 2 waves (cap already 4); bullet 6 -> 7; step 16 -> 20
     *         (+4 every 5 waves). */
    { 10, 30, 3, 3, 2, 4, 0, 4, 3, 7, 5, 4, 20 }
};

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
    idx = (currentWave > 0) ? (u16)(currentWave - 1) : 0;

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
    currentWave = 1;
    applyWaveTuning();
}

void difficulty_nextWave(void) {
    if (currentWave < 999) currentWave++;
    applyWaveTuning();
}

u16 difficulty_wave(void) {
    return currentWave;
}
