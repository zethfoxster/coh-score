/*\
 *
 *	patrol.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles loading patrol paths for use by encounters,
 *	scripting, and ai
 *
 */

#ifndef __PATROL_H
#define __PATROL_H

#include "stdtypes.h"
#include "grouputil.h"

#define MAX_PATROL_LOCS		20
#define PATROL_NAMELEN		128

typedef enum
{
	PATROL_PINGPONG,		// default
	PATROL_CIRCLE,
	PATROL_ONEWAY,
} PatrolRouteType;

typedef struct PatrolRoute 
{
	char			name[PATROL_NAMELEN];
	Vec3			locations[MAX_PATROL_LOCS];
	U8				locationnums[MAX_PATROL_LOCS];
	U32				numlocs;
	PatrolRouteType	routetype;
} PatrolRoute;

void LoadPatrolRoutes(void); // should be loaded by encounterload()
PatrolRoute* LoadPatrolRoute(GroupDefTraverser* traverser, char* name);
PatrolRoute* PatrolRouteFind(char* name);

#endif // __PATROL_H