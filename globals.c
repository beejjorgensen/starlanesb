#include "globals.h"

char *VERSION = "1.4.0";

int MAPX = 12;                  // X dimension of map
int MAPY = 10;                  // Y dimension of map
int SCREEN_LINES;               // Lines in screen
int SCREEN_COLUMNS;             // Columns in screen
char *map;                      // Pointer to the map data
PLAYER *pl;                     // Pointer to array of players
COMPANY *co;                    // Pointer to array of companies
int numplayers, turn;           // Number of players, whose turn it is
WINDOW *mapwin, *general, *coinfo;      // Pointers to the windows
int color;                      // True if we want color

