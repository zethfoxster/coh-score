/*\
 *
 *	scripttable.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides the link table for all the script functions available
 *
 */

#include "scripttable.h"
#include "error.h"
#include "stdio.h"
#include "strings_opt.h"

#define MAX_SCRIPTS 200	// If we need more than this, it's time to re-revaluate what we're doing
#define DECLARE(x)	void x(void);

DECLARE(TerraVoltaMission)
DECLARE(CavernOfTranscendenceMission)
DECLARE(TestScript)
DECLARE(ShadowShardInvasion);
DECLARE(WinterEvent);
DECLARE(EdenMission);
DECLARE(CotSummonSpawn);
DECLARE(ClockworkPaladin);
DECLARE(OctopusAttack);
DECLARE(HellionsFire);
DECLARE(TrollsRave);
DECLARE(StrigaRobotsWakeUp);  
DECLARE(StrigaBossFightAndEscape);
DECLARE(GhostShip);
DECLARE(CouncilVs5thColumn);
DECLARE(CroatoaBattle);
DECLARE(RunningBoss);
DECLARE(CompetitiveSearch);
DECLARE(StopEscape);
// DECLARE(ChattyBoss);
// DECLARE(HalloweenEvent);


ScriptFunction g_scriptFunctions[MAX_SCRIPTS] =
{
	{ "TerraVoltaMission",	TerraVoltaMission,	SCRIPT_MISSION,		
		{ 
			{ "STATUS_GUARD",		NULL,		0 },
			{ "STATUS_STORY",		NULL,		0 },
			{ "STATUS_SCIENTISTS",	NULL,		0 },
			{ "STATUS_BOMB",		NULL,		0 },
			{ "STATUS_CORRIDOR",	NULL,		0 },
			{ "STATUS_CORE",		NULL,		0 }, 
		},
		{ 
			{ "Win",			"WinGame"},
			{ "Lose",			"LoseGame"},
		}
	},

	{ "StrigaBossFightAndEscape",	StrigaBossFightAndEscape,	SCRIPT_MISSION,		
	{ 
		{ "TimeBombTime",				"0.5",			0 }, //TIme between when the boss dies and bomb goes off
		{ "YouMustGetOutMessage",		"Giant Robot Self-Destructs in 30 Seconds!",	0 }, //Alert that comes up when you kill the boss
		{ "20SecMessage",				"20 Seconds to Self-Destruct",	0 },
		{ "10SecMessage",				"10 Seconds to Self-Destruct",	0 },
		{ "WayPointText",				"Escape Point",					0 }, //Alert that comes up when you kill the boss
		{ "BadThingVillainType",		"Objects_Reactor_Core",			0 }, //Alert that comes up when you kill the boss
		{ "BossFleeHealth",				"0.2",							0 },
		{ "SmokeVillainType",			"None",			0 },

		//{ "SmokeVillainLevel",			"None",			0 }, 
		//marker:smoke1
		//marker:escapedoor
		//EncounterGroup:BossSpawn
		//marker:badthing
	},

	{ 	
		{ 0 }, //Commands understood
	}
	},

	{ "StrigaRobotsWakeUp",	StrigaRobotsWakeUp,	SCRIPT_MISSION,		
	{ 
		{ "AmbushSprungMessage",				"Trap is Sprung!",		0 },
		{ "AwakenTimeOfRobotGroup1",			"0.1",					0 },
		{ "AwakenTimeOfRobotGroup2",			"0.5",					0 },
		{ "AwakenTimeOfRobotGroup3",			"1.0",					0 },
		{ "AwakenTimeOfRobotGroup4",			"1.5",					0 },
		{ "AwakenTimeOfRobotGroup5",			"2.0",					0 },
		{ "InnerRobotAttackPL",					"PL_Robot_awake",		0 },
		{ "OuterRobotAttackPL",					"PL_Robot_awake",		0 },
		{ "InnerObjective",						"BluePrint1",			0 },		//The mission objective that triggers the ambush when completed
		{ "OuterObjective",						"BluePrint2",			0 },		//Second 
		{ "LockedDoorMessage",					"Locked!",				0 },
		//HardCoded:	
		//"marker:InnerDoor"
		//"marker:InnerDoor2"
		//"marker:OuterDoor"
		//"EncounterGroup:InnerRobots0"
		//"EncounterGroup:InnerRobots1"
		//"EncounterGroup:InnerRobots2"
		//"EncounterGroup:InnerRobots3"
		//"EncounterGroup:InnerRobots4"
		//"EncounterGroup:InnerRobots5"
		//"EncounterGroup:OuterRobots0"
		//"EncounterGroup:OuterRobots1"
		//"EncounterGroup:OuterRobots2"
		//"EncounterGroup:OuterRobots3"
		//"EncounterGroup:OuterRobots4"
		//"EncounterGroup:OuterRobots5"
		//"trigger:RobotRoom"
		//"trigger:InnerRobotRoom"
	},
	{ 
		{ 0 }, //Commands understood
	}
	},

/* Moved to init function
	{ "ChattyBoss",	ChattyBoss,	SCRIPT_ENCOUNTER,		
	{ 
		{ "Actor",								"",		0 }, 
		{ "BOSS_1STBLOOD_DIALOG",				"",		SP_MULTIVALUE | SP_OPTIONAL }, 
		{ "BOSS_75HEALTH_DIALOG",				"",		SP_MULTIVALUE | SP_OPTIONAL }, 
		{ "BOSS_50HEALTH_DIALOG",				"",		SP_MULTIVALUE | SP_OPTIONAL }, 
		{ "BOSS_25HEALTH_DIALOG",				"",		SP_MULTIVALUE | SP_OPTIONAL }, 
		{ "BOSS_DEAD_DIALOG",					"",		SP_MULTIVALUE | SP_OPTIONAL }, 
		{ "BOSS_KILLEDHERO_DIALOG",				"",		SP_MULTIVALUE | SP_OPTIONAL },
		
	},
	{ 
		{ 0 }, //Commands understood
	}
	},
*/
	{ "RunningBoss",RunningBoss,	SCRIPT_ENCOUNTER,		
	{ 
		{ "Actor",								"",		0 }, 
		{ "RunWhenHealthIsBelow",				"",		0 }, 
		{ "RunWhenTeamSizeIsBelow",				"",		0 }, 
		{ "BOSS_RUNAWAY_DIALOG",				"",		SP_MULTIVALUE }, 
	},
	{ 
		{ 0 }, //Commands understood
	}
	},

	{ "CompetitiveSearch", CompetitiveSearch,	SCRIPT_MISSION,		
	{ 
		{ "SearchingEnemiesEncounter",			"",		0 }, 
		{ "NumberOfObjectivesNeeded",			"",		0 }, 
		{ "SearchersFindObjectiveDialog",		"",		SP_MULTIVALUE }, 
		{ "SearchersDestroyObjectiveDialog",	"",		SP_MULTIVALUE }, 
		{ "SearchersEncounterPlayersDialog",	"",		SP_MULTIVALUE }, 
	},
	{ 
		{ 0 }, //Commands understood
	}
	},

	{ "StopEscape",	StopEscape,			SCRIPT_MISSION,		
	{ 
		{ "WaitTime",					"0.5",				0 }, 		
		{ "DoorLocation",				"",					0 }, 		
		{ "EscapeLimit",				"30",				0 }, 		
		{ "AdditionalSpawns",			"",					0 }, 		
		{ "NumToSpawn",					"100",				0 }, 		
		{ "StatusMessage",				"have escaped",		0 }, 		
		{ "StatusMessageSingular",		"has escaped",		0 }, 		
		{ "RemainMessage",				"have escaped",		0 }, 		
		{ "RemainMessageSingular",		"has escaped",		0 }, 		
		{ "Fled",						"0",				0 }, 		
		{ "WaypointText",				"",					0 }, 		
	},
	{ 
		{ 0 }, //Commands understood
	}
	},



	{ "CavernOfTranscendence",	CavernOfTranscendenceMission,	SCRIPT_MISSION,		
		{ 
			{ "STATUS_RESCUE",		NULL,		0 },
			{ "STATUS_TOUCH",		NULL,		0 },
		},
		{
			{ "Win",			"WinGame"},
			{ "Lose",			"LoseGame"},
		}
	},

	{ "TestScript",			TestScript,			SCRIPT_ZONE,		
		{ 
			{ "PingText",		"e",		SP_MULTIVALUE },
			{ "PongText",		"o",		SP_MULTIVALUE },
		},
		{ 
			{ "Send Hi Signal",		"Hi" },
			{ "Test strings",		"TestStrings" },
			{ "Check AI Team",		"AITeam" },
			{ "Print Location",		"PrintLocation" },
			{ "Set Waypoint",		"SetWaypoint" },
			{ "Clear Waypoint",		"ClearWaypoint" },
			{ "Make5th",			"Make5th" },
			{ "Kill5th",			"Kill5th" },
			{ "Set Handlers",		"SetHandlers" },
		}
	},

	{ "ShadowShardInvasion",	ShadowShardInvasion,	SCRIPT_ZONE, 
	{ 0 },
	{ 
		{ "Spawn More",				"SpawnMore" },
		{ "Test Every Loc",			"SpawnAtEveryLocation" },
		{ "End Event",				"End" },
	}
	},

	{	"WinterEvent",	WinterEvent,	SCRIPT_ZONE, 
		{ 0 },
		{ 
			{ "Start Stage One",				"MoveToStageOne" },
			{ "Start Stage Two",				"MoveToStageTwo" },
			{ "Start Stage Three",				"MoveToStageThree" },
			{ "Start Stage Four",				"MoveToStageFour" },
			{ "Start Stage Five",				"MoveToStageFive" },
			{ "Start Stage Six",				"MoveToStageSix" },
			{ "Test Every Loc",					"SpawnAtEveryLocation" },
		}
	},

/* moved to init function
	{ "Halloween",				HalloweenEvent,			SCRIPT_ZONE, 
	{ 0 },
	{ 
		{ "End Event",				"End" },
		{ "1 King",					"King1" },
		{ "2 King",					"King2" },
		{ "3 King",					"King3" },
		{ "4 King",					"King4" },
		{ "6 King",					"King6" },
		{ "8 King",					"King8" },
		{ "10 King",				"King10" },
		{ "TT Every 5 seconds",		"TT5s" },
		{ "TT Every 15 seconds",	"TT15s" },
		{ "TT Every 30 seconds",	"TT30s" },
		{ "TT Every 45 seconds",	"TT45s" },
		{ "TT Every 60 seconds",	"TT60s" },
		{ "TT Every 75 seconds",	"TT75s" },
		{ "TT Every 90 seconds",	"TT90s" },
		{ "Treat 50%",				"Treat50%" },
		{ "Treat 60%",				"Treat60%" },
		{ "Treat 70%",				"Treat70%" },
		{ "Treat 80%",				"Treat80%" },
		{ "Treat 90%",				"Treat90%" },
	}
	},
*/
	{ "EdenMission",	EdenMission,	SCRIPT_MISSION,		
		{ 
			{ "DESTROY_ROCK_WALL",		NULL,			0 },
			{ "DESTROY_MOLD_WALL",		NULL,			0 },
			{ "DEFEAT_CRYSTAL_TITAN",	NULL,			0 },
			{ "FREE_HEROES",			NULL,			0 },
		},
		{ 0 }
	},

	{	"HellionsFire",	 HellionsFire,	SCRIPT_LOCATION, 
		{ 
			{ "SpawnLevel",				NULL,		0 },  
			{ "MaxHellions",			NULL,		0 }, 
			{ "TotalHellions",			NULL,		0 }, 
			{ "MaxFires",				NULL,		0 }, 
			{ "BuildingHealth",			NULL,		0 }, 
			{ "FireDamage",				NULL,		0 }, 
			{ "RespawnCheckTime",		NULL,		0 }, 
			{ "BossDoorChance",			NULL,		0 }, 
			{ "BossName",				NULL,		0 }, 
			{ "BossNumber",				NULL,		0 }, 
			{ "FireSpawnRoof",			NULL,		0 }, 
			{ "FireSpawnWall",			NULL,		0 }, 
			{ "TimeToCleanupLose",		NULL,		0 }, 
			{ "TimeToCleanupWin",		NULL,		0 }, 
			{ "TimeToRevert",			NULL,		0 }, 
			{ "CrowdCheer",				NULL,		SP_MULTIVALUE }, 
			{ "PoliceCheer",			NULL,		SP_MULTIVALUE }, 
			{ "CrowdBoo",				NULL,		SP_MULTIVALUE }, 
			{ "PoliceBoo",				NULL,		SP_MULTIVALUE }, 
			{ "PoliceSpeech",			NULL,		SP_MULTIVALUE }, 
			{ "PoliceSpeech2",			NULL,		SP_MULTIVALUE }, 
			{ "PoliceSpeech3",			NULL,		SP_MULTIVALUE }, 
			{ "AlertMessage",			NULL,		0 }, 
			{ "CompleteMessage",		NULL,		0 }, 
			{ "WaypointLocationName",	NULL,		0 }, 
			{ "WatergunReward",			NULL,		0 }, 
			{ "WatergunRewardAdv",		NULL,		0 }, 
			{ "Reward",					NULL,		0 }, 
			{ "RewardRadius",			NULL,		0 }, 
		},
		{ 
			{ "Win",			"WinGame"},
			{ "Lose",			"LoseGame"},
		}
	},
	{	"TrollsRave",	 TrollsRave,	SCRIPT_LOCATION, 
		{ 
			{ "SpawnLevel",				NULL,		0 }, 
			{ "MaxTrolls",				NULL,		0 }, 
			{ "MinTrollCrowd",			NULL,		0 }, 
			{ "TotalTrolls",			NULL,		0 }, 
			{ "BossName",				NULL,		0 }, 
			{ "RespawnCheckTime",		NULL,		0 }, 
			{ "PoliceSpawn",			NULL,		0 }, 
			{ "TrollsSpawnFar",			NULL,		0 }, 
			{ "TrollsSpawnDoor",		NULL,		0 }, 
			{ "TimeToCleanupLose",		NULL,		0 }, 
			{ "TimeToCleanupWin",		NULL,		0 }, 
			{ "PoliceSpeech",			NULL,		SP_MULTIVALUE }, 
			{ "PoliceSpeech2",			NULL,		SP_MULTIVALUE }, 
			{ "RunToLocation",			NULL,		SP_MULTIVALUE }, 
			{ "AlertMessage",			NULL,		0 }, 
			{ "CompleteMessage",		NULL,		0 }, 
			{ "WaypointLocationName",	NULL,		0 }, 
			{ "TimeToCleanupWin",		NULL,		0 }, 
		},
		{ 
			{ "Cleanup",		"Cleanup"},
			{ "Shutdown",		"Shutdown"},
		}
	},

// Encounter scripts
	{ "CotSummonSpawn",	CotSummonSpawn,	SCRIPT_ENCOUNTER,		
		{ 
			{ "COTSUMMON_TIME",			NULL,			0 },
			{ "COTSUMMON_VILLAIN",		NULL,			0 },
			{ "COTSUMMON_LEVEL",		NULL,			0 },
			{ "COTSUMMON_NAME",			NULL,			0 },
			{ "COTSUMMON_SAYBEFORE",	NULL,			0 }, 
			{ "COTSUMMON_SAYAFTER",		NULL,			0 },
		},
		{ 
			{ "Kill Demon",		"KillDemon"},
		}
	},


	//"ClockworkPaladin" = Name of the script, Nmaed in TaskDef for mission scripts
	//ClockWorkPaladin = Function pointer to the sctipt
	//{ "DESTROY_ROCK_WALL", = 
	{ "ClockworkPaladin",	ClockworkPaladin,	SCRIPT_ENCOUNTER,		
	{ 
		{ "ATTACK_DOUGHNUT_INNER",		NULL,		0 },  // 50 };
		{ "ATTACK_DOUGHNUT_OUTER",		NULL,		0 },  //500 );
		{ "FarSpawnLayout",				NULL,		0 }, //Followranks
		{ "MinClockwork",				NULL,		0 },  //60 );
		{ "TimeToHeroSuccess",			NULL,		0 }, //30 );
		{ "UnfinishedPaladinType",		NULL,		0 }, //"I don't know" );
		{ "UnfinishedPaladinLevel",		"7",		0 },
		{ "UnfinishedPaladinDisplayName",NULL,		0 }, //"My Name Here" );
		{ "PaladinType",				NULL,		0 }, //"I don't know" );
		{ "PaladinLevel",		        "7",		0 },
		{ "PaladinDisplayName",			NULL,		0 }, //"My Name Here" );
		{ "PowerTower",					NULL,		0 }, //"I don't know" );
		{ "PowerTowerName",				NULL,		0 }, //"My Name Here" );
		{ "StartingFarSpawnCount",		NULL,		0 }, //10 );
		{ "StartingFarSpawn",			NULL,		0 }, //"SpawnDefs/ZoneEvents/ClockworkAttack/ClockworkFirstWave_D1_V0.spawndef" );
		{ "StartingFarSpawnLevel",		"5",		0 },
		{ "HeapSpawn",					NULL,		0 },
		{ "HeapSpawnStage1",			NULL,		0 }, //"SpawnDefs/ZoneEvents/ClockworkAttack/ClockworkFirstWave_D1_V0.spawndef" );
		{ "HeapSpawnLevelStage1",		NULL,		0 }, //1 );
		{ "HeapSpawnTimeStage2",		NULL,		0 }, //10 );
		{ "HeapSpawnStage2",			NULL,		0 },
		{ "HeapSpawnLevelStage2",		NULL,		0 }, //4 );
		{ "HeapSpawnTimeStage3",		NULL,		0 }, //20 );
		{ "HeapSpawnStage3",			NULL,		0 },
		{ "HeapSpawnLevelStage3",		NULL,		0 }, //5 );
		{ "HeapSpawnTimeStage4",		NULL,		0 }, //25 );
		{ "HeapSpawnStage4",			NULL,		0 },
		{ "HeapSpawnLevelStage4",		NULL,		0 }, //6 );

		{ "PaladinFront",				NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"<em roar>I am born! <em flex>" );
		{ "MinionSuccess",				NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"<em roar>It is done!" );
		{ "MinionFront1",				NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"If we build it, he will rule." );
		{ "MinionFront2",				NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"For the King!" );
		{ "MinionFront3",				NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"receiving command : Protect the Paladin>!" );
		{ "MinionNeedsMore1",			NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"Build more soldiers to protect the construction site!" );
		{ "MinionNeedsMore2",			NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"Yes, more soldiers!" );
		{ "MinionNextPhase",			NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"NO! We can not fail the King. We must have more soldiers!" );
		{ "MinionFailure",				NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"No! No! No!" );
		{ "MinionRebuildPaladin",		NULL,		SP_MULTIVALUE | SP_OPTIONAL }, //"reset : We must rebuild him!" );

		{ "Reward",						"ClockworkPaladinEventBadge",		0 },
		{ "RewardRadius",				"500.0",							0 },

		{ "AlertMessage",				NULL,								0 }, 
		{ "ClearMessage",				NULL,								0 }, 
		{ "PaladinMessage",				NULL,								0 }, 

	},						//vars needed defined in a mission description.  Since this is not a mission
	{ 
		{ "Win",				"Win" },
		{ "Lose",				"Lose" },
	}

	},

		//"OctopusAttack" = Name of the script, Nmaed in TaskDef for mission scripts
		//OctopusAttack = Function pointer to the sctipt
	{ "OctopusAttack",	OctopusAttack,	SCRIPT_ENCOUNTER,		
	{ 
		{ "OctopusLayout",			NULL,		0 }, //Head is in position 1
		{ "OctopusSpawn",			NULL,		0 },
		{ "DockWorkerLayout",		NULL,		0 }, //Foreman is in position 1
		{ "DockWorkerSpawn",		NULL,		0 }, 

		{ "DockWorkersDanceTime",	NULL,		0 }, 
		{ "OctopusSpawnLevel",		NULL,		0 }, 

		{ "DockWorkerGrumble",		NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockWorkerToHero",		NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockForemanToWorkers",	NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockForemanToHero",		NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockForemanMissesOctopus",NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockForemanSpotsOctopus",NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockWorkerCheer",		NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockForemanCheer",		NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockWorkerToHeroThanks",	NULL,		SP_MULTIVALUE | SP_OPTIONAL },
		{ "DockForemanToHeroThanks",NULL,		SP_MULTIVALUE | SP_OPTIONAL },

		{ "Reward",					"OctopusEventBadge",		0 },
		{ "RewardRadius",			"800.0",					0 },
		{ "AlertMessage",			NULL,						0 }, 
		{ "ClearMessage",			NULL,						0 }, 
	},						//vars needed defined in a encounter description. 

	//Need ScriptMarkers for the Octopus for pointing to
	//Need script location object with parameters
	//1 DockWorker Layout and 3 Octopus Layout placed
	//Need Octopus spawndef, need Dockworker spawndef

	{ "FireItUp", "FireItUp" }, //Commands understood
	},

	//"OctopusAttack" = Name of the script, Nmaed in TaskDef for mission scripts
	//OctopusAttack = Function pointer to the sctipt
	{ "GhostShip",	GhostShip,	SCRIPT_ENCOUNTER,		
		{ 
			{ "GhostShipSpawn",				NULL,		0 }, 
			{ "GhostShipSpawnLevel",		NULL,		0 },  
			{ "GhostSpawn",					NULL,		0 },
			{ "GhostSpawnLevel",			NULL,		0 },
			{ "IntervalBetweenSpawns",		NULL,		0 }, 
			{ "TimeToKillGhosts",			NULL,		0 }, 
			{ "GhostSpeak",					NULL,		SP_MULTIVALUE },
			{ "MaxGhosts",					"60",		0 },
			{ "Reward",					"GhostShipEventBadge",		0 },
			{ "RewardRadius",			"800.0",					0 },
			{ "AlertMessage",				NULL,		0 }, 
			{ "ClearMessage",				NULL,		0 }, 

			//HardCoded:
			//marker:GhostShipPathEnd
			//Ghost Ship Position is 1 in it's spawn
			//Searches for Around and then rumble
			//Encoutner where I spawn
			//Encounter used to spawn my dudes : any or around
		},						

		{ "FireItUp", "FireItUp" }, //Commands understood
	},


	{ "CouncilVs5thColumn",			CouncilVs5thColumn,			SCRIPT_ZONE,		
		{ 
			{ "ATTACK_DOUGHNUT_INNER",	"100",		0 },
			{ "ATTACK_DOUGHNUT_OUTER",	"200",		0 },
			{ "FarCouncilSpawnLayout",	"Around",	0 },

			{ "DEFENSE_DOUGHNUT_OUTER",	"100",		0 },
			{ "Near5thSpawnLayout",		"Around",	0 },

			{ "Min5thDefenders",		"50",		0 },
			{ "MinCouncilAttackers",	"50",		0 },
			
			{ "RefreshFrequency",		"0.2",		0 }, //TO DO : make 5.0
			{ "EncounterClearRadius",	"100.0",	0 },

			{ "",	"100.0",	0 },
		},
		{ 
			{ "End Event",				"End" },
		}
	},

	{ "CroatoaBattle",			CroatoaBattle,				SCRIPT_LOCATION,		
		{ 
			{ "TuathaSpawndef",			"",			0 },
			{ "TuathaSpawnLayout",		"",			0 },
			{ "CabalSpawndef",			"",			0 },
			{ "CabalSpawnLayout",		"",			0 },
			{ "RedCapSpawndef",			"",			0 },
			{ "RedCapGiantSpawndef",	"",			0 },
			{ "RedCapSpawnLayout",		"",			0 },
			{ "FirbolgSpawndef",		"",			0 },
			{ "FirbolgGiantSpawndef",	"",			0 },
			{ "FirbolgSpawnLayout",		"",			0 },

			{ "ChanceOfGiants",			"25",		0 }, 
			{ "AlertMessage",			NULL,		0 }, 
			{ "ClearMessage",			NULL,		0 }, 

		},
		{ 
			{ "End Event",				"EndScript" },
		}
	},


	//End (dont accidentally delete the stuff below)
	{ NULL, 0, 0 }, 
};

int g_scriptFunctionsInitted = 0;
ScriptFunction * g_currScriptFunction;

void NewCurrScriptFunction()
{
	int i;

	// Mustn't run off the end of the array, so we always need the end marker
	// below us.
	for (i = 0; i < MAX_SCRIPTS - 1; i++)
	{
		if (!g_scriptFunctions[i].name)
		{
			g_currScriptFunction = &g_scriptFunctions[i];
			g_scriptFunctions[i + 1].name = NULL;
			return;
		}
	}
	FatalErrorf( "MAX_SCRIPTS-1 reached");
}
#define INITSCRIPT(x) {DECLARE(x); NewCurrScriptFunction(); x(); g_currScriptFunction = NULL;}
void ScriptInitScriptFuntions();

ScriptFunction* ScriptFindFunction(const char* name)
{
	int i;

	if( !g_scriptFunctionsInitted )
	{
		g_scriptFunctionsInitted = 1;
		ScriptInitScriptFuntions();
	}

	for (i = 0; i < MAX_SCRIPTS && g_scriptFunctions[i].name; i++)
	{
		if (stricmp(name, g_scriptFunctions[i].name) == 0)
			return &g_scriptFunctions[i];
	} 
	return NULL;
}

#include <stdio.h>

void ScriptInitScriptFuntions()
{
	// PvP Zones
	INITSCRIPT( ConvertibleFirebaseInit )
	INITSCRIPT( CoVBloodyBayOreGameInit )
	INITSCRIPT( CoVSirensCallHunterGameInit )
	INITSCRIPT( CoVWarburgFootballInit )
	INITSCRIPT( CoVWarburgScientistInit )
	INITSCRIPT( CoVReclusesVictoryInit )
	INITSCRIPT( CoVPvPMapInit )

	// Zone Events
	INITSCRIPT( TestZoneScriptInit )
	INITSCRIPT( GhostTrapInit )
	INITSCRIPT( MinerStrikeInit )
	INITSCRIPT( KeepPopulatedInit )
	INITSCRIPT( CoVZoneKillXInit )
	INITSCRIPT( GenericInvasionInit )
	INITSCRIPT( PlayTogetherInit )
	INITSCRIPT( HolidayEventInit )
	INITSCRIPT( HolidayEventZoneInit )
	INITSCRIPT( HalloweenEventInit )
	INITSCRIPT( SpringFlingInit )
	INITSCRIPT( TrainingRoomRVEventInit )
	INITSCRIPT( GenericBadgeGrantInit )
	INITSCRIPT( GenericDoNothingInit )
	INITSCRIPT( PocketDSkiResortInit )
	INITSCRIPT( PocketDMonkeyFightInit )
	INITSCRIPT( ForbiddenZonesInit )
	INITSCRIPT( HamidonZoneInit )
	INITSCRIPT( RiktiInvasionInit )
	INITSCRIPT( RiktiPylonInit )
	INITSCRIPT( SetMapTokenInit )
	INITSCRIPT( TestCapeBugInit )
	INITSCRIPT( SkiChaletSlalomInit )
	INITSCRIPT( SetLevelZoneScriptInit )
	INITSCRIPT( WeddingEventInit )
	INITSCRIPT( ZombieApocalypseInit )
	INITSCRIPT( MalaiseEventInit )
	INITSCRIPT( SeeNewTrainerInit )
	INITSCRIPT( OnEnterGrantPowerZoneInit )
	INITSCRIPT( NemesisPlotInit )

	// Spawn Scripts
	INITSCRIPT( ChattyBossInit )
	INITSCRIPT( BartenderInit )
	INITSCRIPT( StoreNPCInit )
	INITSCRIPT( CoVSnakePitInit )
	INITSCRIPT( CoVHeavyPetInit )
	INITSCRIPT( InfoNPCInit )
	INITSCRIPT( DataNPCInit )
	INITSCRIPT( TradeNPCInit )
	INITSCRIPT( ContactNPCInit )
	INITSCRIPT( AuctionNPCInit )
	INITSCRIPT( TimedEncounterInit )
	INITSCRIPT( TriggeredBehaviorInit )
	INITSCRIPT( EncounterAlertInit )
	INITSCRIPT( HealthObjectiveInit )
	INITSCRIPT( TetherInit )
	INITSCRIPT( MultiTriggeredBehaviorInit )


	// Taskforces
	INITSCRIPT( CoVSkyRaidersInit )
	INITSCRIPT( CathedralOfPainInit ) 
	INITSCRIPT( CathedralOfPainWillForgeInit ) 
	INITSCRIPT( CathedralOfPainRularuuInit ) 

	// Mission Scripts
	INITSCRIPT( WaveAttacksEncounterInit )
	INITSCRIPT( WaveAttacksInit )
	INITSCRIPT( ProximityTriggerInit )
	INITSCRIPT( StealthCameraInit )
	INITSCRIPT( LeadToPlaceInit ) 
	INITSCRIPT( PvPMissionInit ) 
	INITSCRIPT( DefeatCounterInit ) 
	INITSCRIPT( DefeatListInit ) 
	INITSCRIPT( PlayTogetherMissionInit )
	INITSCRIPT( CoVThornTreeInit )
	INITSCRIPT( SafeGuardInit )
	INITSCRIPT( SafeGuardVandalsInit )
	INITSCRIPT( StopExplosionsInit )
	INITSCRIPT( StartMissionTimerInit )
	INITSCRIPT( MapTokenTriggerInit )
	INITSCRIPT( SetLevelMissionInit )
	INITSCRIPT( ObjectiveTriggerInit )
	INITSCRIPT( DefeatAllInit )
	INITSCRIPT( MissionTeleportGlowieInit )
	INITSCRIPT( ScriptedZoneEventInit )
	INITSCRIPT( ScriptedMissionEventInit )
	INITSCRIPT( DeadlyApocalypseInit )
	INITSCRIPT( TimedObjectiveInit )
	INITSCRIPT( SendTextInit )
	INITSCRIPT( KickAllFromMapInit )
	INITSCRIPT( HealthBarListScriptInit )
	INITSCRIPT( OnEnterObjectiveCompleteInit )
	INITSCRIPT( SetObjectiveTextInit )
	INITSCRIPT( MultiLeadToPlaceInit )
	INITSCRIPT( HolidayEventMissionInit )
	INITSCRIPT( OnEnterGrantPowerMissionInit )

	// Base Raid V2 Scripts
	INITSCRIPT( IOPRaidInit )
	INITSCRIPT( FFARaidInit )
	INITSCRIPT( MayhemRaidInit )

	// Beta Test Scripts
	INITSCRIPT( BetaNPCInit )

	// Etc
	INITSCRIPT( BaseRaidInit )
	INITSCRIPT( MiscInit )

	// Lua
	INITSCRIPT( scriptLuaInitMission )
	INITSCRIPT( scriptLuaInitEncounter )
	INITSCRIPT( scriptLuaInitLocation )
	INITSCRIPT( scriptLuaInitZone )
	INITSCRIPT( scriptLuaInitEntity )

	// Entity
	INITSCRIPT( InfoNPCEntityInit )
	INITSCRIPT( EntityScriptStartInit )
}

