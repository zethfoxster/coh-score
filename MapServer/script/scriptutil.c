/*\
 *
 *	scriptutil.h/c - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Utility functions defined in terms of the interface in script.h.
 *  Defines some ease-of-use macros for state management, utility functions
 *	to iterate over players and do things, etc.
 *
 */

#include "scriptengine.h"
#include "scriptutil.h"

int NumEntitiesInArea(AREA area, TEAM team)
{
	NUMBER i, count = 0;
	NUMBER numents = NumEntitiesInTeam(team);
	for (i = 1; i <= numents; i++)
	{
		if (EntityInArea(GetEntityFromTeam(team, i), area))
			count++;
	}
	return count;
}

int EntityExists(ENTITY entity)
{
	return NumEntitiesInTeam(entity);
}

char *Script_GetCurrentBlame()
{
	return g_currentScript ? g_currentScript->pchSourceFile : "";
}
