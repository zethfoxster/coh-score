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

extern int g_arenaVerbose;

void UpdateStatTitle(void);

extern StashTable g_sgrpFromId;

#define LOGID "statserver"

// *********************************************************************************
// debugging
// *********************************************************************************

typedef int (*statDebug_fpDbgPrintf)(const char *format, ...);
void statDebugHandleString(char *inputStr, statDebug_fpDbgPrintf fpDbgPrintf);
char* statserver_UpdateBaseUpkeep(int idSgrp);
Supergroup *stat_sgrpFromIdSgrp(int idSgrp, bool loadSync);
Supergroup *stat_sgrpFromName(char *name, int *pResIdSgrp, bool loadSync);



#endif // ARENASERVER_H
