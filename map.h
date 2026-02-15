#ifndef MAP_H
#define MAP_H

void showmap(void);
void drawmap(int loc, char c);

// Macros to look at surrounding spaces on the map

#define up_obj(move) (((move)-MAPX < 0)?SPACE:map[(move)-MAPX])
#define down_obj(move) (((move)+MAPX >= MAPX*MAPY)?SPACE:map[(move)+MAPX])
#define left_obj(move) (((move)%MAPX)?map[(move)-1]:SPACE)
#define right_obj(move) (((move)%MAPX == MAPX-1)?SPACE:map[(move)+1])
#define iscompany(c) ((c)>='A'&&(c)<='Z')
#define ripe(c) ((c)==STAR||(c)==NEWCO)
#define co_near(move) (iscompany(up_obj(move))||iscompany(down_obj(move))||iscompany(left_obj(move))||iscompany(right_obj(move)))
#define s_or_bh(c) ((c)==SPACE||(c)==BLACKHOLE)

#endif
