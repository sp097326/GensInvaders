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
BlastEm default keys: arrows = D-pad, S = B (shoot), A = A (RapidFire), D = C (shield), Enter = START.
Note: the line `Error 127 (ignored)` during linking is a known SGDK/LTO quirk on Windows and
does not affect the ROM.

## Estructura del código
Public headers live in inc/; implementation in src/. SGDK's makefile.gen already compiles every src/*.c (except oot/rom_head.c) and adds -Iinc -Isrc. Module data is static inside each .c; only shared types and accessors are exported. src/main.c is init, the starfield, and the main-loop dispatch.

* inc/config.h — shared gameplay constants (formation size, edges, bullet caps, lives, SFX ids). Tunables that change by difficulty stay in the difficulty table.
* inc/game.h — GameEntity, GameState, killGameEntity / 
eviveGameEntity, and GAME_VERSION (includes git-ignored src/version.h when present, otherwise V0.1.DEV).
* difficulty.c/.h — DifficultyConfig table, names, current difficulty, difficulty_config() / difficulty_cycle().
* ullets.c/.h — player and enemy bullet pools, shooting, movement and despawn.
* hud.c/.h — score / ships / difficulty lines, centered text, overlay clear.
* nemies.c/.h — two-row formation, 
owsOpposite movement, volleys, palettes, win/lose descent flag.
* player.c/.h — ship sprite, movement bounds, lives, respawn and invulnerability blink.
* collision.c/.h — AABB checks; player bullets vs enemies and enemy bullets vs the player.
* 	itle.c/.h — block logo, menu, gold pulse; GAME_VERSION under the logo.
* powerups.c/.h — RapidFire (A) and the Shield ability (C; duration and recharge come from the difficulty table); on-screen pickup entity reserved for later.
* game_state.c/.h — title / play / pause / GAME OVER / YOU WIN, joy handler, reset and end-screen text.
* src/main.c — toolchain init, starfield (drawStarfield + vertical scroll of offset) and the loop that calls game_state_update(). The starfield stayed in main.c because no background module was in the plan.

## Controls
* Title screen: UP/DOWN move the cursor between START GAME (INICIAR) and DIFFICULTY.
        LEFT/RIGHT (or START / A on DIFFICULTY) cycle EASY / NORMAL / HARD.
        START on START GAME begins the match with the chosen difficulty (default NORMAL).
* D-pad left/right: move the ship (during play).
* B: shoot.
* A: toggle the RapidFire powerup (up to 6 bullets on screen; expires on its own).
* C: activate the shield if it is charged. Duration and recharge depend on
        difficulty: EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
        Starts ready at the beginning of a match. Not usable while exploding, dead,
        paused, or recharging. On BlastEm the default key for C is D.
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
*       12/11/26 - Refactor of src/main.c into modules (config/game headers,
                difficulty, bullets, hud, enemies, player, collision, title,
                powerups, game_state). No gameplay changes. main.c is init
                and dispatch; the starfield stays there.
*       12/11/26 - NORMAL fireChance set to 10 so every 120-frame volley
                actually fires (2 shots, cap 2). EASY and HARD unchanged.
*       12/11/26 - Enemy volleys pick random distinct columns and avoid
                repeating the previous volley columns when others remain.
                RNG is re-seeded when START begins a match (title frame
                count + vtimer + getTick()). Difficulty table unchanged.
*       12/11/26 - Player explosion on hit: 6-frame 16x16 sheet
                (flash, fireball, debris, fade) at the ship position
                for 48 frames (~0.8 s). Input and enemy fire freeze
                during it; then respawn or GAME OVER as before. Brief
                starfield shake and PAL1 flash.
*       12/11/26 - Shield ability on button C (B is shoot, A is still
                RapidFire). 5 s blue/white bubble around the ship
                (blocks enemy bullets), then 30 s recharge. Starts
                charged. HUD shows ESC: LISTO / ON n / ns. Recharge
                state is kept on death; a new match starts ready.
*       12/11/26 - Shield duration and recharge now follow the difficulty
                table: EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
                Seconds are converted to frames with IS_PAL_SYSTEM
                (x60 NTSC, x50 PAL). The HUD still shows remaining seconds.

TODO:
*       Compose a real intro (XGM track or a proper short jingle); the current one is a trimmed placeholder
*       Start a list of where to take the base "game" to make something better, here's some ideas:
        - [Done 12/11/26] Add more enemies, control a second row.
        - Change sprites, use a different one for enemies, maybe 2.
          The per-row EnemyRowType is ready: drop a 16x16 sheet in res/tiles/ and
          point the row's spriteDef at it in resources.res / enemies.c. The bottom
          row already uses a different palette as a stand-in.
        - [Done 12/11/26] Add an explosion sprite for the player when hit by enemy fire.
        - [Done 12/11/26] Can two enemies fire at once?
        - Difficulty values are data-driven (difficultyTable in difficulty.c): step down, max
          enemy bullets, shots per volley, fire chance, shot interval, enemy speed,
          bullet speed, rowsOpposite, and shield duration / recharge (seconds)
          can be tuned there without changing the movement or fire code. On in
          NORMAL and HARD; set EASY's rowsOpposite to 1 to enable it there too.
          Shield times: EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
        - [Done 12/11/26] Title screen with a 2-option menu: START GAME and DIFFICULTY (EASY / NORMAL / HARD).
          Large blocky GENS / INVADERS logo drawn with solid tiles on plane A.
        - [Done 12/11/26] Shield power-up (button C). Duration and recharge follow
          the difficulty table (EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s).
          Ready at the start of a match. HUD shows LISTO or the remaining seconds.
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

**Modular refactor (later still on Nov 12)**
* Split src/main.c (1226 lines at V0.1.25) into modules with no behavior change. Public headers in inc/; SGDK already compiles every src/*.c and includes -Iinc.
* New modules: `config.h` / `game.h`, `difficulty`, `bullets`, `hud`, `enemies`, `player`, `collision`, `title`, `powerups`, `game_state`. `main.c` is now init + starfield + loop dispatch (~91 lines).
* GAME_VERSION is included from game.h so title and pause still show V0.1.xx. After header changes, delete out/src/*.o before rebuilding.
* Not moved: starfield (drawStarfield and offset scroll) stays in main.c (no background module was planned).

**NORMAL always fires its volley (later still on Nov 12)**
* After the refactor Alberto noticed that on NORMAL the ships barely seemed to shoot. A diff against V0.1.25 showed no regression: same ticker, pool, getters and table. NORMAL had `fireChance` 8, so about 1 in 5 volleys was skipped and two misses in a row meant 4-6 seconds with no bullets (EASY and HARD already fire every attempt).
* `fireChance` on NORMAL is now 10: every 120-frame volley (~2 s) fires two shots, still capped at 2. Interval, speed, step, cap and `rowsOpposite` are unchanged. EASY and HARD are unchanged.

**More varied enemy shooters (later still on Nov 12)**
* On NORMAL the volley rate was correct (2 ships every ~2 s) but it was almost always the same ships.
* Root causes: SGDK only auto-seeds `random()` on the first button of the session (often a menu tap right after boot, with a similar tick); `random() % n` used the low bits of the 16-bit xorshift and could clump columns; the picker did not avoid the previous volley columns.
* START that begins a match now calls `setRandomSeed` with a title-screen frame counter, `vtimer` and `getTick()`. Each volley still fires the bottom-most living ship in a column, but columns are chosen at random, stay distinct within the volley, and skip the previous volley columns when other eligible ones exist. Uses high bits of `random()` (`randRange`). Applies to all difficulties; table values are unchanged.

**Player explosion (later still on Nov 12)**
* New 6-frame 16x16 indexed sheet `res/tiles/imgexplosion.bmp` (96x16, color 0 transparent, same PAL1 as `space.bmp`: white flash, expanding orange/red fireball, debris, fade). `SPRITE explosion` in `resources.res` (2x2, FAST, anim speed 0; frames are advanced by hand so it does not loop).
* On hit the player ship is hidden and the explosion plays at that position for 48 frames (~0.8 s, 8 ticks per visual frame). Input is already blocked while health is 0; pause is also blocked. Enemy fire and the shot ticker are skipped (`player_isExploding`); enemies and bullets already on screen keep moving. A short decaying horizontal shake on the starfield (BG_B) and a 4-frame flash of PAL1 color 2 to white, then restored.
* If lives remain, the remaining `RESPAWN_DELAY` (42 frames) runs after the explosion so total dead time stays ~90 frames, then the usual blinking invulnerability. If no lives are left, GAME OVER waits until the explosion finishes (`player_update` returns 1). Logic lives in the player module. Hardware-sprite budget stays at 70: the 16x16 explosion (4 cells) replaces the hidden ship.

**Shield power-up (later still on Nov 12)**
* Button C activates a shield (B is already shoot, A is still the RapidFire toggle). It starts charged at the beginning of a match, lasts 5 s, then recharges for 30 s and cannot be retriggered until then. NTSC uses 300 / 1800 frames; on PAL, `IS_PAL_SYSTEM` selects 250 / 1500 so the wall-clock time stays 5 s / 30 s. The project had no PAL timing before this.
* While active, a 32x32 4-frame pulsing blue/white bubble (`res/tiles/imgshield.bmp`, same PAL1, color 0 transparent) follows the ship. Enemy bullets that touch the 32x32 box are destroyed and do no damage. This game has no ship-vs-enemy body collision, so the shield only has to stop bullets. Short PCM blips play on channel 1 when the shield goes up and when the recharge finishes.
* Cannot be used during the player explosion, while dead, while paused, or during recharge. The timer only ticks in STATE_PLAYING, so pause freezes it. On life loss the recharge (or LISTO) state is kept; a bullet cannot kill the player while the shield is up, so in practice the recharge just keeps running through the explosion and respawn. `powerups_reset` on a new match sets it back to ready. HUD field on the right of line 2: `ESC: LISTO`, `ESC: ON n`, or `ESC: ns`, redrawn with the score line so the layout stays intact.
* One extra hardware sprite (32x32 = 1 VDP sprite, 16 tiles). Real sprite count is about 27 of 80. The earlier "70" budget counted each 16x16 ship as 4 sprites; this ability adds one real VDP sprite, not 16.

**Shield times by difficulty (later still on Nov 12)**
* After playtesting, 5 s of shield felt too long. Duration and recharge moved into the difficulty table (`shieldDurationSec` / `shieldRechargeSec`): EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
* `powerups` still converts those seconds to frames with `IS_PAL_SYSTEM` (x60 NTSC, x50 PAL) at reset, so the wall-clock time is the same on both systems. The HUD continues to show remaining seconds (`ESC: ON n`, `ESC: ns`). The fixed `SHIELD_DURATION_*` / `SHIELD_RECHARGE_*` macros were removed from `config.h`.
