#ifndef _COMMON_H
#define _COMMON_H


#include "earray.h"
#include "StashTable.h"
#include "Index2StringMap.h"
#include "file.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>



#define MAX_PLAYER_SECURITY_LEVEL		60
#define MAX_POWER_NAME_LENGTH   200
#define MAX_NAME_LENGTH			100
#define MAX_PLAYER_POWER_SETS	8		// i think this is more than enough
#define MAX_PLAYER_POWERS		50
#define MAX_VILLAIN_TYPES		2000
#define MAX_TEAM_SIZE			8
#define MAX_CONTACTS			400
//#define MAX_POWERSET_CHOICES	12 // Number of power sets available per prim/sec

#define MAX_POWER_TYPES			4000
#define MAX_ITEM_TYPES			4000
#define MAX_DAMAGE_TYPES		16

typedef int BOOL;
#define TRUE	1
#define FALSE	0


extern FILE * g_ConnectionLog;
extern FILE * g_LogParserErrorLog;
extern FILE * g_LogParserUnhandledLog;
extern char * g_WebDir;

// Global variables used for quick log scanning/dumping
extern BOOL g_bQuickScan, g_bContinueQuickScan;
extern FILE * g_QuickScanLog;
extern char g_QuickScanPattern[2000];
extern char g_QuickScanFileName[200];
extern int g_QuickScanStartTime, g_QuickScanFinishTime;

// Global variables used for reward file scanning
extern BOOL g_bRewardScan;
extern FILE * g_RewardScanLog;



typedef enum E_Map_Type
{
	MAP_TYPE_CITY_ZONE = 0,
	MAP_TYPE_TRIAL_ZONE,
	MAP_TYPE_HAZARD_ZONE,
	MAP_TYPE_PVP_ZONE,
	MAP_TYPE_MISSION,
	MAP_TYPE_BASE,

	MAP_TYPE_COUNT,

} E_Map_Type;

E_Map_Type   GetMapTypeIndex(char * mapName);
const char * GetMapTypeName(E_Map_Type type);


typedef enum E_Origin_Type
{

	ORIGIN_TYPE_MAGIC = 0,
	ORIGIN_TYPE_MUTATION,
	ORIGIN_TYPE_NATURAL,
	ORIGIN_TYPE_SCIENCE,
	ORIGIN_TYPE_TECHNOLOGY,

	ORIGIN_TYPE_COUNT,

} E_Origin_Type;

E_Origin_Type GetOriginTypeIndex(char * originName);
const char *  GetOriginTypeName(int type);


typedef enum E_Class_Type
{
	CLASS_TYPE_BLASTER = 0,             // 0
	CLASS_TYPE_CONTROLLER,              // 1
	CLASS_TYPE_DEFENDER,                // 2
	CLASS_TYPE_SCRAPPER,                // 3
	CLASS_TYPE_TANKER,                  // 4
	CLASS_TYPE_PEACEBRINGER,            // 5
	CLASS_TYPE_WARSHADE,                // 6
	CLASS_TYPE_BRUTE,                   // 7
	CLASS_TYPE_CORRUPTOR,               // 8
	CLASS_TYPE_DOMINATOR,               // 9
	CLASS_TYPE_MASTERMIND,              // 10
	CLASS_TYPE_STALKER,                 // 11
    CLASS_TYPE_ARACHNOSSOLDIER,         // 12
    CLASS_TYPE_ARACHNOSWIDOW,           // 13

	CLASS_TYPE_UNKNOWN, // Should always be last

	CLASS_TYPE_COUNT,

} E_Class_Type;

E_Class_Type GetClassTypeIndex(char * className);
const char * GetClassTypeName(int type);


// used to determine the last action completed by a team, so that sources of rewards is known
typedef enum E_LastRewardAction {LAST_REWARD_ACTION_UNKNOWN = 0, // must be zero! (default value)
LAST_REWARD_ACTION_KILL, 
LAST_REWARD_ACTION_TASK_COMPLETE, 
LAST_REWARD_ACTION_MISSION_COMPLETE
}E_LastRewardAction;

// Typedef for tracking influnce earned
typedef enum E_Income_Type {
	INCOME_TYPE_UNKNOWN = 0,
	INCOME_TYPE_KILL,
	INCOME_TYPE_MISSION,
	INCOME_TYPE_STORE,

	INCOME_TYPE_COUNT,
} E_Income_Type;

// Typedef for tracking influence spent
typedef enum E_Expense_Type {
	EXPENSE_TYPE_UNKNOWN = 0,
	EXPENSE_TYPE_ENHANCEMENT,
	EXPENSE_TYPE_INSPIRATION,
	EXPENSE_TYPE_TAILOR,

	EXPENSE_TYPE_COUNT,
} E_Expense_Type;

// Typedef for tracking xp earned
typedef enum E_XPGain_Type {
	XPGAIN_TYPE_UNKNOWN = 0,
	XPGAIN_TYPE_KILL,
	XPGAIN_TYPE_MISSION,

	XPGAIN_TYPE_COUNT,
} E_XPGain_Type;

extern Index2StringMap g_VillainIndexMap;
extern Index2StringMap g_PowerIndexMap;
extern Index2StringMap g_ItemIndexMap;
extern Index2StringMap g_ContactIndexMap;
extern Index2StringMap g_DamageTypeIndexMap;

extern StashTable g_AllPlayers;
extern StashTable g_AllTeams;
extern StashTable g_AllZones;
extern StashTable g_UnknownEntries;
extern StashTable g_PowerSets;
extern StashTable g_DamageTracker;
extern StashTable g_PetTracker;
extern StashTable g_MissionCompletion;
typedef struct DailyStats DailyStats;
extern DailyStats **g_DailyStats;

extern FILE * g_ConnectionLog;
extern FILE * g_TeamActivityLog;
//extern FILE * g_rewardLog;

#endif  // _COMMON_H
