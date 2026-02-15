#ifndef ANNOUNCE_H
#define ANNOUNCE_H

void new_co_announce(int newc);
void suck_announce(int conum, int grown);
void merge_announce(int c1, int c2);
void xaction_announce(int c1, int c2);
void split_announce(int conum);

#endif
