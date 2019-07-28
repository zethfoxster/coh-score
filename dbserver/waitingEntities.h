#ifndef WAITINGENTITIES_H
#define WAITINGENTITIES_H

#include "stdtypes.h"


// Handles a list of entities waiting for their maps to start and for them to be unlocked

typedef struct EntCon EntCon;
typedef struct MapCon MapCon;

int waitingEntitiesRemove(EntCon *ent_con); // Return 1 on unload
void waitingEntitiesAddEnt(EntCon *ent_con, U32 dest_map_id, int is_map_xfer, MapCon *destMap);
void waitingEntitiesCheck(void);

void waitingEntitiesDumpStats(void);

int waitingEntitiesIsIDWaiting(int id);

// Callbacks in dbdispatch.c
void doneWaitingMapXferFailed(EntCon *ent_con, char *errmsg);
void doneWaitingMapXferSucceeded(EntCon *ent_con, int new_map_id);

// Callbacks in clientcomm.c
void doneWaitingChoosePlayerFailed(EntCon *ent_con, char *errmsg);
void doneWaitingChoosePlayerSucceded(EntCon *ent_con);

// stats
U32 waitingEntitiesGetPeakSize(bool bReset);



#endif