#include "scriptutil.h"

int SeeNewTrainerOnEnterMap(ENTITY player)
{
	// This popup is for characters who haven't leveled up yet but could if
	// they talked to a trainer.
	if (GetLevel(player) == 1 && CanLevel(player))
	{
		if (IsEntityPraetorian(player))
			SendPlayerDialog(player, StringLocalize(VarGet("MessagePraetorians")));
		else if (IsEntityOnRedSide(player))
			SendPlayerDialog(player, StringLocalize(VarGet("MessageVillains")));
		else if (IsEntityOnBlueSide(player))
			SendPlayerDialog(player, StringLocalize(VarGet("MessageHeroes")));
	}

	return 0;
}

void SeeNewTrainerRunOnInitialization(void)
{
	OnPlayerEnteringMap(SeeNewTrainerOnEnterMap);
}

void SeeNewTrainer(void)
{
	INITIAL_STATE

		ON_ENTRY 
				
		END_ENTRY

	END_STATES

	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

	ON_STOPSIGNAL
		EndScript();
	END_SIGNAL
}

void SeeNewTrainerInit(void)
{
	SetScriptName("SeeNewTrainer");
	SetScriptFunc(SeeNewTrainer);
	SetScriptFuncToRunOnInitialization(SeeNewTrainerRunOnInitialization);
	SetScriptType(SCRIPT_ZONE);		

	SetScriptVar("MessagePraetorians",	NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("MessageHeroes",		NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("MessageVillains",		NULL,	SP_DONTDISPLAYDEBUG);

	SetScriptSignal("End", "End");		

}