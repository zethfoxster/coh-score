/*\
*
*	sgraid.h/c - Copyright 2005 Cryptic Studios
*		All Rights Reserved
*		Confidential property of Cryptic Studios
*
*	Code to run a supergroup raid - probably hosted in a supergroup base
*
 */

#include "sgraid.h"
#include "cmdserver.h"
#include "svr_player.h"
#include "svr_base.h"
#include "character_base.h"
#include "comm_game.h"
#include "entity.h"
#include "missiongeoCommon.h"			// for ItemPosition struct - common to mission objectives and trophies
#include "storyarcprivate.h"
#include "grouputil.h"
#include "group.h"
#include "NpcServer.h"
#include "entai.h"
#include "dbdoor.h"
#include "entGameActions.h"
#include "arenamap.h"
#include "teamCommon.h"
#include "Supergroup.h"
#include "timing.h"
#include "raidstruct.h"
#include "container/container_store.h"
#include "dbcontainer.h"
#include "comm_backend.h"
#include "raidmapserver.h"
#include "bases.h"
#include "powers.h"
#include "baseserver.h"
#include "basesystems.h"
#include "door.h"
#include "badges_server.h"
#include "character_pet.h"
#include "dbnamecache.h"
#include "team.h"

typedef struct RaidPlayerScore
{
	int dbid;		// player's dbid
	int victories;	// number of times they've killed an opponent
	int defeats;	// number of times they have been killed
} RaidPlayerScore;

RaidPlayerScore ** raidScores;	// keep track of the players' scores, there won't be so many that
								// search through this array every time will cause problems


//	Didn't pick 1 because by convention, 1 is for coop maps
//	Didn't pick 2 because by convention, 2 is for hostages
#define GANG_ID_DEFENDERS 3
#define GANG_ID_ATTACKERS 4

PlayerEnts* activeplayers = &player_ents[PLAYER_ENTS_ACTIVE];

// new fields
U32*	baseraids = 0;
U32		running_baseraid = 0;
U32		complete_baseraid = 0;

// *********************************************************************************
//  Supergroup raid - new stuff
// *********************************************************************************

void RaidAttackerWarp(Entity* e, int override_id, int debug_print)
{
	int i = 0, numdoors;
	int id = e->db_id;
	if (override_id)
		id = override_id;

	numdoors = getNumberOfDoorways();
	if (numdoors)
		i = saSeedRoll(id+running_baseraid, numdoors);
	sendPlayerToDoorway(e, i, debug_print);
}

void RaidPlayerJoined(U32 dbid)
{
	RaidPlayerScore * rps;
	int i;

	// don't add this guy if he's already in the list of scores
	for (i=0;i<eaSize(&raidScores);i++)
		if (raidScores[i]->dbid == dbid)
			return;

	rps = malloc(sizeof(RaidPlayerScore));
	rps->dbid = dbid;
	rps->victories = 0;
	rps->defeats = 0;
	eaPush(&raidScores,rps);
}


void RaidInitPlayer(Entity* e)
{
	// players always lose the deploy pylon power when entering a new map
	//BasePower *ppowDeployPylon = powerdict_GetBasePowerByString(&g_PowerDictionary, 
	//	"Temporary_Powers.Temporary_Powers.Raid_Deploy_Pylon", NULL);
	//character_RemoveRewardPower(e->pchar, ppowDeployPylon);

	if (!OnSGBase()) return;

	if (e->supergroup_id == SGBaseId())
	{
		entSetOnGang(e, GANG_ID_DEFENDERS);
		RaidPlayerJoined(e->db_id);
	}
	else
	{
		// if not currently running a base raid, let them stay
		if (!running_baseraid)
		{
			entSetOnGang(e, GANG_ID_DEFENDERS);
		}

		if (running_baseraid)
		{
			int i;

			if (e->access_level < BASE_FREE_ACCESS && (e->supergroup_id != RaidAttackingSG() || !RaidPlayerIsAttacking(e)))
			{ //Kick out uninvolved players
				leaveMission(e,0,0);
				return;
			}

			// loop through everyone on the players team and kick them if they are on the map, but not on same team
			for( i = 0;e->teamup && i < e->teamup->members.count; i++ )
			{
				Entity * teammate = entFromDbId( e->teamup->members.ids[i] );
				if( teammate && teammate->supergroup_id != e->supergroup_id )
				{
					team_MemberQuit(e);
					break;
				}
			}

			entSetOnGang(e, GANG_ID_ATTACKERS);

			// give the Raid_Deploy_Pylon temporary power to attackers
			//character_AddRewardPower(e->pchar, ppowDeployPylon);

			// put the attacker in a random location
			RaidAttackerWarp(e, e->supergroup_id, 0);
			RaidPlayerJoined(e->db_id);
		}
	}

	if(running_baseraid)
	{
		if(e->pchar->iGangID == GANG_ID_DEFENDERS)
			character_SetPowerMode(e->pchar, g_eRaidDefenderMode);
		else
			character_SetPowerMode(e->pchar, g_eRaidAttackerMode);
	}
	else
	{
		// This should be redundant, but can't hurt.
		character_UnsetPowerMode(e->pchar, g_eRaidDefenderMode);
		character_UnsetPowerMode(e->pchar, g_eRaidAttackerMode);
	}

}

// any defenders who aren't participating 
void EjectNonDefenders(void)
{
	int i;
	for (i = 0; i < activeplayers->count; i++)
	{
		Entity* e = activeplayers->ents[i];
		if (e->supergroup_id != RaidDefendingSG() ||
			!RaidPlayerIsDefending(e))
		{
			leaveMission(e, 0, 0);
		}
	}
}

// all attackers have to be sent off the map
void RemoveAttackers(void)
{
	int i;
	for (i = 0; i < activeplayers->count; i++)
	{
		Entity* e = activeplayers->ents[i];
		if (e->supergroup_id == RaidAttackingSG())
		{
			// Destroy the pets early, or else they'll sit around for a tick or two dealing damage
			if(e->pchar) character_DestroyAllPets(e->pchar);
			leaveMission(e, 0, 0);
		}
	}
}

void RaidStart(U32 raidid)
{
	int i;
	if (running_baseraid)
	{
		Errorf("bad call to RaidStart");
		return;
	}
	running_baseraid = raidid;
	//EjectNonDefenders(); // done every tick, this kicked atacker if he was first to enter base
	ScriptBegin("BaseRaid", NULL, SCRIPT_ZONE, NULL, NULL, "");
	eaDestroyEx(&raidScores,NULL);	// clear this out before each match
	for (i = 0; i < activeplayers->count; i++)
	{
		Entity* e = activeplayers->ents[i];
		sendRaidState(e,1);

		RaidPlayerJoined(e->db_id);
		if(e->pchar->iGangID == GANG_ID_DEFENDERS)
			character_SetPowerMode(e->pchar, g_eRaidDefenderMode);
		else
			character_SetPowerMode(e->pchar, g_eRaidAttackerMode);
	}
	printf("Starting base raid %i\n", raidid);
}

void RaidStop(void)
{
	int i;
	if (!running_baseraid)
	{
		Errorf("bad call to RaidStop");
		return;
	}
	printf("Stopping base raid %i\n", running_baseraid);
	basePostRaidUpdate(&g_base, RaidIsInstant());
	ScriptEnd(ScriptRunIdFromName("BaseRaid", SCRIPT_ZONE), 0);
	RemoveAttackers();
	running_baseraid = 0;
	for (i = 0; i < activeplayers->count; i++)
	{
		Entity* e = activeplayers->ents[i];
		sendRaidState(e,0);

		character_UnsetPowerMode(e->pchar, g_eRaidDefenderMode);
		character_UnsetPowerMode(e->pchar, g_eRaidAttackerMode);
	}
}

static void RaidCompleteStats(int attackerswin, const char* why, bool isAttacker, U32 *parts)
{
	int i;
	char whyUnderscored[512];

	if( !why )
	{
		return;	
	}

	Strncpyt(whyUnderscored, why);
	strchrReplace(whyUnderscored,' ','_');

	for( i = 0; i < BASERAID_MAX_PARTICIPANTS; ++i ) 
	{
		U32 dbid = parts[i];
		if( dbid > 0 )
		{
			char *statAddCmd = NULL;

			estrCreate(&statAddCmd); 

			// won/lost
			estrPrintf( &statAddCmd, "badge_stat_add_relay \"%s.raid\" 1 %d", RAID_STAT(isAttacker, attackerswin), dbid );
			serverParseClient( statAddCmd, NULL );

			// won/lost.item_of_power etc.
			estrPrintf( &statAddCmd, "badge_stat_add_relay \"%s.raid.%s\" 1 %d", RAID_STAT(isAttacker, attackerswin), whyUnderscored, dbid );
			serverParseClient( statAddCmd, NULL );

			estrDestroy(&statAddCmd);
		}
	}
}

void SendRaidSummary(int attackerswin, const char * why)
{
	int i,j,k;
	ScheduledBaseRaid *raid = NULL;

	if (!running_baseraid)
		return;

	raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);

	if (raid)
	{
		int sendToAttackers;
		for (sendToAttackers=0;sendToAttackers<=1;sendToAttackers++)
		{
			for (i=0;i<BASERAID_MAX_PARTICIPANTS;i++)
			{
				int attacker;
				Entity * e=entFromDbId(sendToAttackers?(raid->attacker_participants[i]):(raid->defender_participants[i]));
				if (e==NULL)
					continue;
				START_PACKET(pak, e, SERVER_RAID_RESULT);
				pktSendString(pak,why);				// send reason for the raid ending
				pktSendBits(pak,1,attackerswin);	// and whether or not the attackers won
				for (attacker=1;attacker>=0;attacker--)	// attackers first
				{
					U32 * parts		= attacker ? raid->attacker_participants	: raid->defender_participants;
					int count=BaseRaidNumParticipants(raid,attacker);
					pktSendBitsAuto(pak,count);
					for (j=0;j<BASERAID_MAX_PARTICIPANTS;j++)
					{
						if (parts[j]==0)
							continue;

						for (k=0;k<eaSize(&raidScores);k++)
						{
							if (raidScores[k]->dbid == parts[j])
							{
								count--;
								pktSendString(pak,dbPlayerNameFromIdLocalHash(raidScores[k]->dbid));
								pktSendBitsAuto(pak,raidScores[k]->victories);
								pktSendBitsAuto(pak,raidScores[k]->defeats);
							}
						}
					}
					while (count--)	// if someone never logged on, send some blank data for them
					{
						pktSendString(pak,"");
					}
				}
				END_PACKET
			}
		}
	}
	eaDestroyEx(&raidScores,NULL);
}

void RaidComplete(int attackerswin, const char* why, const char * nameOfItemOfPowerCaptured, int creationTimeOfItemOfPowerCaptured )
{
	Packet* pak;
	ScheduledBaseRaid *raid = NULL;

	if (!running_baseraid)
		return;
	if (!why)
		why = "Unknown";
	if (!nameOfItemOfPowerCaptured )
		nameOfItemOfPowerCaptured = "";

	// ----------
	// update globals 

	complete_baseraid = running_baseraid;
	raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);

	SendRaidSummary(attackerswin,why);

	// ----------
	// raid server 

	// post complete message to raidserver
	pak = dbContainerRelayCreate(RAIDCLIENT_RAID_COMPLETE, CONTAINER_BASERAIDS, running_baseraid, 0, 0);
	pktSendBitsAuto(pak, SGBaseId());
	pktSendBits(pak, 1, attackerswin? 1: 0);
	pktSendString(pak, why);
	pktSendString(pak, nameOfItemOfPowerCaptured );
	pktSendBitsAuto(pak, creationTimeOfItemOfPowerCaptured );

	pktSend(&pak, &db_comm_link);

	// ----------
	// stats

	if( raid )
	{
		RaidCompleteStats( attackerswin, why, true, raid->attacker_participants );
		RaidCompleteStats( attackerswin, why, false, raid->defender_participants );
	}
}

int RaidIsInstant(void)
{
	if (running_baseraid)
	{
		ScheduledBaseRaid* raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);
		if (raid)
			return raid->instant;
	}
	return 0;
}

int RaidIsRunning(void) { return running_baseraid != 0; }

U32 RaidDefendingSG(void)
{
	ScheduledBaseRaid* raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);
	if (raid)
		return raid->defendersg;
	return 0;
}

U32 RaidAttackingSG(void)
{
	ScheduledBaseRaid* raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);
	if (raid)
		return raid->attackersg;
	return 0;
}

int RaidMapLevelOverride(Character* p)
{
	if (RaidIsRunning() && ENTTYPE(p->entParent)==ENTTYPE_PLAYER)
	{
		p->iCombatLevel = BASE_FORCED_LEVEL-1;
		return 1;
	}
	return 0;
}

int RaidTimeRemaining(void)
{
	ScheduledBaseRaid* raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);
	if (!raid)
	{
		Errorf("Couldn't find running base raid?");
		return MAX_BASERAID_LENGTH;
	}
	return raid->length - (dbSecondsSince2000() - raid->time);
}

// Attention float to all members of the sg in the raid, or all participants if sg==0
void RaidDisplayFloat(U32 sg, const char *message)
{
	int i;
	for (i = 0; i < activeplayers->count; i++)
	{
		Entity* e = activeplayers->ents[i];
		if(sg==0 || sg==e->supergroup_id)
		{
			serveFloater(e,e,message,0.0f,kFloaterStyle_Attention, 0);
		}
	}
}


// past the initial phase when attackers don't have to maintain a hold on the map
int RaidPastEntryPhase(void)
{
	ScheduledBaseRaid* raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);
	if (!raid)
	{
		Errorf("Couldn't find running base raid?");
		return 0;
	}
	return dbSecondsSince2000() > raid->time + BASERAID_ENTRY_PHASE;
}

void RaidHandleUpdate(int* ids, int deleting)
{
	int i; 
	U32 found = 0;
	U32 shouldrun = 0;
	U32 cur = dbSecondsSince2000();

	eaiCopy(&baseraids, &ids);

	for (i = 0; i < eaiSize(&baseraids); i++)
	{
		ScheduledBaseRaid* raid = cstoreGet(g_ScheduledBaseRaidStore, baseraids[i]);
		if (raid->defendersg == SGBaseId() &&
			cur >= raid->time &&
			cur < raid->time + 3600 && 
			!raid->complete_time &&
			raid->id != complete_baseraid)
		{
			shouldrun = baseraids[i];
		}
	}

	if (running_baseraid && !shouldrun)
		RaidStop();
	else if (!running_baseraid && shouldrun)
		RaidStart(shouldrun);
}

static void requestBaseUpdate(void)
{
	Packet* pak = dbContainerRelayCreate(RAIDCLIENT_BASE_UPDATE, CONTAINER_BASERAIDS, 0, 0, db_state.map_id);
	pktSendBitsPack(pak, 1, SGBaseId());
	pktSend(&pak, &db_comm_link);
}

#define UPDATE_FREQ		2.0
void RaidTick(void)
{
	static int timer = 0;
	if (!OnSGBase()) return;

	// TODO - make this neater
	if (!timer)
	{
		timer = timerAlloc();
		timerStart(timer);
	}
	if (timerElapsed(timer) > (UPDATE_FREQ))
	{
		requestBaseUpdate();
		timerStart(timer);
	}
}

void RaidMapHandleKill(Entity * killer,Entity * victim)
{
	ScheduledBaseRaid *raid = NULL;
	int i;
	int killerID = killer->db_id;
	int victimID = victim->db_id;

	// we don't want to award a kill to someone if the target
	// wasn't another player
	if (!running_baseraid || !killer || !victim || !victim->pl)
		return;

	raid = cstoreGet(g_ScheduledBaseRaidStore, running_baseraid);
	if (!raid)
		return;

	for (i=0;i<eaSize(&raidScores);i++)
	{
		if (raidScores[i]->dbid==killerID)
			raidScores[i]->victories++;
		if (raidScores[i]->dbid==victimID)
			raidScores[i]->defeats++;
	}
}
