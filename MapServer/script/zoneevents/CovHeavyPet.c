// LOCATION SCRIPT

// This is a location script that runs the Heavy Pet event
//

#include "scriptutil.h"

#define COV_RECLUSES_VICTORY_BOUNTY_TOKEN			"ReclusesVictoryBounty"
#define COV_RECLUSES_VICTORY_HEAVY_TOKEN			"ReclusesVictoryHeavy"

void CoVHeavyPetSpawn()
{
	ENCOUNTERGROUP group = "first"; 

	// spawn heavy
	group = FindEncounterGroupByRadius( 
		MyLocation(),
		0, 
		100, 
		"Heavy", 
		1, 
 		0 );  

	if (stricmp(group, "location:none") != 0) {
		Spawn( 1, "Heavy", VarGet("Spawn"), group, "Heavy", 50, 0 );  

		// setting PL of the pet
		AddAIBehavior("Heavy", VarGet("SpawnBehavior"));
//		AddAIBehavior("Heavy", "DoNothing(AnimBit(\"Quick\"), AnimBit(\"Spawn\"), AnimBit(\"Sleep\"), Untargetable)");

	}
}


// This function is called when the player initiates the heavy capture.  It checks to see if the heavy 
//		is of the same side as the player.
int CoVHeavyPetControlInteract(ENTITY player, ENTITY target)
{
	if (IsEntityHero(player) == VarGetNumber("Side")) {
		if (EntityHasRewardToken(player, COV_RECLUSES_VICTORY_HEAVY_TOKEN))
		{
			SendPlayerDialog(player, StringLocalize(VarGet("AlreadyHaveOne")));
			return true;
		} else {
			return false;
		}
	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("CantCapture")));
		return true;
	}
}

// This function is called when the player has successfully completed the control interact timer.  This
//		function spawns the heavy and switches control to the player.
int CoVHeavyPetControlComplete(ENTITY player, ENTITY target)
{
	// clean
	RemoveAllBehaviors("Heavy");

	// setting the player as the master of the heavy
	SetAsPets(player, "Heavy");
	VarSet("Owner", player);

	// these don't teleport
	AddAIBehavior("Heavy", "Flag(NoTeleport)");

	// mark player as owning pet
	EntityGrantRewardToken(player, COV_RECLUSES_VICTORY_HEAVY_TOKEN, 1);

	// activate!
	AddAIBehavior("Heavy", VarGet("CaptureBehavior"));
//	AddAIBehavior("Heavy", "DoNothing(AnimBit(\"Quick\"), AnimBit(\"Spawn\"), AnimBit(\"Sleep\"), AnimBit(\"No\"), Timer(5))");

	// give credit
	GiveBadgeCredit(player, kScriptBadgeTypeRVHeavyPet);

	// give score
	{
		NUMBER currentBounty = EntityGetRewardToken(player, COV_RECLUSES_VICTORY_BOUNTY_TOKEN);

		// checking to make sure player has a current bounty marker
		if (currentBounty == -1) 
		{
			EntityGrantRewardToken(player, COV_RECLUSES_VICTORY_BOUNTY_TOKEN, 0);
			currentBounty = 0;
		}

		EntityGrantRewardToken(player, COV_RECLUSES_VICTORY_BOUNTY_TOKEN, VarGetNumber("HeavyScore") + currentBounty);
	}


	SET_STATE("Underway");

	return true;
}

void CoVHeavyPetOnEntityDefeat(ENTITY player, ENTITY victor)
{
	// don't need to bother with this if it isn't a player
	if (IsEntityPlayer(player))
	{
		if (StringEqual(player, VarGet("Owner")))
		{
			// owner has been killed, kill pet and reset
			SET_STATE("Reset");
		}
	}
}


void CoVHeavyPetShutdown(void)
{
		GlowieRemove(VarGet("Glowie"));

		Kill("Heavy", false);
		Kill("glowies", false);

		EntityRemoveRewardToken(VarGet("Owner"), COV_RECLUSES_VICTORY_HEAVY_TOKEN);

		EndScript();
}

void CoVHeavyPet(void)
{  
	ENTITY glowie;
	GLOWIEDEF bunkerControl = NULL; 

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// setup glowie
			bunkerControl = GlowieCreateDef(StringLocalize(VarGet("ControlName")), 
											VarGet("ControlModel"), 
											VarGet("ControlInteract"), VarGet("ControlInterrupt"), 
											VarGet("ControlComplete"), VarGet("ControlInteractBar"), 
											VarGetNumber("ControlInteractTime"));

			// create glowie
			glowie = GlowiePlace(bunkerControl, MyLocation(), false, CoVHeavyPetControlInteract, CoVHeavyPetControlComplete);
			SetScriptTeam(glowie, "glowies");

			VarSet("Glowie", glowie);

			OnEntityDefeated ( CoVHeavyPetOnEntityDefeat );		// for death of owner

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		SET_STATE("Startup");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Startup") ////////////////////////////////////////////////////////
	
		// reset the glowie
		GlowieSetState(VarGet("Glowie"));

		CoVHeavyPetSpawn();

		SET_STATE("Waiting");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Waiting") ////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Underway") ////////////////////////////////////////////////////////

		if( DoEvery("DeathCheck", VarGetFraction("DeathCheck"), 0 ) ) 
		{ 
			// check to see if pet is gone
			if (NumEntitiesInTeam("Heavy") < 1)
				SET_STATE("Reset");

			// check to see if owner is gone
			if(!StringEqual(VarGet("Owner"), "") && !EntityExists( VarGet( "Owner" ) ) )
				SET_STATE("Reset");

		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Reset") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			Kill("Heavy", false);

			EntityRemoveRewardToken(VarGet("Owner"), COV_RECLUSES_VICTORY_HEAVY_TOKEN);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if( DoEvery("DeathCheck", VarGetFraction("LullTime"), 0 ) ) // restart timer
		{ 
			SET_STATE("Startup");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Shutdown") ////////////////////////////////////////////////////////
		CoVHeavyPetShutdown();

	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "Shutdown" );
	END_SIGNAL

	ON_SIGNAL("Reset")
		SET_STATE( "Reset" );
	END_SIGNAL

	ON_STOPSIGNAL
		CoVHeavyPetShutdown();
	END_SIGNAL


}


void CoVHeavyPetInit()
{
	SetScriptName( "CoVHeavyPet" );
	SetScriptFunc( CoVHeavyPet );
	SetScriptType( SCRIPT_LOCATION );		

	SetScriptVar( "ControlName",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ControlModel",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ControlInteract",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ControlInterrupt",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ControlComplete",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ControlInteractBar",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ControlInteractTime",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CantCapture",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "AlreadyHaveOne",							NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeavyScore",								NULL,		0 );

	SetScriptVar( "Spawn",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "Side",									NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "SpawnBehavior",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CaptureBehavior",						NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "DeathCheck",								"0.5",		SP_OPTIONAL );
	SetScriptVar( "LullTime",								"0.1",		SP_OPTIONAL );

	SetScriptSignal( "End", "EndScript" );		
	SetScriptSignal( "Reset", "Reset" );		
}

