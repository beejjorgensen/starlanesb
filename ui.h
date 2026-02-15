#ifndef UI_H
#define UI_H

#include "conf.h"
#include CURSES_HEADER

void color_setup(void);
void center(WINDOW * win, int width, int row, char *s);
void redraw(void);
void clear_general(char *s, int blink);
int my_mvwgetstr(WINDOW * win, int y, int x, int max, int num_only,
                 char *s);
#endif
