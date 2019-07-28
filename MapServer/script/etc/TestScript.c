// ZONE SCRIPT

// This script is just intended to test the scripting system. 
// It sends a lot of output to the debug console.

#include "scriptutil.h"

static void heartBeat(void)
{
	DebugString(VarGet("HeartbeatText"));
}

static void playerEntersMap(ENTITY player)
{
	DebugString(StringAdd(EntityName(player), " entering map"));
}

static void playerExitsMap(ENTITY player)
{
	DebugString(StringAdd(EntityName(player), " exiting map"));
}

static int libertyTouched(ENTITY player, ENTITY liberty)
{
	DebugString(StringAdd(EntityName(player), " touched Ms Liberty"));
	if (RandomNumber(0, 1))
	{
		Say(liberty, StringAdd(StringAdd("Don't touch me, ", EntityName(player)) ,"!<em>Stay away!"), 0);
		DebugString("And I didn't let them continue");
		return 1;
	}
	return 0;
}

void TestScript(void)
{
	int i, n;

	INITIAL_STATE
		//SET_STATE("Ping") 
		VarSetNumber("Count",  1);

	STATE("Ping")
		ON_ENTRY 
			DebugString("Ping-");
			VarSet("HeartbeatText", VarGet("PingText"));
		END_ENTRY

		DoEvery("Heartbeat", 0.01f, heartBeat);
		if (STATE_MINS > 0.25)
			SET_STATE("Pong")

	STATE("Pong")
		ON_ENTRY
			DebugString("Pong-");
			VarSet("HeartbeatText", VarGet("PongText"));
		END_ENTRY
		
		DoEvery("Heartbeat", 0.1f, heartBeat);
		if (STATE_MINS > 0.25)
		{
			if (0)
				SET_STATE("Ping")
			else
				EndScript();
		}

	END_STATES

	ON_SIGNAL("TestStrings")
		if (VarIsEmpty("TestString"))
			VarSet("TestString", "Test");
		VarSet("TestString", StringAdd(VarGet("TestString"), VarGet("TestString")));
		VarSet("TestString", StringAdd(VarGet("TestString"), VarGet("TestString")));
		VarSet("TestString", StringAdd(VarGet("TestString"), VarGet("TestString")));
	END_SIGNAL

	ON_SIGNAL("AITeam")
		STRING team;

		n = VarGetNumber("NumEnts");
		for (i = 1; i <= n; i++)
		{
			VarSet(StringAdd("Ent", NumberToString(i)), "");
		}

		VarSetNumber("CurrentTeam", VarGetNumber("CurrentTeam")+1);
		team = StringAdd("aiteam:", VarGet("CurrentTeam"));
		n = NumEntitiesInTeam(team);
		VarSetNumber("NumEnts", n);
		for (i = 1; i <= n; i++)
		{
			VarSet(StringAdd("Ent", NumberToString(i)), GetEntityFromTeam(team, i));
		}
	END_SIGNAL

	ON_SIGNAL("Hi")
		Spawn(1, "TestTeam", "SpawnDefs/City_01_01/Mugging_Thugs_L1_3_V0.spawndef", "EncounterGroup:TestGroup", NULL, 0,0);
		DetachSpawn("TestTeam");
		BroadcastAttentionMessage("Hello back!",0, 0);
	END_SIGNAL

	ON_SIGNAL("PrintLocation")
		DebugString("MyLocation() = ");
		DebugString(MyLocation());
	END_SIGNAL

	ON_SIGNAL("SetWaypoint")
//		SetWaypoint(ALL_PLAYERS, "marker:ShadowShardInvasion_01", "Script Waypoint!");
	END_SIGNAL

	ON_SIGNAL("ClearWaypoint")
//		ClearWaypoint(ALL_PLAYERS);
	END_SIGNAL

	ON_SIGNAL("Make5th") 
		CreateVillain( "5th", "Event_Sailor_Ghost_Boss", 0, "MonkeyBoy", "coord:-897,543,-619" );
	END_SIGNAL

	ON_SIGNAL("Kill5th")
		Kill( "5th", 0 );  //Get rid of the heaps
	END_SIGNAL

	ON_SIGNAL("SetHandlers")
		OnPlayerEnteringMap(playerEntersMap);
		OnPlayerExitingMap(playerExitsMap);
		OnPlayerInteractWithNPCOrGlowie(libertyTouched, "pnpc:MsLiberty");
	END_SIGNAL
}