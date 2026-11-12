#include <genesis.h>
#include <resources.h>
#include "bullets.h"
#include "difficulty.h"

static GameEntity bullets[MAX_BULLETS];
static GameEntity enemyBullets[MAX_ENEMY_BULLETS];
static u16 bulletsOnScreen = 0;
static u16 enemyBulletsOnScreen = 0;
static u16 shotTicker = 0;
static u16 shotByPlayer = 0;
static u16 maxPlayerBullets = MAX_PLAYER_BULLETS;

void bullets_init(void) {
    u16 i;
    GameEntity* bullt = bullets;
    for (i = 0; i < MAX_BULLETS; i++)
    {
        bullt->x = 0;
        bullt->y = -10;
        bullt->w = 8;
        bullt->h = 8;
        bullt->health = 0;
        bullt->sprite = SPR_addSprite(&bullet, 0, -10, TILE_ATTR(PAL1, 0, FALSE, FALSE));
        SPR_setVisibility(bullt->sprite, HIDDEN);
        sprintf(bullt->name, "Bu%d",i);
        bullt++;
    }
    bullt = enemyBullets;
    for (i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        bullt->x = 0;
        bullt->y = -10;
        bullt->w = 8;
        bullt->h = 8;
        bullt->health = 0;
        bullt->sprite = SPR_addSprite(&bullet, 0, -10, TILE_ATTR(PAL1, 0, FALSE, FALSE));
        SPR_setVisibility(bullt->sprite, HIDDEN);
        sprintf(bullt->name, "EB%d", i);
        bullt++;
    }
}

void bullets_reset(void) {
    u16 i;
    for (i = 0; i < MAX_BULLETS; i++) {
        killGameEntity(&bullets[i]);
    }
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        killGameEntity(&enemyBullets[i]);
    }
    bulletsOnScreen = 0;
    shotByPlayer = 0;
    enemyBulletsOnScreen = 0;
    shotTicker = 0;
}

void bullets_hideAll(void) {
    u16 i;
    for (i = 0; i < MAX_BULLETS; i++) {
        killGameEntity(&bullets[i]);
    }
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        killGameEntity(&enemyBullets[i]);
    }
    bulletsOnScreen = 0;
    shotByPlayer = 0;
    enemyBulletsOnScreen = 0;
}

void bullets_update(void) {
    u16 i = 0;
    GameEntity *bllt;
    for (i = 0; i < MAX_BULLETS; i++)
    {
        bllt = &bullets[i];
        if (bllt->health > 0) {
            bllt->y += bllt->vely;
            if (bllt->y + bllt->h < 0) {
                killGameEntity(bllt);
                bulletsOnScreen--;
                shotByPlayer--;
            } else {
                SPR_setPosition(bllt->sprite, bllt->x, bllt->y);
            }
        }
    }
    for (i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        bllt = &enemyBullets[i];
        if (bllt->health > 0) {
            bllt->y += bllt->vely;
            if (bllt->y > BOTTOM_EDGE) {
                killGameEntity(bllt);
                enemyBulletsOnScreen--;
            } else {
                SPR_setPosition(bllt->sprite, bllt->x, bllt->y);
            }
        }
    }
}

void bullets_shootPlayer(GameEntity shooter) {
    GameEntity* bullet;
    u16 i;

    if (shotByPlayer >= maxPlayerBullets) {
        return;
    }
    if (bulletsOnScreen >= MAX_BULLETS) {
        return;
    }
    for (i = 0; i < MAX_BULLETS; i++) {
        bullet = &bullets[i];
        if (bullet->health == 0) {
            bullet->x = shooter.x + 4;
            bullet->y = shooter.y;
            bullet->vely = -3;
            reviveGameEntity(bullet);
            XGM_startPlayPCM(SFX_LASER, 9, SOUND_PCM_CH3);
            SPR_setPosition(bullet->sprite, bullet->x, bullet->y);
            bulletsOnScreen++;
            shotByPlayer++;
            break;
        }
    }
}

void bullets_shootEnemy(GameEntity* shooter) {
    GameEntity* bullet;
    u16 i;
    const DifficultyConfig* cfg = difficulty_config();

    if (enemyBulletsOnScreen >= cfg->maxEnemyBullets) {
        return;
    }
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        bullet = &enemyBullets[i];
        if (bullet->health == 0) {
            bullet->x = shooter->x + 4;
            bullet->y = shooter->y;
            bullet->vely = cfg->bulletSpeed;
            reviveGameEntity(bullet);
            SPR_setPosition(bullet->sprite, bullet->x, bullet->y);
            enemyBulletsOnScreen++;
            break;
        }
    }
}

void bullets_tickShot(void) {
    shotTicker++;
}

void bullets_clearShotTicker(void) {
    shotTicker = 0;
}

u16 bullets_shotTicker(void) {
    return shotTicker;
}

u16 bullets_enemyOnScreen(void) {
    return enemyBulletsOnScreen;
}

void bullets_setMaxPlayer(u16 n) {
    maxPlayerBullets = n;
}

u16 bullets_maxPlayer(void) {
    return maxPlayerBullets;
}

GameEntity* bullets_playerAt(u16 i) {
    return &bullets[i];
}

GameEntity* bullets_enemyAt(u16 i) {
    return &enemyBullets[i];
}

void bullets_despawnPlayer(GameEntity* b) {
    killGameEntity(b);
    bulletsOnScreen--;
    shotByPlayer--;
}

void bullets_despawnEnemy(GameEntity* b) {
    killGameEntity(b);
    enemyBulletsOnScreen--;
}
