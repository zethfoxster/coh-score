/*\
 *
 *	pnpc.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	deals with persistent NPCs.  Basically any NPC created
 *  as the map server loads who stands still or goes through a definite
 *  AI script and always exists.  Usually attached to Contacts or used
 *  in tasks
 *
 */

#ifndef __PNPC_H
#define __PNPC_H

#include "storyarcinterface.h"

typedef struct PNPCDef PNPCDef;

typedef struct PNPCInfo
{
	const PNPCDef*	pnpcdef;
	ContactHandle	contact;
	U32				nextShoutTime;
} PNPCInfo;


void PNPCTouched(ClientLink* client, Entity* npc); // called when a NPC in the world is touched by a player
void PNPCPrintAllOnMap(ClientLink* client);

////////////////////////////////////////////////////////////////////////////
// private to story arc system

void PNPCLoad(void);
void PNPCReload(void);
void PNPCRespawnTick(void);

void PNPCTick(Entity* npc);			// called for persistentNPCs only

Entity* PNPCFindLocally(const char* pnpcName);
StoryLocation* PNPCFindLocation(const char* pnpcName);
int PNPCHeadshotIndex(const char* pnpcName);
const char* PNPCDisplayName(const char* pnpcName);

Entity* PNPCFindEntity(const char* pnpcName);


bool PNPCValidateData(TokenizerParseInfo pti[], void* structptr);

#endif // __PNPC_H
