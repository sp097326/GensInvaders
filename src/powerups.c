#include <genesis.h>
#include "powerups.h"
#include "bullets.h"

/* Reserved for a future on-screen pickup; spawn/position are still stubs. */
static GameEntity powerup = {0, 0, 8, 8, 0, 0, 0, NULL, 0, 0, ""};
static u16 powerupTimer = 0;
static bool powerOn = 0;

void powerups_init(void) {
    PAL_setColor(34, RGB24_TO_VDPCOLOR(0x0078f8));
    (void)powerup; /* reserved for a future on-screen pickup */
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
}

void powerups_reset(void) {
    if (powerOn) powerups_deactivate();
    powerupTimer = 0;
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
