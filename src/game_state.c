#include <genesis.h>
#include "game_state.h"
#include "difficulty.h"
#include "bullets.h"
#include "hud.h"
#include "enemies.h"
#include "powerups.h"
#include "player.h"
#include "title.h"
#include "collision.h"
#include "explosions.h"

static int score = 0;
static GameState gameState = STATE_TITLE;
static u16 menuFrames = 0;  /* ticks on every screen, used to seed the RNG on START */
static u16 wavePauseTimer = 0;

static void resetGame(void);
static void drawWaveBanner(void);

static void showEndScreen(GameState newState) {
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
static void clearEndScreen() {
    hud_clearOverlay();
}

/* Pause screen: START freezes the action and shows the game version. */
static void pauseGame() {
    char line[28];
    gameState = STATE_PAUSED;
    player_stop();
    hud_drawCentered("PAUSED", 12);
    hud_drawCentered("GENS INVADERS " GAME_VERSION, 14);
    sprintf(line, "DIFFICULTY: %s", difficulty_name());
    hud_drawCentered(line, 16);
    sprintf(line, "OLEADA %d", difficulty_wave());
    hud_drawCentered(line, 17);
    hud_drawCentered("PRESS START", 18);
}

static void resumeGame() {
    clearEndScreen();
    gameState = STATE_PLAYING;
    if (wavePauseTimer > 0) drawWaveBanner();
}

/* Hide every game sprite and reset bullet counters. Used by the title screen. */
static void hideGameSprites() {
    player_hide();
    enemies_hideAll();
    bullets_hideAll();
    powerups_hide();
    explosions_hide();
}

void game_state_showTitle() {
    gameState = STATE_TITLE;
    player_get()->velx = 0;
    wavePauseTimer = 0;
    hideGameSprites();
    title_show();
}

static void startGameFromMenu() {
    /* Re-seed at the moment START begins the match. SGDK only auto-seeds on the
     * first button of the session, often a menu tap right after boot, so later
     * matches (and quick starts) reused a similar sequence. Mix a title-screen
     * frame counter with the vblank timer and getTick(). */
    u16 seed = menuFrames ^ (u16)vtimer ^ (u16)getTick() ^ (u16)(getTick() >> 8);
    setRandomSeed(seed);
    title_clear();
    resetGame();
}

/* Handler function for the joystick movement, Interrupts are already implemented in SGDK */
void game_state_joy(u16 joy, u16 changed, u16 state) {
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
        if (!player_isExploding()) pauseGame();
        return;
    }

    // On the end screens only START matters: it returns to the title screen.
    if (gameState != STATE_PLAYING) {
        if (state & BUTTON_START & changed) {
            game_state_showTitle();
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
    if (state & BUTTON_C & changed) {
        powerups_tryShield();
    }
}

/* Restart after confirming a difficulty: reuse the sprites already allocated,
 * just reset positions, counters and flags. */
static void drawWaveBanner(void) {
    char line[20];
    sprintf(line, "OLEADA %d", difficulty_wave());
    hud_drawCentered(line, 12);
}

static void startNextWave(void) {
    /* Enemy shots vanish; player shots, explosions, score, lives and the
     * shield / RapidFire state are kept. */
    bullets_clearEnemy();
    difficulty_nextWave();
    wavePauseTimer = IS_PAL_SYSTEM ? WAVE_BANNER_FRAMES_PAL : WAVE_BANNER_FRAMES_NTSC;
    clearEndScreen();
    drawWaveBanner();
    hud_updateScore(score, enemies_left(), player_lives());
}

static void finishWavePause(void) {
    enemies_reset();
    clearEndScreen();
    hud_updateScore(score, enemies_left(), player_lives());
}

static void resetGame() {
    bullets_reset();
    powerups_reset();

    player_reset();

    difficulty_beginMatch();
    wavePauseTimer = 0;
    enemies_reset();
    explosions_reset();
    score = 0;
    clearEndScreen();
    hud_updateScore(score, enemies_left(), player_lives());
    gameState = STATE_PLAYING;
}

void game_state_update(void) {
    menuFrames++;
    if (gameState == STATE_TITLE) {
        title_updatePulse();
    } else if (gameState == STATE_PLAYING) {
        if (player_update()) {
            wavePauseTimer = 0;
            showEndScreen(STATE_GAME_OVER);
        }
        bullets_update();
        if (wavePauseTimer > 0) {
            /* Brief intermission: no formation or enemy fire, banner up.
             * The player can still move and shoot; the shield keeps ticking. */
            wavePauseTimer--;
            if (wavePauseTimer == 0) finishWavePause();
        } else {
            if (enemies_update()) {
                showEndScreen(STATE_GAME_OVER);
            }
            {
                u16 hit = collision_update(&score);
                if (hit == COLLISION_WIN) {
                    startNextWave();
                } else if (hit == COLLISION_GAME_OVER) {
                    showEndScreen(STATE_GAME_OVER);
                }
            }
        }
        powerups_update();
        explosions_update();
    } else if (gameState == STATE_GAME_OVER || gameState == STATE_WIN) {
        /* Let a burst finish under the end text (last kill or invasion).
         * Pause freezes the animation, like the rest of the timers. */
        explosions_update();
    }
}

