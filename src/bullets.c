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
static GameEntity chargedBullet;
static GameEntity bones[MAX_BOSS_BONES];

/* Bone colours into the free PAL1 slots 7-9 (see config.h). Called at init
 * and again on every spawn, so nothing that reloads PAL1 can leave the
 * bone with the wrong colours. */
static void loadBonePal(TransferMethod tm) {
    PAL_setColors(16 + BOSS_BONE_PAL_FIRST,
                  bossbone.palette->data + BOSS_BONE_PAL_FIRST,
                  BOSS_BONE_PAL_COUNT, tm);
}

static void killBones(void) {
    u16 i;
    for (i = 0; i < MAX_BOSS_BONES; i++) {
        killGameEntity(&bones[i]);
    }
}

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
    /* One 24x24 piercing bolt (3x3 tiles, 1 hardware sprite). Dedicated
     * slot so it never fights the RapidFire cap or the 8x8 pool. */
    chargedBullet.x = 0;
    chargedBullet.y = -40;
    chargedBullet.w = CHARGED_BULLET_SIZE;
    chargedBullet.h = CHARGED_BULLET_SIZE;
    chargedBullet.health = 0;
    chargedBullet.vely = CHARGED_BULLET_SPEED;
    chargedBullet.sprite = SPR_addSprite(&bulletc, 0, -40, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    SPR_setVisibility(chargedBullet.sprite, HIDDEN);
    sprintf(chargedBullet.name, "ChB");
    /* Boss bones: 48x16 = 6x2 tiles, 2 hardware sprites each (32+16 wide),
     * 12 VRAM tiles each. PAL1 so the boss hit tint (PAL2) never reaches it. */
    for (i = 0; i < MAX_BOSS_BONES; i++) {
        bones[i].x = 0;
        bones[i].y = -BOSS_BONE_H;
        bones[i].w = BOSS_BONE_W;
        bones[i].h = BOSS_BONE_H;
        bones[i].velx = 0;
        bones[i].vely = 1;
        bones[i].health = 0;
        bones[i].sprite = SPR_addSprite(&bossbone, 0, -BOSS_BONE_H, TILE_ATTR(PAL1, 0, FALSE, FALSE));
        SPR_setVisibility(bones[i].sprite, HIDDEN);
        sprintf(bones[i].name, "Bn%d", i);
    }
    loadBonePal(DMA);
}

void bullets_reset(void) {
    u16 i;
    for (i = 0; i < MAX_BULLETS; i++) {
        killGameEntity(&bullets[i]);
    }
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        killGameEntity(&enemyBullets[i]);
    }
    killGameEntity(&chargedBullet);
    killBones();
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
    killGameEntity(&chargedBullet);
    killBones();
    bulletsOnScreen = 0;
    shotByPlayer = 0;
    enemyBulletsOnScreen = 0;
}

void bullets_clearEnemy(void) {
    u16 i;
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        killGameEntity(&enemyBullets[i]);
    }
    killBones();
    enemyBulletsOnScreen = 0;
    shotTicker = 0;
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
            bllt->x += bllt->velx;
            bllt->y += bllt->vely;
            if (bllt->y > BOTTOM_EDGE || bllt->x < LEFT_EDGE - 8 || bllt->x > RIGHT_EDGE) {
                killGameEntity(bllt);
                enemyBulletsOnScreen--;
            } else {
                SPR_setPosition(bllt->sprite, bllt->x, bllt->y);
            }
        }
    }
    for (i = 0; i < MAX_BOSS_BONES; i++)
    {
        bllt = &bones[i];
        if (bllt->health > 0) {
            bllt->y += bllt->vely;
            if (bllt->y > BOTTOM_EDGE) {
                killGameEntity(bllt);
            } else {
                SPR_setPosition(bllt->sprite, bllt->x, bllt->y);
            }
        }
    }
    if (chargedBullet.health > 0) {
        chargedBullet.y += chargedBullet.vely;
        if (chargedBullet.y + chargedBullet.h < 0) {
            killGameEntity(&chargedBullet);
        } else {
            SPR_setPosition(chargedBullet.sprite, chargedBullet.x, chargedBullet.y);
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

u16 bullets_shootCharged(GameEntity shooter) {
    if (chargedBullet.health > 0) {
        return 0;
    }
    /* Center the 24x24 bolt on the 16x16 ship. */
    chargedBullet.x = shooter.x - 4;
    chargedBullet.y = shooter.y - 4;
    chargedBullet.vely = CHARGED_BULLET_SPEED;
    reviveGameEntity(&chargedBullet);
    XGM_startPlayPCM(SFX_CHARGE_SHOT, 11, SOUND_PCM_CH3);
    SPR_setPosition(chargedBullet.sprite, chargedBullet.x, chargedBullet.y);
    return 1;
}

u16 bullets_chargedActive(void) {
    return (chargedBullet.health > 0) ? 1 : 0;
}

GameEntity* bullets_charged(void) {
    return &chargedBullet;
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
            bullet->velx = 0;
            bullet->vely = cfg->bulletSpeed;
            reviveGameEntity(bullet);
            SPR_setPosition(bullet->sprite, bullet->x, bullet->y);
            enemyBulletsOnScreen++;
            break;
        }
    }
}

void bullets_shootEnemyXY(s16 x, s16 y) {
    GameEntity* bullet;
    u16 i;
    const DifficultyConfig* cfg = difficulty_config();

    if (enemyBulletsOnScreen >= MAX_ENEMY_BULLETS) {
        return;
    }
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        bullet = &enemyBullets[i];
        if (bullet->health == 0) {
            bullet->x = x;
            bullet->y = y;
            bullet->velx = 0;
            bullet->vely = cfg->bulletSpeed;
            reviveGameEntity(bullet);
            SPR_setPosition(bullet->sprite, bullet->x, bullet->y);
            enemyBulletsOnScreen++;
            break;
        }
    }
}

/* Boss fan shot: explicit muzzle and velocity (diagonal bullets).
 * Returns 1 if a free slot was found. */
u16 bullets_shootEnemyXYV(s16 x, s16 y, s16 vx, s16 vy) {
    GameEntity* bullet;
    u16 i;

    if (enemyBulletsOnScreen >= MAX_ENEMY_BULLETS) {
        return 0;
    }
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        bullet = &enemyBullets[i];
        if (bullet->health == 0) {
            bullet->x = x;
            bullet->y = y;
            bullet->velx = vx;
            bullet->vely = vy;
            reviveGameEntity(bullet);
            SPR_setPosition(bullet->sprite, bullet->x, bullet->y);
            enemyBulletsOnScreen++;
            return 1;
        }
    }
    return 0;
}

u16 bullets_shootBone(s16 x, s16 y) {
    const DifficultyConfig* cfg = difficulty_config();
    s16 vy = (s16)cfg->bulletSpeed - BOSS_BONE_SLOWER;
    u16 i;

    if (vy < 1) vy = 1;
    for (i = 0; i < MAX_BOSS_BONES; i++) {
        GameEntity* b = &bones[i];
        if (b->health == 0) {
            b->x = x;
            b->y = y;
            b->velx = 0;
            b->vely = vy;
            loadBonePal(DMA);
            reviveGameEntity(b);
            SPR_setPosition(b->sprite, b->x, b->y);
            return 1;
        }
    }
    return 0;
}

GameEntity* bullets_boneAt(u16 i) {
    return &bones[i];
}

void bullets_boneHitbox(GameEntity* bone, GameEntity* out) {
    out->x = bone->x + BOSS_BONE_HIT_OX;
    out->y = bone->y + BOSS_BONE_HIT_OY;
    out->w = BOSS_BONE_HIT_W;
    out->h = BOSS_BONE_HIT_H;
}

void bullets_despawnBone(GameEntity* b) {
    killGameEntity(b);
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
