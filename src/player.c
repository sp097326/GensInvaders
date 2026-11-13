#include <genesis.h>
#include <resources.h>
#include "player.h"
#include "hud.h"
#include "enemies.h"
#include "powerups.h"

static GameEntity player = {0, 0, 16, 16, 0, 0, 0, NULL, 0, 0, "PLYR" };
static u16 lives = MAX_LIVES;
static u16 respawnTimer = 0;   // > 0 while the player is waiting to come back
static u16 invulnTimer = 0;    // > 0 while the player is blinking and can't be hit

static Sprite* explosionSpr = NULL;
static u16 explTimer = 0;
static u16 explActive = 0;
static u16 pendingGameOver = 0;
static u16 pendingFatal = 0;     /* invasion / landing: a 1UP must not cancel GAME OVER */
static u32 nextExtraLife = EXTRA_LIFE_SCORE;
static s16 explX = 0;
static s16 explY = 0;

static void stopExplosion(void) {
    explActive = 0;
    explTimer = 0;
    if (explosionSpr != NULL) {
        SPR_setVisibility(explosionSpr, HIDDEN);
    }
    VDP_setHorizontalScroll(BG_B, 0);
    PAL_setColor(16 + 2, RGB24_TO_VDPCOLOR(0xF83800));
}

static void startExplosion(void) {
    explActive = 1;
    explTimer = EXPLOSION_DURATION;
    explX = player.x;
    explY = player.y;
    if (explosionSpr != NULL) {
        SPR_setAutoAnimation(explosionSpr, FALSE);
        SPR_setAnimAndFrame(explosionSpr, 0, 0);
        SPR_setPosition(explosionSpr, explX, explY);
        SPR_setVisibility(explosionSpr, VISIBLE);
    }
    /* Brief flash: the shared orange slot of PAL1 goes white for a few frames. */
    PAL_setColor(16 + 2, RGB24_TO_VDPCOLOR(0xFCFCFC));
}

static void placePlayer(void) {
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.velx = 0;
    SPR_setAnim(player.sprite, ANIM_STRAIGHT);
    SPR_setHFlip(player.sprite, FALSE);
    SPR_setPosition(player.sprite, player.x, player.y);
}

static void updatePlayerLife(void) {
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

static void positionPlayer(void) {
    if (player.health == 0) return;   // waiting to respawn
    //handling velocity, adding it to position
    player.x += player.velx;

    /* checking to keep the player in the boundaries of the screen */
    if (player.x < LEFT_EDGE) player.x = LEFT_EDGE;
    if (player.x + player.w > RIGHT_EDGE) player.x = RIGHT_EDGE - player.w;

    //positioning the sprite using the Sprite Engine in SGDK
    SPR_setPosition(player.sprite, player.x, player.y);
}

/* Returns 1 if GAME OVER should be shown now (explosion just ended, no lives). */
static u16 updateExplosion(void) {
    u16 elapsed;
    u16 frame;
    s16 shake;

    if (!explActive) return 0;

    explTimer--;
    elapsed = EXPLOSION_DURATION - explTimer;
    frame = elapsed / (EXPLOSION_DURATION / EXPLOSION_ANIM_FRAMES);
    if (frame >= EXPLOSION_ANIM_FRAMES) frame = EXPLOSION_ANIM_FRAMES - 1;
    if (explosionSpr != NULL) {
        SPR_setFrame(explosionSpr, frame);
        SPR_setPosition(explosionSpr, explX, explY);
    }

    /* Decaying horizontal shake on the starfield plane. */
    if (explTimer > (EXPLOSION_DURATION * 2) / 3) {
        shake = (explTimer & 2) ? EXPLOSION_SHAKE : -EXPLOSION_SHAKE;
    } else if (explTimer > (EXPLOSION_DURATION / 3)) {
        shake = (explTimer & 2) ? 1 : -1;
    } else {
        shake = 0;
    }
    VDP_setHorizontalScroll(BG_B, shake);

    /* Restore the orange after the initial 4-frame flash. */
    if (elapsed == 4) {
        PAL_setColor(16 + 2, RGB24_TO_VDPCOLOR(0xF83800));
    }

    if (explTimer == 0) {
        stopExplosion();
        if (pendingGameOver) {
            pendingGameOver = 0;
            return 1;
        }
        /* Keep the overall dead time close to the old RESPAWN_DELAY. */
        if (RESPAWN_DELAY > EXPLOSION_DURATION) {
            respawnTimer = RESPAWN_DELAY - EXPLOSION_DURATION;
        } else {
            respawnTimer = 1;
        }
    }
    return 0;
}

void player_init(void) {
    /* adding the player to the screen, can this be moved to a function?*/
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.health = 1;
    player.sprite = SPR_addSprite(&ship, player.x, player.y, TILE_ATTR(PAL1, 0, FALSE, FALSE));

    /* One 16x16 explosion sprite (4 hardware cells). The player is hidden while
     * it plays, so the hardware-sprite budget stays at 70. High priority so it
     * sits above bullets and the starfield. Animation frames are set by hand. */
    explosionSpr = SPR_addSprite(&explosion, 0, -32, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    SPR_setAutoAnimation(explosionSpr, FALSE);
    SPR_setVisibility(explosionSpr, HIDDEN);
}

void player_reset(void) {
    stopExplosion();
    pendingGameOver = 0;
    pendingFatal = 0;
    nextExtraLife = EXTRA_LIFE_SCORE;
    placePlayer();
    reviveGameEntity(&player);
    lives = MAX_LIVES;
    respawnTimer = 0;
    invulnTimer = 0;
}

void player_hide(void) {
    killGameEntity(&player);
    stopExplosion();
}

u16 player_update(void) {
    if (updateExplosion()) return 1;
    updatePlayerLife();
    positionPlayer();
    return 0;
}

void player_stop(void) {
    player.velx = 0;
    SPR_setAnim(player.sprite, ANIM_STRAIGHT);
}

u16 player_hit(int score) {
    killGameEntity(&player);
    player.velx = 0;
    if (powerups_isOn()) powerups_deactivate();
    // Own channel (CH2) and top priority (15) so nothing can cut the explosion.
    XGM_startPlayPCM(SFX_XPLOSION,15,SOUND_PCM_CH2);
    lives--;
    hud_updateScore(score, enemies_left(), lives);
    pendingGameOver = (lives == 0) ? 1 : 0;
    pendingFatal = 0;
    startExplosion();
    /* GAME OVER waits until the explosion finishes (player_update returns 1). */
    return 0;
}

void player_hitFatal(int score) {
    if (explActive || player.health == 0) return;
    killGameEntity(&player);
    player.velx = 0;
    if (powerups_isOn()) powerups_deactivate();
    XGM_startPlayPCM(SFX_XPLOSION,15,SOUND_PCM_CH2);
    lives = 0;
    hud_updateScore(score, enemies_left(), lives);
    pendingGameOver = 1;
    pendingFatal = 1;
    startExplosion();
}

GameEntity* player_get(void) {
    return &player;
}

u16 player_lives(void) {
    return lives;
}

u16 player_invulnTimer(void) {
    return invulnTimer;
}

u16 player_isExploding(void) {
    return explActive;
}

u16 player_checkExtraLife(int score) {
    u16 granted = 0;
    u32 sc;

    if (score < 0) return 0;
    sc = (u32)score;

    while (sc >= nextExtraLife) {
        /* Invasion already decided the match: keep counting thresholds but
         * do not add a ship that would cancel GAME OVER. */
        if (!pendingFatal && lives < LIVES_CAP) {
            lives++;
            granted++;
        }
        if (nextExtraLife > (0xFFFFFFFFul - (u32)EXTRA_LIFE_SCORE)) {
            nextExtraLife = 0xFFFFFFFFul;
            break;
        }
        nextExtraLife += (u32)EXTRA_LIFE_SCORE;
    }

    if (granted > 0) {
        /* A 1UP earned after losing the last ship (bullet hit, not invasion)
         * cancels the pending GAME OVER and the player will respawn. */
        if (pendingGameOver && !pendingFatal) pendingGameOver = 0;
        XGM_startPlayPCM(SFX_1UP, 12, SOUND_PCM_CH4);
    }
    return granted;
}
