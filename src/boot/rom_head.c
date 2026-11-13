#include "genesis.h"

__attribute__((externally_visible))
const ROMHeader rom_header = {
#if (ENABLE_BANK_SWITCH != 0)
    "SEGA SSF        ",
#elif (MODULE_MEGAWIFI != 0)
    "SEGA MEGAWIFI   ",
#else
    "SEGA MEGA DRIVE ",
#endif
    "(C)AMH  2026.NOV",
    "GENS INVADERS                                   ",
    "GENS INVADERS                                   ",
    "GM 00000000-00",
    0x000,
    "JD              ",
    0x00000000,
#if (ENABLE_BANK_SWITCH != 0)
    0x003FFFFF,
#else
    0x000FFFFF,
#endif
    0xE0FF0000,
    0xE0FFFFFF,
    /* SRAM: "RA" + 0xF820 (8-bit, odd addresses) at 0x200000-0x20FFFF.
     * Needed so emulators (BlastEm) allocate a .sram file for the TOP 5. */
    "RA",
    0xF820,
    0x00200000,
    0x0020FFFF,
    "            ",
    "GENS INVADERS - HOMEBREW BY AMH         ",
    "JUE             "
};
