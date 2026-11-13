#include <genesis.h>
#include "scores.h"
#include "hud.h"
#include "difficulty.h"

#define SRAM_MAGIC_0     'G'
#define SRAM_MAGIC_1     'I'
#define SRAM_MAGIC_2     'H'
#define SRAM_MAGIC_3     'S'
#define SRAM_FORMAT      1

#define OFF_MAGIC        0
#define OFF_FORMAT       4
#define OFF_CHECK        6
#define OFF_ENTRIES      8
#define ENTRY_BYTES      10  /* 3 name + 1 diff + 4 score + 2 wave */
#define SRAM_BYTES       (OFF_ENTRIES + SCORES_TOP * ENTRY_BYTES)

#define ALPHA_LEN        28  /* A-Z, space, '.' */

static const char ALPHA[ALPHA_LEN] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ .";

static const ScoreEntry kDefaults[SCORES_TOP] = {
    { { 'G', 'E', 'N' }, DIFF_NORMAL, 1000, 1 },
    { { 'I', 'N', 'V' }, DIFF_NORMAL,  800, 1 },
    { { 'U', 'D', 'L' }, DIFF_EASY,    600, 1 },
    { { 'A', 'M', 'H' }, DIFF_NORMAL,  400, 1 },
    { { 'B', 'I', 'M' }, DIFF_EASY,    200, 1 }
};

static ScoreEntry table[SCORES_TOP];

static char  entryName[SCORES_NAME_LEN];
static u16   entryCursor;
static u16   entryBlink;
static u16   entryTick;
static u32   entryScore;
static u16   entryWave;
static u8    entryDiff;
static s16   entryRank;

static u8 sramGet(u32 off) {
    return SRAM_readByte(off);
}

static void sramPut(u32 off, u8 val) {
    SRAM_writeByte(off, val);
}

static u16 checksumPayload(void) {
    u16 sum = 0;
    u32 i;
    sum = (u16)SRAM_MAGIC_0 + SRAM_MAGIC_1 + SRAM_MAGIC_2 + SRAM_MAGIC_3;
    sum += SRAM_FORMAT;
    for (i = OFF_ENTRIES; i < SRAM_BYTES; i++) {
        sum += sramGet(i);
    }
    return sum;
}

static void packEntry(u32 off, const ScoreEntry* e) {
    sramPut(off + 0, (u8)e->name[0]);
    sramPut(off + 1, (u8)e->name[1]);
    sramPut(off + 2, (u8)e->name[2]);
    sramPut(off + 3, e->difficulty);
    sramPut(off + 4, (u8)(e->score >> 24));
    sramPut(off + 5, (u8)(e->score >> 16));
    sramPut(off + 6, (u8)(e->score >> 8));
    sramPut(off + 7, (u8)(e->score));
    sramPut(off + 8, (u8)(e->wave >> 8));
    sramPut(off + 9, (u8)(e->wave));
}

static void unpackEntry(u32 off, ScoreEntry* e) {
    e->name[0] = (char)sramGet(off + 0);
    e->name[1] = (char)sramGet(off + 1);
    e->name[2] = (char)sramGet(off + 2);
    e->difficulty = sramGet(off + 3);
    e->score = ((u32)sramGet(off + 4) << 24)
             | ((u32)sramGet(off + 5) << 16)
             | ((u32)sramGet(off + 6) << 8)
             |  (u32)sramGet(off + 7);
    e->wave = ((u16)sramGet(off + 8) << 8) | (u16)sramGet(off + 9);
}

static u16 nameOk(char c) {
    if ((c >= 'A' && c <= 'Z') || c == ' ' || c == '.') return 1;
    return 0;
}

static u16 tableLooksValid(void) {
    u16 i, k;
    char c;
    for (i = 0; i < SCORES_TOP; i++) {
        if (table[i].difficulty >= DIFF_COUNT) return 0;
        if (table[i].wave == 0) return 0;
        for (k = 0; k < SCORES_NAME_LEN; k++) {
            c = table[i].name[k];
            if (!nameOk(c)) return 0;
        }
    }
    return 1;
}

static void loadDefaults(void) {
    u16 i;
    for (i = 0; i < SCORES_TOP; i++) {
        table[i] = kDefaults[i];
    }
}

static void writeSram(void) {
    u16 i;
    u16 sum;
    SRAM_enable();
    sramPut(OFF_MAGIC + 0, SRAM_MAGIC_0);
    sramPut(OFF_MAGIC + 1, SRAM_MAGIC_1);
    sramPut(OFF_MAGIC + 2, SRAM_MAGIC_2);
    sramPut(OFF_MAGIC + 3, SRAM_MAGIC_3);
    sramPut(OFF_FORMAT, SRAM_FORMAT);
    sramPut(OFF_FORMAT + 1, 0);
    for (i = 0; i < SCORES_TOP; i++) {
        packEntry(OFF_ENTRIES + (u32)i * ENTRY_BYTES, &table[i]);
    }
    sum = checksumPayload();
    sramPut(OFF_CHECK, (u8)(sum >> 8));
    sramPut(OFF_CHECK + 1, (u8)sum);
    SRAM_disable();
}

static u16 readSram(void) {
    u16 i;
    u16 stored;
    u16 sum;
    SRAM_enableRO();
    if (sramGet(OFF_MAGIC + 0) != SRAM_MAGIC_0 ||
        sramGet(OFF_MAGIC + 1) != SRAM_MAGIC_1 ||
        sramGet(OFF_MAGIC + 2) != SRAM_MAGIC_2 ||
        sramGet(OFF_MAGIC + 3) != SRAM_MAGIC_3 ||
        sramGet(OFF_FORMAT) != SRAM_FORMAT) {
        SRAM_disable();
        return 0;
    }
    for (i = 0; i < SCORES_TOP; i++) {
        unpackEntry(OFF_ENTRIES + (u32)i * ENTRY_BYTES, &table[i]);
    }
    stored = ((u16)sramGet(OFF_CHECK) << 8) | (u16)sramGet(OFF_CHECK + 1);
    sum = checksumPayload();
    SRAM_disable();
    if (stored != sum) return 0;
    if (!tableLooksValid()) return 0;
    return 1;
}

void scores_init(void) {
    if (!readSram()) {
        loadDefaults();
        writeSram();
    }
}

u16 scores_qualifies(u32 score) {
    return (score > table[SCORES_TOP - 1].score) ? 1 : 0;
}

u16 scores_submit(const char* name, u32 score, u16 wave, u8 difficulty) {
    u16 i;
    u16 slot;
    ScoreEntry neu;

    if (!scores_qualifies(score)) return 0xFFFF;

    neu.name[0] = name[0];
    neu.name[1] = name[1];
    neu.name[2] = name[2];
    if (difficulty >= DIFF_COUNT) difficulty = DIFF_NORMAL;
    neu.difficulty = difficulty;
    neu.score = score;
    neu.wave = (wave == 0) ? 1 : wave;

    slot = SCORES_TOP - 1;
    for (i = 0; i < SCORES_TOP; i++) {
        if (score > table[i].score) {
            slot = i;
            break;
        }
    }
    for (i = SCORES_TOP - 1; i > slot; i--) {
        table[i] = table[i - 1];
    }
    table[slot] = neu;
    writeSram();
    return slot;
}

const ScoreEntry* scores_at(u16 i) {
    if (i >= SCORES_TOP) return &table[0];
    return &table[i];
}

char scores_diffLetter(u8 difficulty) {
    if (difficulty == DIFF_EASY) return 'F';
    if (difficulty == DIFF_HARD) return 'D';
    return 'N';
}

void scores_drawTable(u16 startRow, s16 highlightIndex) {
    char line[40];
    u16 i;
    const ScoreEntry* e;
    char n0, n1, n2;

    for (i = 0; i < SCORES_TOP; i++) {
        e = &table[i];
        n0 = e->name[0];
        n1 = e->name[1];
        n2 = e->name[2];
        /* Spaces in a name would collapse visually; show a dot instead. */
        if (n0 == ' ') n0 = '.';
        if (n1 == ' ') n1 = '.';
        if (n2 == ' ') n2 = '.';
        sprintf(line, "%c %d. %c%c%c  %lu  OLA %u  %c",
                ((s16)i == highlightIndex) ? '>' : ' ',
                i + 1, n0, n1, n2,
                (unsigned long)e->score, e->wave,
                scores_diffLetter(e->difficulty));
        hud_drawCentered(line, startRow + i);
    }
}

static u16 letterIndex(char c) {
    u16 i;
    for (i = 0; i < ALPHA_LEN; i++) {
        if (ALPHA[i] == c) return i;
    }
    return 0;
}

static void stepLetter(s16 dir) {
    u16 idx = letterIndex(entryName[entryCursor]);
    if (dir > 0) {
        idx++;
        if (idx >= ALPHA_LEN) idx = 0;
    } else {
        if (idx == 0) idx = ALPHA_LEN - 1;
        else idx--;
    }
    entryName[entryCursor] = ALPHA[idx];
}

static void drawEntryName(void) {
    char line[20];
    char a, b, c;
    a = entryName[0];
    b = entryName[1];
    c = entryName[2];
    if (a == ' ') a = '_';
    if (b == ' ') b = '_';
    if (c == ' ') c = '_';
    /* Blink the cursor letter to an underscore so you can tell which one. */
    if (entryBlink) {
        if (entryCursor == 0) a = '_';
        else if (entryCursor == 1) b = '_';
        else c = '_';
    }
    sprintf(line, "NOMBRE: %c%c%c", a, b, c);
    VDP_clearText(0, 15, 40);
    hud_drawCentered(line, 15);
}

void scores_entryStart(u32 score, u16 wave, u8 difficulty) {
    entryName[0] = 'A';
    entryName[1] = 'A';
    entryName[2] = 'A';
    entryCursor = 0;
    entryBlink = 0;
    entryTick = 0;
    entryScore = score;
    entryWave = wave;
    entryDiff = difficulty;
    entryRank = -1;
}

void scores_entryDraw(void) {
    char line[32];
    hud_drawCentered("GAME OVER", 10);
    sprintf(line, "PUNTAJE: %lu", (unsigned long)entryScore);
    hud_drawCentered(line, 12);
    sprintf(line, "OLEADA %u  -  %s", entryWave, difficulty_name());
    hud_drawCentered(line, 13);
    drawEntryName();
    hud_drawCentered("ARRIBA/ABAJO: LETRA", 17);
    hud_drawCentered("IZQ/DER: CURSOR", 18);
    hud_drawCentered("A O START: CONFIRMA", 19);
}

void scores_entryUpdate(void) {
    entryTick++;
    if ((entryTick & 15) == 0) {
        entryBlink ^= 1;
        drawEntryName();
    }
}

u16 scores_entryJoy(u16 changed, u16 state) {
    if (state & BUTTON_UP & changed) {
        stepLetter(1);
        entryBlink = 0;
        entryTick = 0;
        drawEntryName();
    } else if (state & BUTTON_DOWN & changed) {
        stepLetter(-1);
        entryBlink = 0;
        entryTick = 0;
        drawEntryName();
    }
    if (state & BUTTON_LEFT & changed) {
        if (entryCursor > 0) entryCursor--;
        entryBlink = 0;
        entryTick = 0;
        drawEntryName();
    } else if (state & BUTTON_RIGHT & changed) {
        if (entryCursor < (SCORES_NAME_LEN - 1)) entryCursor++;
        entryBlink = 0;
        entryTick = 0;
        drawEntryName();
    }
    if ((state & BUTTON_START & changed) || (state & BUTTON_A & changed)) {
        entryRank = (s16)scores_submit(entryName, entryScore, entryWave, entryDiff);
        return 1;
    }
    return 0;
}

s16 scores_entryRank(void) {
    return entryRank;
}
