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
`run.bat build` also writes `src/version.h` with the version `V0.3.xx`, where xx is the
total number of git commits (`git rev-list --count HEAD`); the 0.2 prefix marks this
stage of the game, and the last number is not reset. Build after committing so the
version matches GitHub. If `version.h` is missing, the game shows `V0.3.DEV`.
It opens the ROM in BlastEm 0.6.2, installed at `C:\Tools\BlastEm\blastem-win32-0.6.2\blastem.exe`.
BlastEm default keys: arrows = D-pad, S = B (shoot), A = A (RapidFire), D = C (shield), Enter = START.
Note: the line `Error 127 (ignored)` during linking is a known SGDK/LTO quirk on Windows and
does not affect the ROM.

## Estructura del código
Public headers live in inc/; implementation in src/. SGDK's makefile.gen already compiles every src/*.c (except oot/rom_head.c) and adds -Iinc -Isrc. Module data is static inside each .c; only shared types and accessors are exported. src/main.c is init, the starfield, and the main-loop dispatch.

* inc/config.h — shared gameplay constants (formation size, edges, bullet caps, lives, SFX ids). Tunables that change by difficulty stay in the difficulty table.
* inc/game.h — GameEntity, GameState, killGameEntity / 
eviveGameEntity, and GAME_VERSION (includes git-ignored src/version.h when present, otherwise V0.3.DEV).
* difficulty.c/.h — DifficultyConfig table, names, current difficulty, difficulty_config() / difficulty_cycle().
* ullets.c/.h — player and enemy bullet pools, shooting, movement and despawn.
* hud.c/.h — score / ships / difficulty lines, shield bar (ESC + 8 tiles), centered text, overlay clear.
* nemies.c/.h — two-row formation, 
owsOpposite movement, volleys, palettes, win/lose descent flag.
* player.c/.h — ship sprite, movement bounds, lives, respawn and invulnerability blink.
* collision.c/.h — AABB checks; player bullets vs enemies, enemy bullets vs the player, and fatal enemy-player body contact (or a ship reaching the visible bottom).
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
        The HUD shows ESC and a bar on the right of line 2: blue draining while
        active, dark red filling while recharging, green (brief blue blink) when ready.
* START: during play, pause / resume (the pause screen shows the game version and the
        current difficulty). After GAME OVER or YOU WIN, returns to the title screen
        with the last difficulty preselected.
* You get 3 ships; after a bullet hit the ship respawns and blinks for ~2 s (can't be hit by bullets).
* Rules: a bullet hit costs one life. An enemy ship touching the player is always
        GAME OVER after the explosion, even with lives or a shield left (the shield
        still blocks bullets). If invaders slip past without touching and reach the
        bottom of the screen, that is also GAME OVER, so the match cannot stall.

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
*       12/11/26 - Version prefix raised to 0.2. The last number is still
                the total git commit count (this release is V0.2.42).
                Fallback label is V0.2.DEV. Tagged v0.2.42.
*       12/11/26 - Shield HUD is now a short ESC label plus an 8-tile bar
                (64 steps) on the right of line 2: blue emptying while
                active, dark red filling while recharging, green when
                ready (brief blue blink when the recharge finishes).
*       12/11/26 - GAME OVER on invasion is now a body collision with the
                player (explosion, then GAME OVER, even with lives left).
                Reaching the player's row without touching is no longer
                enough. If a ship slips past and hits the visible bottom,
                the same fatal explosion applies.
*       12/11/26 - Enemy ships play a distinct 6-frame 16x16 burst
                 (compact white/yellow flash, green sparks flying out)
                 for 24 frames (~0.4 s) when destroyed. Drawn with PAL3
                 so it stays different from the player's orange/red
                 fireball. A pool of 6 simultaneous bursts; the ship
                 vanishes at once and the explosion takes its place.
                 On a crash into the player, both explode before GAME OVER.
*       13/11/26 - Endless waves: clearing both rows no longer ends the
                 match. A brief OLEADA N banner (~1.5 s) plays and two
                 new rows spawn at the top. Score, lives and the shield
                 state are kept. Each wave scales movement and fire rate
                 from the difficulty's base values, with caps. Version
                 prefix raised to 0.3; the last number is still the
                 total commit count (this release is V0.3.46). Fallback
                 is V0.3.DEV. Tagged v0.3.46.
*       13/11/26 - TOP 5 high-score table saved in cartridge SRAM.
                 After GAME OVER, a qualifying score (beats 5th place)
                 asks for 3 arcade initials, then the table is shown
                 under the GAME OVER text (rank, initials, score, wave,
                 difficulty letter F/N/D). One shared table. Also
                 reachable from the title menu (PUNTAJES). Tagged v0.3.47.
*       13/11/26 - Extra life every 1000 points (1000, 2000, 3000...),
                 cap of 9 ships so the HUD stays one digit. A blinking
                 "+1 NAVE" appears for ~1 s and a short two-note 1UP
                 jingle plays on PCM channel 4. Same threshold on all
                 difficulties; resets each match. Tagged v0.3.48.
*       13/11/26 - Points per destroyed ship now follow the difficulty
                 table: EASY 10, NORMAL 20, HARD 30 (both rows). A full
                 wave is worth 140 / 280 / 420. Extra lives stay every
                 1000 points. Tagged v0.3.49.
*       13/11/26 - Lives are now up to 9 miniature ships on the right of
                 the HUD (plane tiles, not sprites). The "+1 NAVE" banner
                 stays solid and readable for ~1 s; the new icon blinks.
                 Tagged v0.3.50.
*       13/11/26 - Life icons moved to the bottom-left corner (row 27,
                 just below the shield). "+1 NAVE" sits next to them;
                 the top-right of the HUD is clear again. Tagged v0.3.51.
*       13/11/26 - Short between-wave warp: faster stars, engine flame,
                 ship lifts a little, then the next two rows slide in
                 from the top under the OLEADA N banner (~2.5 s).
                 Tagged v0.3.52.
*       13/11/26 - Shield hum lasts for the whole active duration
                 (2 s sample on CH1, retriggered on EASY/NORMAL). The
                 old 0.16 s blip was just too short. Tagged v0.3.53.
*       13/11/26 - Charged shot: hold B ~1 s, release a 24x24 piercing
                 bolt. A short tap still fires the normal 8x8 shot.
                 Tagged v0.3.54.
*       13/11/26 - Charged-shot release sound is louder and lower
                 (a bass boom, distinct from the normal laser).
                 Tagged v0.3.55.

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
          Ready at the start of a match. HUD shows ESC plus a bar (blue / dark red / green).
        - More powerups: ship speed up? a bomb?
        - [Done 12/11/26] Formation steps down on each turn (classic invaders). GAME OVER if an enemy ship collides with the player, or reaches the visible bottom after slipping past.
        - [Done 12/11/26] Distinct explosion animation when an enemy ship
          is destroyed (and when one crashes into the player). Smaller and
          faster than the player's (~0.4 s, green/yellow sparks, pool of 6).
        - [Done 13/11/26] Endless waves: clearing a pair of rows spawns
          the next wave a bit faster (movement and fire rate), with caps
          per difficulty. The match no longer ends with YOU WIN.
        - [Done 13/11/26] TOP 5 high-score table in SRAM, initials entry
          after GAME OVER, and a PUNTAJES option on the title screen.
        - [Done 13/11/26] Extra life every 1000 points, cap 9, with a
          short 1UP jingle and a "+1 NAVE" banner.
        - [Done 13/11/26] Points per enemy depend on difficulty
          (EASY 10, NORMAL 20, HARD 30); both rows use the same value.
        - [Done 13/11/26] HUD lives are miniature ships (up to 9) and
          the "+1 NAVE" banner is a solid, readable plane-A message.
        - [Done 13/11/26] Life icons sit in the bottom-left corner,
          with the "+1 NAVE" banner beside them.
        - [Done 13/11/26] Between-wave transition: starfield warp,
          engine flame, and the next formation sliding in from the top.
        - [Done 13/11/26] Shield sound lasts for the whole active
          duration (2 s hum on CH1, not a 0.16 s blip).
        - [Done 13/11/26] Charged shot (hold B ~1 s) fires a piercing
          24x24 bolt; taps still shoot the normal 8x8 bullet.
        - [Done 13/11/26] Charged-shot release SFX is a louder, deeper boom.

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

**Version 0.2 (later still on Nov 12)**
* The version prefix is now 0.2. The last number is still the total commit count (`git rev-list --count HEAD`), not a count from a 0.2 base tag, so this release is V0.2.42 (the next commit after V0.1.41). The fallback without `src/version.h` is `V0.2.DEV`. Annotated tag `v0.2.42`.
* `run.bat build` writes `V0.2.xx` to `src/version.h` with Python so `echo` no longer leaves a trailing space, and it deletes every `out/src/*.o` before compiling because several modules include the version through `game.h`.

**Shield HUD bar (later still on Nov 12)**
* Replaced the numeric shield HUD (`ESC: LISTO` / `ESC: ON n` / `ESC: ns`) with a short `ESC` label and an 8-tile bar (64 levels, eighth-block tiles) on the right of line 2, leaving SCORE and DIFF untouched.
* While the shield is up the bar is blue (PAL1) and drains with the time left. During recharge it fills in dark red (PAL1). When it is ready it is full and green (PAL3 color 1, the same green as the bottom enemy row) and flashes blue for about half a second when the recharge finishes, along with the existing ready SFX.
* Plane A is only rewritten when the level, state or blink phase changes. The tiles are generated in code and loaded at `TILE_USER_INDEX + 2` (title still uses the first two user tiles). No new palette or BMP; fill colors already exist in PAL1 / PAL3. The empty part of the bar is a black track (palette index 4).

**Invasion GAME OVER on body contact (later still on Nov 12)**
* The match no longer ends just because a living enemy reaches the player's row (`PLAYER_START_Y`) without touching the ship. `collision` now tests enemy-player body overlap: that is always GAME OVER after the player's explosion (`player_hitFatal`), even if lives remain. Classic invasion: you do not just lose one ship.
* The shield and post-respawn invulnerability do not protect against that contact (they still stop enemy bullets).
* If a ship steps past the player's height without overlapping in X, the formation keeps marching. When any living enemy reaches the visible bottom (`y + h >= BOTTOM_EDGE`, 224), the same fatal explosion and GAME OVER apply, so the game cannot stall with invaders sitting below the gun. They do not bounce or freeze.

**Enemy explosions (later still on Nov 12)**
* New 6-frame 16x16 indexed sheet `res/tiles/imgenemyexp.bmp` (96x16, color 0 transparent, same palette as `space.bmp`). The burst is smaller than the player's fireball: a tight white/yellow flash, then green sparks that fly outward and fade. `SPRITE enemyexp` in `resources.res` (2x2, FAST, anim speed 0; frames are advanced by hand). Drawn with PAL3 during play (green/yellow/white) so it stays clearly different from the player's PAL1 orange/red/white explosion.
* Duration is 24 frames (~0.4 s at 60 Hz, 4 ticks per visual frame). The enemy ship is hidden immediately (`killGameEntity`) and the burst plays at that `x,y`. Explosions are independent sprites: they do not collide and they do not affect formation movement.
* Pool of 6 (`MAX_ENEMY_EXPLOSIONS` in `inc/explosions.h` / `src/explosions.c`). If all slots are busy, the oldest (lowest remaining timer) is reused. Each burst is 1 VDP sprite (16x16). Peak hardware count is about 33 of 80 (14 enemies + player + player explosion + shield + 6 player bullets + 4 enemy bullets + 6 enemy bursts). Dead ships are already hidden, so a scanline holds bursts rather than a full live row and stays under the 20-sprite limit.
* Hooked from `collision`: a player bullet that kills a ship spawns a burst, then the existing score / YOU WIN path. A body crash into the player (or a landing at `BOTTOM_EDGE`) spawns a burst on every overlapping (or landed) ship, kills them without awarding score or YOU WIN, and then `player_hitFatal` so both explosions play before GAME OVER. The enemy burst can finish under the YOU WIN / GAME OVER text; pause freezes it like the other timers.
* Optional short high pop SFX (`res/sounds/enemyexp.wav`, PCM id 69, ~0.10 s) on channel 3 at moderate priority, so it can play at the same time as the player's explosion on channel 2 during an invasion crash.

**Endless waves and version 0.3 (13/11/2026)**
* Clearing both rows no longer shows YOU WIN. The game stays in STATE_PLAYING, shows a centered `OLEADA N` banner for about 1.5 s (90 NTSC frames / 75 PAL), then respawns two rows at `ENEMY_START_Y`. Score, lives, RapidFire and the shield (including mid-recharge) are kept. Enemy bullets and the volley ticker are cleared; player bullets and any enemy explosions already playing continue. The player can still move, shoot and use the shield during the banner; pause freezes the timer and redraws the banner on resume.
* Wave 1 is an exact copy of the existing difficulty table. Later waves scale a runtime `DifficultyConfig` (`difficulty_beginMatch` / `difficulty_nextWave` in `difficulty.c`). Per-difficulty increments and caps:
  * EASY: interval 90 down by 10 to a floor of 45; speed 1 to 3 every 3 waves; volley/cap stay 1 until wave 4, then 2 (never more); bullet speed 5 to 7 every 3 waves; no descent (stepDown stays 0).
  * NORMAL: interval 120 to 40; speed 1 to 3 every 3 waves; volley and bullet cap grow from 2 to 4 every 3 waves; bullet speed 5 to 7; stepDown 8 to 16 (+4 every 4 waves). Rows still move in opposite directions; column picking is unchanged.
  * HARD: interval 60 to 30; speed 2 to 3 every 3 waves; volley 2 to 4 every 2 waves (the bullet cap is already 4); bullet speed 6 to 7; stepDown 16 to 20 (+4 every 5 waves).
  The enemy bullet pool stays at 4, so the hardware-sprite budget does not grow with the wave number.
* The HUD shows `OLEADA n` on line 2 at column 16, after `DIFF:` and before the `ESC` bar (fits even with 3-digit waves). The pause screen also shows the wave.
* Version prefix is now 0.3. The last number is still the total commit count (`git rev-list --count HEAD`), not a count from a 0.3 base tag, so this release is V0.3.46. Fallback without `src/version.h` is `V0.3.DEV`. `run.bat build` writes `V0.3.xx`. Annotated tag `v0.3.46`. GAME OVER rules are unchanged; the shield still only stops bullets.

**TOP 5 high scores (13/11/2026)**
* One shared table of 5 entries is stored in cartridge SRAM (magic `GIHS`, format 1, 16-bit checksum). Each row keeps 3 initials, the score, the wave reached and a difficulty letter (F/N/D). A single table with a difficulty column was preferred over three separate lists, so scores from Easy, Normal and Hard can be compared at a glance. If the SRAM block is missing or fails the checksum, defaults are written: GEN 1000, INV 800, UDL 600, AMH 400, BIM 200 (wave 1).
* After GAME OVER, a score that beats the current 5th place (ties stay out) opens a 3-letter initials entry: up/down cycle A-Z, space and `.`; left/right move the cursor; A or START confirm. The table is then drawn under `GAME OVER` (rows 12-16) with a blinking `>` on the new row, and `PRESS START` still returns to the title. A score that does not qualify skips the entry prompt and just shows the table. The player explosion still finishes before this screen.
* The title menu has a third option, `PUNTAJES (TOP 5)`. START or A opens the table under the logo; START or B returns to the menu.
* The ROM header already declared SRAM (`RA`, 0xF820, 0x200000-0x20FFFF) in `rom_head.c`; a comment was added. In BlastEm the file is `%LOCALAPPDATA%\blastem\rom\save.sram` because the ROM is launched as `out\rom.bin`. Deleting that file and relaunching restores the defaults.

**Extra life every 1000 points (13/11/2026)**
* A ship is awarded at 1000, 2000, 3000... (`EXTRA_LIFE_SCORE` in `config.h`, same on all difficulties). If one kill crosses more than one threshold, every crossed mark is counted. Lives are capped at 9 (`LIVES_CAP`) so `SHIPS: n` on the HUD stays one digit; past the cap no ship is added but the next threshold still advances (no queued 1UPs). Starting lives stay at 3 (`MAX_LIVES`). The counter resets in `player_reset` at the start of a match.
* On a grant, `SHIPS` is updated immediately and a blinking `+1 NAVE` (or `+n NAVE`) appears for about 1 s (60 NTSC / 50 PAL frames) on row 2, under the HUD and clear of the wave banner. A short two-note ding (`res/sounds/oneup.wav`, PCM id 70, ~0.22 s) plays on channel 4 so it does not cut the laser (CH3) or the player explosion (CH2).
* No award on the GAME OVER / initials screens (score no longer increases there). A 1UP earned during the death explosion of the last ship from a bullet cancels the pending GAME OVER and the player respawns. An invasion or bottom-edge fatal hit still ends the match even if a threshold is crossed while the explosion plays; the threshold advances but no ship is added.

**Points per ship by difficulty (13/11/2026)**
* The only score source remains a player bullet destroying an enemy (no wave bonus; an invasion crash still awards nothing). Previously both difficulties used a fixed per-row split: top row 20, bottom row 10, so a full wave of 14 was always 210 points.
* Points now come from `enemyPoints` in the difficulty table, the same for every ship: EASY 10, NORMAL 20, HARD 30. A cleared wave is worth 140 / 280 / 420. Extra lives stay at every 1000 points on all difficulties, so the first 1UP arrives around wave 8 on EASY (7 waves = 980), wave 4 on NORMAL (3 waves = 840) and wave 3 on HARD (2 waves = 840).
* The shared TOP 5 table is unchanged, including the placeholder entries. Hard scores will climb faster than Easy ones; the difficulty letter (F/N/D) still shows which run it was.

**HUD ship icons and readable 1UP banner (13/11/2026)**
* The "+1 NAVE" glitch was not the 20-sprite-per-scanline limit. The banner was already plane-A text on row 2 (above the enemies, which start at Y=32). Two things made it hard to read: `hud_updateExtraLife` cleared and redrew the whole row every frame during active display, which produced flickering horizontal lines on the VDP, and the text used low priority, so player bullets climbing off the top punched holes in it. The 8-on/8-off blink also left the message invisible half of the ~1 s window.
* The banner is now drawn once, stays solid for the same ~1 s (60 NTSC / 50 PAL frames), and uses high-priority plane-A tiles (`VDP_setTextPriority`) so sprites pass behind it. Tiles are only touched when the banner appears or disappears.
* `SHIPS: n` is gone. Up to 9 miniature 8x8 ships (a downscaled silhouette of the player ship, PAL1, plane A, not sprites) sit in the top-right corner, columns 31-39, left-aligned so a new life appears to the right of the ones already there. All 9 fit, so there is no "x9" overflow form. They update when a life is lost or gained. During the 1UP banner the newly awarded icon blinks; the others stay solid. `SCORE` and `LEFT` remain on the left of row 0, with a gap before the icons.

**Life icons moved to the bottom-left (13/11/2026)**
* The nine 8x8 ship icons left the top-right of row 0 (the top HUD was already busy with SCORE, LEFT, DIFF, OLEADA and the ESC bar) and now sit at the bottom-left of the visible plane: row 27, pixels 216-223, columns 0-8, still left-aligned so a new life appears to the right. They remain high-priority plane-A tiles, not sprites.
* That row is just below the 32px shield (which ends at Y=216) and does not overlap the player ship (Y=192-207, only moves horizontally), so `PLAYER_START_Y` was left at 192. Enemy bullets and a late invasion in that corner pass behind the icons. Explosions stay at the ship's Y and do not cover the row.
* The solid `+1 NAVE` banner moved to the same row, starting at column 10, just after the nine icon slots, so the top of the screen is no longer used for it. Clearing the banner does not wipe the icons. The newly awarded icon still blinks while the banner is up. `SCORE` / `LEFT` once again use the full width of row 0.

**Between-wave warp animation (13/11/2026)**
* Clearing a wave no longer jumps straight to the next formation after a 1.5 s banner. There is now a ~2.5 s transition (150 NTSC / 125 PAL frames) with `OLEADA N` on screen the whole time. Enemy bullets are cleared; score, lives, the shield (including its timers) and RapidFire are kept. Player bullets already in the air can finish; new shots, RapidFire toggles and a fresh shield are blocked. Horizontal movement and pause still work. The 1UP banner and life icons are unchanged (the 8x8 flame sits at Y=208-215, just above the icons on row 27).
* The ship rises 8 pixels and an 8x8 two-frame exhaust flame (`res/tiles/imgflame.bmp`, PAL1, one hardware sprite) flickers under the hull. The starfield step ramps from 2 to 14 and back down. For the first ~1.25 s the next two rows wait fully above the screen; in the last 1.25 s they slide into their home slots (80 px) as the stars slow and the flame goes out. Wave 1 still starts in place, with no warp.
* Sprite budget: one extra 8x8 sprite (35 allocated, still well under 80). The flame does not share a scanline with the 16x16 ship and only overlaps the bottom of the shield if it is already on, so the 20-sprites-per-line limit is not a concern.

**Shield hum while active (13/11/2026)**
* The activation sound seemed to cut off, but nothing was stealing its channel. `shield_on.wav` is only 0.16 s; shots (CH3), explosions (CH2) and the 1UP jingle (CH4) already use other PCM channels. A new 2 s hum (`res/sounds/shield_hum.wav`, PCM id 71) plays on CH1 (the existing shield channel) for the whole time the shield is up: 2 s on HARD (one play), 3 s on NORMAL and 5 s on EASY (the sample is retriggered when it ends). It starts with the original activation attack so the press still has a punch, then sits under the action as a quieter drone.
* The hum is stopped when the shield timer ends, on pause, on death, on GAME OVER and on the title screen. If the shield is still active after unpausing or after a respawn, it starts again. During the between-wave warp the shield stays up, so the hum stays too. The "shield ready" ding remains on CH1 and only plays after the hum has stopped. The unused short `SFX_SHIELD_ON` sample is still in the ROM but is no longer started on its own.

**Charged shot (13/11/2026)**
* Fire now happens on button release so a hold can charge. A short tap (under ~1 s, 60 NTSC / 50 PAL frames) still fires the usual 8x8 shot and uses the RapidFire cap (3 or 6). Holding B for about a second arms a charged shot: an orange 16x16 aura appears around the ship, then blinks white when ready and a short lock-on ding plays on CH4 (priority 10, below the 1UP jingle; it never uses CH1, so it cannot cut the shield hum). Releasing then fires a 24x24 bolt (3x the normal shot, one 3x3 hardware sprite, slightly faster at -4 px/frame) that pierces every enemy it touches, awarding the usual per-ship points, until it leaves the screen.
* Only one charged bolt can be on screen; it has its own slot and does not count against the RapidFire cap. If one is still flying, a charged release falls back to a normal shot so the button is never a dead input. The charge is cancelled (no shot) when a wave-warp starts, on death, and on the title / GAME OVER screens. After pause, if B was released while paused, the charge is dropped without firing. RapidFire is unchanged: it only raises the small-bullet cap; charging works the same with it on or off.
**Charged-shot boom (13/11/2026)**
* The charged-shot release sample was thinner and higher than the normal laser (peak 16249, RMS 6733, attack around 1.8 kHz). It is now a ~0.32 s bass boom (sweep about 280 Hz down to 70 Hz) with a short click so it still reads as a shot. Peak 20000, just under the player explosion (20814) and louder than the laser (17972); RMS about 9600, close to the laser, so it stays present without clipping. Still played on CH3, so it cannot cut the shield hum on CH1. The charge-ready ding is slightly fuller (added a lower harmonic) but remains quieter than the boom and the 1UP jingle.

