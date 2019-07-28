/*\
 *
 * 	missionscript.h/c - Copyright 2003, 2004 Cryptic Studios
 * 		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	hooks for scripting into missions
 *
 */

#include "mission.h"
#include "storyarcprivate.h"


// *********************************************************************************
//  Running scripts
// *********************************************************************************

int VerifyMissionScripts(const MissionDef* mission, const char* logicalname)
{
	int i, n, ret = 1;

	n = eaSize(&mission->scripts);
	for (i = 0; i < n; i++)
	{
		const char* scriptName = mission->scripts[i]->scriptname;
        ScriptFunction* function = ScriptFindFunction(scriptName);
		if (!function || function->type != SCRIPT_MISSION)
		{
			ErrorFilenamef(mission->filename, "Mission %s called for script that doesn't exist or is wrong type: %s", 
				logicalname, scriptName);
			ret = 0;
		}
	}
	return ret;
}

void StartMissionScripts(const MissionDef* mission, ScriptVarsTable* vars)
{
	int i, n;
	n = eaSize(&mission->scripts);
	for (i = 0; i < n; i++)
	{
		const ScriptDef* script = mission->scripts[i];
		ScriptVarsTablePushScope(vars, &script->vs);
		ScriptBegin(script->scriptname, vars, SCRIPT_MISSION, (void*)mission, mission->filename, mission->filename);
		ScriptVarsTablePopScope(vars);
	}
}

U32 StartMissionScriptDef(const ScriptDef *pScriptDef, const MissionDef* mission, ScriptVarsTable* vars)
{
	U32 scriptId = 0;
	if(!pScriptDef || !mission || !vars)
	{
		Errorf("Invalid parameters to StartMissionScriptDef");
		return scriptId;
	}

	ScriptVarsTablePushScope(vars, &pScriptDef->vs);
	scriptId = ScriptBegin(pScriptDef->scriptname, vars, SCRIPT_MISSION, (void*)mission, "StartMissionScriptDef", mission->filename);
	ScriptVarsTablePopScope(vars);

	return scriptId;
}

void MissionSetOverrideStatus(const char* status)
{
	strncpyt(g_overridestatus, status, OVERRIDE_STATUS_LEN);
	MissionRefreshAllTeamupStatus();
}
