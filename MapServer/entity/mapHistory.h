#ifndef MAP_HISTORY_H
#define MAP_HISTORY_H

#include "stdtypes.h"

typedef struct EntPlayer EntPlayer;
typedef struct Entity Entity;

// The stack of maps a player has been to since the last static map

typedef enum serverMapType {
	MAPTYPE_STATIC, //Not a valid type for a history for now
	MAPTYPE_MISSION,
	MAPTYPE_ARENA,
	MAPTYPE_BASE,
} serverMapType;

typedef struct MapHistory {
	serverMapType mapType;
	int mapID;
	Vec3 last_pos;
	Vec3 last_pyr;
} MapHistory;

MapHistory *mapHistoryCreate();
void mapHistoryDestroy(MapHistory *map);

// Return the maptype corresponding to this map server
serverMapType getMapType();
// Return the mapID for this type of maptype
int getMapID();
// Clear out map history of this player
void mapHistoryClear(EntPlayer *pl);
// Return top of history stack
MapHistory *mapHistoryLast(EntPlayer *pl);
// Add to top of history stack
void mapHistoryPush(EntPlayer *pl, MapHistory *map);
// Remove and return top of history stack. Caller responsible for destroying it
MapHistory *mapHistoryPop(EntPlayer *pl);

#endif
