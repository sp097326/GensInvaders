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

void player_init(void) {
    /* adding the player to the screen, can this be moved to a function?*/
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.health = 1;
    player.sprite = SPR_addSprite(&ship, player.x, player.y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
}

void player_reset(void) {
    placePlayer();
    reviveGameEntity(&player);
    lives = MAX_LIVES;
    respawnTimer = 0;
    invulnTimer = 0;
}

void player_hide(void) {
    killGameEntity(&player);
}

void player_update(void) {
    updatePlayerLife();
    positionPlayer();
}

void player_stop(void) {
    player.velx = 0;
    SPR_setAnim(player.sprite, ANIM_STRAIGHT);
}

u16 player_hit(int score) {
    killGameEntity(&player);
    player.velx = 0;
    if (powerups_isOn()) powerups_deactivate();
    //present the xplosion sprite instead....
    // Own channel (CH2) and top priority (15) so nothing can cut the explosion.
    XGM_startPlayPCM(SFX_XPLOSION,15,SOUND_PCM_CH2);
    lives--;
    hud_updateScore(score, enemies_left(), lives);
    if (lives == 0) {
        return 1;
    }
    respawnTimer = RESPAWN_DELAY;
    return 0;
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
