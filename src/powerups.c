#include <genesis.h>
#include <resources.h>
#include "powerups.h"
#include "bullets.h"
#include "player.h"
#include "hud.h"
#include "difficulty.h"
#include "collision.h"

static GameEntity capsule = {0, 0, 8, 8, 0, 0, 0, NULL, 0, 0, ""};
static u16 capsuleTick = 0;
static u16 killsSinceDrop = 0;
static u16 powerupTimer = 0;
static u16 rapidDuration = 0;
static bool powerOn = 0;

static Sprite* shieldSpr = NULL;
static u16 shieldActive = 0;
static u16 shieldTimer = 0;
static u16 shieldRecharge = 0;
static u16 shieldDuration = 0;
static u16 shieldRechargeMax = 0;
static u16 shieldHudMode = 0xFFFF;
static u16 shieldHudLevel = 0xFFFF;
static u16 shieldHudBlink = 0xFFFF;
static u16 shieldReadyBlink = 0;
static u16 shieldHumMuted = 0;
static u16 shieldHumPlaying = 0;
static u16 rapidHudActive = 0xFFFF;
static u16 rapidHudLevel = 0xFFFF;

static void stopShieldHum(void) {
    if (shieldHumPlaying) {
        XGM_stopPlayPCM(SOUND_PCM_CH1);
        shieldHumPlaying = 0;
    }
}

static void playShieldHum(void) {
    XGM_startPlayPCM(SFX_SHIELD_HUM, 13, SOUND_PCM_CH1);
    shieldHumPlaying = 1;
}

/* Keep a 2 s hum going on CH1 for as long as the shield is up. Shots
 * (CH3), explosions (CH2) and the 1UP jingle (CH4) cannot cut it.
 * The sample is retriggered if the difficulty lasts longer than 2 s. */
static void shieldHumUpdate(void) {
    u16 want;

    if (shieldHumMuted) return;
    want = 0;
    if (shieldActive && (shieldTimer > 0) &&
        player_get()->health && !player_isExploding()) {
        want = 1;
    }
    if (!want) {
        stopShieldHum();
        return;
    }
    if (!XGM_isPlayingPCM(SOUND_PCM_CH1_MSK)) {
        playShieldHum();
    }
}

void powerups_muteShieldSound(void) {
    shieldHumMuted = 1;
    stopShieldHum();
}

void powerups_unmuteShieldSound(void) {
    shieldHumMuted = 0;
    shieldHumUpdate();
}

static void applyTimings(void) {
    const DifficultyConfig* cfg = difficulty_config();
    u16 rate = IS_PAL_SYSTEM ? 50 : 60;
    shieldDuration = cfg->shieldDurationSec * rate;
    shieldRechargeMax = cfg->shieldRechargeSec * rate;
    rapidDuration = cfg->rapidFireSec * rate;
}

static void hideShieldSprite(void) {
    if (shieldSpr != NULL) {
        SPR_setVisibility(shieldSpr, HIDDEN);
        SPR_setAutoAnimation(shieldSpr, FALSE);
    }
}

static void showShieldSprite(void) {
    GameEntity* p;

    if (shieldSpr == NULL) return;
    p = player_get();
    SPR_setPosition(shieldSpr, p->x - SHIELD_OFFSET, p->y - SHIELD_OFFSET);
    SPR_setAnimAndFrame(shieldSpr, 0, 0);
    SPR_setAnimationLoop(shieldSpr, TRUE);
    SPR_setAutoAnimation(shieldSpr, TRUE);
    SPR_setVisibility(shieldSpr, VISIBLE);
}

static void followShieldSprite(void) {
    GameEntity* p;

    if (shieldSpr == NULL) return;
    p = player_get();
    if (p->health == 0) {
        SPR_setVisibility(shieldSpr, HIDDEN);
        return;
    }
    SPR_setPosition(shieldSpr, p->x - SHIELD_OFFSET, p->y - SHIELD_OFFSET);
    SPR_setVisibility(shieldSpr, VISIBLE);
}

static void shieldRefreshHud(u16 force) {
    u16 mode;
    u16 level;
    u16 frames;
    u16 max;
    u16 blink;

    if (shieldActive) {
        mode = SHIELD_HUD_ACTIVE;
        frames = shieldTimer;
        max = shieldDuration;
        /* Ceil so the last pixel stays until the timer actually hits 0. */
        if (max == 0) level = 0;
        else level = (u16)(((u32)frames * SHIELD_BAR_STEPS + max - 1) / max);
    } else if (shieldRecharge > 0) {
        mode = SHIELD_HUD_RECHARGE;
        frames = shieldRecharge;
        max = shieldRechargeMax;
        if (max == 0) level = SHIELD_BAR_STEPS;
        else level = (u16)(((u32)(max - frames) * SHIELD_BAR_STEPS) / max);
    } else {
        mode = SHIELD_HUD_READY;
        level = SHIELD_BAR_STEPS;
    }
    if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;

    blink = (u16)((mode == SHIELD_HUD_READY && shieldReadyBlink && (shieldReadyBlink & 4)) ? 1 : 0);

    if (!force && mode == shieldHudMode && level == shieldHudLevel
        && blink == shieldHudBlink) return;
    shieldHudMode = mode;
    shieldHudLevel = level;
    shieldHudBlink = blink;
    hud_updateShieldBar(mode, level, blink);
}

/* The ship body is PAL1 colour 2 (index 18). Red while RapidFire is on;
 * back to the usual orange when it ends. */
static void setShipRapidTint(u16 on) {
    if (on) {
        PAL_setColor(16 + 2, RGB24_TO_VDPCOLOR(0xF80000));
    } else {
        PAL_setColor(16 + 2, RGB24_TO_VDPCOLOR(0xF83800));
    }
}

static void rapidRefreshHud(u16 force) {
    u16 active;
    u16 level;
    u16 max;

    active = (powerOn && powerupTimer > 0) ? 1 : 0;
    max = rapidDuration;
    if (!active || max == 0) {
        level = 0;
    } else {
        level = (u16)(((u32)powerupTimer * SHIELD_BAR_STEPS + max - 1) / max);
        if (level > SHIELD_BAR_STEPS) level = SHIELD_BAR_STEPS;
    }
    if (!force && active == rapidHudActive && level == rapidHudLevel) return;
    rapidHudActive = active;
    rapidHudLevel = level;
    hud_updateRapidBar(active, level);
}

void powerups_clearCapsule(void) {
    if (capsule.sprite != NULL) {
        killGameEntity(&capsule);
        SPR_setAutoAnimation(capsule.sprite, FALSE);
    } else {
        capsule.health = 0;
    }
    capsuleTick = 0;
}

static void collectCapsule(void) {
    powerups_clearCapsule();
    /* CH4, below the 1UP jingle, never on the shield hum channel. */
    XGM_startPlayPCM(SFX_RAPID_PICK, 11, SOUND_PCM_CH4);
    applyTimings();
    bullets_setMaxPlayer(MAX_BULLETS);
    powerupTimer = rapidDuration;
    setShipRapidTint(1);
    powerOn = POWER_ON;
    rapidRefreshHud(1);
}

void powerups_tryDrop(s16 x, s16 y) {
    s16 cx;

    if (killsSinceDrop < 255) killsSinceDrop++;

    /* At most one capsule on screen. Pity still counts so the next
     * free kill can force a drop after this one is gone. */
    if (capsule.health > 0) return;
    if (capsule.sprite == NULL) return;

    if (killsSinceDrop < RAPID_DROP_PITY) {
        if ((random() % RAPID_DROP_MOD) != 0) return;
    }

    killsSinceDrop = 0;
    cx = x + 4;
    if (cx < LEFT_EDGE) cx = LEFT_EDGE;
    if (cx > (s16)(RIGHT_EDGE - 8)) cx = (s16)(RIGHT_EDGE - 8);
    capsule.x = cx;
    capsule.y = y + 4;
    capsule.vely = CAPSULE_SPEED;
    capsule.health = 1;
    capsuleTick = 0;
    SPR_setAnimAndFrame(capsule.sprite, 0, 0);
    SPR_setPosition(capsule.sprite, capsule.x, capsule.y);
    SPR_setVisibility(capsule.sprite, VISIBLE);
}

void powerups_init(void) {
    applyTimings();
    setShipRapidTint(0);
    /* One 32x32 bubble (1 hardware sprite, 16 tiles). High priority so the
     * ring sits above the ship and incoming bullets. PAL1 matches space.bmp. */
    shieldSpr = SPR_addSprite(&shield, 0, -40, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    SPR_setAutoAnimation(shieldSpr, FALSE);
    SPR_setVisibility(shieldSpr, HIDDEN);

    /* 8x8 "R" capsule, 2 blink frames, one hardware sprite. Low priority
     * so the bottom-row life icons and RF bar stay readable if it falls
     * through that corner. */
    capsule.sprite = SPR_addSprite(&capsuleSpr, 0, -16, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    SPR_setAutoAnimation(capsule.sprite, FALSE);
    SPR_setVisibility(capsule.sprite, HIDDEN);
    capsule.health = 0;
}

void powerups_deactivate(void) {
    bullets_setMaxPlayer(MAX_PLAYER_BULLETS);
    setShipRapidTint(0);
    powerOn = POWER_OFF;
    powerupTimer = 0;
    rapidRefreshHud(1);
}

void powerups_update(void) {
    if (powerOn) {
        if (powerupTimer > 0) {
            powerupTimer--;
        }
        if (powerupTimer == 0) {
            powerups_deactivate();
        } else {
            rapidRefreshHud(0);
        }
    }

    if (capsule.health > 0) {
        GameEntity* p;

        capsule.y += capsule.vely;
        if (capsule.y + capsule.h >= BOTTOM_EDGE) {
            powerups_clearCapsule();
        } else {
            capsuleTick++;
            SPR_setAnimAndFrame(capsule.sprite, 0, (capsuleTick >> 3) & 1);
            SPR_setPosition(capsule.sprite, capsule.x, capsule.y);
            p = player_get();
            if (p->health && !player_isExploding() && collideEntities(&capsule, p)) {
                collectCapsule();
            }
        }
    }

    if (shieldActive) {
        followShieldSprite();
        if (shieldTimer > 0) shieldTimer--;
        if (shieldTimer == 0) {
            shieldActive = 0;
            hideShieldSprite();
            stopShieldHum();
            shieldRecharge = shieldRechargeMax;
        } else {
            shieldHumUpdate();
        }
        shieldRefreshHud(0);
    } else if (shieldRecharge > 0) {
        shieldRecharge--;
        if (shieldRecharge == 0) {
            XGM_startPlayPCM(SFX_SHIELD_READY, 10, SOUND_PCM_CH1);
            shieldReadyBlink = 32;
        }
        shieldRefreshHud(0);
    } else if (shieldReadyBlink > 0) {
        shieldReadyBlink--;
        shieldRefreshHud(1);
    }
}

void powerups_reset(void) {
    if (powerOn) powerups_deactivate();
    powerupTimer = 0;
    killsSinceDrop = 0;
    powerups_clearCapsule();
    applyTimings();
    shieldActive = 0;
    shieldTimer = 0;
    shieldRecharge = 0;
    shieldReadyBlink = 0;
    hideShieldSprite();
    shieldHumMuted = 0;
    stopShieldHum();
    shieldRefreshHud(1);
    rapidRefreshHud(1);
}

void powerups_hide(void) {
    hideShieldSprite();
    stopShieldHum();
    powerups_clearCapsule();
    if (powerOn) powerups_deactivate();
}

u16 powerups_isOn(void) {
    return powerOn ? 1 : 0;
}

void powerups_tryShield(void) {
    if (player_isExploding()) return;
    if (player_get()->health == 0) return;
    if (shieldActive || shieldRecharge > 0) return;
    shieldActive = 1;
    shieldTimer = shieldDuration;
    showShieldSprite();
    shieldHumMuted = 0;
    playShieldHum();
    shieldRefreshHud(1);
}

u16 powerups_shieldActive(void) {
    return shieldActive;
}
