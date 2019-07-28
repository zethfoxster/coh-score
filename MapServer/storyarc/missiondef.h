/*\
 *
 *	missiondef.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	functions to handle static mission defs, and
 *	verify them on load
 *
 */

#ifndef __MISSIONDEF_H
#define __MISSIONDEF_H

#include "storyarcinterface.h"

// *********************************************************************************
//  Load and parse mission defs
// *********************************************************************************

int VerifyAndProcessMissionDefinition(MissionDef* mission, const char* logicalname,int minlevel, int maxlevel);
void MissionPreload();

int MissionIsZoneTransfer(const MissionDef* def);

#endif //__MISSIONDEF_H