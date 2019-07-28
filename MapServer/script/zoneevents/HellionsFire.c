// LOCATION SCRIPT

// This is an encounter script that runs the Hellions Burning Building event
//

#include "scriptutil.h"


#define CLEANUP_BEHAVIOR	"RunIntoDoor"

// called when the player leaves the map
int HellionsFireOnLeaveMap(ENTITY player)
{
	ClearWaypoint(player, VarGetNumber("Waypoint"));

	ClearAllWaypoints(player);

	return 0;
}

// called when the player enters the map
int HellionsFireOnEnterMap(ENTITY player)
{
	// removing old waypoints
	ClearAllWaypoints(player);

	SetWaypoint(player, VarGetNumber("Waypoint"));

	return 0;
}

void HellionsFire(void)
{  
	ENCOUNTERGROUP group = "first"; 
	ENCOUNTERGROUP groupTarget = "first"; 
	int i;
	int groupNumber;
	int currentHellions;
	int buildingHealth = VarGetNumber( "BuildingHealth");
	STRING fireSpawn = "none";
	STRING fireSpawnLoc = "none";
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

			InstanceVarSet("ZoneEventMutex", "Hellions Fire");

			// Spawn crowd
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				300, //Within 300 feet of this encounter
				"FireCrowd", 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Crowd", "scripts.loc/SpawnDefs/ZoneEvents/Expansion5/Crowd_D01_V0.spawndef", 
						group, "FireCrowd", VarGetNumber("SpawnLevel"), 0 );  

				VarSet("PoliceEntity", ActorFromEncounterPos(group, 1));

				ConditionOnClick(ActorFromEncounterPos(group, 1), ONCLICK_HAS_BADGE, "HellionsFireSilver",  VarGet("PoliceSpeech3"), VarGet("WatergunRewardAdv"));
				ConditionOnClick(ActorFromEncounterPos(group, 1), ONCLICK_HAS_BADGE, "HellionsFireBronze",  VarGet("PoliceSpeech2"), VarGet("WatergunReward"));
				ConditionOnClick(ActorFromEncounterPos(group, 1), ONCLICK_DEFAULT, NULL, VarGet("PoliceSpeech"),  VarGet("WatergunReward"));
			}

			////End spawn crowd


			// Spawn ground hellions
			while ( NumEntitiesInTeam( "Hellions" ) < VarGetNumber( "MaxHellions" ) &&
					stricmp(group, "location:none") != 0)
			{

				group = FindEncounterGroupByRadius( 
					MyLocation(),
					0, 
					300, //Within 300 feet of this encounter
					"FireHellionsGround", 
					1, 
 					0 );  

				if (stricmp(group, "location:none") != 0)
					Spawn( 1, "Hellions", "scripts.loc/SpawnDefs/ZoneEvents/Expansion5/HellionsGround_D01_V0.spawndef", 
							group, "FireHellionsGround", VarGetNumber("SpawnLevel"), 0 );  

	//			DetachSpawn( "Hellions" ); 

			}
			////End spawn hellions

			// Spawn wall fires
			while ( NumEntitiesInTeam( "Fires" ) < VarGetNumber( "MaxFires" )  &&
					stricmp(group, "location:none") != 0)
			{
				int percent = rule30Rand() % 100;

				if (percent < 25) {
					fireSpawn = "FireSpawnRoof";
					fireSpawnLoc = "FireRoof";
				} else {
					fireSpawn = "FireSpawnWall";
					fireSpawnLoc = "FireWall";
				}

				group = FindEncounterGroupByRadius( 
					MyLocation(),
					0, 
					300, //Within 300 feet of this encounter
					fireSpawnLoc, 
					1, 
 					0 );  

				if (stricmp(group, "location:none") != 0)
					Spawn( 1, "Fires", VarGet(fireSpawn), group, fireSpawnLoc, VarGetNumber("SpawnLevel"), 0 );  

			}
			////End spawn wall fires

			// setting up some basic variables
			VarSetNumber( "CurrentHellions", NumEntitiesInTeam( "Hellions" ));
			VarSetNumber( "TotalHellions", (VarGetNumber( "TotalHellions") - VarGetNumber( "CurrentHellions")));

			VarSetNumber( "BuildingID", DynamicGeometryFind("BurningBuilding", MyLocation(), -1));
			VarSetFraction( "StartingBuildingHealth", (FRACTION) buildingHealth);
		
			// create icon
			waypointID = CreateWaypoint(MyLocation(), VarGet("WaypointLocationName"), "map_enticon_mission_a", "map_enticon_mission_b", false, true, -1.0f);
			VarSetNumber( "Waypoint", waypointID);

			// add icon to all players on map
			SetWaypoint(ALL_PLAYERS, waypointID);

			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			OnPlayerExitingMap( HellionsFireOnLeaveMap );
			OnPlayerEnteringMap( HellionsFireOnEnterMap );

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
									"zonename", StringLocalizeMenuMessages(GetMapName())));

		SET_STATE("HellionsAttack");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////
	STATE("HellionsAttack") ////////////////////////////////////////////////////////

	currentHellions = NumEntitiesInTeam( "Hellions" ) + NumEntitiesInTeam( "HellionsRunning");
	VarSetNumber( "TotalHellions", (VarGetNumber( "TotalHellions") - (VarGetNumber( "CurrentHellions") - currentHellions)));

	if( DoEvery("RespawnCheckTime", VarGetFraction("RespawnCheckTime"), 0 ) ) 
	{

		// Respawn killed hellions
		while (	currentHellions < VarGetNumber( "MaxHellions") &&
				VarGetNumber( "TotalHellions") > 0 &&
				stricmp(group, "location:none") != 0)
		{
			// check to see if we should spawn bosses from door
			int percent = rule30Rand() % 100;

			if (percent <= VarGetNumber( "BossDoorChance"))
			{	
				int number = rule30Rand() % VarGetNumber("BossNumber");
				int i;

				for (i = 0; i < number; i++) {
					ENTITY e = CreateVillain("HellionsRunning", VarGet("BossName"), VarGetNumber("SpawnLevel"), NULL, MyLocation());
					SetAIPriorityList(e, "PL_RunOutOfDoor");
				}
			} else {

				// Spawn More Hellions using distant arounds
				group = FindEncounterGroupByRadius( 
					MyLocation(),
					300, 
					900, //Between 300 and 900 feet of this encounter
					"Around", 
					1, 
 					0 );  

				if (stricmp(group, "location:none") != 0) {
					Spawn( 1, "HellionsRunning", "scripts.loc/SpawnDefs/ZoneEvents/Expansion5/Arounds_D01_V0.spawndef", 
							group, "Around", VarGetNumber("SpawnLevel"), 0 );  


					// finding a place to run to - try to find an original spawn location that is now empty
					groupTarget = FindEncounterGroupByRadius( 
									MyLocation(),
									0, 
									300, //Within 300 feet of this encounter
									"FireHellionsGround", 
									1, 
 									0 );  

					AddAIBehavior(group, "Combat");
					if (stricmp(groupTarget, "location:none") != 0) {
						// found an empty original spawn.  Head there.
						SetAIMoveTarget( group, EncounterPos( groupTarget, 1 ), MEDIUM_PRIORITY, 0, 1, 10.0f );
					} else {
						// Can't find any empty originals, just head to the event marker
						SetAIMoveTarget( group, MyLocation(), MEDIUM_PRIORITY, 0, 1, 10.0f );
					}
					DetachSpawn( "HellionsRunning" );
				}
			}

			currentHellions = NumEntitiesInTeam( "Hellions" ) + NumEntitiesInTeam( "HellionsRunning");
		}

		// check to see if we're out of hellions
		if (VarGetNumber( "TotalHellions") <= 0 && currentHellions <= VarGetNumber( "MaxHellions"))
		{
			SET_STATE("FiresRemaining");
		}

		// Spawn wall fires
		while ( NumEntitiesInTeam( "Fires" ) < VarGetNumber( "MaxFires" )  &&
				stricmp(group, "location:none") != 0)
		{

			int percent = rule30Rand() % 100;

			if (percent < 25) {
				fireSpawn = "FireSpawnRoof";
				fireSpawnLoc = "FireRoof";
			} else {
				fireSpawn = "FireSpawnWall";
				fireSpawnLoc = "FireWall";
			}

			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				300, //Within 300 feet of this encounter
				fireSpawnLoc, 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0)
				Spawn( 1, "Fires", VarGet(fireSpawn), group, fireSpawnLoc, VarGetNumber("SpawnLevel"), 0 );  

		}
		////End spawn wall fires

	} // End respawn check time

	VarSetNumber( "CurrentHellions", currentHellions);

	// apply fire damage
	buildingHealth -= (NumEntitiesInTeam( "Fires" ) * VarGetNumber( "FireDamage"));
	VarSetNumber( "BuildingHealth", buildingHealth);

	if (VarGetNumber( "BuildingID" )) 
	{
		FRACTION percent = (FRACTION) buildingHealth / VarGetFraction("StartingBuildingHealth");
		DynamicGeometrySetHP(VarGetNumber( "BuildingID" ), (NUMBER) (percent * 100.0f), 0);
	}

	// check for building death
	if (buildingHealth <= 0)
	{
		SET_STATE("EventLose");
	}

	// check for other health state transitions

	//////////////////////////////////////////////////////////////////////////////////
	STATE("FiresRemaining") ////////////////////////////////////////////////////////

	// apply fire damage
	buildingHealth -= (NumEntitiesInTeam( "Fires" ) * VarGetNumber( "FireDamage"));
	VarSetNumber( "BuildingHealth", buildingHealth);

	if (VarGetNumber( "BuildingID" ))
		DynamicGeometrySetHP(VarGetNumber( "BuildingID" ), (NUMBER) ((FRACTION) buildingHealth / VarGetFraction("StartBuildingHealth")) * 100, 0);

	// check for building death
	if (buildingHealth <= 0)
	{
		SET_STATE("EventLose");
	}

	// check to see if they've gotten all the fires
	if (NumEntitiesInTeam( "Fires" ) <= 0)
	{
		SET_STATE("EventWin");
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventWin") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
		// Give Ph4t L00t
		GiveRewardToAllInRadius( VarGet("Reward"), MyLocation() , VarGetFraction("RewardRadius") );

		// Yea! Crowd!
		groupNumber = NumEntitiesInTeam( "Crowd" );
		for( i = 1 ; i <= groupNumber ; i++ )
		{
			if (i == 1) {
				Say(GetEntityFromTeam("Crowd", i), VarGet("PoliceCheer"), 0);
			} else {
				Say(GetEntityFromTeam("Crowd", i), VarGet("CrowdCheer"), 0);
			}

		}

		ConditionOnClickClear( VarGet("PoliceEntity") );

		// setting building to win state
		if (VarGetNumber( "BuildingID" ))
			DynamicGeometrySetHP(VarGetNumber( "BuildingID" ), (NUMBER) 100.0f , 1);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	if( DoEvery("TimeTillCleanup", VarGetFraction("TimeToCleanupWin"), 0 ) ) 
	{
		SET_STATE("EventCleanup");
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventLose") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

		// spawn exploding duck
		{
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				300, //Within 300 feet of this encounter
				"FireRoof", 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0)
				Spawn( 1, "Fires", "scripts.loc/SpawnDefs/ZoneEvents/Expansion5/Explosion_D01_V0.spawndef", 
						group, "FireRoof", VarGetNumber("SpawnLevel"), 0 );  

		}

		SetAIPriorityList( "HellionsRunning", CLEANUP_BEHAVIOR ); 
		SetAIPriorityList( "Hellions", "PL_DanceAndFight" ); 
		Kill("Fires", false);

		// Boo! Crowd!
		groupNumber = NumEntitiesInTeam( "Crowd" );
		for( i = 1 ; i <= groupNumber ; i++ )
		{
			if (i == 1) {
				Say(GetEntityFromTeam("Crowd", i), VarGet("PoliceBoo"), 0);
			} else {
				Say(GetEntityFromTeam("Crowd", i), VarGet("CrowdBoo"), 0);
			}

		}
		ConditionOnClickClear( VarGet("PoliceEntity") );

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////



	if( DoEvery("TimeTillCleanup", VarGetFraction("TimeToCleanupLose"), 0 ) ) 
	{
		SET_STATE("EventCleanup");
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventCleanup") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			SetAIPriorityList( "HellionsRunning", CLEANUP_BEHAVIOR ); 
			SetAIPriorityList( "Hellions", CLEANUP_BEHAVIOR ); 
			SetAIPriorityList( "Crowd", CLEANUP_BEHAVIOR ); 
			SetAIPriorityList( "Fires", "PL_DestroyMe" ); 

			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			OnPlayerExitingMap( NULL );
			OnPlayerEnteringMap( NULL );

			// remove icon
			DestroyWaypoint(VarGetNumber("Waypoint"));

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("CompleteMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

	if( DoEvery("TimeTillRevert", VarGetFraction("TimeToRevert"), 0 ) ) 
	{
		SET_STATE("EventEnd");
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventEnd") ////////////////////////////////////////////////////////

	Kill("HellionsRunning", false);
	Kill("Hellions", false);
	Kill("Crowd", false);
	Kill("Fires", false);

	// resetting building back to start
	if (VarGetNumber( "BuildingID" ))
		DynamicGeometrySetHP(VarGetNumber( "BuildingID" ), (NUMBER) 100.0f , 0);

	InstanceVarRemove("ZoneEventMutex");

	EndScript();

	END_STATES

	ON_SIGNAL("WinGame")
		SET_STATE( "EventWin" );
	END_SIGNAL

	ON_SIGNAL("LoseGame")
		SET_STATE( "EventLose" );
	END_SIGNAL


}