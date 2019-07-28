#ifndef STATICMAPINFO_H
#define STATICMAPINFO_H

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef struct
{
	int min;
	int max;
} Range;

typedef enum OverridesMap
{
	kOverridesMapFor_Nobody = 0,		// Default, means no overriding happens.
	kOverridesMapFor_PrimalEarth,		// Primal born and ex-Praetorians
	kOverridesMapFor_Count
} OverridesMap;

// StaticMapInfo is recorded in maps.db and is per-instance info
// it's only valid for static maps
typedef struct StaticMapInfo
{
	int mapID;
	char* name;
	int baseMapID;
	int instanceID;

	// Some static maps (trial zones) have hospital zones that you are only allowed to use if you
	// die while on that map.
	int		dontTrackGurneys;

	// Static maps are auto-started by the dbserver when it boots up (in production) this flag
	// is for static test maps that don't need to be running.
	int		dontAutoStart;

	// Transient maps are static maps that are allowed to shut down when the idle exit timeout
	// is reached.
	int		transient;

	// If nonzero, puts the mapserver into remote map editing mode and allows players with the
	// specified access level to edit the map, as well as have their commands executed as if they
	// were remoteEditLevel
	int		remoteEdit;
	int		remoteEditLevel;

	// Zone to introduce new players to the game
	//    1=hero tutorial
	//    2=villain tutorial
	//    3=praetorian tutorial
	//    4=neutral primal tutorial
	int		introZone;

	// This map doesn't let you see anything on the automap until you explore it
	int		opaqueFogOfWar;
	
	// Max safe player count for this map.  Low is when to spawn clones and not offer entrance.  High is when to deny entrance.
	int		safePlayersLow;
	int		safePlayersHigh;

	// Don't let more of this type in if its full
	int		maxHeroCount;
	int		maxVillainCount;

	// Number of players on this map, according to the latest info from db server.
	int		playerCount;
	int		heroCount;
	int		villainCount;
	
	// Bitflags for which monorail lines this map is on.
	int		monorailLine;

	// Some numbers to override normal spawn behavior
	int		minCritters;
	int		maxCritters;

	// Keys that can be used in the servers.cfg to block access to this map
	char**	mapKeys;

	int		chatRadius;

	// Characters trying to travel to the map listed below get redirected to
	// this map if they are in the specified category/condition.
	int		overridesMapID;
	int		overridesMapFor;

	// For each type of override, record which map this map is overridden by.
	// This gets filled out after maps.db has been read in - it makes it easier
	// to decide whether this map has been overridden for destination.
	int		overriddenBy[kOverridesMapFor_Count];

	// Is this Ouroboros for any group?
	int		ouroboros;
} StaticMapInfo;

typedef struct ClientLink ClientLink;

void staticMapInfosLoad();
StaticMapInfo* staticMapInfoFind(int mapID);
StaticMapInfo* staticMapGetBaseInfo(int mapID); // as above, but automatically gets the base version
int staticMapInfoGetClones(int baseMapID, StaticMapInfo** infos);
int staticMapInfoMatchNames( char *name );

int staticMapAnyoneCanEnter(StaticMapInfo* info);
void staticMapPrint(ClientLink* client);
void staticMapInfoResetPlayerCount();
int staticMapInfoGetHighestBaseID();

// Specifies what missions are allowed inside a map
typedef enum MissionsAllowed
{
	MA_NONE,			// Don't allow any missions here
	MA_ALL,				// Allow both hero and villain missions
	MA_HERO,			// Allow hero missions only
	MA_VILLAIN,			// Allow villain missions only
} MissionsAllowed;

#if SERVER
int staticMapInfo_IsZoneAllowedToHaveMission();
#endif 

// If none are flagged, all primal earth players are allowed to enter. Praetorians
// can only get into maps which explicitly allow them. In such cases primal earth
// entry must also be explicitly permitted.
#define MAP_ALLOW_HEROES			(1 << 0)
#define MAP_ALLOW_VILLAINS			(1 << 1)
#define MAP_ALLOW_PRIMAL			(1 << 2)
#define MAP_ALLOW_PRAETORIANS		(1 << 3)

#define MAP_ALLOWS_HEROES(spec)			((!spec->playersAllowed) || (spec)->playersAllowed & MAP_ALLOW_HEROES)
#define MAP_ALLOWS_VILLAINS(spec)		((!spec->playersAllowed) || (spec)->playersAllowed & MAP_ALLOW_VILLAINS)
#define MAP_ALLOWS_HEROES_ONLY(spec)	( ((spec)->playersAllowed & MAP_ALLOW_HEROES) && !((spec)->playersAllowed & MAP_ALLOW_VILLAINS) )
#define MAP_ALLOWS_VILLAINS_ONLY(spec)	( !((spec)->playersAllowed & MAP_ALLOW_HEROES) && ((spec)->playersAllowed & MAP_ALLOW_VILLAINS) )

#define MAP_ALLOWS_PRAETORIANS(spec)		((spec)->playersAllowed & MAP_ALLOW_PRAETORIANS)
#define MAP_ALLOWS_PRAETORIANS_ONLY(spec)	(((spec)->playersAllowed & MAP_ALLOW_PRAETORIANS) && !(spec->playersAllowed & MAP_ALLOW_PRIMAL))
#define MAP_ALLOWS_PRIMAL(spec)			(!((spec)->playersAllowed & MAP_ALLOW_PRAETORIANS) || (spec->playersAllowed & MAP_ALLOW_PRIMAL))
#define MAP_ALLOWS_PRIMAL_ONLY(spec)	(!((spec)->playersAllowed & MAP_ALLOW_PRAETORIANS))

#define MAP_ALLOWS_DEV_ONLY(spec)		((spec)->devOnly)

// MapSpec is in specs/maps.spec and is per-mapfile
// it can be used to govern instanced missions as well as static maps
typedef struct MapSpec
{
	const char* mapfile;

	Range entryLevelRange;		// levels requirements to enter map - 1 based
	Range missionLevelRange;	// levels to get a mission assigned on this map - 1 based
	Range spawnOverrideLevelRange;	// levels for task spawn overrides on this map - 1 based
	MissionsAllowed missionsAllowed;// Which type of missions are allowed on this map

	const char** missionDoorTypes;	// these door types exist on this map
	const char** persistentNPCs;		// these persistent NPCs exist on this map
	const char** visitLocations;		// these visit locations exist on this map
	const char** villainGroups;		// these villain groups have normal spawndefs on this map
	const char** encounterLayouts;	// these encounter layouts are available on this map
	const char** zoneScripts;			// zone scripts to run at startup
	const char *globalAIBehaviors;	// ai behavior to add to all zone spawns
	float NPCBeaconRadius;		// radius around a nav beacon that NPCs can pick as their destination

	unsigned int disableBaseHospital;// If flagged, players may not ressurect in their base on this map, or any mission from this map
	unsigned int playersAllowed;// Which type of players are allowed on this map

	unsigned int devOnly;		// Must be access level 1 or higher to be in this map


	unsigned int teamArea;			// Hero-only, common primal, etc - derived from playersAllowed

	const char **monorailRequires;	// Monorail map access requires the following
	const char **monorailVisibleRequires;	// Monorail visibility requires the following
	const char **loadScreenPages;		// List of images to display using the "Comic Book Loading Screen" tech
} MapSpec;

typedef struct MapSpecList
{
	const MapSpec** mapSpecs;
	cStashTable mapSpecHash;
} MapSpecList;

extern SHARED_MEMORY MapSpecList mapSpecList;

typedef struct MonorailStop
{
	char*				name;		// Internal name
	char*				desc;		// Get translated and displayed
	int					mapID;		// Base map this stop is found on
	char*				spawnName;	// Where to spawn when sent to this stop
	StaticMapInfo**		mapInfos;	// All maps that are destinations for this stop
} MonorailStop;

typedef struct MonorailLine
{
	char*				name;		// Used to find the line by doors
	char*				title;		// Name shown in header
	char*				logo;		// Background image for choice window
	MonorailStop**		stops;		// List of stops, in order, that make up this line
	char**				allowedZoneTransferType;
} MonorailLine;

void MapSpecReload(void);
const MapSpec* MapSpecFromMapId(int mapID);
const MapSpec* MapSpecFromMapName(const char* mapfile);
int MapSpecCanHaveMission(const MapSpec* spec, int level);
int MapSpecCanOverrideSpawn(const MapSpec* spec, int level);
const char** MapSpecDoorList(const char* mapfile);
const char** MapSpecNPCList(const char* mapfile);
const char** MapSpecVisitLocations(const char* mapfile);
const char** MapSpecVillainGroups(const char* mapfile);
const char** MapSpecEncounterLayouts(const char* mapfile);

char * getTranslatedMapName( int map_id );

int MapSpecAreaCanReachArea( U8 src, U8 dst );

void setMapInfoFlags(void);
extern bool g_MapIsTutorial;
extern bool g_MapIsMissionMap;

void MonorailsLoad(void);
MonorailLine* MonorailGetLine(char* linename);

#endif
