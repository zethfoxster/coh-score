//
//	process_entry.c
//

#include "process_entry.h"
#include "memorypool.h"
#include "player_stats.h"
#include "parse_util.h"
#include "StashTable.h"
#include "mathutil.h"
#include "Statistic.h"
#include "utils.h"
#include "zone_stats.h"
#include "power_stats.h"
#include "daily_stats.h"
#include "villain_stats.h"
#include "common.h"
#include "team.h"
#include <direct.h>



#define MAX_ENTRY_INTERVAL_TIME		300	// inactivity for more than this # of seconds will be ignored

StashTable g_String2ActionMap;
StashTable g_Action2StringMap;
int g_nextPeriodUpdateHour = 0;




// not the most efficient, but I need a quick way to determine if an entry is a supergroup action or not.
int IsSuperGroupAction(E_Action_Type action)
{
	return( action == ACTION_SUPERGROUP_MODE
		|| action == ACTION_SUPERGROUP_RANK
		|| action == ACTION_SUPERGROUP_MOTD
		|| action == ACTION_SUPERGROUP_CREATE
		|| action == ACTION_SUPERGROUP_DELETE
		|| action == ACTION_SUPERGROUP_ADD_PLAYER
		|| action == ACTION_SUPERGROUP_REMOVE_PLAYER
		|| action == ACTION_SUPERGROUP_LEADER
		|| action == ACTION_SUPERGROUP_RANKNAME
		|| action == ACTION_SUPERGROUP_COSTUME
		|| action == ACTION_SUPERGROUP_QUIT
		|| action == ACTION_SUPERGROUP_MOTTO
		|| action == ACTION_SUPERGROUP_KICK);
}

// not the most efficient, but I need a quick way to determine if an entry is a supergroup action or not.
int IsTaskForceAction(E_Action_Type action)
{
	return( action == ACTION_TASKFORCE_CREATE
		 || action == ACTION_TASKFORCE_ADD_PLAYER
		 || action == ACTION_TASKFORCE_REMOVE_PLAYER
		 || action == ACTION_TASKFORCE_DELETE);
}

void RegisterAction(char * name, EntryProcessingFunc func)
{
	int lookup;
	if(stashFindPointer(g_String2ActionMap, name, &lookup))
	{
		assert(!"registered 2x!!");
	}

	stashAddPointer(g_String2ActionMap, name, func, true);
    stashAddressAddPointer(g_Action2StringMap, func, name);
}

// cheesy hack: overload this to store function pointers as well
void RegisterAction2(char * name, EntryProcessingFunc func, char *name)
{
    if(!stashAddPointer(g_String2ActionMap, name, func, FALSE))
        Errorf("added %s twice", name);
}

void DisconnectActivePlayers()
{
	StashElement element;
	StashTableIterator it;
	PlayerStats * pStats;

	//printf("\nAuto-disconnecting players...\n");

	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it,&element))
	{
		pStats = stashElementGetPointer(element);
		if(pStats->isPlaying)
		{
			//printf("\nAuto-disconnecting %s", pStats->playerName);
			PlayerDisconnect(pStats,pStats->lastEntryTime, pStats->currentMap, pStats->playerName);
		}
	}
}


void CleanupActivePlayers()
{
	StashElement element;
	StashTableIterator it;
	PlayerStats * pStats;

//	fprintf(g_ConnectionLog, "\n%s\tCleaning up active players", sec2DateTime(g_GlobalStats.lastTime));


	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it,&element))
	{
		pStats = stashElementGetPointer(element);
		if(pStats->isPlaying)
		{
			printf("Updating Player Time (EOF reached w/o disconnecting) %s\n", pStats->playerName);
			PlayerUpdateLevelTime(pStats, pStats->lastEntryTime);
			PlayerUpdateConnectionTime(pStats, pStats->lastEntryTime);
		}
	}
}

void CleanupActiveZoneStats()
{
	StashElement element;
	StashTableIterator it;
	ZoneStats * pZone;

//	fprintf(g_ConnectionLog, "\n%s\tCleaning up active zone stats", sec2DateTime(g_GlobalStats.lastTime));

	stashGetIterator(g_AllZones, &it);
	while(stashGetNextElement(&it,&element))
	{
		pZone = stashElementGetPointer(element);

		pZone->activeEncounterCount = 0;
	}
}

const char * GetActionName(int action)
{
    char *res;
	assert(action >= 0 && action < ACTION_TYPE_COUNT);

    if(stashIntFindPointer(g_Action2StringTable,action,&res))
        return res;
    return NULL;
}

EntryProcessingFunc GetActionFunc(char *name)
{
    EntryProcessingFunc res;
    if(stashFindPointer(g_String2ActionMap,name,&res))
        return res;
    return NULL;
}



F32* ExtractPos(char const *str)
{
    static F32 pos[3];
    char *tmp;
    if(!str)
        return NULL;
    
    if(!STRSTRTOEND(tmp,str,"pos="))
        return NULL;
    sscanf(tmp,"<%f,%f,%f>",pos,pos+1,pos+2);
    return pos;
}


// used to compute perodic stats for zones
int GetHour(int time)
{
	// lowering this # will increase the # of data points
	// time / 60 is per minute
	return (time / 3600);	// may need to change later to adjust for time zone stuff
	
}
int ComputePeriod(int hour)
{
	int startHour = GetHour(g_GlobalStats.startTime);
	return (hour - startHour);
}

// MapServer:Connect
//   MapID: %map_id at IP: %ip_0/%ip_1 Port: %udp_port
//   Fails On: nothing
// Gets and resets the ZoneStats struct from the stash based on the zone name
BOOL ProcessMapserverConnect(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	ZoneStats * pZone = GetZoneStats(mapName);
	pZone->activeEncounterCount = 0;
	pZone->activePlayerCount	= 0;
	return TRUE;
}

// serverquit
//   trying to disconnect on map %mapnum
//   Fails On: nothing
// Empty handler
BOOL ProcessMapserverQuit(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// ChatServer:Connected
//   
//   Fails On: nothing
// ChatServer:Disconnected
//   
//   Fails On: nothing
// Empty handler
BOOL ProcessChatserver(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Connection:Connect
//   Connected client and resuming character From %ipaddr:%port
//   Connected client and creating character From %isaddr:%port
//   Fails On: nothing
// Connection:ResumeCharacter
//   From %ipaddr, AuthName '%authname' dbID: %dbid
//   Fails On: bad format, dbID mismatch with previous dbID
// Connection:Disconnect:Dropped
//   (Bad Connection) Removing link to entity %owner; %packets packets in output buffer
//   Fails On: nothing
// Connection:Disconnect
//   Fails On: nothing
// Connection:Logout
//   Fails On: nothing
// Handles various connection messages by creating, (re)connecting and disconnecting players
BOOL ProcessConnectionEntry(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pStats = GetPlayerOrCreate(entityName,time,mapName);

	switch(action)
	{
	case ACTION_CONNECTION_CONNECT:
		{
			ZoneStats * pZone = GetZoneStats(mapName);
			pZone->activePlayerCount++;
			PlayerConnect(pStats, time, mapName, entityName);
			return TRUE;
		}

	case ACTION_CONNECTION_RESUME_CHARACTER:
		{
			char authName[MAX_NAME_LENGTH];
			char ipAddress[MAX_NAME_LENGTH];
			int dbID;
			if(!ExtractTokens(desc, "From %n, AuthName %s dbID: %d", ipAddress, authName,&dbID)) {
				printf("Extracted %s %s %d\n",ipAddress,authName,dbID);
				fprintf(g_LogParserErrorLog,"Bad format: %s %s %d\n",ipAddress,authName,dbID);
				return FALSE;
			}
			else
				return PlayerResumeCharacter(pStats, time, mapName, entityName, teamID, authName, ipAddress, dbID);
		}
	case ACTION_CONNECTION_DISCONNECT:
	case ACTION_CONNECTION_DISCONNECT_DROPPED:
	case ACTION_CONNECTION_LOGOUT:
		{
			PlayerDisconnect(pStats,time, mapName, entityName);
			return TRUE;
		}

	default:
		return TRUE;
	}

}


// Entity:Resume:Character
//   %origin %class, Level:%seclevel XPLevel:%xplevel XP:%xp HP:%hp End:%end Debt: %debt Inf:%inf AccessLvl: %accesslevel
//   Fails On: Bad format
// Handles the resumption of a character by assuming all the passed in values are legit, creates new character if unknown
BOOL ProcessEntityResumeCharacter(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char origin[100];
	char className[100];
	int level, xp;
	float hp, end;
	int debt = 0;
	int inf = 0;
	int accessLvl = 0;
	int i;
	int dummy;

	PlayerStats * pStats = GetPlayerOrCreate(entityName,time,mapName);

	if( ! ExtractTokens(desc, "%s Class_%s, Level:%d XPLevel:%d XP:%d HP:%f End:%f Debt: %d Inf:%d AccessLvl: %d", origin, className, &dummy, &level, &xp, &hp, &end, &debt, &inf, &accessLvl)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}

	assert(level);
	
	pStats->accessLevel = accessLvl;
	pStats->currentXP	= xp;
	pStats->currentInfluence = inf;
	pStats->originType = GetOriginTypeIndex(origin);
	pStats->classType  = GetClassTypeIndex(className);	
	pStats->currentLevel = level;

	// erase all timing values for levels higher than current one
	// this covers 
	for(i=pStats->currentLevel +1; i < MAX_PLAYER_SECURITY_LEVEL; i++)
	{
		pStats->influencePerLevel[i] = 0;
		pStats->debtPayedPerLevel[i] = 0;
		pStats->rewardMOPerLevel[i] = 0;
		pStats->rewardMHPerLevel[i] = 0;
		pStats->killsPerLevel[i] = 0;
		pStats->deathsPerLevel[i] = 0;
		pStats->xpPerLevel[i] = 0;
		pStats->xpPerLevelTeamed[i] = 0;
		pStats->totalLevelTime[i] = 0;
		pStats->totalLevelTimeMissions[i] = 0;
	}

	return TRUE;
}

// Entity:MapMove
//   Going to %mapnum
//   Fails On: nothing
// Empty handler
BOOL ProcessEntityMapMove(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Entity:Create:Villain
//   Level %level %name (%rank)
//   Level %level %name (%rank) (wanted a level %requestedlevel)
//   Fails On: nothing
// Just a handler, does not currently do anything
BOOL ProcessEntityCreateVillain(int time, char * mapName, char * entityName, int teamID, const char * desc) {
	return TRUE;
}

// Entity:Create:Pet
//   Level %level %name doing %activty (owned by %owner)
//   Fails On: bad format
// If the pet is owner by a player, add it to the pet tracking stash
BOOL ProcessEntityCreatePet(int time, char * mapName, char * entityName, int teamID, const char * desc) {
	char newpetstash[1000];
	char petname[MAX_NAME_LENGTH];
	char petowner[1000];
	char petactivity[1000];
	int petlevel;
	PlayerStats *pStats;
	PetStats *petstats;
	const char *rest;
	char temp[MAX_NAME_LENGTH];

	// Gotta do funky shit here since the owner isn't quoted
	rest = ExtractTokens(desc, "Level %d %s doing %s (owned by", &petlevel,petname,petactivity);
	if (!rest || !rest[0]) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}
	// Switch the opening space to a left paren
	sprintf(temp,"(%s",rest);
	if(!ExtractTokens(temp, "%n",petowner)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}
	
	petstats = malloc(sizeof(PetStats));
	pStats = GetPlayer(petowner);

	if (pStats) {
		// Owned by a player!
		petstats->playerOwner = pStats;
		petstats->villainOwner = 0;
		petstats->villainType = 0;
		// Stick it in the pet stash
		sprintf(newpetstash,"%s_%s",mapName,entityName);
		stashAddPointer(g_PetTracker,newpetstash,petstats,true);
	}
	else {
		// Owned by a villain (or a pet)
	}
	return TRUE;
}




// Entity:RecordKill:Player
//   Victim: %player
//   Fails On: bad format
// Notes in the daily stats that the villain killed some player, and record that the player died
BOOL ProcessEntityRecordKillPlayer(int time, char * mapName, char * entityName, int teamID, const char * desc) 
{
	char victimName[MAX_NAME_LENGTH];
	int villainType;
	char villainTypeName[MAX_NAME_LENGTH];
	DailyStats * pDaily = GetCurrentDay(time);
	PlayerStats *pStats;

	if(!ExtractTokens(desc, "Victim: %n", victimName)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}

	if(GetVillainTypeFromName(entityName, villainTypeName))
	{
		villainType = GetIndexFromString(&g_VillainIndexMap, villainTypeName);
		pDaily->villain_stats[villainType].player_kills++;
	}

	pStats = GetPlayerOrCreate(victimName,time,mapName);
	if(pStats)
	{
		int level = pStats->currentLevel;

		g_GlobalStats.deathsPerLevel[level]++;
		pStats->deathsPerLevel[level]++;

		// record daily stats
		pDaily->deaths_per_level[pStats->currentLevel][pStats->classType]++;
		if (pStats->teamID) TeamAddDeath(pStats->teamID);
	}

	return TRUE;
}
// Entity:RecordKill
//   Victim: %victim (Lvl %level)
//   Fails On: bad format
// Notes in the daily stats that the player killed a villain
BOOL ProcessEntityRecordKill(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char victimName[MAX_NAME_LENGTH];
	int level;
	int villainType;
	char villainTypeName[MAX_NAME_LENGTH];
	DailyStats * pDaily = GetCurrentDay(time);
	PlayerStats * pPlayer = GetPlayerOrCreate(entityName,time,mapName);

	//[Entity:RecordKill] Victim: "Gravedigger Chopper:2971" (Lvl 8)
	if(!ExtractTokens(desc, "Victim: %n (Lvl %d)", victimName, &level))
	{
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}

	if(GetVillainTypeFromName(victimName, villainTypeName))
	{
		villainType = GetIndexFromString(&g_VillainIndexMap, villainTypeName);
		pDaily->villain_stats[villainType].deaths++;
	}

	// An NPC could have killed another NPC due to confuse
	if (pPlayer) 
	{
		SetLastRewardAction(pPlayer, LAST_REWARD_ACTION_KILL);
		if (pPlayer->teamID) 
			TeamAddMobKill(pPlayer->teamID,level);
		else
			pPlayer->killsPerLevel[pPlayer->currentLevel]++;
	}

	return TRUE;
}

// Entity:Die:Pet
//   (owned by %owner) (%xpos %ypos %zpos)
//   Fails On: nothing
// Removes a pet from the pet stash
BOOL ProcessEntityDiePet(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	void * pv;
	char stash[1000];
	sprintf(stash,"%s_%s",mapName,entityName);
	stashRemovePointer(g_PetTracker,stash,&pv);
	return TRUE;
}

// Entity:Die
//   (%xpos %ypos %zpos)
//   Fails On: nothing
// Does nothing, deaths handled in Entity:RecordKill and Entity:RecordKill:Player
BOOL ProcessEntityDie(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}


// Entity:Fly
// Entity:Immobilized
// Entity:Knocked
// Entity:Stunned
// Entity:Resurrect
// Entity:Sleep
// Entity:Intangible
//   ???
//   Fails On: nothing
// Does nothing
BOOL ProcessEntityEffect(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

BOOL ProcessEntityPeriodicInfo(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
    PlayerStats *p = GetPlayer(entityName);
    F32 *pos;
    PlayerPos *pp = NULL;
    if(!p)
        return TRUE; // not necessarily a bad line, may just not have this entity
    pos = ExtractPos(desc);
    if(pos)
    {
        pp = PlayerPos_Create(time,pos);
        eaPush(&p->poss,pp);
    }

    return TRUE;
}

// Power:ToggleOn
//   Toggle %powername
//   Fails On: nothing
// Power:ToggleOff
//   Toggle %powername
//   Fails On: nothing
// Does nothing, as powers that end up getting activated still get a Power:Activate dblog
BOOL ProcessPowerToggle(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Power:UseInspiration
//   %inspname
//   Fails On: nothing
// Does nothing
BOOL ProcessPowerUseInspiration(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Power:Queue
//   id:%powerid %powertype %powername targeting %target (%xpos %ypos %zpos)
//   Fails On: nothing
// Does nothing, as powers that end up getting activated still get a Power:Activate dblog
BOOL ProcessPowerQueue(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Power:Activate
//   id:%powerid %powertype %powername targeting %target (%xpos %ypos %zpos)
//   Fails On: Bad format
// Tracks power use and activation for damage tracking
BOOL ProcessPowerActivate(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	DailyStats * day = GetCurrentDay(time);
	int powerIndex, powerID;
	char powerName[MAX_POWER_NAME_LENGTH];
	char powerType[100];
	PlayerStats * pStats;
	
	if(!ExtractTokens(desc, "id:%d %s %n", &powerID, powerType, powerName)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}
	
	powerIndex = GetIndexFromString(&g_PowerIndexMap, powerName);
	day->power_stats[powerIndex].activated++;

	pStats = GetPlayer(entityName);
	if (pStats)	trackPowerActivate(powerID,mapName,pStats,NULL, time);
	else trackPowerActivate(powerID,mapName,NULL,entityName, time);

	return TRUE;
}

// Power:Cancelled
//   Not enough endurance: has %cur_end, needs %need_end for %powertype %powername
//   Fails On: nothing
// Does nothing
BOOL ProcessPowerCancelled(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Power:Hit
//   id:%powerid target %target
//   Fails On: nothing
// Does nothing
BOOL ProcessPowerHit(int time, char * mapName, char * entityName, int teamID, const char * desc)
{

	return TRUE;
}

// Power:Miss
//   id:%powerid target %target
//   Fails On: nothing
// Does nothing
BOOL ProcessPowerMiss(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Power:Effect
//   id: %powerid Cur.%damagetype %damage
//   Fails On: bad format
// Most frequent dblog entry, notes the damage, puts it in the tracker
BOOL ProcessPowerEffect(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char powerEffectType[200];
	int powerID, powerEffectSize, damageIndex;
	PlayerStats* pPlayer;
	DailyStats * pDaily = GetCurrentDay(time);

	if(!ExtractTokens(desc, "id:%d Cur.%n %d", &powerID, powerEffectType, &powerEffectSize)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}

	damageIndex = GetIndexFromString(&g_DamageTypeIndexMap, powerEffectType);
	pPlayer = GetPlayer(entityName);
	if (pPlayer) {
		g_HourlyStats[pPlayer->currentLevel][pPlayer->classType].damage[damageIndex] += abs(powerEffectSize);
		pDaily->damage_per_level[pPlayer->currentLevel][damageIndex] += abs(powerEffectSize);
		if (powerEffectSize < 0) {
			// Taking damage
			pPlayer->damageTakenPerLevel[pPlayer->currentLevel] -= powerEffectSize;
		}
		else {
			// Taking healing
			pPlayer->healTakenPerLevel[pPlayer->currentLevel] += powerEffectSize;
		}
	}

	if (powerEffectSize) trackPowerEffect(powerID,mapName,damageIndex,powerEffectSize,time);

	return TRUE;
}

// Power:Windup
//   %powertype %powername targeting %target (%xpos %ypos %zpos).
//   Fails On: nothing
// Empty handler
BOOL ProcessPowerWindup(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Power:Interrupted
//   %powertype %powername
//   Fails On: nothing
// Empty handler
BOOL ProcessPowerInterrupted(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Character:SetBoost
//   Boost %boostname.%level+%inc to %idx
//   Fails On: nothing
// Character:DestroyBoost
//   %idx
//   Fails On: nothing
// Character:CombineEnhancement
//   Boost %boostname.%level+%inc to %idx
//   Fails On: nothing
// Power:ReplaceBoost
//   Boost %boostname.%level+%inc to Boost %boostname.%level
//   Fails On: nothing
// Power:DestroyBoost
//   %powertype %powername.%idx
//   Fails On: nothing
// Empty handler
BOOL ProcessEnhancements(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Reward:LevelUp
//   New Level: %level
//   Fails On: bad format
// Dumps old level's stats
BOOL ProcessRewardLevelUp(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayerOrCreate(entityName, time, mapName);
	int oldLevel;
	int newLevel;
	int levelTime;

	if(! ExtractTokens(desc, "New Level: %d", &newLevel)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}

	PlayerUpdateLevelTime(pPlayer, time);
	oldLevel = pPlayer->currentLevel;

	if(oldLevel)	// make sure we've read in the player's level
	{

		levelTime = pPlayer->totalLevelTime[oldLevel];
		if(levelTime)  // don't count if time == 0, as we're not counting their time for some reason)
		{
/*			if((oldLevel +1) != (newLevel))
			{
				printf("ERROR: old (%d) and new (%d) levels are not consecutive\n", oldLevel, newLevel);
				return FALSE;
			}
*/
			if(pPlayer->fullLevelStats)
			{
				StatisticRecord(&(g_GlobalStats.completedLevelTimeStatistics[oldLevel]), levelTime); 
				g_GlobalStats.minimumXPforLevel[pPlayer->currentLevel] = MIN(pPlayer->currentXP, g_GlobalStats.minimumXPforLevel[pPlayer->currentLevel]);
			}
		}
	}

	pPlayer->currentLevel = newLevel;
	pPlayer->fullLevelStats = TRUE;
	if (pPlayer->teamID) TeamUpdateLevel(pPlayer->teamID,time);

	return TRUE;
}

// Reward:Points
//   XP: %xp, Debt: %debt, Inf: %inf
//   Fails On: bad format
// Records xp and influence gains in way too many different ways
BOOL ProcessRewardPoints(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	//[Reward:Points] XP: 5, Debt: 5, Inf: 6
	int xp, debt, inf, level, rewardaction;
	Team * pTeam;
	PlayerStats *pPlayer = GetPlayerOrCreate(entityName, time, mapName);

	if(! ExtractTokens(desc, "XP: %d, Debt: %d, Inf: %d", &xp, &debt, &inf)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}

	level = pPlayer->currentLevel;

	g_GlobalStats.xpPerLevel[level]		   += xp;
	g_GlobalStats.debtPayedPerLevel[level] += debt;

	// Add this xp to the player's team, if they're on one (xp and debt)
	if (pPlayer->teamID) TeamAddXP(pPlayer->teamID,xp+debt);

	// Assign the income to the action
	rewardaction = pPlayer->lastRewardAction;
	if (rewardaction == LAST_REWARD_ACTION_MISSION_COMPLETE) 
		rewardaction = LAST_REWARD_ACTION_TASK_COMPLETE;

	g_GlobalStats.influencePerLevel[level][rewardaction] += inf;

	// Track hourly stats for this level/class
	g_HourlyStats[level][pPlayer->classType].influenceIncome[rewardaction] += inf;
	g_HourlyStats[level][pPlayer->classType].xpNormal[rewardaction] += xp;
	g_HourlyStats[level][pPlayer->classType].xpDebt[rewardaction] += debt;
	
	pPlayer->currentInfluence += inf;

	pPlayer->currentXP += xp;
	pPlayer->xpPerLevel[level] += xp;
	// If the player is on a team of size > 1
	if (pPlayer->teamID > 0) {
		pTeam = GetTeam(teamID);
		if (pTeam && (1 < eaSizeUnsafe(&pTeam->players)))
			pPlayer->xpPerLevelTeamed[level] += xp;
	}
	else {
		// If the player is on a task force
		if (pPlayer->taskForceID > 0)
			pPlayer->xpPerLevelTeamed[level] += xp;
	}

	// If the player is on a mission map
	if(GetMapTypeIndex(pPlayer->currentMap)==MAP_TYPE_MISSION)
		pPlayer->xpPerLevelMissions[level] += xp;
	
	pPlayer->debtPayedPerLevel[level] += debt;
	pPlayer->influencePerLevel[level] += inf;

	return RecordInfluenceReward(pPlayer, inf, time);
}

// Character:AddDebt
//   %debt, now %totaldebt
//   %debt, now %totaldebt (capped)
//   Fails On: none
// Empty Handler, debt tracked through reward points
BOOL ProcessCharacterAddDebt(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Reward:AddInspiration
//   received %insp
//   did not receive %insp (full inventory)
//   Fails On: nothing
// Just a handler, does not currently do anything
BOOL ProcessRewardAddInspiration(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
/*	char inspiration[200];

	if(! ExtractTokens(desc, "received %s", inspiration))
		return FALSE;

	//IncrementCount(g_Inspiration)
	*/
	return TRUE;
}

// Reward:AddSpecialization
//   received %boost
//   did not receive %boost (full inventory)
//   did not receive %boost (exemplared)
//   Fails On: nothing
// Just a handler, does not currently do anything
BOOL ProcessRewardAddSpecialization(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	//IncrementCount(g_Enhancements, name);
	return TRUE;
}

// Reward:AddTempPower
//   received %powername
//   Fails On: nothing
// Reward:RemoveTempPower
//   removed %powername
//   Fails On: nothing
// Empty Handler
BOOL ProcessRewardTempPower(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Reward:StoryReward
//   %reward given by task %task
//   Fails On: bad format
// Tracks MissionOwner and MissionHelper rewards
BOOL ProcessRewardStoryReward(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char reward[100];
	char task[100];
	PlayerStats *pPlayer;
	if(!ExtractTokens(desc, "%s given by task %s", reward, task))
		return FALSE;

	pPlayer = GetPlayer(entityName);
	if(pPlayer)
	{
		if(0==strcmp(reward,"MissionOwner"))
			pPlayer->rewardMOPerLevel[pPlayer->currentLevel] += 1;
		else if(0==strcmp(reward,"MissionHelper"))
			pPlayer->rewardMHPerLevel[pPlayer->currentLevel] += 1;
	}

	return TRUE;
}

// Reward:MissionReward
//   %player being sent reward %reward
//   Fails On: bad format
// Tracks MissionOwner and MissionHelper rewards
BOOL ProcessRewardMissionReward(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char reward[100];
	char player[100];
	PlayerStats *pPlayer;
	if(!ExtractTokens(desc, "%s being sent reward %s", player, reward))
		return FALSE;

	pPlayer = GetPlayer(entityName);
	if(pPlayer)
	{
		if(0==strcmp(reward,"MissionOwner"))
			pPlayer->rewardMOPerLevel[pPlayer->currentLevel] += 1;
		else if(0==strcmp(reward,"MissionHelper"))
			pPlayer->rewardMHPerLevel[pPlayer->currentLevel] += 1;
	}

	return TRUE;
}

// Trade:Receive:Influence
//   %inf
//   Fails On: nothing
// Trade:Receive:Inspiration
//   Inspiration %insp
//   Fails On: nothing
// Trade:Receive:Enhancement
//   Boost %boost %level
//   Fails On: nothing
// Empty Handler
BOOL ProcessTrade(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Mission:Attach
//   Attaching team %teamid to map %mapnum
//   Fails On: nothing
// Just a handler, does not currently do anything
BOOL ProcessMissionAttach(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Mission:Success
// Mission:Failure
//  Name: %filename %logicalname, Time to Complete: %timeStr (%mapname %mapStr %mapLength)
//  Fails On: Bad format
// Notes that the player in question just finished a mission, for reward tracking
// Tracks mission completion information
//  - Tracks mission map information
BOOL ProcessMissionComplete(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	//Name: CONTACTS/TUTORIAL/TASKS/CONTACT4_TASK.TASKSET Tutorial4, Time to Complete: 1:46 (maps/Missions/Abandoned_Office/Abandoned_Office_15/Abandoned_Office_15_Layout_01_01.txt AbandonedOffice 15)
	char missionSet[400];
	char missionName[100];
	char timeStr[100];
	char missionMapName[500];
	char * shortMapName;
	char mapType[100];
	int mapLength;
	int completionTime;
	PlayerStats *pPlayer;
	MissionCompletionStats *pMission;
//	int teamSize;
//	Team * pTeam;

	if(! ExtractTokens(desc, "Name: %n %n, Time to Complete: %n (%s.txt %s %d)", missionSet, missionName, timeStr, missionMapName, mapType, &mapLength ))
		return FALSE;


	pPlayer = GetPlayer(entityName);
	if(pPlayer)
		SetLastRewardAction(pPlayer, LAST_REWARD_ACTION_MISSION_COMPLETE);

	// convert string time to integer
	completionTime = stopwatchTime2Sec(timeStr);

	if (!stashFindPointer(g_MissionCompletion, missionName, &pMission)) {
		pMission = malloc(sizeof(MissionCompletionStats));
		memset(pMission,0,sizeof(MissionCompletionStats));
		stashAddPointer(g_MissionCompletion,missionName,pMission,true);
	}

	if (action == ACTION_MISSION_SUCCESS) {
		pMission->countSuccess++;
		pMission->timeSuccess += completionTime;
	}
	else if (action == ACTION_MISSION_FAILURE) {
		pMission->countFailure++;
		pMission->timeFailure += completionTime;
	}

	// trim off directory stuff
	shortMapName = getFileName(missionMapName);

	strcatf(mapType, "_%d", mapLength);	// add time to map type

	//pTeam = stashFindValue(g_AllTeams, int2str(teamID));
	//if(!pTeam)
	//{
	//	printf("\nError: Team %d does not exist in stash", teamID);
	//	return FALSE;
	//}
	//else
	{
		//teamSize = eaSize(&(pTeam->players));
		//RecordMissionTime(g_GlobalStats.missionTimesStash, missionName, completionTime, teamSize);
		//RecordMissionTime(g_GlobalStats.missionMapTimesStash, shortMapName, completionTime, teamSize);
		//RecordMissionTime(g_GlobalStats.missionTypeTimesStash, mapType, completionTime, teamSize);
		PlayerStats * pPlayer = GetPlayer(entityName);
		if(pPlayer)
		{
			int taskForceID = pPlayer->taskForceID;

			if(taskForceID != NO_TASKFORCE)
			{
				FILE * file;
				char filename[MAX_FILENAME_SIZE];
				char * openMode = "a+t";

				StashElement element;
				StashTableIterator it;
				static int s_count = 0;

				sprintf(filename, "completed_taskforce_missions.txt");

				if(s_count == 0)
				{
					s_count = 1;
					openMode = "w+t";
				}

				file = fopen(filename, openMode);
				assert(file);

				fprintf(file, "%s %s %s: ", sec2DateTime(time), missionName, mapType); 

				// count # of players per level
				stashGetIterator(g_AllPlayers, &it);
				while(stashGetNextElement(&it,&element))
				{
					PlayerStats * pPlayer = stashElementGetPointer(element);
					if(pPlayer->taskForceID == taskForceID)
						fprintf(file, "%s,  ", pPlayer->playerName);
				}

				fprintf(file, "\n");

				fclose(file);
			}
		}
	}

	return TRUE;
}




















// Team:Leader
//   %player is new leader (previous leader (%player) left)
//   Fails On: nothing
// Empty Handler
BOOL ProcessTeamLeader(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}

// Team:Kick
//   %s kicked.
//   Fails On: nothing
// Just a handler, team removal tracked in Team:Remove
BOOL ProcessTeamKick(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}










// Character:AssignSlots
//   Assigned slot to %powertype %powername
//   Fails On: bad format, bad power name
// Empty handler
BOOL ProcessAssignSlots(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char powertype[20];
	char powername[MAX_POWER_NAME_LENGTH];
	char * powerfinalname = NULL;
	PlayerStats *pPlayer = GetPlayerOrCreate(entityName,time,mapName);

	if(!ExtractTokens(desc, "Assigned slot to %n %n", powertype, powername)) {
		fprintf(g_LogParserErrorLog,"Bad format:\n");
		return FALSE;
	}

	// find the last period
	if (powerfinalname = strrchr(powername,'.')) {
		int i;
		for (i=0; i<eaSize(&(pPlayer->powerNames)); i++) {
			if (!strcmp(&powerfinalname[1],pPlayer->powerNames[i])) {
                pPlayer->powerSlots[i]++;
				break;
			}
		}
		return TRUE;
	}

	fprintf(g_LogParserErrorLog,"Bad power name:\n");
	return FALSE;
}













BOOL ProcessCharacterCreate(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayer(entityName);
	if(pPlayer) {
		// The player already exists!
		RemovePlayerFromTeam(pPlayer,time);
		stashRemovePointer(g_AllPlayers,pPlayer->playerName,&pPlayer);
		// Clean the memory
		PlayerDelete(pPlayer);
	}
	// Make a new player entry
	PlayerNew(time,mapName,entityName);
	pPlayer = GetPlayer(entityName);
	pPlayer->fullLevelStats = TRUE;	

	return TRUE;
}




void RecordMissionTime(StashTable table, char * key, int value, int teamSize)
{
	Statistic * p;

	if(!stashFindPointer(table, key, &p))
	{
		int i;
		p = malloc((MAX_TEAM_SIZE + 1) * sizeof(Statistic));
		for(i = 0; i < (MAX_TEAM_SIZE + 1); i++)
		{
			StatisticInit(&p[i]);
		}
		stashAddPointer(table, key, p, true);
	}

	teamSize = MINMAX(teamSize, 1, MAX_TEAM_SIZE);
	StatisticRecord(&p[teamSize], value);
	StatisticRecord(&p[0], value);	// index 0 is used for "Total" team sizes
}





BOOL ProcessTaskComplete(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayer(entityName);
	if (!pPlayer) {
		printf("Player %s does not exist (creating)\n", entityName);
		fprintf(g_LogParserErrorLog,"Player %s does not exist (creating)\n",entityName);
		pPlayer = PlayerNew(time, mapName, entityName);
	}
	SetLastRewardAction(pPlayer, LAST_REWARD_ACTION_TASK_COMPLETE);
	return TRUE;
}

BOOL ProcessEncounter(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char groupName[200];
	char positionStr[100];
	ZoneStats * pZone;
	EncounterStats * pEncounter;
/*
	int minTime = dateTime2Sec("040219", 0, 0, 0);	// "Encounter:Closed" messages do not exist before this date

	if(time < minTime)
	{
		return TRUE;		// skip this entry if it was made before "Encounter:Closed" messages existed
	}
*/
	//[Encounter:Completed] Group (null) (-2786.34, 4.81, 2623.05)
	if(! ExtractTokens(desc, "Group %s %s", groupName, positionStr))
		return FALSE;


	pZone = GetZoneStats(mapName);

	if(!stashFindPointer(pZone->encounterActivityStash, positionStr,&pEncounter))
	{
		pEncounter = malloc(sizeof(EncounterStats));
		memset(pEncounter, 0, sizeof(EncounterStats));
		stashAddPointer(pZone->encounterActivityStash, positionStr, pEncounter, true);
	}
	
	if(action == ACTION_ENCOUNTER_SPAWN)
	{
		if(!pEncounter->isSpawned)
		{		
			pEncounter->totalSpawns++;
			pZone->currentEncounterSpawnCount++;
		}

		pEncounter->isSpawned = TRUE;

	}
	else if(action == ACTION_ENCOUNTER_COMPLETED)
	{
		pEncounter->totalComplete++;
	}
	else if(action == ACTION_ENCOUNTER_ACTIVE)
	{
		if(!pEncounter->isActive)
		{		
			pEncounter->totalActive++;
			pZone->currentEncounterActiveCount++;
		}
		pEncounter->isActive = TRUE;
	}
	else if(action == ACTION_ENCOUNTER_INACTIVE)
	{
		if(pEncounter->isActive)
		{
			pZone->currentEncounterActiveCount--;
			pEncounter->isActive = FALSE;
		}
	}	
	else if(action == ACTION_ENCOUNTER_DESPAWN)
	{
		if(pEncounter->isActive)
		{
			pZone->currentEncounterActiveCount--;
			pEncounter->isActive = FALSE;
		}

		if(pEncounter->isSpawned)
		{
			pZone->currentEncounterSpawnCount--;
			pEncounter->isSpawned = FALSE;
		}

		if(pZone->activeEncounterCount < 0)
		{
			pZone->currentEncounterActiveCount = 0;
			printf("Running active encounter count for map '%s' was below 0!\n", mapName);
		}

		if(pZone->currentEncounterSpawnCount < 0)
		{
			pZone->currentEncounterSpawnCount = 0;
			printf("Running encounter spawn count for map '%s' was below 0!\n", mapName);
		}

	}

	return TRUE;
}


BOOL ProcessCmdParse(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char cmdName[200];
	const char * rest;
    PlayerStats * pPlayer = GetPlayer(entityName);

	rest = ExtractTokens(desc, "%s", cmdName);
		
	if(!rest)
		return FALSE;

	if(   !stricmp(cmdName, "levelup")
	   || !stricmp(cmdName, "levelupxp"))
	{
		int level;

		if(!pPlayer)
			return FALSE;

		if(!ExtractTokens(rest, "%d", &level))
			return FALSE;

		pPlayer->xpCheatLevel = max(pPlayer->xpCheatLevel,level);
		printf("Player %s cheated to level %d\n", pPlayer->playerName, pPlayer->xpCheatLevel);
	}
	else if(!stricmp(cmdName, "influence"))
	{
		int value;
		int delta;
		DailyStats * day = GetCurrentDay(time);

		if(!pPlayer)
			return FALSE;
		
		if(!ExtractTokens(rest, "%d", &value))
			return FALSE;

		// this command only matters if the number input is GREATER than the current level
		delta = value - pPlayer->currentInfluence;


		if(delta > 0)
		{
			day->influence.createdByCommand += delta;
		}
		else
		{
			day->influence.destroyedByCommand += delta;
		}

		pPlayer->currentInfluence = value;
	}

    // log commands called by each player
    if(pPlayer)
    {
        if(!pPlayer->cmdparse_calls)
            pPlayer->cmdparse_calls = stashTableCreateWithStringKeys(128, StashDefault);
        stashAddOrModifyInt(pPlayer->cmdparse_calls,cmdName,1); 
    }

	return TRUE;
}

// chat
//   *
//   Fails On: nothing
// Just a handler, does not currently do anything
BOOL ProcessChat(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	return TRUE;
}



BOOL ProcessInitialPowerSet(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char setName[MAX_POWER_NAME_LENGTH];
	int id;
	PlayerStats* pPlayer;


/*	if(strcmp(entityName, "__UNKNOWN__"))
	{
		// earlier logs have the wrong name for these messages (prio
		return TRUE;
	}
*/
	if(!ExtractTokens(desc, "%s", setName))
		return FALSE;

	
	if(!stashFindInt(g_PowerSets, setName,&id))
	{
		// add new entry (offset by 1, as index '0' indicates no power set
		id = 1 + stashGetValidElementCount(g_PowerSets);
		stashAddInt(g_PowerSets, setName, id, true);		
	}

	pPlayer = GetPlayer(entityName);
	if( ! pPlayer->powerSets[0])
	{
		// first power set
		pPlayer->powerSets[0] = id;
	}
	else if(! pPlayer->powerSets[1])
	{
		pPlayer->powerSets[1] = id;
	}
	else
	{
		assert(!"Player has more than 2 initial power sets!! Was he created twice?");
	}

	eaPush(&(pPlayer->powerSetNames),StructAllocString(setName));

	return TRUE;
}

BOOL ProcessInitialPower(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char powerType[MAX_POWER_NAME_LENGTH];
	char powerName[MAX_POWER_NAME_LENGTH];
	char * powerfinalname;
	PlayerStats* pPlayer;

	if(!ExtractTokens(desc, "%s %n", powerType, powerName))
		return FALSE;

	pPlayer = GetPlayerOrCreate(entityName,time,mapName);
	if (powerfinalname=strrchr(powerName,'.')) {
		eaPush(&(pPlayer->powerNames),StructAllocString(&powerfinalname[1]));
		eaiPush(&(pPlayer->powerSlots),0);
	}

	return TRUE;
}

BOOL ProcessBuyPowerSet(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char setName[MAX_POWER_NAME_LENGTH];
	int id;
	PlayerStats* pPlayer;


	if(!ExtractTokens(desc, "%s", setName))
		return FALSE;

	if(!stashFindInt(g_PowerSets, setName,&id))
	{
		// add new entry (offset by 1, as index '0' indicates no power set
		id = 1 + stashGetValidElementCount(g_PowerSets);
		stashAddInt(g_PowerSets, setName, id, true);		
	}

	pPlayer = GetPlayer(entityName);
	if (pPlayer) {
		eaPush(&(pPlayer->powerSetNames),StructAllocString(setName));
		return TRUE;
	}

	return FALSE;
}

BOOL ProcessBuyPower(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char powerType[MAX_POWER_NAME_LENGTH];
	char powerName[MAX_POWER_NAME_LENGTH];
	char * powerfinalname;
	PlayerStats* pPlayer;

	if(!ExtractTokens(desc, "%s %n", powerType, powerName))
		return FALSE;

	pPlayer = GetPlayerOrCreate(entityName,time,mapName);
	if (powerfinalname=strrchr(powerName,'.')) {
		eaPush(&(pPlayer->powerNames),StructAllocString(&powerfinalname[1]));
		eaiPush(&(pPlayer->powerSlots),0);
	}
	return TRUE;
}




//[Store:Purchase] Boost Boosts.Generic_Recharge.Generic_Recharge (Cost 93)
BOOL ProcessStorePurchase(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char name[200];
	char type[200];
	int cost;
	int i;
	int level;
	DailyStats * day = GetCurrentDay(time);
	PlayerStats * pStats = GetPlayer(entityName);
	if(!pStats)
		return FALSE;
	level = pStats->currentLevel;
	if(level < pStats->xpCheatLevel)
	{
	//		fprintf(g_ConnectionLog, "\nPlayer %s stats will be ignored, as they skipped to level %d", pPlayer->playerName, pPlayer->xpCheatLevel);
		return TRUE;;
	}
	assert(pStats->accessLevel == 0);	// should be ignoring admins in higher-level function

	if(!ExtractTokens(desc, "%n %n (Cost %d)", type, name, &cost))
		return FALSE;

	i = GetIndexFromString(&g_ItemIndexMap, name);

	StatisticRecord(&g_GlobalStats.itemsBought[i], cost);

	if(!stricmp(type, "Boost")) {
		day->influence.buyEnhancements += cost;
		g_GlobalStats.influenceSpentPerLevel[level][EXPENSE_TYPE_ENHANCEMENT] += cost;
		g_HourlyStats[level][pStats->classType].influenceExpense[EXPENSE_TYPE_ENHANCEMENT] += cost;
	}
	else if(!stricmp(type, "Inspiration")) {
		day->influence.buyInspirations += cost;
		g_GlobalStats.influenceSpentPerLevel[level][EXPENSE_TYPE_INSPIRATION] += cost;
		g_HourlyStats[level][pStats->classType].influenceExpense[EXPENSE_TYPE_INSPIRATION] += cost;
	}
	else
		return FALSE;


	return TRUE;
}

//[Store:Sell] Boost Boosts.Generic_Run.Generic_Run (Paid 5)
BOOL ProcessStoreSell(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	char name[200];
	char type[200];
	int cost;
	int i;
	int level;
	DailyStats * day = GetCurrentDay(time);
	PlayerStats * pStats = GetPlayer(entityName);
	if(!pStats)
		return FALSE;
	level = pStats->currentLevel;
	if(level < pStats->xpCheatLevel)
	{
	//		fprintf(g_ConnectionLog, "\nPlayer %s stats will be ignored, as they skipped to level %d", pPlayer->playerName, pPlayer->xpCheatLevel);
		return TRUE;;
	}
	assert(pStats->accessLevel == 0);	// should be ignoring admins in higher-level function

	if(!ExtractTokens(desc, "%n %n (Paid %d)", type, name, &cost))
		return FALSE;
	
	i = GetIndexFromString(&g_ItemIndexMap, name);

	StatisticRecord(&g_GlobalStats.itemsSold[i], cost);


	g_GlobalStats.influencePerLevel[level][INCOME_TYPE_STORE] += cost;
	g_HourlyStats[level][pStats->classType].influenceIncome[INCOME_TYPE_STORE] += cost;
	pStats->influencePerLevel[level] += cost;

	if(!stricmp(type, "Boost"))
		day->influence.sellEnhancements += cost;
	else if(!stricmp(type, "Inspiration"))
		day->influence.sellInspirations += cost;
	else
		return FALSE;


	return TRUE;
}

BOOL ProcessCostumeTailor(int time, char * mapName, char * entityName, int teamID, const char * desc) {
	PlayerStats * pStats;
	int cost;
	
	pStats = GetPlayer(entityName);
	if (pStats) {

		//[Costume:Tailor] Spent Influence: 856
		if(!ExtractTokens(desc, "Spent Influence: %d", &cost))
			return FALSE;

		g_HourlyStats[pStats->currentLevel][pStats->classType].influenceExpense[EXPENSE_TYPE_TAILOR] += cost;
		return TRUE;
	}

	return FALSE;
}


BOOL ProcessTeamCreate(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	// if team exists, clear it, else, allocate it

	Team * pTeam;
	char teamName[100];

	sprintf(teamName, "%d", teamID);

	if(stashFindPointer(g_AllTeams, teamName,&pTeam)) {
		if (!pTeam->createdOnDemand) {
			fprintf(g_TeamActivityLog, "ERR: Team already exists! (prolly unavoidable mapserver crash issues)\n");
			DestroyTeam(pTeam);
			stashRemovePointer(g_AllTeams, teamName, &pTeam);
		}
		else {
			pTeam->createdOnDemand = FALSE;
		}
	}
	else {
        pTeam = CreateTeam(teamID);
	}

	return TRUE;
}


BOOL ProcessTeamDelete(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	// if team exists, clear it, else, allocate it

	Team * pTeam;
	char teamName[100];

	sprintf(teamName, "%d", teamID);

	if(!stashFindPointer(g_AllTeams, teamName,&pTeam))
	{
		return FALSE;
	}
	else
	{
		DestroyTeam(pTeam);
		stashRemovePointer(g_AllTeams, teamName,&pTeam);
	}

	fprintf(g_TeamActivityLog, "%s\tDeleted team %s\n", sec2DateTime(time),teamName);

	return TRUE;
}



BOOL ProcessTeamAddPlayer(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayerOrCreate(entityName,time,mapName);
	Team * pTeam = GetTeam(teamID);
	
	if(!pPlayer)
		return FALSE;
	
	if (!pTeam) {
		// This can happen when a team member is added to a just-created team on a different map.
		pTeam = CreateTeam(teamID);
		pTeam->createdOnDemand = TRUE;
	}

	if(pPlayer->teamID)
	{
		RemovePlayerFromTeam(pPlayer,time);
	}

	AddPlayerToTeam(pPlayer, teamID,time);

	//fprintf(g_TeamActivityLog, "\n%s\tAdded %s to team %d (size %d)", sec2DateTime(time), entityName, teamID, eaSize(&pTeam->players));
	return TRUE;
}

BOOL ProcessTeamRemovePlayer(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	// remove player
	//Team * pTeam = GetTeam(teamID);
	PlayerStats * pPlayer = GetPlayer(entityName);


	if(!pPlayer)
		return FALSE;

	if(pPlayer->teamID == teamID)
	{
		//fprintf(g_TeamActivityLog, "\n%s\tRemoved %s from team %d (old size %d)", sec2DateTime(time), entityName, teamID, eaSize(&pTeam->players) );
		RemovePlayerFromTeam(pPlayer,time);
	}
	else {
		fprintf(g_TeamActivityLog, "ERR: %s\tPlayer %s removed from team %d from log team %d\n", sec2DateTime(time), entityName, pPlayer->teamID, teamID);
	}

	return TRUE;
}


BOOL ProcessTeamQuit(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	// remove player
	Team * pTeam = GetTeam(teamID);
	PlayerStats * pPlayer = GetPlayer(entityName);

	if(pPlayer && pTeam)
	{
		if(pPlayer->teamID == teamID)
		{
//			fprintf(g_TeamActivityLog, "%s\tPlayer %s quit team %d (old size %d)\n", sec2DateTime(time), entityName, teamID, eaSize(&pTeam->players) );
			RemovePlayerFromTeam(pPlayer,time);
		}
		else {
			fprintf(g_TeamActivityLog, "ERR: %s\tPlayer %s quitting team %d from log team %d\n", sec2DateTime(time), entityName, pPlayer->teamID, teamID);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}







BOOL RecordInfluenceReward(PlayerStats * pPlayer, int influence, int time)
{
	DailyStats * pDay = GetCurrentDay(time);
	E_LastRewardAction action;

	action = pPlayer->lastRewardAction;

	if(pPlayer->teamID)
	{
		Team * team;
		if(stashFindPointer(g_AllTeams, int2str(pPlayer->teamID),&team))
			action = team->lastAction;
	}


	switch(action)
	{
	case LAST_REWARD_ACTION_KILL:
		pDay->influence.earnedByKill += influence;
		break;

	case LAST_REWARD_ACTION_TASK_COMPLETE:
		pDay->influence.earnedbyCompleteTask += influence;
		break;

	case LAST_REWARD_ACTION_MISSION_COMPLETE:
		pDay->influence.earnedByCompleteMission += influence;
		break;

	default:
		// unknown action !!!

		pDay->influence.unknown += influence;
	}

	return TRUE;
}


void SetLastRewardAction(PlayerStats * pPlayer, E_LastRewardAction action)
{
	pPlayer->lastRewardAction = action;
	if(pPlayer->teamID != 0)
	{
		Team * team = GetTeam(pPlayer->teamID);
		if(team)
			team->lastAction = action;
	}

	//fprintf(g_rewardLog, "\nAction   %s %d %s", pPlayer->playerName, pPlayer->teamID, GetLastRewardActionString(action));
}


const char * GetLastRewardActionString(E_LastRewardAction action)
{
	switch(action)
	{
	case LAST_REWARD_ACTION_KILL:
		return "Kill";

	case LAST_REWARD_ACTION_TASK_COMPLETE:
		return "Task:Success";

	case LAST_REWARD_ACTION_MISSION_COMPLETE:
		return "Mission:Success";

	default:
		return "Unknown";
	}
}



BOOL ProcessContactAdd(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayer(entityName);
	char contact[1000];
	int contactID;

	return TRUE;

	if(!pPlayer)
		return FALSE;

	//040313 21:21:32 City_00_01       "Jade Warrior"     0 [Contact:Add] CONTACTS/TUTORIAL/DETECTIVE_WRIGHT.CONTACT (via CONTACTS/TUTORIAL/SERGEANT_HICKS.CONTACT)
	if(!ExtractTokens(desc, "%n", contact))	// ignore the "via" part for now...
		return FALSE;

	contactID = GetIndexFromString(&g_ContactIndexMap, contact);

	pPlayer->contacts[contactID] = TRUE;

	return TRUE;
}




void EmptyTaskForce(int taskForceID)
{
	StashElement element;
	StashTableIterator it;

	// count # of players per level
	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it,&element))
	{
		PlayerStats * pPlayer = stashElementGetPointer(element);
		if(pPlayer->taskForceID == taskForceID)
			pPlayer->taskForceID = NO_TASKFORCE;
	}
}


int TaskForceSize(int taskForceID)
{
	StashElement element;
	StashTableIterator it;
	int count = 0;

	// count # of players per level
	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it,&element))
	{
		PlayerStats * pPlayer = stashElementGetPointer(element);
		if(pPlayer->taskForceID == taskForceID)
			count++;
	}
	return count;
}

BOOL ProcessTaskForceCreate(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	assert(TaskForceSize(teamID)==0);

	EmptyTaskForce(teamID);

	return TRUE;
}

BOOL ProcessTaskForceDelete(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	EmptyTaskForce(teamID);

	return TRUE;
}

BOOL ProcessTaskForceAddPlayer(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayer(entityName);
	if(!pPlayer)
		return FALSE;

	pPlayer->taskForceID = teamID;

	return TRUE;
}

BOOL ProcessTaskForceRemovePlayer(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayer(entityName);
	if(!pPlayer)
		return FALSE;

	if(pPlayer->taskForceID != teamID)
		printf("Error: Task force ID does not match stored ID\n");

	pPlayer->taskForceID = NO_TASKFORCE;

	return TRUE;
}


// i don't think this one is used...
BOOL ProcessTaskForceQuit(int time, char * mapName, char * entityName, int teamID, const char * desc)
{
	PlayerStats * pPlayer = GetPlayer(entityName);
	if(!pPlayer)
		return FALSE;
	if(pPlayer->taskForceID != teamID)
		printf("Error: Task force ID does not match stored ID\n");

	pPlayer->taskForceID = NO_TASKFORCE;

	return TRUE;
}

void InitEntryMap()
{
	g_String2ActionMap = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);


    g_Action2StringMap = stashTableCreateInt(1000);

#define REGISTER_ACTION_FUNC(STEM, ACTION) RegisterActionEx(#STEM ## #ACTION,0,ACTION, #ACTION)

	// Mapserver stuff
	RegisterAction("MapServer","Connect",				ACTION_MAPSERVER_CONNECT,					ProcessMapserverConnect);
	RegisterAction("serverquit",					ACTION_MAPSERVER_QUIT,						ProcessMapserverQuit);

	// Chatserver stuff
	RegisterAction("ChatServer","Connected",			ACTION_CHATSERVER_CONNECT,					ProcessChatserver);
	RegisterAction("ChatServer","Disconnected",		ACTION_CHATSERVER_DISCONNECT,				ProcessChatserver);

	// Player Connections
	RegisterAction("Connection","Connect",			ACTION_CONNECTION_CONNECT,					ProcessConnectionEntry);
	RegisterAction("Connection","ResumeCharacter",	ACTION_CONNECTION_RESUME_CHARACTER,			ProcessConnectionEntry);
	RegisterAction("Connection","Disconnect",			ACTION_CONNECTION_DISCONNECT,				ProcessConnectionEntry);
	RegisterAction("Connection","Disconnect:Dropped",	ACTION_CONNECTION_DISCONNECT_DROPPED,		ProcessConnectionEntry);
	RegisterAction("Connection","Logout",				ACTION_CONNECTION_LOGOUT,					ProcessConnectionEntry);

	// Character Resume
	RegisterAction("Entity","Resume:Character",		ACTION_ENTITY_RESUME_CHARACTER,				ProcessEntityResumeCharacter);
	RegisterAction("Entity","MapMove",				ACTION_ENTITY_MAPMOVE,						ProcessEntityMapMove);

	// Entity Creation
	RegisterAction("Entity","Create:Villain",			ACTION_ENTITY_CREATE_POWER,					ProcessEntityCreateVillain);
	RegisterAction("Entity","Create:Pet",				ACTION_ENTITY_CREATE_PET,					ProcessEntityCreatePet);

	// Entity Death
	RegisterAction("Entity","RecordKill:Player",		ACTION_ENTITY_RECORD_KILL_PLAYER,			ProcessEntityRecordKillPlayer);
	RegisterAction("Entity","RecordKill",				ACTION_ENTITY_RECORD_KILL,					ProcessEntityRecordKill);
	RegisterAction("Entity","Die:Pet",				ACTION_ENTITY_DIE_PET,						ProcessEntityDiePet);
	RegisterAction("Entity","Die",					ACTION_ENTITY_DIE,							ProcessEntityDie);

	// Entity Effects
	RegisterAction("Entity","Fly",					ACTION_ENTITY_FLY,							ProcessEntityEffect);
	RegisterAction("Entity","Immobilized",			ACTION_ENTITY_IMMOBILIZED,					ProcessEntityEffect);
	RegisterAction("Entity","Knocked",				ACTION_ENTITY_KNOCKED,						ProcessEntityEffect);
	RegisterAction("Entity","Stunned",				ACTION_ENTITY_STUNNED,						ProcessEntityEffect);
	RegisterAction("Entity","Resurrect",				ACTION_ENTITY_RESURRECT,					ProcessEntityEffect);
	RegisterAction("Entity","Sleep",					ACTION_ENTITY_SLEEP,						ProcessEntityEffect);
	RegisterAction("Entity","Intangible",				ACTION_ENTITY_INTANGIBLE,					ProcessEntityEffect);
	RegisterAction("Entity","PeriodicInfo",0, ProcessEntityPeriodicInfo);



	// Powers
	RegisterAction("Power","ToggleOn",				ACTION_POWER_TOGGLE_ON,						ProcessPowerToggle);	
	RegisterAction("Power","ToggleOff",				ACTION_POWER_TOGGLE_OFF,					ProcessPowerToggle);
	RegisterAction("Power","UseInspiration",			ACTION_POWER_USE_INSPIRATION,				ProcessPowerUseInspiration);
	RegisterAction("Power","Queue",					ACTION_POWER_QUEUE,							ProcessPowerQueue);
	RegisterAction("Power","Activate",				ACTION_POWER_ACTIVATE,						ProcessPowerActivate);
	RegisterAction("Power","Cancelled",				ACTION_POWER_CANCELLED,						ProcessPowerCancelled);
	RegisterAction("Power","Hit",						ACTION_POWER_HIT,							ProcessPowerHit);
	RegisterAction("Power","Miss",					ACTION_POWER_MISS,							ProcessPowerMiss);
	RegisterAction("Power","Effect",					ACTION_POWER_EFFECT,						ProcessPowerEffect);
	RegisterAction("Power","WindUp",					ACTION_POWER_WINDUP,						ProcessPowerWindup);
	RegisterAction("Power","Interrupted",				ACTION_POWER_INTERRUPTED,					ProcessPowerInterrupted);

	// Enhancements
	RegisterAction("Character","SetBoost",			ACTION_CHARACTER_SET_BOOST,					ProcessEnhancements);
	RegisterAction("Character","DestroyBoost",		ACTION_CHARACTER_DESTROY_BOOST,				ProcessEnhancements);
	RegisterAction("Character","CombineEnhancement",	ACTION_CHARACTER_COMBINE_ENHANCEMENT,		ProcessEnhancements);
	RegisterAction("Power","ReplaceBoost",			ACTION_POWER_REPLACE_BOOST,					ProcessEnhancements);	
	RegisterAction("Power","DestroyBoost",			ACTION_POWER_DESTROY_BOOST,					ProcessEnhancements);	


	// Rewards (and debt)
	RegisterAction("Reward","LevelUp",				ACTION_REWARD_LEVELUP,						ProcessRewardLevelUp);
	RegisterAction("Reward","Points",					ACTION_REWARD_POINTS,						ProcessRewardPoints);
	RegisterAction("Character","AddDebt",				ACTION_CHARACTER_ADDDEBT,					ProcessCharacterAddDebt);
//	RegisterAction("Reward","Inspiration",			ACTION_REWARD_INSPIRATION,					ProcessRewardInspiration);
//	RegisterAction("Reward","Enhancement",			ACTION_REWARD_ENHANCEMENT,					ProcessRewardEnhancement);
	RegisterAction("Reward","AddInspiration",			ACTION_REWARD_ADD_INSPIRATION,				ProcessRewardAddInspiration);
	RegisterAction("Reward","AddSpecialization",		ACTION_REWARD_ADD_SPECIALIZATION,			ProcessRewardAddSpecialization);
	RegisterAction("Reward","AddPower",				ACTION_REWARD_ADD_POWER,					0);
	RegisterAction("Reward","AddTempPower",			ACTION_REWARD_ADD_TEMP_POWER,				ProcessRewardTempPower);
	RegisterAction("Reward","RemoveTempPower",		ACTION_REWARD_REMOVE_TEMP_POWER,			ProcessRewardTempPower);
	RegisterAction("Reward","AddAccoladePower",		ACTION_REWARD_ADD_ACCOLADE_POWER,			0);
	RegisterAction("Reward","ChoiceReceived",			ACTION_REWARD_CHOICE_RECEIVED,				0);
	RegisterAction("Reward","ChoiceForced",			ACTION_REWARD_CHOICE_FORCED,				0);
	RegisterAction("Reward","StoryReward",			ACTION_REWARD_STORY_REWARD,					ProcessRewardStoryReward);
	RegisterAction("Reward","MissionReward",			ACTION_REWARD_MISSION_REWARD,				ProcessRewardMissionReward);
	RegisterAction("Reward","MissionSouvenir",		ACTION_REWARD_MISSION_SOUVENIR,				0);
	RegisterAction("Reward","SouvenirClue",			ACTION_REWARD_SOUVENIR_CLUE,				0);
	RegisterAction("Reward","BadgeEarned",			ACTION_REWARD_BADGE_EARNED,					0);
	RegisterAction("Reward","BadgeRevoke",			ACTION_REWARD_BADGE_REVOKE,					0);

	// Trading
	RegisterAction("Trade","Receive:Influence",		ACTION_TRADE_RECEIVE_INFLUENCE,				ProcessTrade);
	RegisterAction("Trade","Receive:Inspiration",		ACTION_TRADE_RECEIVE_INSPIRATION,			ProcessTrade);
	RegisterAction("Trade","Receive:Enhancement",		ACTION_TRADE_RECEIVE_ENHANCEMENT,			ProcessTrade);

		
	// Missions
	RegisterAction("Mission","Attach", 				ACTION_MISSION_ATTACH,						ProcessMissionAttach);
	RegisterAction("Mission","Success", 				ACTION_MISSION_SUCCESS,						ProcessMissionComplete);
	RegisterAction("Mission","Failure", 				ACTION_MISSION_FAILURE,						ProcessMissionComplete);

	RegisterAction("Task","Success",					ACTION_TASK_SUCCESS,						ProcessTaskComplete);

	// Encounters
	RegisterAction("Encounter","Spawn",				ACTION_ENCOUNTER_SPAWN,						ProcessEncounter);
	RegisterAction("Encounter","Created",				ACTION_ENCOUNTER_CREATED,					ProcessEncounter);
	RegisterAction("Encounter","Active",				ACTION_ENCOUNTER_ACTIVE,					ProcessEncounter);
	RegisterAction("Encounter","Inactive",			ACTION_ENCOUNTER_INACTIVE,					ProcessEncounter);
	RegisterAction("Encounter","Completed",			ACTION_ENCOUNTER_COMPLETED,					ProcessEncounter);
	RegisterAction("Encounter","Despawn",				ACTION_ENCOUNTER_DESPAWN,					ProcessEncounter);
	RegisterAction("Encounter","Destroyed",			ACTION_ENCOUNTER_DESTROYED,					ProcessEncounter);

	// Character Creation and Development
	RegisterAction("Character","Create",				ACTION_CHARACTER_CREATE,					ProcessCharacterCreate);
	RegisterAction("Character","InitialPowerSet",		ACTION_CHARACTER_INITIAL_POWER_SET,			ProcessInitialPowerSet);
	RegisterAction("Character","InitialPower",		ACTION_CHARACTER_INITIAL_POWER,				ProcessInitialPower);
	RegisterAction("Character","BuyPowerSet",			ACTION_CHARACTER_BUY_POWER_SET,				ProcessBuyPowerSet);
	RegisterAction("Character","BuyPower",			ACTION_CHARACTER_BUY_POWER,					ProcessBuyPower);
	RegisterAction("Character","AssignSlots",			ACTION_CHARACTER_ASSIGN_SLOTS,				ProcessAssignSlots);

	// Team
	RegisterAction("Team","Create",					ACTION_TEAM_CREATE,							ProcessTeamCreate);
	RegisterAction("Team","Delete",					ACTION_TEAM_DELETE,							ProcessTeamDelete);
	RegisterAction("Team","AddPlayer",				ACTION_TEAM_ADD_PLAYER,						ProcessTeamAddPlayer);
	RegisterAction("Team","RemovePlayer",				ACTION_TEAM_REMOVE_PLAYER,					ProcessTeamRemovePlayer);
	RegisterAction("Team","Quit",						ACTION_TEAM_QUIT,							ProcessTeamQuit);
	RegisterAction("Team","Leader",					ACTION_TEAM_LEADER,							ProcessTeamLeader),
	RegisterAction("Team","Kick",						ACTION_TEAM_KICK,							ProcessTeamKick),

	// Task Force
	RegisterAction("TaskForce","Create",				ACTION_TASKFORCE_CREATE,					ProcessTaskForceCreate);
	RegisterAction("TaskForce","Quit",				ACTION_TASKFORCE_QUIT,						ProcessTaskForceQuit);
	RegisterAction("TaskForce","Leader",				ACTION_TASKFORCE_LEADER,					0),
	RegisterAction("TaskForce","Delete",				ACTION_TASKFORCE_DELETE,					ProcessTaskForceDelete);
	RegisterAction("TaskForce","AddPlayer",			ACTION_TASKFORCE_ADD_PLAYER,				ProcessTaskForceAddPlayer);
	RegisterAction("TaskForce","RemovePlayer",		ACTION_TASKFORCE_REMOVE_PLAYER,				ProcessTaskForceRemovePlayer);




	RegisterAction("Costume","Tailor",				ACTION_COSTUME_TAILOR,						ProcessCostumeTailor);


	RegisterAction("cmdparse",						ACTION_CMDPARSE,							ProcessCmdParse);
	RegisterAction("chat",							ACTION_CHAT,								ProcessChat);
	


	RegisterAction("Contact","Add",					ACTION_CONTACT_ADD,							ProcessContactAdd);
	

	RegisterAction("Character","BuyItem",				ACTION_CHARACTER_BUY_ITEM,					0);
	RegisterAction("Store","Purchase",				ACTION_STORE_PURCHASE,						ProcessStorePurchase);
	RegisterAction("Store","Sell",					ACTION_STORE_SELL,							ProcessStoreSell);





	RegisterAction("MapXfer",						ACTION_MAP_XFER,							0);

	RegisterAction("Security","Accesslevel",			ACTION_SECURITY_ACCESSLEVEL,				0);



	RegisterAction("SuperGroup","Mode",				ACTION_SUPERGROUP_MODE,						0);
	RegisterAction("SuperGroup","Rank",				ACTION_SUPERGROUP_RANK,						0);
	RegisterAction("SuperGroup","MOTD",				ACTION_SUPERGROUP_MOTD,						0);
	RegisterAction("SuperGroup","Create",				ACTION_SUPERGROUP_CREATE,					0);
	RegisterAction("SuperGroup","Delete",				ACTION_SUPERGROUP_DELETE,					0);
	RegisterAction("SuperGroup","AddPlayer",			ACTION_SUPERGROUP_ADD_PLAYER,				0);
	RegisterAction("SuperGroup","RemovePlayer",		ACTION_SUPERGROUP_REMOVE_PLAYER,			0);
	RegisterAction("SuperGroup","Leader",				ACTION_SUPERGROUP_LEADER,					0);
	RegisterAction("SuperGroup","RankName",			ACTION_SUPERGROUP_RANKNAME,					0);
	RegisterAction("SuperGroup","Costume",			ACTION_SUPERGROUP_COSTUME,					0);
	RegisterAction("SuperGroup","Quit",				ACTION_SUPERGROUP_QUIT,						0);
	RegisterAction("SuperGroup","Motto",				ACTION_SUPERGROUP_MOTTO,					0);
	RegisterAction("SuperGroup","Kick",				ACTION_SUPERGROUP_KICK,						0);	
}

typedef struct MapStatTick
{
    PlayerStats **players_this_tick;
    StashTable players_so_far; // warning! shared amoung all instances of this object
    int time;
} MapStatTick;

MP_DEFINE(MapStatTick);
static MapStatTick* MapStatTick_Create( MapStatTick *prev )
{
	MapStatTick *res = NULL;

	// create the pool, arbitrary number
	MP_CREATE(MapStatTick, 64); 
	res = MP_ALLOC( MapStatTick );
	if( res )
	{
        if(prev)
            res->players_so_far = prev->players_so_far;
        else
            res->players_so_far = stashTableCreateWithStringKeys(128, StashDefault);
	}
	return res;
}

static void MapStatTick_Destroy( MapStatTick *hItem )
{
	if(hItem)
	{
		MP_FREE(MapStatTick, hItem);
	}
}


static void MapStatTick_AddPlayer(PlayerStats *p)
{
    if(!p)
        return;
    
}


typedef struct MapStats
{
    char map_name[64];
    MapStatTick **ticks;
} MapStats;
StashTable maps_stats; // {map name, MapStats*}

MP_DEFINE(MapStats);
static MapStats* MapStats_Create( char *mapName )
{
	MapStats *res = NULL;

	// create the pool, arbitrary number
	MP_CREATE(MapStats, 64);
	res = MP_ALLOC( MapStats );
	if( res )
	{
		strcpy_unsafe(res->map_name,mapName);
	}
	return res;
}

static void MapStats_Destroy( MapStats *hItem )
{
	if(hItem)
	{
		MP_FREE(MapStats, hItem);
	}
}


MapStatTick* MapStats_FindOrAddTick(MapStats *ms, int time)
{
    int i;
    MapStatTick *r;

    // just assume the last one is the correct time.
    i = eaSize(&ms->ticks) - 1;
    r = eaGet(&ms->ticks,i);
    if(r && r->time == time)
        return r;

    r = MapStatTick_Create(r);
    eaPush(&ms->ticks,r);
    return r;
//     for( i = eaSize(&ms->ticks) - 1; i >= 0; --i)
//     {
//         r = ms->ticks[i];
//         if(time >= t->time)
//             break;
//     }
//     // ab: quantize here. i == one less than goal if no match.
//     if(r->time == time)
//         return r;
//     r = eaGet(&ms->ticks,i);
//     r = MapStatTick_Create(r);
//     if(eaSize(&ms->ticks)<i)
//         eaPush(&ms->ticks,r);
//     else
//         eaInsert(&ms->ticks,i,r);
}



//----------------------------------------
//  Workhorse function for dispatching all actions
//----------------------------------------
int ProcessEntry(const char * entry)
{
    MapStats *map_stats;
    MapStatTick *map_tick;
    char mapIp[MAX_ENTRY_SIZE];
	char mapName[MAX_ENTRY_SIZE];
	char entityName[MAX_ENTRY_SIZE];
	int teamID;
	int time;
	int hour;
	static int lastcleanedtime = 0;
	const char * escaped_rest;
	const char * rest;
	EntryProcessingFunc func;
	PlayerStats * pPlayer;

	// Catch empty lines
	if(!entry[1] || !entry[2])
		return TRUE;

	unescapeString_unsafe(entry);

	escaped_rest = ParseEntry(entry, &time, mapName, mapIp, entityName, &teamID, &func);

	if(!escaped_rest)
	{
		fprintf(g_LogParserErrorLog,"Bad basic format:\n");
		return FALSE;
	}

	rest = unescapeString_unsafe(escaped_rest);

	time = CorrectTimeZone(time, false);

	if(g_GlobalStats.lastTime > time)	// we have an out-of-order entry
		fprintf(g_LogParserErrorLog,"Out of order (Cur %d Last %d):\n",time,g_GlobalStats.lastTime);

	g_GlobalStats.lastTime = time;

	if( 0 == g_GlobalStats.totalEntries++)
		g_GlobalStats.startTime = time;
	if( 0 == g_GlobalStats.logStartTime)
		g_GlobalStats.logStartTime = time;

	pPlayer = GetPlayer(entityName);

	// Skip nonzero accesslevel players
	if(pPlayer && pPlayer->accessLevel > 0)
		return TRUE;

    // ----------
    // get map tick info

    if(!maps_stats)
        maps_stats = stashTableCreateWithStringKeys(128, StashDefault);

    if(!stashFindPointer(maps_stats,mapName,&map_stats))
    {
        map_stats = MapStats_Create(mapName);
        stashAddPointer(maps_stats,map_stats->map_name,map_stats,true);
    }
    map_tick = MapStats_FindOrAddTick(map_stats,time);
    
    // ----------
	// check if time between a player's messages has been TOO long...

	if(pPlayer)
	{
		int delta = (time - pPlayer->lastEntryTime); 
		if( delta > MAX_ENTRY_INTERVAL_TIME)
		{
			// Player was inactive for a long period of time;
			// Disconnect the player and skip over the interval.

			PlayerUpdateConnectionTime(pPlayer, pPlayer->lastEntryTime);
			PlayerUpdateLevelTime(pPlayer, pPlayer->lastEntryTime);

			pPlayer->lastEntryTime		= time;	// this time is the NEW last entry
			pPlayer->lastConnectTime		= time;
			pPlayer->lastLevelUpdateTime = time;
		}
		else {
			// The delta is reasonable;  Note this "played" time in the hourly stats
			g_HourlyStats[pPlayer->currentLevel][pPlayer->classType].playTime += (float)delta;
		}

        // new player, update stash
        if(!eaFind(&map_tick->players_this_tick,pPlayer))
        {
            eaPush(&map_tick->players_this_tick,pPlayer);
            stashAddPointer(map_tick->players_so_far,pPlayer->playerName,pPlayer,false);
        }
	}

	// record # of times each action is logged
	stashAddOrModifyInt(g_GlobalStats.actionCounts,GetActionName(func),1);
    
    // I've overloaded action type to hold a function
    // pointer because I'm lazy
    if(INRANGE0(action,ACTION_TYPE_COUNT))
        func = g_Action2Func[action];
    else
        func = (EntryProcessingFunc)action;

	if(strstri("Reward",entry))
	{
		printf("");
	}

	if(func)
	{
		if( !func(time, mapName, entityName, teamID, action, rest) )
			return FALSE;
	}
	else
		fprintf(g_LogParserUnhandledLog,"%s\n",entry);

	// Refetch the player in case they were changed or created by calling the function
	pPlayer = GetPlayer(entityName);

	// Clean the damage stats every 60s
	if (time > (lastcleanedtime + 60)) {
        cleanDamageStats(lastcleanedtime);
		lastcleanedtime = time;
	}

	// update periodic stats
	hour = GetHour(time);

	if (!g_nextPeriodUpdateHour)
		g_nextPeriodUpdateHour = hour+1;

	if(hour >= g_nextPeriodUpdateHour)
	{
		StashElement element;
		StashTableIterator it;
		int archcount;
		int levelcount;
		char filename[200];
		FILE* file;
		int damagetypeCount = stashGetValidElementCount(g_DamageTypeIndexMap.table);
		int damageindex;


		int period = ComputePeriod(hour);

		g_nextPeriodUpdateHour = hour + 1;

		stashGetIterator(g_AllZones, &it);
		while(stashGetNextElement(&it,&element))
		{
			ZoneStats * pStats = stashElementGetPointer(element);
			const char * mapName = stashElementGetKey(element);
			ZoneStatsUpdatePeriodStats(mapName, pStats, time, period);
		}

		// Make sure our target directory exists
		sprintf(filename,"%s\\Hourly Stats",g_WebDir);
		mkdirtree(filename);
		mkdir(filename);

		sprintf(filename, "%s\\Hourly Stats\\HourlyStats%d.txt",g_WebDir,hour);
		file = fopen(filename, "w+t");
		assert(file);

//		printf("\n\n**********  Hourly Stats  ***************\n");
		for(archcount=0;archcount<CLASS_TYPE_COUNT;archcount++) {
//			printf("\n*** Arch %d\n",archcount);
			fprintf(file,"Arch %d\n",archcount);
			for(levelcount=0;levelcount<MAX_PLAYER_SECURITY_LEVEL;levelcount++) {
				// Skip if if no one actually played!
				if (!((g_HourlyStats[levelcount][archcount].playTime/3600.0) > 0.0))
					continue;

				fprintf(file,"\n  Level %d\n",levelcount);
				fprintf(file,"    PTm  %f\n",g_HourlyStats[levelcount][archcount].playTime/3600.0);
				fprintf(file,"    NXP  %d %d %d\n",
					g_HourlyStats[levelcount][archcount].xpNormal[XPGAIN_TYPE_UNKNOWN],
					g_HourlyStats[levelcount][archcount].xpNormal[XPGAIN_TYPE_KILL],
					g_HourlyStats[levelcount][archcount].xpNormal[XPGAIN_TYPE_MISSION]);
				fprintf(file,"    DXP  %d %d %d\n",
					g_HourlyStats[levelcount][archcount].xpDebt[XPGAIN_TYPE_UNKNOWN],
					g_HourlyStats[levelcount][archcount].xpDebt[XPGAIN_TYPE_KILL],
					g_HourlyStats[levelcount][archcount].xpDebt[XPGAIN_TYPE_MISSION]);
				fprintf(file,"    IINF %d %d %d %d\n",
					g_HourlyStats[levelcount][archcount].influenceIncome[INCOME_TYPE_UNKNOWN],
					g_HourlyStats[levelcount][archcount].influenceIncome[INCOME_TYPE_KILL],
					g_HourlyStats[levelcount][archcount].influenceIncome[INCOME_TYPE_MISSION],
					g_HourlyStats[levelcount][archcount].influenceIncome[INCOME_TYPE_STORE]);
				fprintf(file,"    OINF %d %d %d %d\n",
					g_HourlyStats[levelcount][archcount].influenceExpense[EXPENSE_TYPE_UNKNOWN],
					g_HourlyStats[levelcount][archcount].influenceExpense[EXPENSE_TYPE_ENHANCEMENT],
					g_HourlyStats[levelcount][archcount].influenceExpense[EXPENSE_TYPE_INSPIRATION],
					g_HourlyStats[levelcount][archcount].influenceExpense[EXPENSE_TYPE_TAILOR]);
				for(damageindex=0;damageindex<damagetypeCount;damageindex++) {
					if (g_HourlyStats[levelcount][archcount].damage[damageindex] > 0)
						fprintf(file,"    %s %d\n",GetStringFromIndex(&g_DamageTypeIndexMap, damageindex),g_HourlyStats[levelcount][archcount].damage[damageindex]);
				}
			}
			fprintf(file,"\n\n");
		}
		fclose(file);
		// Reset Hourly Stats
		InitHourlyStats();

	}

	// finally, update last entry time & verify team info is correct
	if(pPlayer)
	{
		pPlayer->lastEntryTime = time;

		// check team info
		if(	   ! IsSuperGroupAction(action)	// supergroup entries use different value for Team ID (nasty!!!)
			&& ! IsTaskForceAction(action)
			&&   action != ACTION_CONNECTION_DISCONNECT
			&&   action != ACTION_TEAM_REMOVE_PLAYER
			&&   action != ACTION_TEAM_KICK)	
		{
			if(teamID != pPlayer->teamID)
			{
				if(pPlayer->teamID)
					RemovePlayerFromTeam(pPlayer,time);
				if(teamID)
					AddPlayerToTeam(pPlayer, teamID,time);
			}
		}
	}

	return 1;
}
