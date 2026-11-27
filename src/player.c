#include <genesis.h>
#include <resources.h>
#include "player.h"
#include "hud.h"
#include "enemies.h"
#include "powerups.h"
#include "bullets.h"
#include "difficulty.h"

static GameEntity player = {0, 0, 16, 16, 0, 0, 0, NULL, 0, 0, "PLYR" };
static u16 lives = MAX_LIVES;
static u16 respawnTimer = 0;   // > 0 while the player is waiting to come back
static u16 invulnTimer = 0;    // > 0 while the player is blinking and can't be hit
/* 1 for every frame of an active dash, including the last movement
 * frame when positionPlayer has already cleared boostTimer. Collision
 * runs after player_update, so this keeps the last dash frame safe. */
static u16 boostInvuln = 0;

static Sprite* explosionSpr = NULL;
static Sprite* flameSpr = NULL;
static u16 flameOn = 0;
static u16 flameTick = 0;
static Sprite* chargeGlowSpr = NULL;
static u16 chargeHeld = 0;
static u16 chargeTimer = 0;
static u16 chargeReady = 0;

/* Boost / dash (button A + direction), see BOOST_* in config.h. */
static Sprite* ghostSpr[BOOST_GHOSTS] = { NULL, NULL };
static s16 trailHist[BOOST_HIST];
static u16 trailIdx = 0;
static u16 trailTimer = 0;      /* > 0 while the trail lingers after a dash */
static u16 boostTimer = 0;      /* > 0 while dashing */
static s16 boostDir = 0;
static u16 boostCooldown = 0;   /* > 0 while recharging */
static u16 boostRefillPending = 0;  /* "I" capsule caught mid-dash */
static u16 boostCooldownMax = 0;
static u16 boostReadyBlink = 0;
static u16 boostHudMode = 0xFFFF;
static u16 boostHudLevel = 0xFFFF;
static u16 boostHudBlink = 0xFFFF;

static void hideChargeGlow(void);
static void updateChargeGlow(void);
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
    if (flameSpr != NULL) {
        SPR_setVisibility(flameSpr, HIDDEN);
    }
    player_chargeCancel();
    /* Brief flash: the shared orange slot of PAL1 goes white for a few frames. */
    PAL_setColor(16 + 2, RGB24_TO_VDPCOLOR(0xFCFCFC));
}

static void placePlayer(void) {
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.velx = 0;
    if (player.sprite != NULL) {
        SPR_setAnim(player.sprite, ANIM_STRAIGHT);
        SPR_setHFlip(player.sprite, FALSE);
        SPR_setPosition(player.sprite, player.x, player.y);
    }
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
        if (player.sprite != NULL) {
            SPR_setVisibility(player.sprite, ((invulnTimer & 4) && invulnTimer > 0) ? HIDDEN : VISIBLE);
        }
    }
}

static void hideGhost(void) {
    u16 i;
    for (i = 0; i < BOOST_GHOSTS; i++) {
        if (ghostSpr[i] != NULL) SPR_setVisibility(ghostSpr[i], HIDDEN);
    }
}

static void fillTrail(s16 x) {
    u16 i;
    for (i = 0; i < BOOST_HIST; i++) trailHist[i] = x;
}

static u16 boostRechargeFrames(void) {
    u16 rate = IS_PAL_SYSTEM ? 50 : 60;
    return (u16)(difficulty_config()->shieldRechargeSec * rate);
}

static void boostRefreshHud(u16 force) {
    u16 mode;
    u16 level;
    u16 blink;

    if (boostTimer > 0) {
        mode = SHIELD_HUD_ACTIVE;
        level = (u16)(((u32)boostTimer * SHIELD_BAR_STEPS + BOOST_FRAMES - 1) / BOOST_FRAMES);
    } else if (boostCooldown > 0) {
        mode = SHIELD_HUD_RECHARGE;
        if (boostCooldownMax == 0) level = SHIELD_BAR_STEPS;
        else level = (u16)(((u32)(boostCooldownMax - boostCooldown) * SHIELD_BAR_STEPS) / boostCooldownMax);
    } else {
        mode = SHIELD_HUD_READY;
        level = SHIELD_BAR_STEPS;
    }
    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    blink = (u16)((mode == SHIELD_HUD_READY && (boostReadyBlink & 4)) ? 1 : 0);
    if (!force && mode == boostHudMode && level == boostHudLevel && blink == boostHudBlink) return;
    boostHudMode = mode;
    boostHudLevel = level;
    boostHudBlink = blink;
    hud_updateBoostBar(mode, level, blink);
}

/* End a dash (finished, or the ship was hit) and start the recharge.
 * linger != 0 keeps the trail for BOOST_TRAIL_LINGER frames. */
static void endBoost(u16 linger) {
    if (boostTimer == 0) return;
    boostTimer = 0;
    boostCooldownMax = boostRechargeFrames();
    boostCooldown = boostCooldownMax;
    if (boostRefillPending) {
        boostRefillPending = 0;
        boostCooldown = 0;
        boostReadyBlink = BOOST_READY_BLINK;
    }
    trailTimer = linger ? BOOST_TRAIL_LINGER : 0;
    if (!linger) hideGhost();
    boostRefreshHud(1);
}

void player_refillBoost(void) {
    if (boostTimer > 0) {
        boostRefillPending = 1;
        return;
    }
    if (boostCooldown > 0) {
        boostCooldown = 0;
        boostReadyBlink = BOOST_READY_BLINK;
        boostRefreshHud(1);
    }
}

u16 player_boostReady(void) {
    return (boostTimer == 0 && boostCooldown == 0) ? 1 : 0;
}

static void updateBoostTimers(void) {
    if (boostTimer > 0) return;  /* counted down in positionPlayer */
    if (boostCooldown > 0) {
        boostCooldown--;
        if (boostCooldown == 0) boostReadyBlink = BOOST_READY_BLINK;
        boostRefreshHud(0);
    } else if (boostReadyBlink > 0) {
        boostReadyBlink--;
        boostRefreshHud(0);
    }
}

/* Trail: ghosts sit where the ship was BOOST_GHOST_LAG1/2 frames ago,
 * every frame, so they stretch out during the dash and slide back into
 * the ship while lingering. A ghost on top of the ship is hidden. */
static void updateGhost(void) {
    static const u16 lag[BOOST_GHOSTS] = { BOOST_GHOST_LAG1, BOOST_GHOST_LAG2 };
    u16 i;

    trailIdx = (trailIdx + 1) & (BOOST_HIST - 1);
    trailHist[trailIdx] = (s16)player.x;
    if (trailTimer > 0 && boostTimer == 0) trailTimer--;
    if ((boostTimer == 0 && trailTimer == 0) || player.health == 0 || explActive) {
        trailTimer = 0;
        hideGhost();
        return;
    }
    for (i = 0; i < BOOST_GHOSTS; i++) {
        s16 gx = trailHist[(trailIdx - lag[i]) & (BOOST_HIST - 1)];
        if (ghostSpr[i] == NULL) continue;
        if (gx == (s16)player.x) {
            SPR_setVisibility(ghostSpr[i], HIDDEN);
            continue;
        }
        SPR_setHFlip(ghostSpr[i], (boostDir > 0) ? TRUE : FALSE);
        SPR_setPosition(ghostSpr[i], gx, player.y);
        SPR_setVisibility(ghostSpr[i], VISIBLE);
    }
}

u16 player_tryBoost(s16 dir) {
    if (player.health == 0 || explActive || respawnTimer > 0) return 0;
    if (boostTimer > 0 || boostCooldown > 0) return 0;
    if (dir == 0) return 0;
    boostDir = (dir > 0) ? 1 : -1;
    boostTimer = BOOST_FRAMES;
    boostReadyBlink = 0;
    trailTimer = 0;
    fillTrail((s16)player.x);
    boostRefreshHud(1);
    return 1;
}

static void positionPlayer(void) {
    if (player.health == 0) return;   // waiting to respawn
    //handling velocity, adding it to position
    if (boostTimer > 0) {
        /* Dash: fixed speed in the dash direction, ignores the d-pad. */
        player.x += boostDir * BOOST_SPEED;
        boostTimer--;
        if (boostTimer == 0) {
            boostTimer = 1;  /* let endBoost see an active dash */
            endBoost(1);
        } else {
            boostRefreshHud(0);
        }
    } else {
        player.x += player.velx;
    }

    /* checking to keep the player in the boundaries of the screen */
    if (player.x < LEFT_EDGE) player.x = LEFT_EDGE;
    if (player.x + player.w > RIGHT_EDGE) player.x = RIGHT_EDGE - player.w;

    //positioning the sprite using the Sprite Engine in SGDK
    if (player.sprite != NULL) {
        SPR_setPosition(player.sprite, player.x, player.y);
    }
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
    player.sprite = addGameSprite(&ship, player.x, player.y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    player.health = (player.sprite != NULL) ? 1 : 0;

    /* One 16x16 explosion sprite (4 hardware cells). The player is hidden while
     * it plays, so the hardware-sprite budget stays at 70. High priority so it
     * sits above bullets and the starfield. Animation frames are set by hand. */
    explosionSpr = addGameSprite(&explosion, 0, -32, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    if (explosionSpr != NULL) {
        SPR_setAutoAnimation(explosionSpr, FALSE);
        SPR_setVisibility(explosionSpr, HIDDEN);
    }

    /* 8x8 exhaust (2 frames). One hardware sprite, sits under the ship
     * at Y=208-215, just above the life icons (row 27) and below the
     * 16x16 hull, so it does not add to a busy scanline. */
    flameSpr = addGameSprite(&flame, 0, -16, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    if (flameSpr != NULL) {
        SPR_setAutoAnimation(flameSpr, FALSE);
        SPR_setVisibility(flameSpr, HIDDEN);
    }
    flameOn = 0;

    /* 16x16 aura around the ship (2 frames). One hardware sprite, same
     * scanlines as the hull, so the 20-sprite/line budget only grows by 1. */
    chargeGlowSpr = addGameSprite(&chargeglow, 0, -32, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    if (chargeGlowSpr != NULL) {
        SPR_setAutoAnimation(chargeGlowSpr, FALSE);
        SPR_setVisibility(chargeGlowSpr, HIDDEN);
    }

    /* Dash trail: 2 ghost ships (1 hardware sprite and 4 tiles each),
     * added after the ship so it draws on top. They use PAL0 colours 1-4
     * (the font only uses 0 and 15) tinted as a blue ghost of the ship's
     * blue / orange / white / black. */
    {
        u16 i;
        PAL_setColor(1, RGB24_TO_VDPCOLOR(0x2060C0));
        PAL_setColor(2, RGB24_TO_VDPCOLOR(0x40A0FC));
        PAL_setColor(3, RGB24_TO_VDPCOLOR(0xA0E0FC));
        PAL_setColor(4, RGB24_TO_VDPCOLOR(0x002060));
        for (i = 0; i < BOOST_GHOSTS; i++) {
            ghostSpr[i] = addGameSprite(&ship, 0, -32, TILE_ATTR(PAL0, 0, FALSE, FALSE));
            if (ghostSpr[i] != NULL) {
                SPR_setAutoAnimation(ghostSpr[i], FALSE);
                SPR_setAnim(ghostSpr[i], ANIM_MOVE);
                SPR_setVisibility(ghostSpr[i], HIDDEN);
            }
        }
    }
}

void player_reset(void) {
    stopExplosion();
    pendingGameOver = 0;
    pendingFatal = 0;
    nextExtraLife = EXTRA_LIFE_SCORE;
    player_setWarpFlame(0);
    player_chargeCancel();
    placePlayer();
    reviveGameEntity(&player);
    lives = MAX_LIVES;
    respawnTimer = 0;
    invulnTimer = 0;
    boostInvuln = 0;
    boostTimer = 0;
    boostCooldown = 0;
    boostCooldownMax = 0;
    boostReadyBlink = 0;
    boostRefillPending = 0;
    trailTimer = 0;
    hideGhost();
    boostRefreshHud(1);
}

void player_hide(void) {
    killGameEntity(&player);
    stopExplosion();
    player_setWarpFlame(0);
    player_chargeCancel();
    boostTimer = 0;  /* no HUD redraw here: the title may be on screen */
    boostInvuln = 0;
    trailTimer = 0;
    hideGhost();
}

static void updateFlame(void) {
    if (flameSpr == NULL) return;
    if (!flameOn || player.health == 0 || explActive) {
        SPR_setVisibility(flameSpr, HIDDEN);
        return;
    }
    /* Hide with the ship during the post-respawn invuln blink. */
    if (invulnTimer > 0 && (invulnTimer & 4)) {
        SPR_setVisibility(flameSpr, HIDDEN);
        return;
    }
    flameTick++;
    SPR_setAnimAndFrame(flameSpr, 0, (flameTick >> 2) & 1);
    SPR_setPosition(flameSpr, player.x + 4, player.y + 16);
    SPR_setVisibility(flameSpr, VISIBLE);
}

u16 player_update(void) {
    if (updateExplosion()) return 1;
    /* Latch before positionPlayer: the last dash frame decrements
     * boostTimer to 0, but the ship still moved this frame. */
    boostInvuln = (boostTimer > 0) ? 1 : 0;
    updatePlayerLife();
    positionPlayer();
    updateBoostTimers();
    updateGhost();
    updateFlame();
    updateChargeGlow();
    return 0;
}

void player_stop(void) {
    player.velx = 0;
    if (player.sprite != NULL) SPR_setAnim(player.sprite, ANIM_STRAIGHT);
}

u16 player_hit(int score) {
    endBoost(0);  /* a dash cut short by a hit still costs the recharge */
    killGameEntity(&player);
    player.velx = 0;
    if (powerups_isOn()) powerups_deactivate();
    powerups_clearCapsule();
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
    endBoost(0);
    killGameEntity(&player);
    player.velx = 0;
    if (powerups_isOn()) powerups_deactivate();
    powerups_clearCapsule();
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

u16 player_isInvulnerable(void) {
    return (invulnTimer > 0 || boostTimer > 0 || boostInvuln) ? 1 : 0;
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

void player_setWarpFlame(u16 on) {
    flameOn = on ? 1 : 0;
    flameTick = 0;
    if (!flameOn && flameSpr != NULL) {
        SPR_setVisibility(flameSpr, HIDDEN);
    }
}

static void hideChargeGlow(void) {
    if (chargeGlowSpr != NULL) {
        SPR_setVisibility(chargeGlowSpr, HIDDEN);
    }
}

static void updateChargeGlow(void) {
    u16 need;
    if (chargeGlowSpr == NULL) return;
    if (!chargeHeld || player.health == 0 || explActive) {
        hideChargeGlow();
        return;
    }
    if (invulnTimer > 0 && (invulnTimer & 4)) {
        hideChargeGlow();
        return;
    }
    need = IS_PAL_SYSTEM ? CHARGE_FRAMES_PAL : CHARGE_FRAMES_NTSC;
    if (!chargeReady) {
        if (chargeTimer < need) chargeTimer++;
        if (chargeTimer >= need) {
            chargeReady = 1;
            XGM_startPlayPCM(SFX_CHARGE_READY, 10, SOUND_PCM_CH4);
        }
    }
    SPR_setAnimAndFrame(chargeGlowSpr, 0, chargeReady ? 1 : 0);
    /* Blink only once fully charged; while charging the dim frame stays on. */
    if (chargeReady && (chargeTimer & 4)) {
        hideChargeGlow();
        chargeTimer++;
        return;
    }
    if (chargeReady) chargeTimer++;
    SPR_setPosition(chargeGlowSpr, player.x, player.y);
    SPR_setVisibility(chargeGlowSpr, VISIBLE);
}

void player_chargeBegin(void) {
    if (player.health == 0 || explActive) return;
    chargeHeld = 1;
    chargeTimer = 0;
    chargeReady = 0;
}

void player_chargeRelease(void) {
    u16 ready;

    ready = chargeReady;
    chargeHeld = 0;
    chargeTimer = 0;
    chargeReady = 0;
    hideChargeGlow();
    if (player.health == 0 || explActive) return;
    if (ready) {
        /* One charged bolt at a time; if it is still on screen, fall back
         * to a normal shot so the release is never a dead input. */
        if (!bullets_shootCharged(player)) {
            bullets_shootPlayer(player);
        }
    } else {
        bullets_shootPlayer(player);
    }
}

void player_chargeCancel(void) {
    chargeHeld = 0;
    chargeTimer = 0;
    chargeReady = 0;
    hideChargeGlow();
}

void player_chargeSyncButton(u16 held) {
    /* After pause: if B was released while we were not looking, drop the
     * charge without firing so unpausing never shoots by accident. */
    if (!held && chargeHeld) {
        player_chargeCancel();
    }
}
