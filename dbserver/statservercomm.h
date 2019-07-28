/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STATSERVERCOMM_H
#define STATSERVERCOMM_H

#include "stdtypes.h"
#include "container.h"

typedef struct Packet Packet;
typedef struct NetLink	NetLink;

void statCommInit(void);
int statServerCount(void);
int statServerSecondsSinceUpdate(void);
NetLink *statLink();

void handleSgrpStatAdj(Packet* pak,NetLink* link);
void handleMiningDataRelay(Packet* pak);

typedef struct StatCon
{
	ServerCon serverCon;
	bool starting;
	int		launcher_id; // id of the launcher who was assigned to launch this map
	int		cookie; // The last cookie handed off to a launcher for starting this specific map
	U32		lastRecvTime; // Last time we received a who update (i.e. the mapserver is in it's main loop)
} StatCon;
extern StatCon g_stat_con;

#endif //STATSERVERCOMM_H
