
#ifndef PNPCCOMMON_H
#define PNPCCOMMON_H

#include "gameData/store.h"
#ifdef SERVER
#include "scriptengine.h"
#endif

typedef struct PNPCDef
{
	const char* name;
	const char* filename;
	const char* model;			// not variable
	const char* displayname;	// not variable
	const char* contact;		// not variable
	const char* deprecated;		// DEPRECATED
	const char* ai;				// not variable
	const char** dialog;		// not variable
	int canlevel;				// not variable
	int canRegisterSupergroup;	// not variable
	int canTailor;
	int canRespec;
	int canAuction;				// this NPC acts as an auction house contact
	int noheadshot;
	const char** autoRewards;	// rewards given just by clicking on this guy
	const char* workshop;		// opens a workshop when clicked
	MultiStore store;			// not variable
	int talksToStrangers;

	const char** shoutDialog;	// not variable
	int	shoutFrequency;			// min time between shouts
	int shoutVariation;			// variation+frequency = max time between shouts

	const char **visionPhaseRawStrings;
	int bitfieldVisionPhases;	// stored here for ease of use and for PNPCs without Entities

	const char *exclusiveVisionPhaseRawString;
	int exclusiveVisionPhase;

	const char *wrongAllianceString;
	const char *villainDefName;
	int level;

#ifdef SERVER
	const ScriptDef **scripts;
#endif

} PNPCDef;

typedef struct PNPCDefList
{
	const PNPCDef** pnpcdefs;
} PNPCDefList;
extern SHARED_MEMORY PNPCDefList g_pnpcdeflist;

typedef struct VisionPhaseNames
{
	char** visionPhases;
} VisionPhaseNames;

extern SHARED_MEMORY VisionPhaseNames g_visionPhaseNames;
extern SHARED_MEMORY VisionPhaseNames g_exclusiveVisionPhaseNames;

void PNPCPreload(void);

const PNPCDef *PNPCFindDef(const char* pnpcName);
const PNPCDef *PNPCFindDefFromFilename(char *filename); // not hashed

#if CLIENT
Entity *PNPCFindEntityOnClient(const PNPCDef *pnpcDef); // not hashed
#endif // CLIENT

#endif // PNPCCOMMON_H
