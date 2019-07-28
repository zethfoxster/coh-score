// MISSION SCRIPT
//

// Runs the thorn vines in the CoT CoV Respec mission "Thorn Tree"

#include "scriptutil.h"

// Variables
//		SpawnTimer#		Time to respawn for this spawn
//		SpawnActive#	Is this spawn active? (should it be used base on player count)
//		SpawnInUse#		Is this spawn been spawned? (should I be able to check it for team size)
//		SpawnGroup#		Group for this spawn


void CoVThornTreeSpawnNew(NUMBER location)
{
	char				groupName[50], timer[50], inuse[50];
	ENCOUNTERGROUP		group = "first"; 
	LOCATION			loc;

	sprintf(groupName, "SpawnGroup%d", location);
	sprintf(timer, "SpawnTimer%d", location);
	sprintf(inuse, "SpawnInUse%d", location);

	// reset timer
	VarSetNumber(timer, 0);

	// Find  spawn location
	if (LocationExists("marker:VineMarker"))
	{
		loc = "marker:VineMarker";
	} else{
		loc = LOC_ORIGIN;
	}

	group = FindEncounterGroupByRadius( 
		loc,
		0, 
		9999,
		groupName, 
		1, 
  		0 );    

	if (stricmp(group, "location:none") != 0) {
		Spawn( 1, groupName, VarGet("SpawnName"), group, groupName, MissionLevel(), 0 );  
		VarSetNumber(inuse, 1);
	}
}

void CoVThornTreeUpdateUI()
{
	int		i, remaining;
	char	group[50];
	char	inuse[50];

	// check to see if any are around
	remaining = 0;
	for (i = 0; i < VarGetNumber("MaxSpawns"); i++) 
	{
		sprintf(inuse, "SpawnInUse%d", i);
		if (VarGetNumber(inuse) > 0) {
			sprintf(group, "SpawnGroup%d", i);
			remaining += NumEntitiesInTeam(group);
		}
	}

	VarSetNumber("RemainCount", remaining);

	if (remaining <= 0) 
	{
		// all of them are dead
		if (!StringEmpty(VarGet("Reward")) &&	!StringEqual("None", VarGet("Reward")))
			MissionGrantReward(VarGet("Reward"));

		if (!StringEmpty(VarGet("Objective")) &&	!StringEqual("None", VarGet("Objective")))
			SetMissionObjectiveComplete(OBJECTIVE_SUCCESS, VarGet("Objective"));

		EndScript();
	}
}

void CoVThornTreeUpdateSpawns()
{
	int		playerCount = MAX(MissionNumHeroes(), VarGetNumber("MinPlayers"));
	int		i, lastDeath;
	char	group[50], timer[50];
	char	active[50];
	char	inuse[50];

	// check for respawns
	for (i = 0; i < VarGetNumber("MaxSpawns"); i++) 
	{
		if (i <= playerCount) 
		{
			sprintf(inuse, "SpawnInUse%d", i);
			sprintf(active, "SpawnActive%d", i);
			sprintf(group, "SpawnGroup%d", i);

			// check to see if spawn is dead
			if (VarGetNumber(inuse) != 1 || NumEntitiesInTeam(group) <= 0)
			{
				sprintf(timer, "SpawnTimer%d", i);
				lastDeath = VarGetNumber(timer);
				if (lastDeath) {
					if (lastDeath <= CurrentTime())
						CoVThornTreeSpawnNew(i);
				} else {
					if (VarGetNumber(active)) 
					{
						// first time we've detected that this spawn has died.
						// set timer
						VarSetNumber(timer, CurrentTime() + VarGetNumber("RespawnTime"));
					} else {
						CoVThornTreeSpawnNew(i);
					}
				}
			}
			VarSetNumber(active, 1);
		} else {
			VarSetNumber(active, 0);
		}
	}
}

void CoVThornTree(void)
{
	INITIAL_STATE
	{
		ON_ENTRY 
		{
			if (!OnMissionMap()) 
			{
				EndScript();
				return;
			}

			// start the spawning!
			CoVThornTreeUpdateSpawns();

			// create the UI
			ScriptUITitle(ONENTER("TitleUI"), "DisplayTitle", "DisplayTooltip");
			ScriptUIList(ONENTER("CountUI"), "DisplayTooltip", "CountText", "UIColor", "RemainCount", "UIColor");
			VarSet("UIColor", "ffffffff");

//			ScriptUICreateCollection(ONENTER("UI"), 2, "TitleUI", "CountUI" );
			CoVThornTreeUpdateUI();			

		}
		END_ENTRY

		SET_STATE("Running"); 
	}

	STATE( "Running")
	{
		if (DoEvery("DelayTimer", VarGetFraction("CheckTime"), NULL))
		{
			CoVThornTreeUpdateSpawns();			
		}

		if (DoEvery("UITimer", 0.01f, NULL))
		{
			CoVThornTreeUpdateUI();			
		}


	}

	END_STATES


	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

	ON_STOPSIGNAL
		EndScript();
	END_SIGNAL
}

void CoVThornTreeInit()
{
	SetScriptName( "CoVThornTree" );
	SetScriptFunc( CoVThornTree );

	SetScriptType( SCRIPT_MISSION );			

	SetScriptVar( "MinPlayers",						"4",					SP_OPTIONAL); 
	SetScriptVar( "RespawnTime",					"180",					0);  // in seconds
	SetScriptVar( "SpawnName",						NULL,					0);
	SetScriptVar( "Reward",							NULL,					0);
	SetScriptVar( "Objective",						NULL,					0);

	SetScriptVar( "DisplayTitle",					NULL,					0);
	SetScriptVar( "DisplayTooltip",					NULL,					0);
	SetScriptVar( "CountText",						NULL,					0);


	SetScriptVar( "CheckTime",						"0.25",					SP_OPTIONAL); 
	SetScriptVar( "MaxSpawns",						"8",					SP_OPTIONAL); 

}