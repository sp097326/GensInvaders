#ifndef POWERUPS_H
#define POWERUPS_H

#include "game.h"

void powerups_init(void);
void powerups_reset(void);
void powerups_hide(void);
void powerups_update(void);
void powerups_deactivate(void);
u16  powerups_isOn(void);
/* Try to spawn a RapidFire capsule at the destroyed enemy. No-op if one
 * is already falling. The caller skips this when the kill cleared the wave. */
void powerups_tryDrop(s16 x, s16 y);
/* Hide a falling capsule (warp, death, title, GAME OVER). The RapidFire
 * timer itself is not cleared. */
void powerups_clearCapsule(void);
/* Button C: start the shield if it is charged and the player is alive. */
void powerups_tryShield(void);
u16  powerups_shieldActive(void);
/* CH1 hum: mute on pause / title / GAME OVER; unmute on resume if the shield is still up. */
void powerups_muteShieldSound(void);
void powerups_unmuteShieldSound(void);

#endif
