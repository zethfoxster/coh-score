// ZONE SCRIPT
//

// Used for testing new features of the script system

#include "scriptutil.h"
#include "entPlayer.h"
#include "TeamReward.h"
#include "timing.h"

extern Entity ** ents;
Entity* EntTeamInternal(TEAM team, int index, int* num);

void ScriptUIIdlePoll();

void TestZoneScript(void)
{
	static int scriptID=0;
	static int thunderval=0;
	static int meter1value=0;
	static int meter2value=0;
	static int sendme = 0;
	ENTITY player;
	INITIAL_STATE

		ON_ENTRY
		sendme = 0;
		VarSet( "Obelisk", "My Title" );
		VarSet( "Victory", "100" );
		VarSet( "MyCollect", "100" );
		VarSetNumber( "HideMe", 1);
		VarSetNumber( "ClockEnd", 900 + timerSecondsSince2000());
		ScriptUIMeter("NewMeter","Victory","VictoryIsMine","HideMe","200","1000", "2255ffff", "00ff00ff", "{MAX}/{MIN} Jugglers");
		ScriptUIMeter("NewMeter2","VictoryTwo","Testing 1 2 3 4 5","HideMe","10","30", "2255ffff", "00ff00ff", "{{VAL}}/{{MAX}}");
		ScriptUIMeter("NewMeter3","VictoryTwo","ShortStuff","HideMe","0","100", "2255ffff", "00ff00ff", "Jugglers");
		ScriptUITitle("TitleUI", "Obelisk", "info for the title");
		ScriptUITimer("TheTimer", "ClockEnd", "You better run", "Time left before you explode");
		ScriptUICollectItems("TheCollectionIsMade", 3, "Token:BloodyBayOre1", "ItemOne", "sp_icon_meteor_full.tga", "Token:BloodyBayOre2", "SecondItem", "sp_icon_meteor_full.tga", "Token:BloodyBayOre3", "ThirdandLastItem", "sp_icon_meteor_full.tga");
		ScriptUIList(ONENTER("TheList"), "This is a list", PERPLAYER("MyListEnemy"), "ff0000ff", "100", "ffff00ff");
		
			// put stuff that should run once on initialization here
		/*	scriptID=ScriptUICreate();
			ScriptUIAddTitleBar(scriptID,"Title","Talk to Iris Parker","Info box text goes here.");
			ScriptUIAddMeter(scriptID,"Power","Power",0,1000,"Watts", 0, 0);
			ScriptUIAddMeter(scriptID,"Victory","Victory",0,1000,"Jugglers", 0, 0);
			ScriptUIAddList(scriptID,"List","Wanted: Dead or Alive!");
			ScriptUIAddMeter(scriptID,"Awesomeness","Entity:BloodyBayOre4",0,1000,"Jons", 0, 0);
			ScriptUIAddMeter(scriptID,"Might","Might",0,1000,"Newtons", 0, 0);
			TimerSet( "TheTime", 2);
			ScriptUIAddScriptTimer(scriptID,"TheTime","Go!!!","Thunder!");
//			scriptUIAddMeter(scriptID,"Meter 2","Entity:BloodyBayOre1",0,1000, "blah", 0, 0);
			{
				char images[256];
				char backgrounds[256];
				char magics[256];
				sprintf(images,"sp_icon_meteor_01.tga,sp_icon_meteor_02.tga,sp_icon_meteor_03.tga,sp_icon_meteor_04.tga,sp_icon_meteor_05.tga,sp_icon_meteor_06.tga");
				sprintf(backgrounds,"Red Ore,Green Ore,Blue Ore,Yellow Ore,Orange Ore,Purple Ore");
				sprintf(magics,"Entity:BloodyBayOre1,Entity:BloodyBayOre2,Entity:BloodyBayOre3,Thunderforce,Entity:BloodyBayOre5,Entity:BloodyBayOre6");
				ScriptUIAddCollectNItems(scriptID,"Ore",images,backgrounds,magics,"Ore Power");
			}
			TimerSet( "ThunderTimer", 3);

//			scriptUIAddScriptTimer(scriptID,"ThunderTimer","Powerforce");

			ScriptUIEntStartUsing(scriptID,GetEntityFromTeam(ALL_PLAYERS,1));
			ScriptUISetList(GetEntityFromTeam(ALL_PLAYERS,1),"ffff00ff Thundernator,ff0000ff power forcer");
			giveRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre1");
			giveRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre2");
			giveRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre3");
			giveRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre4");
			giveRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre5");
			giveRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre6");
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre1")->val=0;
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre2")->val=0;
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre3")->val=0;
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre4")->val=0;
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre5")->val=0;
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre6")->val=0;
			{
//				RewardToken * r=getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre1");
//				r->val=0;
			}*/
		END_ENTRY
		/*if (EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0)==0)
			return;
		if (meter1value%10==0)
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre1")->val^=1;
		if (meter1value%20==0)
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre2")->val^=1;
		if (meter1value%30==0)
			getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre3")->val^=1;

		if (meter1value%10==0) {
			ScriptUISetVar(scriptID,"Thunderforce",(meter1value/10)%2);
			if (meter1value%20)
				ScriptUIHideWidget(scriptID,GetEntityFromTeam(ALL_PLAYERS,1),"Power");
			else
				ScriptUIShowWidget(scriptID,GetEntityFromTeam(ALL_PLAYERS,1),"Power");
		}*/
//		getRewardToken(EntTeamInternal(GetEntityFromTeam(ALL_PLAYERS,1),0,0),"BloodyBayOre4")->val++;
		//ScriptUISetVar(scriptID,"Power",meter1value+=1);
//		ScriptUISetVar(scriptID,"Victory",meter2value+=2);
		
		// put stuff that should run every tick here
//		VarSetNumber( "Victory", 500 );
		VarSet( "Item", "1" );
		player = GetEntityFromTeam(ALL_PLAYERS,1);
		//VarSet(EntVar(player, "MyListEnemy"), "Conor");
		ScriptUIShow("TitleUI", player);
	END_STATES

	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

	ON_STOPSIGNAL
		//ScriptUIEntStopUsing(scriptID,GetEntityFromTeam(ALL_PLAYERS,1));
		//ScriptUIDestroy(scriptID);
		EndScript();
	END_SIGNAL
}

void TestZoneScriptInit()
{
	SetScriptName( "TestZoneScript" );
	SetScriptFunc( TestZoneScript );
	SetScriptType( SCRIPT_ZONE );		

}