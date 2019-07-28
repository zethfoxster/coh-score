#ifndef MISSIONMAPCOMMON_H
#define MISSIONMAPCOMMON_H

#include "storyarcCommon.h"
#include "playerCreatedStoryarcValidate.h"
#ifndef TEST_CLIENT
#include "groupMetaMinimap.h"
#endif

// *********************************************************************************
//  Mission map sets
// *********************************************************************************

typedef enum MapSetEnum {
	MAPSET_NONE,
	MAPSET_OFFICE,
	MAPSET_ABANDONEDOFFICE,
	MAPSET_FLOODEDOFFICE,
	MAPSET_WAREHOUSE,
	MAPSET_ABANDONEDWAREHOUSE,
	MAPSET_5THCOLUMN,
	MAPSET_SEWERS,
	MAPSET_CAVES,
	MAPSET_CIRCLEOFTHORNS,
	MAPSET_TECH,
	MAPSET_TEST,
	MAPSET_SMOOTHCAVES,
	MAPSET_SHADOWSHARD_SMOOTHCAVES,
	MAPSET_COUNCIL,
	MAPSET_CARGOSHIP,
	MAPSET_VILLAINABANDONEDOFFICE,
	MAPSET_VILLAINABANDONEDWAREHOUSE,
	MAPSET_VILLAINCAVES,
	MAPSET_VILLAINCIRCLEOFTHORNS,
	MAPSET_VILLAINOFFICE,
	MAPSET_VILLAINSEWERS,
	MAPSET_VILLAINWAREHOUSE,
	MAPSET_VILLAINSMOOTHCAVES,
	MAPSET_ARACHNOS,
	MAPSET_LONGBOW,
	MAPSET_LONGBOWUNDERWATER,
	MAPSET_CARGOSHIPOUTDOOR,
	MAPSET_VILLAINARACHNOSWAREHOUSE,
	MAPSET_VILLAINLONGBOWWAREHOUSE,
	MAPSET_VILLAINARACHNOIDTECH,
	MAPSET_VILLAINARACHNOIDCAVE,
	MAPSET_VILLAINOFFICETOCAVE,
	MAPSET_VILLAINOFFICETOSEWER,
	MAPSET_VILLAINSEWERTOSEWER,
	MAPSET_VILLAINMOSTOSMOOTHCAVE,
	MAPSET_VILLAINWAREHOUSETOARACH,
	MAPSET_ABANDONEDTECH,
	MAPSET_RIKTI,
	MAPSET_5THCOLUMN_FLASHBACK,
	MAPSET_CAVES_MEDITERRANEAN,
	MAPSET_PRAETORIAN_OFFICE,
	MAPSET_PRAETORIAN_OFFICE_SOLO,
	MAPSET_PRAETORIAN_WAREHOUSE,
	MAPSET_PRAETORIAN_WAREHOUSE_SOLO,
	MAPSET_PRAETORIAN_TUNNELS,
	MAPSET_PRAETORIAN_TECH,
	MAPSET_PRAETORIAN_PPD,
	MAPSET_PRAETORIAN_TECH_BIO,
	MAPSET_PRAETORIAN_TECH_POWER,
	MAPSET_NEW_WAREHOUSE,
	MAPSET_GRANITE_CAVES,
	MAPSET_TALONS_CAVES,
	MAPSET_KALLISTI_OFFICE,
	MAPSET_RANDOM,
	MAPSET_SPECIFIC,
} MapSetEnum;

extern StaticDefineInt ParseMapSetEnum[];

typedef struct MissionMap
{
	char*	mapfile;
} MissionMap;

typedef struct MissionMapTime
{
	int				maptime;
	MissionMap**	maps;
	MapLimit		randomLimit;
} MissionMapTime;

typedef struct MissionMapSet
{
	MapSetEnum				mapset;
	const char*				pchDisplayName;
	const char**			randomDoorTypes;
	const MissionMapTime**	missionmaptimes;
} MissionMapSet;

typedef struct MissionMapList
{
	const MissionMapSet** missionmapsets;
} MissionMapList;

extern SHARED_MEMORY MissionMapList g_missionmaplist;

void MissionMapPreload();

void MissionMapSetForEach(const char* matchfile, int (*func)(MapSetEnum mapset, int time));

int MapSetAllowsRandomDoorType(MapSetEnum mapset, char* doorType);

MissionMap **MissionMapGetList(int mapset, int maptime);

// *********************************************************************************
//  Function for beaconizer to get the map list.
// *********************************************************************************
void MissionMapGetAllMapNames(char*** mapNames);



typedef struct MissionCustomEncounterStats
{
	char *type;
	int countGrouped;
	int countOnly;
} MissionCustomEncounterStats;

typedef struct MissionPlaceStats 
{
	MissionPlaceEnum place;
	int genericEncounterOnlyCount;
	int genericEncounterGroupedCount;
	int hostageLocationCount;
	int wallObjectiveCount;
	int floorObjectiveCount;
	int roofObjectiveCount;
	MissionCustomEncounterStats **customEncounters;
} MissionPlaceStats;

typedef struct MissionMapStats 
{
	char * pchMapName;

	int roomCount;
	int floorCount;
	int objectiveRoomCount;
	int placeCount;
	MissionPlaceStats **places;

} MissionMapStats;

typedef struct MissionMapStatList 
{
	MissionMapStats ** ppStat;
} MissionMapStatList;

#ifndef TEST_CLIENT
typedef struct MinimapHeaderList 
{
	ArchitectMapHeader ** ppHeader;
} MinimapHeaderList;
#endif



MissionMapStats* missionMapStats_initialize(char * pchName);
void missionMapStats_destroy(MissionMapStats *mapStats);
void missionMapStats_Save(MissionMapStats *mapStats, const char * pchFile);

MissionMapStats * missionMapStat_Get( const char * pchFile );
#ifndef TEST_CLIENT
ArchitectMapHeader * missionMapHeader_Get( const char * pchFile );
void missionMapHeader_Destroy( MinimapHeaderList * headerList );
#endif
void missionMapHeader_GetMapNames(char ***);
const MapLimit* missionMapStat_GetRandom( const char *pchCategory, int iTime );
int missionMapStat_isFileValid( char * pchFile, char *pchCategory, int iTime );
#endif