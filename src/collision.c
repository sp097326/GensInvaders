#include "collision.h"
#include "bullets.h"
#include "enemies.h"
#include "player.h"
#include "hud.h"
#include "powerups.h"
#include "explosions.h"

int collideEntities(GameEntity* a, GameEntity* b) {
    return (a->x < b->x + b->w &&
    a->x + a->w > b->x &&
    a->y < b->y + b->h &&
    a->y + a->h >= b->y);
}

u16 collision_update(int* score) {
    GameEntity* b;
    GameEntity* e;
    int i = 0;
    int j = 0;

    /* Player bullets vs enemies. bullets[] is now the player-only pool. */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        b = bullets_playerAt(i);
        if (b->health > 0) {
            for (j = 0; j < MAX_ENEMIES; j++)
            {
                e = enemies_at(j);
                if (e->health > 0) {
                    if (collideEntities(b,e)) {
                        explosions_spawn(e->x, e->y);
                        killGameEntity(e);
                        bullets_despawnPlayer(b);
                        enemies_decLeft();
                        *score += e->points;
                        hud_updateScore(*score, enemies_left(), player_lives());
                        if (enemies_left() == 0) {
                            /* Wave cleared: the caller starts the next one
                             * instead of ending the match with YOU WIN. */
                            return COLLISION_WIN;
                        }
                        break;
                    }
                }
            }
        }
    }

    /* Each enemy bullet has its own collision against the player. A live
     * shield uses a 32x32 box around the ship: the bullet is destroyed and
     * the player is not hurt. Body contact with an enemy is handled below
     * and is always fatal; the shield does not apply there. */
    {
        GameEntity* p = player_get();
        GameEntity shieldBox;

        shieldBox.x = p->x - SHIELD_OFFSET;
        shieldBox.y = p->y - SHIELD_OFFSET;
        shieldBox.w = SHIELD_SPRITE_SIZE;
        shieldBox.h = SHIELD_SPRITE_SIZE;

        for (i = 0; i < MAX_ENEMY_BULLETS; i++)
        {
            b = bullets_enemyAt(i);
            if (b->health > 0 && p->health > 0) {
                if (powerups_shieldActive() && collideEntities(b, &shieldBox)) {
                    bullets_despawnEnemy(b);
                    continue;
                }
                if (player_invulnTimer() == 0 && collideEntities(b, p)) {
                    bullets_despawnEnemy(b);
                    if (player_hit(*score)) {
                        return COLLISION_GAME_OVER;
                    }
                    return COLLISION_NONE;
                }
            }
        }
    }

    /* Invasion: overlapping a living enemy is always GAME OVER (shield and
     * invulnerability do not protect). The explosion plays first; the caller
     * shows GAME OVER when player_update returns 1. */
    {
        GameEntity* p = player_get();
        u16 crashed = 0;

        if (p->health > 0 && !player_isExploding()) {
            for (j = 0; j < MAX_ENEMIES; j++) {
                e = enemies_at(j);
                if (e->health > 0 && collideEntities(e, p)) {
                    explosions_spawn(e->x, e->y);
                    killGameEntity(e);
                    enemies_decLeft();
                    crashed = 1;
                }
            }
            /* If a ship slipped past the player without touching, ending at
             * the visible bottom avoids a stalemate below the gun line. */
            if (!crashed) {
                for (j = 0; j < MAX_ENEMIES; j++) {
                    e = enemies_at(j);
                    if (e->health > 0 && (e->y + e->h >= BOTTOM_EDGE)) {
                        explosions_spawn(e->x, e->y);
                        killGameEntity(e);
                        enemies_decLeft();
                        crashed = 1;
                    }
                }
            }
            if (crashed) {
                player_hitFatal(*score);
                return COLLISION_NONE;
            }
        }
    }
    return COLLISION_NONE;
}
