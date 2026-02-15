#include "consts.h"
#include "globals.h"
#include "player.h"

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

