//
// data.c
//

#include "overall_stats.h"
#include "error.h"
#include "utils.h"
#include <string.h>
#include <cassert>

GlobalStats g_GlobalStats;
HourlyStats g_HourlyStats[MAX_PLAYER_SECURITY_LEVEL][CLASS_TYPE_COUNT];
StashTable g_PowerSets;


E_Map_Type GetMapTypeIndex(char * mapName)
{
	if(!strncmp(mapName, "City_", 5) || !strncmp(mapName, "V_City_", 7))
		return MAP_TYPE_CITY_ZONE;
	else if(!strncmp(mapName, "Trial_", 6))
		return MAP_TYPE_TRIAL_ZONE;
	else if(!strncmp(mapName, "Hazard_", 7))
		return MAP_TYPE_HAZARD_ZONE;
	else if(!strncmp(mapName, "V_PvP_", 6))
		return MAP_TYPE_PVP_ZONE;
	else if(!strncmp(mapName, "super", 5))
		return MAP_TYPE_BASE;
	else
		return MAP_TYPE_MISSION;
}


const char * GetMapTypeName(E_Map_Type type)
{
	switch(type)
	{
	case MAP_TYPE_CITY_ZONE:
		return "City Zone";
	case MAP_TYPE_TRIAL_ZONE:
		return "Trial Zone";
	case MAP_TYPE_HAZARD_ZONE:
		return "Hazard Zone";
	case MAP_TYPE_PVP_ZONE:
		return "PvP Map";
	case MAP_TYPE_MISSION:
		return "Mission Map";
	case MAP_TYPE_BASE:
		return "Base";
	default:
		assert(!"No name for this type!");
		return "NO TYPE NAME SPECIFIED!";
	}
}


E_Origin_Type GetOriginTypeIndex(char * originName)
{
	if( ! stricmp(originName, "Science"))
		return ORIGIN_TYPE_SCIENCE;
	else if( ! stricmp(originName, "Technology"))
		return ORIGIN_TYPE_TECHNOLOGY;
	else if( ! stricmp(originName, "Magic"))
		return ORIGIN_TYPE_MAGIC;
	else if( ! stricmp(originName, "Natural"))
		return ORIGIN_TYPE_NATURAL;
	else if( ! stricmp(originName, "Mutation"))
		return ORIGIN_TYPE_MUTATION;
	else
	{
		assert(!"Bad origin name");
		return ORIGIN_TYPE_COUNT;	// to avoid warnings
	}
}

const char * GetOriginTypeName(int type)
{
	switch(type)
	{
	case ORIGIN_TYPE_SCIENCE:
		return "Science";
	case ORIGIN_TYPE_TECHNOLOGY:
		return "Technology";
	case ORIGIN_TYPE_MAGIC:
		return "Magic";
	case ORIGIN_TYPE_NATURAL:
		return "Natural";
	case ORIGIN_TYPE_MUTATION:
		return "Mutation";
	default:
		assert(!"No name for this type!");
		return "NO TYPE NAME SPECIFIED!";
	}
}

struct ClassIdStrPairs
{
    int id;
    char *str;
} class_id_str_pairs[] = {
    {CLASS_TYPE_BLASTER,"BLASTER"},
    {CLASS_TYPE_CONTROLLER,"CONTROLLER"},
    {CLASS_TYPE_DEFENDER,"DEFENDER"},
    {CLASS_TYPE_SCRAPPER,"SCRAPPER"},
    {CLASS_TYPE_TANKER,"TANKER"},
    {CLASS_TYPE_PEACEBRINGER,"PEACEBRINGER"},
    {CLASS_TYPE_WARSHADE,"WARSHADE"},
    {CLASS_TYPE_BRUTE,"BRUTE"},
    {CLASS_TYPE_CORRUPTOR,"CORRUPTOR"},
    {CLASS_TYPE_DOMINATOR,"DOMINATOR"},
    {CLASS_TYPE_MASTERMIND,"MASTERMIND"},
    {CLASS_TYPE_STALKER,"STALKER"},
    {CLASS_TYPE_ARACHNOSSOLDIER,"ARACHNOS_SOLDIER"},
    {CLASS_TYPE_ARACHNOSWIDOW,"ARACHNOS_WIDOW"},
    {CLASS_TYPE_UNKNOWN,"UNKNOWN"},
};    
STATIC_ASSERT(ARRAY_SIZE(class_id_str_pairs) == CLASS_TYPE_COUNT);

E_Class_Type GetClassTypeIndex(char * className)
{
    int ct = CLASS_TYPE_UNKNOWN;
    static StashTable ct_from_str = NULL;
    if(!ct_from_str)
    {
        int i;
        ct_from_str = stashTableCreateWithStringKeys(CLASS_TYPE_COUNT, StashDefault);
        for(i = 0; i < ARRAY_SIZE( class_id_str_pairs ); ++i)
            assert(stashAddInt(ct_from_str,class_id_str_pairs[i].str,class_id_str_pairs[i].id,false));
    }
    if(stashFindInt(ct_from_str,className,&ct))
        return ct;
    return CLASS_TYPE_UNKNOWN;
}


const char * GetClassTypeName(int type)
{
    char *str = NULL;
    static StashTable str_from_ct = NULL;
    if(!str_from_ct)
    {
        int i;
        str_from_ct = stashTableCreateInt(CLASS_TYPE_COUNT);
        for(i = 0; i < ARRAY_SIZE( class_id_str_pairs ); ++i)
            assert(stashIntAddPointer(str_from_ct,class_id_str_pairs[i].id,class_id_str_pairs[i].str,false));
    }
    if(stashIntFindPointer(str_from_ct,type,&str))
        return str;
    Errorf("couldn't find string for type %i",type); 
    return NULL;
}



void InitGlobalStats()
{
	int i;

	memset(&g_GlobalStats, 0, sizeof(g_GlobalStats));

	for(i = 0; i < (MAX_PLAYER_SECURITY_LEVEL); i++)
	{
		StatisticInit(&(g_GlobalStats.completedLevelTimeStatistics[i]));
	}

	for(i = 0; i < (MAX_ITEM_TYPES); i++)
	{
		StatisticInit(&(g_GlobalStats.itemsBought[i]));
		StatisticInit(&(g_GlobalStats.itemsSold[i]));
	}

	// stash tables
	g_GlobalStats.missionTimesStash	 = stashTableCreateWithStringKeys(500, StashDeepCopyKeys);
	g_GlobalStats.missionMapTimesStash  = stashTableCreateWithStringKeys(200, StashDeepCopyKeys);
	g_GlobalStats.missionTypeTimesStash = stashTableCreateWithStringKeys(20,  StashDeepCopyKeys);
    g_GlobalStats.actionCounts = stashTableCreateWithStringKeys(500,StashDefault);
}

void InitHourlyStats() {
	memset(&g_HourlyStats,0,sizeof(g_HourlyStats[0][0])*MAX_PLAYER_SECURITY_LEVEL*CLASS_TYPE_COUNT);
}
