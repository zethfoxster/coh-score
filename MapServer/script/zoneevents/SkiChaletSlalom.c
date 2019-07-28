// ZONE SCRIPT
//
// This script will set a map token when it starts and then remove it when it shuts down.

#include "scriptutil.h"

static StringArray playerList;
 
void SkiChaletEndRun(ENTITY player)
{
	STRING timerName = StringAdd("timer", VarGet("RaceNameKey"));
	VarSetEmpty(EntVar(player, "Timer"));
	VarSetEmpty(EntVar(player, timerName));

	ScriptUIHide("Collection:RaceUI", player);
	SendPlayerAttentionMessage(player, VarGet("EndRunFloat"));
	DestroyWaypoint(EntityGetRewardToken(player, "WaypointToken"));
	EntityRemoveRewardToken(player, "WaypointToken");
	EntityRemoveRewardToken(player, "LastWaypointToken");
	EntityRemoveRewardToken(player, "RaceCourseToken");
}

int SkiChaletSlalomIsPlayerOnRun(ENTITY player)
{
	return EntityHasRewardToken(player, "WaypointToken");
}

int SkiChaletSlalomIsPlayerOnThisRace(ENTITY player)
{
	return (StringHash(VarGet("RaceNameKey")) == EntityGetRewardToken(player, "RaceCourseToken"));
}

int SkiChaletSlalomOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	int i, count;
	STRING results = StringLocalize(VarGet("ContactIntro"));

	// If the player is on this race, reset their timer instead of displaying the scoreboard.
	if (SkiChaletSlalomIsPlayerOnThisRace(player))
	{
		SkiChaletEndRun(player);
		return 0;
	}

	switch (link)
	{
	case 1:	// HELLO
//		sprintf(response->dialog,  getContactHeadshotScriptSMF( target, player));

		// best times
		count = VarGetNumber("TopListSize");

		for (i = 0; i < count; i++)
		{	
			if (VarGetFraction(StringAdd("BestTime", NumberToString(i))) < 999.0f)
			{
				if ( i == 0 )
					results = StringAdd(results, StringLocalize(VarGet("ContactHeader")));
				else
					results = StringAdd(results, StringLocalize(VarGet("ResultTable4")));

				results = StringAdd(results, VarGet(StringAdd("BestName", NumberToString(i))));
				results = StringAdd(results, StringLocalize(VarGet("ResultTable5")));
				results = StringAdd(results, FractionTimeToString(VarGetFraction(StringAdd("BestTime", NumberToString(i)))));
				results = StringAdd(results, StringLocalize(VarGet("ResultTable6")));
			}
		}

		results = StringAdd(results, StringLocalize(VarGet("ResultTable7")));

		strcat(response->dialog, results);

		AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);

		return 1;
		break;
	default:
		return 0;
	}

}

static void SkiChaletSlalomSpawnContacts()
{
	ENCOUNTERGROUP group = "first"; 
	int i = 0;
	STRING loc, spawn;
	ENTITY InfoNPC;

	while (stricmp(group, "location:none") != 0)
	{
		loc = VarGetArrayElement("ContactList", i);
		spawn = VarGetArrayElement("ContactSpawnList", i);

		// spawn contact hero one
		group = FindEncounterGroupByRadius( 
			loc,
			0, 
			100, 
			VarGet("ContactLayout"), 
			1, 
			0 );  

		if (stricmp(group, "location:none") != 0) {
			Spawn( 1, "Contact", spawn, group, VarGet("ContactLayout"), 1, 0 );  

			// add on click handler
			InfoNPC = ActorFromEncounterPos( group, VarGetNumber("NPCEncounterPosition")); 

			OnScriptedContactInteract(SkiChaletSlalomOnContactClick, InfoNPC);
			i++;
		}
	}

	VarSetNumber("ContactCount", i);
}

void SkiChaletSlalomUpdateTimes(ENTITY player, FRACTION time)
{
	int i, count = VarGetNumber("TopListSize");
	int found = false;
	STRING var;
	STRING playerName = EntityName(player);
	STRING oldName, oldName2;
	FRACTION oldTime = 999.9f, oldTime2 = 999.9f;

	for (i = 0; i < count; i++)
	{
		FRACTION best = VarGetFraction(StringAdd("BestTime", NumberToString(i)));

		if (!found)
		{
			if (time < best)
			{
				// new best
				found = true;

				var = StringAdd("BestName", NumberToString(i));
				oldName = StringCopy(VarGet(var));
				VarSet(var, playerName);

				var = StringAdd("BestTime", NumberToString(i));
				oldTime = VarGetFraction(var);
				VarSetFraction(var, time);
			} else if (StringEqual(VarGet(StringAdd("BestName", NumberToString(i))), playerName)) {
				return;
			}
		} else {
			if (StringEqual(oldName, playerName))
			{
				return;
			}
			else
			{
				// moving things down
				var = StringAdd("BestName", NumberToString(i));
				oldName2 = StringCopy(VarGet(var));
				VarSet(var, oldName);
				oldName = oldName2;

				var = StringAdd("BestTime", NumberToString(i));
				oldTime2 = VarGetFraction(var);
				VarSetFraction(var, oldTime);
				oldTime = oldTime2;
			}
		}
	}
}

void SkiChaletSlalomAddToList(ENTITY player)
{
	char *pEntString = NULL;
	int idx = StringArrayFind(playerList, player);

	// check to see if they're already on the list
	if (idx == -1)
	{	
		pEntString = (char *) malloc(strlen(player) + 1);
		strcpy(pEntString, player);
		eaInsert(&playerList, pEntString, 0);
	}
}

ENTITY SkiChaletSlalomAddRemoveFromList(ENTITY player)
{
	int idx = StringArrayFind(playerList, player);
	char *pEntString = NULL;
	STRING retval;

	if (idx != -1)
	{	
		pEntString = eaRemove(&playerList, idx);
		if (pEntString) 
		{
			retval = StringAdd(pEntString, "");
			free(pEntString);
		}
	}
	return retval;
}

void SkiChaletSlalomPlayerEntersVolume(ENTITY player, STRING name)
{
	int waypointID; 
	int i;
	int len;

	if (VarIsEmpty("RaceNameKey"))
		return;

	len = strlen(VarGet("RaceNameKey"));
	if (!strnicmp(name, VarGet("RaceNameKey"), len))
	{
		const char *sNumber = &(name[len]);
		int number = atoi(sNumber);

		if (!SkiChaletSlalomIsPlayerOnRun(player) && number == 1)
		{
			/////////////////////////////////////////////////////////
			// START GAME
			STRING markerName = StringAdd("marker:", VarGet("RaceNameKey"));
			STRING timerName = StringAdd("timer", VarGet("RaceNameKey"));
			markerName = StringAdd(markerName, "1");

			SendPlayerAttentionMessage(player, VarGet("StartRunFloat"));
		
			if (EntityHasRewardToken(player, "WaypointToken"))
			{
				DestroyWaypoint(EntityGetRewardToken(player, "WaypointToken"));
			}

			waypointID = CreateWaypoint(markerName, "Gate", NULL, NULL, true, false, -1.0f);
			EntityGrantRewardToken(player, "WaypointToken", waypointID);
			EntityGrantRewardToken(player, "LastWaypointToken", 1);
			EntityGrantRewardToken(player, "RaceCourseToken", StringHash(VarGet("RaceNameKey")));
			SetWaypoint(player, waypointID);

			ScriptUIShow("Collection:RaceUI", player);
			VarSetNumber(EntVar(player, "Timer"), SecondsSince2000());
			VarSetFraction(EntVar(player, timerName), CpuTicksInSeconds());

			SkiChaletSlalomAddToList(player);

		} 
		else if (SkiChaletSlalomIsPlayerOnThisRace(player))
		{
			int lastWaypoint = EntityGetRewardToken(player, "LastWaypointToken");

			if (number == lastWaypoint + 1)
			{
				if (number == VarGetNumber("GateCount"))
				{
					/////////////////////////////////////////////////////////
					// END GAME
					STRING timerName = StringAdd("timer", VarGet("RaceNameKey"));

					FRACTION time = TimeSinceCpuTicksInSeconds(VarGetFraction(EntVar(player, timerName)));
					STRING results = StringAdd(StringLocalize(VarGet("ResultTable1")), EntityName(player));
					int count = VarGetNumber("TopListSize");

					// end run
					SkiChaletEndRun(player);

					// see if we're the best
					SkiChaletSlalomUpdateTimes(player, time);

					// check for badges
					if (time <= VarGetFraction("GoldTime"))
					{
						EntityGrantReward(player, VarGet("GoldBadge"));
					} 
					else if (time <= VarGetFraction("SilverTime"))
					{
						EntityGrantReward(player, VarGet("SilverBadge"));
					} 
					else if (time <= VarGetFraction("BronzeTime"))
					{
						EntityGrantReward(player, VarGet("BronzeBadge"));
					} 

					// show timer
					results = StringAdd(results, StringLocalize(VarGet("ResultTable2")));
					results = StringAdd(results, FractionTimeToString(time));

					// best times
					for (i = 0; i < count; i++)
					{	
						if (VarGetFraction(StringAdd("BestTime", NumberToString(i))) < 999.0f)
						{
							if ( i == 0 )
								results = StringAdd(results, StringLocalize(VarGet("ResultTable3")));
							else
								results = StringAdd(results, StringLocalize(VarGet("ResultTable4")));

							results = StringAdd(results, VarGet(StringAdd("BestName", NumberToString(i))));
							results = StringAdd(results, StringLocalize(VarGet("ResultTable5")));
							results = StringAdd(results, FractionTimeToString(VarGetFraction(StringAdd("BestTime", NumberToString(i)))));
							results = StringAdd(results, StringLocalize(VarGet("ResultTable6")));
						}
					}

					results = StringAdd(results, StringLocalize(VarGet("ResultTable7")));
					SendPlayerDialog( player, results );

				} else {

					/////////////////////////////////////////////////////////
					// NEXT GATE

					STRING loc = StringAdd("marker:", VarGet("RaceNameKey"));
					loc = StringAdd(loc, NumberToString(number));
					UpdateWaypoint(EntityGetRewardToken(player, "WaypointToken"), loc, "Gate", NULL, -1.0f);
					EntityGrantRewardToken(player, "LastWaypointToken", number);
				}
			}
		}
	}
}

// called when the player enters the map
int SkiChaletSlalomOnEnterMap(ENTITY player)
{
	// clear any old race data
	EntityRemoveRewardToken(player, "WaypointToken");
	EntityRemoveRewardToken(player, "LastWaypointToken");
	EntityRemoveRewardToken(player, "RaceCourseToken");

	SkiChaletSlalomAddRemoveFromList(player);

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// MAIN 
//////////////////////////////////////////////////////////////////////////////////////////

void SkiChaletSlalom(void)
{
	int i, size;
	STRING timerName = StringAdd("timer", VarGet("RaceNameKey"));

	//////////////////////////////////////////////////////////////////////////////////
	INITIAL_STATE

		SET_STATE("RaceInitializing");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("RaceInitializing") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			//////////////////////////////////////////////////////////////////////////
			// Setting up the player list
			//////////////////////////////////////////////////////////////////////////
			eaCreate(&playerList);

			// create UI
			ScriptUITitle("RaceTitleUI", "RaceName", "RaceInfo");
			ScriptUITimer("RaceTimerUI", PERPLAYER("Timer"), "TimeText", "TimeToolTip");
			ScriptUICreateCollection("RaceUI", 2, "RaceTitleUI", "RaceTimerUI");

			// setting best time
			size = VarGetNumber("TopListSize");
			for (i = 0; i < size; i++)
			{
				VarSetFraction(StringAdd("BestTime", NumberToString(i)), 999.9f);
				VarSet(StringAdd("BestName", NumberToString(i)), "none");
			}
			
			// Settup NPCs
			SkiChaletSlalomSpawnContacts();

			// 
			OnPlayerEnteringMap( SkiChaletSlalomOnEnterMap );

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		SET_STATE("RaceRunning");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("RaceRunning") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			//////////////////////////////////////////////////////////////////////////
			// Setup anything the needs to be reset after pausing
			//////////////////////////////////////////////////////////////////////////
			OnPlayerEntersVolume(SkiChaletSlalomPlayerEntersVolume);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// check for old races
		size = eaSize(&playerList);
		for (i = size - 1; i >= 0; i--) // go backwards so we can delete old entries
		{
			ENTITY pEntString = playerList[i];
			if (pEntString && !VarIsEmpty(EntVar(pEntString, timerName)))
			{
				FRACTION time = TimeSinceCpuTicksInSeconds(VarGetFraction(EntVar(pEntString, timerName)));

				if ( (int) time > (60 * VarGetNumber("MaxTime")))
				{
					// old race - remove
					ScriptUIHide("Collection:RaceUI", pEntString);
					DestroyWaypoint(EntityGetRewardToken(pEntString, "WaypointToken"));
					EntityRemoveRewardToken(pEntString, "WaypointToken");
					EntityRemoveRewardToken(pEntString, "LastWaypointToken");
					EntityRemoveRewardToken(pEntString, "RaceCourseToken");
					VarSetEmpty(EntVar(pEntString, "Timer"));
					VarSetEmpty(EntVar(pEntString, timerName));

					SkiChaletSlalomAddRemoveFromList(pEntString);
				}
			}
		}

		if (DoEvery("PauseCheck", 0.25f, NULL))
		{
			if (MapHasDataToken(VarGet("PauseMapToken")))
				SET_STATE("RacePaused");
		}


	//////////////////////////////////////////////////////////////////////////////////
	STATE("RacePaused") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			//////////////////////////////////////////////////////////////////////////
			// clear callback
			//////////////////////////////////////////////////////////////////////////
			OnPlayerEntersVolume(NULL);

			//////////////////////////////////////////////////////////////////////////
			// remove UI from all players currently running race
			//////////////////////////////////////////////////////////////////////////
			size = eaSize(&playerList);
			for (i = size - 1; i > 0; i--) // go backwards so we can delete old entries
			{
				ENTITY pEntString = playerList[i];
				if (pEntString)
				{
					// old race - remove
					ScriptUIHide("Collection:RaceUI", pEntString);
					DestroyWaypoint(EntityGetRewardToken(pEntString, "WaypointToken"));
					EntityRemoveRewardToken(pEntString, "WaypointToken");
					EntityRemoveRewardToken(pEntString, "LastWaypointToken");
					EntityRemoveRewardToken(pEntString, "RaceCourseToken");
					VarSetEmpty(EntVar(pEntString, "Timer"));
					SkiChaletSlalomAddRemoveFromList(pEntString);
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// Clear the player list
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			// Remove Contacts
			//////////////////////////////////////////////////////////////////////////
			Kill("Contact", false);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

	END_STATES


	ON_SIGNAL("Pause")
		SET_STATE("RacePaused");
	END_SIGNAL

	ON_SIGNAL("Run")
		SET_STATE("RaceInitializing");
	END_SIGNAL

	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

	ON_STOPSIGNAL
		EndScript();
	END_SIGNAL
}

void SkiChaletSlalomInit()
{
	SetScriptName( "SkiChaletSlalom" );
	SetScriptFunc( SkiChaletSlalom );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End", "End" );		
	SetScriptSignal( "Pause", "Pause" );		
	SetScriptSignal( "Run", "Run" );		

	SetScriptVar( "StartRunFloat",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "EndRunFloat",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "RaceName",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "RaceInfo",								NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "TimeText",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TimeToolTip",							NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "GateCount",								NULL,		0 );
	SetScriptVar( "MaxTime",								NULL,		0 );

	SetScriptVar( "GoldTime",								NULL,		0 );
	SetScriptVar( "SilverTime",								NULL,		0 );
	SetScriptVar( "BronzeTime",								NULL,		0 );

	SetScriptVar( "GoldBadge",								NULL,		0 );
	SetScriptVar( "SilverBadge",							NULL,		0 );
	SetScriptVar( "BronzeBadge",							NULL,		0 );

	SetScriptVar( "ResultTable1",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ResultTable2",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ResultTable3",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ResultTable4",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ResultTable5",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ResultTable6",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ResultTable7",							NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "RaceNameKey",							NULL,		0 );

	SetScriptVar( "ContactList",							NULL,		0 );
	SetScriptVar( "ContactSpawnList",						NULL,		0 );
	SetScriptVar( "ContactLayout",							NULL,		0 );

	SetScriptVar( "NPCEncounterPosition",					"1",		0 );
	SetScriptVar( "TopListSize",							"5",		0 );

	SetScriptVar( "Leave",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ContactHeader",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ContactIntro",							NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "PauseMapToken",							NULL,		0 );

}
