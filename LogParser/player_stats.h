#ifndef _PLAYER_STATS_H
#define _PLAYER_STATS_H

#include "StashTable.h"
#include "overall_stats.h"
#include "textparser.h"
#include <string.h>

// id used if player is NOT on a task force
#define NO_TASKFORCE	(-1)	

typedef struct PlayerPos
{
    int time;
    F32 pos[3];
} PlayerPos;

PlayerPos* PlayerPos_Create(int time, F32 *pos);
void PlayerPos_Destroy( PlayerPos *hItem );

#define PLAYER_STRUCT_VERSION 1
typedef struct PlayerStats
{
	char * authName;
	char * playerName;
	int dbID;
	char * currentMap;
	int isConnected;
	int isPlaying;

	BOOL fullLevelStats;		// set to FALSE if parser thinks the player might be mid-way
								// through a level when their first log entry appears
								// This allows partial level times to be ignored 

	int lastConnectTime;
	int lastEntryTime;
	int lastLevelUpdateTime;

	int currentLevel;
	int currentXP;

	int accessLevel;

	int currentInfluence;

	int totalConnections;
	int totalTime;
	int teamID;
	int taskForceID;
	int firstMessageTime;
	int * totalLevelTime;
	int * totalLevelTimeMissions;

	int * xpPerLevel;
	int * xpPerLevelTeamed;
	int * xpPerLevelMissions;
	int * debtPayedPerLevel;
	int * influencePerLevel;


	int * rewardMOPerLevel;
	int * rewardMHPerLevel;
	int * deathsPerLevel;
	int * killsPerLevel;
	int * damageTakenPerLevel;
	int * damageDealtPerLevel;
	int * healTakenPerLevel;
	int * healDealtPerLevel;

	int * powerSets;
	char** powerSetNames;
	char** powerNames;
	int* powerSlots;

	int xpCheatLevel;		// level that the player cheated to get up to

	E_LastRewardAction lastRewardAction;	// used to guess what reward was for

	E_Origin_Type originType;
	E_Class_Type classType;

	int * contacts;
    StashTable cmdparse_calls;
    PlayerPos **poss;
} PlayerStats;

typedef struct PlayerStatsList {
	PlayerStats **pStatsList;	
} PlayerStatsList;

void playerStatsListSave(char * fname);
void playerStatsListLoad(char * fname);
void PlayerStatsInit(PlayerStats * pStats);

typedef struct PetStats
{
	int	villainType;	// Since it's not a player, it is mapped as a villain
	// Owner is either a villain or a player.  There is not currently a way to recurse through a list
	//  so these need to be the root owner
	PlayerStats * playerOwner;
	int villainOwner;
} PetStats;



PlayerStats * GetPlayer(const char * player);
PlayerStats * GetPlayerOrCreate(const char * player, int time, char * mapName);

PlayerStats * PlayerNew(int time, char * mapName, char * entityName);
void PlayerDelete(PlayerStats * pPlayer);
BOOL PlayerConnect(PlayerStats * pStats, int time, char * mapName, char * entityName);
BOOL PlayerResumeCharacter(PlayerStats * pStats, int time, char * mapName, char * entityName, int teamID, char * authName, char * ipAddress, int dbID);
BOOL PlayerDisconnect(PlayerStats * pStats, int time, char * mapName, char * entityName);
void PlayerUpdateLevelTime(PlayerStats * pStats, int time);
void PlayerUpdateConnectionTime(PlayerStats * pStats, int time);
// char *PlayerStats_MakeEntityName(PlayerStats * pStats);

#endif  // _PLAYER_STATS_H
