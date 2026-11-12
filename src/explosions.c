#include <genesis.h>
#include <resources.h>
#include "explosions.h"

typedef struct {
    Sprite* spr;
    u16 timer;
    u16 active;
    s16 x;
    s16 y;
} EnemyExplosion;

static EnemyExplosion pool[MAX_ENEMY_EXPLOSIONS];

static void hideSlot(u16 i) {
    pool[i].active = 0;
    pool[i].timer = 0;
    if (pool[i].spr != NULL) {
        SPR_setVisibility(pool[i].spr, HIDDEN);
    }
}

void explosions_init(void) {
    u16 i;
    /* PAL3 (green/yellow in play) so the burst stays distinct from the
     * player's orange/red PAL1 fireball. 16x16 = 1 VDP sprite each. */
    for (i = 0; i < MAX_ENEMY_EXPLOSIONS; i++) {
        pool[i].spr = SPR_addSprite(&enemyexp, 0, -32, TILE_ATTR(PAL3, 1, FALSE, FALSE));
        SPR_setAutoAnimation(pool[i].spr, FALSE);
        SPR_setVisibility(pool[i].spr, HIDDEN);
        pool[i].active = 0;
        pool[i].timer = 0;
        pool[i].x = 0;
        pool[i].y = 0;
    }
}

void explosions_reset(void) {
    u16 i;
    for (i = 0; i < MAX_ENEMY_EXPLOSIONS; i++) hideSlot(i);
}

void explosions_hide(void) {
    explosions_reset();
}

void explosions_spawn(s16 x, s16 y) {
    u16 i;
    u16 slot = MAX_ENEMY_EXPLOSIONS;
    u16 minT = 0xFFFF;

    for (i = 0; i < MAX_ENEMY_EXPLOSIONS; i++) {
        if (!pool[i].active) {
            slot = i;
            break;
        }
        if (pool[i].timer < minT) {
            minT = pool[i].timer;
            slot = i;
        }
    }
    /* If every slot was busy, slot holds the oldest (lowest remaining timer). */
    if (slot >= MAX_ENEMY_EXPLOSIONS) slot = 0;

    pool[slot].active = 1;
    pool[slot].timer = EXPLOSION_ENEMY_DURATION;
    pool[slot].x = x;
    pool[slot].y = y;
    if (pool[slot].spr != NULL) {
        SPR_setAutoAnimation(pool[slot].spr, FALSE);
        SPR_setAnimAndFrame(pool[slot].spr, 0, 0);
        SPR_setPosition(pool[slot].spr, x, y);
        SPR_setVisibility(pool[slot].spr, VISIBLE);
    }
    XGM_startPlayPCM(SFX_ENEMY_EXP, 11, SOUND_PCM_CH3);
}

void explosions_update(void) {
    u16 i;
    u16 elapsed;
    u16 frame;
    u16 ticks;

    ticks = EXPLOSION_ENEMY_DURATION / EXPLOSION_ENEMY_ANIM_FRAMES;
    if (ticks == 0) ticks = 1;

    for (i = 0; i < MAX_ENEMY_EXPLOSIONS; i++) {
        if (!pool[i].active) continue;
        pool[i].timer--;
        elapsed = EXPLOSION_ENEMY_DURATION - pool[i].timer;
        frame = elapsed / ticks;
        if (frame >= EXPLOSION_ENEMY_ANIM_FRAMES) frame = EXPLOSION_ENEMY_ANIM_FRAMES - 1;
        if (pool[i].spr != NULL) {
            SPR_setFrame(pool[i].spr, frame);
            SPR_setPosition(pool[i].spr, pool[i].x, pool[i].y);
        }
        if (pool[i].timer == 0) hideSlot(i);
    }
}
