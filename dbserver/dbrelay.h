#ifndef DBRELAY_H
#define DBRELAY_H

#include "netio.h"
#include "dbinit.h"

void handleSendToEnt(DBClientLink *client, Packet *pak_in);
void sendToEnt(DBClientLink *client, int ent_id, int force, char *msg);
void handleSendToGroupMembers(DBClientLink *client, Packet *pak_in);
void handleSendToAllServers(Packet *pak_in);
void dbRelayQueueCheck(void);
void dbRelayQueueRemove(int ent_id);
void sendToServers(int server_id, char *msg, int static_only);

extern int log_relay_verbose;

#endif