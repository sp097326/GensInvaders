#include <genesis.h>
#include "title.h"
#include "hud.h"
#include "difficulty.h"
#include "game.h"
#include "scores.h"

#define TILE_TITLE_FILL   TILE_USER_INDEX
#define TILE_TITLE_SHADOW (TILE_USER_INDEX + 1)

static u16 titleCursor = TITLE_OPT_START;
static u16 titlePulseTick = 0;
static u16 titleScoresView = 0;

/* Solid 8x8 block tiles for the title logo (palette index 1 = fill, 2 = shadow).
 * Drawn on plane A so a wide word like INVADERS does not hit the 20-sprites-per-
 * scanline limit. The starfield stays on plane B. */
static const u32 TITLE_TILE_FILL[8] = {
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
    0x11111111, 0x11111111, 0x11111111, 0x11111111
};
static const u32 TITLE_TILE_SHADOW[8] = {
    0x22222222, 0x22222222, 0x22222222, 0x22222222,
    0x22222222, 0x22222222, 0x22222222, 0x22222222
};

/* 5x5 glyphs for the short top word GENS (bit 4 = leftmost). */
static const u8 GLYPH5_G[5] = { 0x0E, 0x10, 0x16, 0x11, 0x0E };
static const u8 GLYPH5_E[5] = { 0x1F, 0x10, 0x1E, 0x10, 0x1F };
static const u8 GLYPH5_N[5] = { 0x11, 0x19, 0x15, 0x13, 0x11 };
static const u8 GLYPH5_S[5] = { 0x0F, 0x10, 0x0E, 0x01, 0x1E };

/* 4x5 glyphs for INVADERS (bit 3 = leftmost). */
static const u8 GLYPH4_A[5] = { 0x6, 0x9, 0xF, 0x9, 0x9 };
static const u8 GLYPH4_D[5] = { 0xE, 0x9, 0x9, 0x9, 0xE };
static const u8 GLYPH4_E[5] = { 0xF, 0x8, 0xE, 0x8, 0xF };
static const u8 GLYPH4_I[5] = { 0xF, 0x6, 0x6, 0x6, 0xF };
static const u8 GLYPH4_N[5] = { 0x9, 0xD, 0xB, 0x9, 0x9 };
static const u8 GLYPH4_R[5] = { 0xE, 0x9, 0xE, 0xA, 0x9 };
static const u8 GLYPH4_S[5] = { 0xE, 0x8, 0x6, 0x1, 0x7 };
static const u8 GLYPH4_V[5] = { 0x9, 0x9, 0x9, 0x9, 0x6 };

static const u8* titleGlyph(char c, u16 width) {
    if (width == 5) {
        switch (c) {
            case 'G': return GLYPH5_G;
            case 'E': return GLYPH5_E;
            case 'N': return GLYPH5_N;
            case 'S': return GLYPH5_S;
            default:  return NULL;
        }
    }
    switch (c) {
        case 'A': return GLYPH4_A;
        case 'D': return GLYPH4_D;
        case 'E': return GLYPH4_E;
        case 'I': return GLYPH4_I;
        case 'N': return GLYPH4_N;
        case 'R': return GLYPH4_R;
        case 'S': return GLYPH4_S;
        case 'V': return GLYPH4_V;
        default:  return NULL;
    }
}

static void loadTitleTiles() {
    VDP_loadTileData(TITLE_TILE_FILL, TILE_TITLE_FILL, 1, CPU);
    VDP_loadTileData(TITLE_TILE_SHADOW, TILE_TITLE_SHADOW, 1, CPU);
}

static void setTitlePalette() {
    /* PAL3 is reserved for the logo: color 1 pulses, color 2 is the shadow. */
    PAL_setColor(49, RGB24_TO_VDPCOLOR(0xF8F800));
    PAL_setColor(50, RGB24_TO_VDPCOLOR(0x000068));
}

static void stampGlyph(const u8* rows, u16 width, u16 x, u16 y, u16 tileIndex) {
    u16 r, b, bit;
    u16 attr = TILE_ATTR_FULL(PAL3, 1, 0, 0, tileIndex);
    for (r = 0; r < 5; r++) {
        for (b = 0; b < width; b++) {
            bit = width - 1 - b;
            if (rows[r] & (1 << bit)) {
                VDP_setTileMapXY(BG_A, attr, x + b, y + r);
            }
        }
    }
}

/* Draw a word in solid blocks, with a 1-tile drop shadow, then the fill on top. */
static void drawTitleWord(const char* word, u16 x, u16 y, u16 width) {
    u16 i;
    u16 col;
    const u8* g;
    u16 step = width + 1;

    col = x;
    for (i = 0; word[i] != 0; i++) {
        g = titleGlyph(word[i], width);
        if (g != NULL) {
            stampGlyph(g, width, col + 1, y + 1, TILE_TITLE_SHADOW);
        }
        col += step;
    }
    col = x;
    for (i = 0; word[i] != 0; i++) {
        g = titleGlyph(word[i], width);
        if (g != NULL) {
            stampGlyph(g, width, col, y, TILE_TITLE_FILL);
        }
        col += step;
    }
}

static void drawTitleLogo() {
    /* Two-row logo so 12 letters stay large but fit in 320 px:
     * GENS is 5x5 (23 tiles, centered), INVADERS is 4x5 (39 tiles, full width). */
    drawTitleWord("GENS", 8, 2, 5);
    drawTitleWord("INVADERS", 0, 8, 4);
}

void title_clear() {
    u16 row;
    VDP_clearTileMapRect(BG_A, 0, 0, 40, 28);
    for (row = 0; row < 28; row++) {
        VDP_clearText(0, row, 40);
    }
}

void title_drawMenu() {
    char line[32];
    u16 row;
    titleScoresView = 0;
    for (row = 15; row <= 23; row++) VDP_clearText(0, row, 40);
    hud_drawCentered(GAME_VERSION, 15);
    sprintf(line, "%c START GAME (INICIAR)", (titleCursor == TITLE_OPT_START) ? '>' : ' ');
    hud_drawCentered(line, 17);
    sprintf(line, "%c DIFFICULTY: %s", (titleCursor == TITLE_OPT_DIFF) ? '>' : ' ', difficulty_name());
    hud_drawCentered(line, 19);
    sprintf(line, "%c PUNTAJES (TOP 5)", (titleCursor == TITLE_OPT_SCORES) ? '>' : ' ');
    hud_drawCentered(line, 21);
}

void title_cycleDifficulty(s16 dir) {
    difficulty_cycle(dir);
    title_drawMenu();
}

void title_updatePulse() {
    /* Gold / orange ping-pong so the block logo feels alive. */
    static const u16 colors[8] = {
        RGB24_TO_VDPCOLOR(0xF8F800),
        RGB24_TO_VDPCOLOR(0xF8D000),
        RGB24_TO_VDPCOLOR(0xF8A000),
        RGB24_TO_VDPCOLOR(0xF87800),
        RGB24_TO_VDPCOLOR(0xF85000),
        RGB24_TO_VDPCOLOR(0xF87800),
        RGB24_TO_VDPCOLOR(0xF8A000),
        RGB24_TO_VDPCOLOR(0xF8D000)
    };
    titlePulseTick++;
    if ((titlePulseTick & 7) == 0) {
        PAL_setColor(49, colors[(titlePulseTick >> 3) & 7]);
    }
}

void title_showScores(void) {
    u16 row;
    titleScoresView = 1;
    for (row = 15; row <= 23; row++) VDP_clearText(0, row, 40);
    hud_drawCentered("TOP 5", 15);
    scores_drawTable(16, -1);
    hud_drawCentered("PRESS START", 22);
}

void title_hideScores(void) {
    title_drawMenu();
}

u16 title_isScoresView(void) {
    return titleScoresView;
}

void title_show(void) {
    titleCursor = TITLE_OPT_START;
    titleScoresView = 0;
    title_clear();
    loadTitleTiles();
    setTitlePalette();
    drawTitleLogo();
    title_drawMenu();
}

u16 title_cursor(void) {
    return titleCursor;
}

void title_setCursor(u16 cursor) {
    titleCursor = cursor;
}
