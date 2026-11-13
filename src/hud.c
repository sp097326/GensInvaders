#include "hud.h"
#include "difficulty.h"
#include "config.h"

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

/* Nine 8x8 ship icons on the right of row 0 (cols 31-39), left-aligned
 * so a new life appears to the right of the ones already there. SCORE
 * and LEFT stay on the left; even "SCORE: 999999  LEFT: 14" ends at
 * column 24, leaving a gap before the icons. */
#define LIVES_ICON_X       31
#define LIVES_ICON_Y       0
#define LIVES_ICON_SLOTS   LIVES_CAP

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

    /* Leave columns 31-39 alone for the ship icons. */
    sprintf(hud_string, "SCORE: %d  LEFT: %d", score, enemiesLeft);
    hud_string[LIVES_ICON_X] = 0;
    VDP_clearText(0, 0, LIVES_ICON_X);
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
}

void hud_updateShieldBar(u16 state, u16 level, u16 blink) {
    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    shieldState = state;
    shieldLevel = level;
    shieldBlink = blink;
    drawShieldBar(0);
}

void hud_clear(void) {
    extraLifeTimer = 0;
    extraLifeGranted = 0;
    extraLifeIconOn = 1;
    hudLives = 0;
    VDP_clearText(0, 0, 40);
    VDP_clearText(0, 1, 40);
    VDP_clearText(0, 2, 40);
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
    /* Draw the banner once and leave it. Clearing and redrawing every
     * frame during active display caused flickering horizontal lines
     * and made the text almost unreadable. */
    VDP_clearText(0, 2, 40);
    hud_drawCentered(extraLifeText, 2);
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
        VDP_clearText(0, 2, 40);
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
    VDP_clearText(0, 2, 40);
}
