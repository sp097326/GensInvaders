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
/* Turtle patrol / dive state (V0.3.72). */
#define DIVE_PATROL 0
#define DIVE_WARN   1
#define DIVE_DOWN   2
#define DIVE_BACK   3
static u16 diveState = DIVE_PATROL;
static u16 diveTimer = 0;
static u16 warnTimer = 0;
static s16 drawOX = 0;  /* telegraph shake, sprite only */

/* Per-boss layout (23/11/2026). La Cookie keeps exactly the values it had
 * before (BOSS_* in config.h); the turtle wizard uses BOSS2_*. Since
 * V0.3.73 the turtle is the odd appearances (waves 10, 30...) and La Cookie
 * the even ones (20, 40...).
 * All offsets are from the sprite's top-left corner. */
#define BOSS_SHOT_BONE 0
#define BOSS_SHOT_STAR 1
typedef struct {
    const SpriteDefinition* def;
    s16 w, h;
    s16 coreOX, coreOY, coreW, coreH;  /* damage zone */
    s16 yMax;                          /* lowest roam y (top half) */
    s16 fanOX, fanOY;                  /* 3-bullet fan muzzle */
    u16 shot;                          /* main attack: bone or star */
    s16 shotOX, shotOY;                /* main attack muzzle */
    u16 flashFrame;                    /* 1 = show frame 1 while flashing */
    u16 animShift;                     /* frame swap every 2^n frames */
    u16 patrolDive;                    /* 1 = left-right patrol + dive (turtle) */
    s16 homeY;                         /* roam centre / patrol height */
} BossKind;

static const BossKind kindCookie = {
    &mothership, BOSS_W, BOSS_H,
    BOSS_CORE_OX, BOSS_CORE_OY, BOSS_CORE_W, BOSS_CORE_H,
    BOSS_Y_MAX,
    (BOSS_W / 2) - 4, BOSS_H - 4,
    BOSS_SHOT_BONE, (BOSS_W - BOSS_BONE_W) / 2, BOSS_H - 4,
    1, 3, 0, BOSS_HOME_Y
};

static const BossKind kindTurtle = {
    &boss2turtle, BOSS2_W, BOSS2_H,
    BOSS2_CORE_OX, BOSS2_CORE_OY, BOSS2_CORE_W, BOSS2_CORE_H,
    BOSS2_Y_MAX,
    BOSS2_FAN_OX, BOSS2_FAN_OY,
    BOSS_SHOT_STAR, BOSS2_STAR_OX, BOSS2_STAR_OY,
    0, BOSS2_ANIM_SHIFT, 1, BOSS2_HOME_Y
};

static const BossKind* kind = &kindCookie;

static const BossKind* kindForAppearance(u16 n) {
    u16 odd = n & 1;
    if (!BOSS2_ON_ODD) odd = !odd;
    return odd ? &kindTurtle : &kindCookie;
}

static s16 kindHomeY(const BossKind* k) {
    return k->homeY;  /* Cookie: 42 = BOSS_HOME_Y, turtle: BOSS2_HOME_Y */
}

/* Random frames until the next turtle dive, by difficulty. */
static u16 nextDiveDelay(void) {
    u16 lo, hi;

    switch (difficulty_current()) {
    case DIFF_EASY: lo = BOSS2_DIVE_MIN_EASY; hi = BOSS2_DIVE_MAX_EASY; break;
    case DIFF_HARD: lo = BOSS2_DIVE_MIN_HARD; hi = BOSS2_DIVE_MAX_HARD; break;
    default:        lo = BOSS2_DIVE_MIN_NORMAL; hi = BOSS2_DIVE_MAX_NORMAL; break;
    }
    return lo + (u16)(random() % (u16)(hi - lo + 1));
}

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
    PAL_setColors(32, kind->def->palette->data, 16, CPU);
    flashTimer = 0;
}

static void flashCore(void) {
    /* Red tint on the body colors (1-3) on PAL2 only. The body is
     * cream, so a white flash would barely show. On the turtle 1-3 are
     * the shell and limb greens (robe, wand and stars keep their colour). */
    PAL_setColor(33, RGB24_TO_VDPCOLOR(0xFC6C6C));
    PAL_setColor(34, RGB24_TO_VDPCOLOR(0xFC6C6C));
    PAL_setColor(35, RGB24_TO_VDPCOLOR(0xFC6C6C));
    flashTimer = 6;
}

static void placeSprite(void) {
    u16 frames;
    u16 frame = 0;

    if (boss.sprite == NULL) return;
    SPR_setPosition(boss.sprite, boss.x + drawOX, boss.y);
    /* Cookie holds its bright frame while the core is flashing. A
     * 1-frame boss (the turtle for now) always shows frame 0; drop a 2nd
     * frame into its PNG and it starts animating with no code change. */
    frames = kind->def->animations[0]->numFrame;
    if (frames > 1) {
        if (kind->flashFrame && flashTimer > 0) frame = 1;
        else frame = (animTick >> kind->animShift) & 1;
    }
    SPR_setAnimAndFrame(boss.sprite, 0, frame);
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
    diveState = DIVE_PATROL;
    drawOX = 0;
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
     * Odd boss appearances (waves 10, 30...) swap this same sprite to the
     * 64x64 turtle wizard (boss_beginEnter); even ones put La Cookie back.
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

    /* Pick the boss for this appearance and swap the sprite definition
     * (same Sprite, VRAM re-allocated by SGDK). If the swap fails, fall
     * back to La Cookie so the wave is still playable. */
    kind = kindForAppearance(n);
    if (!SPR_setDefinition(boss.sprite, kind->def)) {
        kind = &kindCookie;
        SPR_setDefinition(boss.sprite, kind->def);
    }
    SPR_setVisibility(boss.sprite, HIDDEN);
    boss.w = kind->w;
    boss.h = kind->h;
    applyBossPal();
    hpMax = hpForAppearance(n);
    hp = hpMax;
    /* Lissajous starts at the centre of the top-half roam box. */
    angX = 0;
    angY = 0;
    diveState = DIVE_PATROL;
    diveTimer = nextDiveDelay();
    warnTimer = 0;
    drawOX = 0;
    homeY = kindHomeY(kind);
    boss.x = (RIGHT_EDGE - kind->w) / 2;
    boss.y = homeY - (s16)offset;
    boss.velx = (s16)cfg->bossSpeed;
    if (kind->patrolDive) boss.velx += BOSS2_PATROL_EXTRA;  /* turtle: 2/3/4 */
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

/* Main attack. Cookie (19/11/2026): one 48x16 bone, centred under the
 * belly (x = boss.x + 8, the span the old 8x8 volley covered), falling
 * straight down. Speed, hitbox and cap: BOSS_BONE_* in config.h.
 * Turtle (23/11/2026): one 16x16 star from the wand tip, same rules
 * (BOSS_STAR_* in config.h). */
static void fireMain(void) {
    s16 x = boss.x + kind->shotOX;
    s16 y = boss.y + kind->shotOY;

    if (kind->shot == BOSS_SHOT_STAR) bullets_shootStar(x, y);
    else bullets_shootBone(x, y);
}

/* Second attack: 3 diagonal bullets from the belly, tilted toward the
 * player (left, straight or right) and a bit faster than enemy bullets. */
static void fireFan(void) {
    const DifficultyConfig* cfg = difficulty_config();
    s16 mx = boss.x + kind->fanOX;
    s16 my = boss.y + kind->fanOY;
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
    else fireMain();
    lastWasFan = useFan;
}

/* Smooth Lissajous roam (La Cookie): never leaves the top half of the
 * screen. Speed scales with difficulty (bossSpeed 1/2/3). */
static void roam(u16 speed) {
    s16 xMid = (RIGHT_EDGE - kind->w) / 2;
    s16 xAmp = xMid;
    s16 yMid = kindHomeY(kind);
    s16 yAmp = (kind->yMax - BOSS_Y_MIN) / 2;
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
    if (boss.x > RIGHT_EDGE - kind->w) boss.x = RIGHT_EDGE - kind->w;
    if (boss.y < BOSS_Y_MIN) boss.y = BOSS_Y_MIN;
    if (boss.y > kind->yMax) boss.y = kind->yMax;
}

/* Turtle: constant left-right patrol at its home height. */
static void patrol(void) {
    s16 xMax = RIGHT_EDGE - kind->w;

    boss.x += boss.velx;
    if (boss.x <= LEFT_EDGE) {
        boss.x = LEFT_EDGE;
        if (boss.velx < 0) boss.velx = -boss.velx;
    } else if (boss.x >= xMax) {
        boss.x = xMax;
        if (boss.velx > 0) boss.velx = -boss.velx;
    }
    boss.y = homeY;
}

/* Telegraph tint: body colours 1-3 blink yellow. A red hit flash wins. */
static void warnTint(u16 on) {
    if (flashTimer > 0) return;
    if (on) {
        PAL_setColor(33, RGB24_TO_VDPCOLOR(0xFCFC6C));
        PAL_setColor(34, RGB24_TO_VDPCOLOR(0xFCFC6C));
        PAL_setColor(35, RGB24_TO_VDPCOLOR(0xFCFC6C));
    } else {
        applyBossPal();
    }
}

/* Turtle state machine. Returns 1 while it may fire (patrol only). */
static u16 updateTurtle(const DifficultyConfig* cfg, u16 playerOk) {
    s16 speed;

    switch (diveState) {
    case DIVE_WARN:
        drawOX = ((warnTimer >> 1) & 1) ? 1 : -1;
        warnTint((warnTimer >> 2) & 1);
        if (warnTimer > 0) warnTimer--;
        if (warnTimer == 0) {
            drawOX = 0;
            warnTint(0);
            diveState = DIVE_DOWN;
        }
        return 0;
    case DIVE_DOWN:
        speed = BOSS2_DIVE_SPEED_BASE + (s16)cfg->bossSpeed;
        boss.y += speed;
        if (boss.y > BOTTOM_EDGE) {
            /* Fully below the screen: jump to just above the top (the
             * sprite is off screen at both ends, so no visible wrap). */
            boss.y = -kind->h;
            /* Come back at a random column (Alberto, V0.3.73). */
            boss.x = LEFT_EDGE + (s16)(random() % (u16)(RIGHT_EDGE - kind->w - LEFT_EDGE + 1));
            diveState = DIVE_BACK;
        }
        return 0;
    case DIVE_BACK:
        boss.y += BOSS2_RETURN_SPEED;
        if (boss.y >= homeY) {
            boss.y = homeY;
            diveState = DIVE_PATROL;
            diveTimer = nextDiveDelay();
        }
        return 0;
    default:
        patrol();
        if (!playerOk) return 1;
        if (diveTimer > 0) diveTimer--;
        if (diveTimer == 0) {
            diveState = DIVE_WARN;
            warnTimer = BOSS2_WARN_FRAMES;
            return 0;
        }
        return 1;
    }
}

u16 boss_bodyHits(GameEntity* p) {
    s16 bx, by;

    if (!active || entering || boss.health == 0 || p == NULL) return 0;
    if (!kind->patrolDive) return 0;
    if (diveState != DIVE_DOWN && diveState != DIVE_BACK) return 0;
    bx = boss.x + BOSS2_BODY_OX;
    by = boss.y + BOSS2_BODY_OY;
    return (p->x < bx + BOSS2_BODY_W && p->x + p->w > bx &&
            p->y < by + BOSS2_BODY_H && p->y + p->h >= by) ? 1 : 0;
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
    if (kind->patrolDive) {
        u16 playerOk = (player_isExploding() || player_get()->health == 0) ? 0 : 1;
        u16 canFire = updateTurtle(cfg, playerOk);
        placeSprite();
        if (!playerOk || !canFire) return;
    } else {
        roam((u16)cfg->bossSpeed);
        placeSprite();
        if (player_isExploding() || player_get()->health == 0) return;
    }

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

    cx = boss.x + kind->coreOX;
    cy = boss.y + kind->coreOY;
    if (!overlaps(shot->x, shot->y, shot->w, shot->h, cx, cy, kind->coreW, kind->coreH)) {
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
        explosions_spawn(boss.x + boss.w - 24, boss.y + 4);
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
