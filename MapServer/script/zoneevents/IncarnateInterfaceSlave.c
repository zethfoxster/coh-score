#include "IncarnateInterfaceEvent.h"
#include "missionobjective.h"

ENTITY *eventGlowies;

void IncarnateInterfaceSlave_Activate()
{
	STRING zoneNumber = VarGet("ZoneNumber");
	STRING zoneType = VarGet("ZoneType");
	char temp[500];
	int glowieCount = atoi(VarGet("GlowieCount"));
	int activeGlowie = rand() % glowieCount;

	sprintf(temp, "IncarnateInterfaceSlave%s%s_ActiveGlowie", zoneNumber, zoneType);
	MapSetDataToken(temp, activeGlowie);

	SET_STATE("Run");
}

void IncarnateInterfaceSlave_Deactivate()
{
	STRING zoneNumber = VarGet("ZoneNumber");
	STRING zoneType = VarGet("ZoneType");
	char temp[500];

	sprintf(temp, "IncarnateInterfaceSlave%s%s_ActiveGlowie", zoneNumber, zoneType);
	MapSetDataToken(temp, -1);

	SET_STATE("Run");
}

void IncarnateInterfaceSlave_CheckOrders()
{
	STRING zoneNumber = VarGet("ZoneNumber");
	STRING zoneType = VarGet("ZoneType");
	char temp[500];
	int currentOrder;

	if (!stricmp(zoneNumber, "-1"))
	{
		printf("Illegal zone number.\n");
		return;
	}

	sprintf(temp, "IncarnateInterfaceSlave%s%s_Order", zoneNumber, zoneType);
	currentOrder = MapGetDataToken(temp);

	if (currentOrder == 1)
	{
		SET_STATE("Activate");
		MapSetDataToken(temp, 0);
	}

	if (currentOrder >= 2)
	{
		SET_STATE("Deactivate");
		MapSetDataToken(temp, currentOrder - 2);
	}
}

int IncarnateInterfaceSlave_GlowieInteract(ENTITY player, ENTITY target)
{
	NUMBER zoneNumber = atoi(VarGet("ZoneNumber"));
	char badgeName[50];
	sprintf(badgeName, "IncarnateInterfaceUnlockZone%i", zoneNumber);

	printf("IncarnateInterfaceSlave_GlowieInteract() called.\n");

	if (GlowieGetState(target) == MOS_INITIALIZED &&
		GetLevel(player) == 50 &&
		// and player owns x-pack
		!EntityHasBadge(player, badgeName))
	{
		return 0;
	}
	else
	{
		return 1; // error
	}
}

int IncarnateInterfaceSlave_GlowieComplete(ENTITY player, ENTITY target)
{
	NUMBER zoneNumber = atoi(VarGet("ZoneNumber"));
	char badgeName[50];
	sprintf(badgeName, "IncarnateInterfaceUnlockZone%i", zoneNumber);

	printf("IncarnateInterfaceSlave_GlowieComplete() called.\n");

	EntityGrantBadge(player, badgeName, false);
	return false; // don't turn off the glowie
}

void IncarnateInterfaceSlave_UpdateGlowies()
{
	STRING zoneNumber = VarGet("ZoneNumber");
	STRING zoneType = VarGet("ZoneType");
	NUMBER glowieCount = atoi(VarGet("GlowieCount"));
	NUMBER index;
	char temp[500];
	NUMBER activeGlowie;

	sprintf(temp, "IncarnateInterfaceSlave%s%s_ActiveGlowie", zoneNumber, zoneType);
	activeGlowie = MapGetDataToken(temp);

	// Turn off all glowies.
	for (index = 0; index < glowieCount; index++)
	{
		GlowieClearState(eventGlowies[index]);
	}

	// Turn on the active glowie.
	if (activeGlowie != -1)
	{
		GlowieSetState(eventGlowies[activeGlowie]);
	}
}

void IncarnateInterfaceSlave_Run()
{
	STRING zoneNumber = VarGet("ZoneNumber");
	STRING zoneType = VarGet("ZoneType");
	char temp[500];
	NUMBER activeGlowie;
	
	sprintf(temp, "IncarnateInterfaceSlave%s%s_ActiveGlowie", zoneNumber, zoneType);
	activeGlowie = MapGetDataToken(temp);

	if (activeGlowie == -1 || GlowieGetState(eventGlowies[activeGlowie]) != MOS_INITIALIZED)
	{
		IncarnateInterfaceSlave_UpdateGlowies();
	}
}

void IncarnateInterfaceSlave()
{
	STRING zoneNumber = VarGet("ZoneNumber");
	STRING zoneType = VarGet("ZoneType");
	NUMBER glowieCount = atoi(VarGet("GlowieCount"));
	NUMBER mapid;
	NUMBER master_mapid;
	NUMBER time;
	NUMBER master_time;
	NUMBER index;
	GLOWIEDEF glowie;
	ENTITY entity = 0;
	char temp[500];

	INITIAL_STATE
		ON_ENTRY
			EArrayCreate(&eventGlowies);
			for (index = 0; index < glowieCount; index++)
			{
				glowie = GlowieCreateDef(StringLocalize(VarGet("ControlName")), 
												VarGet("ControlModel"), 
												VarGet("ControlInteract"), VarGet("ControlInterrupt"), 
												VarGet("ControlComplete"), VarGet("ControlInteractBar"), 
												VarGetNumber("ControlInteractTime"));
				GlowieSetDescriptions(glowie, NULL, NULL);
				sprintf(temp, "marker:IncarnateInterfaceGlowie%i", index + 1);

				entity = GlowiePlace(glowie, temp, true, IncarnateInterfaceSlave_GlowieInteract, IncarnateInterfaceSlave_GlowieComplete);

				EArrayInsert(&eventGlowies, strdup(entity), index);
			}

			IncarnateInterfaceSlave_UpdateGlowies();
		END_ENTRY
		SET_STATE("Run");

	STATE("Run")
		mapid = GetMapID();
		sprintf(temp, "IncarnateInterfaceSlave%s%s_mapid", zoneNumber, zoneType);
		master_mapid = MapGetDataToken(temp);
		time = SecondsSince2000();
		sprintf(temp, "IncarnateInterfaceSlave%s%s_lastTickTime", zoneNumber, zoneType);
		master_time = MapGetDataToken(temp);

		if (mapid == master_mapid)
		{
			// I am the interpreter.
			IncarnateInterfaceSlave_CheckOrders();
			sprintf(temp, "IncarnateInterfaceSlave%s%s_lastTickTime", zoneNumber, zoneType);
			MapSetDataToken(temp, time);
		}
		else if (master_time + atoi(VarGet("InterpreterTimeout")) < time)
		{
			// I will become the interpreter.
			sprintf(temp, "IncarnateInterfaceSlave%s%s_mapid", zoneNumber, zoneType);
			MapSetDataToken(temp, mapid);
			sprintf(temp, "IncarnateInterfaceSlave%s%s_lastTickTime", zoneNumber, zoneType);
			MapSetDataToken(temp, time);
		}

		// Even if I am not the interpreter, run.
		IncarnateInterfaceSlave_Run();
		
	STATE("Activate")
		IncarnateInterfaceSlave_Activate();

	STATE("Deactivate")
		IncarnateInterfaceSlave_Deactivate();

	END_STATES

	ON_SIGNAL("Activate")
		SET_STATE("Activate")
	END_SIGNAL

	ON_SIGNAL("Deactivate")
		SET_STATE("Deactivate")
	END_SIGNAL
}

void IncarnateInterfaceSlaveInit()
{
	SetScriptName("IncarnateInterfaceSlave");
	SetScriptFunc(IncarnateInterfaceSlave);
	SetScriptType(SCRIPT_ZONE);

	SetScriptVar("InterpreterTimeout",		"600",	0); // 10 minutes

	SetScriptVar("ZoneNumber",				"-1",	0);
	SetScriptVar("ZoneType",				"H",	0);

	SetScriptVar("GlowieCount",				"0",	0);

	SetScriptVar("ControlName",				NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("ControlModel",			NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("ControlInteract",			NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("ControlInterrupt",		NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("ControlComplete",			NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("ControlInteractBar",		NULL,	SP_DONTDISPLAYDEBUG);
	SetScriptVar("ControlInteractTime",		NULL,	SP_DONTDISPLAYDEBUG);
}