#ifndef GLOBALS_H
#define GLOBALS_H

#include "conf.h"
#include CURSES_HEADER
#include "company.h"
#include "player.h"

extern char *VERSION;

extern int MAPX;                       // X dimension of map
extern int MAPY;                       // Y dimension of map
extern int SCREEN_LINES;               // Lines in screen
extern int SCREEN_COLUMNS;             // Columns in screen
extern char *map;                      // Pointer to the map data
extern PLAYER *pl;                     // Pointer to array of players
extern COMPANY *co;                    // Pointer to array of companies
extern int numplayers, turn;           // Number of players, whose turn it is
extern WINDOW *mapwin, *general, *coinfo;      // Pointers to the windows
extern int color;                      // True if we want color

#endif
