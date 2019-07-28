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


#include "scriptutil.h"

// don't run on city zones that don't have the marker we expect
static int CorrectCityZone()
{
	return LocationExists("marker:ShadowShardInvasion_01");
}

// look for a Rularuu invasion marker that doesn't have a vine already nearby
static STRING FindFreeVineLocation(void)
{
	static const STRING invasionpoints[] = 
	{
		"marker:ShadowShardInvasion_01",
		"marker:ShadowShardInvasion_02",
		"marker:ShadowShardInvasion_03",
		"marker:ShadowShardInvasion_04",
		"marker:ShadowShardInvasion_05",
		"marker:ShadowShardInvasion_06",
		"marker:ShadowShardInvasion_07",
		"marker:ShadowShardInvasion_08",
		"marker:ShadowShardInvasion_09",
		"marker:ShadowShardInvasion_10",
		"",
	};

	NUMBER i, n, start, firsttime;
	for (i = 0; ; i++)
	{
		if (!invasionpoints[i][0]) break;
	}
	n = i;

	// start at random point and look circularly
	firsttime = 1;
	start = RandomNumber(0, n-1);
	for (i = start; ; i = (i + 1) % n)
	{
		// check and see if any vines are nearby
		if (LocationExists(invasionpoints[i]) && 
			NumEntitiesInArea(invasionpoints[i], "Vines") == 0)
		{
			return invasionpoints[i];
		}

		// make sure we don't loop forever
		if (i == start && !firsttime) return LOCATION_NONE;
		firsttime = 0;
	}
}

// choose one of the vines at random and return its location
static STRING FindExistingVineLocation(void)
{
	NUMBER i, n;
	n = NumEntitiesInTeam("Vines");
	if (n == 0)
		return LOCATION_NONE;

	i = RandomNumber(1, n);
	return PointFromEntity(GetEntityFromTeam("Vines", i));
}

static void SpawnInvaders(void)
{
	if ( NumEntitiesInTeam("Invaders") < 100 )
	{
		STRING location;
		STRING spawndef = "SpawnDefs/ZoneEvents/ShadowShardInvasion/";
		spawndef = StringAdd(spawndef, GetShortMapName());
		spawndef = StringAdd(spawndef, "_Invaders_D1_V0.spawndef");

		location = FindExistingVineLocation();
		if (LocationExists(location))
		{
			DebugString(StringAdd("Spawning invaders at ", location));
			Spawn(1, "Invaders", spawndef, location, NULL, 0,0);
			DetachSpawn("Invaders");
			SetAIPriorityList("Invaders", "PL_Patrol_Group");
		}
	}
}

static void SpawnVines(void)
{
	if (NumEntitiesInTeam("Vines") < VarGetNumber("MaxVines") AND VarIsFalse("DontSpawnVines"))
	{
		STRING location;
		STRING group;
		STRING spawndef = "SpawnDefs/ZoneEvents/ShadowShardInvasion/";
		spawndef = StringAdd(spawndef, GetShortMapName());
		spawndef = StringAdd(spawndef, "_Vine_D1_V0.spawndef");

		location = FindFreeVineLocation();
		if (LocationExists(location))
		{
			group = FindEncounterGroup(location, "Around", 0, 0);
			if (LocationExists(group))
			{
				DebugString(StringAdd("Spawning vine at ", location));
				CloseEncounter(group); // if it's open, just get rid of anyone there
				ReserveEncounter(group); // don't spawn anything else there until I'm done
				Spawn(1, "Vines", spawndef, group, NULL, 0, 0);
				DetachSpawn("Vines");
			}
		}
	}
}


void ShadowShardInvasion(void)
{
	int i;

	INITIAL_STATE

		ON_ENTRY 
			VarSetNumber("MaxVines", 4);
			if (CorrectCityZone())
			{
				for (i = 1; i <= 4; i++)
					SpawnVines();
				for (i = 1; i <= 8; i++)
					SpawnInvaders();
			}
			else
				EndScript();

		END_ENTRY

		DoEvery("SpawnVines", 5, SpawnVines);
		DoEvery("SpawnInvaders", 0.5, SpawnInvaders);

	END_STATES

	ON_SIGNAL("SpawnMore")
		for (i = 1; i <= 2; i++)
            SpawnVines();
		for (i = 1; i <= 4; i++)
			SpawnInvaders();
	END_SIGNAL
		
	ON_SIGNAL("SpawnAtEveryLocation")
		VarSetNumber("MaxVines", 6);
		for (i = 1; i <= 6; i++)
            SpawnVines();
		for (i = 1; i <= 10; i++)
			SpawnInvaders();
	END_SIGNAL

	ON_SIGNAL("End")
		Kill("Vines", 0);
		VarSetTrue("DontSpawnVines");
	END_SIGNAL

	ON_STOPSIGNAL
		Kill("Vines", 0);
		Kill("Invaders", 0 );
	END_SIGNAL
}