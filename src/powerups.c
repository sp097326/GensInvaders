#include <genesis.h>
#include <resources.h>
#include "powerups.h"
#include "bullets.h"
#include "player.h"
#include "hud.h"
#include "difficulty.h"

/* Reserved for a future on-screen pickup; spawn/position are still stubs. */
static GameEntity powerup = {0, 0, 8, 8, 0, 0, 0, NULL, 0, 0, ""};
static u16 powerupTimer = 0;
static bool powerOn = 0;

static Sprite* shieldSpr = NULL;
static u16 shieldActive = 0;
static u16 shieldTimer = 0;
static u16 shieldRecharge = 0;
static u16 shieldDuration = 0;
static u16 shieldRechargeMax = 0;
static u16 shieldHudMode = 0xFFFF;
static u16 shieldHudSecs = 0xFFFF;

static void applyShieldTimings(void) {
    const DifficultyConfig* cfg = difficulty_config();
    u16 rate = IS_PAL_SYSTEM ? 50 : 60;
    shieldDuration = cfg->shieldDurationSec * rate;
    shieldRechargeMax = cfg->shieldRechargeSec * rate;
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
    u16 secs;
    u16 rate;
    u16 frames;
    char line[12];

    if (shieldActive) mode = 1;
    else if (shieldRecharge > 0) mode = 2;
    else mode = 0;

    rate = IS_PAL_SYSTEM ? 50 : 60;
    if (mode == 0) {
        secs = 0;
    } else {
        frames = (mode == 1) ? shieldTimer : shieldRecharge;
        secs = (u16)((frames + rate - 1) / rate);
    }

    if (!force && mode == shieldHudMode && secs == shieldHudSecs) return;
    shieldHudMode = mode;
    shieldHudSecs = secs;

    if (mode == 1) {
        sprintf(line, "ESC: ON %d", secs);
    } else if (mode == 2) {
        sprintf(line, "ESC: %ds", secs);
    } else {
        sprintf(line, "ESC: LISTO");
    }
    hud_updateShield(line);
}

void powerups_init(void) {
    PAL_setColor(34, RGB24_TO_VDPCOLOR(0x0078f8));
    (void)powerup; /* reserved for a future on-screen pickup */
    applyShieldTimings();
    /* One 32x32 bubble (1 hardware sprite, 16 tiles). High priority so the
     * ring sits above the ship and incoming bullets. PAL1 matches space.bmp. */
    shieldSpr = SPR_addSprite(&shield, 0, -40, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    SPR_setAutoAnimation(shieldSpr, FALSE);
    SPR_setVisibility(shieldSpr, HIDDEN);
}

void powerups_deactivate(void) {
    bullets_setMaxPlayer(MAX_PLAYER_BULLETS);
    // Restore the original color. PAL_setColor takes a single u16 color value;
    // PAL_setPaletteColors expects a u16* array, hence the previous warning/crash.
    PAL_setColor(34, RGB24_TO_VDPCOLOR(0x0078f8));
    // Clear the flag so the A button toggles the powerup back ON next time.
    // This line was missing, so powerOn was stuck at 1 after the first press.
    powerOn = POWER_OFF;
}

void powerups_activate(Powerup type) {
    switch (type)
    {
    case RapidFire: {
        // RapidFire = let the player keep more bullets on screen at once.
        // Was MAX_PLAYER_BULLETS (3), i.e. identical to the default, so the
        // powerup did nothing. MAX_BULLETS (6) is the size of the player pool.
        bullets_setMaxPlayer(MAX_BULLETS);
        powerupTimer = POWERUP_DURATION;
        // Tint the powerup color RED while active, using the SAME palette index
        // (34) that deactivatePowerup() restores so the toggle is symmetric;
        // it used to be index 18 here, which is why the color never reset.
        // Use PAL_setColor (one u16 color). The old PAL_setPaletteColors call
        // wanted a u16* array, so passing a color value was the "aqui truena" bug.
        PAL_setColor(34, RGB24_TO_VDPCOLOR(0xf83000));
        powerOn = POWER_ON;
        break;
    }
    default:
        return;
    }
}

void powerups_update(void) {
    if (powerOn) {
        // Guard against u16 underflow: decrementing 0 would wrap to 65535,
        // leaving the powerup stuck on for ~18 minutes (65535 frames).
        if (powerupTimer > 0) {
            powerupTimer--;
        }
        if (powerupTimer == 0) {
            powerups_deactivate();
        }
    }

    if (shieldActive) {
        followShieldSprite();
        if (shieldTimer > 0) shieldTimer--;
        if (shieldTimer == 0) {
            shieldActive = 0;
            hideShieldSprite();
            shieldRecharge = shieldRechargeMax;
        }
        shieldRefreshHud(0);
    } else if (shieldRecharge > 0) {
        shieldRecharge--;
        if (shieldRecharge == 0) {
            XGM_startPlayPCM(SFX_SHIELD_READY, 10, SOUND_PCM_CH1);
        }
        shieldRefreshHud(0);
    }
}

void powerups_reset(void) {
    if (powerOn) powerups_deactivate();
    powerupTimer = 0;
    applyShieldTimings();
    shieldActive = 0;
    shieldTimer = 0;
    shieldRecharge = 0;
    hideShieldSprite();
    shieldRefreshHud(1);
}

void powerups_hide(void) {
    hideShieldSprite();
}

void powerups_toggle(void) {
    if (powerOn) {
        powerups_deactivate();
    } else {
        powerups_activate(RapidFire);
    }
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
    XGM_startPlayPCM(SFX_SHIELD_ON, 12, SOUND_PCM_CH1);
    shieldRefreshHud(1);
}

u16 powerups_shieldActive(void) {
    return shieldActive;
}
