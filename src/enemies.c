#include <genesis.h>
#include <resources.h>
#include "enemies.h"
#include "difficulty.h"
#include "bullets.h"
#include "player.h"

/* Per-row look. Swap spriteDef later for a different ship graphic;
 * for now both rows use &ship, with PAL3 (tinted) on the bottom row.
 * Points come from the difficulty table (same value for every ship). */
typedef struct
{
    const SpriteDefinition* spriteDef;
    u16 palette;
    u16 vFlip;
} EnemyRowType;

static const EnemyRowType enemyRows[ENEMY_ROWS] = {
    { &ship, PAL2, TRUE },  /* top row: original colors */
    { &ship, PAL3, TRUE },  /* second row: green/yellow tint */
    { &ship, PAL2, TRUE }   /* third row (staggered on 11-19, full on 21+): original
                             * colors for now; a dedicated ship may come later */
};

static GameEntity enemies[MAX_ENEMIES];
static u16 enemiesLeft = 0;
static s16 enemyDir = 1;       // formation direction: +1 right, -1 left
static s16 enemyRowDir[ENEMY_ROWS];  // per-row direction (+1 / -1) when rowsOpposite is set
/* Sub-pixel speed (V0.4.83): enemySpeed is in quarter pixels per frame.
 * Each frame adds it to the accumulator and moves the whole pixels; the
 * remainder carries over (1.25 px/frame moves 1, 1, 1, 2, ...). Every
 * ship and every row moves the same amount on a given frame, so the
 * formation stays aligned and the edge checks see whole-pixel x. */
static u16 enemySubPx = 0;
static s16 enemyStepPx = 1;    // whole pixels to move this frame

static void advanceSubPixel(void) {
    enemySubPx += difficulty_config()->enemySpeed;
    enemyStepPx = (s16)(enemySubPx >> 2);
    enemySubPx &= 3;
}

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
    u16 row, col;
    GameEntity* enemy;
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
                    }
                }
            }
        }
    }

    for (row = 0; row < ENEMY_ROWS; row++) {
        dir = (s16)(enemyRowDir[row] * enemyStepPx);
        for (col = 0; col < ENEMIES_PER_ROW; col++) {
            enemy = &enemies[row * ENEMIES_PER_ROW + col];
            if (enemy->health > 0 && enemy->sprite != NULL) {
                enemy->x += dir;
                SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
            }
        }
    }
    return 0;
}
u16 enemies_update() {
    u16 exploding = player_isExploding();

    if (!exploding) bullets_tickShot();
    advanceSubPixel();

    if (difficulty_config()->rowsOpposite) {
        if (positionEnemyRowsOpposite()) return 1;
        if (!exploding) enemiesTryShoot();
        return 0;
    }

    {
    u16 i = 0;
    GameEntity* enemy;
    u16 hitEdge = 0;
    u16 step = (*difficulty_config()).stepDown;

    /* Formation movement (classic Space Invaders): if ANY living enemy reaches
     * an edge, the whole group turns around and, depending on difficulty,
     * steps down together. GAME OVER is decided in collision (body contact
     * with the player, or a ship reaching the visible bottom). */
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
            }
        }
    }

    for (i = 0; i < MAX_ENEMIES; i++) {
        enemy = &enemies[i];
        if (enemy->health > 0 && enemy->sprite != NULL) {
            enemy->x += (s16)(enemyDir * enemyStepPx);
            SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
        }
    }
    }

    if (!exploding) enemiesTryShoot();
    return 0;
}
/* Row layout per wave. Rows 0-1 are always full. Row 2 is a staggered
 * row (columns 0, 2, 4, 6, aligned with the rows above so the column
 * based targeting is unchanged) on waves 11-19, full from wave 21 on
 * (V0.4.84), and empty on 1-9. Boss waves never place the formation. */
u16 enemies_rowMaskForWave(u16 wave, u16 row) {
    if (row < 2) return ENEMY_ROW_FULL;
    if (row == 2 && wave >= FULL_ROW3_FIRST_WAVE) return ENEMY_ROW_FULL;
    if (row == 2 && wave >= STAGGER_ROW_FIRST_WAVE && wave <= STAGGER_ROW_LAST_WAVE) {
        return ENEMY_ROW_STAGGER;
    }
    return 0;
}

/* Puts every enemy back in its starting slot (used at boot and on restart).
 * Only the slots in this wave's layout are revived and counted; the rest
 * stay hidden with health 0, so every loop that checks health ignores them. */
static void placeEnemies() {
    u16 row, col, i, mask;
    GameEntity* enem;
    u16 wave = difficulty_wave();
    enemiesLeft = 0;
    for (row = 0; row < ENEMY_ROWS; row++) {
        mask = enemies_rowMaskForWave(wave, row);
        for (col = 0; col < ENEMIES_PER_ROW; col++) {
            i = row * ENEMIES_PER_ROW + col;
            enem = &enemies[i];
            enem->x = col * ENEMY_COL_SPACING;
            enem->y = ENEMY_START_Y + row * ENEMY_ROW_SPACING;
            enem->points = difficulty_config()->enemyPoints;
            enem->row = row;
            /* Odd rows start on the right when they will move left, so they
             * are not already sitting on the left edge (which would bounce
             * and step down on the first frame). */
            if ((*difficulty_config()).rowsOpposite && (row & 1)) {
                enem->x += (RIGHT_EDGE - ((ENEMIES_PER_ROW - 1) * ENEMY_COL_SPACING + enem->w));
            }
            if (enem->sprite != NULL) {
                SPR_setPosition(enem->sprite, enem->x, enem->y);
            }
            if (mask & (1 << col)) {
                reviveGameEntity(enem);
                if (enem->health > 0) enemiesLeft++;
            } else {
                killGameEntity(enem);
            }
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
            enem->points = difficulty_config()->enemyPoints;
            enem->row = row;
            enem->sprite = addGameSprite(type->spriteDef, enem->x, enem->y,
                TILE_ATTR(type->palette, 0, type->vFlip, FALSE));
            sprintf(enem->name, "En%d", i);
            /* Slots outside the wave-1 layout (the third row) start hidden.
             * A slot without a sprite stays dead and is never counted. */
            if (enem->sprite != NULL && (enemies_rowMaskForWave(1, row) & (1 << col))) {
                enem->health = 1;
                enemiesLeft++;
            } else {
                enem->health = 0;
                if (enem->sprite != NULL) SPR_setVisibility(enem->sprite, HIDDEN);
            }
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
    u16 row;
    lastShotCount = 0;
    enemyDir = 1;
    enemySubPx = 0;
    enemyStepPx = 1;
    for (row = 0; row < ENEMY_ROWS; row++) {
        /* Even rows start right; odd rows start left when rowsOpposite. */
        enemyRowDir[row] = (cfg->rowsOpposite && (row & 1)) ? -1 : 1;
    }
    enemies_setRowPalettes();
    placeEnemies();
}

void enemies_beginEnter(u16 offset) {
    u16 i;
    enemies_reset();
    if (offset == 0) return;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].y -= (s16)offset;
        if (enemies[i].sprite != NULL) {
            SPR_setPosition(enemies[i].sprite, enemies[i].x, enemies[i].y);
        }
    }
}

void enemies_advanceEnter(u16 remain, u16 total, u16 offset) {
    u16 i;
    GameEntity* enem;
    s16 dest;
    s16 shift;

    if (total == 0) {
        shift = 0;
    } else {
        shift = (s16)(((u32)offset * (u32)remain) / (u32)total);
    }
    for (i = 0; i < MAX_ENEMIES; i++) {
        enem = &enemies[i];
        dest = (s16)(ENEMY_START_Y + enem->row * ENEMY_ROW_SPACING);
        enem->y = dest - shift;
        if (enem->sprite != NULL) {
            SPR_setPosition(enem->sprite, enem->x, enem->y);
        }
    }
}

void enemies_hideAll(void) {
    u16 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        killGameEntity(&enemies[i]);
    }
    enemiesLeft = 0;
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

