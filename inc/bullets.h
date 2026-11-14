#ifndef BULLETS_H
#define BULLETS_H

#include "game.h"

/* Player and enemy bullet pools. Data lives in bullets.c. */

void bullets_init(void);
void bullets_reset(void);
void bullets_hideAll(void);
/* Hide every enemy bullet and reset the volley ticker. Player shots stay. */
void bullets_clearEnemy(void);
void bullets_update(void);
void bullets_shootPlayer(GameEntity shooter);
/* One 24x24 piercing shot at a time; does not use the RapidFire cap.
 * Returns 1 if it actually fired. */
u16  bullets_shootCharged(GameEntity shooter);
u16  bullets_chargedActive(void);
void bullets_shootEnemy(GameEntity* shooter);
/* Like shootEnemy, but at an explicit muzzle and up to MAX_ENEMY_BULLETS
 * (used by the boss, whose volley can exceed the formation cap). */
void bullets_shootEnemyXY(s16 x, s16 y);
GameEntity* bullets_charged(void);

void bullets_tickShot(void);
void bullets_clearShotTicker(void);
u16  bullets_shotTicker(void);
u16  bullets_enemyOnScreen(void);

void bullets_setMaxPlayer(u16 n);
u16  bullets_maxPlayer(void);

GameEntity* bullets_playerAt(u16 i);
GameEntity* bullets_enemyAt(u16 i);
void bullets_despawnPlayer(GameEntity* b);
void bullets_despawnEnemy(GameEntity* b);

#endif
