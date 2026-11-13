#ifndef SCORES_H
#define SCORES_H

#include <genesis.h>

#define SCORES_TOP      5
#define SCORES_NAME_LEN 3

typedef struct
{
    char name[SCORES_NAME_LEN]; /* not NUL-terminated */
    u8   difficulty;            /* DIFF_EASY / DIFF_NORMAL / DIFF_HARD */
    u32  score;
    u16  wave;
} ScoreEntry;

void scores_init(void);

/* 1 if this score beats the current 5th place (ties stay out). */
u16  scores_qualifies(u32 score);

/* Inserts, saves to SRAM and returns the 0-based rank, or 0xFFFF if it does not qualify. */
u16  scores_submit(const char* name, u32 score, u16 wave, u8 difficulty);

const ScoreEntry* scores_at(u16 i);
char scores_diffLetter(u8 difficulty);

/* highlightIndex -1 = no highlight. startRow is the first of 5 lines. */
void scores_drawTable(u16 startRow, s16 highlightIndex);

/* Arcade 3-letter entry used on the GAME OVER path. */
void scores_entryStart(u32 score, u16 wave, u8 difficulty);
void scores_entryDraw(void);
void scores_entryUpdate(void);
/* Returns 1 when the name has been confirmed and written to the table. */
u16  scores_entryJoy(u16 changed, u16 state);
s16  scores_entryRank(void);

#endif
