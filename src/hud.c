#include "hud.h"
#include "difficulty.h"

#define SHIELD_LABEL_X     29
#define SHIELD_BAR_X       32
#define SHIELD_BAR_Y       1
#define SHIELD_BAR_LEN     8
#define TILE_BAR_EMPTY     (TILE_USER_INDEX + 2)
#define TILE_BAR_BLUE1     (TILE_BAR_EMPTY + 1)   /* 1/8 .. 8/8, color 1 */
#define TILE_BAR_RED1      (TILE_BAR_BLUE1 + 8)   /* 1/8 .. 8/8, color 5 */
#define BAR_TILE_COUNT     17
#define BAR_BAND_Y0        2
#define BAR_BAND_Y1        5

static char hud_string[40] = "";
static u16 shieldState = SHIELD_HUD_READY;
static u16 shieldLevel = SHIELD_BAR_STEPS;
static u16 shieldBlink = 0;
static u16 barAttr[8];
static u16 barDrawn = 0;

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

void hud_init(void) {
    static u32 tiles[BAR_TILE_COUNT][8];
    u16 i;

    buildBarTile(tiles[0], 0, 1);
    for (i = 1; i <= 8; i++) buildBarTile(tiles[i], i, 1);
    for (i = 1; i <= 8; i++) buildBarTile(tiles[8 + i], i, 5);
    VDP_loadTileData((const u32*)tiles, TILE_BAR_EMPTY, BAR_TILE_COUNT, CPU);
    barDrawn = 0;
}

void hud_updateScore(int score, u16 enemiesLeft, u16 lives) {
    sprintf(hud_string, "SCORE: %d - LEFT: %d - SHIPS: %d", score, enemiesLeft, lives);
    VDP_clearText(0,0,40);
    VDP_drawText(hud_string, 0,0);
    VDP_clearText(0,1, SHIELD_BAR_X);
    sprintf(hud_string, "DIFF: %s", difficulty_name());
    VDP_drawText(hud_string, 0,1);
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
    VDP_clearText(0, 0, 40);
    VDP_clearText(0, 1, 40);
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
