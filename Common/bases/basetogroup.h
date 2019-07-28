#ifndef _BASETOGROUP_H
#define _BASETOGROUP_H

typedef struct Base Base;
typedef struct BaseRoom BaseRoom;
typedef struct GroupEnt GroupEnt;

void baseCreate(char *style,int random);
void baseRescanTest();
void baseToDefsWithOffset(Base *base, int suffix, Vec3 offset);
void baseToDefs(Base *base, int suffix);
GroupEnt *roomAddPartYaw(BaseRoom *room,char *partname,char *subname,F32 x,F32 y,F32 z,F32 yaw,F32 sideways,F32 forward);

#endif
