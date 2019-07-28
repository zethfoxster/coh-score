// ZONE SCRIPT
//
// The Shadow Shard Invasion will be the first shard event
// it is pretty simple, and just tries to maintain a constant number of
// rularuu invaders in each city zone.  It maintains 4 vines in the
// city, and 100 invaders that have spawned out of vines.

//Portal spawns every 5 minutes (changed from 10)
//Still only 4 Portals
//1 group spawns every 30 seconds (changed from 1 minutes)
//Max. 100 entities

#include "mathutil.h"

#include "scriptutil.h"

////////////////////////////////////////////////////////////////////////////////////////

#define SPAWNS_NUM		7
#define SPAWNS_HIGH		20

typedef struct WESpawnTypes
{
	int prob;
	char* spawn[2];
} WESpawnTypes;

WESpawnTypes WESpawns[] = {
	{ 20,	"Winter_Event_High_D1_V0.spawndef", "Winter_Event_Low_D1_V0.spawndef" },
	{ 40,	"Winter_Event_High_D1_V1.spawndef", "Winter_Event_Low_D1_V1.spawndef" },
	{ 60,	"Winter_Event_High_D1_V2.spawndef", "Winter_Event_Low_D1_V2.spawndef" },
	{ 75,	"Winter_Event_High_D1_V3.spawndef", "Winter_Event_Low_D1_V3.spawndef" },
	{ 90,	"Winter_Event_High_D1_V4.spawndef", "Winter_Event_Low_D1_V4.spawndef" },
	{ 95,	"Winter_Event_High_D1_V5.spawndef", "Winter_Event_Low_D1_V5.spawndef" },
	{ 101,	"Winter_Event_High_D1_V6.spawndef", "Winter_Event_Low_D1_V6.spawndef" },
};

////////////////////////////////////////////////////////////////////////////////////////

typedef struct WEZoneTypes
{
	int percentChance[6];	
	int spawnsActive[6];
} WEZoneTypes;

WEZoneTypes WEZoneChances[] = {
	{ { 0, 0, 5, 50, -1, -1 }, { 0, 0, 0, 0, 5, 10 } },			// City Zones
	{ { 5, 25, 50, -1, -1, -1 }, { 0, 0, 0, 1, 5, 10 } },		// Hazard Zones
	{ { 25, 50, 100, -1, -1, -1 }, { 0, 0, 0, 1, 5, 10 } },		// Trial Zones
};

////////////////////////////////////////////////////////////////////////////////////////

typedef struct WEZoneDataType
{
	char *map_name;	
	int zone_type;
	int spawnLevel;
} WEZoneDataType;


WEZoneDataType WEZoneData[] =
{
	{ "City_01_01",		0, 5 },		// Atlas Park
	{ "City_01_03",		0, 5 },		// Galaxy City
	{ "City_01_02",		1, 10 },	// Kings Row
	{ "Hazard_01_01",	1, 12 },	// Perez Park
	{ "Hazard_01_02",	1, 12 },	// Hollows
	{ "City_02_01",		0, 15 },	// Steel Canyon
	{ "City_02_02",		0, 15 },	// Steel Canyon
	{ "Hazard_02_01",	1, 17 },	// Boomtown
	{ "Trial_02_01",	2, 18 },	// Faultline
	{ "City_03_01",		0, 25 },	// Talos Island
	{ "City_03_02",		0, 25 },	// Independence Port
	{ "Hazard_03_01",	1, 27 },	// Dark Astoria
	{ "Hazard_03_02",	1, 27 },	// Striga Isle
	{ "Trial_03_01",	2, 28 },	// Terra Volta
	{ "City_04_01",		0, 35 },	// Founder's Falls
	{ "City_04_02",		0, 35 },	// Brickstown
	{ "Hazard_04_01",	1, 37 },	// Crey's Folley
	{ "Trial_04_01",	2, 40 },	// Eden
	{ "City_05_01",		0, 45 },	// Peregrine Island
	{ "Trial_05_01",	2, 47 },	// Rikti Crashsite
	{ "End",			0, 0 },		// End Marker
};


////////////////////////////////////////////////////////////////////////////////////////

// don't run on city zones that don't have the marker we expect
static int CorrectCityZone()
{
	return LocationExists("marker:Winter_Event_01");
}

static void WESpawnVillainsHere(STRING location)
{
	int i, r;
		
	STRING spawndef = "SpawnDefs/ZoneEvents/Expansion3/";
	int spawnLevel = VarGetNumber("SpawnLevel");

	r = rule30Rand() % 100;
	for ( i = 0; i < SPAWNS_NUM; i++) {
		if (r < WESpawns[i].prob) {
			if (spawnLevel >= SPAWNS_HIGH) {
				spawndef = StringAdd(spawndef, WESpawns[i].spawn[0]);				
			} else {
				spawndef = StringAdd(spawndef, WESpawns[i].spawn[1]);				
			}
			break;
		}
	}

	if (LocationExists(location))
	{
		STRING group = FindEncounterGroupByRadius(location, 0, 300, "Around", 0, 0);
		if (LocationExists(group))
		{
			DebugString(StringAdd("Spawning winter event villains at ", location));
			CloseEncounter(group); // if it's open, just get rid of anyone there
			ReserveEncounter(group); // don't spawn anything else there until I'm done
			Spawn(1, "WEVillains", spawndef, group, NULL, spawnLevel, 0);
//			DetachSpawn("WEVillains"); // don't detach
		}
	}

}

static void WESpawnVillains(void)
{
	int i, r;
	int SpawnPercent = WEZoneChances[VarGetNumber("ZoneType")].percentChance[VarGetNumber("Stage")-1];
	int VillainsActive = WEZoneChances[VarGetNumber("ZoneType")].spawnsActive[VarGetNumber("Stage")-1];
	STRING spawndef = "SpawnDefs/ZoneEvents/Expansion3/";

	if (SpawnPercent > 0) 
	{
		for (i = 0; i < 6; i++) {
			STRING marker = "marker:Winter_Event_0";
			marker = StringAdd(marker, NumberToString(i+1));

			r = rule30Rand() % 100;
			if (r < SpawnPercent) {
				// spawn monsters here
				WESpawnVillainsHere(marker);
			}
		}
	} 
	else if (NumEntitiesInTeam("WEVillains") < VillainsActive)
	{
		int spawnLevel = VarGetNumber("SpawnLevel");
		int loopCount = 5;		// don't loop more than 5 times before exiting and waiting till next check

		// create villians until there are enough
		while (NumEntitiesInTeam("WEVillains") < VillainsActive &&
				loopCount > 0)
		{
			// need to reset things for each loop
			STRING marker = "marker:Winter_Event_0";
			spawndef = "SpawnDefs/ZoneEvents/Expansion3/";

			// decrement loop count
			loopCount--;

			// pick random spawn type
			r = rule30Rand() % 100;
			for ( i = 0; i < SPAWNS_NUM; i++) {
				if (r < WESpawns[i].prob) {
					if (spawnLevel >= SPAWNS_HIGH) {
						spawndef = StringAdd(spawndef, WESpawns[i].spawn[0]);				
					} else {
						spawndef = StringAdd(spawndef, WESpawns[i].spawn[1]);				
					}
					break;
				}
			} 

			// pick random spawn location
			r = rule30Rand() % 6;
			marker = StringAdd(marker, NumberToString(r+1));

			if (LocationExists(marker))
			{
				STRING group = FindEncounterGroupByRadius(marker, 0, 300, "Around", 0, 0);
				if (LocationExists(group))
				{
					DebugString(StringAdd("Spawning winter event villains at ", marker));
					CloseEncounter(group); // if it's open, just get rid of anyone there
					ReserveEncounter(group); // don't spawn anything else there until I'm done
					Spawn(1, "WEVillains", spawndef, group, NULL, spawnLevel, 0);
					// DetachSpawn("WEVillains"); // don't detach
				}
				 else {
					// can't find group to spawn - exit out of loop
					break;
				}
			} else {
				// can't find anyplace to spawn - exit out of loop
				break;
			}
		}
	}
}

 
void WinterEvent(void)
{
	int i;
	int SpawnPercent;
	
	if (VarGetNumber("Stage") <= 0)
		VarSetNumber("Stage", 1);

	SpawnPercent = WEZoneChances[VarGetNumber("ZoneType")].percentChance[VarGetNumber("Stage")-1];

	if (SpawnPercent > 0) 
	{
		DoEvery("WESpawnVillains", 60.0, WESpawnVillains);
	} else {
		DoEvery("WESpawnVillains", 0.25, WESpawnVillains);
	}

	VarSetNumber("LiveVillains", NumEntitiesInTeam("WEVillains"));

	INITIAL_STATE

		ON_ENTRY 
			// Does this zone have the event?
			if (CorrectCityZone())
			{
				VarSetNumber("SpawnLevel", 0);

				// loop through all of the zone entries
				for (i = 0; stricmp(WEZoneData[i].map_name, "End"); i++)
				{
					if (!stricmp(WEZoneData[i].map_name, GetShortMapName())) 
					{
						// found map - save data into variable
						VarSetNumber("ZoneType", WEZoneData[i].zone_type);
						VarSetNumber("SpawnLevel", WEZoneData[i].spawnLevel);
						break;
					}
				}

				// sanity check to make sure we've found the map in the list
				if (VarGetNumber("SpawnLevel") == 0)
				{
					EndScript();
				}
			}
			else
			{
				EndScript();
			}
		END_ENTRY

		// Go to stage one!
		SET_STATE( "StageOne" );

	STATE("StageOne")
		ON_ENTRY 
			VarSetNumber("Stage", 1);
		END_ENTRY

	STATE("StageTwo")
		ON_ENTRY 
			VarSetNumber("Stage", 2);
		END_ENTRY

	STATE("StageThree")
		ON_ENTRY 
			VarSetNumber("Stage", 3);
		END_ENTRY

	STATE("StageFour")
		ON_ENTRY 
			VarSetNumber("Stage", 4);
		END_ENTRY

	STATE("StageFive")
		ON_ENTRY 
			VarSetNumber("Stage", 5);
		END_ENTRY

	STATE("StageSix")
		ON_ENTRY 
			VarSetNumber("Stage", 6);
		END_ENTRY

	END_STATES

	ON_SIGNAL("MoveToStageOne")
		SET_STATE( "StageOne" );
	END_SIGNAL

	ON_SIGNAL("MoveToStageTwo")
		SET_STATE( "StageTwo" );
	END_SIGNAL

	ON_SIGNAL("MoveToStageThree")
		SET_STATE( "StageThree" );
	END_SIGNAL

	ON_SIGNAL("MoveToStageFour")
		SET_STATE( "StageFour" );
	END_SIGNAL

	ON_SIGNAL("MoveToStageFive")
		SET_STATE( "StageFive" );
	END_SIGNAL

	ON_SIGNAL("MoveToStageSix")
		SET_STATE( "StageSix" );
	END_SIGNAL

	ON_STOPSIGNAL
		Kill("WEVillains", 0);
	END_SIGNAL
}