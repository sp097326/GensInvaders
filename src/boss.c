#include <genesis.h>
#include <resources.h>
#include "boss.h"
#include "bullets.h"
#include "player.h"
#include "hud.h"
#include "difficulty.h"
#include "explosions.h"
#include "powerups.h"

static GameEntity boss = {0, 0, BOSS_W, BOSS_H, 0, 0, 0, NULL, 0, 0, "BOSS"};
static u16 hp = 0;
static u16 hpMax = 0;
static u16 active = 0;
static u16 entering = 0;
static u16 animTick = 0;
static u16 fireTimer = 0;
static u16 chargedHit = 0;
static u16 hitSfxCooldown = 0;
static s16 homeY = BOSS_HOME_Y;

static u16 appearanceOf(u16 wave) {
    if (wave < BOSS_WAVE_EVERY) return 0;
    return (u16)(wave / BOSS_WAVE_EVERY);
}

static u16 hpForAppearance(u16 n) {
    u32 value;
    u16 i;

    if (n == 0) return 0;
    value = BOSS_HP_BASE;
    for (i = 1; i < n; i++) {
        value <<= 1;
        if (value > BOSS_HP_CAP) {
            return BOSS_HP_CAP;
        }
    }
    if (value > BOSS_HP_CAP) return BOSS_HP_CAP;
    return (u16)value;
}

static void placeSprite(void) {
    if (boss.sprite == NULL) return;
    SPR_setPosition(boss.sprite, boss.x, boss.y);
    SPR_setAnimAndFrame(boss.sprite, 0, (animTick >> 3) & 1);
    SPR_setVisibility(boss.sprite, (boss.health > 0) ? VISIBLE : HIDDEN);
}

static void refreshBar(void) {
    hud_updateBossBar(active && (hpMax > 0) ? 1 : 0, hp, hpMax);
}

u16 boss_isWave(u16 wave) {
    return (wave >= BOSS_WAVE_EVERY && (wave % BOSS_WAVE_EVERY) == 0) ? 1 : 0;
}

u16 boss_isActive(void) {
    return active;
}

u16 boss_hp(void) {
    return hp;
}

u16 boss_hpMax(void) {
    return hpMax;
}

u16 boss_points(void) {
    return boss.points;
}

void boss_hide(void) {
    active = 0;
    entering = 0;
    chargedHit = 0;
    fireTimer = 0;
    if (boss.sprite != NULL) {
        killGameEntity(&boss);
    } else {
        boss.health = 0;
    }
    hp = 0;
    refreshBar();
}

void boss_reset(void) {
    boss_hide();
    hpMax = 0;
}

void boss_init(void) {
    /* 64x24 mothership: two 32x24 hardware sprites, 2 animation frames.
     * High priority so it sits above the starfield. Placeholder art;
     * Alberto will swap in a final sheet later. */
    boss.w = BOSS_W;
    boss.h = BOSS_H;
    boss.sprite = SPR_addSprite(&mothership, 0, -40, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    SPR_setAutoAnimation(boss.sprite, FALSE);
    SPR_setVisibility(boss.sprite, HIDDEN);
    boss.health = 0;
    active = 0;
}

void boss_beginEnter(u16 offset) {
    const DifficultyConfig* cfg = difficulty_config();
    u16 wave = difficulty_wave();
    u16 n = appearanceOf(wave);

    boss_hide();
    if (n == 0) return;

    hpMax = hpForAppearance(n);
    hp = hpMax;
    boss.x = (RIGHT_EDGE - BOSS_W) / 2;
    homeY = BOSS_HOME_Y;
    boss.y = homeY - (s16)offset;
    boss.velx = (s16)cfg->bossSpeed;
    if (boss.velx == 0) boss.velx = 1;
    boss.health = 1;
    boss.points = cfg->bossPoints * n;
    active = 1;
    entering = (offset != 0) ? 1 : 0;
    chargedHit = 0;
    fireTimer = cfg->bossShotInterval;
    animTick = 0;
    placeSprite();
    refreshBar();
}

void boss_advanceEnter(u16 remain, u16 total, u16 offset) {
    s16 shift;

    if (!active || !entering) return;
    if (total == 0) shift = 0;
    else shift = (s16)(((u32)offset * (u32)remain) / (u32)total);
    boss.y = homeY - shift;
    placeSprite();
    if (remain == 0) {
        entering = 0;
        boss.y = homeY;
        placeSprite();
    }
}

static void fireVolley(void) {
    const DifficultyConfig* cfg = difficulty_config();
    u16 n = cfg->bossShotsPerVolley;
    s16 base;
    s16 gap;
    u16 i;
    GameEntity muzzle;

    if (n == 0) return;
    if (n > MAX_ENEMY_BULLETS) n = MAX_ENEMY_BULLETS;
    muzzle.w = 8;
    muzzle.h = 8;
    muzzle.y = boss.y + BOSS_H - 4;
    if (n == 1) {
        muzzle.x = boss.x + (BOSS_W / 2) - 4;
        bullets_shootEnemyXY(muzzle.x, muzzle.y);
        return;
    }
    /* Spread the volley across the underside, inset 8 px from each tip. */
    base = boss.x + 8;
    gap = (s16)((BOSS_W - 16 - 8) / (n - 1));
    for (i = 0; i < n; i++) {
        bullets_shootEnemyXY(base + gap * (s16)i, muzzle.y);
    }
}

void boss_update(void) {
    const DifficultyConfig* cfg;

    if (!active || boss.health == 0) return;

    animTick++;
    if (hitSfxCooldown > 0) hitSfxCooldown--;
    if (!bullets_chargedActive()) chargedHit = 0;

    if (entering) {
        placeSprite();
        return;
    }

    boss.x += boss.velx;
    if (boss.velx > 0 && (boss.x + BOSS_W >= RIGHT_EDGE)) {
        boss.x = RIGHT_EDGE - BOSS_W;
        boss.velx = -boss.velx;
    } else if (boss.velx < 0 && boss.x <= LEFT_EDGE) {
        boss.x = LEFT_EDGE;
        boss.velx = -boss.velx;
    }
    placeSprite();

    if (player_isExploding() || player_get()->health == 0) return;

    cfg = difficulty_config();
    if (fireTimer > 0) fireTimer--;
    if (fireTimer == 0) {
        fireVolley();
        fireTimer = cfg->bossShotInterval;
        if (fireTimer == 0) fireTimer = 60;
    }
}

static u16 overlaps(s16 ax, s16 ay, s16 aw, s16 ah, s16 bx, s16 by, s16 bw, s16 bh) {
    return (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah >= by) ? 1 : 0;
}

u16 boss_hitShot(GameEntity* shot, u16 charged) {
    s16 cx, cy;
    u16 dmg;
    const DifficultyConfig* cfg;

    if (!active || entering || boss.health == 0 || shot == NULL) return 0;
    if (shot->health == 0) return 0;

    if (!overlaps(shot->x, shot->y, shot->w, shot->h, boss.x, boss.y, boss.w, boss.h)) {
        return 0;
    }

    cx = boss.x + BOSS_CORE_OX;
    cy = boss.y + BOSS_CORE_OY;
    if (!overlaps(shot->x, shot->y, shot->w, shot->h, cx, cy, BOSS_CORE_W, BOSS_CORE_H)) {
        /* Wing / hull: a normal shot is blocked with no damage; the
         * charged bolt flies through, still without damaging. */
        return 1;
    }

    if (charged && chargedHit) {
        /* Already applied this bolt's damage on this pass. */
        return 2;
    }

    cfg = difficulty_config();
    dmg = cfg->bossDamage;
    if (charged) {
        dmg *= BOSS_CHARGED_MULT;
        chargedHit = 1;
    }
    if (dmg >= hp) {
        hp = 0;
    } else {
        hp -= dmg;
    }
    refreshBar();

    if (hp == 0) {
        explosions_spawn(boss.x + 8, boss.y + 4);
        explosions_spawn(cx, cy);
        explosions_spawn(boss.x + BOSS_W - 24, boss.y + 4);
        XGM_startPlayPCM(SFX_BOSS_DIE, 15, SOUND_PCM_CH2);
        powerups_grantRapidFire();
        boss_hide();
        return 3;
    }

    if (!charged && hitSfxCooldown == 0) {
        XGM_startPlayPCM(SFX_BOSS_HIT, 8, SOUND_PCM_CH4);
        hitSfxCooldown = 4;
    }
    return 2;
}
