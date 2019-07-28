#ifndef SCRIPTTYPES_H
#define SCRIPTTYPES_H


// Script types
typedef char* mSTRING; // String that is still modifiable
typedef const char* STRING;
typedef const char* VARIABLE;
typedef int NUMBER;
typedef float FRACTION;

// TEAM & ENTITY can be used interchangably.  ENTITY parameters will only affect
// the first entity in the team.
typedef const char* TEAM;
typedef const char* ENTITY;

// WIDGET & COLLECTION can be used interchangably just like TEAM & ENTITY
// COLLECTION references to a list of widgets
typedef const char* WIDGET;
typedef const char* COLLECTION;

// AREA & LOCATION & ENCOUNTERGROUP can be used interchangably.  These parameter
// show what kind of location the function is expecting or returning, but they are
// always interchangeable.
//		AREA parameters affect an area
//		LOCATION parameters refer to a single point
//		ENCOUNTERGROUP parameters refer to an encountergroup
typedef const char* AREA;
typedef const char* LOCATION;
typedef const char* ENCOUNTERGROUP;

// Teams & entities
#define TEAM_NONE					"ent:none"
#define ALL_PLAYERS					"each:player"
#define ALL_PLAYERS_READYORNOT		"each:player_readyornot"
#define ALL_CRITTERS				"each:critter"
#define ALL_DOORS					"each:door"
#define LOC_ORIGIN					"coord:0.0,0.0,0.0"

#endif