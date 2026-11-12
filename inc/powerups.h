#ifndef POWERUPS_H
#define POWERUPS_H

#include "game.h"

typedef enum
{
    RapidFire
} Powerup;

void powerups_init(void);
void powerups_reset(void);
void powerups_hide(void);
void powerups_update(void);
void powerups_activate(Powerup type);
void powerups_deactivate(void);
void powerups_toggle(void);
u16  powerups_isOn(void);
/* Button C: start the shield if it is charged and the player is alive. */
void powerups_tryShield(void);
u16  powerups_shieldActive(void);

#endif
