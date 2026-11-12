#include "collision.h"
#include "bullets.h"
#include "enemies.h"
#include "player.h"
#include "hud.h"

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
                        killGameEntity(e);
                        bullets_despawnPlayer(b);
                        enemies_decLeft();
                        *score += e->points;
                        hud_updateScore(*score, enemies_left(), player_lives());
                        if (enemies_left() == 0) {
                            return COLLISION_WIN;
                        }
                        break;
                    }
                }
            }
        }
    }

    /* Each enemy bullet has its own collision against the player. */
    for (i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        b = bullets_enemyAt(i);
        if (b->health > 0 && player_get()->health > 0) {
            if (player_invulnTimer() == 0 && collideEntities(b, player_get())) {
                bullets_despawnEnemy(b);
                if (player_hit(*score)) {
                    return COLLISION_GAME_OVER;
                }
                return COLLISION_NONE;
            }
        }
    }
    return COLLISION_NONE;
}
