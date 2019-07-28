/*\
 *
 *	scripthookwaypoint.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides all the functions CoH script files should be referencing -
 *		essentially a list of hooks into game systems.
 *
 *	This file contains all functions related to dynamic waypoints
 *
 */

#ifndef SCRIPTHOOKWAYPOINT_H
#define SCRIPTHOOKWAYPOINT_H

// waypoints
NUMBER CreateWaypoint(LOCATION location, STRING text, STRING icon, STRING iconB, NUMBER compass, NUMBER pulse, FRACTION rotate);
void DestroyWaypoint(NUMBER id);
void SetWaypoint(TEAM team, NUMBER id);
void ClearWaypoint(TEAM team, NUMBER id);
void ClearAllWaypoints(TEAM team);
void UpdateWaypoint(NUMBER id, LOCATION location, STRING text, STRING icon, FRACTION rotate);

#endif