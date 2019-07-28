#ifndef EVENTHISTORY_H_
#define EVENTHISTORY_H_

typedef struct Entity Entity;
typedef struct KarmaEventHistory KarmaEventHistory;

KarmaEventHistory *eventHistory_CreateFromEnt(Entity *ent);
void eventHistory_SendToDB(KarmaEventHistory *event);
void eventHistory_FindOnDB(U32 sender_dbid, U32 min_time, U32 max_time, char *auth, char *player);
void eventHistory_ReceiveResults(Packet *pak);
#endif
