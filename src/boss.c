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
static u16 flashTimer = 0;
static u16 angX = 0;
static u16 angY = 0;
static u16 bossPal[16];
static u16 bossPalSaved = 0;
static s16 homeY = BOSS_HOME_Y;
static u16 lastWasFan = 0;

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

/* PAL2 is free during a boss wave (no formation). A hit flash only
 * whites out the mothership, not the player on PAL1. The snapshot is
 * taken once at init so a RapidFire tint on PAL1 is not copied over. */
static void captureBossPal(void) {
    u16 i;

    for (i = 0; i < 16; i++) {
        bossPal[i] = PAL_getColor(32 + i);
    }
    bossPalSaved = 1;
}

static void restoreBossPal(void) {
    u16 i;

    if (!bossPalSaved) return;
    for (i = 0; i < 16; i++) {
        PAL_setColor(32 + i, bossPal[i]);
    }
    flashTimer = 0;
}

/* Load the boss sprite's own 16 colors into PAL2 for the boss wave. */
static void applyBossPal(void) {
    PAL_setColors(32, mothership.palette->data, 16, CPU);
    flashTimer = 0;
}

static void flashCore(void) {
    /* Red tint on the body colors (1-3) on PAL2 only. The body is
     * cream, so a white flash would barely show. */
    PAL_setColor(33, RGB24_TO_VDPCOLOR(0xFC6C6C));
    PAL_setColor(34, RGB24_TO_VDPCOLOR(0xFC6C6C));
    PAL_setColor(35, RGB24_TO_VDPCOLOR(0xFC6C6C));
    flashTimer = 6;
}

static void placeSprite(void) {
    if (boss.sprite == NULL) return;
    SPR_setPosition(boss.sprite, boss.x, boss.y);
    /* Hold the bright frame while the core is flashing. */
    SPR_setAnimAndFrame(boss.sprite, 0, (flashTimer > 0) ? 1 : ((animTick >> 3) & 1));
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
    restoreBossPal();
    active = 0;
    entering = 0;
    chargedHit = 0;
    fireTimer = 0;
    flashTimer = 0;
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
    /* 64x56 boss (temporary art from Alberto, 2 frames: flame flicker).
     * Uses its own palette in PAL2 during the boss wave; boss_hide puts
     * the background palette back for the formation. PAL2 so the hit
     * tint does not touch the player (PAL1). High priority so it sits
     * above the starfield. */
    boss.w = BOSS_W;
    boss.h = BOSS_H;
    boss.sprite = SPR_addSprite(&mothership, 0, -40, TILE_ATTR(PAL2, 1, FALSE, FALSE));
    SPR_setAutoAnimation(boss.sprite, FALSE);
    SPR_setVisibility(boss.sprite, HIDDEN);
    boss.health = 0;
    active = 0;
    captureBossPal();
}

void boss_beginEnter(u16 offset) {
    const DifficultyConfig* cfg = difficulty_config();
    u16 wave = difficulty_wave();
    u16 n = appearanceOf(wave);

    boss_hide();
    if (n == 0) return;

    applyBossPal();
    hpMax = hpForAppearance(n);
    hp = hpMax;
    /* Lissajous starts at the centre of the top-half roam box. */
    angX = 0;
    angY = 0;
    homeY = BOSS_HOME_Y;
    boss.x = (RIGHT_EDGE - BOSS_W) / 2;
    boss.y = homeY - (s16)offset;
    boss.velx = (s16)cfg->bossSpeed;
    if (boss.velx == 0) boss.velx = 1;
    boss.health = 1;
    boss.points = cfg->bossPoints * n;
    active = 1;
    entering = (offset != 0) ? 1 : 0;
    chargedHit = 0;
    flashTimer = 0;
    fireTimer = cfg->bossShotInterval;
    lastWasFan = 0;
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
        angX = 0;
        angY = 0;
        placeSprite();
    }
}

/* Main attack (19/11/2026): one 48x16 bone, centred under the belly
 * (x = boss.x + 8, the span the old 8x8 volley covered), falling
 * straight down. Speed, hitbox and cap: BOSS_BONE_* in config.h. */
static void fireBone(void) {
    bullets_shootBone(boss.x + (BOSS_W - BOSS_BONE_W) / 2, boss.y + BOSS_H - 4);
}

/* Second attack: 3 diagonal bullets from the belly, tilted toward the
 * player (left, straight or right) and a bit faster than enemy bullets. */
static void fireFan(void) {
    const DifficultyConfig* cfg = difficulty_config();
    s16 mx = boss.x + (BOSS_W / 2) - 4;
    s16 my = boss.y + BOSS_H - 4;
    s16 dx = player_get()->x - mx;
    s16 aim = 0;
    s16 vy = (s16)cfg->bulletSpeed + 1;

    if (dx > BOSS_FAN_AIM_ZONE) aim = 1;
    else if (dx < -BOSS_FAN_AIM_ZONE) aim = -1;
    bullets_shootEnemyXYV(mx, my, aim, vy);
    bullets_shootEnemyXYV(mx, my, aim - BOSS_FAN_SPREAD, vy);
    bullets_shootEnemyXYV(mx, my, aim + BOSS_FAN_SPREAD, vy);
}

/* Below half HP: alternate fan and bone. Otherwise: random fan. */
static void fireAttack(void) {
    u16 useFan;

    if ((u32)hp * 2 <= (u32)hpMax) useFan = lastWasFan ? 0 : 1;
    else useFan = ((random() % BOSS_FAN_CHANCE) == 0) ? 1 : 0;
    if (useFan) fireFan();
    else fireBone();
    lastWasFan = useFan;
}

/* Smooth Lissajous roam: never leaves the top half of the screen.
 * Speed scales with difficulty (bossSpeed 1/2/3). */
static void roam(u16 speed) {
    s16 xMid = (RIGHT_EDGE - BOSS_W) / 2;
    s16 xAmp = xMid;
    s16 yMid = BOSS_HOME_Y;
    s16 yAmp = (BOSS_Y_MAX - BOSS_Y_MIN) / 2;
    fix16 sx;
    fix16 sy;

    if (speed == 0) speed = 1;
    angX = (angX + (u16)(4 * speed)) & 1023;
    angY = (angY + (u16)(3 * speed)) & 1023;
    sx = sinFix16(angX);
    sy = sinFix16(angY);
    boss.x = xMid + (s16)(((s32)sx * (s32)xAmp) / 64);
    boss.y = yMid + (s16)(((s32)sy * (s32)yAmp) / 64);
    if (boss.x < LEFT_EDGE) boss.x = LEFT_EDGE;
    if (boss.x > RIGHT_EDGE - BOSS_W) boss.x = RIGHT_EDGE - BOSS_W;
    if (boss.y < BOSS_Y_MIN) boss.y = BOSS_Y_MIN;
    if (boss.y > BOSS_Y_MAX) boss.y = BOSS_Y_MAX;
}

void boss_update(void) {
    const DifficultyConfig* cfg;

    if (!active || boss.health == 0) return;

    animTick++;
    if (hitSfxCooldown > 0) hitSfxCooldown--;
    if (flashTimer > 0) {
        flashTimer--;
        if (flashTimer == 0) applyBossPal();
    }
    if (!bullets_chargedActive()) chargedHit = 0;

    if (entering) {
        placeSprite();
        return;
    }

    cfg = difficulty_config();
    roam((u16)cfg->bossSpeed);
    placeSprite();

    if (player_isExploding() || player_get()->health == 0) return;

    if (fireTimer > 0) fireTimer--;
    if (fireTimer == 0) {
        fireAttack();
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
        /* Wing / hull: spark, no damage. A normal shot is blocked;
         * the charged bolt flies through, still without damaging. */
        explosions_spawn(shot->x, shot->y);
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

    flashCore();
    placeSprite();
    if (hitSfxCooldown == 0) {
        XGM_startPlayPCM(SFX_BOSS_HIT, 8, SOUND_PCM_CH4);
        hitSfxCooldown = 3;
    }
    return 2;
}
