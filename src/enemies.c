#include <genesis.h>
#include <resources.h>
#include "enemies.h"
#include "difficulty.h"
#include "bullets.h"

/* Per-row look and scoring. Swap spriteDef later for a different ship graphic;
 * for now both rows use &ship, with PAL3 (tinted) on the bottom row. */
typedef struct
{
    const SpriteDefinition* spriteDef;
    u16 palette;
    u16 vFlip;
    u16 points;
} EnemyRowType;

static const EnemyRowType enemyRows[ENEMY_ROWS] = {
    { &ship, PAL2, TRUE, 20 },  /* top row: original colors, 20 points */
    { &ship, PAL3, TRUE, 10 }   /* bottom row: green/yellow tint, 10 points */
};

static GameEntity enemies[MAX_ENEMIES];
static u16 enemiesLeft = 0;
static s16 enemyDir = 1;       // formation direction and speed: +speed right, -speed left
static s16 enemyRowDir[ENEMY_ROWS];  // per-row dir when rowsOpposite is set

/* Last volley's columns, so the next one can avoid repeating them. */
static u16 lastShotCol[ENEMIES_PER_ROW];
static u16 lastShotCount = 0;

/* High bits of SGDK's 16-bit xorshift; (random() % n) leans on the low bits and can clump. */
static u16 randRange(u16 n) {
    if (n <= 1) return 0;
    return (u16)(((u32)random() * (u32)n) >> 16);
}

static u16 columnWasInLastVolley(u16 col) {
    u16 i;
    for (i = 0; i < lastShotCount; i++) {
        if (lastShotCol[i] == col) return 1;
    }
    return 0;
}

/* Fires a volley of distinct living enemies, capped by the current difficulty.
 * One shot per eligible column (bottom-most living ship). Prefers columns that
 * did not fire in the previous volley so the same ships do not keep shooting. */
static void enemiesTryShoot() {
    const DifficultyConfig* cfg = difficulty_config();
    u16 freshIdx[ENEMIES_PER_ROW];
    u16 freshCol[ENEMIES_PER_ROW];
    u16 freshCount = 0;
    u16 reuseIdx[ENEMIES_PER_ROW];
    u16 reuseCol[ENEMIES_PER_ROW];
    u16 reuseCount = 0;
    u16 firedCol[ENEMIES_PER_ROW];
    u16 firedCount = 0;
    u16 i, n, pick, shots, slots, col, row, chosenIdx;
    s16 chosen;
    GameEntity* enemy;

    if (bullets_shotTicker() < cfg->shotInterval) return;

    /* Consume the interval even if the roll misses or the pool is full,
     * so later frames don't retry every tick. */
    bullets_clearShotTicker();

    if (cfg->fireChance < 10 && (randRange(10) >= cfg->fireChance)) {
        return;
    }

    /* Prefer the bottom-most living enemy in each column, so the front row
     * shoots first (and a hole in the front uncovers the ship behind it). */
    for (col = 0; col < ENEMIES_PER_ROW; col++) {
        chosen = -1;
        for (row = 0; row < ENEMY_ROWS; row++) {
            i = row * ENEMIES_PER_ROW + col;
            if (enemies[i].health > 0) {
                if (chosen < 0 || enemies[i].y > enemies[chosen].y) {
                    chosen = (s16)i;
                }
            }
        }
        if (chosen >= 0) {
            chosenIdx = (u16)chosen;
            if (columnWasInLastVolley(col)) {
                reuseIdx[reuseCount] = chosenIdx;
                reuseCol[reuseCount] = col;
                reuseCount++;
            } else {
                freshIdx[freshCount] = chosenIdx;
                freshCol[freshCount] = col;
                freshCount++;
            }
        }
    }
    if ((freshCount + reuseCount) == 0) return;

    slots = 0;
    if (cfg->maxEnemyBullets > bullets_enemyOnScreen()) {
        slots = cfg->maxEnemyBullets - bullets_enemyOnScreen();
    }
    shots = cfg->shotsPerVolley;
    if (shots > slots) shots = slots;
    if (shots > (freshCount + reuseCount)) shots = freshCount + reuseCount;

    for (n = 0; n < shots; n++) {
        if (freshCount > 0) {
            pick = randRange(freshCount);
            enemy = &enemies[freshIdx[pick]];
            firedCol[firedCount] = freshCol[pick];
            firedCount++;
            freshCount--;
            freshIdx[pick] = freshIdx[freshCount];
            freshCol[pick] = freshCol[freshCount];
        } else {
            pick = randRange(reuseCount);
            enemy = &enemies[reuseIdx[pick]];
            firedCol[firedCount] = reuseCol[pick];
            firedCount++;
            reuseCount--;
            reuseIdx[pick] = reuseIdx[reuseCount];
            reuseCol[pick] = reuseCol[reuseCount];
        }
        bullets_shootEnemy(enemy);
    }

    lastShotCount = firedCount;
    for (i = 0; i < firedCount; i++) {
        lastShotCol[i] = firedCol[i];
    }
}

static u16 rowHasLiving(u16 row) {
    u16 col;
    for (col = 0; col < ENEMIES_PER_ROW; col++) {
        if (enemies[row * ENEMIES_PER_ROW + col].health > 0) return 1;
    }
    return 0;
}

static s16 rowMinY(u16 row) {
    u16 col;
    s16 minY = 32767;
    GameEntity* e;
    for (col = 0; col < ENEMIES_PER_ROW; col++) {
        e = &enemies[row * ENEMIES_PER_ROW + col];
        if (e->health > 0 && e->y < minY) minY = e->y;
    }
    return minY;
}

/* Keep at least ENEMY_ROW_SPACING between a row and the next living row below
 * so they never share a scanline. If the row below is gone, this row may drop. */
static u16 rowCanStepDown(u16 row, u16 step) {
    u16 r;
    s16 thisY, belowY;
    if (step == 0 || !rowHasLiving(row)) return 0;
    thisY = rowMinY(row);
    for (r = row + 1; r < ENEMY_ROWS; r++) {
        if (rowHasLiving(r)) {
            belowY = rowMinY(r);
            if (belowY - (thisY + (s16)step) < (s16)ENEMY_ROW_SPACING) {
                return 0;
            }
            return 1;
        }
    }
    return 1;
}
/* rowsOpposite: each row has its own direction, bounces on its own living
 * edges, and only that row steps down (if it would not crowd the row below). */
static u16 positionEnemyRowsOpposite() {
    const DifficultyConfig* cfg = difficulty_config();
    u16 step = cfg->stepDown;
    u16 row, col, i;
    GameEntity* enemy;
    u16 reachedPlayer = 0;
    u16 hitEdge;
    s16 dir;

    for (row = 0; row < ENEMY_ROWS; row++) {
        if (!rowHasLiving(row)) continue;
        dir = enemyRowDir[row];
        hitEdge = 0;
        for (col = 0; col < ENEMIES_PER_ROW; col++) {
            enemy = &enemies[row * ENEMIES_PER_ROW + col];
            if (enemy->health > 0) {
                if ((dir > 0 && enemy->x + enemy->w >= RIGHT_EDGE) ||
                    (dir < 0 && enemy->x <= LEFT_EDGE)) {
                    hitEdge = 1;
                    break;
                }
            }
        }
        if (hitEdge) {
            enemyRowDir[row] = (s16)(-dir);
            if (rowCanStepDown(row, step)) {
                for (col = 0; col < ENEMIES_PER_ROW; col++) {
                    enemy = &enemies[row * ENEMIES_PER_ROW + col];
                    if (enemy->health > 0) {
                        enemy->y += step;
                        if (enemy->y + enemy->h >= PLAYER_START_Y) {
                            reachedPlayer = 1;
                        }
                    }
                }
            }
        }
    }

    if (reachedPlayer) {
        for (i = 0; i < MAX_ENEMIES; i++) {
            enemy = &enemies[i];
            if (enemy->health > 0) {
                SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
            }
        }
        return 1;
    }

    for (row = 0; row < ENEMY_ROWS; row++) {
        dir = enemyRowDir[row];
        for (col = 0; col < ENEMIES_PER_ROW; col++) {
            enemy = &enemies[row * ENEMIES_PER_ROW + col];
            if (enemy->health > 0) {
                enemy->x += dir;
                SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
            }
        }
    }
    return 0;
}
u16 enemies_update() {
    bullets_tickShot();

    if (difficulty_config()->rowsOpposite) {
        if (positionEnemyRowsOpposite()) return 1;
        enemiesTryShoot();
        return 0;
    }

    {
    u16 i = 0;
    GameEntity* enemy;
    u16 hitEdge = 0;
    u16 reachedPlayer = 0;
    u16 step = (*difficulty_config()).stepDown;

    /* Formation movement (classic Space Invaders): if ANY living enemy reaches
     * an edge, the whole group turns around and, depending on difficulty,
     * steps down together. If they reach the player's row, it's GAME OVER. */
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemy = &enemies[i];
        if (enemy->health > 0) {
            if ((enemyDir > 0 && enemy->x + enemy->w >= RIGHT_EDGE) ||
                (enemyDir < 0 && enemy->x <= LEFT_EDGE)) {
                enemyDir = -enemyDir;
                hitEdge = 1;
                break;
            }
        }
    }

    if (hitEdge && step > 0) {
        for (i = 0; i < MAX_ENEMIES; i++) {
            enemy = &enemies[i];
            if (enemy->health > 0) {
                enemy->y += step;
                /* The bottom of the sprite reached the player's starting row. */
                if (enemy->y + enemy->h >= PLAYER_START_Y) {
                    reachedPlayer = 1;
                }
            }
        }
        if (reachedPlayer) {
            for (i = 0; i < MAX_ENEMIES; i++) {
                enemy = &enemies[i];
                if (enemy->health > 0) {
                    SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
                }
            }
            return 1;
        }
    }

    for (i = 0; i < MAX_ENEMIES; i++) {
        enemy = &enemies[i];
        if (enemy->health > 0) {
            enemy->x += enemyDir;
            SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
        }
    }
    }

    enemiesTryShoot();
    return 0;
}
/* Puts every enemy back in its starting slot (used at boot and on restart). */
static void placeEnemies() {
    u16 row, col, i;
    GameEntity* enem;
    const EnemyRowType* type;
    enemiesLeft = 0;
    for (row = 0; row < ENEMY_ROWS; row++) {
        type = &enemyRows[row];
        for (col = 0; col < ENEMIES_PER_ROW; col++) {
            i = row * ENEMIES_PER_ROW + col;
            enem = &enemies[i];
            enem->x = col * ENEMY_COL_SPACING;
            enem->y = ENEMY_START_Y + row * ENEMY_ROW_SPACING;
            enem->points = type->points;
            enem->row = row;
            /* Odd rows start on the right when they will move left, so they
             * are not already sitting on the left edge (which would bounce
             * and step down on the first frame). */
            if ((*difficulty_config()).rowsOpposite && (row & 1)) {
                enem->x += (RIGHT_EDGE - ((ENEMIES_PER_ROW - 1) * ENEMY_COL_SPACING + enem->w));
            }
            reviveGameEntity(enem);
            SPR_setPosition(enem->sprite, enem->x, enem->y);
            enemiesLeft++;
        }
    }
}

void enemies_init() {
    /* Creating Enemy Sprites — one SpriteDefinition / palette / score per row. */
    u16 row, col, i;
    GameEntity* enem;
    const EnemyRowType* type;
    enemiesLeft = 0;
    for (row = 0; row < ENEMY_ROWS; row++) {
        type = &enemyRows[row];
        for (col = 0; col < ENEMIES_PER_ROW; col++) {
            i = row * ENEMIES_PER_ROW + col;
            enem = &enemies[i];
            enem->x = col * ENEMY_COL_SPACING;
            enem->y = ENEMY_START_Y + row * ENEMY_ROW_SPACING;
            enem->w = 16;
            enem->h = 16;
            enem->velx = 1;
            enem->health = 1;
            enem->points = type->points;
            enem->row = row;
            enem->sprite = SPR_addSprite(type->spriteDef, enem->x, enem->y,
                TILE_ATTR(type->palette, 0, type->vFlip, FALSE));
            sprintf(enem->name, "En%d", i);
            enemiesLeft++;
        }
    }
}

/* PAL3 is the title logo on the menu and the bottom-row tint in play. */
void enemies_setRowPalettes() {
    PAL_setPalette(PAL3, background.palette->data, CPU);
    PAL_setColor(48 + 1, RGB24_TO_VDPCOLOR(0x00E800)); /* green instead of blue */
    PAL_setColor(48 + 2, RGB24_TO_VDPCOLOR(0xF8F800)); /* yellow instead of orange */
}

void enemies_reset(void) {
    const DifficultyConfig* cfg = difficulty_config();
    s16 speed = (s16)cfg->enemySpeed;
    u16 row;
    lastShotCount = 0;
    enemyDir = speed;
    for (row = 0; row < ENEMY_ROWS; row++) {
        /* Even rows start right; odd rows start left when rowsOpposite. */
        enemyRowDir[row] = (cfg->rowsOpposite && (row & 1)) ? (s16)(-speed) : speed;
    }
    enemies_setRowPalettes();
    placeEnemies();
}

void enemies_hideAll(void) {
    u16 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        killGameEntity(&enemies[i]);
    }
}

GameEntity* enemies_at(u16 i) {
    return &enemies[i];
}

u16 enemies_left(void) {
    return enemiesLeft;
}

void enemies_decLeft(void) {
    enemiesLeft--;
}

