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

CODER: 
*   AMH - MANZANAS
INITIAL_DATE: 
*   07/16/2021
LAST_MODIFY_DATE:
*   06/07/2023

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

TODO:
*       Maybe organize and clean code
*       Check why the sound is not working
*       Start a list of where to take the base "game" to make something better, here's some ideas:
        - Add more enemies, control a second row.
        - Change sprites, use a different one for enemies, maybe 2
        - Add an explosion sprite for the player when hit by enemy fire.
        - Can two enemies fire at once?
        - More powerups: ship speed up? a bomb?