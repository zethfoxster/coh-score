#ifndef _MAPGROUP_H
#define _MAPGROUP_H

#include "dbcontainer.h"

// do not persist these tokens!
typedef struct MapDataToken
{
	char	name[64];
	int		val;   //arbitary value associated with this string
} MapDataToken;

typedef struct
{
	char			name[64];
	char			activeEvent[64];		// if set, there is an active shard event
	char			eventSignal[64];		// signal to send to shard event
	U32				eventSignalTS;			// secondsSince2000, when signal was sent
	MapDataToken	**data;					// list of global data for this map/group, do not persist this earray or its contents!
} MapGroup;

extern MapGroup	g_mapgroup;

char *mapGroupTemplate();
char *mapGroupSchema();
void containerHandleMapGroup(ContainerInfo *ci);
void mapGroupLock();
void mapGroupUpdateUnlock();
void mapGroupInit(char *unique_name);

MapDataToken * getMapDataToken(const char * pName); // do not persist the token!
MapDataToken * giveMapDataToken(const char *pName); // do not persist the token!
void removeMapDataToken(const char *pName);

int mapGroupIsInitalized();

#endif
