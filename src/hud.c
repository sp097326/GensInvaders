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
#define EXTRA_LIFE_TEXT_W  18  /* stops short of the RF bar at col 29 */
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
static u16 bossBarActive = 0;
static u16 bossBarLevel = 0;
static u16 bossBarAttr[16];
static u16 bossBarDrawn = 0;

static void drawBossBar(u16 force);

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
    hudLives = (lives > LIVES_CAP) ? LIVES_CAP : lives;

    if (boss_isActive()) {
        sprintf(hud_string, "SCORE: %d  JEFE", score);
    } else {
        sprintf(hud_string, "SCORE: %d  LEFT: %d", score, enemiesLeft);
    }
    VDP_clearText(0, 0, 40);
    VDP_drawText(hud_string, 0, 0);
    drawLivesIcons();
    VDP_clearText(0, 1, SHIELD_BAR_X);
    sprintf(hud_string, "DIFF: %s", difficulty_name());
    VDP_drawText(hud_string, 0, 1);
    /* Col 16 leaves a gap after "DIFF: NORMAL" (12 chars) and stays
     * clear of the ESC bar at col 29, even with "OLEADA 999". */
    sprintf(hud_string, "OLEADA %d", difficulty_wave());
    VDP_drawText(hud_string, 16, 1);
    barDrawn = 0;
    drawShieldBar(1);
    /* Score/wave text only touches rows 0-1; force the JEFE bar on
     * row 2 so a later score refresh cannot leave it stale. */
    if (boss_isActive() && bossBarActive) {
        drawBossBar(1);
    }
}

void hud_updateShieldBar(u16 state, u16 level, u16 blink) {
    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    shieldState = state;
    shieldLevel = level;
    shieldBlink = blink;
    drawShieldBar(0);
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
    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    rapidActive = (active && level > 0) ? 1 : 0;
    rapidLevel = level;
    drawRapidBar(0);
}

void hud_hideRapidBar(void) {
    rapidActive = 0;
    rapidLevel = 0;
    drawRapidBar(1);
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
            drawBossBar(1);
        }
        return;
    }
    level = (u16)(((u32)hp * BOSS_BAR_STEPS + (u32)hpMax - 1) / (u32)hpMax);
    if (level > BOSS_BAR_STEPS) level = BOSS_BAR_STEPS;
    if (level < 1) level = 1;
    if (bossBarActive && bossBarDrawn && bossBarLevel == level) return;
    bossBarActive = 1;
    bossBarLevel = level;
    drawBossBar(1);
}

void hud_hideBossBar(void) {
    bossBarActive = 0;
    bossBarLevel = 0;
    drawBossBar(1);
}

void hud_clear(void) {
    extraLifeTimer = 0;
    extraLifeGranted = 0;
    extraLifeIconOn = 1;
    hudLives = 0;
    rapidActive = 0;
    rapidLevel = 0;
    rapidBarDrawn = 0;
    bossBarActive = 0;
    bossBarLevel = 0;
    bossBarDrawn = 0;
    VDP_clearText(0, 0, 40);
    VDP_clearText(0, 1, 40);
    VDP_clearText(0, BOSS_BAR_Y, 40);
    VDP_clearText(0, LIVES_ICON_Y, 40);
    barDrawn = 0;
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
