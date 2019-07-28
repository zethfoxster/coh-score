/*\
 *
 * 	missiongeo.h/c - Copyright 2003, 2004 Cryptic Studios
 * 		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles loading geometry objects related to missions and
 *  tracking info about them.  Includes information about
 *	logical mission rooms.
 *
 */

#ifndef __MISSIONGEO_H
#define __MISSIONGEO_H

#include "earray.h"
#include "missionMapCommon.h"
#include "entity.h"
#include "stdio.h"

//#include "storyarcinterface.h"
//#include "missionspec.h"
//#include "svr_base.h"

// *********************************************************************************
//  Mission markers
// *********************************************************************************

#define MAX_OBJECTIVE_MARKER 10

typedef enum LocationTypeEnum {
	LOCATION_PERSON,
	LOCATION_ITEMWALL,
	LOCATION_ITEMFLOOR,
	LOCATION_ITEMROOF,
	LOCATION_LEGACY,	// delete later
	LOCATION_TROPHY,		// for supergroup raids
} LocationTypeEnum;

typedef struct ItemPosition
{
	Mat4 pos;
	int used;
	LocationTypeEnum loctype;
	char * customName; 
} ItemPosition;

typedef struct EncounterGroup EncounterGroup;

typedef struct RoomInfo 
{
	DefTracker* tracker;
	int roompace;
	ItemPosition** items;				// list of possible positions for items
	EncounterGroup** standardEncounters;	// list of normal encounter groups in this room
	EncounterGroup** hostageEncounters;		// list of hostage encounter groups in this room
	int* objectiveMarkers;					// list of objective markers on this room
	Vec3 markerPos;							// original position of the room marker
	unsigned int startroom: 1;		// is this room the start room
	unsigned int endroom: 1;		// is this room the end room
	unsigned int hadtracker: 1;		// just a return signal from RoomGetInfo
	unsigned int lobbyroom: 1;		// the room that players enter into (different than the start room which is the first room that normal mobs spawn in)
} RoomInfo;


extern RoomInfo** g_rooms;
extern int g_lowpace;
extern int g_highpace;

void MissionLoad(int forceload, int mapid, char * mapname);	// load the map objects
void MissionProcessEncounterGroup(EncounterGroup* group); 
	// called from encounter load - attach group room it belongs in

typedef struct EncounterGroup EncounterGroup;

RoomInfo* RoomGetInfoEncounter(EncounterGroup* group);
RoomInfo* RoomGetInfo(const Vec3 pos);
RoomInfo* RoomFindClosest(const Vec3 findPos);
RoomInfo* RoomInfoFromPace(int pace);
RoomInfo* RoomInfoFromTracker(DefTracker* tracker);
RoomInfo* StartRoomInfo();
RoomInfo* EndRoomInfo();

// *********************************************************************************
//  Mission marker debugging
// *********************************************************************************

RoomInfo* ItemTestingLoad(ClientLink* client);

void MissionDebugSaveMapStatsToFile(Entity *player, char * mapname);
void MissionMapStats_MakeFile( char * pchFile );
void MissionMapStats_gather(MissionMapStats *mapStats, Entity *player);


#endif //__MISSIONGEO_H
