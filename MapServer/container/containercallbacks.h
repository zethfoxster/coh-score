#ifndef _CONTAINERCALLBACKS_H
#define _CONTAINERCALLBACKS_H

#include "dbcontainer.h"


void containerSetDbCallbacks(void);
void dealWithLostMembership(int list_id, int container_id, int ent_id);
void SendReflectionToEnts(int* ids, int deleting, ContainerReflectInfo** reflectinfo, void (*func)(int*, int, Entity*));
bool isBadEntity(int db_id);

#endif
