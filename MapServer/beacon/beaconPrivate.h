
#ifndef BEACONPRIVATE_H
#define BEACONPRIVATE_H

#include "stdtypes.h"
#include <float.h>
#include "beacon.h"
#include "assert.h"
#include "error.h"
#include "beaconDebug.h"
#include "entityRef.h"
#include "StashTable.h"

#define BEACON_GALAXY_GROUP_JUMP_INCREMENT	5
#define BEACON_GALAXY_GROUP_COUNT			20

typedef struct NavPathWaypoint	NavPathWaypoint;
typedef struct BeaconBlock		BeaconBlock;
typedef struct BeaconAvoidNode	BeaconAvoidNode;

typedef struct BeaconCurvePoint {
	Vec3				pos;
	Vec3				pyr;
} BeaconCurvePoint;

typedef struct BeaconConnection {
	Beacon*				destBeacon;
	F32					minHeight;
	F32					maxHeight;
	U32					timeWhenIWasBad;
	S32					searchDistance;
} BeaconConnection;

typedef struct BeaconBlock {
	union{
		BeaconBlock*	parentBlock;
		BeaconBlock*	cluster;
	};
	
	BeaconBlock*		galaxy;
	Vec3				pos;
	F32					searchY;
	Array				beaconArray;
	Array				subBlockArray;
	Array				gbbConns;						// ground BeaconBlockConnections
	Array				rbbConns;						// raised BeaconBlockConnections
	U32					searchInstance;
	void*				astarInfo;
	U32					isGridBlock				: 1;
	U32					isGalaxy				: 1;
	U32					isCluster				: 1;
	U32					madeConnections			: 1;
	U32					isGoodForSearch			: 1;
	U32					pathLimitedAllowed		: 1;	// usable by path limited entities
} BeaconBlock;

typedef struct BeaconBlockConnection {
	BeaconBlock*		destBlock;
	F32					minJumpHeight;
	F32					reverseMinJumpHeight;
	F32					minHeight;
	F32					maxHeight;
} BeaconBlockConnection;

typedef struct BeaconClusterConnection {
	BeaconBlock*				targetCluster;
	BeaconDynamicConnection*	dynConnToTarget;

	struct {
		Vec3			pos;
		Beacon*			beacon;
		EntityRef		entity;
	} source;

	struct {
		Vec3			pos;
		Beacon*			beacon;
		EntityRef		entity;
	} target;
} BeaconClusterConnection;

typedef struct BeaconAvoidNode {
	struct {
		BeaconAvoidNode*	next;
		BeaconAvoidNode*	prev;
	} beaconList;
	
	struct {
		BeaconAvoidNode*	next;
		//BeaconAvoidNode*	prev;
	} entityList;
	
	Beacon*					beacon;
	Entity*					entity;
} BeaconAvoidNode;

extern StashTable combatBeaconGridBlockTable;
extern Array combatBeaconGridBlockArray;
extern Array combatBeaconGalaxyArray[BEACON_GALAXY_GROUP_COUNT];
extern Array combatBeaconClusterArray;

//----------------------------------------------------------------
// beaconFile.c
//----------------------------------------------------------------

typedef struct BeaconProcessInfo	BeaconProcessInfo;
typedef struct BeaconDiskSwapBlock	BeaconDiskSwapBlock;

typedef struct BeaconProcessState {
	const char* 				titleMapName;
	
	char*						beaconFileName;
	char*						beaconDateFileName;

	U32							beaconFileTime;

	U32							latestDataFileTime;
	char*						latestDataFileName;

	U32							fullMapCRC;

	Vec3						world_min_xyz;
	Vec3						world_max_xyz;

	int							groupDefCount;

	BeaconProcessInfo*			infoArray;
	int*						processOrder;
	Entity*						entity;
	int							memoryAllocated;
	int							legalCount;
	BeaconDiskSwapBlock*		curDiskSwapBlock;
	int							nextDiskSwapBlockIndex;
	int							validStartingPointMaxLevel;
} BeaconProcessState;

extern BeaconProcessState beacon_process;

F32 beaconSnapPosToGround(Vec3 posInOut);

Beacon* addCombatBeacon(const Vec3 beaconPos,
						S32 silent,
						S32 snapToGround,
						S32 destroyIfTooHigh);
						
void beaconProcessSetTitle(F32 progress, const char* sectionName);
void beaconProcessSetConsoleCtrlHandler(int on);
void beaconCreateFilenames();
S32 beaconFileIsUpToDate(S32 noFileCheck);

//----------------------------------------------------------------
// beaconConnection.c
//----------------------------------------------------------------

typedef struct BeaconDynamicInfo BeaconDynamicInfo;

typedef struct BeaconDynamicConnection {
	BeaconDynamicConnection*	prev;
	BeaconDynamicConnection*	next;
	
	BeaconDynamicInfo**			infos;

	Beacon*						source;
	BeaconConnection*			conn;
	S32							raised;
	
	S32							blockedCount;
	S32							doorBlockedCount;
} BeaconDynamicConnection;

typedef struct BeaconDynamicInfo {
	Entity*						entity;
	S32							connsEnabled;
	S32							isDoor;
	BeaconDynamicConnection**	conns;
} BeaconDynamicInfo;

void beaconFreeAllMemory(void);
void* beaconGetMemory(U32 size);
void beaconInitArray(Array* array, U32 count);
void beaconInitCopyArray(Array* array, Array* source);

void beaconBlockInitArray(Array* array, U32 count);

int createCurve(Position* source, Position* target, BeaconCurvePoint* output, int pointCount, int makeLastPoint);
int beaconNPCClusterTraverser(Beacon* beacon, int* curSize, int maxSize);


//----------------------------------------------------------------
// beacon.c
//----------------------------------------------------------------

void beaconClearBeaconData(int clearNPC, int clearTraffic, int clearCombat);

Beacon* createBeacon(void);
void destroyCombatBeacon(Beacon* beacon);
void destroyNonCombatBeacon(Beacon* beacon);

F32 beaconGetDistanceYToCollision(const Vec3 pos, F32 dy);
F32 beaconGetPointFloorDistance(const Vec3 posParam);
F32 beaconGetFloorDistance(Beacon* b);
F32 beaconGetPointCeilingDistance(const Vec3 posParam);
F32 beaconGetCeilingDistance(Beacon* b);

BeaconAvoidNode* beaconAddAvoidNode(Beacon* beacon, Entity* e);
void beaconDestroyAvoidNode(BeaconAvoidNode* node);

#endif
