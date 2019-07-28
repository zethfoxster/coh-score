#include "mapHistory.h"
#include "entity.h"
#include "entPlayer.h"
#include "MemoryPool.h"
#include "earray.h"
#include "arenamap.h"
#include "raidmapserver.h"
#include "storyarcinterface.h"
#include "dbcomm.h"
#include "mathutil.h"

MP_DEFINE(MapHistory);

MapHistory *mapHistoryCreate() 
{
	MP_CREATE(MapHistory, 100);
	return MP_ALLOC(MapHistory);
}
void mapHistoryDestroy(MapHistory *map) 
{
	MP_FREE(MapHistory, map);
}

serverMapType getMapType() 
{
	if (OnMissionMap())
		return MAPTYPE_MISSION;
	if (OnArenaMap())
		return MAPTYPE_ARENA;
	if (OnSGBase())
		return MAPTYPE_BASE;
	return MAPTYPE_STATIC;
}

int getMapID() {
	int mapType = getMapType();
	switch (mapType)
	{	
	xcase MAPTYPE_ARENA:
		return g_arenamap_eventid;
	xcase MAPTYPE_BASE:
		return db_state.map_supergroupid;
	xdefault:
		return db_state.map_id;
	}
}


// Clear out map history of this player
void mapHistoryClear(EntPlayer *pl) 
{
	if (verify(pl && pl->mapHistory))
	{	
		eaClearEx(&pl->mapHistory,mapHistoryDestroy);
		eaClear(&pl->mapHistory);
	}
}
// Return top of history stack
MapHistory *mapHistoryLast(EntPlayer *pl) 
{
	if (verify(pl && pl->mapHistory) && eaSize(&pl->mapHistory) > 0)
	{
		return pl->mapHistory[eaSize(&pl->mapHistory)-1];
	}
	return NULL;
}
// Add to top of history stack. Just updates it if already there
void mapHistoryPush(EntPlayer *pl, MapHistory *map)
{
	if (verify(pl && pl->mapHistory))
	{
		if (eaSize(&pl->mapHistory) > 0)
		{		
			MapHistory *top = pl->mapHistory[eaSize(&pl->mapHistory)-1];	
			if (top->mapType == map->mapType && top->mapID == map->mapID) {
				copyVec3(map->last_pos,top->last_pos);
				copyVec3(map->last_pyr,top->last_pyr);
				mapHistoryDestroy(map);
				return;
			}
		}
		eaPush(&pl->mapHistory, map);
	}
}
// Remove and return top of history stack. Caller responsible for destroying it
MapHistory *mapHistoryPop(EntPlayer *pl)
{	
	if (verify(pl && pl->mapHistory) && eaSize(&pl->mapHistory) > 0)
	{
		return eaPop(&pl->mapHistory);
	}
	return NULL;
}