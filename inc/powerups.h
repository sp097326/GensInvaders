#ifndef POWERUPS_H
#define POWERUPS_H

#include "game.h"

void powerups_init(void);
void powerups_reset(void);
void powerups_hide(void);
/* frozen != 0 during the wave warp: the RapidFire time left and an
 * active shield's time left do not run down (they are useless there:
 * no shooting, no enemy fire); recharges and the capsule keep going. */
void powerups_update(u16 frozen);
void powerups_deactivate(void);
u16  powerups_isOn(void);
/* Try to spawn a RapidFire capsule at the destroyed enemy. No-op if one
 * is already falling. The caller skips this when the kill cleared the wave. */
void powerups_tryDrop(s16 x, s16 y);
/* Hide a falling capsule (warp, death, title, GAME OVER). The RapidFire
 * timer itself is not cleared. */
void powerups_clearCapsule(void);
/* Boss reward: always drop a capsule centred on (x, y), clamped to the
 * screen; replaces a capsule already falling. It is kept by
 * powerups_clearWaveCapsule so it falls through the warp. */
void powerups_dropBossCapsule(s16 x, s16 y);
/* Wave change: clear a falling enemy capsule but keep a boss capsule. */
void powerups_clearWaveCapsule(void);
/* Points earned by an E / I capsule caught with its bar already full
 * since the last call (0 if none). Clears the pending value. */
u16  powerups_takeBonus(void);
/* Button C: start the shield if it is charged and the player is alive. */
void powerups_tryShield(void);
u16  powerups_shieldActive(void);
/* CH1 hum: mute on pause / title / GAME OVER; unmute on resume if the shield is still up. */
void powerups_muteShieldSound(void);
void powerups_unmuteShieldSound(void);

#endif
