#ifndef _BASETRIM_H
#define _BASETRIM_H

typedef struct BaseRoom BaseRoom;

void roomAddBlockTrim(BaseRoom *room,int x,int z,F32 lower,F32 upper,int outer_edge);
void roomSetDetailBlocks(BaseRoom *room);
void blockConnectTrim(BaseRoom *room,int x,int z);
void blockConnectTrimCaps(BaseRoom *room,int x,int z);

#endif
