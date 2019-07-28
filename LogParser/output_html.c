//
//	output_html.c
//

#include "output_html.h"
#include "overall_stats.h"
#include "StashTable.h"
#include "player_stats.h"
#include "zone_stats.h"
#include "parse_util.h"
#include "html_util.h"
#include "mathutil.h"
#include "plot_util.h"
#include "daily_stats.h"
#include "power_stats.h"
#include "villain_stats.h"
#include "utils.h"
#include <stdlib.h>
#include <direct.h>
#include <assert.h>



char g_WebFolder[MAX_FILENAME_SIZE];		// Ex: "C:\Web\April 7 - 9"
//char g_WebFolderShort[MAX_FILENAME_SIZE];	// Ex: "April 7 - 9"

extern GlobalStats g_GlobalStats;
extern StashTable g_AllPlayers;
extern StashTable g_AllZones;
extern StashTable g_PowerSets;
extern StashTable g_Villain2IndexTable;
const char CITY_ZONE_DIR[] = "CityZones";
const char PLAYER_DIR[] = "Players";

FILE * commandFile;


void OutputHTML(const char * webDir, const char * postProcessScript, const char * mapImageDir)
{	
	char filename[MAX_FILENAME_SIZE];
	FILE * file;
	FILE * mainFile;
	int ret;
	char cmd[10000];
	
	sprintf(g_WebFolder, "%s\\%s", webDir, GetOutDirName(g_GlobalStats.startTime, g_GlobalStats.lastTime));


	printf("\nTarget Dir: %s\n", g_WebFolder);

	// make an empty target folder
	rmdir(g_WebFolder);
	mkdirtree(g_WebFolder);
	mkdir(g_WebFolder);

	sprintf(filename, "%s\\index.html", g_WebFolder);

	mainFile = fopen(filename, "w");
	assert(mainFile);

	OpenPlotCommandFile(g_WebFolder);

	PAGE_BEGIN(mainFile);


		TABLE_BEGIN(mainFile, "Total Connection Stats", 75);

			//TABLE_ROW_2("Total Connections",		sec2StopwatchTime(g_GlobalStats.totalConnections));
			TABLE_ROW_2("Total Connection Time",	sec2StopwatchTime(g_GlobalStats.totalConnectionTime));
			TABLE_ROW_2("Entries",					int2str(g_GlobalStats.totalEntries));
			TABLE_ROW_2("Start Time",				sec2DateTime(g_GlobalStats.startTime));
			TABLE_ROW_2("End Time",					sec2DateTime(g_GlobalStats.lastTime));
			TABLE_ROW_2("Total Log Time",			sec2StopwatchTime(g_GlobalStats.totalLogTime));
			TABLE_ROW_2("Connection Time",			sec2StopwatchTime(g_GlobalStats.totalConnectionTime));
		
		TABLE_END();


		//file = CreateLinkedPage(mainFile, "Connection Stats", "connection_stats");
		//OutputConnectionStats(file);
		//fclose(file);

		file = CreateLinkedPage(mainFile, "Player Stats", "player_stats");
		OutputPlayerStats(file);
		fclose(file);

		file = CreateLinkedPage(mainFile, "Origin/Archetype Stats", "origin_archetype_stats");
		OutputPlayerCategoryStats(file);
		fclose(file);

		OutputMissionStats(mainFile);

		file = CreateLinkedPage(mainFile, "City Zones", "city_zones");
		OutputCityZones(file);
		fclose(file);

		//file = CreateLinkedPage(mainFile, "Daily Stats", "daily_stats");
		OutputDailyStats(mainFile);
		//fclose(file);

		file = CreateLinkedPage(mainFile, "Action Counts", "action_counts");
		OutputActionCounts(file);
		fclose(file);

		//OutputLink(mainFile, "Influence Stats (Economy)", "influence.xls");
		OutputInfluenceStats(mainFile);
		//fclose(file);

//		OutputContactStats(mainFile);

	PAGE_END(mainFile);

	fclose(mainFile);

	ClosePlotCommandFile();

	// run perl script to make spreadsheets & images
	SetCurrentDirectory(g_WebFolder);	// all paths in command file are relative (so that they can be reprocessed anywhere)
	sprintf(cmd, "\"%s\" plot_commands.txt %s", postProcessScript, mapImageDir);
	printf("CMD:\n%s\n", cmd);
	ret = system(cmd);
	assert(ret == 0);
}

void OutputConnectionStats(FILE * file)
{
	char ** list = 0;
	// output total connection stats

	PAGE_BEGIN(file);

	// output XP/reward statistics

	PAGE_END(file);
}

void OutputActionCounts(FILE * file)
{
	int sum = 0;
	int i;
    int j;
	int *sortedIndices = NULL;
    char **sortedIndexNames = NULL;
    
    FOR_EACH_IN_STASHTABLE(g_GlobalStats.actionCounts, int, v)
        eaPush(&sortedIndexNames,stashElementGetStringKey(evElem));
        eaiPush(&sortedIndices,*v);
    FOR_EACH_END;

	// compute sum, and sort.
    for( i = 0; i < eaiSize(&sortedIndices); ++i )
    {
        sum += sortedIndices[i];
        for( j = i+1; j > 0; --j)
        {
            if(sortedIndices[j-1] > sortedIndices[j])
                break;
            eaiSwap(&sortedIndices,j,j-1);
            eaSwap(&sortedIndexNames,j,j-1);
        }
    }
    

	PAGE_BEGIN(file);
    {
        // output list of each action, sorted by count
        
		TABLE_BEGIN(file, "Action Counts (# of times each entry occured in log files)", 100);
        {
            
            TABLE_ROW_BEGIN();
            TABLE_HEADER_CELL("Action");
            TABLE_HEADER_CELL("Entries");
            TABLE_HEADER_CELL("%");
            TABLE_ROW_END();
            
            for(i = 0; i < eaiSize(&sortedIndices) ; i++)
            {
                int count = sortedIndices[i];
                const char * actionName = sortedIndexNames[i];
                
                TABLE_ROW_BEGIN();
                TABLE_ROW_CELL(actionName);
                TABLE_ROW_CELL(int2str(count));
                TABLE_ROW_CELL(PercentString(count, sum));
                TABLE_ROW_END();
            }
        }        
		TABLE_END();
        
    }
	PAGE_END(file);

}

void OutputPlayerStats(FILE * file)
{
	char ** list = 0;
//	int count;
	int i;
//	char filename[MAX_FILENAME_SIZE];
//	char fullPath[MAX_FILENAME_SIZE];
	FILE * playerFile = 0;

	PAGE_BEGIN(file);

		TABLE_BEGIN(file, "Time Spent Per Level (Hrs)", 100);

			TABLE_STATISTIC_HEADER("Level", "Players");
			for(i = 0; i < MAX_PLAYER_SECURITY_LEVEL; i++)
			{			
				Statistic * stat = &(g_GlobalStats.completedLevelTimeStatistics[i]);
				if(stat->count)
				{
					TABLE_TIME_STATISTIC_ROW(int2str(i), stat);
				}
			}

		TABLE_END();


		TABLE_BEGIN(file, "Avg Player Stats per Hour", 100);

			TABLE_ROW_BEGIN();
				TABLE_HEADER_CELL("Level");
				TABLE_HEADER_CELL("Total Hours");
				TABLE_HEADER_CELL("XP");
				TABLE_HEADER_CELL("Debt");
				TABLE_HEADER_CELL("Total XP (debt + xp)");
				TABLE_HEADER_CELL("Influence Gained");
//				TABLE_HEADER_CELL("Inf - Unknown");
//				TABLE_HEADER_CELL("Inf - Defeat");
//				TABLE_HEADER_CELL("Inf - Mission");
//				TABLE_HEADER_CELL("Inf - Store");
				TABLE_HEADER_CELL("Deaths");
				//TABLE_HEADER_CELL("Kills");
			TABLE_ROW_END();

			for(i = 0; i < MAX_PLAYER_SECURITY_LEVEL; i++)
			{			
				int time = g_GlobalStats.totalLevelTime[i];
				float hours	= ((float)time) / 3600.0f;
				if(hours)
				{
					float xp		= (float) g_GlobalStats.xpPerLevel[i];
					float debt		= (float) g_GlobalStats.debtPayedPerLevel[i];
					float inf_unknown	= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_UNKNOWN];
					float inf_defeat	= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_KILL];
					float inf_mission	= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_MISSION];
					float inf_store		= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_STORE];
//					float inf_total	= (float) g_GlobalStats.influencePerLevel[i];
					float inf_total	= inf_unknown+inf_defeat+inf_mission+inf_store;
					float deaths	= (float) g_GlobalStats.deathsPerLevel[i];
					//float kills	= (float) g_GlobalStats.killsPerLevel[i];
					float totalXP	= (float) (xp + debt);
				

					TABLE_ROW_BEGIN();
						TABLE_ROW_CELL(int2str(i));
						TABLE_ROW_CELL(float2str(hours));
						TABLE_ROW_CELL(float2str( xp		/	hours));
						TABLE_ROW_CELL(float2str( debt		/	hours));
						TABLE_ROW_CELL(float2str( totalXP	/	hours));
						TABLE_ROW_CELL(float2str( inf_total	/	hours));
//						TABLE_ROW_CELL(float2str( inf_unknown	/	hours));
//						TABLE_ROW_CELL(float2str( inf_defeat	/	hours));
//						TABLE_ROW_CELL(float2str( inf_mission	/	hours));
//						TABLE_ROW_CELL(float2str( inf_store	/	hours));
						TABLE_ROW_CELL(float2str( deaths	/	hours));
						//TABLE_ROW_CELL(float2str( kills		/	hours));
					TABLE_ROW_END();


				}
			}

		TABLE_END();


		TABLE_BEGIN(file, "Total Stats per Level", 100);

			TABLE_ROW_BEGIN();
				TABLE_HEADER_CELL("Level");
//				TABLE_HEADER_CELL("Total Hours");
//				TABLE_HEADER_CELL("XP");
//				TABLE_HEADER_CELL("Debt");
//				TABLE_HEADER_CELL("Total XP (debt + xp)");
				TABLE_HEADER_CELL("Inf - Total");
				TABLE_HEADER_CELL("Inf - Defeat");
				TABLE_HEADER_CELL("Inf - Mission");
				TABLE_HEADER_CELL("Inf - Store");
				TABLE_HEADER_CELL("Inf - Unknown");
				TABLE_HEADER_CELL("Saved Inf");
				TABLE_HEADER_CELL("Spent Inf - Total");
				TABLE_HEADER_CELL("Spent Inf - Enhancements");
				TABLE_HEADER_CELL("Spent Inf - Inspriations");

//				TABLE_HEADER_CELL("Deaths");
				//TABLE_HEADER_CELL("Kills");
			TABLE_ROW_END();

			for(i = 0; i < MAX_PLAYER_SECURITY_LEVEL; i++)
			{			
				int time = g_GlobalStats.totalLevelTime[i];
				float hours	= ((float)time) / 3600.0f;
				if(hours)
				{
					float xp		= (float) g_GlobalStats.xpPerLevel[i];
					float debt		= (float) g_GlobalStats.debtPayedPerLevel[i];
					float inf_unknown	= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_UNKNOWN];
					float inf_defeat	= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_KILL];
					float inf_mission	= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_MISSION];
					float inf_store		= (float) g_GlobalStats.influencePerLevel[i][INCOME_TYPE_STORE];
//					float inf_total	= (float) g_GlobalStats.influencePerLevel[i];
					float inf_total	= inf_unknown+inf_defeat+inf_mission+inf_store;

					float sinf_insp = (float)g_GlobalStats.influenceSpentPerLevel[i][EXPENSE_TYPE_INSPIRATION];
					float sinf_enh = (float)g_GlobalStats.influenceSpentPerLevel[i][EXPENSE_TYPE_ENHANCEMENT];
					float sinf_total = sinf_insp + sinf_enh;

					float inf_saved = inf_total - sinf_total;


					float deaths	= (float) g_GlobalStats.deathsPerLevel[i];
					//float kills	= (float) g_GlobalStats.killsPerLevel[i];
					float totalXP	= (float) (xp + debt);
				

					TABLE_ROW_BEGIN();
						TABLE_ROW_CELL(int2str(i));
//						TABLE_ROW_CELL(float2str(hours));
//						TABLE_ROW_CELL(float2str( xp		/	hours));
//						TABLE_ROW_CELL(float2str( debt		/	hours));
//						TABLE_ROW_CELL(float2str( totalXP	/	hours));
						TABLE_ROW_CELL(float2str( inf_total));
						TABLE_ROW_CELL(float2str( inf_defeat));
						TABLE_ROW_CELL(float2str( inf_mission));
						TABLE_ROW_CELL(float2str( inf_store));
						TABLE_ROW_CELL(float2str( inf_unknown));
						TABLE_ROW_CELL(float2str( inf_saved));
						TABLE_ROW_CELL(float2str( sinf_total));
						TABLE_ROW_CELL(float2str( sinf_enh));
						TABLE_ROW_CELL(float2str( sinf_insp));
//						TABLE_ROW_CELL(float2str( deaths	/	hours));
						//TABLE_ROW_CELL(float2str( kills		/	hours));
					TABLE_ROW_END();


				}
			}

		TABLE_END();


		//TABLE_BEGIN(file, "General Player Stats", 50);

		//	TABLE_ROW_2("Unique Player Names", int2str(stashGetSize(g_AllPlayers)));

		//TABLE_END();

		//MakeHtmlDir(PLAYER_DIR);

		//TABLE_BEGIN(file, "Individual Player Stats", 90);

		//	TABLE_ROW_BEGIN();
		//		TABLE_HEADER_CELL("Player Name");
		//		TABLE_HEADER_CELL("Level");
		//		TABLE_HEADER_CELL("Total Time");
		//	TABLE_ROW_END();

		//	// output stats for each player
		//	list = GetSortedKeyList(g_AllPlayers, &count);
		//	for(i=0; i<count; i++)
		//	{
		//		char * name = list[i];
		//		PlayerStats * pStats = GetPlayer(name);
		//		sprintf(filename, "%s\\player_%s.html", PLAYER_DIR, name);
		//		TABLE_ROW_BEGIN();

		//			TABLE_ROW_LINK(name, filename);
		//			TABLE_ROW_CELL(int2str(pStats->currentLevel));
		//			TABLE_ROW_CELL(sec2StopwatchTime(pStats->totalTime));

		//			sprintf(fullPath, "%s\\%s", g_WebFolder, filename);
		//			playerFile = fopen(fullPath, "w");
		//			if(playerFile)
		//			{
		//				OutputPlayerPage(playerFile, list[i]);
		//				fclose(playerFile);
		//				TABLE_RESUME(file);
		//			}
		//			else
		//				printf("\nUnable to open file for player \"%s\"", name);

		//		TABLE_ROW_END();
		//	}
		//	free(list);		

		//TABLE_END();

	PAGE_END(file);
}



void CreateMissionTable(const char * title, const char * label0, const char * label1, StashTable table)
{
	char** list;
	int count;
	int i,k;
	char fullTitle[1000];

 	list = GetSortedKeyList(table, &count);


	sprintf(fullTitle, "Worksheet: %s", title);
	PLOT_LINE_ENTRY(fullTitle);

	// header
	PLOT_ENTRY("");
	for(i = 0; i <= MAX_TEAM_SIZE; i++)
	{
		char buf[100];
		if(i)
			sprintf(buf, "Team Size %d", i);
		else
			strcpy_unsafe(buf, "All Team Sizes Combined");

		PLOT_ENTRY(buf);
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
	}
	PLOT_END_LINE();

	PLOT_ENTRY(label0);
	for(i = 0; i <= MAX_TEAM_SIZE; i++)
	{
		PLOT_STATISTIC_HEADER(label1);
		PLOT_ENTRY("");
	}
	PLOT_END_LINE();

	// data
	for(i=0; i<count; i++)
	{
		Statistic * stat;
		stashFindPointer(table, list[i], &stat);
		PLOT_ENTRY(list[i]);
		for(k=0; k <= MAX_TEAM_SIZE; k++)
		{
			PLOT_TIME_STATISTIC(&stat[k]);
			PLOT_ENTRY(""); // insert blank column
		}
		PLOT_END_LINE();
	}

	FreeKeyList(list,count);
}


void OutputMissionStats(FILE * file)
{
	char filename[] = "mission_stats.xls";

	PLOT_NEW("SpreadSheet", 0, filename);
	OutputLink(file, "Mission Stats (XLS)", filename);

		CreateMissionTable("Mission Times", "Mission", "Completions", g_GlobalStats.missionTimesStash);
		CreateMissionTable("Mission Map Times", "Map", "Completions", g_GlobalStats.missionMapTimesStash);
		CreateMissionTable("Mission Type Times", "Type", "Completions", g_GlobalStats.missionTypeTimesStash);

	PLOT_END();
}





void OutputCityZones(FILE * file)
{
	int i, k, count, sum;
	FILE * zoneFile;
	char filename[MAX_FILENAME_SIZE];

	char ** list = GetSortedKeyList(g_AllZones, &count);

	MakeHtmlDir(CITY_ZONE_DIR);

	PAGE_BEGIN(file);

		// output connection stats per map type

		TABLE_BEGIN(file, "Time Spent per Map Type", 100);

			TABLE_ROW_BEGIN();
				TABLE_HEADER_CELL("Type");
				TABLE_HEADER_CELL("Time");
				TABLE_HEADER_CELL("%");
			TABLE_ROW_END();

			sum = 0;
			for(i = 0; i < MAP_TYPE_COUNT; i++)
			{			
				int time = g_GlobalStats.totalMapTypeConnectionTime[i];
				sum += time;
			}

			for(i = 0; i < MAP_TYPE_COUNT; i++)
			{			
				int time = g_GlobalStats.totalMapTypeConnectionTime[i];
				TABLE_ROW_BEGIN();
					TABLE_ROW_CELL(GetMapTypeName(i));
					TABLE_ROW_CELL(sec2StopwatchTime(time));
					TABLE_ROW_CELL(PercentString(time,sum));
				TABLE_ROW_END();
			}

			TABLE_ROW_BEGIN();
				TABLE_ROW_CELL("Totals");
				TABLE_ROW_CELL(sec2StopwatchTime(sum));
				TABLE_ROW_CELL(PercentString(sum,sum));
			TABLE_ROW_END();

		TABLE_END();





		TABLE_BEGIN(file, "Total Hours Spent per Map (per Player Level)", 100);

			TABLE_ROW_BEGIN();
				TABLE_HEADER_CELL("Zone / Player Level");
				
				for(i = 0; i < (MAX_PLAYER_SECURITY_LEVEL / 5); i++)
				{
					char buf[100];
					sprintf(buf, "%d - %d", (i * 5) + 1, ((i + 1) * 5));
					TABLE_HEADER_CELL(buf);
				}
				TABLE_HEADER_CELL("Total");
			TABLE_ROW_END();


			for(i = 0; i < count; i++)
			{

				ZoneStats * pZone = GetZoneStats(list[i]);
				sum = 0;		
				TABLE_ROW_BEGIN();
					TABLE_ROW_CELL(list[i]);

					for(k=0; k < (MAX_PLAYER_SECURITY_LEVEL / 5); k++)
					{
						int time = pZone->timeSpentPerLevel[k];
						sum += time;
						if(time)
							TABLE_ROW_CELL(sec2FormattedHours(time));
						else
							TABLE_ROW_CELL("0");
					}
					TABLE_ROW_CELL(sec2FormattedHours(sum));

				TABLE_ROW_END();
			
			}

		TABLE_END();


		for(i=0; i<count; i++)
		{
			char * zone = list[i];

			// FOR NOW, only city zones
			if(	   strstri(zone, "City_")
				|| strstri(zone, "Hazard_")
				|| strstri(zone, "Trial_"))
			{
				sprintf(filename, "%s\\%s", CITY_ZONE_DIR, zone);
				zoneFile = CreateLinkedPage(file, zone, filename);
				OutputSpecificCityZone(zoneFile, zone);
				fclose(zoneFile);
			}
		}
	PAGE_END(file);


	FreeKeyList(list,count);
}


void OutputSpecificCityZone(FILE * file, char * zoneName)
{
	ZoneStats * zone = GetZoneStats(zoneName);
	char filename[MAX_FILENAME_SIZE];
	char buf[200];
	StashElement element;
	StashTableIterator it;
	int size;
	int i;
	int count;
	int completed = 0; 
	int active = 0;
	int spawned = 0;

	char ** list = GetSortedKeyList(zone->encounterActivityStash, &count);	

	char cityZoneDir[2000]= "CityZones";

	PAGE_BEGIN(file);


		// Frequency Plot (colored circles plotted on top of map image)
		sprintf(filename, "%s_encounter_frequency.jpg", zoneName);
		InsertImage(file, filename);

		PLOT_NEW("Frequency Plot", cityZoneDir, filename);
			PLOT_ENTRY(zoneName);
			PLOT_END_LINE();

			stashGetIterator(zone->encounterActivityStash, &it);
			while(stashGetNextElement(&it,&element))
			{
				const char * pos = stashElementGetKey(element);
				EncounterStats * e = stashElementGetPointer(element);
				PLOT_ENTRY(pos);
				PLOT_ENTRY(int2str(e->totalSpawns));
				//PLOT_ENTRY(int2str(e->completions));
				PLOT_END_LINE();
			}


		PLOT_END();

		// Active Encounters line graph
		sprintf(filename, "%s_encounter_periodic.jpg", zoneName);
		InsertImage(file, filename);

		PLOT_NEW("Line Graph", cityZoneDir, filename);
			sprintf(buf, "Active Encounters (%s)", zoneName);
			PLOT_LINE_ENTRY(buf);
			PLOT_LINE_ENTRY("Elapsed Time (Hours)");
			PLOT_LINE_ENTRY("Active Encounters");

			size = eaiSize(&zone->periodicActiveEncounters);
//			assert(size);	// must have at least 1 spawn, otherwise there is (most likely) a problem
			assert(size == eaiSize(&zone->periodicActivePlayers));		// will be plotting both together

			for(i = 0; i < size; i++)
			{
				PLOT_ENTRY(int2str(i));
				//PLOT_ENTRY(int2str(zone->periodicActivePlayers[i]));
				PLOT_ENTRY(int2str(zone->periodicActiveEncounters[i]));
				PLOT_END_LINE();
			}
		PLOT_END();



		// Spawned Encounters line graph
		sprintf(filename, "%s_encounter_spawned_periodic.jpg", zoneName);
		InsertImage(file, filename);

		PLOT_NEW("Line Graph", cityZoneDir, filename);
		sprintf(buf, "Spawned Encounters (%s)", zoneName);
		PLOT_LINE_ENTRY(buf);
		PLOT_LINE_ENTRY("Elapsed Time (Hours)");
		PLOT_LINE_ENTRY("Spawned Encounters");

		size = eaiSize(&zone->periodicSpawnedEncounters);
		for(i = 0; i < size; i++)
		{
			PLOT_ENTRY(int2str(i));
			PLOT_ENTRY(int2str(zone->periodicSpawnedEncounters[i]));
			PLOT_END_LINE();
		}
		PLOT_END();




		//Active Players line graph
		sprintf(filename, "%s_player_periodic.jpg", zoneName);
		InsertImage(file, filename);

		PLOT_NEW("Line Graph", cityZoneDir, filename);
		sprintf(buf, "Active Players (%s)", zoneName);
		PLOT_LINE_ENTRY(buf);
		PLOT_LINE_ENTRY("Elapsed Time (Hours)");
		PLOT_LINE_ENTRY("Active Players");

		size = eaiSize(&zone->periodicActiveEncounters);
		assert(size == eaiSize(&zone->periodicActivePlayers));		// will be plotting both together

		for(i = 0; i < size; i++)
		{
			PLOT_ENTRY(int2str(i));
			PLOT_ENTRY(int2str(zone->periodicActivePlayers[i]));
			//PLOT_ENTRY(int2str(zone->periodicActiveEncounters[i]));
			PLOT_END_LINE();
		}
		PLOT_END();




		// encounter stats
		TABLE_BEGIN(file, "Encounters", 100);

			TABLE_ROW_BEGIN();
			TABLE_HEADER_CELL("Position");
			TABLE_HEADER_CELL("Spawns");
			TABLE_HEADER_CELL("Set Active");
			TABLE_HEADER_CELL("Completed");
			TABLE_ROW_END();

			for(i=0; i<count; i++)
			{
				char * pos = list[i];
				EncounterStats * e;
				stashFindPointer(zone->encounterActivityStash, pos, &e);
				spawned += e->totalSpawns;
				active += e->totalActive;
				completed += e->totalComplete;
				TABLE_ROW_BEGIN();
				TABLE_ROW_CELL(pos);
				TABLE_ROW_CELL(int2str(e->totalSpawns));
				TABLE_ROW_CELL(int2str(e->totalActive));
				TABLE_ROW_CELL(int2str(e->totalComplete));
				TABLE_ROW_END();
			}

			TABLE_ROW_BEGIN();
				TABLE_ROW_CELL("TOTAL");
				TABLE_ROW_CELL(int2str(spawned));
				TABLE_ROW_CELL(int2str(active));
				TABLE_ROW_CELL(int2str(completed));
			TABLE_ROW_END();

		TABLE_END();

	PAGE_END(file);

	FreeKeyList(list,count);
}



typedef struct PlayerCategoryStats
{
	Statistic timePerLevel[MAX_PLAYER_SECURITY_LEVEL];
	Statistic totalTime;

	int killsPerLevel[MAX_PLAYER_SECURITY_LEVEL];
	int totalKills;

	int deathsPerLevel[MAX_PLAYER_SECURITY_LEVEL];
	int totalDeaths;

//	int xpPerLevel[MAX_PLAYER_SECURITY_LEVEL];
//	int debtPayedPerLevel[MAX_PLAYER_SECURITY_LEVEL];

	int maxLevel;

	int count;

} PlayerCategoryStats;

void PlayerCategoryStatsInit(PlayerCategoryStats * pStats)
{
	int i;

	memset(pStats, 0, sizeof(PlayerCategoryStats));

	StatisticInit(&(pStats->totalTime));

	for(i = 0; i < MAX_PLAYER_SECURITY_LEVEL; i++)
	{
		StatisticInit(&(pStats->timePerLevel[i]));
	}

	pStats->maxLevel = -1;	
}

void PlayerCategoryStatsRecord(PlayerCategoryStats * pCat, PlayerStats * pPlayer)
{
	int deaths;
	int kills;
	int time;
	int i;

	for(i = 0; i < pPlayer->currentLevel; i++)
	{
		deaths = pPlayer->deathsPerLevel[i];
		pCat->deathsPerLevel[i] += deaths;
		pCat->totalDeaths += deaths;

		kills = pPlayer->killsPerLevel[i];
		pCat->killsPerLevel[i] += kills;
		pCat->totalKills += kills;

		time = pPlayer->totalLevelTime[i];
		if(time)
			StatisticRecord(&(pCat->timePerLevel[i]), time);
	}

	StatisticRecord(&(pCat->totalTime), pPlayer->totalTime);

	pCat->count++;
	pCat->maxLevel = MAX(pCat->maxLevel, pPlayer->currentLevel);
}


void CreateCategoryTableHeader(int categoryCount, const char * col1, const char * (*GetPlayerCategoryNameFunction) (int))
{
	int i;

	TABLE_ROW_BEGIN();
		TABLE_HEADER_CELL(col1);
		for(i = 0; i < categoryCount; i++)
		{
			TABLE_HEADER_CELL(GetPlayerCategoryNameFunction(i));
		}
	TABLE_ROW_END();
}

void CreatePlayerCategoryPage(FILE * file, int categoryCount,  int (*GetPlayerCategoryIndexFunction) (PlayerStats *), const char * (*GetPlayerCategoryNameFunction) (int))
{
	StashElement element;
	StashTableIterator it;
	int i,k;
	PlayerCategoryStats * categories = malloc(categoryCount * sizeof(PlayerCategoryStats));

	// obtain data

	for(i = 0; i < categoryCount; i++)
	{
		PlayerCategoryStatsInit(&(categories[i]));
	}

	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it,&element))
	{
		PlayerStats * pStats = stashElementGetPointer(element);
		int index = GetPlayerCategoryIndexFunction(pStats);

		if(index == -1)
			continue;	// skip -1 values

		assert(index >= 0 && index < categoryCount);  

		PlayerCategoryStatsRecord(&(categories[index]), pStats);
	}



	// output overall stats

	TABLE_BEGIN(file, "Average Time Spent Per Level (Per Player) (Hrs)", 100);

	CreateCategoryTableHeader(categoryCount, "", GetPlayerCategoryNameFunction);


		TABLE_ROW_BEGIN();
			TABLE_ROW_CELL("Total Time");

			for(k = 0; k < categoryCount; k++)
			{
				const char * out = "";
				Statistic * time = &(categories[k].totalTime);

				if(time->total)
				{
					out = sec2StopwatchTime(time->total);
				}

				TABLE_ROW_CELL(out);
			}

		TABLE_ROW_END();


		TABLE_ROW_BEGIN();
			TABLE_ROW_CELL("Players");
			for(k = 0; k < categoryCount; k++)
			{
				const char * out = "";
				Statistic * time = &(categories[k].totalTime);

				if(time->total)
				{
					out = int2str(time->count);
				}

				TABLE_ROW_CELL(out);
			}
		TABLE_ROW_END();

		TABLE_ROW_BEGIN();
			TABLE_ROW_CELL("Average Time");

			for(k = 0; k < categoryCount; k++)
			{
				char out[200] = "";
				Statistic * time = &(categories[k].totalTime);

				if(time->total)
				{
					//out = sec2StopwatchTime(StatisticGetAverage(time));
	
					sprintf(out, "%s (%d)", sec2StopwatchTime(StatisticGetAverage(time)), time->count);

				}

				TABLE_ROW_CELL(out);
			}
		TABLE_ROW_END();



	TABLE_END();

	// output time per level

	TABLE_BEGIN(file, "Average Time Spent Per Level (Per Player) (Hrs)", 100);

	CreateCategoryTableHeader(categoryCount, "Level", GetPlayerCategoryNameFunction);

	for(i = 0; i < MAX_PLAYER_SECURITY_LEVEL; i++)
	{
		TABLE_ROW_BEGIN();
		TABLE_ROW_CELL(int2str(i));

		for(k = 0; k < categoryCount; k++)
		{
			char out[200] = "";
			Statistic * time = &(categories[k].timePerLevel[i]);

			if(time->total)
			{
				//out = sec2StopwatchTime(StatisticGetAverage(time));
				sprintf(out, "%s (%d)", sec2StopwatchTime(StatisticGetAverage(time)), time->count);
			}

			TABLE_ROW_CELL(out);
		}

		TABLE_ROW_END();
	}

	TABLE_END();


	// output death rates

	//TABLE_BEGIN(file, "Deaths Per Player Per Hour", 100);

	//CreateCategoryTableHeader(categoryCount, "Level", GetPlayerCategoryNameFunction);

	//	for(i = 0; i < MAX_PLAYER_SECURITY_LEVEL; i++)
	//	{
	//		TABLE_ROW_BEGIN();
	//			TABLE_ROW_CELL(int2str(i));

	//			for(k = 0; k < categoryCount; k++)
	//			{
	//				char out[300] = "";
	//				int time = categories[k].timePerLevel[i].total;
	//				int deaths = categories[k].deathsPerLevel[i];
	//				int playerCount = categories[k].timePerLevel[i].count;

	//				if(time && deaths)
	//				{
	//					float rate = ((float) deaths) / ((float) time);  // total deaths per sec
	//					rate *= 3600;		//total deaths per hour
	//					rate /= playerCount;	//deaths per player per hour

	//					sprintf(out, "%.2f (%d)", rate, deaths);
	//				}

	//				TABLE_ROW_CELL(out);
	//			}

	//		TABLE_ROW_END();
	//	}

	//TABLE_END();

	free(categories);
}

int GetPlayerClassIndex(PlayerStats * pPlayer)
{
	return pPlayer->classType;
}

int GetPlayerOriginIndex(PlayerStats * pPlayer)
{
	return pPlayer->originType;
}

int GetPlayerOriginClassComboIndex(PlayerStats * pPlayer)
{
	return ((pPlayer->originType * CLASS_TYPE_COUNT) + pPlayer->classType);
}

char * GetPlayerOriginClassComboName(int i)
{
	static char buf[200];

	int originIndex = i / CLASS_TYPE_COUNT;
	int classIndex  = i % CLASS_TYPE_COUNT;

	sprintf(buf, "%s/%s", GetClassTypeName(classIndex), GetOriginTypeName(originIndex));
	return (&(buf[0]));
}

int GetPlayerPowerSetComboIndex(PlayerStats * pPlayer)
{
	return ((pPlayer->originType * CLASS_TYPE_COUNT) + pPlayer->classType);
}

int GetPlayerInitialPowerSetIndex(PlayerStats * pPlayer)
{
	if(pPlayer->powerSets[0] && pPlayer->powerSets[1])
	{
		int count = stashGetValidElementCount(g_PowerSets);
		return ((count * pPlayer->powerSets[0]) + pPlayer->powerSets[1]);
	}
	else
	{
		// no info
		return -1;
	}
}

const char * GetPlayerInitialPowerSetName(int i)
{	
	static char buf[MAX_POWER_NAME_LENGTH * 2];
	int count = stashGetValidElementCount(g_PowerSets);
	int setIDs[2];
	char * setNames[2];
	StashElement element;
	StashTableIterator it;
	

	setIDs[0] = i / count;
	setIDs[1] = i % count;

	setNames[0] = 0;
	setNames[1] = 0;

	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it,&element))
	{
		int id = stashElementGetInt(element);

		for(i = 0; i < 2; i++)
		{
			if(setIDs[i] == id)
			{
				setNames[i] = strdup(stashElementGetKey(element));
			}
		}
	}

	assert(setNames[0] && setNames[1]);

	sprintf(buf, "%s/%s", setNames[0], setNames[1]);

	free(setNames[0]);
	free(setNames[1]);

	return &(buf[0]);

}

void OutputPlayerCategoryStats(FILE * file)
{
	int count;
	int size;
	FILE * specificPage;

	specificPage = CreateLinkedPage(file, "Archetype Stats", "player_category_archetypes");
	CreatePlayerCategoryPage(specificPage, CLASS_TYPE_COUNT, GetPlayerClassIndex, GetClassTypeName);
	fclose(specificPage);

	specificPage = CreateLinkedPage(file, "Origin Stats", "player_category_origins");
	CreatePlayerCategoryPage(specificPage, ORIGIN_TYPE_COUNT, GetPlayerOriginIndex, GetOriginTypeName);
	fclose(specificPage);

	specificPage = CreateLinkedPage(file, "Archetype/Origin Combination Stats", "player_category_archetype_origins_combos");
	CreatePlayerCategoryPage(specificPage, (ORIGIN_TYPE_COUNT * CLASS_TYPE_COUNT), GetPlayerOriginClassComboIndex, GetPlayerOriginClassComboName);
	fclose(specificPage);


	// Will be added in once new logging code is in
	size = stashGetValidElementCount(g_PowerSets);
	count = size * (size - 1);
	//CreatePlayerCategoryPage(file, size, GetPlayerInitialPowerSetIndex, GetPlayerInitialPowerSetName);
}

void OutputDailyPlayersAtEachLevel(E_Class_Type classType)
{
	char title[200];
	char type[200];
	int i,k,m;
	int dayCount = eaSizeUnsafe(&g_DailyStats);

	if(classType == CLASS_TYPE_COUNT)
	{
		strcpy_unsafe(type,"Total Existing Players");
	}
	else
	{
		sprintf(type, "Existing %ss", GetClassTypeName(classType));
	}


	sprintf(title, "WorkSheet: %s", type);
	PLOT_LINE_ENTRY(title);


		PLOT_ENTRY("Level");
		for(k = 0; k < dayCount; k++)
		{
			if(g_DailyStats[k])
				PLOT_ENTRY(GetDateStrFromDayOffset(k));
		}
		PLOT_END_LINE();

		for(i=1; i < MAX_PLAYER_SECURITY_LEVEL; i++)
		{
			PLOT_ENTRY(int2str(i));
			for(k = 0; k < dayCount; k++)
			{	
				if(g_DailyStats[k])
				{
					int players = 0;
					if(classType == CLASS_TYPE_COUNT)
					{
						for(m = 0; m < CLASS_TYPE_COUNT; m++)
						{
							players += ((DailyStats*)g_DailyStats[k])->players_per_level[i][m];
						}
					}
					else
					{
						players = ((DailyStats*)g_DailyStats[k])->players_per_level[i][classType]; 
					}
					PLOT_ENTRY(int2str(players));
				}
			}
			PLOT_END_LINE();
		}
}


void OutputInfluenceStats(FILE * file)
{
	int dayCount = eaSizeUnsafe(&g_DailyStats);
	int size = 0;
	char ** list = 0;
	int i;
	char filename[] = "influence.xls";

	PLOT_NEW("SpreadSheet", 0, filename);
	OutputLink(file, "Influence Stats (XLS)", filename);

	// daily influence stats
	PLOT_LINE_ENTRY("WorkSheet: Daily Influence");

	PLOT_ENTRY("Date");
	PLOT_ENTRY("Net +/-");
	PLOT_ENTRY("Total Created");
	PLOT_ENTRY("Total Destroyed");
	PLOT_ENTRY("Enhancements Bought");
	PLOT_ENTRY("Enhancements Sold");
	PLOT_ENTRY("Inspirations Bought");
	PLOT_ENTRY("Inspirations Sold");
	PLOT_ENTRY("/influence created");
	PLOT_ENTRY("/influence destroyed");
	PLOT_ENTRY("Tasks");
	PLOT_ENTRY("Kill Reward");
	PLOT_ENTRY("Unknown");
	PLOT_END_LINE();
	
	for(i = 0; i < dayCount; i++)
	{
		DailyStats * day = g_DailyStats[i];

		if(day)
		{
			int destroyed =   day->influence.buyEnhancements
							+ day->influence.buyInspirations
							+ day->influence.destroyedByCommand;

			int created =     day->influence.createdByCommand
							+ day->influence.earnedByCompleteMission
							+ day->influence.earnedbyCompleteTask
							+ day->influence.earnedByKill
							+ day->influence.sellEnhancements
							+ day->influence.sellInspirations
							+ day->influence.unknown;


			PLOT_ENTRY(GetDateStrFromDayOffset(i));
			PLOT_ENTRY(int2str(created - destroyed));
			PLOT_ENTRY(int2str(created));
			PLOT_ENTRY(int2str(destroyed));
			PLOT_ENTRY(int2str(day->influence.buyEnhancements));
			PLOT_ENTRY(int2str(day->influence.sellEnhancements));
			PLOT_ENTRY(int2str(day->influence.buyInspirations));
			PLOT_ENTRY(int2str(day->influence.sellInspirations));
			PLOT_ENTRY(int2str(day->influence.createdByCommand));
			PLOT_ENTRY(int2str(day->influence.destroyedByCommand));
			PLOT_ENTRY(int2str(day->influence.earnedByCompleteMission + day->influence.earnedbyCompleteTask));
			PLOT_ENTRY(int2str(day->influence.earnedByKill));
			PLOT_ENTRY(int2str(day->influence.unknown));



			PLOT_END_LINE();
		}
	}

	list = GetSortedKeyList(g_ItemIndexMap.table, &size);

	// purchased items
	PLOT_LINE_ENTRY("WorkSheet: Buy");
	PLOT_STATISTIC_HEADER_ROW("Item", "Count");
	for(i = 0; i < size; i++)
	{
		int item = GetIndexFromString(&g_ItemIndexMap, list[i]);
		PLOT_STATISTIC_ROW(list[i], &(g_GlobalStats.itemsBought[item]));
	}

	// sell items
	PLOT_LINE_ENTRY("WorkSheet: Sell");
	PLOT_STATISTIC_HEADER_ROW("Item", "Count");
	for(i = 0; i < size; i++)
	{
		int item = GetIndexFromString(&g_ItemIndexMap, list[i]);
		PLOT_STATISTIC_ROW(list[i], &(g_GlobalStats.itemsSold[item]));
	}


	FreeKeyList(list,size);

	PLOT_END();
}


void OutputDailyStats(FILE * file)
{
	int dayCount = eaSizeUnsafe(&g_DailyStats);
	int powerCount = stashGetValidElementCount(g_PowerIndexMap.table);
	int villainCount = stashGetValidElementCount(g_VillainIndexMap.table);
	int i,k,m;
	int count;
	char filename[] = "daily_stats.xls";
	char ** list = 0;
	StashTableIterator it;
	StashElement el;


	PLOT_NEW("SpreadSheet", 0, filename);
	OutputLink(file, "Daily Stats (XLS)", filename);


	// players at each level 

	OutputDailyPlayersAtEachLevel(CLASS_TYPE_BLASTER);
	OutputDailyPlayersAtEachLevel(CLASS_TYPE_CONTROLLER);
	OutputDailyPlayersAtEachLevel(CLASS_TYPE_DEFENDER);
	OutputDailyPlayersAtEachLevel(CLASS_TYPE_SCRAPPER);
	OutputDailyPlayersAtEachLevel(CLASS_TYPE_TANKER);
	OutputDailyPlayersAtEachLevel(CLASS_TYPE_COUNT);	// All Archetypes Combined

	// player deaths

	PLOT_LINE_ENTRY("Worksheet: Player Deaths");
	PLOT_ENTRY("Archetype");
	for(k = 0; k < dayCount; k++)
	{
		PLOT_ENTRY(GetDateStrFromDayOffset(k));
	}
	PLOT_ENTRY("Total");
	PLOT_END_LINE();
	
	// individual archetypes
	for(i = 0; i < CLASS_TYPE_COUNT-1; i++)
	{
		int sum = 0;

			PLOT_ENTRY(GetClassTypeName(i));
			for(k = 0; k < dayCount; k++)
			{
				int deaths = 0;
				if(g_DailyStats[k])
				{
					for(m = 0; m < MAX_PLAYER_SECURITY_LEVEL; m++)
					{
						deaths += ((DailyStats*)g_DailyStats[k])->deaths_per_level[m][i];
					}
				}
				
				PLOT_ENTRY(int2str(deaths));

				sum += deaths;
				
			}
			PLOT_ENTRY(int2str(sum));
		PLOT_END_LINE();
	}


	// mission completion information
	PLOT_LINE_ENTRY("Worksheet: Mission Completion");
	PLOT_LINE_ENTRY("Mission Completion Information");
	PLOT_ENTRY("Mission");
	PLOT_ENTRY("Completions");
	PLOT_ENTRY("Successes");
	PLOT_ENTRY("Failures");
	PLOT_ENTRY("Success Rate");
	PLOT_ENTRY("");
	PLOT_ENTRY("Success Time");
	PLOT_ENTRY("Failure Time");
	PLOT_END_LINE();

		stashGetIterator(g_MissionCompletion,&it);
		while(stashGetNextElement(&it,&el)) {
			MissionCompletionStats *pMission = stashElementGetPointer(el);
			int total = pMission->countSuccess + pMission->countFailure;

			PLOT_ENTRY(stashElementGetKey(el));
			PLOT_ENTRY(int2str(total));
			PLOT_ENTRY(int2str(pMission->countSuccess));
			PLOT_ENTRY(int2str(pMission->countFailure));
			PLOT_ENTRY(total?float2str(pMission->countSuccess/total):"");
			PLOT_ENTRY("");
			PLOT_ENTRY(pMission->countSuccess?int2str(pMission->timeSuccess/pMission->countSuccess):"");
			PLOT_ENTRY(pMission->countFailure?int2str(pMission->timeFailure/pMission->countFailure):"");

			PLOT_END_LINE();
		}


	// villain kills
	PLOT_LINE_ENTRY("Worksheet: Villain Kills");
	PLOT_LINE_ENTRY("Number of times each villain type killed a Player");

		PLOT_ENTRY("Villain Group");
			for(k = 0; k < dayCount; k++)
			{
				PLOT_ENTRY(GetDateStrFromDayOffset(k));
			}
			PLOT_ENTRY("Total");
		PLOT_END_LINE();

		for(i = 0; i < villainCount; i++)
		{
			int sum = 0;

			PLOT_ENTRY(GetStringFromIndex(&g_VillainIndexMap, i));
			for(k = 0; k < dayCount; k++)
			{
				int kills = 0;
				if(g_DailyStats[k])
				{
					kills = ((DailyStats*)g_DailyStats[k])->villain_stats[i].player_kills;
				}

				PLOT_ENTRY(int2str(kills));

				sum += kills;
			}
			PLOT_ENTRY(int2str(sum));
			PLOT_END_LINE();
		}

	// villain kills
	PLOT_LINE_ENTRY("Worksheet: Villains Deaths");

		PLOT_ENTRY("Villain Group");
		for(k = 0; k < dayCount; k++)
		{
			PLOT_ENTRY(GetDateStrFromDayOffset(k));
		}
		PLOT_ENTRY("Total");
		PLOT_END_LINE();

		for(i = 0; i < villainCount; i++)
		{
			int sum = 0;

			PLOT_ENTRY(GetStringFromIndex(&g_VillainIndexMap, i));
			for(k = 0; k < dayCount; k++)
			{
				int deaths = 0;
				if(g_DailyStats[k])
				{
					deaths = ((DailyStats*)g_DailyStats[k])->villain_stats[i].deaths;
				}

				PLOT_ENTRY(int2str(deaths));

				sum += deaths;
			}
			PLOT_ENTRY(int2str(sum));
			PLOT_END_LINE();
		}

	// villain damage
	PLOT_LINE_ENTRY("Worksheet: Villain Damage Dealt");

		PLOT_ENTRY("Villain Group");
		for(k = 0; k < (int)stashGetValidElementCount(g_DamageTypeIndexMap.table); k++)
		{
			PLOT_ENTRY(GetStringFromIndex(&g_DamageTypeIndexMap, k));
		}
		PLOT_END_LINE();

		for(i = 0; i < villainCount; i++)
		{
			PLOT_ENTRY(GetStringFromIndex(&g_VillainIndexMap, i));
			for(k = 1; k < dayCount; k++)
			{
				if(g_DailyStats[k])	{
					// Add damage from each day into day 0
					for (m=0; m<MAX_DAMAGE_TYPES; m++)
                        ((DailyStats*)g_DailyStats[0])->villain_stats[i].damage[m] += 
						((DailyStats*)g_DailyStats[k])->villain_stats[i].damage[m];
				}
			}
			for (m=0; m<MAX_DAMAGE_TYPES; m++) {
				int dsum = ((DailyStats*)g_DailyStats[0])->villain_stats[i].damage[m];
				if (dsum != 0) 
					PLOT_ENTRY(int2str(dsum));
				else
					PLOT_ENTRY("");
			}
			PLOT_END_LINE();
		}




	// power usage per day
		list = GetSortedKeyList(g_PowerIndexMap.table, &count);
		PLOT_LINE_ENTRY("Worksheet: Power Usage");

			PLOT_ENTRY("Power");
			for(k = 0; k < dayCount; k++)
			{
				PLOT_ENTRY(GetDateStrFromDayOffset(k));
			}
		PLOT_END_LINE();

		// hero powers
		for(i = 0; i < count; i++)
		{
			int sum = 0;

			if(    !strnicmp(list[i], "Blaster", 7)
				|| !strnicmp(list[i], "Defender", 8)
				|| !strnicmp(list[i], "Scrapper", 8)
				|| !strnicmp(list[i], "Tanker", 6)
				|| !strnicmp(list[i], "Controller", 10))
			{
				PLOT_ENTRY(list[i]);
				for(k = 0; k < dayCount; k++)
				{
					int powIndex = GetIndexFromString(&g_PowerIndexMap, list[i]);
					int activated = 0;
					
					if(g_DailyStats[k])
					{
						activated = ((DailyStats*)g_DailyStats[k])->power_stats[powIndex].activated;
					}
					PLOT_ENTRY(int2str(activated));
				}
				PLOT_END_LINE();
			}
		}

		// villain powers (on same sheet as hero powers, at the bottom
		PLOT_LINE_ENTRY("");	// two blank lines;
		PLOT_LINE_ENTRY("");
		for(i = 0; i < count; i++)
		{
			int sum = 0;

			if(    strnicmp(list[i], "Blaster", 7)
				&& strnicmp(list[i], "Defender", 8)
				&& strnicmp(list[i], "Scrapper", 8)
				&& strnicmp(list[i], "Tanker", 6)
				&& strnicmp(list[i], "Controller", 10))
			{
				PLOT_ENTRY(list[i]);
				for(k = 0; k < dayCount; k++)
				{
					int powIndex = GetIndexFromString(&g_PowerIndexMap, list[i]);
					int activated = 0;

					if(g_DailyStats[k])
					{
						activated = ((DailyStats*)g_DailyStats[k])->power_stats[powIndex].activated;
					}
					PLOT_ENTRY(int2str(activated));
				}
				PLOT_END_LINE();
			}
		}


	// heroes per archetype
	PLOT_LINE_ENTRY("Worksheet: Existing Players");

			PLOT_ENTRY("Archetype");
			for(k = 0; k < dayCount; k++)
			{
				PLOT_ENTRY(GetDateStrFromDayOffset(k));
			}
		PLOT_END_LINE();

		// individual archetypes
		for(i = 0; i < CLASS_TYPE_COUNT-1; i++)
		{
			PLOT_ENTRY(GetClassTypeName(i));
			for(k = 0; k < dayCount; k++)
			{
				int players = 0;
				if(g_DailyStats[k])
				{
					for(m = 0; m < MAX_PLAYER_SECURITY_LEVEL; m++)
					{
						players += ((DailyStats*)g_DailyStats[k])->players_per_level[m][i];
					}
				}
				
				PLOT_ENTRY(int2str(players));

			}

			PLOT_END_LINE();
		}
	
	PLOT_END();

	FreeKeyList(list,count);
}



void OutputContactStats(FILE * file)
{
	int dayCount = eaSizeUnsafe(&g_DailyStats);
	int contactCount = 0;
	int playerCount = 0;
	char ** contactList = 0;
	char ** playerList = 0;
	int i,k;
	char filename[] = "contacts.xls";

	PLOT_NEW("SpreadSheet", 0, filename);
	OutputLink(file, "Contact Stats (XLS)", filename);

	contactList = GetSortedKeyList(g_ContactIndexMap.table, &contactCount);
	playerList = GetSortedKeyList(g_AllPlayers, &playerCount);

	// list of contacts owned by each player
	PLOT_LINE_ENTRY("WorkSheet: Player Contacts");
	
	// header
	PLOT_ENTRY("Player Name");
	for(i = 0; i < contactCount; i++)
	{
		char buf[MAX_FILENAME_SIZE];
		// separate path & file extension from contact name
		saUtilFilePrefix(buf, contactList[i]);
		PLOT_ENTRY(buf);
	}
	PLOT_END_LINE();

	// individual players
	
	for(i = 0; i < playerCount; i++)
	{
		PlayerStats * pPlayer = GetPlayer(playerList[i]);
		if(!pPlayer)
		{
			printf("\nError: Couldn't find player %s\n", playerList[i]);
			continue;
		}
		PLOT_ENTRY(playerList[i]);
		for(k=0; k < contactCount; k++)
		{
			int index = GetIndexFromString(&g_ContactIndexMap, contactList[k]);
			if(pPlayer->contacts[index])
				PLOT_ENTRY("YES");
			else
				PLOT_ENTRY("");
		}
		PLOT_END_LINE();
	}

	PLOT_END();

	FreeKeyList(contactList,contactCount);
	FreeKeyList(playerList,playerCount);
}
