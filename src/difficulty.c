#include "difficulty.h"

static Difficulty currentDiff = DIFF_NORMAL;

static const DifficultyConfig difficultyTable[DIFF_COUNT] = {
    /* EASY:   no descent, 1 bullet, fires regularly every 90 frames */
    { 0,                   1, 1, 10,  90, 1, 5, 0 },
    /* NORMAL: 8 px step, 2 shots / cap 2, every 120 frames;
     *         rowsOpposite like HARD (row 0 right, row 1 left, bounce alone). */
    { ENEMY_STEP_DOWN,     2, 2, 10, SHOT_INTERVAL, 1, 5, 1 },
    /* HARD:   16 px step, twice as fast sideways, volley every 60 frames,
     *         slightly faster bullets; cap 4 so consecutive volleys can overlap.
     *         rowsOpposite: row 0 goes right, row 1 goes left, each bounces alone. */
    { ENEMY_STEP_DOWN * 2, 4, 2, 10,  60, 2, 6, 1 }
};

static const char* difficultyNames[DIFF_COUNT] = {
    "EASY",
    "NORMAL",
    "HARD"
};

void difficulty_init(void) {
    currentDiff = DIFF_NORMAL;
}

Difficulty difficulty_current(void) {
    return currentDiff;
}

void difficulty_set(Difficulty d) {
    if (d < DIFF_COUNT) currentDiff = d;
}

void difficulty_cycle(s16 dir) {
    s16 d = (s16)currentDiff + dir;
    if (d < 0) d = DIFF_COUNT - 1;
    if (d >= (s16)DIFF_COUNT) d = 0;
    currentDiff = (Difficulty)d;
}

const DifficultyConfig* difficulty_config(void) {
    return &difficultyTable[currentDiff];
}

const char* difficulty_name(void) {
    return difficultyNames[currentDiff];
}
