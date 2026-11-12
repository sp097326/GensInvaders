# Gens Invaders
Second Game for the Sega Genesis 16-Bit machine. This time with the intention of learning
better sprite work before moving to something more complex. Again as GensPong this is code
derived directly from the tutorial on Ohsat page:
https://www.ohsat.com/tutorial/megalaga/megalaga-1/

He provides everything required for the code to work, including a really easy, step by step
and intuitive solution.

Again I'll be modifing and deriving code from the tutorial but the actual base is credit to 
OHSAT. This time notes and information will reside on this README.MD file instead of the
initial portion of the main.c file.

So, Gens Invaders is a simple clone of the space invaders formula with a 16-Bit Genesis
graphics flavour.

## How to build
Requirements (Windows):
* SGDK 2.11 installed at `C:\tools\SGDK` with the `GDK` environment variable pointing to it.
* Java 21 (Eclipse Temurin) on the PATH; SGDK's `rescomp.jar` and `sizebnd.jar` need it.

From the project folder:

    %GDK%\bin\make -f %GDK%\makefile.gen

The ROM ends up in `out/rom.bin` (256 KB).

To play it, run `run.bat` from the project root (or `run.bat build` to compile first).
`run.bat build` also writes `src/version.h` with the version `V0.1.xx`, where xx is the
number of git commits; build after committing so the version matches GitHub.
It opens the ROM in BlastEm 0.6.2, installed at `C:\Tools\BlastEm\blastem-win32-0.6.2\blastem.exe`.
BlastEm default keys: arrows = D-pad, S = B (shoot), A = A (powerup), Enter = START.
Note: the line `Error 127 (ignored)` during linking is a known SGDK/LTO quirk on Windows and
does not affect the ROM.

## Controls
* Title screen: UP/DOWN move the cursor between START GAME (INICIAR) and DIFFICULTY.
        LEFT/RIGHT (or START / A on DIFFICULTY) cycle EASY / NORMAL / HARD.
        START on START GAME begins the match with the chosen difficulty (default NORMAL).
* D-pad left/right: move the ship (during play).
* B: shoot.
* A: toggle the RapidFire powerup (up to 6 bullets on screen; expires on its own).
* START: during play, pause / resume (the pause screen shows the game version and the
        current difficulty). After GAME OVER or YOU WIN, returns to the title screen
        with the last difficulty preselected.
* You get 3 ships; after a hit the ship respawns and blinks for ~2 s (can't be hit).

## Change log
Detailed changes live in `CHANGELOG.md`. The June 2026 repair session is documented in
`gensinvaders-session-log.html`.

NOTES SECTION
-- OK, I really abandoned this for quite a while, I'll try to get something done before the
    en of the month, can't promise to much, guess I bit a little more than I can chew...
-- AFTER QUITE A WHILE I finally found the time to do somehting. Little surprise though...
    since version 1.80 of the SGDK several things are not working or playinly breaking
    compilation. I'll try to see If I can sort through it but my lack of C knowledge coupled
    with my lack of Genesis (68000 CPU, VDP and Sound) knowledge make turn this into an 
    excersice on futility. Fixed at least one problem though....
-- Moving along with more code, for some reason it's really slow. I don't know if it's my code
    or something to do with the computer setup.
-- Once again my time is not proportional to the speed of the SDGK development and alas here
    we are in version 2.0 for the SGDK. Code seems to work as it is, still haven't unraveled
    the sound problem and the sprite animation issues.
-- Nov 2026: back at it with help from Bimo. Updated to SGDK 2.11, the game builds cleanly
    again and now has an actual ending (GAME OVER / YOU WIN) with restart.

CODER: 
*   AMH - MANZANAS
INITIAL_DATE: 
*   07/16/2021
LAST_MODIFY_DATE:
*   11/12/2026

HISTORY:
*       16/07/21 - Created template and helloworld type code.
*   	21/06/23 - Added space bkgnd and initial user sprite.
*       22/06/23 - Added enemies, enemies sprites and enemies initial animation, implemented basic 
                joy handler and player positioning, implemented bullets and it's animation.
*       23/06/23 - Added collision detection routines and collition result as such.
*       28/06/23 - Added enemy fire, collision for both enemy and player
*       30/06/23 - Added sounds -- btw not working *, also changed emulator
*       06/07/23 - Added code to trigger powerup code, still on the works, checking working funcionallity and 
                VDP to PAL code translation.
*       21/08/23 - Added code for an intro song, still trying to debug why sfx won't play
*       22/08/23 - Changed rom header information to show gamename
*       26/06/26 - Repair session: fixed powerup toggle/RapidFire/palette crash, powerup auto-expire,
                updated sega.s boot stub, WAV driver changed to XGM. Builds again.
*       10/11/26 - SGDK 2.11 + Java 21 toolchain. Added game states (GAME OVER / YOU WIN) and restart
                with START, fixed enemy bullet not freed on player hit, interrupts never re-enabled,
                player name overflow; removed global i, renamed handleCollisions. Added CHANGELOG.md.
                Later: intro trimmed to 5 s, explosion on its own PCM channel, ROM header renamed to
                GENS INVADERS, run.bat to launch BlastEm. Then: 3 lives with respawn and blinking
                invulnerability, starfield background drawn once, enemies move as a formation.
                Pause screen on START showing the game version (V0.1.xx = git commit count).
*       12/11/26 - Formation steps down 8 px on each edge bounce; GAME OVER if it reaches
                the player's row. START still resets the formation to Y = 32.
*       12/11/26 - Difficulty menu (EASY / NORMAL / HARD) and a dedicated enemy bullet pool
                so two distinct enemies can fire at once. Tuning is data-driven
                (difficultyTable). After GAME OVER / YOU WIN, START returns to the menu.
*       12/11/26 - Retuned difficulties: the table now also has shotInterval, enemySpeed
                and bulletSpeed. EASY fires regularly every 90 frames; NORMAL is unchanged;
                HARD moves and fires twice as fast (step 16, interval 60, speed 2).
*       12/11/26 - Title screen with a large blocky GENS / INVADERS logo (solid tiles on
                plane A, with shadow and a gold color pulse) and a 2-option menu:
                START GAME (INICIAR) and DIFFICULTY. The version is shown under the logo.
*       12/11/26 - Second row of 7 enemies (14 total), one formation; bottom row uses a
                tinted palette and is worth 10 points, top row 20. Firing prefers the
                lowest living ship in each column.
*       12/11/26 - On HARD, rows move in opposite directions and bounce independently
                (rowsOpposite flag; set it to 1 on NORMAL to enable it there too). A row
                will not step down if it would come within 24 px of the row below.
*       12/11/26 - Opposite-row movement enabled on NORMAL as well (rowsOpposite=1);
                EASY still moves as a single formation.

TODO:
*       Compose a real intro (XGM track or a proper short jingle); the current one is a trimmed placeholder
*       Start a list of where to take the base "game" to make something better, here's some ideas:
        - [Done 12/11/26] Add more enemies, control a second row.
        - Change sprites, use a different one for enemies, maybe 2.
          The per-row EnemyRowType is ready: drop a 16x16 sheet in res/tiles/ and
          point enemyRows[n].spriteDef at it in resources.res / main.c. The bottom
          row already uses a different palette as a stand-in.
        - Add an explosion sprite for the player when hit by enemy fire.
        - [Done 12/11/26] Can two enemies fire at once?
        - Difficulty values are data-driven (difficultyTable in main.c): step down, max
          enemy bullets, shots per volley, fire chance, shot interval, enemy speed,
          bullet speed and rowsOpposite can be tuned there without changing the
          movement or fire code. On in NORMAL and HARD; set EASY's rowsOpposite to 1 to enable it there too.
        - [Done 12/11/26] Title screen with a 2-option menu: START GAME and DIFFICULTY (EASY / NORMAL / HARD).
          Large blocky GENS / INVADERS logo drawn with solid tiles on plane A.
        - More powerups: ship speed up? a bomb?
        - [Done 12/11/26] Formation steps down on each turn (classic invaders) and GAME OVER if it reaches the player.

## Work log - 10/11/2026 (with Bimo)
Everything below was done on Nov 10, 2026 (up to V0.1.16). Details per change are in `CHANGELOG.md`.

**Toolchain and build**
* Moved to SGDK 2.11 and installed Java 21 (Eclipse Temurin), needed by `rescomp`; the project compiles with no warnings.
* `run.bat` launches the ROM in BlastEm 0.6.2; `run.bat build` writes `src/version.h` (V0.1.xx, xx = git commit count), compiles and launches.
* Fixed `run.bat build` reusing a stale `main.o`, which made the pause screen show `V0.1.DEV`.
* `.gitignore` now ignores `out/`, `src/boot/sega.s`, `src/boot/*.bak` and `src/version.h`; `src/boot/rom_head.c` is tracked.

**Gameplay**
* Game states: playing, paused, GAME OVER and YOU WIN; START restarts after the game ends.
* 3 ships: after a hit the ship respawns and blinks for about 2 seconds, without taking damage.
* Enemies move as a formation: when one reaches an edge, all of them turn around together (they no longer overlap).
* Pause screen on START showing "PAUSED", "GENS INVADERS V0.1.xx" and "PRESS START".
* HUD shows score, enemies left and ships left.

**Graphics and sound**
* Starfield background (the commented-out code) now works; it is drawn once instead of every frame, which fixed the slowdown.
* Intro trimmed to a 5 second placeholder with fade (`music/intro_short.wav`); the original `Intro.wav` is kept.
* Player explosion plays on its own PCM channel so the laser no longer cuts it off.
* ROM header shows "GENS INVADERS" instead of the SGDK "SAMPLE PROGRAM".

**Bug fixes and cleanup**
* Enemy bullet not freed when it hit the player; interrupts never re-enabled; player name buffer overflow.
* Removed unused globals (`i`, `thex`, `they`, `val`) and renamed the collision routine to `handleCollisions`.

## Work log - 12/11/2026 (with Bimo)
Everything below was done on Nov 12, 2026. Details per change are in `CHANGELOG.md`.

**Gameplay**
* Formation descent (README TODO #7): when any living enemy reaches an edge, the whole group turns around and steps down 8 pixels (`ENEMY_STEP_DOWN`).
* GAME OVER if any living enemy's sprite reaches the player's row (the bottom of the sprite meets `PLAYER_START_Y`). Uses the existing `showEndScreen(STATE_GAME_OVER)` path.
* Restart with START still resets the formation to Y = 32 via `placeEnemies()` and restores `enemyDir`.
* Enemy bullets and collisions are unchanged; they keep using the updated positions.

**Difficulty and enemy fire (later on Nov 12)**
* Data-driven difficulty table (`DifficultyConfig difficultyTable`): EASY (no descent, 1 enemy bullet), NORMAL (8 px step, up to 2 bullets, two enemies fire together), HARD (16 px step, two fire together, up to 4 bullets on screen). `fireChance` is in the table for later tuning.
* Dedicated enemy bullet pool (`enemyBullets`, 4 sprites). Each bullet collides with the player on its own and is freed on hit, off-screen, or reset. Player bullets stay in their own pool. Lives, respawn and invulnerability are unchanged.
* Pre-game menu "SELECT DIFFICULTY": UP/DOWN to choose, START to confirm (default NORMAL). After GAME OVER or YOU WIN, START returns to that menu with the last choice preselected. Current difficulty is shown on the HUD (`DIFF:`) and on the pause screen.
* The standalone SELECT DIFFICULTY menu was later replaced by the title screen (see below).

**Difficulty retune (later still on Nov 12)**
* Root cause of HARD feeling like NORMAL: `currentDiff` was applied correctly, but HARD shared the 120-frame fire interval and 1 px horizontal speed, and the 4-bullet cap almost never filled because bullets left the screen in ~38 frames. No frame-drop evidence.
* The table now also has `shotInterval`, `enemySpeed` and `bulletSpeed`.
* EASY: fires every 90 frames with `fireChance` 10 (1 bullet). NORMAL: unchanged (120, 2 shots, cap 2, step 8, speed 1, bullet 5). HARD: interval 60, cap 4, step 16, horizontal speed 2, bullet speed 6.

**Title screen (later still on Nov 12)**
* Replaced the standalone SELECT DIFFICULTY menu with a title screen (`STATE_TITLE`).
* Large two-row logo, GENS (5x5 blocks) over INVADERS (4x5 blocks), drawn as solid 8x8 tiles on plane A with a drop shadow, so it stays within the 20-sprites-per-scanline limit. The fill color pulses gold/orange. `V0.1.xx` is shown under the logo. The starfield keeps scrolling on plane B; game sprites are hidden.
* Menu: START GAME (INICIAR) and DIFFICULTY: EASY/NORMAL/HARD. UP/DOWN move the cursor; LEFT/RIGHT (and START or A on DIFFICULTY) cycle the level; START on START GAME begins the match. After GAME OVER / YOU WIN, START returns here with the last difficulty preselected.

**Second enemy row (later still on Nov 12)**
* Two rows of 7 (14 ships) move, bounce and step down as one formation. GAME OVER uses the lowest living enemy; YOU WIN when both rows are cleared. Volleys pick the bottom-most living ship in each column.
* Per-row `EnemyRowType` (sprite, palette, points). For now both rows reuse `ship`; the bottom row is tinted green/yellow on PAL3 and is worth 10 points, the top row 20. A different 16x16 graphic later is just a new `SPRITE` in `resources.res` and a pointer change — same size has no extra performance cost.
* Sprite budget: 14 enemies + 1 player + 6 player bullets + 4 enemy bullets = 25 software sprites. In hardware sprites (16x16 = 4 cells, 8x8 = 1): 14*4 + 4 + 6 + 4 = 70 of 80. Per scanline, one full row uses 14 of 20 (rows are 24 px apart so they do not share a line).

**Opposite rows on HARD (later still on Nov 12)**
* New `rowsOpposite` flag in the difficulty table: 0 on EASY and NORMAL (both rows still move as one formation), 1 on HARD. Set NORMAL's flag to 1 to enable the same behavior there.
* On HARD, the top row starts right and the bottom row starts left (shifted to the right so it is not already on the left edge). Each row bounces on its own living ships and only that row steps down. A row will not drop if it would come within 24 px of the next living row below; it just reverses. If the bottom row is gone, the top row can descend freely. GAME OVER still uses the lowest living enemy.

**Opposite rows on NORMAL too (later still on Nov 12)**
* NORMAL now also has `rowsOpposite=1`. Its other values are unchanged (step 8, speed 1, interval 120, cap 2). EASY stays 0 (both rows move together). HARD is unchanged.
