
#ifndef BEACONPATH_H
#define BEACONPATH_H

#include "stdtypes.h"

typedef struct AStarSearchData	AStarSearchData;
typedef struct Beacon			Beacon;
typedef struct BeaconBlock		BeaconBlock;
typedef struct BeaconConnection	BeaconConnection;
typedef struct DoorEntry		DoorEntry;
typedef struct Entity			Entity;
typedef struct Position			Position;

typedef enum NavPathConnectType {
	NAVPATH_CONNECT_GROUND,
	NAVPATH_CONNECT_JUMP,
	NAVPATH_CONNECT_FLY,
	NAVPATH_CONNECT_WIRE,
	NAVPATH_CONNECT_ENTERABLE,
	
	NAVPATH_CONNECT_COUNT,
} NavPathConnectType;

typedef struct NavPathWaypoint {
	struct NavPathWaypoint*		next;
	struct NavPathWaypoint*		prev;

	Vec3						pos;
	Beacon*						beacon;
	BeaconConnection*			connectionToMe;
	DoorEntry*					door;
	
	NavPathConnectType			connectType;
} NavPathWaypoint;

typedef struct NavPath {
	NavPathWaypoint*			head;
	NavPathWaypoint*			tail;
	NavPathWaypoint*			curWaypoint;
	int							waypointCount;
} NavPath;

typedef struct NavSearch {
	Beacon*						src;
	Beacon*						dst;
	NavPath						path;
	float						destinationHeight;
	unsigned int				searching		: 1;
} NavSearch;

typedef enum NavSearchResultType {
	NAV_RESULT_BAD_POSITIONS,
	NAV_RESULT_NO_SOURCE_BEACON,
	NAV_RESULT_NO_TARGET_BEACON,
	NAV_RESULT_BLOCK_ERROR,
	NAV_RESULT_NO_BLOCK_PATH,
	NAV_RESULT_NO_BEACON_PATH,
	NAV_RESULT_CLUSTER_CONN_BLOCKED,
	NAV_RESULT_SUCCESS,
	NAV_RESULT_PARTIAL,		// pathfind was successful but did not make a path to the follow target
} NavSearchResultType;

typedef struct NavSearchResults {
	NavSearchResultType			resultType;
	Vec3						sourceBeaconPos;
	Vec3						targetBeaconPos;
	char*						resultMsg;
	U32							losFailed : 1;
} NavSearchResults;

//-------------------------------------------------------------------------------------------------

void beaconSetPathFindEntity(Entity* e, float maxJumpHeight);

void beaconSetDebugClient(Entity* e);

//-------------------------------------------------------------------------------------------------

NavPathWaypoint* createNavPathWaypoint(void);

void destroyNavPathWaypoint(NavPathWaypoint* waypoint);

void navPathAddTail(NavPath* path, NavPathWaypoint* wp);

void navPathAddHead(NavPath* path, NavPathWaypoint* wp);

//-------------------------------------------------------------------------------------------------

NavSearch* createNavSearch(void);

void destroyNavSearch(NavSearch* search);

void clearNavSearch(NavSearch* search);

//-------------------------------------------------------------------------------------------------

typedef int (*BeaconScoreFunction)(Beacon* src, BeaconConnection* conn);

enum{
	BEACONSCORE_FINISHED =	-1,
	BEACONSCORE_CANCEL =	-2,
};

typedef enum GCCBflags { 
	GCCB_IGNORE_LOS  			= 1<<0, // Don't even try to get line of sight (won't work with no sourceBeacon).
	GCCB_PREFER_LOS	 			= 1<<1, // Try to get line of sight on sourcePos, fall back to no line of sight (if sourceBeacon).
	GCCB_REQUIRE_LOS 			= 1<<2, // Only return beacon if it has los on sourcePos.
	GCCB_IF_ANY_LOS_ONLY_LOS	= 1<<3, // Prefer los, but if you find *any* los beacon that fails to connect to source beacon, don't try no-los beacons.
} GCCBflags;


Beacon* beaconGetClosestCombatBeacon(const Vec3 sourcePos, F32 maxLOSRayRadius, Beacon* sourceBeacon, GCCBflags losFlags, S32* losFailed);

Beacon* beaconGetClosestCombatBeaconVerifier(Vec3 sourcePos);

int beaconGalaxyPathExists(Beacon* source, Beacon* target, F32 maxJumpHeight, S32 canFly);

int beaconGetNextClusterWaypoint(BeaconBlock* sourceCluster, BeaconBlock* targetCluster, Vec3 outPos);

void beaconPathFindSimple(NavSearch* search, Beacon* src, BeaconScoreFunction funcCheckBeacon);

void beaconPathFindSimpleStart(NavSearch* search, Position* pos, BeaconScoreFunction funcCheckBeacon);

void beaconPathFind(NavSearch* search, const Vec3 sourcePos, const Vec3 targetPos, NavSearchResults* results);

void beaconPathFindRaw(NavSearch* search, AStarSearchData* searchData, Beacon* src, Beacon* dst, int steps, NavSearchResults* results);

void beaconPathInit(int forceUpdate);

void beaconPathResetTemp(void);

int beaconPathExists( Vec3 pos1, Vec3 pos2, int canFly, F32 jumpHeight, int beaconlessPositionsAreErrors );

void beaconClearBadDestinationsTable();

#endif