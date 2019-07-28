/*\
 *
 * 	missionscript.h/c - Copyright 2003, 2004 Cryptic Studios
 * 		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	hooks for scripting into missions
 *
 */

#ifndef __MISSIONSCRIPT_H
#define __MISSIONSCRIPT_H

#include "storyarcinterface.h"

// *********************************************************************************
//  Running scripts
// *********************************************************************************

int VerifyMissionScripts(const MissionDef* mission, const char* logicalname);
void StartMissionScripts(const MissionDef* mission, ScriptVarsTable* vars);
U32 StartMissionScriptDef(const ScriptDef *pScriptDef, const MissionDef* mission, ScriptVarsTable* vars);

// *********************************************************************************
//  Hooks back into mission
// *********************************************************************************

void MissionSetOverrideStatus(const char* status);


#endif //__MISSIONSCRIPT_H
