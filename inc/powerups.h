#ifndef POWERUPS_H
#define POWERUPS_H

#include "game.h"

typedef enum
{
    RapidFire
} Powerup;

void powerups_init(void);
void powerups_reset(void);
void powerups_update(void);
void powerups_activate(Powerup type);
void powerups_deactivate(void);
void powerups_toggle(void);
u16  powerups_isOn(void);

#endif
