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
#include "scores.h"

static int score = 0;
static GameState gameState = STATE_TITLE;
static u16 menuFrames = 0;  /* ticks on every screen, used to seed the RNG on START */
static u16 wavePauseTimer = 0;
static u16 warpTotal = 0;
static u16 warpAccel = 0;
static u16 warpArrive = 0;
static s16 starSpeed = STAR_SPEED_NORMAL;
static s16 scoreHighlight = -1;
static u16 scoreBlink = 0;

static void resetGame(void);
static void drawWaveBanner(void);
static void clearEndScreen(void);
static void drawGameOverTable(void);
static void stopWaveWarp(void);
static void updateWaveWarp(void);

s16 game_state_starSpeed(void) {
    return starSpeed;
}

static void stopWaveWarp(void) {
    wavePauseTimer = 0;
    starSpeed = STAR_SPEED_NORMAL;
    player_setWarpFlame(0);
    if (player_get()->health && !player_isExploding()) {
        player_get()->y = PLAYER_START_Y;
    }
}

static void drawGameOverTable(void) {
    clearEndScreen();
    hud_drawCentered("GAME OVER", 10);
    hud_drawCentered("TOP 5", 11);
    scores_drawTable(12, scoreHighlight);
    hud_drawCentered("PRESS START", 18);
}

static void showEndScreen(GameState newState) {
    gameState = newState;
    player_get()->velx = 0;
    stopWaveWarp();
    powerups_muteShieldSound();
    hud_hideExtraLife();
    if (newState == STATE_WIN) {
        clearEndScreen();
        hud_drawCentered("YOU WIN!", 12);
        hud_drawCentered("PRESS START", 14);
        return;
    }
    /* Qualifying scores go through the 3-letter entry first. */
    if (scores_qualifies((u32)score)) {
        gameState = STATE_NAME_ENTRY;
        scores_entryStart((u32)score, difficulty_wave(), (u8)difficulty_current());
        clearEndScreen();
        scores_entryDraw();
    } else {
        scoreHighlight = -1;
        gameState = STATE_GAME_OVER;
        drawGameOverTable();
    }
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
    powerups_muteShieldSound();
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
    powerups_unmuteShieldSound();
    player_chargeSyncButton(JOY_readJoypad(JOY_1) & BUTTON_B);
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
    stopWaveWarp();
    hud_hideExtraLife();
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
        if (title_isScoresView()) {
            if ((state & BUTTON_START & changed) || (state & BUTTON_B & changed)) {
                title_hideScores();
            }
            return;
        }
        if (state & BUTTON_UP & changed) {
            if (title_cursor() > 0) {
                title_setCursor(title_cursor() - 1);
                title_drawMenu();
            }
        } else if (state & BUTTON_DOWN & changed) {
            if (title_cursor() + 1 < TITLE_OPT_COUNT) {
                title_setCursor(title_cursor() + 1);
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
        if (title_cursor() == TITLE_OPT_SCORES) {
            if ((state & BUTTON_START & changed) || (state & BUTTON_A & changed)) {
                title_showScores();
            }
            return;
        }
        if (state & BUTTON_START & changed) {
            if (title_cursor() == TITLE_OPT_START) {
                startGameFromMenu();
            } else if (title_cursor() == TITLE_OPT_DIFF) {
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

    if (gameState == STATE_NAME_ENTRY) {
        if (scores_entryJoy(changed, state)) {
            scoreHighlight = scores_entryRank();
            gameState = STATE_GAME_OVER;
            scoreBlink = 0;
            drawGameOverTable();
        }
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
    /* During the between-wave warp the player can still steer, but
     * shots and a fresh shield would break the empty-space beat. An
     * already-active shield keeps ticking and follows the ship. */
    if (wavePauseTimer == 0) {
        /* Fire on release: a short tap is the same 8x8 shot as before,
         * a ~1 s hold fires the piercing charged bolt. */
        if (changed & BUTTON_B) {
            if (state & BUTTON_B) {
                player_chargeBegin();
            } else {
                player_chargeRelease();
            }
        }
        if (state & BUTTON_A & changed) {
            powerups_toggle();
        }
        if (state & BUTTON_C & changed) {
            powerups_tryShield();
        }
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
     * shield / RapidFire state are kept. The new rows wait just above the
     * screen and slide in during the last part of the warp. */
    bullets_clearEnemy();
    difficulty_nextWave();
    warpTotal  = IS_PAL_SYSTEM ? WAVE_WARP_FRAMES_PAL  : WAVE_WARP_FRAMES_NTSC;
    warpAccel  = IS_PAL_SYSTEM ? WAVE_WARP_ACCEL_PAL   : WAVE_WARP_ACCEL_NTSC;
    warpArrive = IS_PAL_SYSTEM ? WAVE_WARP_ARRIVE_PAL  : WAVE_WARP_ARRIVE_NTSC;
    wavePauseTimer = warpTotal;
    enemies_beginEnter(WAVE_WARP_ENTER);
    player_chargeCancel();
    player_setWarpFlame(1);
    starSpeed = STAR_SPEED_NORMAL;
    clearEndScreen();
    drawWaveBanner();
    hud_updateScore(score, enemies_left(), player_lives());
    updateWaveWarp();
}

static void updateWaveWarp(void) {
    u16 elapsed;
    s16 lift;
    s16 spd;
    GameEntity* p;

    if (warpTotal == 0 || warpAccel == 0 || warpArrive == 0) {
        starSpeed = STAR_SPEED_NORMAL;
        return;
    }

    elapsed = warpTotal - wavePauseTimer;
    lift = 0;
    spd = STAR_SPEED_NORMAL;

    if (wavePauseTimer >= (warpTotal - warpAccel)) {
        if (elapsed > warpAccel) elapsed = warpAccel;
        spd = STAR_SPEED_NORMAL + (s16)(((STAR_SPEED_WARP - STAR_SPEED_NORMAL) * (s32)elapsed) / (s32)warpAccel);
        lift = (s16)(((s32)WAVE_WARP_LIFT * (s32)elapsed) / (s32)warpAccel);
    } else if (wavePauseTimer > warpArrive) {
        spd = STAR_SPEED_WARP;
        lift = WAVE_WARP_LIFT;
    } else {
        spd = STAR_SPEED_NORMAL + (s16)(((STAR_SPEED_WARP - STAR_SPEED_NORMAL) * (s32)wavePauseTimer) / (s32)warpArrive);
        lift = (s16)(((s32)WAVE_WARP_LIFT * (s32)wavePauseTimer) / (s32)warpArrive);
        enemies_advanceEnter(wavePauseTimer, warpArrive, WAVE_WARP_ENTER);
        if (wavePauseTimer < (warpArrive / 5)) {
            player_setWarpFlame(0);
        }
    }
    if (spd < STAR_SPEED_NORMAL) spd = STAR_SPEED_NORMAL;
    starSpeed = spd;

    p = player_get();
    if (p->health && !player_isExploding()) {
        p->y = PLAYER_START_Y - lift;
    }
}

static void finishWavePause(void) {
    enemies_advanceEnter(0, 1, WAVE_WARP_ENTER);
    stopWaveWarp();
    clearEndScreen();
    hud_updateScore(score, enemies_left(), player_lives());
}

static void resetGame() {
    bullets_reset();
    powerups_reset();

    player_reset();

    difficulty_beginMatch();
    stopWaveWarp();
    enemies_reset();
    explosions_reset();
    score = 0;
    clearEndScreen();
    hud_hideExtraLife();
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
            /* Warp intermission: no formation AI or enemy fire. The player
             * can steer; shooting and a new shield are blocked in the joy
             * handler. Shield timers, 1UP banner and life icons keep going. */
            wavePauseTimer--;
            if (wavePauseTimer == 0) {
                finishWavePause();
            } else {
                updateWaveWarp();
            }
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
        hud_updateExtraLife();
    } else if (gameState == STATE_NAME_ENTRY) {
        explosions_update();
        scores_entryUpdate();
    } else if (gameState == STATE_GAME_OVER || gameState == STATE_WIN) {
        /* Let a burst finish under the end text (last kill or invasion).
         * Pause freezes the animation, like the rest of the timers. */
        explosions_update();
        if (gameState == STATE_GAME_OVER && scoreHighlight >= 0) {
            scoreBlink++;
            if ((scoreBlink & 31) == 0) {
                /* Flash the new row so it is easy to spot in the table. */
                scores_drawTable(12, ((scoreBlink & 32) != 0) ? -1 : scoreHighlight);
            }
        }
    }
}

