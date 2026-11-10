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
* D-pad left/right: move the ship.
* B: shoot.
* A: toggle the RapidFire powerup (up to 6 bullets on screen; expires on its own).
* START: pause / resume (the pause screen shows the game version). Restarts after GAME OVER or YOU WIN.
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
*   11/10/2026

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

TODO:
*       Compose a real intro (XGM track or a proper short jingle); the current one is a trimmed placeholder
*       Start a list of where to take the base "game" to make something better, here's some ideas:
        - Add more enemies, control a second row.
        - Change sprites, use a different one for enemies, maybe 2
        - Add an explosion sprite for the player when hit by enemy fire.
        - Can two enemies fire at once?
        - More powerups: ship speed up? a bomb?
        - Formation steps down on each turn (classic invaders) and GAME OVER if it reaches the player.
