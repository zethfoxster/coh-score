#ifndef BEACON_H
#define BEACON_H

#include "mathutil.h"
#include "grouputil.h"
#include "ArrayOld.h"
#include "position.h"
#include "timing.h"
#include "StashTable.h"

typedef struct Beacon					Beacon;
typedef struct NavPath					NavPath;
typedef struct NavSearch				NavSearch;
typedef struct BeaconDynamicConnection	BeaconDynamicConnection;
typedef struct BeaconBlock				BeaconBlock;
typedef struct BeaconAvoidNode			BeaconAvoidNode;
typedef struct Entity Entity;

extern Array* beaconsByTypeArray[];

extern Array basicBeaconArray;
extern Array trafficBeaconArray;

extern F32		combatBeaconGridBlockSize;
extern Array	combatBeaconArray;

extern StashTable namedSpatialMarkers;

enum{
	BEACON_CURVE_POINT_COUNT = 20,
};

typedef struct BeaconState {
	int		beaconSearchInstance;
	int		galaxySearchInstance;
	int		needBlocksRebuilt;
} BeaconState;

extern BeaconState beacon_state;

typedef enum BeaconType {
	BEACONTYPE_BASIC = 0,
	BEACONTYPE_COMBAT,
	BEACONTYPE_TRAFFIC,
	
	BEACONTYPE_COUNT,
} BeaconType;

struct Beacon{
	Mat4						mat;				// Must be first thing in struct.
	//Vec3						pos;

	Vec3						pyr;
	
	BeaconType					type;

	BeaconBlock*				block;				// The block that the beacon is currently in (combat beacons only).
	
	S32							proximityRadius;
	
	Array						gbConns;			// ground BeaconConnections.
	Array						rbConns;			// raised BeaconConnections.
	
	BeaconDynamicConnection*	disabledConns;		// BeaconConnections that have been temporarily removed by doors.
	
	F32							floorDistance;
	F32							ceilingDistance;
	
	BeaconAvoidNode*			avoidNodes;

	union{
		struct{
			Array				curveArray;			// Array of curves to matching traffic beacons.
			S16					trafficGroupFlags;	// Bits represent which group(s) the beacon belongs to
			U8					trafficTypeNumber;	// Type number must match for traffic beacons to connect to each other.
		};
		struct{
			void*				astarInfo;			// Pointer used by AStarSearch.  Should be NULL for all beacons.
			S32					pathsBlockedToMe;	// How many paths were blocked to me.
			S32					searchInstance;		// Search instance for combat beacons.
			S8					connectionLevel;	// Used when showing connections.
		};
	};
	
	union{
		void*					userPointer;		// User defined stuff, use for temporary things.
		S32						userInt;
		F32						userFloat;
	};
	
	U32							drawnConnections		: 1;// Debug flag for showing connections.
	U32							madeGroundConnections	: 1;// Set if ground connections have been made (there might not be any).
	U32							madeRaisedConnections	: 1;// Set if raised connections have been made (there might not be any).
	U32							killerBeacon			: 1;// Set for traffic beacons that destroy the entity when it arrives at this beacon.
	U32							isValidStartingPoint	: 1;// This beacon was created from a known valid starting point.
	U32							wasReachedFromValid		: 1;// This beacon was reached during processing from a valid starting point.
	U32							noGroundConnections		: 1;// Disable making ground connections in beacon processor.
	U32							groundConnsSorted		: 1;// Used during clusterization for fast target lookups.
	U32							raisedConnsSorted		: 1;// Used during clusterization for fast target lookups.
	U32							isEmbedded				: 1;// Used during NPC beacon processing to avoid connecting to beacons that we know to be embedded in geometry.
	U32							NPCClusterProcessed		: 1;// Used to keep track of whether this beacon has been processed in NPC Cluster finding.
	U32							NPCNoAutoConnect		: 1;// Disable automatically making a connection out of this island.
	U32							pathLimitedAllowed		: 1;// Entities that are set as path limited are allowed to use this beacon.
};

void beaconFix();

typedef void BeaconForEachBlockCallback(Array* beaconArray, void* userData);

void beaconForEachBlock(const Vec3 pos, float rx, float ry, float rz, BeaconForEachBlockCallback func, void* userData);

Beacon* beaconGetNearestBeacon(const Vec3 pos);

int beaconEntitiesInSameCluster( Entity * e1, Entity * e2 );

#endif
