#include <stdlib.h>
#include "conf.h"
#include CURSES_HEADER
#include "announce.h"
#include "consts.h"
#include "globals.h"
#include "map.h"

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

