#ifndef TITLE_H
#define TITLE_H

#include <genesis.h>

#define TITLE_OPT_START  0
#define TITLE_OPT_DIFF   1
#define TITLE_OPT_SCORES 2
#define TITLE_OPT_COUNT  3

void title_show(void);
void title_clear(void);
void title_drawMenu(void);
void title_cycleDifficulty(s16 dir);
void title_updatePulse(void);
u16  title_cursor(void);
void title_setCursor(u16 cursor);
void title_showScores(void);
void title_hideScores(void);
u16  title_isScoresView(void);

#endif
