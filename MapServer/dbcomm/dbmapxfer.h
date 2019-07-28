#ifndef _DBMAPDOOR_H
#define _DBMAPDOOR_H

#include "account/AccountTypes.h"

typedef struct Entity Entity;

typedef int MapTransferType; 
// these should be exclusive
#define XFER_STATIC			(1 << 0)
#define XFER_MISSION		(1 << 1)
#define XFER_FROMINTRO		(1 << 2)
#define XFER_ARENA			(1 << 3)
#define XFER_BASE			(1 << 4)
#define XFER_GURNEY			(1 << 5)
#define XFER_SHARD			(1 << 6)
// optionally add the following
#define XFER_FINDBESTMAP	(1 << 31)

int dbCheckMapXfer(Entity *e,NetLink *client_link); // returns whether transfer was cancelled
void dbAsyncMapMove(Entity *e,int map_id,char* mapinfo,MapTransferType xfer);
void dbAsyncShardJump(Entity *e, const char *dest);
void dbShardXferInit(Entity *e, OrderId order_id, int type, int homeShard, char *shardVisitorData);
void dbShardXferComplete(Entity *e, int cookie, int dbid, int addr, int port);
void dbShardXferJump(Entity *e);
void handleDbNewMapLoading(Packet *pak);
void handleDbNewMapReady(Packet *pak);
void handleDbNewMap(Packet *pak);
void handleDbNewMapFail(Packet *pak);
int dbUndoMapXfer(Entity *e, char *errmsg);
void dbUseTwoStageMapXfer(bool bTwoStage);

#endif
