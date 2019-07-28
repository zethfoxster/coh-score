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

#include "pnpc.h"
#include "storyarcprivate.h"
#include "grouputil.h"
#include "comm_game.h"
#include "gameData/store_net.h"
#include "Npc.h"
#include "dbdoor.h"
#include "entgenCommon.h"
#include "ArrayOld.h"
#include "dbcomm.h"
#include "NpcServer.h"
#include "entai.h"
#include "langServerUtil.h"
#include "entPlayer.h"
#include "staticmapinfo.h"
#include "groupnetsend.h"
#include "Reward.h"
#include "TeamReward.h"
#include "costume.h"
#include "entity.h"
#include "group.h"
#include "groupProperties.h"
#include "character_combat.h"
#include "character_net_server.h"
#include "sgrpserver.h"
#include "entaivars.h"
#include "aiBehaviorPublic.h"
#include "pnpcCommon.h"
#include "basedata.h"
#include "baseserver.h"
#include "MessageStoreUtil.h"

StashTable g_localpnpcs = 0;		// entity info structures, indexed by pnpc name
int g_pnpccount = 0;

// 1 minute
#define PNPC_RESPAWN_TICKS 1800

typedef struct PNPCEntInfo {
	EntityRef entref;
	const PNPCDef *def;
	Mat4 spawnpos;
} PNPCEntInfo;

bool PNPCValidateData(TokenizerParseInfo pti[], PNPCDefList* plist)
{
	int i;
	int ret = 1;

	saUtilPreprocessScripts(pti, plist);
	for (i = 0; i < eaSize(&g_pnpcdeflist.pnpcdefs); i++)
	{
		PNPCDef* def = cpp_const_cast(PNPCDef*)(plist->pnpcdefs[i]);

		// Make sure the NPC has a displayname.
		if(!def->displayname)
		{
			char* message = "pnpcLoader: NPC (%s) has no display name. \n";
			ErrorFilenamef(def->filename, message, def->filename);
			ret = 0;
		}

		if(def->contact)
		{
			if(!ContactDefinition(ContactGetHandle(def->contact)))
			{
				char* message = "pnpcLoader: Contact (%s) refers to unknown contact definition (%s).\n";
				ErrorFilenamef(def->filename, message, def->filename, def->contact);
				ret = 0;
			}
		}

		if(def->store.ppchStores)
		{
			int i;

			def->store.iCnt = eaSize(&def->store.ppchStores);

			for(i=0; i<def->store.iCnt; i++)
			{
				if(!store_Find(def->store.ppchStores[i]))
				{
					char* message = "pnpcLoader: Contact (%s) refers to unknown store (%s).\n";
					ErrorFilenamef(def->filename, message, def->filename, def->store);
					ret = 0;
				}
			}
		}
	}

	return ret;
}


static void StartPNPCScripts(Entity *pnpc)
{
	EntityScriptsBegin(pnpc, pnpc->persistentNPCInfo->pnpcdef->scripts, eaSize(&pnpc->persistentNPCInfo->pnpcdef->scripts), NULL, pnpc->persistentNPCInfo->pnpcdef->filename);
}

static bool SpawnPNPC(PNPCEntInfo* info)
{
	const PNPCDef *def = info->def;
	const VillainDef *villainDef = 0;
	const char* displayname;
	Entity* ent;

	if (def->villainDefName)
		villainDef = villainFindByName(def->villainDefName);

	ent = npcCreate(def->model, villainDef ? ENTTYPE_CRITTER : ENTTYPE_NPC);
	if (!ent)
	{
		ErrorFilenamef(def->filename, "pnpcLoader: unable to find NPC definition for %s\n", def->model);
		return false;
	}

	// If this PNPC is a critter, assign villain def and set level
	if (villainDef)
		villainCreateInPlace(ent, villainDef, def->level, NULL, false, NULL, def->model, 0, NULL);

	entUpdateMat4Instantaneous(ent, info->spawnpos);	// set orientation too

	// other initializations
	ent->persistentNPCInfo = calloc(sizeof(PNPCInfo), 1);
	ent->persistentNPCInfo->pnpcdef = def;
	ent->bitfieldVisionPhasesDefault = def->bitfieldVisionPhases;
	ent->exclusiveVisionPhaseDefault = def->exclusiveVisionPhase;
	ent->persistentNPCInfo->contact = 0;
	ent->isContact = true;
	ent->canInteract = true;
	ent->fade = 1;

	// localized display name
	displayname = def->displayname;
	if (displayname)
		displayname = saUtilScriptLocalize(displayname);
	if (displayname)
		strcpy(ent->name, displayname);
	if (!displayname)
		strcpy(ent->name, "No name!");

	// hook up to contact
	if (def->contact)
	{
		ContactHandle handle;
		Contact * contact;
		char contactname[MAX_PATH];

		saUtilCleanFileName(contactname, def->contact);
		handle = ContactGetHandle(contactname);
		if (!handle)
		{
			ErrorFilenamef(def->filename, "pnpcLoader: unable to find contact %s\n", def->contact);
		}
		ent->persistentNPCInfo->contact = handle;
		contact = GetContact(handle);
		if (contact)
		{
			contact->pnpcParent = def;
			contact->entParent = ent;
			if (CONTACT_IS_PLAYERMISSIONGIVER(contact->def))
			{
				ent->contactArchitect = 1;
			}
		}
	}

	// set up AI
	aiInit(ent, NULL);
	if (def->ai) aiBehaviorAddString(ent, ENTAI(ent)->base, def->ai);

	StartPNPCScripts(ent);

	info->entref = erGetRef(ent);

	return true;
}

// spawns the npcs based on generator locations
int pnpcLoader(GroupDefTraverser* traverser)
{
	extern Array hashStackSpawnAreaDefs; // HACK - hooking into existing generator, remove later
	Pair* pickPercentageWeightedPair(Array* pairs);

	PropertyEnt* generatorProp;
	const PNPCDef* def;
	char buf[MAX_PATH];
	int i, n;
	Vec3 pyr;

	// find the spawn location
	if (!traverser->def->properties) {
		if (!traverser->def->child_properties)	// if none of our children have props, don't go further
			return 0;
		return 1;
	}
	stashFindPointer( traverser->def->properties, "PersistentNPC", &generatorProp );

	// if we have a location
	if (generatorProp)
	{
		PNPCEntInfo *newinfo;
		// look for the matching definition
		saUtilCleanFileName(buf, generatorProp->value_str);
		n = eaSize(&g_pnpcdeflist.pnpcdefs);
		def = NULL;
		for (i = 0; i < n; i++)
		{
			// the strstr is to advance past "scripts.loc/" which used to be stripped out 
			// as part of the preparsing.  The client doesn't know about scripts.loc though
			if (strstriConst(g_pnpcdeflist.pnpcdefs[i]->filename, buf))
			{
				def = g_pnpcdeflist.pnpcdefs[i];
				break;
			}
		}

		// error if we can't find a match
		if (!def)
		{
			char error[2000];
			sprintf(error,
				"Persistent NPC marker doesn't have corresponding .npc file (%s), coord (%0.2f,%0.2f,%0.2f)\n",
				buf, traverser->mat[3][0], traverser->mat[3][1], traverser->mat[3][2]);
			ErrorFilenamef(saUtilCurrentMapFile(), error);
			return 0;
		}

		// we're ok.  spawn npc immediately
		newinfo = calloc(sizeof(PNPCEntInfo), 1);
		newinfo->def = def;

		copyMat4(traverser->mat, newinfo->spawnpos);
		// MS: Turn around the generator yaw 180 degrees because all the generator geometries are BACK-FUCKING-WARDS!!!
		getMat3YPR(newinfo->spawnpos, pyr);
		pyr[1] = addAngle(pyr[1], RAD(180));
		pyr[2] = 0;
		createMat3YPR(newinfo->spawnpos, pyr);

		if (SpawnPNPC(newinfo)) {
			stashAddPointer(g_localpnpcs, def->name, newinfo, false);
			g_pnpccount++;
		} else {
			free(newinfo);
		}

		return 0; // don't look at children
	} // if generatorProp

	if (!traverser->def->child_properties)
		return 0;
	return 1; // continue through children
}

// verify we have all the pnpc's we expected to have
static void VerifyMapPNPCs(void)
{
	const char** requiredNPCs;
	int i, n = 0;
	
	requiredNPCs = MapSpecNPCList(saUtilCurrentMapFile());
	if (requiredNPCs)
		n = eaSize(&requiredNPCs);
	for (i = 0; i < n; i++)
	{
		if (!stashFindPointerReturnPointer(g_localpnpcs, requiredNPCs[i]))
		{
			ErrorFilenamef(saUtilCurrentMapFile(), "Map required to have a %s persistent npc, but does not", requiredNPCs[i]);
		}
	}
}

// look through map for spawn points of persistent npcs
void PNPCLoad(void)
{
	GroupDefTraverser traverser = {0};

	loadstart_printf("Loading persistent npcs.. ");
	if (g_localpnpcs) stashTableDestroy(g_localpnpcs);
	g_localpnpcs = stashTableCreateWithStringKeys(10, StashDeepCopyKeys);
	g_pnpccount = 0;
	groupProcessDefExBegin(&traverser, &group_info, pnpcLoader);
	VerifyMapPNPCs();
	loadend_printf("%i found", g_pnpccount);
}

static ClientLink* paom_client = NULL;
static FILE* paom_file = NULL;

static int PNPCPrintName(StashElement el)
{
	conPrintf(paom_client, "  %s\n", stashElementGetStringKey(el));
	if (paom_file)
	{
		char buf[200];
		sprintf(buf, "%s, ", stashElementGetStringKey(el));
		fwrite(buf, 1, strlen(buf), paom_file);
	}
	return 1;
}

void PNPCPrintAllOnMap(ClientLink* client)
{
	// print them
	paom_client = client;
	if (debugFilesEnabled())
	{
		paom_file = fopen(fileDebugPath("pnpclist.txt"), "wb");
		fwrite("PersistentNPCs ", 1, strlen("PersistentNPCs "), paom_file);
	}
	conPrintf(client, "%s\n", localizedPrintf(0,"NPCsOnMap"));
	stashForEachElement(g_localpnpcs, PNPCPrintName);
	if (paom_file)
	{
		fclose(paom_file);
		paom_file = 0;
	}
}

static void pnpcinfo_destroy(void* info)
{
	if (info)
		free(info);
}

void PNPCKillAll(void)
{
	int i;
	for(i = 1; i < entities_max; i++)
	{
		if ((entity_state[i] & ENTITY_IN_USE) && entities[i]->persistentNPCInfo)
		{
			entFree(entities[i]);
		}
	}
	stashTableClearEx(g_localpnpcs, NULL, pnpcinfo_destroy);
	g_pnpccount = 0;
}

void PNPCReload()
{
	saUtilPreload();
	PNPCPreload();
	PNPCKillAll();
	PNPCLoad();
}

// called when a NPC in the world is touched by a player
void PNPCTouched(ClientLink* client, Entity* npc)
{
	if (!npc->persistentNPCInfo) 
		return; // not one of mine

	// Try to finish a item delivery task if possible.
	client->entity->storyInfo->interactEntityRef = erGetRef(npc);
	client->entity->storyInfo->interactTarget = npc->persistentNPCInfo->contact;
	client->entity->storyInfo->interactDialogTree = 0;
	client->entity->storyInfo->interactCell = 0;
	client->entity->storyInfo->interactPosLimited = 1;
	copyVec3(ENTPOS(client->entity), client->entity->storyInfo->interactPos);
	if (DeliveryTaskTargetInteract(client->entity, npc, npc->persistentNPCInfo->contact))
		return;

	// If there were no item delivery tasks to be completed,
	// try interacting with the NPC normally.
	if (npc->persistentNPCInfo->contact)
	{
		ContactOpen(client->entity, ENTPOS(npc), npc->persistentNPCInfo->contact, npc);
	}
	else if (npc->persistentNPCInfo->pnpcdef != NULL)
	{
		int stranger;
		const MapSpec *mapSpec;

		mapSpec = MapSpecFromMapId(db_state.base_map_id);
		if(mapSpec)
		{
			if (npc->persistentNPCInfo->pnpcdef->canRegisterSupergroup) // only PNPC that shouldn't talk to tourists
			{
				stranger = ENT_IS_HERO(client->entity)
					? MAP_ALLOWS_VILLAINS_ONLY(mapSpec)
					: MAP_ALLOWS_HEROES_ONLY(mapSpec);
			}
			else
			{
				stranger = ENT_IS_ON_BLUE_SIDE(client->entity)
					? MAP_ALLOWS_VILLAINS_ONLY(mapSpec)
					: MAP_ALLOWS_HEROES_ONLY(mapSpec);
			}
		}
		else
		{
			stranger = 0;
		}

		if(!SAFE_MEMBER(npc->persistentNPCInfo->pnpcdef,talksToStrangers) && stranger)
		{
			ContactResponse response;
			memset(&response, 0, sizeof(ContactResponse));

			if (SAFE_MEMBER(npc->persistentNPCInfo->pnpcdef, wrongAllianceString))
			{
				strcat(response.dialog, saUtilLocalize(client->entity, npc->persistentNPCInfo->pnpcdef->wrongAllianceString));
			}
			else
			{
				strcat(response.dialog, textStd("DefaultWrongAllianceString"));
			}
			strcat(response.dialog, "<BR>");
			AddResponse(&response, saUtilLocalize(client->entity, "Leave"), CONTACTLINK_BYE);

			START_PACKET(pak, client->entity, SERVER_CONTACT_DIALOG_OPEN)
				ContactResponseSend(&response, pak);
			END_PACKET
			return;
		}

		if (npc->persistentNPCInfo->pnpcdef->canRegisterSupergroup)
		{
			sgroup_sendRegistrationList(client->entity,0,NULL);
		}
		else if(npc->persistentNPCInfo->pnpcdef->store.iCnt>0)
		{
			client->entity->pl->npcStore = npc->owner;
			store_SendOpenStorePNPC(client->entity, npc, &npc->persistentNPCInfo->pnpcdef->store);
		}
		else if (npc->persistentNPCInfo->pnpcdef->canTailor)
		{
			if( client->entity->pl->ultraTailor )
			{
				character_ShutOffAllTogglePowers(client->entity->pchar);
			}

			START_PACKET( pak, client->entity, SERVER_TAILOR_OPEN );
			pktSendBits( pak, 32, client->entity->pl->freeTailorSessions);
			pktSendBits( pak,  1, client->entity->pl->ultraTailor );
			pktSendBits( pak,  1, getRewardToken( client->entity, TAILOR_VET_REWARD_TOKEN_NAME ) ? 1 : 0 );
			pktSendBits( pak,  1, npc->persistentNPCInfo->pnpcdef->canTailor == 2 ? 1 : 0);
			END_PACKET 
		}
		else if (npc->persistentNPCInfo->pnpcdef->canAuction)
		{
			character_toggleAuctionWindow(client->entity, 1, npc->owner);
		}
		else if(npc->persistentNPCInfo->pnpcdef->canRespec && playerCanRespec(client->entity))
		{
			client->entity->pl->isRespeccing = 1;
 			START_PACKET( pak, client->entity, SERVER_RESPEC_OPEN );
			END_PACKET 
		}
		else if (npc->persistentNPCInfo->pnpcdef->workshop)
		{
			copyVec3(ENTPOS(npc), client->entity->pl->workshopPos);
			Strncpyt(client->entity->pl->workshopInteracting, npc->persistentNPCInfo->pnpcdef->workshop);
			sendActivateUpdate(client->entity, true, 0, kDetailFunction_Workshop);
		}
		else if (npc->persistentNPCInfo->pnpcdef->dialog)
		{
			if( !npc->IAmInEmoteChatMode ) //If the NPC is already saying something, don't say this
			{
				ScriptVarsTable vars = {0};
				int next = saRoll(eaSize(&npc->persistentNPCInfo->pnpcdef->dialog));
				U32 time = dbSecondsSince2000();

				if (npc->persistentNPCInfo->nextShoutTime < time)
				{
					ScriptVarsTablePushVarEx(&vars, "Hero", (char*)client->entity, 'E', 0);
					ScriptVarsTablePushVarEx(&vars, "HeroName", (char*)client->entity, 'E', 0);			// deprecated
					ScriptVarsTablePushVarEx(&vars, "Villain", (char*)client->entity, 'E', 0);
					ScriptVarsTablePushVarEx(&vars, "VillainName", (char*)client->entity, 'E', 0);		// deprecated

					npc->audience = erGetRef(client->entity);
					saUtilEntSays(npc, 1, saUtilTableAndEntityLocalize(npc->persistentNPCInfo->pnpcdef->dialog[next], &vars, client->entity));

					npc->persistentNPCInfo->nextShoutTime = time + 10;  // pnpcs will only shout when clicked on every 10 seconds
				}
			}
		}

		if (eaSize( &npc->persistentNPCInfo->pnpcdef->autoRewards ) )
		{
			rewardFindDefAndApplyToEnt(client->entity, npc->persistentNPCInfo->pnpcdef->autoRewards, VG_NONE, 1, false, REWARDSOURCE_PNPC_AUTO, NULL);
		}
	}
}

void PNPCTick(Entity* npc)
{
	if (!npc->persistentNPCInfo) return;
	if (npc->persistentNPCInfo->pnpcdef != NULL)
	{
		PNPCInfo* info = npc->persistentNPCInfo;
		const PNPCDef* def = info->pnpcdef;
		if (def->shoutDialog)
		{
			U32 time = dbSecondsSince2000();
			if (info->nextShoutTime < time)
			{
				int next = def->shoutFrequency;
				if (def->shoutVariation > 0)
					next += saSeedRoll(time, def->shoutVariation);
				info->nextShoutTime = time + next;

				next = saRoll(eaSize(&def->shoutDialog));
				saUtilEntSays(npc, kEntSay_NPC, saUtilScriptLocalize(def->shoutDialog[next]));
			}
		}
	} 
}



int PNPCHeadshotIndex(const char* pnpcName)
{
	int index;
	const PNPCDef* def = PNPCFindDef(pnpcName);
	if (!def) return 0;

	if (def->noheadshot)
		return 0;
	if (npcFindByName(def->model, &index))
		return index;
	return 0;
}

const char* PNPCDisplayName(const char* pnpcName)
{
	const PNPCDef* def = PNPCFindDef(pnpcName);
	if (!def) 
		return NULL;

	return saUtilScriptLocalize(def->displayname);
}

StoryLocation* PNPCFindLocation(const char* pnpcName)
{
	static StoryLocation loc;
	const PNPCDef* pnpc = NULL;
	DoorEntry* npcLoc = NULL;
	char* extension = NULL;
	char filename[1000];

	// What is the PNPC filename?
	pnpc = PNPCFindDef(pnpcName);

	// Todo -	Some storyarc system tried to locate an invalid pnpc.
	//			An error checking system should make sure that these kind of errors
	//			never happen.  This is a task that can never be completed since the
	//			pnpc doesn't exist.
	if(!pnpc)
		return NULL;

	// Lookup where the PNPC should be given the filename.
	//	Do not use the file extension as part of the lookup.
	strcpy(filename, pnpc->filename);
	extension = strrchr(filename, '.');
	if(extension)
		*extension = '\0';
	npcLoc = dbFindDoorWithName(DOORTYPE_PERSISTENT_NPC, filename);
	if(!npcLoc)
		return NULL;

	strcpy(loc.locationName, pnpc->displayname);
	loc.mapID = npcLoc->map_id;
	copyVec3(npcLoc->mat[3], loc.coord);
	return &loc;
}

Entity* PNPCFindEntity(const char* pnpcName)
{
	PNPCEntInfo *pi;
	Entity* e;
	const PNPCDef* def = PNPCFindDef(pnpcName);

	if (!def)
		return NULL;
	if ( !(pi = stashFindPointerReturnPointer( g_localpnpcs, def->name )) )
		return NULL;

	e = erGetEnt(pi->entref);
	if(e && e->persistentNPCInfo && e->persistentNPCInfo->pnpcdef == def)
		return e;

	return NULL;
}

Entity* PNPCFindLocally(const char* pnpcName)
{
	PNPCEntInfo *pi;

	if ( !(pi = stashFindPointerReturnPointer( g_localpnpcs, pnpcName )) )
		return NULL;

	return erGetEnt(pi->entref);
}

static int PNPCCheckForRespawn(StashElement element)
{
	PNPCEntInfo *pi = stashElementGetPointer(element);
	Entity *e;

	if (pi) {
		e = erGetEnt(pi->entref);
		if (!(e && e->persistentNPCInfo && e->persistentNPCInfo->pnpcdef == pi->def)) {
			// Entity either doesn't exist, or is a bad ref, so respawn it
			SpawnPNPC(pi);
		}
	}

	return 1;
}

// Check for any missing persistent NPCs and respawn them if necessary
void PNPCRespawnTick(void)
{
	static int checked = 0;

	if (checked++ < PNPC_RESPAWN_TICKS)
		return;

	checked = 0;
	stashForEachElement(g_localpnpcs, PNPCCheckForRespawn);
}