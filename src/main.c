/**
 * Gens Invaders
 * Created With Genesis-Code extension for Visual Studio Code
 * Based on ohsat (https://www.ohsat.com/) awesome tutorial.
 * Assets by: taken from ohsat page,
 * Code by: AMH
 * Last mod by: AMH (with Bimo) - 12 / 11 / 2026
 *
 * Change history lives in CHANGELOG.md; design notes in README.MD.
 **/
#include <genesis.h>
#include <resources.h>
#include "config.h"
#include "game.h"
#include "difficulty.h"
#include "bullets.h"
#include "enemies.h"
#include "powerups.h"
#include "hud.h"
#include "player.h"
#include "explosions.h"
#include "game_state.h"

// background scroll offset
int offset = 0;

/* Starfield: fills plane B ONCE with random tiles from space.bmp (tiles 1..3,
 * loaded at VRAM index 1). The old code did this inside the main loop, i.e.
 * 1280 tile writes every frame, which is why it was too slow and got commented
 * out. Drawing it once and only moving the vertical scroll is enough: the plane
 * wraps around, so the stars scroll forever for free. */
void drawStarfield() {
    u16 x, y, val;
    for (y = 0; y < BG_TILES_H; y++) {
        for (x = 0; x < BG_TILES_W; x++) {
            val = (random() % 10) + 1;     // 1..10
            if (val > 3) val = 1;          // ~80% tile 1 (empty space), rest are stars
            VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL1, 0, 0, 0, val), x, y);
        }
    }
}

/* END Auxiliary functions */
//---------------------------------------------------------------------------------------------------------------
// Main method by flow definition.
int main(bool hard)
{
    JOY_init();
    JOY_setEventHandler(&game_state_joy);
    difficulty_init();

    //can I place here an initial sound Pac-Man style?
    XGM_setPCM(INTRO_MUSIC,intro, sizeof(intro));
    XGM_startPlayPCM(INTRO_MUSIC,12,SOUND_PCM_CH1);
    //handle sound
    XGM_setPCM(SFX_LASER, laser, sizeof(laser));
    XGM_setPCM(SFX_XPLOSION, xplosion, sizeof(xplosion));
    XGM_setPCM(SFX_SHIELD_ON, shieldon, sizeof(shieldon));
    XGM_setPCM(SFX_SHIELD_READY, shieldrdy, sizeof(shieldrdy));
    XGM_setPCM(SFX_ENEMY_EXP, enxplode, sizeof(enxplode));
    //note to this: read the url for the effects.
    SYS_disableInts();
    VDP_loadTileSet(background.tileset,1,DMA);
    PAL_setPalette(PAL1, background.palette->data, DMA); //changed from VDP_setPalette
    PAL_setPalette(PAL2, background.palette->data, DMA); //changed from VDP_setPalette
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    drawStarfield();
    SPR_init();

    player_init();

    enemies_init();
    bullets_init();
    hud_init();
    powerups_init();
    explosions_init();
    SYS_enableInts();
    /* Intro is already playing; show the title screen before the first match. */
    game_state_showTitle();
    SPR_update();

    /* Actual main loop for the game, this loops while the game is alive */
    while(1)
    {
        // The starfield keeps scrolling on every screen, even GAME OVER.
        VDP_setVerticalScroll(BG_B,offset -= 2);
        if (offset <= -256) offset = 0;

        // Old idea: random starfield tiles every frame. Removed for performance;
        // pregenerating the values once would be the way to bring it back.

        game_state_update();
        SPR_update();
        SYS_doVBlankProcess();
    }

    return (0);
}
