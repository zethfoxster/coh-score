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

#include "memorypool.h"
#include "patrol.h"
#include "textparser.h"
#include "earray.h"
#include "error.h"
#include "group.h"
#include "groupProperties.h"
#include "groupnetsend.h"
#include "utils.h"
#include "mathutil.h"
#include "StashTable.h"

StaticDefineInt ParsePatrolRouteType[] = 
{
	DEFINE_INT
	{ "PingPong",				PATROL_PINGPONG	},
	{ "Circle",					PATROL_CIRCLE	},
	{ "OneWay",					PATROL_ONEWAY	},
	DEFINE_END
};

MP_DEFINE(PatrolRoute);

PatrolRoute** g_patrolRoutes = 0; 
PatrolRoute g_curRoute = {0};

int AddPatrolPoint(PatrolRoute* route, int num, Vec3 pos)
{
	int i;
	if (route->numlocs == MAX_PATROL_LOCS)
	{
		ErrorFilenamef(currentMapFile(), "Too many PatrolPoints: location (%0.1f,%0.1f,%0.1f)",
			pos[0], pos[1], pos[2]);
		return 0;
	}

	// insert, keeping order
	for (i = 0; i < (int)route->numlocs; i++)
	{
		if (num == route->locationnums[i])
		{
			ErrorFilenamef(currentMapFile(),
				"There is already a marker number %d, can't add identical marker at (%0.1f,%0.1f,%0.1f)",
				num, pos[0], pos[1], pos[2]);
			return 0;
		}
		else if (num <= route->locationnums[i]) break;
	}
	memmove(&route->locations[i+1], &route->locations[i], sizeof(route->locations[0]) * (route->numlocs - i));
	memmove(&route->locationnums[i+1], &route->locationnums[i], sizeof(route->locationnums[0]) * (route->numlocs - i));
	copyVec3(pos, route->locations[i]);
	route->locationnums[i] = num;
	route->numlocs++;
	return 1;
}

int patrolLoader(GroupDefTraverser* traverser)
{
	PropertyEnt* prop = NULL;

	if (traverser->def->properties)
		stashFindPointer( traverser->def->properties, "PatrolPoint", &prop );
	if (prop)
	{
		int point = atoi(prop->value_str);
		if (point < 0 || point > MAX_PATROL_LOCS)
		{
			ErrorFilenamef(currentMapFile(), "PatrolPoint gave value %s.  Location (%0.1f,%0.1f,%0.1f)",
				prop->value_str, traverser->mat[3][0], traverser->mat[3][1], traverser->mat[3][2]);
		}
		else
			AddPatrolPoint(&g_curRoute, point, traverser->mat[3]); // this outputs its own errors
	}

	if (traverser->def->properties)
		stashFindPointer( traverser->def->properties, "RouteType", &prop );
	if (prop)
	{
		int type = StaticDefineIntGetInt(ParsePatrolRouteType, prop->value_str);
		if (type < 0)
		{
			ErrorFilenamef(currentMapFile(), "RouteType gave invalid value %s.  Location (%0.1f,%0.1f,%0.1f)",
				prop->value_str, traverser->mat[3][0], traverser->mat[3][1], traverser->mat[3][2]);
		}
		else
		{
			g_curRoute.routetype = type;
		}
	}

	if (!traverser->def->child_properties)
		return 0;
	return 1;
}

// can be called to load a patrol route (if any) from the current location in the
// traverser tree.  this pointer can be invalidated if the map is reinit'ed
PatrolRoute* LoadPatrolRoute(GroupDefTraverser* traverser, char* name)
{
	memset(&g_curRoute, 0, sizeof(g_curRoute));
	groupProcessDefExContinue(traverser, patrolLoader);
	if (g_curRoute.numlocs)
	{
		PatrolRoute* route = MP_ALLOC(PatrolRoute);
		*route = g_curRoute;

		// HACK - I put all named routes at front
		if (name && name[0])
		{
			strncpyt(route->name, name, ARRAY_SIZE(route->name));
			eaInsert(&g_patrolRoutes, route, 0);
		}
		else
			eaPush(&g_patrolRoutes, route);
		return route;
	}
	return NULL;
}

int namedPatrolLoader(GroupDefTraverser* traverser)
{
	PropertyEnt* prop = NULL;

	// if we have a named marker
	if (traverser->def->properties)
		stashFindPointer( traverser->def->properties, "NamedPatrolRoute", &prop );
	if (prop)
	{
		PatrolRoute* route = LoadPatrolRoute(traverser, prop->value_str); // adds to global list
	}
	if (!traverser->def->child_properties)
		return 0;
	return 1;
}

// should be loaded by encounterload()
void LoadPatrolRoutes(void)
{
	int i, n;
	GroupDefTraverser traverser = {0};

	MP_CREATE(PatrolRoute, 10);

	// destroy any routes we currently have - will destroy routes referenced by encounters
	if (!g_patrolRoutes)
		eaCreate(&g_patrolRoutes);
	n = eaSize(&g_patrolRoutes);
	for (i = 0; i < n; i++)
	{
		MP_FREE(PatrolRoute, g_patrolRoutes[i]);
	}
	eaSetSize(&g_patrolRoutes, 0);

	// load any named routes
	loadstart_printf("Loading named patrol routes.. ");
	groupProcessDefExBegin(&traverser, &group_info, namedPatrolLoader);
	loadend_printf("%i patrol routes", eaSize(&g_patrolRoutes));
}

PatrolRoute* PatrolRouteFind(char* name)
{
	int i, n;

	n = eaSize(&g_patrolRoutes);
	for (i = 0; i < n; i++)
	{
		PatrolRoute* ret = g_patrolRoutes[i];

		// HACK - I put all named routes at front
		if (!ret->name[0]) break; 
		if (!stricmp(name, ret->name))
			return ret;
	}
	return NULL;
}