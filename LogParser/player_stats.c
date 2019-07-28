//
// player.c
//

#include "player_stats.h"
#include "mathutil.h"
#include "memorypool.h"
#include "overall_stats.h"
#include "parse_util.h"
#include "zone_stats.h"
#include "common.h"
#include "team.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "utils.h"



ParseTable PlayerStatsEntryInfo[] = 
{
	{ "Name",		TOK_STRUCTPARAM | TOK_STRING(PlayerStats, playerName, 0)},
	{ "{",			TOK_START,		0 },
//	{ "DBID",		TOK_INT,	offsetof(PlayerStats, dbID),	0},
	{ "DBID",		TOK_INT(PlayerStats, dbID, 0)},
	{ "CurMap",		TOK_STRING(PlayerStats, currentMap,0)},
/*	{ "IsConn",		TOK_INT,		offsetof(PlayerStats, isConnected),	0},
	{ "IsPlay",		TOK_INT,		offsetof(PlayerStats, isPlaying),	0},
	{ "UFlLvl",		TOK_INT,		offsetof(PlayerStats, fullLevelStats),	0},
	{ "LConTm",		TOK_INT,		offsetof(PlayerStats, lastConnectTime),	0},
	{ "LEntTm",		TOK_INT,		offsetof(PlayerStats, lastEntryTime),	0},
	{ "LLvlTm",		TOK_INT,		offsetof(PlayerStats, lastLevelUpdateTime),	0},

	{ "CurLvl",		TOK_INT,		offsetof(PlayerStats, currentLevel),	0},
	{ "CurNXP",		TOK_INT,		offsetof(PlayerStats, currentXP),	0},
	
	{ "AccLvl",		TOK_INT,		offsetof(PlayerStats, accessLevel),	0},

	{ "CurInf",		TOK_INT,		offsetof(PlayerStats, currentInfluence),	0},

	{ "TtlCon",		TOK_INT,		offsetof(PlayerStats, totalConnections),	0},
	{ "TtlTim",		TOK_INT,		offsetof(PlayerStats, totalTime),	0},
	{ "TeamID",		TOK_INT,		offsetof(PlayerStats, teamID),	0},
	{ "TFrcID",		TOK_INT,		offsetof(PlayerStats, taskForceID),	0},
	{ "FMSTim",		TOK_INT,		offsetof(PlayerStats, accessLevel),	0},
*/
	{ "IsConn",		TOK_INT(PlayerStats, isConnected,0)},
	{ "IsPlay",		TOK_INT(PlayerStats, isPlaying,0)},
	{ "UFlLvl",		TOK_INT(PlayerStats, fullLevelStats,0)},
	{ "LConTm",		TOK_INT(PlayerStats, lastConnectTime,0)},
	{ "LEntTm",		TOK_INT(PlayerStats, lastEntryTime,0)},
	{ "LLvlTm",		TOK_INT(PlayerStats, lastLevelUpdateTime,0)},

	{ "CurLvl",		TOK_INT(PlayerStats, currentLevel,0)},
	{ "CurNXP",		TOK_INT(PlayerStats, currentXP,0)},
	
	{ "AccLvl",		TOK_INT(PlayerStats, accessLevel,0)},

	{ "CurInf",		TOK_INT(PlayerStats, currentInfluence,0)},

	{ "TtlCon",		TOK_INT(PlayerStats, totalConnections,0)},
	{ "TtlTim",		TOK_INT(PlayerStats, totalTime,0)},
	{ "TeamID",		TOK_INT(PlayerStats, teamID,0)},
	{ "TFrcID",		TOK_INT(PlayerStats, taskForceID,0)},
	{ "FMSTim",		TOK_INT(PlayerStats, accessLevel,0)},

	{ "TotLTm",		TOK_INTARRAY(PlayerStats, totalLevelTime)},
	{ "TotLTmM",	TOK_INTARRAY(PlayerStats, totalLevelTimeMissions)},

	{ "PLvlXP",		TOK_INTARRAY(PlayerStats, xpPerLevel)},
	{ "PLvlXT",		TOK_INTARRAY(PlayerStats, xpPerLevelTeamed)},
	{ "PLvlXM",		TOK_INTARRAY(PlayerStats, xpPerLevelMissions)},
	{ "PLvlDt",		TOK_INTARRAY(PlayerStats, debtPayedPerLevel)},
	{ "PLvlIn",		TOK_INTARRAY(PlayerStats, influencePerLevel)},
	{ "PLvlMO",		TOK_INTARRAY(PlayerStats, rewardMOPerLevel)},
	{ "PLvlMH",		TOK_INTARRAY(PlayerStats, rewardMHPerLevel)},
	{ "PLvlDh",		TOK_INTARRAY(PlayerStats, deathsPerLevel)},
	{ "PLvlKl",		TOK_INTARRAY(PlayerStats, killsPerLevel)},
	{ "PLDamT",		TOK_INTARRAY(PlayerStats, damageTakenPerLevel)},
	{ "PLDamD",		TOK_INTARRAY(PlayerStats, damageDealtPerLevel)},
	{ "PLHeaT",		TOK_INTARRAY(PlayerStats, healTakenPerLevel)},
	{ "PLHeaD",		TOK_INTARRAY(PlayerStats, healDealtPerLevel)},

//	{ "PwrSet",		TOK_INTARRAY(PlayerStats, powerSets)},

	{ "XPLCht",		TOK_INT(PlayerStats, xpCheatLevel,0)},
	{ "LstRew",		TOK_INT(PlayerStats, lastRewardAction,0)},
	{ "OrignT",		TOK_INT(PlayerStats, originType,0)},
	{ "ClassT",		TOK_INT(PlayerStats, classType,0)},

	{ "PwrSet",		TOK_STRINGARRAY(PlayerStats,powerSetNames)},
	{ "Powers",		TOK_STRINGARRAY(PlayerStats,powerNames)},
	{ "PwrSlt",		TOK_INTARRAY(PlayerStats,powerSlots)},

// Taken out for now, wasting a bunch of space
//	{ "Cntcts",		TOK_INTARRAY(PlayerStats, contacts)},

	{ "}",			TOK_END,			0 },
	{ 0 }
};

ParseTable PlayerStatsListInfo[] = {
	{ "Player",		TOK_STRUCT(PlayerStatsList, pStatsList, PlayerStatsEntryInfo)},
	{ 0 }
};

PlayerStatsList psList = {0};

void playerStatsListLoad(char * fname) {
	PlayerStats * pStats = NULL;
	int pcount = 0;

	// Clear the list if it already exists
	if (psList.pStatsList) {
		eaDestroy(&psList.pStatsList);
	}
	ParserLoadFiles(NULL, fname, NULL, 0, PlayerStatsListInfo, &psList);

	while(pStats = eaPop(&psList.pStatsList)) {
		PlayerStats * pStashedStats;
		if(!pStats->playerName) continue;
		// Add the player stats to the stashtable
		stashAddPointer(g_AllPlayers, pStats->playerName, pStats, false); //false?
		// Now get the pointer from the stats in the stashtable, to reattach to the team
		stashFindPointer(g_AllPlayers, pStats->playerName, &pStashedStats);
		assert(pStashedStats);
		StateReattachOldPlayer(pStashedStats);
		pcount++;
	}

	printf("Loaded %d players from %s\n",pcount,fname);
	// DEBUG - reclear struct, loading may be causing errors elsewhere
//	if (psList.pStatsList) {
//		ParserDestroyStruct(PlayerStatsListInfo, &psList);
//	}
}

void playerStatsListSave(char * fname) {
	StashElement element;
	StashTableIterator it;
	PlayerStats * pStats;
	int pcount = 0;
	char backupname[200];

	// Clear the list if it already exists
	if (psList.pStatsList) {
		eaDestroy(&psList.pStatsList);
	}

	// Iterate through the player list
	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it, &element)) {
		// Get the player stats
		pStats = stashElementGetPointer(element);

		// Make sure the timestamps are up-to-date
		PlayerUpdateConnectionTime(pStats, pStats->lastEntryTime);
		PlayerUpdateLevelTime(pStats, pStats->lastEntryTime);

		// Push the player onto the list
		eaPush(&psList.pStatsList,pStats);
		pcount++;
	}

	// Back up the old file
	sprintf(backupname,"%s%s",fname,".bak");
	fileForceRemove(backupname);
	rename(fname, backupname);

	// Write the file
	ParserWriteTextFile(fname, PlayerStatsListInfo, &psList,0,0);
	printf("Saved %d players to %s\n",pcount,fname);
}




void PlayerStatsInit(PlayerStats * pStats)
{
	memset(pStats, 0, sizeof(PlayerStats));

	pStats->taskForceID = NO_TASKFORCE;
}

static char *FixName(char const *entName)
{
    static char buf[MAX_NAME_LENGTH];
    char *tmp;
    if(tmp=strchr(entName,':'))
        strncpyt(buf,entName,1+tmp-entName);
    else
        strcpy_unsafe(buf,entName);
    return buf;
}


// parses the canonical name: 'charactername:authname'
PlayerStats * GetPlayer(const char * entName)
{
	PlayerStats * p = NULL;
	stashFindPointer(g_AllPlayers,FixName(entName), &p);
	return p;
}

// Returns null in the case that the passed character name includes a ':' (and therefore is a mob)
PlayerStats * GetPlayerOrCreate(const char * player, int time, char * mapName)
{
    char *tmp;
	PlayerStats * p = GetPlayer(player);
	if (p) return p;
    tmp = strchr(player, ':');
    if(tmp && isdigit(tmp[1])) // critters are name:<number>
       return NULL;
	return PlayerNew(time,mapName,(char*)player);
}


// entityName = canonical name of form 'character:authname'
PlayerStats * PlayerNew(int time, char * mapName, char * entityName)
{
    char name[128];
    char *auth = strchr(entityName,':');
    PlayerStats * pStats = StructAllocRaw(sizeof(PlayerStats));
    PlayerStatsInit(pStats);
    
    if(auth)
    {
        auth++;
        strncpyt(name,entityName,auth-entityName);
    }
    else
    {
        auth = "_noauth_";
        strcpy_unsafe(name,entityName);
    }
    
    
	// Fixed for playerName,currentMap being a pointer
	pStats->playerName = StructAllocString(name);
	pStats->currentMap = StructAllocString(mapName);
    pStats->authName = StructAllocString(auth);
    
	// Fixes for arrays being pointers
	eaiSetSize(&pStats->totalLevelTime, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->totalLevelTimeMissions, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->xpPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->xpPerLevelTeamed, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->xpPerLevelMissions, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->debtPayedPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->influencePerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->rewardMOPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->rewardMHPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->deathsPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->killsPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->damageTakenPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->damageDealtPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->healTakenPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->healDealtPerLevel, MAX_PLAYER_SECURITY_LEVEL);
	eaiSetSize(&pStats->powerSets, MAX_PLAYER_POWER_SETS);
    
	eaCreate(&pStats->powerSetNames);
	eaCreate(&pStats->powerNames);
	eaiCreate(&pStats->powerSlots);
    
	// Fixed for contact list being a int * to a 200 element array
	eaiSetSize(&pStats->contacts, MAX_CONTACTS);
    
	pStats->firstMessageTime = time;
	pStats->lastLevelUpdateTime = time;
    
	stashAddPointer(g_AllPlayers, name, pStats, true);
    
	return pStats;
}

void PlayerDelete(PlayerStats * pPlayer) {
	int i;

	ParserFreeString(pPlayer->playerName);
	ParserFreeString(pPlayer->currentMap);
	eaiDestroy(&(pPlayer->totalLevelTime));
	eaiDestroy(&(pPlayer->xpPerLevel));
	eaiDestroy(&(pPlayer->xpPerLevelTeamed));
	eaiDestroy(&(pPlayer->xpPerLevelMissions));
	eaiDestroy(&(pPlayer->debtPayedPerLevel));
	eaiDestroy(&(pPlayer->influencePerLevel));
	eaiDestroy(&(pPlayer->rewardMOPerLevel));
	eaiDestroy(&(pPlayer->rewardMHPerLevel));
	eaiDestroy(&(pPlayer->deathsPerLevel));
	eaiDestroy(&(pPlayer->killsPerLevel));
	eaiDestroy(&(pPlayer->damageTakenPerLevel));
	eaiDestroy(&(pPlayer->healTakenPerLevel));
	eaiDestroy(&(pPlayer->powerSets));
	eaiDestroy(&(pPlayer->contacts));

	// Free the power list stuff
	for (i = 0; i < eaSize(&pPlayer->powerSetNames); i++)
		if(pPlayer->powerSetNames[i]) ParserFreeString(pPlayer->powerSetNames[i]);

	for (i = 0; i < eaSize(&pPlayer->powerNames); i++)
		if(pPlayer->powerNames[i]) ParserFreeString(pPlayer->powerNames[i]);

	eaDestroy(&pPlayer->powerSetNames);
	eaDestroy(&pPlayer->powerNames);
	eaiDestroy(&(pPlayer->powerSlots));



	//free(pPlayer);
    StructDestroy(PlayerStatsEntryInfo,pPlayer);
}

BOOL PlayerConnect(PlayerStats * pStats, int time, char * mapName, char * entityName)
{
	if(pStats->isConnected == TRUE)
	{
		PlayerUpdateConnectionTime(pStats, pStats->lastEntryTime);
		PlayerUpdateLevelTime(pStats, pStats->lastEntryTime);
	}

	if(!pStats->firstMessageTime)
	{
		pStats->firstMessageTime = time;
	}

	ParserFreeString(pStats->currentMap);
	pStats->currentMap = StructAllocString(mapName);

	pStats->isConnected = TRUE;

	return TRUE;
}


BOOL PlayerResumeCharacter(PlayerStats * pStats, int time, char * mapName, char * entityName, int teamID, char * authName, char * ipAddress, int dbID)
{
	pStats->isPlaying			= TRUE;
	pStats->lastConnectTime		= time;
	pStats->lastLevelUpdateTime = time;
	if (pStats->dbID == 0)
		pStats->dbID = dbID;
	else 
		if (pStats->dbID != dbID) {
			fprintf(g_LogParserErrorLog,"dbID mismatch: %s had %d, resumed with %d\n",pStats->playerName,pStats->dbID,dbID);
            return FALSE;
		}

	return TRUE;
}



BOOL PlayerDisconnect(PlayerStats * pStats, int time, char * mapName, char * entityName)
{
	if(!stricmp(mapName, pStats->currentMap))
	{
		if(!pStats->isConnected)
			return TRUE;

		PlayerUpdateConnectionTime(pStats, time);
		PlayerUpdateLevelTime(pStats, time);

		// kick player from team!!
		if(pStats->teamID)
			RemovePlayerFromTeam(pStats,time);

		pStats->isConnected = FALSE;
		pStats->isPlaying = FALSE;
	}

	return TRUE;
}



void PlayerUpdateLevelTime(PlayerStats * pStats, int time)
{
	if(pStats->accessLevel > 0)
		return;	// skip stats for players with abnormal accesslevels

	if(    pStats->isPlaying)	// only update time if player actually entered map (via 'Connection:ResumeCharacter' msg)
	{
		int delta = (time - pStats->lastLevelUpdateTime);
		int level = pStats->currentLevel;

		if(level < pStats->xpCheatLevel)
		{
			//printf("\nPlayer %s stats will be ignored, as they skipped to level %d", pStats->playerName, pStats->xpCheatLevel);
			return;
		}

		// update global stats
		g_GlobalStats.totalLevelTime[level] += delta;

		// update player stats
		pStats->totalLevelTime[level] += delta;

		if(GetMapTypeIndex(pStats->currentMap)==MAP_TYPE_MISSION)
			pStats->totalLevelTimeMissions[level] += delta;

		// prep for next time
		pStats->lastLevelUpdateTime = time;
	}
}


void PlayerUpdateConnectionTime(PlayerStats * pStats, int time)
{
	if(pStats->accessLevel > 0)
		return;	// skip stats for players with abnormal accesslevels

	if(pStats->isPlaying)	// only update time if player actually entered map (via 'Connection:ResumeCharacter' msg)
	{
		int level;
		int delta = (time - pStats->lastConnectTime);
		ZoneStats * pZone = GetZoneStats(pStats->currentMap);

		E_Map_Type mapType = GetMapTypeIndex(pStats->currentMap);

		assert(time >= 0);

		assert(time >= 1000000);

//		fprintf(g_ConnectionLog, "\n%s\t%s, %s", sec2DateTime(time), pStats->playerName, sec2StopwatchTime(delta));

		// update player stats
		pStats->totalTime += delta;

		// update global stats
		g_GlobalStats.totalConnectionTime += delta;
		g_GlobalStats.totalMapTypeConnectionTime[mapType] += delta;  // mission/city/trial/hazard

		// update time spent on this map (increments of 5)
		level = pStats->currentLevel;
		if(level > 0)
		{
			pZone->timeSpentPerLevel[(pStats->currentLevel - 1) / 5] += delta; // get zero-based level
		}
		else
		{
			printf("Player (%s) level is zero!!!", pStats->playerName);
		}

		// setup for next time 
		pStats->lastConnectTime = time;
	}
}


// char *PlayerStats_MakeEntityName(PlayerStats * pStats)
// {
//     static char *buf = NULL;
//     estrPrintf(&buf,"%s:%s", pStats->playerName, pStats->authName);
//     return buf;
// }

MP_DEFINE(PlayerPos);
PlayerPos* PlayerPos_Create(int time, F32 *pos )
{
	PlayerPos *res = NULL;

	// create the pool, arbitrary number
	MP_CREATE(PlayerPos, 64); 
	res = MP_ALLOC( PlayerPos );
	if( res )
	{
        res->time = time;
        if(pos)
            copyVec3(pos,res->pos); 
	}
	return res;
}

void PlayerPos_Destroy( PlayerPos *hItem )
{
	if(hItem)
	{
		MP_FREE(PlayerPos, hItem);
	}
}
