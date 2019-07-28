/*\
 *
 *	ArenaServer.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Single shard-wide server to keep track of global list of 
 *  arena events.  Uses db to serialize out this list to sql.
 *
 */

#ifndef ARENASERVER_H
#define ARENASERVER_H

#include "comm_backend.h"

// fields set by ArenaCommandStart
extern U32 g_request_dbid;
extern U32 g_request_user;
extern U32 g_request_eventid;
extern U32 g_request_uniqueid;
extern char g_request_name[32];
extern int g_arenaVerbose;

void ArenaCommandStart(Packet* pak);
void ArenaCommandResult(NetLink* link, ArenaServerResult res, U32 num, char* err);
void UpdateArenaTitle(void);
char* ArenaLocalize(char* string); // returns static buffer

#endif // ARENASERVER_H