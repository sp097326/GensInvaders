/**
 * Gens Invaders
 * Created With Genesis-Code extension for Visual Studio Code
 * Based on ohsat (https://www.ohsat.com/) awesome tutotial.
 * Assets by: taken from ohsat page, 
 * Code by: AMH
 * Last mod by: AMH - 22 / 06/ 2023
 * 
 **/
#include <genesis.h>
#include <resources.h>

//player related vars
typedef struct
{
    /* data */
    int x;
    int y;
    int w;
    int h;
    int velx;
    int vely;
    int health;
    Sprite* sprite;
    char name[6];
} GameEntity;

// Enemy control definitions
#define MAX_ENEMIES 7       // maximum enemies on screen
#define LEFT_EDGE 0         // setting the screen boundries
#define RIGHT_EDGE 320      // in order to control enemie movement-boundries
#define ANIM_STRAIGHT 0
#define ANIM_MOVE 1
#define MAX_BULLETS 6       // maybe we can use this for a turbo power up?
#define MAX_PLAYER_BULLETS 3
#define BOTTOM_EDGE 224
#define SHOT_INTERVAL 120
#define INTRO_MUSIC 66
#define SFX_LASER 64
#define SFX_XPLOSION 65
#define POWERUP_DURATION 100

// on screen entities
GameEntity player = {0, 0, 16, 16, 0, 0, 0, NULL, "PLAYER" };
GameEntity enemies[MAX_ENEMIES];
GameEntity bullets[MAX_BULLETS];
GameEntity powerup = {0,0,8,8,0,0,0, NULL};
u16 enemiesLeft = 0;
u16 bulletsOnScreen = 0;
u16 shotTicker = 0;
u16 shotByPlayer = 0;
u16 maxPlayerBullets = MAX_PLAYER_BULLETS;
u16 powerupTimer = 0;
int score = 0;
char hud_string[40] = "";

typedef enum MyEnum
{
    RapidFire
} Powerup;

// variables for space bkg.
int i=0;
int thex = 0;
int they = 0;
int val = 1;
int offset = 0;

/* Auxiliary functions for game processing */

void updateScoreDisplay() {
    sprintf(hud_string, "SCORE: %d - LEFT: %d", score, enemiesLeft);
    VDP_clearText(0,0,40);
    VDP_drawText(hud_string, 0,0);
}

void killGameEntity(GameEntity* e) {
    e -> health = 0;
    SPR_setVisibility(e -> sprite, HIDDEN);
}

void reviveGameEntity(GameEntity* e) {
    e -> health = 1;
    SPR_setVisibility(e -> sprite, VISIBLE);
}

/* powerup Related */
void spawnPowerupAt(u16 X, u16 Y) {};
void positionPowerup(){};
void activatePowerup(Powerup Type) {
    switch (Type)
    {
    case RapidFire: {
        maxPlayerBullets = MAX_PLAYER_BULLETS;
        powerupTimer = POWERUP_DURATION;
        PAL_setPaletteColors(18, RGB24_TO_VDPCOLOR(0xf83000),DMA);
        break;
    }
    default:
        return;
    }
};
void deactivatePowerup(){
    maxPlayerBullets = MAX_PLAYER_BULLETS;
    PAL_setPaletteColors(18, RGB24_TO_VDPCOLOR(0xf83800), DMA);
};

/* powerup related END */

void positionEnemies() {
    shotTicker++;
    u16 i = 0;
    u16 absHPos = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        GameEntity* enemy = &enemies[i];
        absHPos = enemy->x+enemy->w;
        if (enemy->health > 0) {
            if (absHPos>RIGHT_EDGE){
                enemy->velx = -1;
            } else if ( enemy->x < LEFT_EDGE) {
                enemy->velx = 1;
            }
            enemy->x += enemy->velx;
            SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
            //let's give enemies a chance to shoot!!
            if (shotTicker >= SHOT_INTERVAL) {
                if ( (random() % (10-1+1)+1) > 4) {
                    shootBullet(*enemy);
                    shotTicker = 0;
                }
            }
        }
    }
}

void positionPlayer() {
    //handling veolicity, adding it to position
    player.x += player.velx;

    /* checking to keep the player in the boundries of the screen */
    if (player.x < LEFT_EDGE) player.x = LEFT_EDGE;
    if (player.x + player.w > RIGHT_EDGE) player.x = RIGHT_EDGE - player.w;

    //positioning the srprite using the Sprite Engine in SGDK
    SPR_setPosition(player.sprite, player.x, player.y);
}

void positionBullets() {
    u16 i = 0;
    GameEntity *bllt;
    for ( i = 0; i < MAX_BULLETS; i++)
    {
        bllt = &bullets[i];
        if (bllt->health > 0) {
            bllt->y += bllt->vely;
            if (bllt->y + bllt->h < 0) {
                killGameEntity(bllt);
                bulletsOnScreen--;
                shotByPlayer--;
            } else if (bllt->y > BOTTOM_EDGE) {
                killGameEntity(bllt);
                bulletsOnScreen--;
            } else {
                SPR_setPosition(bllt->sprite, bllt->x, bllt->y);
            }
        }
    }
}

void shootBullet(GameEntity shooter) {

    bool fromPlayer = (shooter.y > 100);

    if (bulletsOnScreen < MAX_BULLETS) {
        if (fromPlayer == TRUE) {
            if (shotByPlayer >= maxPlayerBullets) {
                return;
            }
        }
        GameEntity* bullet;
        u16 i = 0;
        for (i=0; i<MAX_BULLETS; i++) {
            bullet = &bullets[i];
            if (bullet->health == 0) {
                bullet->x = shooter.x + 4;
                bullet->y = shooter.y;
                reviveGameEntity(bullet);
                //this is used to determine velocity according to who shoot the bullet
                if (fromPlayer) {
                    bullet->vely = -3;
                    XGM_startPlayPCM(SFX_LASER,9,SOUND_PCM_CH3);
                    shotByPlayer++;
                } else {
                    bullet->vely = 5;
                }
                SPR_setPosition(bullet->sprite, bullet->x, bullet->y);
                bulletsOnScreen++;
                break;
            }     
        }
    }
}

/* Handler function for the joystick movement, Interrupts are alredy implemented in SGDK */
void myJoyHandler(u16 joy, u16 changed, u16 state) {
    // Control movement for player 1 (can we just add an If for player 2?)
    if (joy == JOY_1){
        if (state & BUTTON_RIGHT) {
            player.velx = 2;
            SPR_setAnim(player.sprite, ANIM_MOVE);
            SPR_setHFlip(player.sprite, TRUE);
        }
        else if (state & BUTTON_LEFT) {
            player.velx = -2;
            SPR_setAnim(player.sprite, ANIM_MOVE);
            SPR_setHFlip(player.sprite, FALSE);
        }
        else {
            if ( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ) {
                player.velx = 0;
                SPR_setAnim(player.sprite, ANIM_STRAIGHT);
            }
        }
        if (state & BUTTON_B & changed) {
            shootBullet(player);
        }
        if (state & BUTTON_A & changed) {
            activatePowerup(RapidFire);
        }
    }
}

int collideEntities(GameEntity* a, GameEntity* b) {
    return (a->x < b->x + b->w && 
    a->x + a->w > b->x && 
    a->y < b->y + b->h &&
    a->y + a->h >= b->y);
}

void handleCollitions() {
    GameEntity* b;
    GameEntity* e;
    int i = 0;
    int j = 0;

    for (i = 0; i < MAX_BULLETS; i++)
    {
        b = &bullets[i];
        if (b->health > 0) {
            if (b->vely < 0) {
                for (j = 0; j < MAX_ENEMIES; j++)
                {
                    e = &enemies[j];
                    if (e->health > 0) {
                        if (collideEntities(b,e)) {
                            //should we shift the animation to an explosion animation and disapear?
                            killGameEntity(e);
                            killGameEntity(b);
                            //place sound of xplosion here for enemy, diff. sound?
                            enemiesLeft--;
                            bulletsOnScreen--;
                            shotByPlayer--;
                            score += 10;
                            updateScoreDisplay();
                            break;                        
                        }
                    }
                }
            } else {
                if (collideEntities(b,&player)) {
                    killGameEntity(&player);
                    //present the xplosion sprite instead....
                    XGM_startPlayPCM(SFX_XPLOSION,9,SOUND_PCM_CH3);
                }
            }
            
        }
    }   
}

void generateEnemies() {
    /* Creating Enemy Sprites*/
    GameEntity* enem = enemies;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enem->x = i*32;
        enem->y = 32;
        enem->w = 16;
        enem->h = 16;
        enem->velx = 1;
        enem->health = 1;
        enem->sprite = SPR_addSprite(&ship, enem->x, enem->y, TILE_ATTR(PAL2, 0, TRUE, FALSE));
        sprintf(enem->name, "En%d", i);
        enemiesLeft++;
        enem++;
    }
}

void generateBullets() {
    GameEntity* bullt = bullets;
    for (i = 0; i < MAX_BULLETS; i++)
    {
        bullt->x = 0;
        bullt->y = -10;
        bullt->w = 8;
        bullt->h = 8;
        bullt->sprite = SPR_addSprite(&bullet, bullets[0].x, bullets[0].y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
        sprintf(bullt->name, "Bu%d",i);
        bullt++;
    }
}

/* END Auxiliary functions */
//---------------------------------------------------------------------------------------------------------------
// Main method by flow definition.
int main(bool hard)
{
    JOY_init();
    JOY_setEventHandler(&myJoyHandler);

    //can I place here an initial sound Pac-Man style?
    XGM_setPCM(INTRO_MUSIC,intro, sizeof(intro));
    XGM_startPlayPCM(INTRO_MUSIC,12,SOUND_PCM_CH1);
    //handle sound
    XGM_setPCM(SFX_LASER, laser, sizeof(laser));
    XGM_setPCM(SFX_XPLOSION, xplosion, sizeof(xplosion));
    //note to this: read the url for the effects.
    SYS_disableInts();
    VDP_loadTileSet(background.tileset,1,DMA);
    PAL_setPalette(PAL1, background.palette->data, DMA); //changed from VDP_setPalette
    PAL_setPalette(PAL2, background.palette->data, DMA); //changed from VDP_setPalette
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    SPR_init();

    /* adding the player to the screen, can this be moved to a function?*/
    player.x = 152;
    player.y = 192;
    player.health = 1;
    player.sprite = SPR_addSprite(&ship, player.x, player.y, TILE_ATTR(PAL1, 0, FALSE, FALSE));

    generateEnemies();
    generateBullets();
    updateScoreDisplay();
    SPR_update();
    PAL_setColor(34, RGB24_TO_VDPCOLOR(0x0078f8));
    
    /* Actual main loop for the game, this loops while the game is alive */
    while(1)
    {
        //For versions prior to SGDK 1.60 use VDP_waitVSync instead.
        VDP_setVerticalScroll(BG_B,offset -= 2);
        if (offset <= -256) offset = 0; 

        // commented out for performance issues.... pregenerate the random values and load?
        /*
        int half = 0;
        for (i=0; i < 1280; i++) {
            half = i%4;
            if (half != 0) {
                thex = i % 40;
                they = i / 40;

                val = (random() % (10-1+1)) + 1;
                if (val > 3) val = 1;
                VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1,0,0,0,val), thex,they);
            }
        } */

        positionPlayer();
        positionBullets();
        positionEnemies();
        handleCollitions();
        SPR_update();
        //should this be the last line called?
        SYS_doVBlankProcess();
    }

    SYS_enableInts();
    return (0);
}