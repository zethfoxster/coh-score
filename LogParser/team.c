
#include "team.h"
#include "player_stats.h"
#include "common.h"
#include "parse_util.h"

ParseTable TeamStatsEntryInfo[] = {
	{ "ID",		TOK_STRUCTPARAM | TOK_INT(Team, teamID, 0)},
	{ "{",			TOK_START,		0 },
	{ "LstAct",		TOK_INT(Team, lastAction,0)},
	{ "LCngTm",		TOK_INT(Team, lastChangedTime,0)},
	{ "XPAccm",		TOK_INT(Team, xpAccumulated,0)},
	{ "LvlTot",		TOK_INT(Team, levelTotal,0)},

	{ "MobKls",		TOK_INTARRAY(Team, mobKills)	},
	{ "PlrDth",		TOK_INT(Team, playerDeaths,0)},


	{ "}",			TOK_END,			0 },
	{ 0 }
};

ParseTable TeamStatsListInfo[] = {
	{ "Team",		TOK_STRUCT(TeamList, pTeamList, TeamStatsEntryInfo)},
	{ 0 }
};

TeamList ptList = {0};



void TeamStatsListLoad(char * fname) {
	Team * pTeam = NULL;
	int tcount = 0;

	// Clear the list if it already exists
	if (ptList.pTeamList) {
		eaDestroy(&ptList.pTeamList);
	}
	ParserLoadFiles(NULL, fname, NULL, 0, TeamStatsListInfo, &ptList);

	while(pTeam = eaPop(&ptList.pTeamList)) {
		// Wipe the array that keeps the handles to the players
		if (pTeam->players) {
			eaDestroy(&(pTeam->players));
			eaCreate(&(pTeam->players));
		}
		// Add the team
		stashAddPointer(g_AllTeams, int2str(pTeam->teamID), pTeam, true);
		tcount++;
	}

	printf("Loaded %d teams from %s\n",tcount,fname);
	// DEBUG - reclear struct, loading may be causing errors elsewhere
//	if (psList.pStatsList) {
//		ParserDestroyStruct(PlayerStatsListInfo, &psList);
//	}
}

void TeamStatsListSave(char * fname) {
	StashElement element;
	StashTableIterator it;
	Team * pTeam = NULL;
	int tcount = 0;
	char backupname[200];

	// Clear the list if it already exists
	if (ptList.pTeamList) {
		eaDestroy(&ptList.pTeamList);
	}

	// Iterate through the team list
	stashGetIterator(g_AllTeams, &it);
	while(stashGetNextElement(&it, &element)) {
		// Get the team
		pTeam = stashElementGetPointer(element);

		// Push the team onto the list
		eaPush(&ptList.pTeamList,pTeam);
		tcount++;
	}

	// Back up the old file
	sprintf(backupname,"%s%s",fname,".bak");
	fileForceRemove(backupname);
	rename(fname, backupname);

	// Write the file
	ParserWriteTextFile(fname, TeamStatsListInfo, &ptList,0,0);
	printf("Saved %d teams to %s\n",tcount,fname);
}


void StateReattachOldPlayer(PlayerStats * pPlayer) {

	if (pPlayer && pPlayer->teamID) {
		Team * pTeam = GetTeam(pPlayer->teamID);
		if(pTeam) {
			if (eaSizeUnsafe(&(pTeam->players)) >= 8)
				printf("Error reloading state, adding %s to full team %d\n",pPlayer->playerName, pPlayer->teamID);
			eaPush(&(pTeam->players), pPlayer);
		}
		else {
			printf("Error reloading state, adding %s to non-existant team %d!\n",pPlayer->playerName,pPlayer->teamID);
		}
	}
	
}

const char * GetTeamStr(int teamID, int curtime)
{
	int i,l;
	static char out[2000];
	static char plist[2000];
	static char ktemp[20];
	Team * pTeam;
	int size;
	stashFindPointer(g_AllTeams, int2str(teamID),&pTeam);
	size = eaSizeUnsafe(&pTeam->players);

	sprintf(out, "ID %d Sz %d Lvl %d Dth %d XP %d ES %d [ ", teamID, size,pTeam->levelTotal,pTeam->playerDeaths,pTeam->xpAccumulated,curtime - pTeam->lastChangedTime);

	for(i = 0; i < size; i++)
	{
		PlayerStats * pPlayer = pTeam->players[i];
		sprintf(plist,"%s(%d %d),",pPlayer->playerName,pPlayer->currentLevel,pPlayer->classType);
		strcat(out,plist);
	}
	strcat(out," ] KL ");
	for (l=0; l < MAX_PLAYER_SECURITY_LEVEL; l++) {
		int k = eaiGet(&pTeam->mobKills,l);
		if (k != 0) {
            sprintf(ktemp,"%d %d ",l,k);
			strcat(out, ktemp);
		}
	}

	return out;
}

// Dumps team info to the team log, automatically culls useless teams (empty or 0 time)
void DumpTeamToLog(int teamID, int curtime) {
	Team * pTeam;
	stashFindPointer(g_AllTeams, int2str(teamID), &pTeam);
	if ((eaSizeUnsafe(&pTeam->players) == 0) || ((curtime - pTeam->lastChangedTime) == 0))
		return;
	fprintf(g_TeamActivityLog, "%s\n", GetTeamStr(teamID,curtime));
}

// will assert if team does not exist
Team * GetTeam(int teamID)
{
	Team * pTeam;
	char teamName[100];
	sprintf(teamName, "%d", teamID);

	stashFindPointer(g_AllTeams, teamName, &pTeam);

	return pTeam;
}


Team * CreateTeam(int teamID)
{
	Team * pTeam;
	char teamName[100];
	sprintf(teamName, "%d", teamID);

	// make sure that team doesn't already exist
	assert(!stashFindPointer(g_AllTeams, teamName, &pTeam));

	pTeam = StructAllocRaw(sizeof(*pTeam)); 
	pTeam->teamID = teamID;
	eaCreate(&(pTeam->players));
	eaiCreate(&pTeam->mobKills);
	eaiSetSize(&pTeam->mobKills, MAX_PLAYER_SECURITY_LEVEL);
	pTeam->xpAccumulated = 0;
	pTeam->lastChangedTime = 0;
	pTeam->levelTotal = 0;
	pTeam->playerDeaths = 0;
	pTeam->lastAction = LAST_REWARD_ACTION_UNKNOWN;
	pTeam->createdOnDemand = FALSE;
	stashAddPointer(g_AllTeams, teamName, pTeam, true);

	return pTeam;
}

void DestroyTeam(Team * pTeam)
{
	int i;
	PlayerStats * pPlayer;

	if(pTeam)
	{
		// empty the team
		int size = eaSizeUnsafe(&pTeam->players);
		if (size > 0) {
			pPlayer = pTeam->players[0];
			if (pPlayer->teamID) 
//				fprintf(g_TeamActivityLog, "0 [Destroy] %s\n", GetTeamStr(pPlayer->teamID));
//				fprintf(g_TeamActivityLog, "%s\n", GetTeamStr(pPlayer->teamID,g_GlobalStats.lastTime));
				DumpTeamToLog(pPlayer->teamID,g_GlobalStats.lastTime);
		}

		for(i = 0; i < size; i++)
		{
			PlayerStats * pPlayer = pTeam->players[i];
			pPlayer->teamID = 0;
		}

		eaDestroy(&(pTeam->players));
		eaiDestroy(&(pTeam->mobKills));
        StructDestroy(TeamStatsEntryInfo,pTeam);
	}
}


void AddPlayerToTeam(PlayerStats * pPlayer, int teamID, int time)
{
	int i;

	if(teamID)	// don't assign anyone to team '0'
	{
		Team * pTeam = GetTeam(teamID);

		if(pTeam)
		{
			// see if he's already a member of this team
			int size = eaSizeUnsafe(&pTeam->players);
			for(i = 0; i < size; i++)
			{
				if(pTeam->players[i] == pPlayer)
				{
//					fprintf(g_TeamActivityLog, "Player %s was already on team %s\n", pPlayer->playerName, GetTeamStr(teamID));
					return;
				}
			}
		}
		else
		{
			pTeam = CreateTeam(teamID);
		}

		assert(pPlayer);

//		fprintf(g_TeamActivityLog, "%d [AddPlayer %s] %s\n", time, pPlayer->playerName, GetTeamStr(teamID));
//		fprintf(g_TeamActivityLog, "%s\n", GetTeamStr(teamID, time));
		DumpTeamToLog(teamID, time);

		eaPush(&(pTeam->players), pPlayer);
//		eaiPush(&(pTeam->playerIDs), pPlayer->dbID);
		pPlayer->teamID = teamID;


		ResetTeamStats(pTeam);
		pTeam->lastChangedTime = time;
		pTeam->levelTotal = CalculateLevel(pTeam);
//		assert(eaSize(&pTeam->players) <= 8);	// max team size is 8
	}
}


// only remove player from team if he's on it, and the team exists.
// This allows code to handle the craziness of the logging code
void RemovePlayerFromTeam(PlayerStats * pPlayer, int time)
{
	Team * pTeam = GetTeam(pPlayer->teamID);
	int i;

	if(pTeam)
	{
		for(i = 0; i < eaSizeUnsafe(&(pTeam->players)); i++)
		{
			if(pTeam->players[i] == pPlayer)
			{
				int old = pPlayer->teamID;
//				fprintf(g_TeamActivityLog, "%d [DropPlayer %s] %s\n", time, pPlayer->playerName, GetTeamStr(old));
//				fprintf(g_TeamActivityLog, "%s\n", GetTeamStr(old, time));
				DumpTeamToLog(old,time);
				eaRemove(&(pTeam->players), i);
				pPlayer->teamID = 0;

				ResetTeamStats(pTeam);
				pTeam->lastChangedTime = time;
				pTeam->levelTotal = CalculateLevel(pTeam);
//				fflush(g_TeamActivityLog);
				return;
			}
		}
	}
}




void EmptyAllTeams()
{
	StashElement element;
	StashTableIterator it;


	stashGetIterator(g_AllTeams, &it);
	while(stashGetNextElement(&it,&element))
	{
		Team * pTeam = stashElementGetPointer(element);
		DestroyTeam(pTeam);
	}

	stashTableClear(g_AllTeams);
}


// Increments the kill count at moblevel for team
void TeamAddMobKill(int teamID, int moblevel) {
	Team * pTeam = GetTeam(teamID);
	int ckc;
	int i;

	if (pTeam && (moblevel < MAX_PLAYER_SECURITY_LEVEL)) {
		ckc = eaiGet(&pTeam->mobKills,moblevel);
		eaiSet(&pTeam->mobKills,ckc+1,moblevel);
		for (i = eaSizeUnsafe(&pTeam->players)-1; i>=0; i--)
		{
			PlayerStats * pPlayer = pTeam->players[i];
			pPlayer->killsPerLevel[pPlayer->currentLevel]++;
		}
	}
}

// Increments the xp track for team
void TeamAddXP(int teamID, int xp) {
	Team * pTeam = GetTeam(teamID);
	if (pTeam) pTeam->xpAccumulated += xp;
}

// Increments the death track for team
void TeamAddDeath(int teamID) {
	Team * pTeam = GetTeam(teamID);
	if (pTeam) pTeam->playerDeaths += 1;
}

// Quick function to zero out a team's kill list
void ResetTeamStats(Team * pTeam) {
	int l;
	pTeam->xpAccumulated = 0;
	pTeam->playerDeaths = 0;
	for (l = 0; l < MAX_PLAYER_SECURITY_LEVEL; l++)
		eaiSet(&pTeam->mobKills,0,l);
}

// Performs the calculation to determine the team's "level"
int CalculateLevel(Team * pTeam) {
	int size;
	int lsum = 0;
	int i;

	if (pTeam) {
		size = eaSizeUnsafe(&pTeam->players);

		for(i = 0; i < size; i++) {
			PlayerStats * pPlayer = pTeam->players[i];
			lsum += pPlayer->currentLevel;
		}
	}
	return lsum;
}

// Called when a player levels, to adjust the level of the team
void TeamUpdateLevel(int teamID, int time) {
	Team * pTeam = GetTeam(teamID);
	if (pTeam) {
		int lsum = CalculateLevel(pTeam);
		// If our team's level total has changed, note it in the log, change it and reset stats
		if (lsum != pTeam->levelTotal) {
//			fprintf(g_TeamActivityLog, "%d [LevelAdjust %d] %s\n", time, lsum, GetTeamStr(teamID, time));
//			fprintf(g_TeamActivityLog, "%s\n", GetTeamStr(teamID, time));
			DumpTeamToLog(teamID,time);
			pTeam->levelTotal = lsum;
			pTeam->lastChangedTime = time;
			ResetTeamStats(pTeam);
		}
	}
}

