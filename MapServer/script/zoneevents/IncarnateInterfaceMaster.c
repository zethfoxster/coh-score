#include "IncarnateInterfaceEvent.h"

#define LOCATION_COUNT 500

NUMBER activatedLocations[LOCATION_COUNT];

bool IncarnateInterfaceMaster_Activate(NUMBER locationIndex)
{
	char temp[500];
	NUMBER order;

	if (activatedLocations[locationIndex])
	{
		// already activated
		return false;
	}

	sprintf(temp, "IncarnateInterfaceSlave%iH_Order", locationIndex + 1);
	order = MAX(0, MapGetDataToken(temp));

	if (order == IncarnateInterfaceSlaveOrder_Activate || order == IncarnateInterfaceSlaveOrder_Reactivate)
	{
		dbLog("IncarnateInterfaceMaster", 0, "Tried to activate location #%i (H) but it was already activated.", locationIndex + 1);
	}

	MapSetDataToken(temp, order + 1);

	sprintf(temp, "IncarnateInterfaceSlave%iV_Order", locationIndex + 1);
	order = MAX(0, MapGetDataToken(temp));

	if (order == IncarnateInterfaceSlaveOrder_Activate || order == IncarnateInterfaceSlaveOrder_Reactivate)
	{
		dbLog("IncarnateInterfaceMaster", 0, "Tried to activate location #%i (V) but it was already activated.", locationIndex + 1);
	}

	MapSetDataToken(temp, order + 1);

	sprintf(temp, "IncarnateInterfaceMaster_UpdatesSinceActive_%i", locationIndex + 1);
	MapSetDataToken(temp, 0);

	activatedLocations[locationIndex] = 1;

	dbLog("IncarnateInterfaceMaster", 0, "Activated location #%i.", locationIndex + 1);
	return true;
}

void IncarnateInterfaceMaster_Run()
{
	NUMBER time = SecondsSince2000();
	NUMBER change_time = MapGetDataToken("IncarnateInterfaceMaster_lastChangeTime");

	if (change_time + atoi(VarGet("MasterChangePeriod")) < time)
	{
		// update the locations.
		NUMBER activeLocationCount = atoi(VarGet("ActiveLocationCount"));
		NUMBER locationCount = atoi(VarGet("LocationCount"));
		NUMBER mapid;
		NUMBER locationIndex;
		NUMBER randomCount;
		char temp[500];
		NUMBER activatedCount = 0;
		NUMBER missStreak;
		NUMBER maxMissStreak = atoi(VarGet("MaxMissStreak"));

		// first, tell all the currently active slaves to deactivate.
		for (locationIndex = 0; locationIndex < activeLocationCount; locationIndex++)
		{
			sprintf(temp, "IncarnateInterfaceMaster_ActiveLocation%i", locationIndex);
			mapid = MapGetDataToken(temp);

			if (mapid != -1)
			{
				sprintf(temp, "IncarnateInterfaceSlave%iH_Order", mapid);
				MapSetDataToken(temp, IncarnateInterfaceSlaveOrder_Deactivate);

				sprintf(temp, "IncarnateInterfaceSlave%iV_Order", mapid);
				MapSetDataToken(temp, IncarnateInterfaceSlaveOrder_Deactivate);
			}
		}

		// second, clear the activatedLocations array.
		for (locationIndex = 0; locationIndex < LOCATION_COUNT; locationIndex++)
		{
			activatedLocations[locationIndex] = 0;
		}

		// then, increment all miss streak counts.
		for (locationIndex = 0; locationIndex < locationCount; locationIndex++)
		{
			NUMBER value;
			sprintf(temp, "IncarnateInterfaceMaster_UpdatesSinceActive_%i", locationIndex + 1);
			value = MAX(0, MapGetDataToken(temp));
			MapSetDataToken(temp, value + 1);
		}

		// next, run the streakbreaker to ensure all indices are activated.
		for (locationIndex = 0; locationIndex < locationCount && activatedCount < activeLocationCount; locationIndex++)
		{
			sprintf(temp, "IncarnateInterfaceMaster_UpdatesSinceActive_%i", locationIndex + 1);
			missStreak = MapGetDataToken(temp);

			if (missStreak >= maxMissStreak)
			{
				if (IncarnateInterfaceMaster_Activate(locationIndex))
				{
					sprintf(temp, "IncarnateInterfaceMaster_ActiveLocation%i", activatedCount);
					MapSetDataToken(temp, locationIndex);
					activatedCount++;
				}
			}
		}

		// finally, randomly activate zones.
		for (randomCount = 0; activatedCount < activeLocationCount && randomCount < 1000; randomCount++) // randomCount is to prevent an infinite loop
		{
			locationIndex = rand() % locationCount;
			if (IncarnateInterfaceMaster_Activate(locationIndex))
			{
				sprintf(temp, "IncarnateInterfaceMaster_ActiveLocation%i", activatedCount);
				MapSetDataToken(temp, locationIndex);
				activatedCount++;
			}
		}

		MapSetDataToken("IncarnateInterfaceMaster_lastChangeTime", time);
	}
}

void IncarnateInterfaceMaster_ResetMapTokens()
{
	NUMBER activeLocationCount = atoi(VarGet("ActiveLocationCount"));
	NUMBER locationCount = atoi(VarGet("LocationCount"));
	NUMBER index;
	char temp[500];

	MapSetDataToken("IncarnateInterfaceMaster_mapid", 0);
	MapSetDataToken("IncarnateInterfaceMaster_lastTickTime", 0);
	MapSetDataToken("IncarnateInterfaceMaster_lastChangeTime", 0);

	for (index = 0; index < activeLocationCount; index++)
	{
		sprintf(temp, "IncarnateInterfaceMaster_ActiveLocation%i", index);
		MapSetDataToken(temp, 0);
	}

	for (index = 1; index <= locationCount; index++)
	{
		sprintf(temp, "IncarnateInterfaceSlave%iH_Order", index);
		MapSetDataToken(temp, 0);

		sprintf(temp, "IncarnateInterfaceSlave%iV_Order", index);
		MapSetDataToken(temp, 0);

		sprintf(temp, "IncarnateInterfaceMaster_UpdatesSinceActive_%i", index);
		MapSetDataToken(temp, 0);
	}
}

void IncarnateInterfaceMaster()
{
	NUMBER mapid;
	NUMBER master_mapid;
	NUMBER time;
	NUMBER master_time;

	INITIAL_STATE
		ON_ENTRY
			if (MapGetDataToken("IncarnateInterfaceMaster_mapid") == -1)
			{
				IncarnateInterfaceMaster_ResetMapTokens();
			}
		END_ENTRY
		SET_STATE("Run");

	STATE("Run")
		mapid = GetMapID();
		master_mapid = MapGetDataToken("IncarnateInterfaceMaster_mapid");
		time = SecondsSince2000();
		master_time = MapGetDataToken("IncarnateInterfaceMaster_lastTickTime");

		if (mapid == master_mapid)
		{
			// I am the master.
			IncarnateInterfaceMaster_Run();
			MapSetDataToken("IncarnateInterfaceMaster_lastTickTime", time);
		}
		else if (master_time + atoi(VarGet("MasterTimeout")) < time)
		{
			// I will become the master.
			MapSetDataToken("IncarnateInterfaceMaster_mapid", mapid);
			MapSetDataToken("IncarnateInterfaceMaster_lastTickTime", time);
		}

	STATE("Recalc")
		MapSetDataToken("IncarnateInterfaceMaster_lastChangeTime", 0);
		SET_STATE("Run");

	STATE("Pause")

	STATE("Reset")
		IncarnateInterfaceMaster_ResetMapTokens();
		SET_STATE("Reset");

	END_STATES

	ON_SIGNAL("Run")
		SET_STATE("Run")
	END_SIGNAL

	ON_SIGNAL("Recalc")
		SET_STATE("Recalc")
	END_SIGNAL

	ON_SIGNAL("Pause")
		SET_STATE("Pause")
	END_SIGNAL

	ON_SIGNAL("Reset")
		SET_STATE("Reset")
	END_SIGNAL
}

void IncarnateInterfaceMasterInit()
{
	printf("IncarnateInterfaceMasterInit() called.\n");

	SetScriptName("IncarnateInterfaceMaster");
	SetScriptFunc(IncarnateInterfaceMaster);
	SetScriptType(SCRIPT_ZONE);

	SetScriptVar("MasterTimeout",		"600",		0); // 10 minutes
	SetScriptVar("MasterChangePeriod",	"86400",	0); // 24 hours
	SetScriptVar("MaxMissStreak",		"20",		0);

	SetScriptVar("ActiveLocationCount",	"2",		0);
	SetScriptVar("LocationCount",		"10",		0);
}