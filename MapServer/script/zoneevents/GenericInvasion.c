// ZONE SCRIPT
//
// The generic invasion script is pretty simple, and just tries to maintain a constant number of
// invaders in each city zone.  It maintains a set number of portals in the
// city, and a set number of invaders that have spawned out of portals.


#include "scriptutil.h"

// don't run on city zones that don't have the marker we expect
static int CorrectCityZone()
{
	return LocationExists(VarGetArrayElement("Markers", 0));
}

// look for a Rularuu invasion marker that doesn't have a vine already nearby
static STRING FindFreePortalLocation(void)
{
	NUMBER i, count, start, firsttime;

	count = VarGetArrayCount("Markers");

	// start at random point and look circularly
	firsttime = 1;
	start = RandomNumber(0, count - 1);
	for (i = start; ; i = (i + 1) % count)
	{
		STRING point = VarGetArrayElement("Markers", i);

		// check and see if any portals are nearby
		if (LocationExists(point) && 
			NumEntitiesInArea(point, "Portals") == 0)
		{
			return point;
		}

		// make sure we don't loop forever
		if (i == start && !firsttime) 
			return LOCATION_NONE;

		firsttime = 0;
	}
}

// done
// choose one of the portals at random and return its location
static STRING FindExistingPortalLocation(void)
{
	NUMBER i, n;
	n = NumEntitiesInTeam("Portals");
	if (n == 0)
		return LOCATION_NONE;

	i = RandomNumber(1, n);
	return PointFromEntity(GetEntityFromTeam("Portals", i));
}


//done
static void SpawnInvaders(void)
{
	if ( NumEntitiesInTeam("Invaders") < VarGetNumber("InvaderCount") )
	{
		STRING location;
		STRING group;
		STRING spawndef = StringAdd("SpawnDefs/", VarGet("SpawnDirectory"));;
		spawndef = StringAdd(spawndef, "/");
		spawndef = StringAdd(spawndef, GetShortMapName());
		spawndef = StringAdd(spawndef, "_Invaders_D1_V0.spawndef");

		location = FindExistingPortalLocation();
		if (LocationExists(location))
		{
			group = FindEncounterGroup(location, VarGet("SpawnLayout"), 0, 0);
			if (LocationExists(group))
			{
				DebugString(StringAdd("Spawning invaders at ", location));
				Spawn(1, "Invaders", spawndef, group, VarGet("SpawnLayout"), 0, 0);
				DetachSpawn("Invaders");
				SetAIPriorityList("Invaders", VarGet("PriorityList"));
			}
		}
	}
}


// done
static void SpawnPortals(void)
{
	if (NumEntitiesInTeam("Portals") < VarGetNumber("PortalCount") AND VarIsFalse("DontSpawnPortals"))
	{
		STRING location;
		STRING group;
		STRING spawndef = StringAdd("SpawnDefs/", VarGet("SpawnDirectory"));;
		spawndef = StringAdd(spawndef, "/");
		spawndef = StringAdd(spawndef, GetShortMapName());
		spawndef = StringAdd(spawndef, "_Portal_D1_V0.spawndef");

		location = FindFreePortalLocation();
		if (LocationExists(location))
		{
			group = FindEncounterGroup(location, VarGet("SpawnLayout"), 0, 0); 
			if (LocationExists(group))
			{
				DebugString(StringAdd("Spawning portal at ", location));
				CloseEncounter(group); // if it's open, just get rid of anyone there
				ReserveEncounter(group); // don't spawn anything else there until I'm done
				Spawn(1, "Portals", spawndef, group, VarGet("SpawnLayout"), 0, 0);
				DetachSpawn("Portals");
			}
		}
	}
}


void GenericInvasion(void)
{
	int i;

	INITIAL_STATE

		ON_ENTRY 
			if (CorrectCityZone())
			{
				for (i = 1; i <= VarGetNumber("PortalCount"); i++)
					SpawnPortals();
				for (i = 1; i <= 8; i++)
					SpawnInvaders();
			}
			else
				EndScript();

		END_ENTRY

		DoEvery("SpawnPortals", VarGetFraction("PortalSpawnTime"), SpawnPortals);
		DoEvery("SpawnInvaders", VarGetFraction("InvaderSpawnTime"), SpawnInvaders);

	END_STATES

	ON_SIGNAL("SpawnMore")
		for (i = 1; i <= 2; i++)
            SpawnPortals();

		for (i = 1; i <= 4; i++)
			SpawnInvaders();
	END_SIGNAL
		
	ON_SIGNAL("SpawnAtEveryLocation")
	{
		int portalCount = VarGetArrayCount("Markers");
		VarSetNumber("PortalCount", portalCount);

		for (i = 1; i <= portalCount; i++)
            SpawnPortals();

		for (i = 1; i <= portalCount; i++)
			SpawnInvaders();
	}
	END_SIGNAL

	ON_SIGNAL("End")
		Kill("Portals", 0);
		VarSetTrue("DontSpawnPortals");
	END_SIGNAL

	ON_STOPSIGNAL
		Kill("Portals", 0);
		Kill("Invaders", 0 );
	END_SIGNAL
}

void GenericInvasionInit()
{
	SetScriptName( "GenericInvasion" );
	SetScriptFunc( GenericInvasion );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End", "End" );		
	SetScriptSignal( "Spawn All", "SpawnAtEveryLocation" );		
	SetScriptSignal( "Spawn More", "SpawnMore" );		

	SetScriptVar( "Markers",								"",			0 );
	SetScriptVar( "PortalCount",							"",			0 );
	SetScriptVar( "InvaderCount",							"",			0 );
	SetScriptVar( "PortalSpawnTime",						"",			0 );
	SetScriptVar( "InvaderSpawnTime",						"",			0 );
	SetScriptVar( "SpawnDirectory",							"",			0 );
	SetScriptVar( "PriorityList",							"",			0 );
	SetScriptVar( "SpawnLayout",							"Around",	0 );

}
