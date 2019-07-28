#ifndef DBEVENTHISTORY_H_
#define DBEVENTHISTORY_H_

typedef struct KarmaEventHistory KarmaEventHistory;
typedef struct DbContainer DbContainer;

void dbEventHistory_Init(void);
void dbEventHistory_StatusCb(DbContainer *container, char *buf);
void dbEventHistory_UpdateCb(DbContainer *container);

void handleEventHistoryFind(Packet *pak, NetLink *link);
KarmaEventHistory **dbEventHistory_Find(U32 min_time, U32 max_time, char *auth, char *player);

#endif
