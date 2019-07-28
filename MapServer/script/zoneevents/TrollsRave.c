// LOCATION SCRIPT

// This is a location script that runs the Trolls Rave event
//


// Teams in this script
//			Police			- Police baracade spawns
//			RunningTrolls	- Trolls running to rave location
//			Trolls			- Trolls outside the rave location
//			EnteringTrolls	- Trolls running into rave door
//			ExitingTrolls	- Trolls running out of rave door
//			BuffedTrolls	- Supatrolls running to riot location
//			WanderTrolls	- SupaTrolls that have made it to the riot location

#include "scriptutil.h"


#define CLEANUP_BEHAVIOR	"RunIntoDoor"
#define MOVETO_VARIANCE		30.0f

// called when the player leaves the map
int TrollsRaveOnLeaveMap(ENTITY player)
{
	ClearWaypoint(player, VarGetNumber("Waypoint"));

	ClearAllWaypoints(player);

	return 0;
}

// called when the player enters the map
int TrollsRaveOnEnterMap(ENTITY player)
{
	// removing old waypoints
	ClearAllWaypoints(player);

	SetWaypoint(player, VarGetNumber("Waypoint"));

	return 0;
}


void TrollsRave(void)
{  
	ENCOUNTERGROUP group = "first"; 
	ENCOUNTERGROUP groupTarget = "first"; 
	int i;
	int groupNumber;
	int currentTrolls;
	int runningTrolls, standingTrolls;
	int waypointID;

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (!InstanceVarIsEmpty("ZoneEventMutex"))
			{
				EndScript();
				return;
			}

			InstanceVarSet("ZoneEventMutex", "TrollRave");

			// Spawn police
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				300, //Within 300 feet of this encounter
				"TrollsRavePolice", 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Police", VarGet("PoliceSpawn"), group, "TrollsRavePolice", VarGetNumber("SpawnLevel"), 0 );  

				VarSet("PoliceCaptain", ActorFromEncounterPos(group, 1));

				ConditionOnClick(ActorFromEncounterPos(group, 1), ONCLICK_HAS_BADGE, "TrollsRaveRoundup",  VarGet("PoliceSpeech2"), VarGet("WatergunReward"));
				ConditionOnClick(ActorFromEncounterPos(group, 1), ONCLICK_DEFAULT, NULL, VarGet("PoliceSpeech"),  NULL);
			}
			////End spawn police

			VarSetNumber( "CurrentTrolls", 0);

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

			// create icon
			waypointID = CreateWaypoint(MyLocation(), VarGet("WaypointLocationName"), "map_enticon_mission_a", "map_enticon_mission_b", false, true, -1.0f);
			VarSetNumber( "Waypoint", waypointID);

			// add icon to all players on map
			SetWaypoint(ALL_PLAYERS, waypointID);

			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			OnPlayerExitingMap( TrollsRaveOnLeaveMap );
			OnPlayerEnteringMap( TrollsRaveOnEnterMap );


			SET_STATE("TrollsRave");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////
	STATE("TrollsRave") ////////////////////////////////////////////////////////

	currentTrolls = NumEntitiesInTeam( "Trolls" ) + NumEntitiesInTeam( "RunningTrolls" );
	VarSetNumber( "TotalTrolls", (VarGetNumber( "TotalTrolls") - (VarGetNumber( "CurrentTrolls") - currentTrolls)));



	if( DoEvery("RunningCheckTime", 0.10f, 0 ) ) 
	{ 
		// If there are any running trolls that made it to the location, move them to the stationary trolls
		runningTrolls = NumEntitiesInTeam( "RunningTrolls" );
		standingTrolls = NumEntitiesInTeam( "Trolls" );

		groupNumber = NumEntitiesInTeam( "RunningTrolls" );
		for( i = 1 ; i <= groupNumber ; i++ )
		{
			ENTITY e = GetEntityFromTeam("RunningTrolls", i);
			if (GetAIReachedObjective(e))
			{
				MarkAIReachedObjective(e);
				SwitchScriptTeam(e, "RunningTrolls", "Trolls");

				AddAIBehavior(e, "DoNothing(AnimList(DanceRave),CombatOverride(Aggressive))");

				runningTrolls = NumEntitiesInTeam( "RunningTrolls" );
				standingTrolls = NumEntitiesInTeam( "Trolls" );

				groupNumber = NumEntitiesInTeam( "RunningTrolls" );
				i--;
			}
		}
		VarSetNumber( "Trolls", NumEntitiesInTeam( "Trolls" ) );

		// checking buffed trolls
		groupNumber = NumEntitiesInTeam( "BuffedTrolls" );
		for( i = 1 ; i <= groupNumber ; i++ )
		{
			ENTITY e = GetEntityFromTeam("BuffedTrolls", i);
			if (GetAIReachedObjective(e))
			{
				MarkAIReachedObjective(e);
				SwitchScriptTeam(e, "BuffedTrolls", "WanderTrolls");

				AddAIBehavior(e, "Wander(CombatOverride(Aggressive))");

				groupNumber = NumEntitiesInTeam( "BuffedTrolls" );
				i--;
			}
		}
		VarSetNumber( "BuffedTrolls", NumEntitiesInTeam( "BuffedTrolls" ) );
		VarSetNumber( "WanderTrolls", NumEntitiesInTeam( "WanderTrolls" ) );


	}

	if( DoEvery("RespawnCheckTime", VarGetFraction("RespawnCheckTime"), 0 ) ) 
	{ 
		// Spawn more trolls
		while (	currentTrolls < VarGetNumber( "MaxTrolls") &&
				VarGetNumber( "TotalTrolls") > 0 &&
				stricmp(group, "location:none") != 0)
		{

			// Spawn More Trolls using distant arounds
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				300, 
				600, //Between 300 and 900 feet of this encounter
				"Around", 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "RunningTrolls", VarGet("TrollsSpawnFar"), group, "Around", VarGetNumber("SpawnLevel"), 0 );  
			
				groupNumber = NumEntitiesInTeam( group );
				for( i = 1 ; i <= groupNumber ; i++ )
				{
					ENTITY e = GetEntityFromTeam(group, i);
					AddAIBehavior(e, "Combat");
					SetAIMoveTarget( e, MyLocation(), LOW_PRIORITY, 0, 0, MOVETO_VARIANCE );
				}

				DetachSpawn( "RunningTrolls" );
			}

			currentTrolls = NumEntitiesInTeam( "Trolls" ) + NumEntitiesInTeam( "RunningTrolls" );
		}

		// check to see if we're out of trolls
		if (VarGetNumber( "TotalTrolls") <= 0 && currentTrolls <= VarGetNumber( "MaxTrolls"))
		{
			SET_STATE("OutOfTrolls");
		}

		// check to see if we should move some trolls into the rave
		if (NumEntitiesInTeam( "Trolls" ) >  VarGetNumber( "MinTrollCrowd") || 
			VarGetNumber( "TotalTrolls") <= 0) 
		{
			int num = rule30Rand() % 3 + 1;
			
			for (i = 1; i <= num; i++)
			{
				ENTITY e = GetEntityFromTeam("Trolls", 1);

				SwitchScriptTeam(e, "Trolls", "EnteringTrolls");
				AddAIBehavior(e, CLEANUP_BEHAVIOR);
			}

			VarSetNumber( "EnteringTrolls", VarGetNumber( "EnteringTrolls" ) + num );

		}

	} // End respawn check time

	if( DoEvery("ExitingTrollCheckTime", VarGetFraction("RespawnCheckTime") * 0.55f, 0 ) ) 
	{ 
		// check to see if we need to spawn some buffed trolls

		if (VarGetNumber( "EnteringTrolls" ) > 0) {
			int num = rule30Rand() % 3 + 1;
			
			for (i = 1; i <= num; i++)
			{
				ENTITY e = CreateVillain("ExitingTrolls", VarGet("BossName"), VarGetNumber("SpawnLevel"), NULL, MyLocation());
				AddAIBehavior(e, "RunOutOfDoor");
			}

			VarSetNumber( "EnteringTrolls", VarGetNumber( "EnteringTrolls" ) - num );
			VarSetNumber( "ExitingTrolls", NumEntitiesInTeam( "ExitingTrolls" ) );
		}
	}


	if( DoEvery("BuffTrollCheckTime", VarGetFraction("RespawnCheckTime") * 0.42f, 0 ) ) 
	{ 
		// check to see if we need to move some buffed trolls

		if (NumEntitiesInTeam( "ExitingTrolls" ) > 0) {
			int num = rule30Rand() % 3 + 1;
			
			for (i = 1; i <= num; i++)
			{
				ENTITY e = GetEntityFromTeam("ExitingTrolls", 1);

				// need to select target location
				AddAIBehavior(e, "Combat");
				SetAIMoveTarget( e, VarGet("RunToLocation"), LOW_PRIORITY, 0, 0, MOVETO_VARIANCE );
				SwitchScriptTeam(e, "ExitingTrolls", "BuffedTrolls");
			}

			VarSetNumber( "ExitingTrolls", NumEntitiesInTeam( "ExitingTrolls" ));
			VarSetNumber( "BuffedTrolls", NumEntitiesInTeam( "BuffedTrolls" ) );
		}
	}

 
	VarSetNumber( "CurrentTrolls", currentTrolls);
	VarSetNumber( "RunningTrolls", NumEntitiesInTeam( "RunningTrolls" ));


	//////////////////////////////////////////////////////////////////////////////////
	STATE("OutOfTrolls") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
				AddAIBehavior("Trolls", CLEANUP_BEHAVIOR);
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

	// check to see if they've gotten all the trolls
	if (NumEntitiesInTeam( "Trolls" ) <= 0)
	{
		SET_STATE("EventEnd");
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventEnd") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
		// Give Ph4t L00t
		/* No reward for this event
		if (!stricmp(VarGet("Reward"), "")) {
			GiveRewardToAllInRadius( VarGet("Reward"), MyLocation() , VarGetFraction("RewardRadius") );
		}
		*/

		// Yea! Crowd!
		groupNumber = NumEntitiesInTeam( "Police" );
		for( i = 1 ; i <= groupNumber ; i++ )
		{
			Say(GetEntityFromTeam("Crowd", i), VarGet("PoliceCheer"), 0);
		}

		ConditionOnClickClear( VarGet("PoliceCaptain") );

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	if( DoEvery("TimeTillCleanup", VarGetFraction("TimeToCleanupWin"), 0 ) ) 
	{
		SET_STATE("EventCleanup");
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventCleanup") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			AddAIBehavior("RunningTrolls", CLEANUP_BEHAVIOR);
			AddAIBehavior("Trolls", CLEANUP_BEHAVIOR);
			AddAIBehavior("EnteringTrolls", CLEANUP_BEHAVIOR);
			AddAIBehavior("ExitingTrolls", CLEANUP_BEHAVIOR);
			AddAIBehavior("BuffedTrolls", CLEANUP_BEHAVIOR);
			AddAIBehavior("WanderTrolls", CLEANUP_BEHAVIOR);

			groupNumber = NumEntitiesInTeam( "Police" );
			for( i = 1 ; i <= groupNumber ; i++ )
			{
				ENTITY ent = GetEntityFromTeam("Police", i);
				if (!IsEntityUntargetable(ent))
					AddAIBehavior(ent, CLEANUP_BEHAVIOR);
			}


			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			OnPlayerExitingMap( NULL );
			OnPlayerEnteringMap( NULL );

			// remove icon
			DestroyWaypoint(VarGetNumber("Waypoint"));

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	if( DoEvery("TimeTillEnd", VarGetFraction("TimeToCleanupWin"), 0 ) ) 
	{
		SET_STATE("EventShutdown");
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		Kill("RunningTrolls", false);
		Kill("Trolls", false);
		Kill("EnteringTrolls", false);
		Kill("ExitingTrolls", false);
		Kill("BuffedTrolls", false);
		Kill("WanderTrolls", false);
		Kill("Police", false);

		// send alert
		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("CompleteMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));

		InstanceVarRemove("ZoneEventMutex");

		EndScript();

	END_STATES

	ON_SIGNAL("Cleanup")
		SET_STATE( "EventCleanup" );
	END_SIGNAL

	ON_SIGNAL("Shutdown")
		SET_STATE( "EventShutdown" );
	END_SIGNAL


}