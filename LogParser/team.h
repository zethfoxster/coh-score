#ifndef _TEAM_H
#define _TEAM_H

#include "common.h"
#include "player_stats.h"

#define TEAM_STRUCT_VERSION 1
typedef struct Team
{
	int teamID;
	int lastAction;
	EArrayHandle players;
//	int * playerIDs;
	int * mobKills;
	int lastChangedTime;
	int xpAccumulated;
	int levelTotal;
	int playerDeaths;
	BOOL createdOnDemand;
}Team;

typedef struct TeamList {
	Team **pTeamList;	
} TeamList;



void TeamStatsListSave(char * fname);
void TeamStatsListLoad(char * fname);
void StateReattachOldPlayer(PlayerStats * pPlayer);

typedef struct PlayerStats PlayerStats;
void RemovePlayerFromTeam(PlayerStats * pPlayer, int time);
void AddPlayerToTeam(PlayerStats * pPlayer, int teamID, int time);

Team * GetTeam(int teamID);
Team * CreateTeam(int teamID);
void DestroyTeam(Team * pTeam);
void EmptyAllTeams();	
//void RemovePlayerFromTeam(PlayerStats * pPlayer);

const char * GetTeamStr(int teamID, int curtime);
void DumpTeamToLog(int teamID, int curtime);


// Team stats stuff
void TeamAddMobKill(int teamID, int moblevel);
void TeamAddXP(int teamID, int xp);
void TeamAddDeath(int teamID);
void ResetTeamStats(Team * pTeam);
int CalculateLevel(Team * pTeam);
void TeamUpdateLevel(int teamID, int time);


#endif // _TEAM_H