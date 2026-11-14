#ifndef TITLE_H
#define TITLE_H

#include <genesis.h>

#define TITLE_OPT_START  0
#define TITLE_OPT_DIFF   1
#define TITLE_OPT_SCORES 2
#define TITLE_OPT_WAVE   3  /* only visible after the title-screen code */
#define TITLE_OPT_COUNT  3  /* locked menu; title_optionCount() is 4 once unlocked */
#define START_WAVE_MIN   1
#define START_WAVE_MAX  99

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
/* Title-screen easter egg (Konami-style). onButton returns 1 on the frame
 * the code is completed. Unlock and the start-wave choice last until reset. */
u16  title_onButton(u16 changed, u16 state);
u16  title_cheatUnlocked(void);
u16  title_startWave(void);
void title_cycleStartWave(s16 dir);
u16  title_optionCount(void);

#endif
