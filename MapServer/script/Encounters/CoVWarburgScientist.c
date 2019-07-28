// Encounter Script
//
#include "scriptutil.h"

enum {
	COV_WARBURGSCIENTIST_PAGE_HELLO = 1,
	COV_WARBURGSCIENTIST_PAGE_LEAVE = 3,
	COV_WARBURGSCIENTIST_PAGE_ACCEPTRESCUE,
	COV_WARBURGSCIENTIST_PAGE_FREE,
};

int CoVWarburgScientistOnClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{ 
	// check to see if this contact is done...
	if (StringEqual(GET_STATE, "RescueComplete") || StringEqual(GET_STATE, "CleanUp") || StringEqual(GET_STATE, "EndScript")) {
		return 0;
	}

	switch (link)
	{
	case COV_WARBURGSCIENTIST_PAGE_HELLO:	// HELLO

		// don't say anything after being rescued
		if (StringEqual(GET_STATE, "RescueComplete")) {
			return 0;
		}

		sprintf(response->dialog, getContactHeadshotScriptSMF(target, player)); 

		// check to see if the encounter has been defeated
		if (StringEqual(GET_STATE, "Hostage")) {
			// I haven't been rescued yet
			strcat(response->dialog, StringLocalizeWithVars(VarGet("SaveMeText"), player));
		} else { 
			// I've been rescued
			if (StringEmpty(VarGet("Rescuer")))
			{
				// I'm not following someone
				if (EntityHasRewardToken(player, "WarburgScientist"))
				{
					// but you already have someone
					strcat(response->dialog, StringLocalizeWithVars(VarGet("AnotherText"), player));
				} else {
					// you don't have someone so I can follow you if you'd like
					strcat(response->dialog, StringLocalizeWithVars(VarGet("RescueInfoText"), player));
					AddResponse(response, StringLocalizeWithVars(VarGet("RescueText"), player), COV_WARBURGSCIENTIST_PAGE_ACCEPTRESCUE);
				}
			} else {
				// I'm already following someone
				if (StringEqual(VarGet("Rescuer"), player))
				{
					// I'm following you, so I can stop following if you'd like
					strcat(response->dialog, StringLocalizeWithVars(VarGet("FreeInfoText"), player));
					AddResponse(response, StringLocalizeWithVars(VarGet("FreeText"), player), COV_WARBURGSCIENTIST_PAGE_FREE);
				} else {
					// I'm not following you, so go away
					strcat(response->dialog, StringLocalizeWithVars(VarGet("FollowingAnotherText"), player));
				}
			}
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("ExitText"), player), COV_WARBURGSCIENTIST_PAGE_LEAVE);
		return 1;
	case COV_WARBURGSCIENTIST_PAGE_ACCEPTRESCUE:
		VarSet("Rescuer", player);
		EntityGrantRewardToken(VarGet("Rescuer"), "WarburgScientist", 1);
		SET_STATE("Rescued");

		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
		strcat(response->dialog, StringLocalizeWithVars(VarGet("RescueAcceptText"), player));
		
		AddResponse(response, StringLocalizeWithVars(VarGet("ExitText"), player), COV_WARBURGSCIENTIST_PAGE_LEAVE);
		return 1;
	case COV_WARBURGSCIENTIST_PAGE_FREE:
		SET_STATE("Released");

		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
		strcat(response->dialog, StringLocalizeWithVars(VarGet("FreeAcceptText"), player));
		
		AddResponse(response, StringLocalizeWithVars(VarGet("ExitText"), player), COV_WARBURGSCIENTIST_PAGE_LEAVE);
	default:
		return 0;
	}
}

void CoVWarburgScientist(void)
{  
	INITIAL_STATE 
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			VarSet( "Rescuee", ActorFromEncounterPos(MyEncounter(), VarGetNumber( "PositionOfRescuee" ) ) );

			//TO DO IF No rescuee here, then abort the script, succeed and throw an error
			if( !EntityExists( VarGet( "Rescuee" ) ) )
			{
				SET_STATE("EndScript");
			}
			else
			{
				OnScriptedContactInteract(CoVWarburgScientistOnClick, VarGet("Rescuee"));
				AddAIBehaviorPermFlag(VarGet("Rescuee"), "DoNotDrawAggro");
				AddAIBehaviorPermFlag(VarGet("Rescuee"), "FindTarget(0)");
				ScriptControlsCompletion( MyEncounter() );
				SET_STATE("Hostage");

			}

			//////////////////////////////////////////////////////////////////////////
			// find count of the bunker locations
			//////////////////////////////////////////////////////////////////////////
			{
				int count = 0,
					finished = false;
				char buf[25];

				while (!finished)
				{
					sprintf(buf, "marker:Bunker%d", count+1);
 
					if (!LocationExists(buf)) 
					{
						finished = true;
					} else {
						count++;
					}
				}

				if (count > 0) {
					VarSetNumber("BunkerCount", count);				
				} else {
					EndScript();
				}
			}

		}
		END_ENTRY
	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("Hostage") ////////////////////////////////////////////////////////
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
		} 
		END_ENTRY

		//I die, we're done
		if( !EntityExists( VarGet( "Rescuee" ) ) )
		{
			SetEncounterComplete(MyEncounter(), 0);
			SET_STATE( "EndScript" );
		}

		//If I'm alive and everybody else is dead, make with the delivery part of the script
		else if( NumEntitiesInTeam( MyEncounter() ) == 1 && GetHealth( VarGet( "Rescuee" ) ) > 0.0 ) 
		{

			AddAIBehaviorPermFlag(VarGet("Rescuee"), "DoNotDrawAggro(0)");
			SetChatObjective( VarGet("Rescuee"), VarGet( "PlaceToTakeRescuee" ) ); //if the place is an entity (door, objective or person, set it )
			SET_STATE("WaitingToBeClicked");
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("Released") ////////////////////////////////////////////////////////
	{
		if (!StringEqual(VarGet("Rescuer"), ""))
			EntityRemoveRewardToken(VarGet("Rescuer"), "WarburgScientist");

		Say( VarGet("Rescuee"), VarGet( "SayOnStranded" ), 2 ); 
		ClearWaypoint(VarGet("Rescuer"), VarGetNumber("BunkerWaypoint"));

		SET_STATE("WaitingToBeClicked");
	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("WaitingToBeClicked") ////////////////////////////////////////////////////////
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			VarSet("Rescuer", 0);
			FollowerFollowsLeader( VarGet("Rescuee"), VarGetFraction( "ControlRadius" ) );
		} 
		END_ENTRY
		// Do Nothing
	}


	////////////////////////////////////////////////////////////////////////////////////////
	STATE("Rescued") ////////////////////////////////////////////////////////
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			int waypointID;

			RemoveAllBehaviors(VarGet("Rescuee"));
			AddAIBehavior(VarGet("Rescuee"), "Pet.Follow" );

			// find location to take scientist to 
			if ((waypointID = VarGetNumber("BunkerWaypoint")) == 0) {
				char buf[50];
				sprintf(buf, "marker:bunker%d", RandomNumber(1, VarGetNumber("BunkerCount")));
				waypointID = CreateWaypoint(buf, VarGet("WayPointText"), NULL, NULL, true, false, -1.0f);
				VarSetNumber("BunkerWaypoint", waypointID);
				VarSet( "PlaceToTakeRescuee", buf );
			} 
			SetWaypoint(VarGet("Rescuer"), waypointID);
		} 
		END_ENTRY
 
		//Rescuee can be revived, so wait till he's gone to fail the objective
		if( !EntityExists( VarGet( "Rescuee" ) ) )
		{
			//If the delivery is what succeeds the encounterf
			if( !VarGetNumber( "EncounterAlreadyCompleted" ) );
				SetEncounterComplete(MyEncounter(), 0);
			SET_STATE( "EndScript" );
		}

		// check to make sure the person I'm following hasn't left
		if(!StringEqual(VarGet("Rescuer"), "") && !EntityExists( VarGet( "Rescuer" ) ) )
		{
			SET_STATE( "Released" );
		}

		//Look to see if you are at your final location (if the variable doesn't exist, instantly succeed
		//(added Door with name to point in area so Mission Return Works automatically and location:any)

		//TO DO : If No Rescue is needed, succeed the encounter, but don't clean it up
		if( VarGet( "PlaceToTakeRescuee" ) && !StringEqual( VarGet( "PlaceToTakeRescuee" ), "None" ) )
		{
			if( !StringEqual( VarGet( "PlaceToTakeRescuee" ), "" ) && (
					StringEqual( VarGet( "PlaceToTakeRescuee" ), "CurrentLocation" ) || 
					EntityInArea( VarGet( "Rescuee" ), VarGet( "PlaceToTakeRescuee" ) ) ) )
				SET_STATE( "RescueComplete" );
		}

		//Otherwise follow your regular rules
		FollowerFollowsLeader( VarGet("Rescuee"), VarGetFraction( "ControlRadius" ) );
 
	}


	////////////////////////////////////////////////////////////////////////////////////////
	STATE( "RescueComplete" ) ////////////////////////////////////////////////////////
	{

		SetFollowers(TEAM_NONE, VarGet( "Rescuee" ) ); //Don't be anyone's follower anymore 

		//Another chance to aquire your objective
		SetChatObjective( VarGet( "Rescuee" ), VarGet( "PlaceToTakeRescuee" ) ); //if the place is an entity (door, objective or person, set it )
		SetAIPriorityList( VarGet( "Rescuee" ), VarGet( "PL_OnArrival" ) );
		
		EntityRemoveRewardToken(VarGet("Rescuer"), "WarburgScientist");

		// award token
		if (EntityHasRewardToken(VarGet("Rescuer"), "WarburgCode2"))
		{
			Say( VarGet( "Rescuee" ), VarGet( "SayOnArrival2" ), 2 );
			EntityGrantRewardToken(VarGet("Rescuer"), "WarburgCode3", 2);

			SendPlayerDialog(VarGet("Rescuer"), StringLocalize(VarGet("HaveAllThreeFragments")));
		} 
		else if (EntityHasRewardToken(VarGet("Rescuer"), "WarburgCode1"))
		{
			Say( VarGet( "Rescuee" ), VarGet( "SayOnArrival" ), 2 );
			EntityGrantRewardToken(VarGet("Rescuer"), "WarburgCode2", 1);
		} 
		else
		{
			Say( VarGet( "Rescuee" ), VarGet( "SayOnArrival" ), 2 );
			EntityGrantRewardToken(VarGet("Rescuer"), "WarburgCode1", 1);
		} 

		SetEncounterComplete( MyEncounter(), 1 );
		SET_STATE( "CleanUp" );
	}


	////////////////////////////////////////////////////////////////////////////////////////
	STATE( "CleanUp" ) ////////////////////////////////////////////////////////
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			DestroyWaypoint( VarGetNumber("BunkerWaypoint") );
		}
		END_ENTRY

		if (DoEvery("CleanUp", 1, NULL))
		{
			SET_STATE( "EndScript" );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE( "EndScript" ) ////////////////////////////////////////////////////////
	{
		CloseMyEncounterAndEndScript();
		DestroyWaypoint(VarGetNumber("BunkerWaypoint"));
	}

	END_STATES
}

void CoVWarburgScientistInit()
{
	SetScriptName( "CoVWarburgScientist" );
	SetScriptFunc( CoVWarburgScientist );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "LookForLeaderInRadius",			"30",							0 ); //radius to look for a new leader
	SetScriptVar( "ControlRadius",					"200",							0 ); //radius leader must be in the follow 
	SetScriptVar( "PositionOfRescuee",				0,								0 );

	SetScriptVar( "PL_OnRescue",					0,								0 ); 
	SetScriptVar( "PL_OnStranded",					0,								0 ); 
	SetScriptVar( "PL_OnArrival",					0,								0 ); 
	SetScriptVar( "SayOnArrival",					0,								SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "SayOnArrival2",					0,								SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "SayOnStranded",					0,								SP_MULTIVALUE | SP_OPTIONAL );

	SetScriptVar( "ExitText",						"Leave",						0 ); 
	SetScriptVar( "RescueText",						"Follow Me",					0 ); 
	SetScriptVar( "FreeText",						"Stop Following Me",			0 ); 
	SetScriptVar( "WayPointText",					"DefaultRescueWaypointText ",	0); 
	SetScriptVar( "SaveMeText",						NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 
	SetScriptVar( "RescueInfoText",					NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 
	SetScriptVar( "RescueAcceptText",				NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 
	SetScriptVar( "FreeInfoText",					NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 
	SetScriptVar( "FreeAcceptText",					NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 
	SetScriptVar( "AnotherText",					NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 
	SetScriptVar( "FollowingAnotherText",			NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 
	SetScriptVar( "HaveAllThreeFragments",			NULL,							SP_MULTIVALUE | SP_OPTIONAL ); 

}