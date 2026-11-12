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
#include "player.h"
#include "title.h"
#include "collision.h"

int score = 0;
GameState gameState = STATE_TITLE;

// background scroll offset
int offset = 0;

/* Function prototypes
 * Some of these are called before they are defined further down the file.
 * Declaring the prototypes here tells the compiler their signatures in advance. */
void resetGame();
void showTitleScreen();
void startGameFromMenu();

/* Auxiliary functions for game processing */

/* Game state helpers ------------------------------------------------------ */

void showEndScreen(GameState newState) {
    gameState = newState;
    player_get()->velx = 0;
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
    player_stop();
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
    player_hide();
    enemies_hideAll();
    bullets_hideAll();
}

void showTitleScreen() {
    gameState = STATE_TITLE;
    player_get()->velx = 0;
    hideGameSprites();
    title_show();
}

void startGameFromMenu() {
    title_clear();
    resetGame();
}

/* Handler function for the joystick movement, Interrupts are already implemented in SGDK */
void myJoyHandler(u16 joy, u16 changed, u16 state) {
    // Control movement for player 1 (can we just add an If for player 2?)
    if (joy != JOY_1) return;

    if (gameState == STATE_TITLE) {
        if (state & BUTTON_UP & changed) {
            if (title_cursor() > TITLE_OPT_START) {
                title_setCursor(TITLE_OPT_START);
                title_drawMenu();
            }
        } else if (state & BUTTON_DOWN & changed) {
            if (title_cursor() < TITLE_OPT_DIFF) {
                title_setCursor(TITLE_OPT_DIFF);
                title_drawMenu();
            }
        }
        if (title_cursor() == TITLE_OPT_DIFF) {
            if (state & BUTTON_LEFT & changed) {
                title_cycleDifficulty(-1);
            } else if (state & BUTTON_RIGHT & changed) {
                title_cycleDifficulty(1);
            } else if (state & BUTTON_A & changed) {
                title_cycleDifficulty(1);
            }
        }
        if (state & BUTTON_START & changed) {
            if (title_cursor() == TITLE_OPT_START) {
                startGameFromMenu();
            } else {
                title_cycleDifficulty(1);
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
    if (player_get()->health == 0) return;

    if (state & BUTTON_RIGHT) {
        player_get()->velx = 2;
        SPR_setAnim(player_get()->sprite, ANIM_MOVE);
        SPR_setHFlip(player_get()->sprite, TRUE);
    }
    else if (state & BUTTON_LEFT) {
        player_get()->velx = -2;
        SPR_setAnim(player_get()->sprite, ANIM_MOVE);
        SPR_setHFlip(player_get()->sprite, FALSE);
    }
    else {
        if ( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ) {
            player_get()->velx = 0;
            SPR_setAnim(player_get()->sprite, ANIM_STRAIGHT);
        }
    }
    if (state & BUTTON_B & changed) {
        bullets_shootPlayer(*player_get());
    }
    if (state & BUTTON_A & changed) {
        powerups_toggle();
    }
}

/* Restart after confirming a difficulty: reuse the sprites already allocated,
 * just reset positions, counters and flags. */
void resetGame() {
    bullets_reset();
    powerups_reset();

    player_reset();

    enemies_reset();
    score = 0;
    clearEndScreen();
    hud_updateScore(score, enemies_left(), player_lives());
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

    player_init();

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
            title_updatePulse();
        } else if (gameState == STATE_PLAYING) {
            player_update();
            bullets_update();
            if (enemies_update()) {
                showEndScreen(STATE_GAME_OVER);
            }
            {
                u16 hit = collision_update(&score);
                if (hit == COLLISION_WIN) {
                    showEndScreen(STATE_WIN);
                } else if (hit == COLLISION_GAME_OVER) {
                    showEndScreen(STATE_GAME_OVER);
                }
            }
            powerups_update();
        }
        SPR_update();
        SYS_doVBlankProcess();
    }

    return (0);
}
