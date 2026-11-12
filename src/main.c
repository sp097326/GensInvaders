/**
 * Gens Invaders
 * Created With Genesis-Code extension for Visual Studio Code
 * Based on ohsat (https://www.ohsat.com/) awesome tutorial.
 * Assets by: taken from ohsat page,
 * Code by: AMH
 * Last mod by: AMH (with Bimo) - 12 / 11 / 2026
 *
 * Change history lives in CHANGELOG.md; design notes in README.MD.
 **/
#include <genesis.h>
#include <resources.h>
#include "config.h"
#include "game.h"
#include "difficulty.h"
#include "bullets.h"
#include "hud.h"
#include "enemies.h"
#include "powerups.h"

#define TITLE_OPT_START 0
#define TITLE_OPT_DIFF  1
#define TILE_TITLE_FILL   TILE_USER_INDEX
#define TILE_TITLE_SHADOW (TILE_USER_INDEX + 1)

// on screen entities
// name[6] holds 5 chars + the terminating '\0', so "PLAYER" (6 chars) didn't fit.
GameEntity player = {0, 0, 16, 16, 0, 0, 0, NULL, 0, 0, "PLYR" };
int score = 0;
GameState gameState = STATE_TITLE;
u16 titleCursor = TITLE_OPT_START;
u16 titlePulseTick = 0;
u16 lives = MAX_LIVES;
u16 respawnTimer = 0;   // > 0 while the player is waiting to come back
u16 invulnTimer = 0;    // > 0 while the player is blinking and can't be hit

// background scroll offset
int offset = 0;

/* Function prototypes
 * Some of these are called before they are defined further down the file.
 * Declaring the prototypes here tells the compiler their signatures in advance. */
void resetGame();
void showTitleScreen();
void drawTitleMenu();
void startGameFromMenu();
void updateTitlePulse();

/* Auxiliary functions for game processing */

/* Game state helpers ------------------------------------------------------ */

void showEndScreen(GameState newState) {
    gameState = newState;
    player.velx = 0;
    if (newState == STATE_WIN) {
        hud_drawCentered("YOU WIN!", 12);
    } else {
        hud_drawCentered("GAME OVER", 12);
    }
    hud_drawCentered("PRESS START", 14);
}

/* Clears the overlay rows used by the menu, pause and end screens. */
void clearEndScreen() {
    hud_clearOverlay();
}

/* Pause screen: START freezes the action and shows the game version. */
void pauseGame() {
    char line[28];
    gameState = STATE_PAUSED;
    player.velx = 0;
    SPR_setAnim(player.sprite, ANIM_STRAIGHT);
    hud_drawCentered("PAUSED", 12);
    hud_drawCentered("GENS INVADERS " GAME_VERSION, 14);
    sprintf(line, "DIFFICULTY: %s", difficulty_name());
    hud_drawCentered(line, 16);
    hud_drawCentered("PRESS START", 18);
}

void resumeGame() {
    clearEndScreen();
    gameState = STATE_PLAYING;
}

/* Hide every game sprite and reset bullet counters. Used by the title screen. */
void hideGameSprites() {
    killGameEntity(&player);
    enemies_hideAll();
    bullets_hideAll();
}

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

void loadTitleTiles() {
    VDP_loadTileData(TITLE_TILE_FILL, TILE_TITLE_FILL, 1, CPU);
    VDP_loadTileData(TITLE_TILE_SHADOW, TILE_TITLE_SHADOW, 1, CPU);
}

void setTitlePalette() {
    /* PAL3 is reserved for the logo: color 1 pulses, color 2 is the shadow. */
    PAL_setColor(49, RGB24_TO_VDPCOLOR(0xF8F800));
    PAL_setColor(50, RGB24_TO_VDPCOLOR(0x000068));
}

void stampGlyph(const u8* rows, u16 width, u16 x, u16 y, u16 tileIndex) {
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
void drawTitleWord(const char* word, u16 x, u16 y, u16 width) {
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

void drawTitleLogo() {
    /* Two-row logo so 12 letters stay large but fit in 320 px:
     * GENS is 5x5 (23 tiles, centered), INVADERS is 4x5 (39 tiles, full width). */
    drawTitleWord("GENS", 8, 2, 5);
    drawTitleWord("INVADERS", 0, 8, 4);
}

void clearTitleScreen() {
    u16 row;
    VDP_clearTileMapRect(BG_A, 0, 0, 40, 28);
    for (row = 0; row < 28; row++) {
        VDP_clearText(0, row, 40);
    }
}

void drawTitleMenu() {
    char line[32];
    VDP_clearText(0, 15, 40);
    VDP_clearText(0, 17, 40);
    VDP_clearText(0, 19, 40);
    hud_drawCentered(GAME_VERSION, 15);
    sprintf(line, "%c START GAME (INICIAR)", (titleCursor == TITLE_OPT_START) ? '>' : ' ');
    hud_drawCentered(line, 17);
    sprintf(line, "%c DIFFICULTY: %s", (titleCursor == TITLE_OPT_DIFF) ? '>' : ' ', difficulty_name());
    hud_drawCentered(line, 19);
}

void cycleDifficulty(s16 dir) {
    difficulty_cycle(dir);
    drawTitleMenu();
}

void updateTitlePulse() {
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

void showTitleScreen() {
    gameState = STATE_TITLE;
    titleCursor = TITLE_OPT_START;
    player.velx = 0;
    hideGameSprites();
    clearTitleScreen();
    loadTitleTiles();
    setTitlePalette();
    drawTitleLogo();
    drawTitleMenu();
}

void startGameFromMenu() {
    clearTitleScreen();
    resetGame();
}

void positionPlayer() {
    if (player.health == 0) return;   // waiting to respawn
    //handling velocity, adding it to position
    player.x += player.velx;

    /* checking to keep the player in the boundaries of the screen */
    if (player.x < LEFT_EDGE) player.x = LEFT_EDGE;
    if (player.x + player.w > RIGHT_EDGE) player.x = RIGHT_EDGE - player.w;

    //positioning the sprite using the Sprite Engine in SGDK
    SPR_setPosition(player.sprite, player.x, player.y);
}

/* Puts the ship back at its starting spot, facing straight. */
void placePlayer() {
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.velx = 0;
    SPR_setAnim(player.sprite, ANIM_STRAIGHT);
    SPR_setHFlip(player.sprite, FALSE);
    SPR_setPosition(player.sprite, player.x, player.y);
}

/* Called once per frame: brings the ship back after RESPAWN_DELAY frames and
 * makes it blink (and immune) for INVULN_FRAMES so it isn't killed on arrival. */
void updatePlayerLife() {
    if (respawnTimer > 0) {
        respawnTimer--;
        if (respawnTimer == 0) {
            placePlayer();
            reviveGameEntity(&player);
            invulnTimer = INVULN_FRAMES;
        }
        return;
    }
    if (invulnTimer > 0) {
        invulnTimer--;
        // Toggle visibility every 4 frames for the blink effect.
        SPR_setVisibility(player.sprite, ((invulnTimer & 4) && invulnTimer > 0) ? HIDDEN : VISIBLE);
    }
}

/* The ship was hit: lose a life, then either respawn or end the game. */
void playerHit() {
    killGameEntity(&player);
    player.velx = 0;
    if (powerups_isOn()) powerups_deactivate();
    //present the xplosion sprite instead....
    // Own channel (CH2) and top priority (15) so nothing can cut the explosion.
    XGM_startPlayPCM(SFX_XPLOSION,15,SOUND_PCM_CH2);
    lives--;
    hud_updateScore(score, enemies_left(), lives);
    if (lives == 0) {
        showEndScreen(STATE_GAME_OVER);
    } else {
        respawnTimer = RESPAWN_DELAY;
    }
}

/* Handler function for the joystick movement, Interrupts are already implemented in SGDK */
void myJoyHandler(u16 joy, u16 changed, u16 state) {
    // Control movement for player 1 (can we just add an If for player 2?)
    if (joy != JOY_1) return;

    if (gameState == STATE_TITLE) {
        if (state & BUTTON_UP & changed) {
            if (titleCursor > TITLE_OPT_START) {
                titleCursor = TITLE_OPT_START;
                drawTitleMenu();
            }
        } else if (state & BUTTON_DOWN & changed) {
            if (titleCursor < TITLE_OPT_DIFF) {
                titleCursor = TITLE_OPT_DIFF;
                drawTitleMenu();
            }
        }
        if (titleCursor == TITLE_OPT_DIFF) {
            if (state & BUTTON_LEFT & changed) {
                cycleDifficulty(-1);
            } else if (state & BUTTON_RIGHT & changed) {
                cycleDifficulty(1);
            } else if (state & BUTTON_A & changed) {
                cycleDifficulty(1);
            }
        }
        if (state & BUTTON_START & changed) {
            if (titleCursor == TITLE_OPT_START) {
                startGameFromMenu();
            } else {
                cycleDifficulty(1);
            }
        }
        return;
    }

    // START pauses / resumes while playing.
    if (gameState == STATE_PAUSED) {
        if (state & BUTTON_START & changed) {
            resumeGame();
        }
        return;
    }
    if (gameState == STATE_PLAYING && (state & BUTTON_START & changed)) {
        pauseGame();
        return;
    }

    // On the end screens only START matters: it returns to the title screen.
    if (gameState != STATE_PLAYING) {
        if (state & BUTTON_START & changed) {
            showTitleScreen();
        }
        return;
    }

    // No control while the ship is gone (waiting to respawn).
    if (player.health == 0) return;

    if (state & BUTTON_RIGHT) {
        player.velx = 2;
        SPR_setAnim(player.sprite, ANIM_MOVE);
        SPR_setHFlip(player.sprite, TRUE);
    }
    else if (state & BUTTON_LEFT) {
        player.velx = -2;
        SPR_setAnim(player.sprite, ANIM_MOVE);
        SPR_setHFlip(player.sprite, FALSE);
    }
    else {
        if ( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ) {
            player.velx = 0;
            SPR_setAnim(player.sprite, ANIM_STRAIGHT);
        }
    }
    if (state & BUTTON_B & changed) {
        bullets_shootPlayer(player);
    }
    if (state & BUTTON_A & changed) {
        powerups_toggle();
    }
}

int collideEntities(GameEntity* a, GameEntity* b) {
    return (a->x < b->x + b->w &&
    a->x + a->w > b->x &&
    a->y < b->y + b->h &&
    a->y + a->h >= b->y);
}

void handleCollisions() {
    GameEntity* b;
    GameEntity* e;
    int i = 0;
    int j = 0;

    /* Player bullets vs enemies. bullets[] is now the player-only pool. */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        b = bullets_playerAt(i);
        if (b->health > 0) {
            for (j = 0; j < MAX_ENEMIES; j++)
            {
                e = enemies_at(j);
                if (e->health > 0) {
                    if (collideEntities(b,e)) {
                        killGameEntity(e);
                        bullets_despawnPlayer(b);
                        enemies_decLeft();
                        score += e->points;
                        hud_updateScore(score, enemies_left(), lives);
                        if (enemies_left() == 0) {
                            showEndScreen(STATE_WIN);
                            return;
                        }
                        break;
                    }
                }
            }
        }
    }

    /* Each enemy bullet has its own collision against the player. */
    for (i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        b = bullets_enemyAt(i);
        if (b->health > 0 && player.health > 0) {
            if (invulnTimer == 0 && collideEntities(b, &player)) {
                bullets_despawnEnemy(b);
                playerHit();
                return;
            }
        }
    }
}

/* Restart after confirming a difficulty: reuse the sprites already allocated,
 * just reset positions, counters and flags. */
void resetGame() {
    bullets_reset();
    powerups_reset();

    placePlayer();
    reviveGameEntity(&player);
    lives = MAX_LIVES;
    respawnTimer = 0;
    invulnTimer = 0;

    enemies_reset();
    score = 0;
    clearEndScreen();
    hud_updateScore(score, enemies_left(), lives);
    gameState = STATE_PLAYING;
}

/* Starfield: fills plane B ONCE with random tiles from space.bmp (tiles 1..3,
 * loaded at VRAM index 1). The old code did this inside the main loop, i.e.
 * 1280 tile writes every frame, which is why it was too slow and got commented
 * out. Drawing it once and only moving the vertical scroll is enough: the plane
 * wraps around, so the stars scroll forever for free. */
void drawStarfield() {
    u16 x, y, val;
    for (y = 0; y < BG_TILES_H; y++) {
        for (x = 0; x < BG_TILES_W; x++) {
            val = (random() % 10) + 1;     // 1..10
            if (val > 3) val = 1;          // ~80% tile 1 (empty space), rest are stars
            VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1, 0, 0, 0, val), x, y);
        }
    }
}

/* END Auxiliary functions */
//---------------------------------------------------------------------------------------------------------------
// Main method by flow definition.
int main(bool hard)
{
    JOY_init();
    JOY_setEventHandler(&myJoyHandler);
    difficulty_init();

    //can I place here an initial sound Pac-Man style?
    XGM_setPCM(INTRO_MUSIC,intro, sizeof(intro));
    XGM_startPlayPCM(INTRO_MUSIC,12,SOUND_PCM_CH1);
    //handle sound
    XGM_setPCM(SFX_LASER, laser, sizeof(laser));
    XGM_setPCM(SFX_XPLOSION, xplosion, sizeof(xplosion));
    //note to this: read the url for the effects.
    SYS_disableInts();
    VDP_loadTileSet(background.tileset,1,DMA);
    PAL_setPalette(PAL1, background.palette->data, DMA); //changed from VDP_setPalette
    PAL_setPalette(PAL2, background.palette->data, DMA); //changed from VDP_setPalette
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    drawStarfield();
    SPR_init();

    /* adding the player to the screen, can this be moved to a function?*/
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.health = 1;
    player.sprite = SPR_addSprite(&ship, player.x, player.y, TILE_ATTR(PAL1, 0, FALSE, FALSE));

    enemies_init();
    bullets_init();
    powerups_init();
    SYS_enableInts();
    /* Intro is already playing; show the title screen before the first match. */
    showTitleScreen();
    SPR_update();

    /* Actual main loop for the game, this loops while the game is alive */
    while(1)
    {
        // The starfield keeps scrolling on every screen, even GAME OVER.
        VDP_setVerticalScroll(BG_B,offset -= 2);
        if (offset <= -256) offset = 0;

        // Old idea: random starfield tiles every frame. Removed for performance;
        // pregenerating the values once would be the way to bring it back.

        if (gameState == STATE_TITLE) {
            updateTitlePulse();
        } else if (gameState == STATE_PLAYING) {
            updatePlayerLife();
            positionPlayer();
            bullets_update();
            if (enemies_update()) {
                showEndScreen(STATE_GAME_OVER);
            }
            handleCollisions();
            powerups_update();
        }
        SPR_update();
        SYS_doVBlankProcess();
    }

    return (0);
}
