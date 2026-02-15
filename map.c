#include "conf.h"
#include CURSES_HEADER
#include "colors.h"
#include "consts.h"
#include "globals.h"
#include "map.h"
#include "ui.h"

/**
 * Draws the map in the map window
 */
void showmap(void)
{
    int i, j, attrs;

    wattron(mapwin, MAP_BORDER);
    box(mapwin, '|', '-');
    wattroff(mapwin, MAP_BORDER);

    wattron(mapwin, MAP_TITLE);
    center(mapwin, MAPX * 3 + 2, 0, " Map ");
    wattroff(mapwin, MAP_TITLE);

    for (i = 0; i < MAPY; i++)
        for (j = 0; j < MAPX; j++) {
            switch (map[j + i * MAPX]) {
            case SPACE:
                attrs = MAP_SPACE;
                break;
            case STAR:
                attrs = MAP_STAR;
                break;
            case NEWCO:
                attrs = MAP_NEWCO;
                break;
            case BLACKHOLE:
                attrs = MAP_BLACKHOLE;
                break;
            case 'A':
                attrs = CO_A;
                break;
            case 'B':
                attrs = CO_B;
                break;
            case 'C':
                attrs = CO_C;
                break;
            case 'D':
                attrs = CO_D;
                break;
            case 'E':
                attrs = CO_E;
                break;
            default:
                attrs = A_NORMAL;
            }
            wattron(mapwin, attrs);
            mvwaddch(mapwin, i + 1, j * 3 + 2, map[j + i * MAPX]);
            wattroff(mapwin, attrs);
        }

    wnoutrefresh(mapwin);
}

/**
 * Puts items on the map
 */
void drawmap(int loc, char c)
{
    int attrs;

    switch (c) {
    case SPACE:
        attrs = MAP_SPACE;
        break;
    case STAR:
        attrs = MAP_STAR;
        break;
    case NEWCO:
        attrs = MAP_NEWCO;
        break;
    case BLACKHOLE:
        attrs = MAP_BLACKHOLE;
        break;
    case 'A':
        attrs = CO_A;
        break;
    case 'B':
        attrs = CO_B;
        break;
    case 'C':
        attrs = CO_C;
        break;
    case 'D':
        attrs = CO_D;
        break;
    case 'E':
        attrs = CO_E;
        break;
    default:
        attrs = A_NORMAL;
    }
    wattron(mapwin, attrs);
    mvwaddch(mapwin, loc / MAPX + 1, (loc % MAPX) * 3 + 2, c);
    wattroff(mapwin, attrs);
}

