#ifndef GENSINV_BOSS_H
#define GENSINV_BOSS_H

#include "game.h"

void boss_init(void);
void boss_reset(void);
void boss_hide(void);
/* Wave 10, 20, 30... */
u16  boss_isWave(u16 wave);
u16  boss_isActive(void);
/* Like the formation enter: start `offset` pixels above home, then slide in. */
void boss_beginEnter(u16 offset);
void boss_advanceEnter(u16 remain, u16 total, u16 offset);
void boss_update(void);
/* Player shot vs mothership. charged != 0 is the piercing bolt (3x damage,
 * one hit per pass). Returns 0 = no overlap, 1 = wing (no damage),
 * 2 = core hit, 3 = core hit that destroyed the boss. */
u16  boss_hitShot(GameEntity* shot, u16 charged);
u16  boss_hp(void);
u16  boss_hpMax(void);
u16  boss_points(void);

#endif
