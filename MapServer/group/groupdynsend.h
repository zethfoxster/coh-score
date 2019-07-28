#ifndef _GROUPDYNSEND_H
#define _GROUPDYNSEND_H

typedef struct Packet Packet;

int groupDynGetHp(int world_id);
void groupDynSetHpAndRepair(int world_id,int hp,int repair);
void groupDynResetSends();
int groupDynSend(Packet *pak,int full_update);
int groupDynFind(const char *name,const Vec3 pos,F32 dist);
void groupDynForceRescan();
void groupDynFinishUpdate();

#endif
