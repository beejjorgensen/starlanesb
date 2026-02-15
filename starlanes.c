#include "conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include CURSES_HEADER
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include "globals.h"
#include "colors.h"
#include "consts.h"
#include "company.h"
#include "player.h"
#include "ui.h"
#include "map.h"
#include "announce.h"

// Function prototypes

void initialize(void);
void get_num_players(void);
int get_move(void);
void do_move(int move);
void do_merge(int *c1, int *c2, int *o1, int *o2);
void holding_bonus(void);
void buy_sell(void);
int check_endgame(void);
int count_used_sectors(void);
void calc_cost(int cnum, int move, int n, int s, int w, int e);
void show_standings(char *title);
int order_compare(const void *v1, const void *v2);
void quit_yn(void);
void shutdown(void);
void usage(void);

/**
 * Sets up the map, players, and companies
 */
void initialize(void)
{
    int i, j;
    char *lines, *columns;

    // Get the size of the screen

    if ((lines = getenv("LINES")) == NULL
        || (columns = getenv("COLUMNS")) == NULL) {
        LINES = DEF_LINES;
        COLUMNS = DEF_COLUMNS;
    } else {
        LINES = atoi(lines);
        COLUMNS = atoi(columns);
    }

    // Allocate space for everything

    if ((map = malloc(MAPX * MAPY)) == NULL) {
        fprintf(stderr, "starlanes: error mallocing space for map\n");
        exit(1);
    }

    if ((co = calloc(1, NUMCO * sizeof(COMPANY))) == NULL) {
        fprintf(stderr,
                "starlanes: error mallocing space for companies\n");
        exit(1);
    }

    if ((pl = calloc(1, MAXPLAYERS * sizeof(PLAYER))) == NULL) {
        fprintf(stderr, "starlanes: error mallocing space for players\n");
        exit(1);
    }

    // Set up the map

    for (i = 0; i < MAPX; i++) {
        for (j = 0; j < MAPY; j++) {
            int n = rand() % 60;
            if (n == 0)
                map[i + j * MAPX] = BLACKHOLE;  // 1/60 chance
            else if (n <= 3)
                map[i + j * MAPX] = STAR;       // 3/60 chance
            else
                map[i + j * MAPX] = SPACE;      // 56/60 chance
        }
    }

    // Initialize the companies

    for (i = 0; i < NUMCO; i++) {
        switch (i) {
        case 0:
            strcpy(co[i].name, "Altair Starways");
            break;
        case 1:
            strcpy(co[i].name, "Beetlejuice, Ltd.");
            break;
        case 2:
            strcpy(co[i].name, "Capella Freight Co.");
            break;
        case 3:
            strcpy(co[i].name, "Denebola Shippers");
            break;
        case 4:
            strcpy(co[i].name, "Eridani Expediters");
            break;
        }
        co[i].size = co[i].price = 0;
    }

    // Initialize the players

    for (i = 0; i < MAXPLAYERS; i++) {
        pl[i].name[0] = '\0';
        pl[i].cash = INIT_CASH;
        for (j = 0; j < NUMCO; j++)
            pl[i].holdings[j] = 0;
    }

    // Create the windows

    if ((mapwin = newwin(MAPY + 2, MAPX * 3 + 2, 1, 1)) == NULL) {
        fprintf(stderr, "starlanes: can't create map window\n");
        exit(1);
    }

    if ((general =
         newwin(LINES - 2 - MAPY - 2, COLUMNS - 2, MAPY + 3, 1)) == NULL) {
        fprintf(stderr, "starlanes: can't create general window\n");
        exit(1);
    }

    if ((coinfo =
         newwin(MAPY + 2, COLUMNS - 2 - (MAPX * 3 + 2), 1,
                MAPX * 3 + 3)) == NULL) {
        fprintf(stderr, "starlanes: can't create coinfo window\n");
        exit(1);
    }

    // Set keypad mode so the arrow keys work

    keypad(stdscr, 1);
    keypad(general, 1);
}

/**
 * Prompts for the number of players
 */
void get_num_players(void)
{
    char c, s[80];
    int i;

    clear();
    refresh();

    WINDOW *npwin = newwin(LINES, COLUMNS, 0, 0);

    if (color)
        wattron(npwin, YELLOW_ON_BLACK | A_BOLD);

    box(npwin, '$', '$');

    sprintf(s, " V%s ", VERSION);
    center(npwin, COLUMNS, LINES - 1, s);
    if (color)
        wattroff(npwin, YELLOW_ON_BLACK | A_BOLD);

    wattron(npwin, color ? YELLOW_ON_BLUE | A_BOLD : A_REVERSE);
    center(npwin, COLUMNS, 6, "* S * T * A * R ** L * A * N * E * S *");
    wattroff(npwin, color ? YELLOW_ON_BLUE | A_BOLD : A_REVERSE);
    sprintf(s, "Please enter the number of players [1-%d]: ", MAXPLAYERS);
    center(npwin, COLUMNS, 9, s);
    wrefresh(npwin);
    noecho();
    raw();
    do {
        c = getch() - '0';
    } while (c < 1 || c > MAXPLAYERS);
    waddch(npwin, c + '0');

    numplayers = (int) c;
    srand(getpid());            // Reseed the dumb random number generator
    turn = rand() % numplayers;

    nl();
    for (i = 0; i < numplayers; i++) {
        sprintf(s, "Player %d, enter your name: ", i + 1);
        center(npwin, COLUMNS - 8, 11 + i, s);
        wrefresh(npwin);
        my_mvwgetstr(npwin, i + 11, 49, 20, 0, pl[i].name);
        if (pl[i].name[0] == '\0')
            i--;
    }
    nonl();

    delwin(npwin);
    clear();
    refresh();
}

/**
 * Gets a players move
 */
int get_move(void)
{
    char s[160], c;
    int move[NUMMOVES], i, j, ok;

    wattron(mapwin, A_REVERSE);
    for (i = 0; i < NUMMOVES; i++) {
        do {
            ok = 1;
            move[i] = rand() % (MAPX * MAPY);
            if (map[move[i]] != SPACE)
                ok = 0;
            for (j = 0; j < i; j++)
                if (move[j] == move[i])
                    ok = 0;
            if (co_avail() == -1) {     // No more companies
                if ((ripe(up_obj(move[i])) || ripe(down_obj(move[i])) ||
                     ripe(left_obj(move[i])) || ripe(right_obj(move[i])))
                    && !co_near(move[i]))
                    ok = 0;
            }
        } while (!ok);
        mvwaddch(mapwin, move[i] / MAPX + 1, (move[i] % MAPX) * 3 + 2,
                 i + 1 + '0');
    }
    wattroff(mapwin, A_REVERSE);
    wnoutrefresh(mapwin);

    sprintf(s, " %s (Cash: $%d) ", pl[turn].name, pl[turn].cash);
    clear_general(s, 0);
    sprintf(s, " %s, enter your move [1-%d]: ", pl[turn].name, NUMMOVES);
    mvwprintw(general, 2, 1, s);
    show_coinfo();
    noecho();
    do {
        wmove(general, 2, 1 + strlen(s));
        wnoutrefresh(general);
        doupdate();
        c = getch();
        if (c == CTRL_L) {
            redraw();
            continue;
        }
        if (c == CTRL_C || toupper(c) == 'Q') {
            quit_yn();
            continue;
        }
        if (toupper(c) == 'S') {
            show_standings(" Current Standings ");
            continue;
        }
        if (toupper(c) == 'C') {
            more_coinfo();
            continue;
        }

        c -= '0';
    } while (c < 1 || c > NUMMOVES);
    echo();

    for (i = 0; i < NUMMOVES; i++)
        mvwaddch(mapwin, move[i] / MAPX + 1, (move[i] % MAPX) * 3 + 2,
                 SPACE);
    wnoutrefresh(mapwin);

    return move[c - 1];
}

/**
 * Make the move, and follow up on the consequences if it does.
 */
void do_move(int move)
{
    int north, south, west, east, newc;
    char newc_type;

    north = up_obj(move);
    south = down_obj(move);
    west = left_obj(move);
    east = right_obj(move);

    if (s_or_bh(north) && s_or_bh(south) && s_or_bh(east) && s_or_bh(west)) {
        if (north == BLACKHOLE || south == BLACKHOLE || west == BLACKHOLE
            || east == BLACKHOLE) {
            suck_announce(-1, move);
            return;
        } else {
            map[move] = NEWCO;  // No one around
            drawmap(move, NEWCO);
            wnoutrefresh(mapwin);
        }
    } else {                    // Check for merge
        if (iscompany(north) && iscompany(south) && north != south) {
            do_merge(&north, &south, &west, &east);
        }

        if (iscompany(north) && iscompany(east) && north != east) {
            do_merge(&north, &east, &south, &west);
        }

        if (iscompany(north) && iscompany(west) && north != west) {
            do_merge(&north, &west, &south, &east);
        }

        if (iscompany(west) && iscompany(east) && west != east) {
            do_merge(&west, &east, &north, &south);
        }

        if (iscompany(east) && iscompany(south) && east != south) {
            do_merge(&east, &south, &west, &north);
        }

        if (iscompany(west) && iscompany(south) && west != south) {
            do_merge(&west, &south, &east, &north);
        }
    }

    if ((ripe(east) || ripe(west) || ripe(north) || ripe(south))
        && !co_near(move)) {
        if ((newc = co_avail()) == -1) {
            map[move] = NEWCO;
            drawmap(move, NEWCO);
            wnoutrefresh(mapwin);
        } else {                // New company!
            map[move] = newc + 'A';
            drawmap(move, newc + 'A');
            co[newc].price = INIT_CO_COST;
            co[newc].size = 1;
            calc_cost(newc, move, north, south, west, east);
            if (co[newc].price <= 0) {  // Too close to black hole
                suck_announce(newc, 0);
            } else {            // Came to life just fine
                pl[turn].holdings[newc] = FOUNDER_BONUS;
                show_coinfo();
                wnoutrefresh(mapwin);
                new_co_announce(newc);
            }
        }
    } else if (map[move] == SPACE) {    // Adding on
        if (iscompany(west))
            newc_type = west;
        if (iscompany(east))
            newc_type = east;
        if (iscompany(south))
            newc_type = south;
        if (iscompany(north))
            newc_type = north;
        map[move] = newc_type;
        drawmap(move, newc_type);
        wnoutrefresh(mapwin);
        co[newc_type - 'A'].size++;
        co[newc_type - 'A'].price += NEWCOCOST;
        calc_cost(newc_type - 'A', move, north, south, west, east);
        if (co[newc_type - 'A'].price <= 0)     // Black holed
            suck_announce(newc_type - 'A', 1);
        else {
            if (co[newc_type - 'A'].price > SPLIT_PRICE)
                split_announce(newc_type - 'A');
            else
                show_coinfo();
        }
    }
}

/**
 * Does all the nasty business behind a merge
 */
void do_merge(int *c1, int *c2, int *o1, int *o2)
{
    int t, i, cb, cs, doswap = 0;

    cb = *c1 - 'A';
    cs = *c2 - 'A';

    if (co[cs].size == co[cb].size) {   // If same size, check prices
        int pb = 0, ps = 0;
        for (i = 0; i < numplayers; i++) {      // Calculate worth of companies
            pb += co[cb].price * pl[i].holdings[cb];
            ps += co[cs].price * pl[i].holdings[cs];
        }

        if (ps > pb)            // If smaller co has higher worth, swap 'em
            doswap = 1;
        else if (ps == pb)      // If same price, choose rand
            doswap = rand() % 2;
    }

    if (co[cs].size > co[cb].size || doswap) {  // cb = merger, cs = mergee
        t = cs;
        cs = cb;
        cb = t;
    }

    for (i = 0; i < MAPX * MAPY; i++)
        if (map[i] == cs + 'A') {
            map[i] = cb + 'A';
            drawmap(i, cb + 'A');
        }

    *c1 = *c2 = cb + 'A';       // Convert to new co
    if (*o1 == cs + 'A')
        *o1 = cb + 'A';
    if (*o2 == cs + 'A')
        *o2 = cb + 'A';

    co[cb].size += co[cs].size;
    co[cb].price += co[cs].price;
    co[cs].size = 0;

    wnoutrefresh(mapwin);       // Show the players what's up

    merge_announce(cb, cs);
    xaction_announce(cb, cs);
    if (co[cb].price > SPLIT_PRICE)
        split_announce(cb);
}

/**
 * Adds value to a company based on surroundings and converts NEWCOs to
 * the company
 */
void calc_cost(int cnum, int move, int n, int s, int w, int e)
{
    if (n == STAR)
        co[cnum].price += STARCOST;     // Stars
    if (s == STAR)
        co[cnum].price += STARCOST;
    if (w == STAR)
        co[cnum].price += STARCOST;
    if (e == STAR)
        co[cnum].price += STARCOST;

    if (n == BLACKHOLE)
        co[cnum].price += BLACKHOLECOST;        // Black holes
    if (s == BLACKHOLE)
        co[cnum].price += BLACKHOLECOST;
    if (w == BLACKHOLE)
        co[cnum].price += BLACKHOLECOST;
    if (e == BLACKHOLE)
        co[cnum].price += BLACKHOLECOST;

    if (n == NEWCO) {           // Starter companies
        map[move - MAPX] = cnum + 'A';
        drawmap(move - MAPX, cnum + 'A');
        co[cnum].size++;
        co[cnum].price += NEWCOCOST;
    }
    if (s == NEWCO) {
        map[move + MAPX] = cnum + 'A';
        drawmap(move + MAPX, cnum + 'A');
        co[cnum].size++;
        co[cnum].price += NEWCOCOST;
    }
    if (w == NEWCO) {
        map[move - 1] = cnum + 'A';
        drawmap(move - 1, cnum + 'A');
        co[cnum].size++;
        co[cnum].price += NEWCOCOST;
    }
    if (e == NEWCO) {
        map[move + 1] = cnum + 'A';
        drawmap(move + 1, cnum + 'A');
        co[cnum].size++;
        co[cnum].price += NEWCOCOST;
    }
    wnoutrefresh(mapwin);
}

/**
 * Gives bonus to player based on current holdings.
 */
void holding_bonus(void)
{
    int i;

    for (i = 0; i < NUMCO; i++) {
        if (co[i].size) {
            pl[turn].cash +=
                (int) (0.05 * ((float) pl[turn].holdings[i]) *
                       ((float) co[i].price));
        }
    }
}

/**
 * Screen that allows the user to buy and sell stocks.
 */
void buy_sell(void)
{
    int i, cos[NUMCO], cocount = 0, cursor = 0, newcur, done =
        0, amt, max, min, pos1, pos2;
    int attrs;
    char s[160], amtstr[15];

    for (i = 0; i < NUMCO; i++) // Make a list of active companies
        if (co[i].size)
            cos[cocount++] = i;

    if (cocount == 0)
        return;                 // No companies yet

    sprintf(s, " %s (Cash: $%d) ", pl[turn].name, pl[turn].cash);
    clear_general(s, 0);

    center(general, COLUMNS - 2, 1,
           "Arrow keys to select a company, return to trade, escape when done:");

    for (i = 0; i < cocount; i++) {
        switch (cos[i]) {
        case 0:
            attrs = CO_A;
            break;
        case 1:
            attrs = CO_B;
            break;
        case 2:
            attrs = CO_C;
            break;
        case 3:
            attrs = CO_D;
            break;
        case 4:
            attrs = CO_E;
            break;
        }
        if (i == cursor)
            wattron(general, A_REVERSE);
        else
            wattron(general, attrs);
        mvwprintw(general, i + 3, 20, co[cos[i]].name);
        if (i == cursor)
            wattroff(general, A_REVERSE);
        else
            wattroff(general, attrs);
    }
    wnoutrefresh(general);

    do {
        newcur = cursor;
        raw();
        noecho();
        doupdate();
        switch (getch()) {
        case CTRL_L:
            redraw();
            break;
        case 's':
        case 'S':
            show_standings(" Current Standings ");
            wmove(general, cursor + 3, strlen(co[cos[cursor]].name) + 20);
            wnoutrefresh(general);
            break;
        case 'c':
        case 'C':
            more_coinfo();
            wmove(general, cursor + 3, strlen(co[cos[cursor]].name) + 20);
            wnoutrefresh(general);
            break;
        case 'q':
        case 'Q':
        case CTRL_C:
            quit_yn();
            wmove(general, cursor + 3, strlen(co[cos[cursor]].name) + 20);
            wnoutrefresh(general);
            break;
        case KEY_UP:
        case '8':
        case 'k':
        case 'K':
            newcur = cursor ? cursor - 1 : cocount - 1;
            break;
        case KEY_DOWN:
        case '5':
        case 'j':
        case 'J':
            newcur = cursor < cocount - 1 ? cursor + 1 : 0;
            break;
        case KEY_RIGHT:
        case 6:
        case CR:
        case LF:
            max = pl[turn].cash / co[cos[cursor]].price;
            min = -pl[turn].holdings[cos[cursor]];
            sprintf(s, "Amount (%d to %d): ", min, max);
            mvwprintw(general, cursor + 3, 40, s);
            wnoutrefresh(general);
            my_mvwgetstr(general, cursor + 3, 40 + strlen(s), 9, 1,
                         amtstr);
            if (tolower(amtstr[0]) == 'm')
                amt = max;
            else if (tolower(amtstr[0]) == 'n')
                amt = min;
            else
                amt = atoi(amtstr);
            if (amt >= min && amt <= max) {
                pl[turn].cash += (-amt * co[cos[cursor]].price);
                pl[turn].holdings[cos[cursor]] += amt;
            } else {
                mvwprintw(general, cursor + 3, 40,
                          "Invalid amount!                      ");
                wmove(general, cursor + 3, 55);
                wnoutrefresh(general);
                doupdate();
                sleep(1);
            }
            mvwprintw(general, cursor + 3, 40,
                      "                                     ");
            sprintf(s, " %s (Cash: $%d) ", pl[turn].name, pl[turn].cash);

            if (amt)
                show_coinfo();

            pos1 = ((COLUMNS - 2) - strlen(s)) / 2;
            pos2 = pos1 + strlen(s);
            wattron(general, GENERAL_TITLE);
            center(general, COLUMNS - 2, 0, s);
            wattroff(general, GENERAL_TITLE);
            wattron(general, GENERAL_BORDER);
            mvwaddstr(general, 0, pos1 - 4, "////");
            mvwaddstr(general, 0, pos2, "////");
            wattroff(general, GENERAL_BORDER);
            wmove(general, cursor + 3, strlen(co[cos[cursor]].name) + 20);
            wnoutrefresh(general);

            break;

        case ESC:
            done = 1;
        }                       // Switch

        if (cursor != newcur) { // Move cursor
            switch (cos[cursor]) {
            case 0:
                attrs = CO_A;
                break;
            case 1:
                attrs = CO_B;
                break;
            case 2:
                attrs = CO_C;
                break;
            case 3:
                attrs = CO_D;
                break;
            case 4:
                attrs = CO_E;
                break;
            }

            wattron(general, attrs);
            mvwprintw(general, cursor + 3, 20, co[cos[cursor]].name);
            wattroff(general, attrs);

            wattron(general, color ? BLACK_ON_WHITE : A_REVERSE);
            mvwprintw(general, newcur + 3, 20, co[cos[newcur]].name);
            wattroff(general, color ? BLACK_ON_WHITE : A_REVERSE);

            wnoutrefresh(general);
            cursor = newcur;
        }
    } while (!done);
}


/**
 * Returns true if the game is over.
 */
int check_endgame(void)
{
    int sum = 0, i, maptotal;

    for (i = 0; i < NUMCO; i++)
        sum += co[i].size;

    maptotal = count_used_sectors();

    return (sum * 100) / (MAPX * MAPY) >= END_PERCENT
        && maptotal <= MAPX * MAPY - 4;
}

/**
 * Counts the number of non-empty sectors.
 */
int count_used_sectors(void)
{
    int maptotal, i;

    for (i = maptotal = 0; i < MAPX * MAPY; i++)        // Must be enough room to move
        if (map[i] != SPACE)
            maptotal++;

    return maptotal;
}

/**
* Pops up a window with the current standings.
*/
void show_standings(char *title)
{
    WINDOW *stand;
    char s[80];
    int i, j, order[MAXPLAYERS], togo, sum;

    if ((stand =
         newwin(9 + numplayers, 60, 5, (COLUMNS - 60) / 2)) == NULL) {
        fprintf(stderr, "starlanes: couldn't create standings window\n");
        exit(1);
    }

    wattron(stand, STAND_BORDER);
    box(stand, '|', '=');
    wattroff(stand, STAND_BORDER);

    wattron(stand, STAND_TITLE);
    sprintf(s, " %s ", title);
    center(stand, 60, 0, s);
    wattroff(stand, STAND_TITLE);

    wattron(stand, STAND_HEADER);
    mvwaddstr(stand, 2, 2,
              "=Player================Stock Value===Cash=====Net Worth=");
    wattroff(stand, STAND_HEADER);

    for (i = 0; i < numplayers; i++) {
        order[i] = i;
        pl[i].svalue = 0;       // Calculate total player stock value
        for (j = 0; j < NUMCO; j++)
            if (co[j].size)
                pl[i].svalue += co[j].price * pl[i].holdings[j];
    }

    qsort(order, numplayers, sizeof(int), order_compare);       // Sort

    for (i = 0; i < numplayers; i++) {
        wattron(stand, A_BOLD);
        mvwprintw(stand, 4 + i, 3, "%-19s   ", pl[order[i]].name);
        wattroff(stand, A_BOLD);

        wprintw(stand, "$%-8d   $%-8d  ", pl[order[i]].svalue,
                pl[order[i]].cash);

        wattron(stand, A_BOLD);
        wprintw(stand, "$%-8d", pl[order[i]].cash + pl[order[i]].svalue);
        wattroff(stand, A_BOLD);
    }

    for (i = sum = 0; i < NUMCO; i++)   // Get company size
        sum += co[i].size;

    togo = (MAPX * MAPY * END_PERCENT / 100) - sum + 1;

    sprintf(s, "Available company sectors remaining: %d", togo);
    center(stand, 60, 5 + numplayers, s);
    center(stand, 60, 7 + numplayers, "Press any key to continue...");
    wnoutrefresh(stand);
    doupdate();
    getch();
    delwin(stand);
    touchwin(mapwin);
    touchwin(general);
    touchwin(coinfo);
    wnoutrefresh(mapwin);
    wnoutrefresh(general);
    wnoutrefresh(coinfo);
}

/**
 * For sorting for show_standings().
 */
int order_compare(const void *v1, const void *v2)
{
    int nw1, nw2, p1, p2;

    p1 = *((int *) v1);
    p2 = *((int *) v2);

    nw1 = pl[p1].svalue + pl[p1].cash;
    nw2 = pl[p2].svalue + pl[p2].cash;

    if (nw1 < nw2)
        return 1;
    if (nw1 > nw2)
        return -1;
    return 0;
}

/**
 * Asks the users if they're sure they want to quit.
 */
void quit_yn(void)
{
    WINDOW *yn;

    if ((yn = newwin(5, 42, 5, (COLUMNS - 42) / 2)) == NULL) {
        fprintf(stderr,
                "starlanes: couldn't open window for y/n prompt\n");
        exit(1);
    }

    wattron(yn, QUIT_BORDER);
    box(yn, '|', '=');
    wattroff(yn, QUIT_BORDER);
    wattron(yn, QUIT_TITLE);
    center(yn, 42, 0, " Really Quit? ");
    wattroff(yn, QUIT_TITLE);
    wattron(yn, QUIT_TEXT);
    mvwaddstr(yn, 2, 2, "Are you sure you want to quit (y/n)? ");
    wattroff(yn, QUIT_TEXT);
    wnoutrefresh(yn);
    doupdate();
    if (toupper(getch()) == 'Y') {
        delwin(yn);
        shutdown();
        exit(2);
    }
    delwin(yn);
    touchwin(mapwin);
    touchwin(coinfo);
    wnoutrefresh(mapwin);
    wnoutrefresh(coinfo);
}

/**
 * Cleans up.
 */
void shutdown(void)
{
    show_standings(" GAME OVER - Final Standings ");
    delwin(mapwin);
    delwin(coinfo);
    delwin(general);
    noraw();
    echo();
    clear();
    refresh();
    endwin();
    printf("Thank you for playing Star Lanes!\n");
}

/**
 * Prints a stderr usage message.
 */
void usage(void)
{
    fprintf(stderr, "usage: starlanesb [-v|c|m]\n");
    exit(1);
}

/**
 * Main.
 */
int main(int argc, char *argv[])
{
    int done = 0, move, colorforce = 0, monoforce = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v") || !strncmp(argv[i], "--v", 3)) {
            fprintf(stderr, "Starlanes for ncurses v%s\n", VERSION);
            return 0;
        }

        else if (!strcmp(argv[i], "-m") || !strncmp(argv[i], "--m", 3)) {
            monoforce = 1;
        }

        else if (!strcmp(argv[i], "-c") || !strncmp(argv[i], "--c", 3)) {
            colorforce = 1;
        }

        else usage();
    }

    initscr();
    start_color();

    if (colorforce)
        color = 1;
    else if (monoforce)
        color = 0;
    else
        color = has_colors();

    if (color)
        color_setup();

    raw();

    // Init map, stocks
    srand(time(NULL));
    initialize();

    // Num players
    get_num_players();

    clear();
    attron(color ? (YELLOW_ON_BLUE | A_BOLD) : A_REVERSE);
    mvprintw(0, 0, " StarLanes ");
    attroff(color ? (YELLOW_ON_BLUE | A_BOLD) : A_REVERSE);
    attron(color ? BLUE_ON_BLACK : A_NORMAL);
    printw
        ("=====================================================================");
    attroff(color ? BLUE_ON_BLACK : A_NORMAL);
    wnoutrefresh(stdscr);
    showmap();
    show_coinfo();

    do {
        move = get_move();
        do_move(move);
        holding_bonus();
        if ((done = check_endgame()) != 1) {
            buy_sell();
            turn = (turn + 1) % numplayers;
        }
    } while (!done);

    shutdown();

    return 0;
}

