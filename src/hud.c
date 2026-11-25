#include "hud.h"
#include "difficulty.h"
#include "config.h"
#include "boss.h"

#define SHIELD_LABEL_X     29
#define SHIELD_BAR_X       32
#define SHIELD_BAR_Y       1
#define SHIELD_BAR_LEN     8
#define TILE_BAR_EMPTY     (TILE_USER_INDEX + 2)
#define TILE_BAR_BLUE1     (TILE_BAR_EMPTY + 1)   /* 1/8 .. 8/8, color 1 */
#define TILE_BAR_RED1      (TILE_BAR_BLUE1 + 8)   /* 1/8 .. 8/8, color 5 */
#define BAR_TILE_COUNT     17
#define TILE_SHIP_ICON     (TILE_BAR_EMPTY + BAR_TILE_COUNT)
#define BAR_BAND_Y0        2
#define BAR_BAND_Y1        5

/* Nine 8x8 ship icons at the bottom-left of the visible plane (row 27,
 * pixels 216-223, cols 0-8), left-aligned so a new life appears to the
 * right of the ones already there. The player sits at Y=192 (pixels
 * 192-207) and the 32px shield reaches Y=216, so this row is just
 * below the bubble and never overlaps the ship; PLAYER_START_Y stays
 * put. High priority keeps the icons above enemy bullets and a late
 * invasion in that corner. Bottom-left is the classic arcade corner
 * and frees the already busy top HUD.
 * "+1 NAVE" sits on the same row, just after the 9 icon slots. */
#define LIVES_ICON_X        0
#define LIVES_ICON_Y       27
#define LIVES_ICON_SLOTS   LIVES_CAP
#define EXTRA_LIFE_TEXT_X  10
#define EXTRA_LIFE_TEXT_Y  27
#define EXTRA_LIFE_TEXT_W   8  /* "+9 NAVE" fits; stops short of the IMP bar at col 18 */
#define BOOST_LABEL_X      18
#define BOOST_BAR_X        21
#define BOOST_BAR_Y        27
#define BOOST_BAR_LEN       8
#define RAPID_LABEL_X      30
#define RAPID_BAR_X        32
#define RAPID_BAR_Y        27
#define RAPID_BAR_LEN       8
#define BOSS_LABEL_X        9
#define BOSS_BAR_X         14
#define BOSS_BAR_Y          2
#define BOSS_BAR_LEN       16

static char hud_string[40] = "";
static u16 shieldState = SHIELD_HUD_READY;
static u16 shieldLevel = SHIELD_BAR_STEPS;
static u16 shieldBlink = 0;
static u16 barAttr[8];
static u16 barDrawn = 0;
static u16 hudLives = 0;
static u16 extraLifeTimer = 0;
static u16 extraLifeGranted = 0;
static u16 extraLifeIconOn = 1;
static u16 rapidActive = 0;
static u16 rapidLevel = 0;
static u16 rapidBarAttr[8];
static u16 rapidBarDrawn = 0;
static u16 boostState = SHIELD_HUD_READY;
static u16 boostLevel = SHIELD_BAR_STEPS;
static u16 boostBlink = 0;
static u16 boostBarAttr[8];
static u16 boostBarDrawn = 0;
static u16 bossBarActive = 0;
static u16 bossBarLevel = 0;
static u16 bossBarAttr[16];
static u16 bossBarDrawn = 0;

static int hudScore = 0;
static u16 hudEnemiesLeft = 0;
static u16 hudBossLine = 0;
static u16 hudWave = 0;
static char hudDiff[12] = "";
static u16 dirtyScore = 0;
static u16 dirtyLives = 0;
static u16 dirtyWave = 0;
static u16 dirtyDiff = 0;
static u16 dirtyShield = 0;
static u16 dirtyBoost = 0;
static u16 dirtyRapid = 0;
static u16 dirtyBoss = 0;
static u16 dirtyFull = 0;

/* Set to 1 to count field draws per flush (worst-case of a multi-kill
 * charged shot should stay at 1 score line, plus lives only if a 1UP
 * happened). Leave at 0. */
#ifndef HUD_DEBUG_COUNTS
#define HUD_DEBUG_COUNTS 0
#endif
#if HUD_DEBUG_COUNTS
static u16 hudDebugFields = 0;
static u16 hudDebugFieldsMax = 0;
#endif

#define HUD_SCORE_W     40  /* whole of row 0; trailing spaces erase leftovers */
#define HUD_DIFF_X       0
#define HUD_DIFF_W      16  /* cols 0-15, before OLEADA */
#define HUD_WAVE_X      16
#define HUD_WAVE_W      13  /* cols 16-28, before ESC at 29 */

static void drawBossBar(u16 force);
static void drawScoreLine(void);
static void drawDiffField(void);
static void drawWaveField(void);

/* 8x8 silhouette of the player ship (PAL1: 1 blue, 2 orange, 3 white,
 * 4 black). Side columns stay empty so adjacent icons do not touch. */
static const u32 shipIconTile[8] = {
    0x00022000,
    0x00033000,
    0x00233200,
    0x00344300,
    0x02433420,
    0x02333320,
    0x03244230,
    0x00011000
};

static void buildBarTile(u32* dst, u16 fillPx, u16 fillColor) {
    u16 y;
    u16 x;
    for (y = 0; y < 8; y++) {
        u32 row = 0;
        for (x = 0; x < 8; x++) {
            u16 col;
            if ((y < BAR_BAND_Y0) || (y > BAR_BAND_Y1)) {
                col = 0;
            } else if (x < fillPx) {
                col = fillColor;
            } else {
                col = 4; /* PAL1/PAL3 index 4 is black: the empty track */
            }
            row = (row << 4) | col;
        }
        dst[y] = row;
    }
}

static void drawShieldLabel(void) {
    VDP_drawText("ESC", SHIELD_LABEL_X, SHIELD_BAR_Y);
}

static void drawShieldBar(u16 force) {
    u16 i;
    u16 pal;
    u16 base;
    u16 state = shieldState;

    if (state == SHIELD_HUD_RECHARGE) {
        pal = PAL1;
        base = TILE_BAR_RED1;
    } else if (state == SHIELD_HUD_ACTIVE) {
        pal = PAL1;
        base = TILE_BAR_BLUE1;
    } else {
        /* Ready: green via PAL3 (in-game color 1 is the bottom-row green).
         * A short blink uses PAL1 so the same tiles flash blue. */
        pal = shieldBlink ? PAL1 : PAL3;
        base = TILE_BAR_BLUE1;
    }

    if (force || !barDrawn) drawShieldLabel();

    for (i = 0; i < SHIELD_BAR_LEN; i++) {
        u16 px;
        u16 idx;
        u16 attr;
        u16 start = i * 8;
        if (shieldLevel >= (start + 8)) px = 8;
        else if (shieldLevel > start) px = shieldLevel - start;
        else px = 0;
        if (px == 0) idx = TILE_BAR_EMPTY;
        else idx = base + (px - 1);
        attr = TILE_ATTR_FULL(pal, 1, FALSE, FALSE, idx);
        if (force || !barDrawn || (barAttr[i] != attr)) {
            VDP_setTileMapXY(BG_A, attr, SHIELD_BAR_X + i, SHIELD_BAR_Y);
            barAttr[i] = attr;
        }
    }
    barDrawn = 1;
}

/* Same look as the shield bar: green ready (PAL3), blue while dashing,
 * red while recharging; a ready blink flashes it blue. */
static void drawBoostBar(u16 force) {
    u16 i;
    u16 pal;
    u16 base;

    if (boostState == SHIELD_HUD_RECHARGE) {
        pal = PAL1;
        base = TILE_BAR_RED1;
    } else if (boostState == SHIELD_HUD_ACTIVE) {
        pal = PAL1;
        base = TILE_BAR_BLUE1;
    } else {
        pal = boostBlink ? PAL1 : PAL3;
        base = TILE_BAR_BLUE1;
    }
    if (force || !boostBarDrawn) VDP_drawText("IMP", BOOST_LABEL_X, BOOST_BAR_Y);
    for (i = 0; i < BOOST_BAR_LEN; i++) {
        u16 px;
        u16 idx;
        u16 attr;
        u16 start = i * 8;
        if (boostLevel >= (start + 8)) px = 8;
        else if (boostLevel > start) px = boostLevel - start;
        else px = 0;
        if (px == 0) idx = TILE_BAR_EMPTY;
        else idx = base + (px - 1);
        attr = TILE_ATTR_FULL(pal, 1, FALSE, FALSE, idx);
        if (force || !boostBarDrawn || (boostBarAttr[i] != attr)) {
            VDP_setTileMapXY(BG_A, attr, BOOST_BAR_X + i, BOOST_BAR_Y);
            boostBarAttr[i] = attr;
        }
    }
    boostBarDrawn = 1;
}

static void padAndDraw(u16 x, u16 y, const char* text, u16 width) {
    char buf[41];
    u16 i;

    if (width > 40) width = 40;
    for (i = 0; i < width; i++) {
        if (text[i] != 0) {
            buf[i] = text[i];
        } else {
            break;
        }
    }
    for (; i < width; i++) buf[i] = ' ';
    buf[width] = 0;
    VDP_drawText(buf, x, y);
#if HUD_DEBUG_COUNTS
    hudDebugFields++;
#endif
}

static void drawScoreLine(void) {
    if (hudBossLine) {
        sprintf(hud_string, "SCORE: %d  JEFE", hudScore);
    } else {
        sprintf(hud_string, "SCORE: %d  LEFT: %d", hudScore, hudEnemiesLeft);
    }
    padAndDraw(0, 0, hud_string, HUD_SCORE_W);
}

static void drawDiffField(void) {
    sprintf(hud_string, "DIFF: %s", hudDiff);
    padAndDraw(HUD_DIFF_X, 1, hud_string, HUD_DIFF_W);
}

static void drawWaveField(void) {
    sprintf(hud_string, "OLEADA %d", hudWave);
    padAndDraw(HUD_WAVE_X, 1, hud_string, HUD_WAVE_W);
}

void hud_updateBoostBar(u16 state, u16 level, u16 blink) {
    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    if (boostState == state && boostLevel == level && boostBlink == blink && boostBarDrawn) {
        return;
    }
    boostState = state;
    boostLevel = level;
    boostBlink = blink;
    dirtyBoost = 1;
}

/* Plane-A tiles, not sprites, so they never add to the 20-sprite/line
 * budget. High priority keeps them above passing bullets. During a 1UP
 * the newly awarded icon(s) blink; the rest stay solid. */
static void drawLivesIcons(void) {
    u16 i;
    u16 attr = TILE_ATTR_FULL(PAL1, 1, FALSE, FALSE, TILE_SHIP_ICON);
    u16 blinkFrom;

    if ((extraLifeTimer > 0) && (extraLifeGranted > 0) &&
        (hudLives >= extraLifeGranted)) {
        blinkFrom = hudLives - extraLifeGranted;
    } else {
        blinkFrom = hudLives;
    }

    for (i = 0; i < LIVES_ICON_SLOTS; i++) {
        u16 show = (i < hudLives);
        if (show && (i >= blinkFrom) && !extraLifeIconOn) {
            show = 0;
        }
        if (show) {
            VDP_setTileMapXY(BG_A, attr, LIVES_ICON_X + i, LIVES_ICON_Y);
        } else {
            VDP_clearText(LIVES_ICON_X + i, LIVES_ICON_Y, 1);
        }
    }
}

void hud_init(void) {
    static u32 tiles[BAR_TILE_COUNT][8];
    u16 i;

    /* HUD text (score, banner, pause, GAME OVER) sits above sprites so
     * player bullets climbing past the top cannot punch holes in it. */
    VDP_setTextPriority(TRUE);

    buildBarTile(tiles[0], 0, 1);
    for (i = 1; i <= 8; i++) buildBarTile(tiles[i], i, 1);
    for (i = 1; i <= 8; i++) buildBarTile(tiles[8 + i], i, 5);
    VDP_loadTileData((const u32*)tiles, TILE_BAR_EMPTY, BAR_TILE_COUNT, CPU);
    VDP_loadTileData(shipIconTile, TILE_SHIP_ICON, 1, CPU);
    barDrawn = 0;
}

void hud_updateScore(int score, u16 enemiesLeft, u16 lives) {
    u16 livesCap;
    u16 wave;
    const char* diff;
    u16 bossLine;

    livesCap = (lives > LIVES_CAP) ? LIVES_CAP : lives;
    if (livesCap != hudLives) {
        hudLives = livesCap;
        dirtyLives = 1;
    }

    bossLine = boss_isActive() ? 1 : 0;
    hudScore = score;
    hudEnemiesLeft = enemiesLeft;
    hudBossLine = bossLine;
    dirtyScore = 1;

    wave = difficulty_wave();
    if (wave != hudWave) {
        hudWave = wave;
        dirtyWave = 1;
    }
    diff = difficulty_name();
    if (diff[0] == 0 || strcmp(hudDiff, diff) != 0) {
        u16 i;
        for (i = 0; i < 11; i++) {
            hudDiff[i] = diff[i];
            if (diff[i] == 0) break;
        }
        hudDiff[11] = 0;
        dirtyDiff = 1;
    }

    /* After the title wipes plane A the bar caches are empty: request a
     * full paint so ESC / IMP / RF / JEFE come back with the score. */
    if (!barDrawn || !boostBarDrawn) {
        dirtyFull = 1;
    }
}

void hud_updateShieldBar(u16 state, u16 level, u16 blink) {
    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    if (shieldState == state && shieldLevel == level && shieldBlink == blink && barDrawn) {
        return;
    }
    shieldState = state;
    shieldLevel = level;
    shieldBlink = blink;
    dirtyShield = 1;
}

static void drawRapidBar(u16 force) {
    u16 i;

    if (!rapidActive) {
        if (force || rapidBarDrawn) {
            VDP_clearText(RAPID_LABEL_X, RAPID_BAR_Y, 2);
            VDP_clearText(RAPID_BAR_X, RAPID_BAR_Y, RAPID_BAR_LEN);
            rapidBarDrawn = 0;
        }
        return;
    }

    if (force || !rapidBarDrawn) {
        VDP_drawText("RF", RAPID_LABEL_X, RAPID_BAR_Y);
    }

    for (i = 0; i < RAPID_BAR_LEN; i++) {
        u16 px;
        u16 idx;
        u16 attr;
        u16 start = i * 8;
        if (rapidLevel >= (start + 8)) px = 8;
        else if (rapidLevel > start) px = rapidLevel - start;
        else px = 0;
        if (px == 0) idx = TILE_BAR_EMPTY;
        else idx = TILE_BAR_RED1 + (px - 1);
        attr = TILE_ATTR_FULL(PAL1, 1, FALSE, FALSE, idx);
        if (force || !rapidBarDrawn || (rapidBarAttr[i] != attr)) {
            VDP_setTileMapXY(BG_A, attr, RAPID_BAR_X + i, RAPID_BAR_Y);
            rapidBarAttr[i] = attr;
        }
    }
    rapidBarDrawn = 1;
}

void hud_updateRapidBar(u16 active, u16 level) {
    u16 on;

    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    on = (active && level > 0) ? 1 : 0;
    if (rapidActive == on && rapidLevel == level && (on == 0 || rapidBarDrawn)) {
        return;
    }
    rapidActive = on;
    rapidLevel = level;
    dirtyRapid = 1;
}

void hud_hideRapidBar(void) {
    if (!rapidActive && !rapidBarDrawn) return;
    rapidActive = 0;
    rapidLevel = 0;
    dirtyRapid = 1;
}

static void drawBossBar(u16 force) {
    u16 i;
    u16 base;

    if (!bossBarActive) {
        if (force || bossBarDrawn) {
            VDP_clearText(0, BOSS_BAR_Y, 40);
            bossBarDrawn = 0;
        }
        return;
    }

    if (force || !bossBarDrawn) {
        VDP_drawText("JEFE", BOSS_LABEL_X, BOSS_BAR_Y);
    }

    /* Blue while healthy, red when under 25%. */
    base = (bossBarLevel * 4 < BOSS_BAR_STEPS) ? TILE_BAR_RED1 : TILE_BAR_BLUE1;

    for (i = 0; i < BOSS_BAR_LEN; i++) {
        u16 px;
        u16 idx;
        u16 attr;
        u16 start = i * 8;
        if (bossBarLevel >= (start + 8)) px = 8;
        else if (bossBarLevel > start) px = bossBarLevel - start;
        else px = 0;
        if (px == 0) idx = TILE_BAR_EMPTY;
        else idx = base + (px - 1);
        attr = TILE_ATTR_FULL(PAL1, 1, FALSE, FALSE, idx);
        if (force || !bossBarDrawn || (bossBarAttr[i] != attr)) {
            VDP_setTileMapXY(BG_A, attr, BOSS_BAR_X + i, BOSS_BAR_Y);
            bossBarAttr[i] = attr;
        }
    }
    bossBarDrawn = 1;
}

void hud_updateBossBar(u16 active, u16 hp, u16 hpMax) {
    u16 level;

    if (!active || hpMax == 0 || hp == 0) {
        if (bossBarActive || bossBarDrawn) {
            bossBarActive = 0;
            bossBarLevel = 0;
            dirtyBoss = 1;
        }
        return;
    }
    level = (u16)(((u32)hp * BOSS_BAR_STEPS + (u32)hpMax - 1) / (u32)hpMax);
    if (level > BOSS_BAR_STEPS) level = BOSS_BAR_STEPS;
    if (level < 1) level = 1;
    if (bossBarActive && bossBarDrawn && bossBarLevel == level && !dirtyBoss) return;
    bossBarActive = 1;
    bossBarLevel = level;
    dirtyBoss = 1;
}

void hud_hideBossBar(void) {
    if (!bossBarActive && !bossBarDrawn && !dirtyBoss) return;
    bossBarActive = 0;
    bossBarLevel = 0;
    dirtyBoss = 1;
}

static void clearDirty(void) {
    dirtyScore = 0;
    dirtyLives = 0;
    dirtyWave = 0;
    dirtyDiff = 0;
    dirtyShield = 0;
    dirtyBoost = 0;
    dirtyRapid = 0;
    dirtyBoss = 0;
    dirtyFull = 0;
}

void hud_invalidate(void) {
    /* Plane A was wiped (title_clear). Keep logical bar levels so the
     * next match can repaint them, but drop "already drawn" so we do
     * not skip the first in-game flush, and do not paint on the title. */
    barDrawn = 0;
    boostBarDrawn = 0;
    rapidBarDrawn = 0;
    bossBarDrawn = 0;
    clearDirty();
}

void hud_flush(void) {
    u16 force;

#if HUD_DEBUG_COUNTS
    hudDebugFields = 0;
#endif
    if (dirtyFull) {
        force = 1;
        drawScoreLine();
        drawDiffField();
        drawWaveField();
        drawLivesIcons();
        drawShieldBar(force);
        drawBoostBar(force);
        drawRapidBar(force);
        drawBossBar(force);
        clearDirty();
#if HUD_DEBUG_COUNTS
        if (hudDebugFields > hudDebugFieldsMax) hudDebugFieldsMax = hudDebugFields;
#endif
        return;
    }
    force = 0;
    if (dirtyScore) drawScoreLine();
    if (dirtyDiff) drawDiffField();
    if (dirtyWave) drawWaveField();
    if (dirtyLives) drawLivesIcons();
    if (dirtyShield) drawShieldBar(force);
    if (dirtyBoost) drawBoostBar(force);
    if (dirtyRapid) drawRapidBar(force);
    if (dirtyBoss) drawBossBar(force);
    clearDirty();
#if HUD_DEBUG_COUNTS
    if (hudDebugFields > hudDebugFieldsMax) hudDebugFieldsMax = hudDebugFields;
#endif
}

void hud_clear(void) {
    extraLifeTimer = 0;
    extraLifeGranted = 0;
    extraLifeIconOn = 1;
    hudLives = 0;
    hudScore = 0;
    hudEnemiesLeft = 0;
    hudBossLine = 0;
    hudWave = 0;
    hudDiff[0] = 0;
    rapidActive = 0;
    rapidLevel = 0;
    boostBarDrawn = 0;
    bossBarActive = 0;
    bossBarLevel = 0;
    VDP_clearText(0, 0, 40);
    VDP_clearText(0, 1, 40);
    VDP_clearText(0, BOSS_BAR_Y, 40);
    VDP_clearText(0, LIVES_ICON_Y, 40);
    hud_invalidate();
}

void hud_drawCentered(const char* text, u16 y) {
    u16 len = strlen(text);
    VDP_drawText(text, (40 - len) / 2, y);
}

void hud_clearOverlay(void) {
    u16 row;
    /* 10-22 covers GAME OVER + TOP 5 + PRESS START, and the name entry. */
    for (row = 10; row <= 22; row++) {
        VDP_clearText(0, row, 40);
    }
}

static char extraLifeText[20] = "+1 NAVE";

void hud_showExtraLife(u16 granted) {
    if (granted == 0) return;
    if (granted == 1) {
        sprintf(extraLifeText, "+1 NAVE");
    } else {
        sprintf(extraLifeText, "+%u NAVE", granted);
    }
    /* Draw the banner once, on the bottom row next to the icons, and
     * leave it. Do not clear the icon slots. */
    VDP_clearText(EXTRA_LIFE_TEXT_X, EXTRA_LIFE_TEXT_Y, EXTRA_LIFE_TEXT_W);
    VDP_drawText(extraLifeText, EXTRA_LIFE_TEXT_X, EXTRA_LIFE_TEXT_Y);
    extraLifeGranted = granted;
    extraLifeIconOn = 1;
    extraLifeTimer = IS_PAL_SYSTEM ? EXTRA_LIFE_BANNER_PAL : EXTRA_LIFE_BANNER_NTSC;
    drawLivesIcons();
}

void hud_showBonus(u16 points) {
    sprintf(extraLifeText, "+%u", points);
    VDP_clearText(EXTRA_LIFE_TEXT_X, EXTRA_LIFE_TEXT_Y, EXTRA_LIFE_TEXT_W);
    VDP_drawText(extraLifeText, EXTRA_LIFE_TEXT_X, EXTRA_LIFE_TEXT_Y);
    if (extraLifeGranted > 0) {
        /* Cut a "+1 NAVE" blink short: put its icons back to solid. */
        extraLifeGranted = 0;
        extraLifeIconOn = 1;
        drawLivesIcons();
    }
    extraLifeTimer = IS_PAL_SYSTEM ? EXTRA_LIFE_BANNER_PAL : EXTRA_LIFE_BANNER_NTSC;
}

void hud_updateExtraLife(void) {
    u16 iconOn;
    if (extraLifeTimer == 0) return;
    extraLifeTimer--;
    if (extraLifeTimer == 0) {
        VDP_clearText(EXTRA_LIFE_TEXT_X, EXTRA_LIFE_TEXT_Y, EXTRA_LIFE_TEXT_W);
        extraLifeGranted = 0;
        extraLifeIconOn = 1;
        drawLivesIcons();
        return;
    }
    /* Only the new icon(s) blink; the "+1 NAVE" text stays solid. */
    iconOn = ((extraLifeTimer & 8) != 0) ? 1 : 0;
    if (iconOn != extraLifeIconOn) {
        extraLifeIconOn = iconOn;
        drawLivesIcons();
    }
}

void hud_hideExtraLife(void) {
    extraLifeTimer = 0;
    extraLifeGranted = 0;
    extraLifeIconOn = 1;
    VDP_clearText(EXTRA_LIFE_TEXT_X, EXTRA_LIFE_TEXT_Y, EXTRA_LIFE_TEXT_W);
}
