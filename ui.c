#include <string.h>
#include <ctype.h>
#include "conf.h"
#include CURSES_HEADER
#include "colors.h"
#include "consts.h"
#include "globals.h"
#include "ui.h"

/**
 * Centers a piece of text on a window on the specified row.
 */
void center(WINDOW *win, int width, int row, char *s)
{
    mvwaddstr(win, row, (width - strlen(s)) / 2, s);
}

/**
 * Sets up the color pairs
 */
void color_setup(void)
{
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    init_pair(8, COLOR_YELLOW, COLOR_BLUE);
    init_pair(9, COLOR_WHITE, COLOR_BLUE);
    init_pair(10, COLOR_BLACK, COLOR_YELLOW);
    init_pair(11, COLOR_BLACK, COLOR_WHITE);
    init_pair(12, COLOR_BLACK, COLOR_RED);
    init_pair(13, COLOR_BLACK, COLOR_BLUE);
    init_pair(14, COLOR_BLACK, COLOR_GREEN);
}

/**
 * Redraws the screen.
 */
void redraw(void)
{
    touchwin(stdscr);
    touchwin(mapwin);
    touchwin(coinfo);
    touchwin(general);
    refresh();
    wrefresh(mapwin);
    wrefresh(coinfo);
    wrefresh(general);
}

/**
 * Clears and titles the general window.
 */
void clear_general(char *s, int blink)
{
    werase(general);
    wattron(general, GENERAL_BORDER);
    box(general, '|', '/');
    wattroff(general, GENERAL_BORDER);
    wattron(general, blink ? GENERAL_TITLE_BLINK : GENERAL_TITLE);
    center(general, COLUMNS - 2, 0, s);
    wattroff(general, blink ? GENERAL_TITLE_BLINK : GENERAL_TITLE);
}

/**
 * Does mywgetstr my way.
 */
int my_mvwgetstr(WINDOW *win, int y, int x, int max, int num_only, char *s)
{
    int cur = 0, c, done = 0;

    s[0] = '\0';

    nl();
    noecho();
    raw();
    keypad(stdscr, 0);
    while (doupdate(), c = getch(), !done && (c != LF && c != CR)) {
        if (c == CTRL_L) {
            redraw();
            continue;
        }
        if (c == erasechar() || c == BS || c == DEL) {
            if (cur) {
                cur--;
                mvwaddch(win, y, x + cur, ' ');
                s[cur] = '\0';
                wmove(win, y, x + cur);
                wnoutrefresh(win);
            }
        } else {
            if (num_only == 1) {        // Only numbers and '-' allowed
                if (!isdigit(c) && c != '-' && tolower(c) != 'm'
                    && tolower(c) != 'n')
                    continue;
                if (tolower(c) == 'm' || tolower(c) == 'n') {
                    if (cur == 0)
                        done = 1;
                    else
                        continue;
                }
                if (c == '-' && cur != 0)
                    continue;
            }
            if (cur < max) {
                mvwaddch(win, y, x + cur, c);
                s[cur] = c;
                s[cur + 1] = '\0';
                cur++;
                wnoutrefresh(win);
            }
        }
    }

    nonl();
    keypad(stdscr, 1);
    return strlen(s);
}

