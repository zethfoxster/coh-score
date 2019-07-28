/*\
 *
 *	scripthooklocation.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides all the functions CoH script files should be referencing -
 *		essentially a list of hooks into game systems.
 *
 *	This file contains all functions related to locations
 *
 */

#ifndef SCRIPTHOOKLOCATION_H
#define SCRIPTHOOKLOCATION_H

#include "scripttypes.h"

typedef StashElement SCRIPTELEMENT;
typedef int (*SCRIPTMARKERPROCESSOR)(SCRIPTELEMENT element);
typedef struct ScriptMarker * SCRIPTMARKER;

#define LOCATION_RADIUS		30		// default radius to add to any point to get an area
#define MAX_MARKER_COUNT	99

// Locations
#define LOCATION_NONE	"location:none"
#define LOCATION_ANY	"location:any"
int EntityInArea(ENTITY ent, AREA area);
AREA RoomFromEntity(ENTITY ent);
LOCATION PointFromEntity(ENTITY ent);
int LocationExists(LOCATION location);
LOCATION MyLocation();				// returns location this script is attached to (SCRIPT_LOCATION only)
FRACTION DistanceBetweenEntities(ENTITY ent1, ENTITY ent2);
FRACTION DistanceBetweenLocations(LOCATION loc1, LOCATION loc2);
STRING GetXYZStringFromLocation(LOCATION loc);
NUMBER CountMarkers(STRING name);

// scripthook.c
void ForEachScriptMarker(SCRIPTMARKERPROCESSOR f);
SCRIPTMARKER GetScriptMarkerFromScriptElement(SCRIPTELEMENT elem);
STRING FindMarkerProperty(SCRIPTMARKER marker, STRING name);
STRING FindMarkerPropertyByMarkerName(STRING markerName, STRING name);
STRING FindMarkerGeoName(SCRIPTMARKER marker);


#endif
