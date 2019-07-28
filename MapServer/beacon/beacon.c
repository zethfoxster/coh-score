
//#include <stdlib.h>
//#include <assert.h>
//#include <memory.h>
//#include <string.h>
//#include <limits.h>
//#include <F32.h>

#include "beaconPrivate.h"
#include "assert.h"
#include "beacon.h"
#include "ArrayOld.h"
#include "error.h"
#include "gridcoll.h"
#include "beaconConnection.h"
#include "strings_opt.h"
#include "beaconFile.h"
#include "group.h"
#include "groupscene.h"
#include "entity.h"
#include "entserver.h"
#include "anim.h"
#include "timing.h"
#include "StashTable.h"

StashTable namedSpatialMarkers;

Array* beaconsByTypeArray[BEACONTYPE_COUNT];

Array basicBeaconArray;
Array trafficBeaconArray;

F32 combatBeaconGridBlockSize = 256;
Array combatBeaconArray;
Array combatBeaconGridBlockArray;
Array combatBeaconGalaxyArray[BEACON_GALAXY_GROUP_COUNT];
Array combatBeaconClusterArray;

StashTable combatBeaconGridBlockTable = 0;

BeaconState beacon_state;

// Beacon

MP_DEFINE(Beacon);

Beacon* createBeacon(){
	MP_CREATE(Beacon, 4096);
	
	return MP_ALLOC(Beacon);
}

void destroyCombatBeacon(Beacon* beacon){
	clearArrayEx(&beacon->gbConns, destroyBeaconConnection);
	clearArrayEx(&beacon->rbConns, destroyBeaconConnection);
	
	MP_FREE(Beacon, beacon);
}

void destroyNonCombatBeacon(Beacon* beacon){
	if(beacon->gbConns.maxSize)
		destroyArrayPartialEx(&beacon->gbConns, destroyBeaconConnection);

	if(beacon->rbConns.maxSize)
		destroyArrayPartialEx(&beacon->rbConns, destroyBeaconConnection);

	if(beacon->type == BEACONTYPE_TRAFFIC && beacon->curveArray.maxSize)
		destroyArrayPartialEx(&beacon->curveArray, NULL);
	
	MP_FREE(Beacon, beacon);
}

void clearBeaconConnectionLevel(){
	S32 i;
	for(i = 0; i < combatBeaconArray.size; i++){
		Beacon* b = combatBeaconArray.storage[i];
		b->connectionLevel = -1;
		b->drawnConnections = 0;
	}
}

F32 beaconGetDistanceYToCollision(const Vec3 pos, F32 dy){
	CollInfo info;
	Vec3 rayEndPt;

	copyVec3(pos, rayEndPt);

	rayEndPt[1] += dy;

	if(collGrid(NULL, pos, rayEndPt, &info, 1, COLL_NOTSELECTABLE | COLL_DISTFROMSTART | COLL_BOTHSIDES)){
		return fabs(pos[1] - info.mat[3][1]);
	}else{
		return fabs(dy);
	}
}

F32 beaconGetPointFloorDistance(const Vec3 posParam){
	#if 1
		const F32 extraHeight = 1.0f;
	#else
		const F32 extraHeight = 3.5f;
	#endif
	Vec3 pos;
	F32 distance;

	copyVec3(posParam, pos);
	
	pos[1] += extraHeight;

	PERFINFO_AUTO_START("beaconGetFloorDistance", 1);
		distance = beaconGetDistanceYToCollision(pos, -5000) - extraHeight;
	PERFINFO_AUTO_STOP();
	
	if(distance <= 0.0f){
		distance = 0.0001f;
	}
	
	return distance;
}

F32 beaconGetFloorDistance(Beacon* b){
	if(!*(S32*)&b->floorDistance){
		b->floorDistance = beaconGetPointFloorDistance(posPoint(b));
	}
	
	return b->floorDistance;
}

F32 beaconGetPointCeilingDistance(const Vec3 posParam){
	Vec3 pos;
	F32 distance;

	copyVec3(posParam, pos);
	
	PERFINFO_AUTO_START("beaconGetCeilingDistance", 1);
		distance = beaconGetDistanceYToCollision(pos, scene_info.maxHeight - vecY(pos));
	PERFINFO_AUTO_STOP();
	
	if(distance <= 0){
		distance = 0.0001;
	}
	
	return distance;
}

F32 beaconGetCeilingDistance(Beacon* b){
	if(!*(S32*)&b->ceilingDistance){
		b->ceilingDistance = beaconGetPointCeilingDistance(posPoint(b));
	}
	
	return b->ceilingDistance;
}

void beaconClearBeaconData(S32 clearNPC, S32 clearTraffic, S32 clearCombat){
	if(clearNPC){
		// Init basic beacons.

		destroyArrayPartialEx(&basicBeaconArray, destroyNonCombatBeacon);
		beaconsByTypeArray[BEACONTYPE_BASIC] = &basicBeaconArray;
	}

	if(clearTraffic){
		// Init the traffic beacons.

		clearArrayEx(&trafficBeaconArray, destroyNonCombatBeacon);
		beaconsByTypeArray[BEACONTYPE_TRAFFIC] = &trafficBeaconArray;
	}

	if(clearCombat){
		// Init combat beacons.

		destroyArrayPartialEx(&combatBeaconArray, destroyCombatBeacon);
		beaconsByTypeArray[BEACONTYPE_COMBAT] = &combatBeaconArray;		

		// Create the hash table for looking up grid blocks.

		if(combatBeaconGridBlockTable) stashTableDestroy(combatBeaconGridBlockTable);
		combatBeaconGridBlockTable = stashTableCreateInt(64);
		
		// Clear the galaxy and cluster arrays.
		
		beaconClearAllBlockData(1);
		
		// Create the array for indexed lookups on the grid blocks.

		destroyArrayPartialEx(&combatBeaconGridBlockArray, destroyBeaconGridBlock);
		
		// Clear all the beacon memory.
		
		beaconFreeAllMemory();
	}
}

GroupDef* beaconFixHelper(GroupDef* def, Mat4 mat){
	S32 i;
	Vec3 pos;
	extern S32 world_modified;
	Mat4 mat2;
	S32 turnOffBeaconFlag = 0;

	if (!def)
		return 0;
	for(i = 0; i < def->count; i++){
		GroupEnt* ent = def->entries + i;
		GroupDef* child = ent->def;
		
		if(!def->has_beacon && stricmp(child->name,"_NAVCMBT") == 0){
			// Group isn't a beacon, and a child is a beacon geometry.
			
			mulMat4(mat, ent->mat, mat2);
			copyVec3(mat2[3], pos);

			printf("  Repaired an abandoned beacon geometry at: ( %1.f, %1.f, %1.f )\n", pos[0], pos[1], pos[2]);
			ent->def = groupDefFind("NAVCMBT");
			def->saved = 0;
			groupDefModify(def);
			world_modified = 1;
		}
		else if(def->has_beacon && child->has_beacon && def->count == 1){
			// Group is a beacon with an only child that is also a beacon.
			
			return child;
		}else{
			GroupDef* newChild;
			
			// Just for good measure, turn off the has_beacon flag.
			
			if(def->has_beacon){
				if(child->has_beacon){
					turnOffBeaconFlag = 1;
				}
			
				if(child->model && def->count > 1){
					// Def is a beacon, but child has an anim and there's more than one child, this is bad.

					Model* model = child->model;
					
					mulMat4(mat, ent->mat, mat2);
					copyVec3(mat2[3], pos);

					printf("Bad Beacon: %s: ( %1.f, %1.f, %1.f )\n", model->name, pos[0], pos[1], pos[2]);
					
					turnOffBeaconFlag = 1;
					
					// Figure out what kind of beacon to make it.
					
					if(stricmp(model->name, "_NAV__NAV") == 0)
						ent->def = groupDefFind("NAV");
					else if(stricmp(model->name, "_NAVCMBT__NAV") == 0)
						ent->def = groupDefFind("NAVCMBT");
					else if(stricmp(model->name, "_DIR__NAV") == 0)
						ent->def = groupDefFind("NAV_DIR");

					def->saved = 0;
					groupDefModify(def);
					child->in_use = 0;
					world_modified = 1;
				}
			}

			mulMat4(mat, ent->mat, mat2);
			copyVec3(mat2[3], pos);

			newChild = beaconFixHelper(child, mat2);
			
			if(newChild){
				// Group has a child that was a fucked up beacon.
				
				printf("  Found a beacon with a child beacon at: ( %1.f, %1.f, %1.f )\n", pos[0], pos[1], pos[2]);
				ent->def = newChild;
				def->saved = 0;
				groupDefModify(def);
				child->in_use = 0;
				world_modified = 1;
			}
		}
	}
	
	if(turnOffBeaconFlag){
		def->has_beacon = 0;
	}

	return NULL;
}

// This function will repair bad beacon things.

void beaconFix(){
	S32 i;
	
	verbose_printf("\nChecking for bad beacons...\n");
	for(i = 0; i < group_info.ref_count; i++){
		beaconFixHelper(group_info.refs[i]->def, group_info.refs[i]->mat);
	}
	verbose_printf("Done checking for bad beacons...\n\n");
}

void beaconForEachBlock(const Vec3 pos, F32 rx, F32 ry, F32 rz, BeaconForEachBlockCallback func, void* userData){
	Vec3 posLow =	{ pos[0] - rx, pos[1] - ry, pos[2] - rz };
	Vec3 posHigh =	{ pos[0] + rx, pos[1] + ry, pos[2] + rz };
	S32 x, y, z;
	S32 lo[3];
	S32 hi[3];
	
	beaconMakeGridBlockCoords(posLow);
	beaconMakeGridBlockCoords(posHigh);
	
	copyVec3(posLow, lo);
	copyVec3(posHigh, hi);

	for(x = lo[0]; x <= hi[0]; x++){
		for(y = lo[1]; y <= hi[1]; y++){
			for(z = lo[2]; z <= hi[2]; z++){
				BeaconBlock* block = beaconGetGridBlockByCoords(x, y, z, 0);
				
				if(block){
					func(&block->beaconArray, userData);
				}
			}
		}
	}	
}

static struct {
	Vec3		pos;
	Beacon*		closestBeacon;
	F32			minDistSQR;
} collectClusterBeacons;

static void beaconGetNearestBeaconHelper(Array* beaconArray, void* userData){
	S32 i;
	S32 count = beaconArray->size;
	Beacon** beacons = (Beacon**)beaconArray->storage;
	
	for(i = 0; i < count; i++){
		Beacon* beacon = beacons[i];
		F32	distSQR = distance3Squared(posPoint(beacon), collectClusterBeacons.pos);
		
		if(distSQR < collectClusterBeacons.minDistSQR){
			collectClusterBeacons.closestBeacon = beacon;
			collectClusterBeacons.minDistSQR = distSQR;
		}
	}
}

Beacon* beaconGetNearestBeacon(const Vec3 pos){
	copyVec3(pos, collectClusterBeacons.pos);
	collectClusterBeacons.closestBeacon = NULL;
	collectClusterBeacons.minDistSQR = SQR(50);

	beaconCheckBlocksNeedRebuild();

	beaconForEachBlock(pos, 50, 1000, 50, beaconGetNearestBeaconHelper, NULL);
	
	if(!collectClusterBeacons.closestBeacon){
		copyVec3(pos, collectClusterBeacons.pos);
		collectClusterBeacons.closestBeacon = NULL;
		collectClusterBeacons.minDistSQR = SQR(300);

		beaconForEachBlock(pos, 350, 1000, 350, beaconGetNearestBeaconHelper, NULL);

		if(!collectClusterBeacons.closestBeacon){
			copyVec3(pos, collectClusterBeacons.pos);
			collectClusterBeacons.closestBeacon = NULL;
			collectClusterBeacons.minDistSQR = SQR(5000);

			beaconForEachBlock(pos, 5000, 1000, 5000, beaconGetNearestBeaconHelper, NULL);
		}
	}
	
	return collectClusterBeacons.closestBeacon;
}

S32 beaconEntitiesInSameCluster(Entity* e1, Entity* e2){
	Beacon* b1 = entGetNearestBeacon(e1);
	Beacon* b2 = entGetNearestBeacon(e2);

	// One or both are in an unbeaconed area.
	
	if(!b1 || !b2)
		return 0;

	if(b1->block->galaxy->cluster == b2->block->galaxy->cluster)
		return 1;

	return 0;
}


MP_DEFINE(BeaconAvoidNode);

static BeaconAvoidNode* createBeaconAvoidNode(){
	MP_CREATE(BeaconAvoidNode, 500);
	
	return MP_ALLOC(BeaconAvoidNode);
}

static void destroyBeaconAvoidNode(BeaconAvoidNode* node){
	MP_FREE(BeaconAvoidNode, node);
}


BeaconAvoidNode* beaconAddAvoidNode(Beacon* beacon, Entity* e){
	BeaconAvoidNode* node = createBeaconAvoidNode();
	
	node->beacon = beacon;
	node->entity = e;

	node->beaconList.next = beacon->avoidNodes;
	node->beaconList.prev = NULL;
	beacon->avoidNodes = node;
	
	if(node->beaconList.next){
		node->beaconList.next->beaconList.prev = node;
	}
	
	return node;
}

void beaconDestroyAvoidNode(BeaconAvoidNode* node){
	Beacon* beacon = node->beacon;
	
	if(node->beaconList.next){
		node->beaconList.next->beaconList.prev = node->beaconList.prev;
	}
	
	if(node->beaconList.prev){
		node->beaconList.prev->beaconList.next = node->beaconList.next;
	}else{
		beacon->avoidNodes = node->beaconList.next;
	}
	
	destroyBeaconAvoidNode(node);
}

