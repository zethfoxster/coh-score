#ifndef _GROUPNETDB_H
#define _GROUPNETDB_H

typedef struct DefTracker DefTracker;
typedef struct Packet Packet;
typedef struct NetLink NetLink;

void sendEditMessage(NetLink *link, int is_error, const char *fmt, ...);

DefTracker* pktGetTrackerHandleUnpacked(Packet *pak);

int worldModified(void);
void groupLoadMap(char *s,int import,int do_beacon_reload);
void worldHandleMsg(NetLink *link,Packet *pak);
void groupdbRunBatch(char *batch_str);
void consoleSave(NetLink *,int);



#endif
