// MISSION SCRIPT
// Encounter Script
//
#include "scriptutil.h"

void SendText(void)
{  
	INITIAL_STATE 
	{
		// If this encounter is active
		if (CheckMissionObjectiveExpression(VarGet("ActivateOnObjectiveComplete")))
		{
			NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);
			STRING chatMessage = VarGet("ChatMessage");
			STRING floaterMessage = VarGet("FloaterMessage");
			STRING reward = VarGet("Reward");

			if (!StringEmpty(chatMessage)) ScriptSendChatMessage(ALL_PLAYERS, chatMessage);
			if (!StringEmpty(floaterMessage)) ScriptServeFloater(ALL_PLAYERS, floaterMessage);
			if (!StringEmpty(reward)) MissionGrantReward(reward);
			EndScript();
		}
	}

	END_STATES
}


void SendTextInit()
{
	SetScriptName("SendText");
	SetScriptFunc(SendText);
	SetScriptType(SCRIPT_MISSION);		

	SetScriptVar("ActivateOnObjectiveComplete",		"",			0);
	SetScriptVar("ChatMessage",						"",			SP_OPTIONAL);
	SetScriptVar("FloaterMessage",					"",			SP_OPTIONAL);
	SetScriptVar("Reward",							"",			SP_OPTIONAL);
}
