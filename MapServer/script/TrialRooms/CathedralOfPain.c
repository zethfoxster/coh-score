// MISSION SCRIPT

//C:\game\data\maps\Missions\unique\Artified\V_Outdoor_Cathedral_of_Pain\V_Outdoor_Cathedral_of_Pain.txt

//C:\game\data\scripts\V_Contacts\BaseMissions

// GetTask CathedralOfPain

// CathedralOfPainContact

//marker:PortalToRularuu
//		   MissionEncounterName name Obelisk1, Obelisk2, Obelisk3, Rularuu
//		   Layout names Obelisk (x3), Rularuu
//		   Rularuu   = EP 1
//		   WillForge = EP 2
//		   Obelisk   = EP 1
//		   WillForge = EP 2

#include "scriptutil.h"

// called when the player leaves the map
int CathedralOfPainOnLeaveMap(ENTITY player)
{
	ClearAllWaypoints(player); // clean up anything that might have been missed

//	if( VarGetNumber("UIId") )
//		ScriptUIEntStopUsing( VarGetNumber("UIId"), player );
	ScriptUIHide("Collection:ObeliskHP", player);
	// DGNOTE 7/21/2010
	// See comment below at call to ScriptUIShow for why this call is no longer needed
	//ScriptUISendDetachCommand(player, 0);
	return 1;
}

// called when the player enters the map (restablish waypoints) 
int CathedralOfPainOnEnterMap(ENTITY player)
{
	if( !VarIsEmpty( "WayPoint" ) )
		SetWaypoint(player, VarGetNumber("WayPoint"));

	//If we are at the time point, set that
//	if( VarGetNumber("UIId") )
//		ScriptUIEntStartUsing( VarGetNumber("UIId"), player );
	// DGNOTE 7/21/2010
	// See comment below at call to ScriptUIShow for why this call is no longer needed
	//ScriptUISendDetachCommand(player, 1);
	return 0;
}


void CathedralOfPain(void)
{
	INITIAL_STATE 
	{
		ON_ENTRY
		{
			ENCOUNTERGROUP obelisk1;
			ENCOUNTERGROUP obelisk2;
			ENCOUNTERGROUP obelisk3;

			SetScriptCombatLevel( VarGetNumber( "CombatLevelToSetAllPLayers" ) );

			obelisk1 = FindEncounter( 0, 0, 0, 0, "Obelisk1", 0, 0, 0 );
			obelisk2 = FindEncounter( 0, 0, 0, 0, "Obelisk2", 0, 0, 0 );
			obelisk3 = FindEncounter( 0, 0, 0, 0, "Obelisk3", 0, 0, 0 );

			SetMissionStatus(VarGet("STATUS_BRING_DOWN_THE_OBELISKS"));

			VarSet( "Obelisk1", obelisk1 );
			VarSet( "Obelisk2", obelisk2 );
			VarSet( "Obelisk3", obelisk3 );

			//CloseDoor( "marker:PortalToRularuu" );
			LockDoor( "marker:PortalToRularuu", VarGet( "PORTAL_TO_RULARUU_DOOR_LOCKED_MESSAGE" ) );

			if( LocationExists( "marker:PortalToRularuu" ) )
			{
				NUMBER waypoint = CreateWaypoint( "marker:PortalToRularuu", VarGet("PortalWayPointText"), NULL, NULL, true, false, -1.0f);
				SetWaypoint( ALL_PLAYERS, waypoint );
				VarSetNumber( "WayPoint", waypoint );
			}

			OnPlayerExitingMap( CathedralOfPainOnLeaveMap );
			OnPlayerEnteringMap( CathedralOfPainOnEnterMap );
		}
		END_ENTRY
		{
			ENTITY ob1 = GetEntityFromTeam( VarGet( "Obelisk1" ), 1 );
			ENTITY ob2 = GetEntityFromTeam( VarGet( "Obelisk2" ), 1 );
			ENTITY ob3 = GetEntityFromTeam( VarGet( "Obelisk3" ), 1 );

			//Wait till obelisks spawn before anything else
			if ( EntityExists( ob1 ) && EntityExists( ob2 ) && EntityExists( ob3 ) )
			{	
				// create the UI
				ScriptUITitle("TitleWidget", VarGet("STATUS_BRING_DOWN_THE_OBELISKS"), "");
				ScriptUIList("ListWidget", "", VarGet("STATUS_BRING_DOWN_THE_OBELISKS"), "ffffffff", " ", "ffffffff");
				ScriptUIMeterEx("ObeliskEnt1", GetDisplayName(ob1), GetDisplayName(ob1), "ObeliskEnt1Var", "100", "Blue", "", "");
				ScriptUIMeterEx("ObeliskEnt2", GetDisplayName(ob2), GetDisplayName(ob2), "ObeliskEnt2Var", "100", "Blue", "", "");
				ScriptUIMeterEx("ObeliskEnt3", GetDisplayName(ob3), GetDisplayName(ob3), "ObeliskEnt3Var", "100", "Blue", "", "");
				ScriptUICreateCollection("ObeliskHP", 5, "TitleWidget", "ListWidget", "ObeliskEnt1", "ObeliskEnt2", "ObeliskEnt3");

				VarSet( "ObeliskEnt1", ob1 );
				VarSet( "ObeliskEnt2", ob2 );
				VarSet( "ObeliskEnt3", ob3 );
				SET_STATE("FightingObelisks");
			}
		}
	}

	STATE( "FightingObelisks" )
	{
		ENTITY ob1 = VarGet( "ObeliskEnt1" ); 
		ENTITY ob2 = VarGet( "ObeliskEnt2" );
		ENTITY ob3 = VarGet( "ObeliskEnt3" );
		FRACTION ob1Hp = GetHealth( ob1 );
		FRACTION ob2Hp = GetHealth( ob2 );
		FRACTION ob3Hp = GetHealth( ob3 );

		// DGNOTE 7/21/2010
		// To avoid problems where legacy scripts sometimes had their script display detached, ScriptUIShow now always
		// forces the display to its attached state.  Therefore we call ScriptUISendDetachCommand(ALL_PLAYERS, 1);
		// immediately after to detach it.
		ScriptUIShow("Collection:ObeliskHP", ALL_PLAYERS);
		ScriptUISendDetachCommand(ALL_PLAYERS, 1);
		VarSetNumber("ObeliskEnt1Var", ob1Hp*100);
		VarSetNumber("ObeliskEnt2Var", ob2Hp*100);
		VarSetNumber("ObeliskEnt3Var", ob3Hp*100);

		if ( !ob1Hp && !ob2Hp && !ob3Hp )
		{
			SetMissionStatus( VarGet( "STATUS_ENTER_THE_PORTAL_AND_DEFEAT_RULARUU" ) );
			SendPlayerAttentionMessage( ALL_PLAYERS, VarGet("ATTENTION_OBELISKS_ARE_ALL_DEFEATED" ) ); 
			//OpenDoor( "marker:PortalToRularuu" );
			UnlockDoor( "marker:PortalToRularuu" );

			//Set All Obelisks to not Respawn
			SetEncounterComplete( VarGet( "Obelisk1" ), 1 );
			SetEncounterComplete( VarGet( "Obelisk2" ), 1 );
			SetEncounterComplete( VarGet( "Obelisk3" ), 1 );
			ScriptUIHide("Collection:ObeliskHP", ALL_PLAYERS);
			SET_STATE("PortalToRularuuOpen");
		}
	}

	STATE("PortalToRularuuOpen")
	{
		ON_ENTRY
		{ 
//			int i;
//			int UIId = 0;

			/// TIMER removed timer
			// set timer
			//TimerSet( "Clock", VarGetFraction( "MinutesToDefeatRularuuAfterObelisksAreDestroyed" ));

			// create UI for timer and display
			//UIId = ScriptUICreate(NULL);
			//VarSetNumber("UIId", UIId);

			//ScriptUIAddTitleBar(UIId, "Title", StringLocalize(VarGet("TimerName")), StringLocalize(VarGet("TimerInfo")));
			//ScriptUIAddScriptTimer(UIId, "Clock", StringLocalize(VarGet("TimerText")),"");

			// check UI
//			for (i = 0; i < NumEntitiesInTeam(ALL_PLAYERS); i++)
//			{	
//				ENTITY targetEnt = GetEntityFromTeam(ALL_PLAYERS, i + 1 );
//				ScriptUIEntStartUsing(VarGetNumber("UIId"), targetEnt);
//			}
		} 
		END_ENTRY

		if ( MissionIsComplete() )
		{
			SET_STATE("MissionComplete");	
		}

//		if( MissionIsComplete() )
//		{
//			int i;
//			for (i = 0; i < NumEntitiesInTeam(ALL_PLAYERS); i++)
//			{	
//				ENTITY targetEnt = GetEntityFromTeam(ALL_PLAYERS, i + 1 );
//				ScriptUIEntStopUsing( VarGetNumber("UIId"), targetEnt);
//			}
//		}

		//if( !MissionIsComplete() && TimerElapsed( "Clock" ) )
		//{
		//	SendPlayerAttentionMessage( ALL_PLAYERS, VarGet("ATTENTION_YOU_FAILED_THE_MISSION" ) ); 
		//	SetMissionComplete( 0 );
		//	SetMissionStatus("");
		//}
	}

	STATE("MissionComplete")
	{
		ON_ENTRY
		{ 
			// grant all players the raid points
			int i;
			for (i = 0; i < NumEntitiesInTeam(ALL_PLAYERS); i++)
			{	
				ENTITY targetEnt = GetEntityFromTeam(ALL_PLAYERS, i + 1 );

				EntityAddRaidPoints(targetEnt, VarGetNumber("RaidPointsPerPlayer"), false);
			}
		}
		END_ENTRY

	}
	END_STATES
}

void CathedralOfPainInit()
{
	SetScriptName( "CathedralOfPain" );
	SetScriptFunc( CathedralOfPain );
	SetScriptType( SCRIPT_MISSION );		

	//marker:PortalToRularuu
	//MissionEncounterName name Obelisk1, Obelisk2, Obelisk3
	
	SetScriptVar( "CombatLevelToSetAllPLayers",						"50",	0			);
	SetScriptVar( "STATUS_BRING_DOWN_THE_OBELISKS",					0,		SP_OPTIONAL );
	SetScriptVar( "PORTAL_TO_RULARUU_DOOR_LOCKED_MESSAGE",			0,		SP_OPTIONAL );
	SetScriptVar( "ATTENTION_OBELISKS_ARE_ALL_DEFEATED",			0,		SP_OPTIONAL );
	SetScriptVar( "MinutesToDefeatRularuuAfterObelisksAreDestroyed",0,		0			);
	SetScriptVar( "TimerName",										0,		0			);
	SetScriptVar( "TimerInfo",										0,		0			);
	SetScriptVar( "TimerText",										0,		0			);
	SetScriptVar( "STATUS_ENTER_THE_PORTAL_AND_DEFEAT_RULARUU",		0,		SP_OPTIONAL );
	SetScriptVar( "PortalWayPointText",								0,		SP_OPTIONAL );
	SetScriptVar( "ATTENTION_YOU_FAILED_THE_MISSION",				0,		SP_OPTIONAL );
	SetScriptVar( "RaidPointsPerPlayer",							"40",	SP_OPTIONAL );
	
}