// LOCATION SCRIPT

// This is an encounter script that runs the Hellions Burning Building event
//

#include "scriptutil.h"


#define CLEANUP_PL	"PL_FleeToNearestDoor"
#define MAYHEM_PL	"PL_Menace"


// tracks the number of fire event running simultaneously
int gMinerStrikeRunning = 0;

int SpawnNearbyMiners( LOCATION loc)
{
	ENCOUNTERGROUP group;
	group = FindEncounterGroupByRadius( 
		loc,
		VarGetNumber( "NewMinersSpawnAtLeastThisFarAway" ), 
		VarGetNumber( "NewMinersSpawnAtMostThisFarAway" ), //Within 300 feet of this encounter
		"Around", //TO DO emerge from door? 
		1, 
		0 );

	//I worst comes to worst, find them by the rally point
	if( !group || StringEqual( group, LOCATION_NONE ) )
	{
		group = FindEncounterGroupByRadius( 
			VarGet( "GatheringGroup" ),
			VarGetNumber( "NewMinersSpawnAtLeastThisFarAway" ), 
			VarGetNumber( "NewMinersSpawnAtMostThisFarAway" ), //Within 300 feet of this encounter
			"Around", //TO DO emerge from door? 
			1, 
			0 );
	}

	if( group && !StringEqual( group, LOCATION_NONE ) )
	{
		LOCATION loc;
		loc = Spawn( 1, "NewMiners", VarGet( "MinerSpawnDef" ), group, "Around", VarGetNumber("MinerSpawnLevel"), 0 );
		if( loc && !StringEqual( loc, LOCATION_NONE ) )
		{
			SetEncounterActive( group );
			DetachSpawn( group );
			return 1;
		}
	}
	return 0;
}

int ReplenishMiners( LOCATION loc )
{
	int success;
	while( NumEntitiesInTeam( "NewMiners" )+ NumEntitiesInTeam( "Miners" ) < VarGetNumber( "FullComplementOfMiners" ) ) 
	{
		success = SpawnNearbyMiners( loc );
		if( !success )
			return 0;
	}
	return 1;
}
 
//Odds = chance a guy will say something any given second
//I know the math is all wrong, but for low odds, it's a fast, decent approximation
static void DoChatter( STRING textVar, TEAM team, FRACTION odds )
{
	int numEnts = NumEntitiesInTeam(team);
  
	odds = ( odds / (60.0*2.0) ) * numEnts ; //Convert minutes to script ticks times number of guys 

	if( odds > RandomFraction( 0, 1 ) ) 
	{
		ENTITY speaker = GetEntityFromTeam(team, RandomNumber( 1, numEnts ) );
		Say( speaker, VarGet( textVar ), 0  );
	}
}

void MinerStrike(void)
{   
	char buf[1000]; 

	if( NumEntitiesInTeam(MyEncounter()) == 0 )      
	{
		SET_STATE( "ScriptEnd" );
	}

	//////////////////////////////////////////////////////////////////////////
	INITIAL_STATE   ///////////////////////////////////`////////////////////    
	{
		ON_ENTRY  
		{
			ENCOUNTERGROUP gatheringGroup;

			// Basic Script Set up.  if there are too many running, don't spawn another
			if (gMinerStrikeRunning > 1)
			{
				CloseMyEncounterAndEndScript();
				return;
			}

			ScriptControlsCompletion( MyEncounter() );

			//////////Find a random gathering point -- I think there will only be one at Scrapyard's grave
			gatheringGroup = FindEncounter( MyEncounter(), 0, 0, 0, 0, 0, VarGet("MinerRallyPointGroupName"), SEF_UNOCCUPIED_ONLY  );
			if( !gatheringGroup )
			{
				CloseMyEncounterAndEndScript();
				return;
			}
			VarSet( "GatheringGroup", gatheringGroup );

			SET_STATE( "WaitingForScrapYard" );
		}
		END_ENTRY
		
	}
 
	//////////////////////////////////////////////////////////////////////////////////
	STATE("WaitingForScrapYard") ////////////////////////////////////////////////////////
	{
		int ralliedMiners;
		#define MAXENTS 100
		ENTITY entlist[MAXENTS];
	
		VarSet( "ScrapyardSpeechPoint", ActorFromEncounterPos(MyEncounter(), 1) ); 
		VarSet( "MacGuffin", ActorFromEncounterPos(MyEncounter(), 2) ); 


		//////////// Spawn a Miners around the zone and send them toward the gathering point
		ReplenishMiners( VarGet( "GatheringGroup" ) );

		AddRandomAngryPLAndFight( "NewMiners" );
		
		sprintf( buf, "MoveToPos( \"%s\", CombatOverride(Aggressive), Variation(20), TargetRadius(10) )", VarGet("GatheringGroup") );
		AddAIBehavior("NewMiners", buf );
		AddAIBehaviorFlag( "NewMiners", "DoNotRun" );
		AddAIBehaviorPermFlag( "NewMiners", "DoNotGoToSleep" );
		//sprintf( buf, "OverrideSpawnPos( \"%s\" )", VarGet("GatheringGroup") );
		//AddAIBehaviorPermFlag( "NewMiners", buf );

		SetChatObjective( "NewMiners", VarGet( "MacGuffin") );

		SwitchScriptTeam( "NewMiners", "NewMiners", "Miners" ); //Now that you've told them what to do, swithc them to the miner team

		/////////// Existing Miners Chatter
		DoChatter( "MinerRallyExclamations", "Miners", VarGetFraction("EachMinersExclamationsPerMinute") );

		//////////////// Check to see if enough Miners have rallied by Scrapyard's grave.  If so, Summon Scrapyard
		ralliedMiners = GetAllEntities( entlist, 
			0,	
			0,		//Not yet implemented
			0,		//Not yet implemented
			0,      //Not yet implemented
			"Miners",
			VarGetFraction( "RalliedMinerRange" ), 
			VarGet( "GatheringGroup" ),
			0,
			1,
			0,
			MAXENTS,
			0
			);

		if( ralliedMiners > VarGetNumber( "RalliedMinersNeededToSummonScrapyard" ) )
		{
			SET_STATE( "ScrapyardArrives" );
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("ScrapyardArrives") ////////////////////////////////////////////////////////
	{
		ON_ENTRY
		{
			ENTITY scrapyard;
			ENCOUNTERGROUP group;

			//////////////Summon Scrapyard, and he gives a speech
			group = Spawn( 1, "ScrapyardTeam", VarGet( "ScrapyardSpawndef" ), VarGet("GatheringGroup" ), NULL, 0, 0 );
			scrapyard = ActorFromEncounterPos( group, VarGetNumber( "PositionOfScrapYardInEncounter" ) );
			JoinAITeam( "Miners", scrapyard ); //Adds scrapyard to the Miner's AI team
			VarSet( "ScrapYardGroup", group );
			VarSet( "Scrapyard", scrapyard );
			
			//TO DO Set Scrapyard Invulnerable
			SetChatObjective( "ScrapyardTeam", VarGet( "MacGuffin") );

			Say( VarGet("Scrapyard"), VarGet("ScrapyardRallySpeech"), 2  );

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertText"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

		}
		END_ENTRY

		///////////  (No need to replenish the miners during this short lull -- it just clutters the code)

		////////// After a while, he is done and it's time to head out
		if( DoEvery( "ScrapyardTalking", VarGetFraction("ScrapyardRallySpeechTime"), 0 ) )
		{
			SET_STATE( "ScrapyardsMarch" );
		}

		//////////////////If scrapyard is dead, what do I do?
		if( GetHealth( VarGet("Scrapyard" ) ) <= 0.0 )
		{
			//TO DO event end
			SET_STATE("EventEnd");
		}
		
	}


	//////////////////////////////////////////////////////////////////////////////////
	STATE("ScrapyardsMarch") ////////////////////////////////////////////////////////	
	{
		ON_ENTRY
		{
			///////////// Scrapyard heads toward the target, telling all existing miners to follow him
			SetEncounterActive( VarGet("ScrapyardGroup" ) );
			DetachSpawn( VarGet("ScrapyardGroup") );

			AddAIBehavior( "ScrapyardTeam", "CombatOverride(Defensive)" );
			AddAIBehaviorPermFlag( "ScrapyardTeam", "DoNotGoToSleep" );

			sprintf( buf, "MoveToPos( %s, MediumPriority, TargetRadius(10) )", VarGet("ScrapyardSpeechPoint")  ); //TODO MOveGroupToPos
			AddAIBehavior("ScrapyardTeam", buf );
			AddAIBehaviorFlag( "ScrapyardTeam", "CombatOverride(Defensive)" );
			AddAIBehaviorFlag( "ScrapyardTeam", "DoNotRun" );
			
			//sprintf( buf, "OverrideSpawnPos( \"%s\" )", VarGet("ScrapyardSpeechPoint") );
			//AddAIBehaviorPermFlag( "ScrapyardTeam", buf );

			///Look for Beacons to use if you can find them
			//TO DO move out?
			if( !VarIsEmpty( "ScriptPathBeaconsName" ) )
			{
				char beaconName[200];
				int i;
				for( i = 10 ; i >= 1 ; i-- ) //TO DO : find largest
				{
					sprintf( beaconName, "marker:%s_%d", VarGet( "ScriptPathBeaconsName" ), i );
					if( LocationExists( beaconName ) )
					{
						sprintf( buf, "MoveToPos( %s, MediumPriority, TargetRadius(10) )", beaconName  ); //TODO MOveGroupToPos
						AddAIBehavior("ScrapyardTeam", buf );
						AddAIBehaviorFlag( "ScrapyardTeam", "CombatOverride(Defensive)" );
						AddAIBehaviorFlag( "ScrapyardTeam", "DoNotRun" );
					}
				}
			}

			//Existing Miners, drop what you are doing and follow me
			RemoveAIBehaviorByType( "Miners", "MoveToPos" );
			AddRandomAngryPLAndFight( "Miners" );
			SetFollowers( VarGet("Scrapyard"), "Miners" ); //TO DO add Follow behavior (and group follow behavior)
			AddAIBehaviorFlag( "Miners", "DoNotRun" );
			AddAIBehaviorPermFlag( "Miners", "DoNotGoToSleep" );
			sprintf( buf, "SetNewSpawnPos(\"%s\")", VarGet("ScrapyardSpeechPoint"));
			AddAIBehaviorPermFlag( "Miners", buf );

		}
		END_ENTRY

		///////////////Get new miners and tell them to follow Scrapyard
		ReplenishMiners( VarGet("Scrapyard") );

		AddRandomAngryPLAndFight( "NewMiners" );
		SetFollowers( VarGet("Scrapyard"), "NewMiners" );
		AddAIBehaviorFlag( "NewMiners", "DoNotRun" );
		AddAIBehaviorPermFlag( "NewMiners", "DoNotGoToSleep" );
		SetChatObjective( "NewMiners", VarGet( "MacGuffin") );
		//sprintf( buf, "OverrideSpawnPos( \"%s\" )", VarGet("ScrapyardSpeechPoint") );
		//AddAIBehaviorPermFlag( "NewMiners", buf );

		SwitchScriptTeam( "NewMiners", "NewMiners", "Miners" );

		////////////// All Miners, Do Chatter -- Scarpyard do Scrapyard talk
		DoChatter( "MinerMarchExclamations", "Miners", VarGetFraction("EachMinersExclamationsPerMinute") );
		DoChatter( "ScrapyardMarchExclamations", VarGet("Scrapyard"), VarGetFraction("ScrapyardsExclamationsPerMinute") );

		//If scrapyard doesn't have enough miners nearby, stop and wait for them
		{
			#define MAXENTS 100
			ENTITY entlist[MAXENTS];
			int nearbyMiners = GetAllEntities( entlist, 
				0,	
				0,		//Not yet implemented
				0,		//Not yet implemented
				0,      //Not yet implemented
				"Miners",
				VarGetFraction( "RalliedMinerRange" ), 
				EntityPos( VarGet("Scrapyard") ),
				0,
				1,
				0,
				MAXENTS,
				0
				);

			if( (NumEntitiesInTeam("Miners") > 20 && nearbyMiners < 5 ) && !StringEqual( "TRUE", VarGet( "ScrapyardWaitingForStragglers" ) ) )
			{
				AddAIBehavior("ScrapyardTeam", "PL_PREACH_LOW" );
				Say( VarGet( "Scrapyard" ), VarGet( "ScrapyardWaitingForStragglersSpeech" ), 1 ); 
				VarSet( "ScrapyardWaitingForStragglers", "TRUE" );
			}
			if( StringEqual( "TRUE", VarGet( "ScrapyardWaitingForStragglers" )) && (nearbyMiners > 10 || NumEntitiesInTeam("Miners") <= 20) )
			{
				RemoveAIBehaviorByType("ScrapyardTeam", "DoNothing" );
				Say( VarGet( "Scrapyard" ), VarGet( "ScrapyardStragglersCatchUpSpeech" ), 1 ); 
				VarSet( "ScrapyardWaitingForStragglers", "FALSE" );
			}
		}

		//////////// When Scrapyard gets to his target, and his minions gather, it's time to make his speech
		if( EntityInArea( VarGet( "Scrapyard" ), MyEncounter() ) )
		{
			//////////////// Check to see if enough Miners have rallied by Scrapyard's grave.  If so, Summon Scrapyard
			int ralliedMiners;
			#define MAXENTS 100
			ENTITY entlist[MAXENTS];

			//Set Scrapyard's encounter spawn here, so he'll always return to here
			
			ralliedMiners = GetAllEntities( entlist, 
				0,	
				0,		//Not yet implemented
				0,		//Not yet implemented
				0,      //Not yet implemented
				"Miners",
				VarGetFraction( "RalliedMinerRange" ), 
				MyEncounter(),
				0,
				1,
				0,
				MAXENTS,
				0
				);

			if( ralliedMiners > VarGetNumber( "RalliedMinersNeededToSummonScrapyard" ) )
				SET_STATE( "ScrapyardsSpeech" );
		}

		//////////////////If scrapyard is dead, what do I do?
		if( GetHealth( VarGet("Scrapyard" ) ) <= 0.0 )
		{
			//TO DO event end
			SET_STATE("EventEnd");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("ScrapyardsSpeech") ////////////////////////////////////////////////////////
	{
		ON_ENTRY
		{
			/////////////Scrapyard starts monologing
			Say(VarGet("Scrapyard"), VarGet("ScrapyardTargetSpeech"), 2 );
		}
		END_ENTRY

		///////////  (No need to replenish the miners during this short lull -- it just clutters the code)

		/////////// When he's done, the battle Begins
		if( DoEvery( "ScrapyardTargetTalking", VarGetFraction("ScrapyardTargetSpeechTime"), 0 ) )
		{
			SET_STATE( "ScrapyardsBattle" );
		}

		//////////////////If scrapyard is dead, what do I do?
		if( GetHealth( VarGet("Scrapyard" ) ) <= 0.0 )
		{
			//TO DO event end
			SET_STATE("EventEnd");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("ScrapyardsBattle") ////////////////////////////////////////////////////////
	{
		ON_ENTRY
		{
			//////////// Tell Scrapyard, this is his new stomping ground
			AddAIBehaviorPermFlag( "ScrapyardTeam", "ReturnToSpawnDistance(30)" );
			AddAIBehaviorPermFlag( "ScrapyardTeam", "AlwaysReturnToSpawn" );

			//////////// Tell all miners to come to the target and raise hell -- otherwise they seem to forget
			RemoveAllBehaviors("Miners");
			AddRandomAngryPLAndFight( "Miners" ); //TO DO Mayhem (Or other anim)
			sprintf( buf, "MoveToPos( %s, CombatOverride(Defensive), Variation(20), TargetRadius(20) )", MyEncounter() ); //TODO MOveGroupToPos
			AddAIBehavior("Miners", buf );
			AddAIBehaviorFlag( "Miners", "DoNotRun" );
			AddAIBehaviorFlag( "Miners", "DoNotGoToSleep" );

			sprintf( buf, "OverrideSpawnPos(%s)", GetXYZStringFromLocation(VarGet("ScrapyardSpeechPoint")));
			AddAIBehaviorPermFlag( "Miners", buf );
			AddAIBehaviorPermFlag( "Miners", "ReturnToSpawnDistance(30)" );
			AddAIBehaviorPermFlag( "Miners", "AlwaysReturnToSpawn" );

			SetChatObjective( "Miners", VarGet( "MacGuffin") );
		}
		END_ENTRY

		//Get New Miners, tell them to come to target and then raise hell
		ReplenishMiners( MyEncounter() );

		//////////// Tell new miners to come to the target and raise hell -- otherwise they seem to forget
		RemoveAllBehaviors("NewMiners");
		AddRandomAngryPLAndFight( "NewMiners" );
		sprintf( buf, "MoveToPos( %s, CombatOverride(Defensive), Variation(20), TargetRadius(20) )", MyEncounter() ); //TODO MOveGroupToPos
		AddAIBehavior("NewMiners", buf );
		AddAIBehaviorFlag( "NewMiners", "DoNotRun" );
		AddAIBehaviorFlag( "NewMiners", "DoNotGoToSleep" );

		sprintf( buf, "OverrideSpawnPos(%s)", GetXYZStringFromLocation(VarGet("ScrapyardSpeechPoint")));
		AddAIBehaviorPermFlag( "NewMiners", buf );
		AddAIBehaviorPermFlag( "NewMiners", "ReturnToSpawnDistance(30)" );
		AddAIBehaviorPermFlag( "NewMiners", "AlwaysReturnToSpawn" );

		SetChatObjective( "NewMiners", VarGet( "MacGuffin") );

		SwitchScriptTeam( "NewMiners", "NewMiners", "Miners" );

		//////////// All miners chatter
		DoChatter( "ScrapyardTargetExclamations", VarGet("Scrapyard"), VarGetFraction("ScrapyardsExclamationsPerMinute") );
		DoChatter( "MinerTargetExclamations", "Miners", VarGetFraction("EachMinersExclamationsPerMinute") );

		//////////////////If scrapyard is dead, what do I do?
		if( GetHealth( VarGet("Scrapyard" ) ) <= 0.0 )
		{
			//TO DO event end
			SET_STATE("EventEnd");
		}

		if( DoEvery("TimeTillEnd", VarGetFraction("TimeToEndEvent"), 0 ) ) 
		{
			SET_STATE("EventEnd");
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventEnd") ////////////////////////////////////////////////////////
	{
		//TO DO what to say? DoChatter( "ScrapyardTargetExclamations", VarGet("Scrapyard"), VarGetFraction("ScrapyardsExclamationsPerMinute") );
		AddAIBehavior( "Miners", "PL_FleeToNearestDoor" );
		Kill("ScrapyardTeam", false);
		Say(VarGet("Scrapyard"), VarGet("ScrapyardDeathRattle"), 2 );

		//Kinda weird way to get guys to say things when Scrapyard dies
		{
			int i,numEnts = NumEntitiesInTeam( "Miners" );
			for( i = 1 ; i <= numEnts ; i++ )
			{
				if( i < 5 ) //five guys max say things -- yes, random I know.
				{
					ENTITY speaker = GetEntityFromTeam( "Miners" , i );
					Say( speaker, VarGet( "ScrapyardDeathMinerExclamations" ), 0  );
				}
			}
		}
		CloseMyEncounterAndEndScript();
	}
	END_STATES

	ON_SIGNAL("WinGame")
		SET_STATE( "EventWin" );
	END_SIGNAL

	ON_SIGNAL("LoseGame")
		SET_STATE( "EventLose" );
	END_SIGNAL

	}


void MinerStrikeInit()
{
	SetScriptName( "MinerStrike" );
	SetScriptFunc( MinerStrike );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "ScriptPathBeaconsName",					NULL,		SP_OPTIONAL );
	SetScriptVar( "MinerRallyPointGroupName",				NULL,		0 );
	SetScriptVar( "MinerSpawnDef",							NULL,		0 );
	SetScriptVar( "MinerSpawnLevel",						"23",		0 );
	SetScriptVar( "FullComplementOfMiners",					NULL,		0 );
	SetScriptVar( "RalliedMinerRange",						NULL,		0 );
	SetScriptVar( "RalliedMinersNeededToSummonScrapyard",	NULL,		0 );
	SetScriptVar( "ScrapyardSpawndef",						NULL,		0 );
	SetScriptVar( "PositionOfScrapYardInEncounter",			NULL,		0 );
	SetScriptVar( "NewMinersSpawnAtLeastThisFarAway",		NULL,		0 );
	SetScriptVar( "NewMinersSpawnAtMostThisFarAway",		NULL,		0 );

	SetScriptVar( "MinerBehaviorAtTargetPoint",				NULL,	0	 );
	SetScriptVar( "MinerBehaviorAtRallyPoint",				NULL,	0	 );
	
	SetScriptVar( "ScrapyardsExclamationsPerMinute",		NULL,	0	 );
	SetScriptVar( "EachMinersExclamationsPerMinute",		NULL,	0	 );

	SetScriptVar( "TimeToEndEvent",							NULL,	0	 );

	SetScriptVar( "MinerRallyExclamations",					NULL,	SP_MULTIVALUE | SP_OPTIONAL  );
	SetScriptVar( "MinerMarchExclamations",					NULL,	SP_MULTIVALUE | SP_OPTIONAL  );
	SetScriptVar( "MinerTargetExclamations",				NULL,	SP_MULTIVALUE | SP_OPTIONAL  );

	SetScriptVar( "ScrapyardRallySpeech",				NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "ScrapyardRallySpeechTime",			NULL,		0 );
	SetScriptVar( "ScrapyardMarchExclamations",			NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "ScrapyardTargetSpeech",				NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "ScrapyardTargetSpeechTime",			NULL,		0 );
	SetScriptVar( "ScrapyardTargetExclamations",		NULL,		SP_MULTIVALUE | SP_OPTIONAL );

	SetScriptVar( "ScrapyardStragglersCatchUpSpeech",	NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "ScrapyardWaitingForStragglersSpeech",NULL,		SP_MULTIVALUE | SP_OPTIONAL );

	SetScriptVar( "ScrapyardDeathRattle",				NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "ScrapyardDeathMinerExclamations",	NULL,		SP_MULTIVALUE | SP_OPTIONAL );

	SetScriptVar( "AlertText",							NULL,		SP_MULTIVALUE | SP_OPTIONAL );

	SetScriptSignal( "End Event", "EndScript" );		
}


