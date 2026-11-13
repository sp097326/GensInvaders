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
void bullets_shootEnemy(GameEntity* shooter);

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
