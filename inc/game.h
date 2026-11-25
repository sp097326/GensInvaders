#ifndef GAME_H
#define GAME_H

#include <genesis.h>
#include "config.h"

/* Game version. run.bat writes src/version.h before each build with
 * V0.4.xx, where xx = number of commits in the git repo (git rev-list --count HEAD).
 * The prefix is 0.4 since V0.4.77 (bosses alternate + boost; 0.3 was endless
 * waves); the last number is still the total count, not a count from a base tag. version.h is git-ignored; if it is missing
 * we fall back to a dev label. */
#if __has_include("version.h")
#include "version.h"
#else
#define GAME_VERSION "V0.4.DEV"
#endif

/* name[6] holds 5 chars + the terminating '\0', so "PLAYER" (6 chars) didn't fit. */
typedef struct
{
    int x;
    int y;
    int w;
    int h;
    int velx;
    int vely;
    int health;
    Sprite* sprite;
    u16 points;             /* score awarded when this entity is destroyed */
    u16 row;                /* enemy formation row (0 = top) */
    char name[6];
} GameEntity;

/* The main loop only moves things while PLAYING.
 * STATE_TITLE is the boot (and post-game) title screen with START / difficulty.
 * NAME_ENTRY is the 3-letter initials prompt after a qualifying GAME OVER.
 * GAME_OVER then shows the TOP 5 and waits for START to return to the title.
 * STATE_WIN remains in the enum but is unused: clearing a wave starts the next
 * one instead of ending the match. */
typedef enum
{
    STATE_TITLE,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_NAME_ENTRY,
    STATE_GAME_OVER,
    STATE_WIN
} GameState;

/* Hide and deactivate. A NULL sprite is fine (allocation may have failed). */
static inline void killGameEntity(GameEntity* e) {
    if (e == NULL) return;
    e->health = 0;
    if (e->sprite != NULL) SPR_setVisibility(e->sprite, HIDDEN);
}

/* Show and activate only when a sprite exists. Without one the entity
 * stays dead: it must never collide or deal damage. */
static inline void reviveGameEntity(GameEntity* e) {
    if (e == NULL) return;
    if (e->sprite == NULL) {
        e->health = 0;
        return;
    }
    e->health = 1;
    SPR_setVisibility(e->sprite, VISIBLE);
}

/* Allocate a sprite; if VRAM is fragmented, defrag once and retry.
 * The caller must treat a NULL return as a dead entity. */
static inline Sprite* addGameSprite(const SpriteDefinition* def, s16 x, s16 y, u16 attr) {
    Sprite* s;

    s = SPR_addSprite(def, x, y, attr);
    if (s == NULL) {
        SPR_defragVRAM();
        s = SPR_addSprite(def, x, y, attr);
    }
    return s;
}

#endif
