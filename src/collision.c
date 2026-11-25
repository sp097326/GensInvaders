#include "collision.h"
#include "bullets.h"
#include "enemies.h"
#include "player.h"
#include "hud.h"
#include "powerups.h"
#include "explosions.h"
#include "boss.h"
#include "difficulty.h"

int collideEntities(GameEntity* a, GameEntity* b) {
    return (a->x < b->x + b->w &&
    a->x + a->w > b->x &&
    a->y < b->y + b->h &&
    a->y + a->h >= b->y);
}

/* A live entity must have both health and a sprite. Without a sprite it
 * must never collide or deal damage. */
static u16 isLive(GameEntity* e) {
    return (e != NULL && e->health > 0 && e->sprite != NULL) ? 1 : 0;
}

/* Formation waves end when the last ship dies. A boss wave hides the
 * rows, so enemiesLeft is already 0 for the whole fight: that count
 * must not clear the wave. Only a mothership whose HP has reached 0
 * may, via boss_hitShot. */
static u16 formationCleared(void) {
    if (boss_isActive()) return 0;
    if (boss_isWave(difficulty_wave())) return 0;
    return (enemies_left() == 0) ? 1 : 0;
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
        if (isLive(b)) {
            for (j = 0; j < MAX_ENEMIES; j++)
            {
                e = enemies_at(j);
                if (isLive(e)) {
                    if (collideEntities(b,e)) {
                        explosions_spawn(e->x, e->y);
                        killGameEntity(e);
                        bullets_despawnPlayer(b);
                        enemies_decLeft();
                        *score += e->points;
                        {
                            u16 gained = player_checkExtraLife(*score);
                            hud_updateScore(*score, enemies_left(), player_lives());
                            if (gained) hud_showExtraLife(gained);
                        }
                        if (formationCleared()) {
                            /* Wave cleared: the caller starts the next one
                             * instead of ending the match with YOU WIN. */
                            return COLLISION_WIN;
                        }
                        powerups_tryDrop(e->x, e->y);
                        break;
                    }
                }
            }
        }
    }

    /* Charged bolt: keep flying and destroy every enemy it touches.
     * Points are the usual per-ship value. */
    {
        GameEntity* c = bullets_charged();
        if (isLive(c)) {
            for (j = 0; j < MAX_ENEMIES; j++) {
                e = enemies_at(j);
                if (isLive(e) && collideEntities(c, e)) {
                    explosions_spawn(e->x, e->y);
                    killGameEntity(e);
                    enemies_decLeft();
                    *score += e->points;
                    {
                        u16 gained = player_checkExtraLife(*score);
                        hud_updateScore(*score, enemies_left(), player_lives());
                        if (gained) hud_showExtraLife(gained);
                    }
                    if (enemies_left() > 0) {
                        powerups_tryDrop(e->x, e->y);
                    }
                }
            }
            /* Do not treat "no formation left" as a win just because a
             * charged bolt is in flight: on a boss wave that count is
             * already 0, which used to skip the mothership and jump
             * straight to the next wave. */
            if (formationCleared()) {
                return COLLISION_WIN;
            }
        }
    }

    /* Mothership: only the glowing core takes damage. Normal shots that
     * hit a wing are blocked; the charged bolt flies through the hull
     * and damages the core once per pass. */
    if (boss_isActive()) {
        for (i = 0; i < MAX_BULLETS; i++) {
            b = bullets_playerAt(i);
            if (!isLive(b)) continue;
            {
                u16 r = boss_hitShot(b, 0);
                if (r == 0) continue;
                if (r == 1 || r == 2) {
                    bullets_despawnPlayer(b);
                }
                if (r == 3) {
                    bullets_despawnPlayer(b);
                    *score += boss_points();
                    {
                        u16 gained = player_checkExtraLife(*score);
                        hud_updateScore(*score, enemies_left(), player_lives());
                        if (gained) hud_showExtraLife(gained);
                    }
                    return COLLISION_WIN;
                }
            }
        }
        {
            GameEntity* c = bullets_charged();
            if (isLive(c)) {
                u16 r = boss_hitShot(c, 1);
                if (r == 3) {
                    *score += boss_points();
                    {
                        u16 gained = player_checkExtraLife(*score);
                        hud_updateScore(*score, enemies_left(), player_lives());
                        if (gained) hud_showExtraLife(gained);
                    }
                    return COLLISION_WIN;
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
            if (isLive(b) && p->health > 0) {
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

        /* Boss bones: the shield does NOT stop them (they pass through the
         * bubble), only dodging works. Only the reduced 40x8 hitbox counts,
         * so grazing the edge of the 48x16 art is safe. */
        for (i = 0; i < MAX_BOSS_BONES; i++)
        {
            GameEntity hit;

            b = bullets_boneAt(i);
            if (!isLive(b) || p->health == 0) continue;
            bullets_boneHitbox(b, &hit);
            if (player_invulnTimer() == 0 && collideEntities(&hit, p)) {
                bullets_despawnBone(b);
                if (player_hit(*score)) {
                    return COLLISION_GAME_OVER;
                }
                return COLLISION_NONE;
            }
        }

        /* Turtle dive (waves 10, 30...): body contact costs a life. The shield
         * does not stop it (BOSS2_DIVE_SHIELD_BLOCKS 0), like the bone and
         * the star; respawn invulnerability does. The turtle keeps going. */
        if (p->health > 0 && player_invulnTimer() == 0 && boss_bodyHits(p)) {
            if (!(BOSS2_DIVE_SHIELD_BLOCKS && powerups_shieldActive())) {
                if (player_hit(*score)) {
                    return COLLISION_GAME_OVER;
                }
                return COLLISION_NONE;
            }
        }

        /* Turtle stars (waves 10, 30...): same rule as the bone, the shield does
         * not stop them; only the 10x10 centre of the 16x16 star counts. */
        for (i = 0; i < MAX_BOSS_STARS; i++)
        {
            GameEntity hit;

            b = bullets_starAt(i);
            if (!isLive(b) || p->health == 0) continue;
            bullets_starHitbox(b, &hit);
            if (player_invulnTimer() == 0 && collideEntities(&hit, p)) {
                bullets_despawnStar(b);
                if (player_hit(*score)) {
                    return COLLISION_GAME_OVER;
                }
                return COLLISION_NONE;
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
                if (isLive(e) && collideEntities(e, p)) {
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
                    if (isLive(e) && (e->y + e->h >= BOTTOM_EDGE)) {
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
