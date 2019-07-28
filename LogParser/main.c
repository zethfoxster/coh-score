#include "memcheck.h"
#include "sysutil.h"
#include "MemoryMonitor.h"
#include "parse_util.h"
#include "StashTable.h"
#include "overall_stats.h"
#include "player_stats.h"
#include "zone_stats.h"
#include "parse_util.h"
#include "output_html.h"
#include "process_entry.h"
#include "daily_stats.h"
#include "utils.h"
#include "common.h"
#include "villain_stats.h"
#include "power_stats.h"
#include "html_util.h"
#include "timing.h"
#include "team.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>


StashTable g_AllPlayers;
StashTable g_AllTeams;
StashTable g_UnknownEntries;
StashTable g_DamageTracker;
StashTable g_PetTracker;
StashTable g_MissionCompletion;

extern GlobalStats g_GlobalStats;


Index2StringMap g_VillainIndexMap;
Index2StringMap g_PowerIndexMap;
Index2StringMap g_ItemIndexMap;
Index2StringMap g_ContactIndexMap;
Index2StringMap g_DamageTypeIndexMap;


FILE * g_ConnectionLog;
FILE * g_LogParserErrorLog;
FILE * g_LogParserUnhandledLog;
FILE * g_TeamActivityLog;
char teamActivityLogFileName[MAX_FILENAME_SIZE];
//FILE * g_rewardLog;
const char * connectionLogName = "connection.log";

int g_processed = 0;
int g_lineCount = 0;
char * g_filename = 0;
char * g_WebDir = 0;

BOOL g_bSort = FALSE;



BOOL g_bCaseSensitive = FALSE;
// Generalized multiscan globals
#define MAX_SCAN_PATTERNS 100
char* g_scanList[MAX_SCAN_PATTERNS];
int g_scanListSize = 0;

// Global variables used for quick log scanning/dumping
BOOL g_bQuickScan = FALSE;
BOOL g_bQuickScanInsensitive = FALSE;
BOOL g_bContinueQuickScan = FALSE;
BOOL g_bQuickTradeAcceptScan = FALSE;
FILE * g_QuickScanLog = NULL;
char g_QuickScanFileName[200];
int g_QuickScanStartTime = 0;
int g_QuickScanFinishTime = 0;

// Global variables used for reward log scanning/dumping
BOOL g_bQuickRewardScan = FALSE;
BOOL g_bRewardScan = FALSE;
BOOL g_bRewardPlayerScan = FALSE;
BOOL g_bRewardCritterScan = FALSE;
BOOL g_bRewardScanDied = FALSE;
FILE * g_RewardScanLog = NULL;

// Global variables used for respec log scanning
BOOL g_bRespecScan = FALSE;
FILE * g_RespecScanLog = NULL;


BOOL g_bSpecialKillScan = FALSE;
char *g_SpecialKillLog = NULL;

char postProcessScript[MAX_FILENAME_SIZE] = "C:\\game\\tools\\util\\make_html_images.bat";
char mapImageDir[MAX_FILENAME_SIZE] = "C:\\Web\\MapImages";


// Global variable to use state saving
BOOL g_bStateSave = FALSE;
BOOL g_bFrequentStateSave = FALSE;

// Global variable to use memmonitor
BOOL g_bMemMonitor = FALSE;

// Structure that contains config information
#define CONFIG_STRUCT_VERSION 1
typedef struct {
	// Server we're operating on
	char * sServerName;

	// Versions of the persistent state structures used 
	int vConfigStruct;
	int vPlayerStruct;
	int vTeamStruct;
	int vZoneStruct;

	// File names of the persistent state files
	char * sPlayerFile;
	char * sTeamFile;
	char * sZoneFile;

	// Information about the current log set
	char * sLastProcessedLog;
	int iLastTimestamp;

} ConfigStruct;

// Base directory to look for config file, data files, etc
char g_baseDir[MAX_FILENAME_SIZE];
ConfigStruct g_currentConfig, g_loadedConfig;
char configFileName[MAX_FILENAME_SIZE];
char playerFileName[MAX_FILENAME_SIZE];
char teamFileName[MAX_FILENAME_SIZE];
char zoneFileName[MAX_FILENAME_SIZE];
char lastProcessedFileName[MAX_FILENAME_SIZE];


ParseTable ConfigStructInfo[] = {
	{ "ServerName",		TOK_STRUCTPARAM | TOK_STRING(ConfigStruct, sServerName, 0)},
	{ "{",			TOK_START,		0 },
	{ "verConfig",				TOK_INT(ConfigStruct, vConfigStruct,0)},
	{ "verPlayer",				TOK_INT(ConfigStruct, vPlayerStruct,0)},
	{ "verTeam",				TOK_INT(ConfigStruct, vTeamStruct,0)},
	{ "verZone",				TOK_INT(ConfigStruct, vZoneStruct,0)},
	{ "filePlayer",				TOK_STRING(ConfigStruct, sPlayerFile,0)},
	{ "fileTeam",				TOK_STRING(ConfigStruct, sTeamFile,0)},
	{ "fileZone",				TOK_STRING(ConfigStruct, sZoneFile,0)},
	{ "fileLastProcessedLog",	TOK_STRING(ConfigStruct, sLastProcessedLog,0)},
	{ "lastTimestamp",			TOK_INT(ConfigStruct, iLastTimestamp,0)},
	{ "}",			TOK_END,			0 },
	{ 0 }
};



typedef enum E_Parse_Mode 
{
	PARSE_MODE_TOTAL, 
	PARSE_MODE_DAY
} E_Parse_Mode;

typedef struct  {
	time_t time;
	char * text;
	unsigned int tiebreaker; // used to force a stable sort, will break if overlowed
} LogEntry;


#define ENTRY_BUFFER_SIZE 6000000
typedef struct  {
	
	int size;
	int count;
	LogEntry data[ENTRY_BUFFER_SIZE];

} EntryBuffer;


#define MAX_LOG_FILES	100000
char * logFileList[MAX_LOG_FILES];
int logFileCount = 0;
int logFileOffset = 0;





void InitConfigStruct(ConfigStruct* pCfgStr) {
	printf("\nInitializing internal configuration...\n");

	sprintf(configFileName,"LogParser_Config.txt");
	sprintf(playerFileName,"LogParserState_Player.txt");
	sprintf(teamFileName,"LogParserState_Team.txt");
	sprintf(zoneFileName,"LogParserState_Zone.txt");
	sprintf(lastProcessedFileName,"");

	pCfgStr->vConfigStruct = CONFIG_STRUCT_VERSION;
	pCfgStr->vPlayerStruct = PLAYER_STRUCT_VERSION;
	pCfgStr->vTeamStruct = TEAM_STRUCT_VERSION;
	pCfgStr->vZoneStruct = ZONE_STRUCT_VERSION;
	pCfgStr->sPlayerFile = playerFileName;
	pCfgStr->sTeamFile = teamFileName;
	pCfgStr->sZoneFile = zoneFileName;
	pCfgStr->sLastProcessedLog = lastProcessedFileName;
}

void LoadConfigStruct(ConfigStruct* pCfgStr) {
	char infilename[200];

	sprintf(infilename,"%s\\%s",g_baseDir,configFileName);
	printf("Loading config file %s...\n",infilename);
	ParserLoadFiles(NULL, infilename, NULL, 0, ConfigStructInfo, pCfgStr);
}

void SaveConfigStruct(ConfigStruct* pCfgStr) {
	char outfilename[200];

	sprintf(outfilename,"%s\\%s",g_baseDir,configFileName);
	printf("Saving config file %s...\n",outfilename);
	ParserWriteTextFile(outfilename, ConfigStructInfo, pCfgStr,0,0);
}

void RebuildState() {
	char fullfilename[200];

	if (!g_bStateSave) return;

	// Init the internal config file
	InitConfigStruct(&g_currentConfig);
	// Load the external config file
	LoadConfigStruct(&g_loadedConfig);

	// If the external config has values for file names and server name, point to those instead of
	//  the internal ones
	if (g_loadedConfig.sServerName) g_currentConfig.sServerName = g_loadedConfig.sServerName;
	if (g_loadedConfig.sPlayerFile) g_currentConfig.sPlayerFile = g_loadedConfig.sPlayerFile;
	if (g_loadedConfig.sTeamFile) g_currentConfig.sTeamFile = g_loadedConfig.sTeamFile;
	if (g_loadedConfig.sZoneFile) g_currentConfig.sZoneFile = g_loadedConfig.sZoneFile;

	// IF the loaded file mentions that we've done some parsing, we actually try to load the data and such
	if (g_loadedConfig.iLastTimestamp != 0) {
		printf("Rebuilding state...\n");
		// If the external config has a value for the last processed file, copy it
		if (g_loadedConfig.sLastProcessedLog) sprintf(lastProcessedFileName,g_loadedConfig.sLastProcessedLog);
		
		// Reset time-tracking stuff
		CorrectTimeZone(g_loadedConfig.iLastTimestamp, true);
		g_GlobalStats.lastTime = g_loadedConfig.iLastTimestamp;

		// Next we attempt to actually rebuild our state
		//  First we do teams
		sprintf(fullfilename,"%s\\%s",g_baseDir,g_currentConfig.sTeamFile);
		TeamStatsListLoad(fullfilename);
		//  Then we do teams so players can reattach to their teams
		sprintf(fullfilename,"%s\\%s",g_baseDir,g_currentConfig.sPlayerFile);
		playerStatsListLoad(fullfilename);
	}
	else
		printf("Not rebuilding state...\n");
}

void SaveState() {
	char fullfilename[200];

	if (!g_bStateSave) return;

	// Write out the saved state
	//  Config
	SaveConfigStruct(&g_currentConfig);
	//  Players
	sprintf(fullfilename,"%s\\%s",g_baseDir,g_currentConfig.sPlayerFile);
	playerStatsListSave(fullfilename);
	//  Teams
	sprintf(fullfilename,"%s\\%s",g_baseDir,g_currentConfig.sTeamFile);
	TeamStatsListSave(fullfilename);
}


void GetCorrectedDBLogFilename(char *filename, char *corrected)
{
	int year, month, day, hour, min, sec;
	int version = 0;
	const char * rest;
	char *under;

	char * p = strstr(filename, "db_");
	assert(p);

	under = strrchr(p,'_');

	if(under == &(p[2]))
	{
		strcpy_unsafe(corrected,p);
	}
	else
	{
		rest = ExtractTokens(p, "db_%d-%d-%d-%d-%d-%d_%d", &year, &month, &day, &hour, &min, &sec, &version);
		assert(rest);
		
		sprintf_s(corrected, 1000, "db_%d-%02d-%02d-%02d-%02d-%02d_%04d", year, month, day, hour, min, sec, version);
	}
}



int compareLogEntries( const void *arg1, const void *arg2 )
{
	// Compare all of both strings:
	int a = ((LogEntry *) arg1)->time;
	int b = ((LogEntry *) arg2)->time;
	
	if(a < b)
		return -1;
	else if(b < a)
		return 1;
	else
	{
		a = ((LogEntry *) arg1)->tiebreaker;
		b = ((LogEntry *) arg2)->tiebreaker;
		// use tiebreaker to force a stable sort
		if(a < b)
			return -1;
		else if(b < a)
			return 1;
		else
			return 0;
	}
}

// used by "GetSortedKeyList()"
int compareLogFilenamesFunc( const void *arg1, const void *arg2 )
{
	// Compare all of both strings:
	return _stricmp( * ( char** ) arg1, * ( char** ) arg2 );
}

int compareCorrectedLogFilenamesFunc( const void *arg1, const void *arg2 )
{
	char arg1c[MAX_FILENAME_SIZE];
	char arg2c[MAX_FILENAME_SIZE];
	GetCorrectedDBLogFilename(* ( char** ) arg1, arg1c);
	GetCorrectedDBLogFilename(* ( char** ) arg2, arg2c);
	return _stricmp( arg1c, arg2c );
}

void EBinit(EntryBuffer * eb)
{
	memset(eb->data, 0, sizeof(LogEntry) * ENTRY_BUFFER_SIZE);
	eb->count = 0;
	eb->size = 0;
}

void EBsort(EntryBuffer * eb)
{
	qsort(eb->data, eb->size, sizeof(LogEntry), compareLogEntries);
}


#define UPDATE_TITLE_INTERVAL	(100000)
void UpdateTitleBar()
{
	static int timer = 0;

	if(!timer)
	{
		timer = timerAlloc();
		timerStart(timer);
	}

	
	if(!(g_processed % UPDATE_TITLE_INTERVAL))
	{
		char buf1[1000];
		char buf2[1000];
		char windowTitle[1000];
		float rate;
		timerPause(timer);
		rate = ((float) g_processed) / timerElapsed(timer);
		timerUnpause(timer);

		sprintf(windowTitle, "Processing %s  %s/%s (%.1f/s)", g_filename, printUnit(buf2, g_lineCount), printUnit(buf1, g_processed), rate);
		SetConsoleTitle(windowTitle);
	}
}

void DoFileCleanup(BOOL forceStateSave)
{
	char tempfilename[MAX_FILENAME_SIZE];

	// Update the processing info
	saUtilFileNoDir(lastProcessedFileName,logFileList[logFileOffset-1]);
	g_currentConfig.iLastTimestamp = g_GlobalStats.lastTime;

	// Fix to allow teamlog writing after the file related to it has been processed
	saUtilFileNoDir(tempfilename,g_filename);
	sprintf(teamActivityLogFileName,"%s\\%s.team",g_baseDir,tempfilename);
	g_TeamActivityLog = fopen(teamActivityLogFileName,"a");

	DisconnectActivePlayers();

	EmptyAllTeams();

	// Reclose the teamlog
	fclose(g_TeamActivityLog);

	CleanupActiveZoneStats();

	saveAllDamageStats();

	// We want to save our state (probably due to the end of a series)
	if (forceStateSave)
		SaveState();

	g_GlobalStats.totalLogTime += (g_GlobalStats.lastTime - g_GlobalStats.logStartTime);
}

int GetTimeFromEntry(const char * entry)
{
	// TEMP
	char date[100];
	int hour, min, sec;
	const char * rest = ExtractTokens(entry, "%10000s %d:%d:%d", date, &hour, &min, &sec);
	
	if(rest)
		return dateTime2Sec(date, hour, min, sec);
	else
		return -1;
}

BOOL IsInitialFile(const char * filename)
{
	// for now, continued filenames end with "-_##.log.sorted"
	if(strstr(filename, "_1.log") || strstr(filename, "_01.log") || strstr(filename, "_001.log") || strstr(filename, "_0001.log"))
	{
		printf("This file (%s) begins a new log series\n",filename);
		return true;
	}
	else
	{
//		printf("\nThis file is a continuation of the prev log\n");
		return false;
	}
}

// correct filenames by padding single-digit increment values (_1.log.gz --> _01.log.gz)
// so that they sort correctly
void CorrectFilenames(char ** filenameList, int count)
{
	int i;
	int year, month, day, hour, min, sec, version;
	const char * rest;
	char ext[200];
	char oldFilename[MAX_FILENAME_SIZE];

	for(i=0; i < count; i++)
	{
		char * p = strstr(filenameList[i], "db_");
		assert(p);

		strcpy_unsafe(oldFilename, filenameList[i]);

//		printf("Changing filename '%s'\np == %s\n", oldFilename, p);

		rest = ExtractTokens(p, "db_%d-%d-%d-%d-%d-%d_%d", &year, &month, &day, &hour, &min, &sec, &version);
		assert(rest);
		
		strcpy_unsafe(ext, rest);
		sprintf_s(p, 1000, "db_%d-%02d-%02d-%02d-%02d-%02d_%04d%s", year, month, day, hour, min, sec, version, ext);

		if(stricmp(oldFilename, filenameList[i]))
		{
			// need to rename file
			rename(oldFilename, filenameList[i]);
		}
	}

}

int InitialEntryTime(char * fname) {
	FILE * pFile = 0;
	char line[MAX_ENTRY_SIZE];
	int initialtime = 0;

	// reset reading mechanism
	getNextLine(0,0);

	// internally adjust time settings (in case server f'ed em up)
	CorrectTimeZone(getTimeFromFilename(fname), true);
	
	// open the file
	if(strstri(fname, ".log.gz"))
		pFile = fopen(fname, "rbz");
	else
		pFile = fopen(fname, "rt");

	assert(pFile && "Unable to open file for reading");

	getNextLine(pFile, line);
	initialtime = GetTimeFromEntry(line);
	printf("%s : Initial Entry Time %d\n",fname,initialtime);

	// reset everything
	fclose(pFile);
	getNextLine(0,0);

	return initialtime;
}

// We finished processing a file, so perform the state-saving tasks associated with this, and prepare for the next one
void ProcessFinishedFile(FILE * pFile, BOOL forceStateSave) {
	if (g_bMemMonitor) memMonitorDisplayStats();
	// Note the name of the file finished
	//  Note the offset is incremented as soon as the new name is pulled
	//  This function cuts off the directory information
	saUtilFileNoDir(lastProcessedFileName,logFileList[logFileOffset-1]);
	g_currentConfig.iLastTimestamp = g_GlobalStats.lastTime;
	// Save the internal state of the parser
	if (g_bFrequentStateSave || forceStateSave) SaveState();
	// Close the file and reset the buffer
	fclose(pFile);
	getNextLine(0,0);
	// Close the team stats file
	if (g_TeamActivityLog) fclose(g_TeamActivityLog);
	g_TeamActivityLog = NULL;
}

// Return true if the file name is less that or equal to the last one we processed
BOOL AlreadyProcessedFileName(char * fname) {
	// Cut off the directory info
	char nodirfname[MAX_FILENAME_SIZE];
	char* stra = nodirfname;
	char* strb = (char*)_strlwr(lastProcessedFileName);
	if(!fname) return 0;
	saUtilFileNoDir(nodirfname,fname);
	_strlwr(nodirfname);
	// Do compare and return
	return (strlen(lastProcessedFileName)>0 && compareCorrectedLogFilenamesFunc(&stra,&strb) <= 0);
}

BOOL ReadNextEntry(LogEntry * entry)
{
	static FILE * pFile = 0;
	char line[MAX_ENTRY_SIZE];
	char tempfilename[MAX_FILENAME_SIZE];
	static unsigned int s_val = 0;	// used to force a stable sort, will break if overflowed

	while(1)	// will either exit with next line or exit '0' ==> no more lines left
	{
		// If we're reading a file and it's still got lines...
		if(pFile && getNextLine(pFile, line))
		{
			entry->text = strdup(line);
			if(g_bSort)
			{
				entry->time = GetTimeFromEntry(line);
				entry->tiebreaker = s_val++;
			}

			g_lineCount++;
			UpdateTitleBar();

			return 1;
		}
		else
		{
			// If we haven't run out of files to scan
			if(logFileCount > logFileOffset)
			{
				// If we had a file open
				if(pFile)
				{
					// Finish it off, don't force a state save
					ProcessFinishedFile(pFile, FALSE);				
				}

				// Get the next file name, increment the offset
				while(AlreadyProcessedFileName(g_filename = logFileList[logFileOffset++])) {
					printf("Skipping %s because last log processed was %s...\n",g_filename,g_currentConfig.sLastProcessedLog);
				}

				if(!g_filename)
					return 0;
//				g_filename = logFileList[logFileOffset++];

				// Another series is next
				if(IsInitialFile(g_filename))
				{
					// If we had a series running already, dump accumulated stuff
					if(pFile)
					{
						// End of series, so request a state save
						DoFileCleanup(TRUE);
						printf("\nProcessed %d lines\n", g_lineCount);
					}

					// Set the reference time for correcting timezone problems
					CorrectTimeZone(getTimeFromFilename(g_filename), true);

					// Reset so that ParseEntry() will update this value
					g_GlobalStats.logStartTime = 0; 
				}

				// supports both ZIP & text formats
				if(strstri(g_filename, ".log.gz"))
					pFile = fopen(g_filename, "rbz");
				else
					pFile = fopen(g_filename, "rt");

				if(!pFile)
				{
					printf("Error: can't open file %s\n\n", g_filename);
					exit(1);
				}
				
				printf("Processing %s...\n", g_filename);

				// Reset buffer
				getNextLine(0,0);	
				g_lineCount = 0;

				// Reset the team log
				saUtilFileNoDir(tempfilename,g_filename);
				sprintf(teamActivityLogFileName,"%s\\%s.team",g_baseDir,tempfilename);
				g_TeamActivityLog = fopen(teamActivityLogFileName,"w");

			}
			else
			{
				// We're out of files to process!  Finish it and request a state save
				ProcessFinishedFile(pFile, TRUE);

				// Clean everything, don't request a finalized state save sice we don't know
				//  if this was the end of a series or not (it probably wasn't)
				DoFileCleanup(FALSE);
				return 0;
			}
		}
	}
}

void EBfill(EntryBuffer * eb)
{
	while(eb->size < ENTRY_BUFFER_SIZE)
	{
		if(!ReadNextEntry(&(eb->data[eb->size])))
		{
			break;
		}
		eb->size++;
	}
	eb->count = 0;
	EBsort(eb);
}




LogEntry * EBnext(EntryBuffer * eb)
{
	int i;

	assert(eb->count <= eb->size);

	if(eb->size == ENTRY_BUFFER_SIZE)
	{
		if(eb->count >= (ENTRY_BUFFER_SIZE / 2))
		{
			int end = eb->size;

			// deallocate first half
			for(i=0; i < eb->count; i++)
			{
				free(eb->data[i].text);
			}
			// copy top half to bottom 
			eb->size = 0;
			for(i = eb->count; i < end; i++)
			{
				eb->data[eb->size++] = eb->data[i];
			}
			// now fill up rest of buffer with new stuff
			EBfill(eb);
		}
	}
	else if(eb->size == 0)
	{
		// initial pass has empty array
		eb->count = 0;
		EBfill(eb);
	}

	if(eb->count < eb->size)
		return &eb->data[eb->count++];
	else
		return 0;

}
void RemoveAsterisks(char * line) {
	int pos = strcspn(line,"*"); 
	int len = strlen(line);
	while (pos < len) {
		line[pos] = ' ';
		pos = strcspn(line,"*");
	}
}

// Parses a '||' seperated string list into a list of individual null-terminated strings,
//  returns number of strings found.  Assumes list has already been allocated,
//  allocates space for each string
int multiargparse(char * arg, char ** list, int maxListSize) {
	char * patternCopy = calloc(strlen(arg)+1,1);
	char * pNextPattern = patternCopy;
	char * pOr;
	int curListSize = 0;

	strcpy_unsafe(patternCopy,arg);

	printf("Parsing %s:\n  ",arg);
	
	while ((pOr = strstr(pNextPattern,"||")) && (curListSize < maxListSize-1)) {
		// Turn the '||' into a pair of null characters
		pOr[0] = pOr[1] = 0;
		list[curListSize] = calloc(strlen(pNextPattern)+1,1);
		strcpy_unsafe(list[curListSize++],pNextPattern);
		printf("%s, ",list[curListSize-1]);
		pNextPattern = &(pOr[2]);
	}
	list[curListSize] = calloc(strlen(pNextPattern)+1,1);
	strcpy_unsafe(list[curListSize++],pNextPattern);
	printf("%s (%d)\n",list[curListSize-1],curListSize);
	free(patternCopy);
	return curListSize;
}

// Returns the index into targets of the first successful strstr, or -1 if none were successful
// Automatically takes g_bCaseSensitive into account
int multistrstr(char * search, char ** targets, int ntargets) {
	int c = 0;
	int rval = -1;
	if (g_bCaseSensitive) {
		while ((c < ntargets) && (!strstr(search,targets[c])))
			c++;
		if (c < ntargets)
			rval = c;
	}
	else {
		while ((c < ntargets) && (!strstri(search,targets[c])))
			c++;
		if (c < ntargets)
			rval = c;
	}
	return rval;
}

// Returns index into targets of the first strcmp that was 0, or -1 if none were 
// Automatically takes g_bCaseSensitive into account
int multistrcmp(char * search, char ** targets, int ntargets) {
	int c = 0;
	int rval = -1;
	if (g_bCaseSensitive) {
		while ((c < ntargets) && (strcmp(search,targets[c])))
			c++;
		if (c < ntargets)
			rval = c;
	}
	else {
		while ((c < ntargets) && (stricmp(search,targets[c])))
			c++;
		if (c < ntargets)
			rval = c;
	}
	return rval;
}


// Resets various global file tracking stuff, opens the next file
// Returns NULL if there isn't a next file
FILE* getNextFile() {
	static FILE * pFile = NULL;
	
	if (pFile) {
		logFileOffset++;
		fclose(pFile);
		pFile = NULL;
	}

	if (logFileOffset < logFileCount) {
		g_filename = logFileList[logFileOffset];
		getNextLine(0, 0);
		g_lineCount = 0;

		printf("Scanning:\t %s\n",g_filename);

		if(strstri(g_filename, ".log.gz"))
			pFile = fopen(g_filename, "rbz");
		else
			pFile = fopen(g_filename, "rt");

		assert(pFile && "Unable to open file for reading");

	}

	return pFile;
}

// Quickly scan the sorted list of db logs, dumping the entry if we find a string match
void QuickDBLogScan() {
	FILE *pFile;
	// Binary search vars
	int bottom = 0;
	int top = logFileCount;

	// Do binary search for appropriate starting log file
	printf("Finding appropriate starting file...\n");
	logFileOffset = bottom + (top-bottom)/2;
	while (bottom < logFileOffset) {
		int filetime;
		printf("Examining file %d: %s...\n",logFileOffset,logFileList[logFileOffset]);
		filetime = InitialEntryTime(logFileList[logFileOffset]);
		if (filetime == g_QuickScanStartTime) {
			bottom = top = logFileOffset;
		}
		else {
			if (filetime < g_QuickScanStartTime) {
				bottom = logFileOffset;
			}
			else {
				top = logFileOffset;
			}
			logFileOffset = bottom + (top-bottom)/2;
		}
	}
	
//	if (!found && InitialEntryTime(logFileList[logFileOffset]) > g_QuickScanStartTime)

	// find the first file that starts AFTER the requested start time
//	while ((logFileOffset <= logFileCount) && 
//		(InitialEntryTime(logFileList[logFileOffset]) < g_QuickScanStartTime)) {
//		logFileOffset++;
//	}

	if (logFileOffset == logFileCount) {
		printf("No files of appropriate time available to scan...\n");
		return;
	}

	// move back one
//	if (logFileOffset > 0) logFileOffset--;

	// while the files don't start after our finish time
	while (pFile = getNextFile()) {
		char line[MAX_ENTRY_SIZE];
		int foundlines = 0;
		int scannedlines = 0;

		if (InitialEntryTime(logFileList[logFileOffset]) > g_QuickScanFinishTime) {
			fclose(pFile);
			break;
		}

		// Dump the log name to the output log
		fprintf(g_QuickScanLog,"--- Start File: %s ---\n",logFileList[logFileOffset]);

		while(getNextLine(pFile, line)){
			scannedlines++;
			if (-1 < multistrstr(line,g_scanList,g_scanListSize)) {
					fprintf(g_QuickScanLog,"%s\n",line);
					foundlines++;
			}
			if (0 == (scannedlines % 10000))
				printf("\r%d in %dk...",foundlines,scannedlines/1000);
		}
	}

	fclose(g_QuickScanLog);
}

// Quickly scan the multitude of reward files, dumping the entry if we find a string match
void QuickRewardScan() {
	char line[MAX_ENTRY_SIZE];
	char linelist[50][MAX_ENTRY_SIZE];
	int curline = 0,c;
	FILE *pFile;
	BOOL ismatch = false;
	int scanned = 0, matches = 0;

	printf("Beginning scanning reward files for entire entries...\n");

	logFileOffset = 0;
	while (pFile = getNextFile()) {
		while (getNextLine(pFile,line)) {
//			if (-1 < multistrstr(line,g_scanList,g_scanListSize))
//				printf("%s\n",line);
			strcpy_unsafe(linelist[curline],line);
			// Is this line empty?
			if (!(line[0] && line[1])) {
				scanned++;
				// If we had a match in this set, print them all out
				if (ismatch) {
					matches++;
					for (c = 0; c < curline; c++)
						fprintf(g_RewardScanLog,"%s\n",linelist[c]);
					fprintf(g_RewardScanLog,"\n");
					printf("\r%d found in %d...",matches,scanned);
				}
				curline=0;
				ismatch = false;
			}
			else {
				// If we haven't found a match yet, and this is a match, note it
				if (!ismatch && -1 < multistrstr(line,g_scanList,g_scanListSize))
					ismatch = true;
				curline++;
				curline = (curline>=50)?49:curline;
			}
		}
	}

	// Close our output file
	fclose(g_RewardScanLog);
}

// One-time function for finding trade_accept cheaters
void QuickTradeAcceptScan() {
	FILE *pFile;
	// find the first file that starts AFTER the requested start time
	while ((logFileOffset < logFileCount) && 
		(InitialEntryTime(logFileList[logFileOffset]) < g_QuickScanStartTime)) {
		logFileOffset++;
	}

	if (logFileOffset == logFileCount) {
		printf("No files of appropriate time available to scan...\n");
		return;
	}

	// move back one
	if (logFileOffset > 0) logFileOffset--;

	// while the files don't start after our finish time
	while (pFile = getNextFile()) {
		char line[MAX_ENTRY_SIZE];
		char *istrade;
		int foundlines = 0;
		int scannedlines = 0;

		if (InitialEntryTime(logFileList[logFileOffset]) > g_QuickScanFinishTime) {
			fclose(pFile);
			break;
		}

		while(getNextLine(pFile, line)){
			scannedlines++;
			if ((istrade = strstri(line,"[chat] trade_accept")) && 
				(strstr(istrade," 0 0 "))) {
					fprintf(g_QuickScanLog,"%s\n",line);
					foundlines++;
			}
			if (0 == (scannedlines % 10000))
				printf("\r%d in %dk...",foundlines,scannedlines/1000);
		}
	}

	fclose(g_QuickScanLog);
}

// Creates a useful condensation of reward information based on teams
void RewardTeamScan() {
	FILE * pFile = 0;
	const char * rest;
	char line[MAX_ENTRY_SIZE];

	int teamlevels[8];
	int teamxps[8];
	int timestamp[6];

	logFileOffset = 0;
	while (pFile = getNextFile()) {
		while(getNextLine(pFile, line)){
			int tdbid, tcnt, ton;
			float tlvl, tkillpct;
			int ttl, ttxp,ttinf,tkml,tmaxl,tminl,tboost,tinsp,thandi;
			char ctarg[30];
			if (strstr(line,"Team") != 0) {
				int i;
				// Found some team info
				tkillpct = 0.0;
				rest = ExtractTokens(line, "Team --------------- %d %f  | cnt: %d   on: %d | %f", &tdbid, &tlvl, &tcnt, &ton, &tkillpct);
				if (tkillpct >= 0.4) {
					ttl = 0; 
					ttxp = 0;
					ttinf = 0;
					tkml = 0;
					tmaxl = 0;
					tminl = 60;
					tboost = 0;
					tinsp = 0;
					thandi = 0;
					for (i = 0; i < ton; i++) {
						int cdbid,ccoml,cxpl,ctargl;
						float pctgs[4];
						int cxp,cinf;
						getNextLine(pFile, line);
						RemoveAsterisks(line);
						rest = ExtractTokens(&(line[20]),"%d  %d %d | %s  %d | %f %f %f %f | %d %d",&cdbid,&ccoml,&cxpl,ctarg,&ctargl,&pctgs[0],&pctgs[1],&pctgs[2],&pctgs[3],&cxp,&cinf);
						ttl += ccoml;
						teamlevels[i] = ccoml;
						teamxps[i] = cxp;
						tminl = (tminl < ccoml) ? tminl : ccoml;
						tmaxl = (tmaxl > ccoml) ? tmaxl : ccoml;
						ttxp += cxp;
						ttinf += cinf;
						if (ctargl > tkml) tkml = ctargl;
						if (rest) {
                            if (strstr(rest,"Boosts.")) tboost = 1;
                            if (strstr(rest,"Inspirations.")) tinsp = 1;
                            if (strstr(rest,"handicap")) thandi = 1;
						}
					}
					fprintf(g_RewardScanLog,"%02d%02d%02d %02d:%02d:%02d ID %d Sz %d TL %d RL %d KL %d %s TX %d TM %d TB %d TI %d HC %d ",timestamp[2],timestamp[0],timestamp[1],timestamp[3],timestamp[4],timestamp[5],tdbid,ton,ttl,tmaxl-tminl,tkml,ctarg,ttxp,ttinf,tboost,tinsp,thandi);
					for (i = 0; i < ton; i++) {
						fprintf(g_RewardScanLog,"(%d %d) ",teamlevels[i],teamxps[i]);
					}
					fprintf(g_RewardScanLog,"\n");
				}
			}
			else {
				if (strstr(line,"Defeated:") != 0) {
					rest = ExtractTokens(line,"%d-%d-%d %d:%d:%d",&timestamp[0],&timestamp[1],&timestamp[2],&timestamp[3],&timestamp[4],&timestamp[5]);
					timestamp[2] -= 2000;
				}
			}
		}
	}

	fclose(g_RewardScanLog);
}

// Creates a useful condensation of reward information based on players
void RewardPlayerScan() {
	FILE * pFile = 0;
	const char * rest;
	char line[MAX_ENTRY_SIZE];

	logFileOffset = 0;
	while (pFile = getNextFile()) {
		char *ctargvg = NULL;
		while(getNextLine(pFile, line)){
			char *pch;
			int tdbid, tcnt, ton;
			float tlvl, tkillpct;
			float ftemp[10];
			char ctarg[30];
			if (strstr(line,"Team") != 0) {
				int i;
				// Found some team info
				tkillpct = 0.0;
				rest = ExtractTokens(line, "Team --------------- %d %f  | cnt: %d   on: %d | %f", &tdbid, &tlvl, &tcnt, &ton, &tkillpct);
				if (tkillpct >= 0.2) {
					for (i = 0; i < ton; i++) {
						int cdbid,ccoml,cxpl,ctargl,cxpgain;
						getNextLine(pFile, line);
						RemoveAsterisks(line);
						rest = ExtractTokens(&(line[20]),"%d  %d %d | %s  %d %f |  %f  %f  %f  %f | %d",
							&cdbid,&ccoml,&cxpl,
							ctarg,&ctargl,&ftemp[0],
							&ftemp[1],&ftemp[2],&ftemp[3],&ftemp[4],
							&cxpgain);
						ctarg[2] = 0;
						if (cxpgain > 0 && rest)
						{
							fprintf(g_RewardScanLog,"dbID %d CLXL %d %d Foe %d %s %s\n",cdbid,ccoml,cxpl,ctargl,ctarg,ctargvg?ctargvg:"UNKNOWN");
						}
					}
				}
			}
			else if (strstr(line,"Reward:") != 0) {
				char cname[30];
				tkillpct = 0.0;
				rest = ExtractTokens(&line[33], "Reward: %s, %s, level %d", cname, ctarg, &tlvl);
				if (strncmp(ctarg,"Mission",7) == 0 || strncmp(ctarg,"ErrandO",7) == 0) {
					int cdbid,ccoml,cxpl,ctargl,cxpgain;
					getNextLine(pFile, line);
					RemoveAsterisks(line);
					rest = ExtractTokens(&(line[20]),"%d  %d %d | %s  %d %f |  %f  %f  %f  %f | %d",
						&cdbid,&ccoml,&cxpl,
						ctarg,&ctargl,&ftemp[0],
						&ftemp[1],&ftemp[2],&ftemp[3],&ftemp[4],
						&cxpgain);
					ctarg[1] = (ctarg[0] == 'M') ? ctarg[7] : 'O'; // MO, MH or EO
					ctarg[2] = 0;
					if (cxpgain > 0 && rest)
					{
						fprintf(g_RewardScanLog,"dbID %d CLXL %d %d Foe %d %s %s\n",cdbid,ccoml,cxpl,ctargl,ctarg,"MISSION");
					}
				}
			}
			else if (0 != (pch = strstr(line,"Defeated: ")))
			{
				char *vge = strchr(&pch[10],'_');
				if (ctargvg) { free(ctargvg); ctargvg = NULL; }
				if (vge)
				{
					*vge = 0;
					ctargvg = strdup(&pch[10]);
					*vge = '_';
				}
			}
		}
	}
	fclose(g_RewardScanLog);
}

// Creates a useful condensation of reward information based on critters
void RewardCritterScan() {
	FILE * pFile = 0;
	const char *rest;
	char line[MAX_ENTRY_SIZE];

	logFileOffset = 0;
	while (pFile = getNextFile()) {
		fprintf(g_RewardScanLog,"-- %s\n",logFileList[logFileOffset]);
		while(getNextLine(pFile, line)){
			char vinfo[MAX_ENTRY_SIZE] = {0};
			char *pch = NULL;
			if (0 != (pch = strstr(line,"Defeated: ")))
			{
				int cdbid,ccoml,cxpl,maxccoml=0,tcoml=0,nc = 0;
				char ctarg[30] = {0};
				char *vin = &pch[10];

				strcpy_unsafe(vinfo,vin);
				getNextLine(pFile, line); // {
				getNextLine(pFile, line); // Damage
				getNextLine(pFile, line); // Breakdown
				if (strncmp(line,"    Breakdown:",14))
					continue;
				getNextLine(pFile, line); // Header
				if (strncmp(line,"                name",20))
					continue;
				getNextLine(pFile, line); // Team
				if (strncmp(line,"Team -",6))
					continue;
				getNextLine(pFile, line); // Reward
				while (line[0] && line[1] && strncmp(line,"Base Salvage --",15)!=0 && strncmp(line," Salvage --",11)!=0)
				{
					ccoml = 0;
					rest = ExtractTokens(&(line[20]),"%d  %d %d | %s  ",
						&cdbid,&ccoml,&cxpl,ctarg);
					getNextLine(pFile, line); // Reward
					if(ccoml)
					{
						nc++;
						tcoml += ccoml;
						if(ccoml > maxccoml) maxccoml = ccoml;
					}
				}
				if (maxccoml && ctarg[0])
				{
					ctarg[2] = 0;
					fprintf(g_RewardScanLog,"%s, %s, %d, %d, %d\n",vinfo,ctarg,maxccoml,tcoml,nc);
				}
				continue;
			}
		}
	}
	fclose(g_RewardScanLog);
}

// Scans the reward files for the deaths of specific mobs
void RewardScanDied() {
	char line[MAX_ENTRY_SIZE];
	FILE * pFile = 0;

	logFileOffset = 0;
	while (pFile = getNextFile()) {
		char *pDied;
		while(getNextLine(pFile, line)){
			if (pDied = strstr(line,"Villain Defeated:")) {
				char *pComma;
				int iDied = 0;
				// Skip to who actually died
				pDied = &(pDied[10]);
				// Switch the comma to a null
				pComma = strchr(pDied,',');
#ifdef CAREFUL
				// Didn't find a comma for some reason, so skip this one and continue
				if (!pComma) {
					while(line[0] && line[1]) getNextLine(pFile,line);
					continue;
				}
#endif
				pComma[0] = 0;

				if (-1 < multistrcmp(pDied,g_scanList,g_scanListSize)) {
					pComma[0] = ',';				
					fprintf(g_RewardScanLog,"%s\n",line);
					while(line[0] && line[1]) {
						getNextLine(pFile,line);
						fprintf(g_RewardScanLog,"%s\n",line);
					}
				}
				else while(line[0] && line[1]) getNextLine(pFile,line);
			}
		}
	}

	fclose(g_RewardScanLog);
}

void addUniqueToList(int i, int* l, int n) {
	int j;
	bool f = false;
	for (j = 0; j < n; j++) {
		if (l[j] == i) {
			f = true;
			break;
		}
		if (l[j] < 0) {
			break;
		}
	}
	if (!f) {
		l[j] = i;
	}
}

// Scans the respec log for useful information
//#define MAX

typedef struct {
	// power indexes
	int iPowerSet;
	int iPower;

	// Count info
	int iPrevOwn, iNewOwn;
	int iPrevEnh, iNewSlot;
} RespecPower;

RespecPower* getRespecPower(int powset, int pow, RespecPower** eaPowers, bool create) {
	int j;
	RespecPower* pRP;
//	const char * pname = (pow>=0)?GetStringFromIndex(&g_PowerIndexMap, pow):GetStringFromIndex(&g_PowerIndexMap, powset);
//	int s = eaSizeUnsafe(&eaPowers)-1;
//	printf("%s %d\n",pname,s);

	for (j=eaSizeUnsafe(&eaPowers)-1; j>=0; j--) {
		pRP = eaPowers[j];
		if (pRP->iPowerSet == powset && pRP->iPower == pow) {
			return pRP;
		}
	}
	if (!create) return NULL;
	pRP = (RespecPower*)calloc(1,sizeof(RespecPower));
	pRP->iPowerSet = powset;
	pRP->iPower = pow;
	eaPush(&eaPowers,pRP);
	return pRP;
}

void RespecScan() {
	char line[MAX_ENTRY_SIZE];
	FILE * pFile = 0;
	char * tmp;
	RespecPower** eaPowers;
	RespecPower* pRP;
	int nPowerPrev[MAX_POWER_TYPES];
	int nPowerNew[MAX_POWER_TYPES];
	int nPowerSetPrev[MAX_POWER_TYPES];
	int nPowerSetNew[MAX_POWER_TYPES];
	int nPowerSetPowersPrev[MAX_POWER_TYPES];
	int nPowerSetPowersNew[MAX_POWER_TYPES];
	int nPowerSlotPrev[MAX_POWER_TYPES];
	int nPowerSlotNew[MAX_POWER_TYPES];
	int powerSetMap[MAX_POWER_TYPES];
	int i;
	int stamina = 0;

	eaCreate(&eaPowers);
	eaSetCapacity(&eaPowers,MAX_POWER_TYPES);
	
	for (i=0; i < MAX_POWER_TYPES; i++) {
		nPowerPrev[i] = 0;
		nPowerNew[i] = 0;
		nPowerSetPrev[i] = 0;
		nPowerSetNew[i] = 0;
		nPowerSetPowersPrev[i] = 0;
		nPowerSetPowersNew[i] = 0;
		nPowerSlotPrev[i] = 0;
		nPowerSlotNew[i] = 0;
		powerSetMap[i] = 0;
	}


	logFileOffset = 0;
	while (pFile = getNextFile()) {
		while(getNextLine(pFile, line)){
			if (strstr(line,"Began verifying")) {
				char powerName[200];
				char powersetName[200];
				char powerType[200];
				char powerBase[200];
				int powerSetsOld[10];
				int powerSetsNew[10];
				char playername[100];
				int ps,p;

				// Set up our simple powerset tracker
				for (i=0; i<10; i++) {
					powerSetsOld[i] = powerSetsNew[i] = -1;
				}

				ExtractTokens(line,"Began verifying respec for player player %s (",playername);
//				printf("Player %s\n",playername);

				// We're at the top of a new entry
				getNextLine(pFile, line);
				while (tmp = strstr(line,"Recieved power")) getNextLine(pFile,line);


				// Make list of prior powers
				while (tmp = strstr(line,"Prev Power:")) {
					ExtractTokens(tmp,"Prev Power: %s %s",powersetName,powerName);
					ps = GetIndexFromString(&g_PowerIndexMap,powersetName);
					p = GetIndexFromString(&g_PowerIndexMap, powerName);
					pRP = getRespecPower(ps,p,eaPowers,true);
					pRP->iPrevOwn++;
					pRP = getRespecPower(ps,-1,eaPowers,true);
					pRP->iPrevEnh++;
//					printf("Prev: %s %s\n",powersetName,powerName);
//					nPowerPrev[p]++;
//					powerSetMap[p] = ps;
//					nPowerSetPowersPrev[ps]++;
					addUniqueToList(ps,powerSetsOld,10);
					getNextLine(pFile,line);
				}

				// Note the initial powers
				while (tmp = strstr(line,"Initial Power:")) {
					ExtractTokens(tmp,"Initial Power: %s %s",powersetName,powerName);
					ps = GetIndexFromString(&g_PowerIndexMap,powersetName);
					p = GetIndexFromString(&g_PowerIndexMap, powerName);
					pRP = getRespecPower(ps,p,eaPowers,true);
					pRP->iNewOwn++;
					pRP = getRespecPower(ps,-1,eaPowers,true);
					pRP->iNewSlot++;
//					printf("Init: %s %s\n",powersetName,powerName);
//					nPowerNew[p]++;
//					powerSetMap[p] = ps;
//					nPowerSetPowersNew[ps]++;
					addUniqueToList(ps,powerSetsNew,10);
					getNextLine(pFile,line);
				}

				
				// Make list of bought powers
				while (tmp = strstr(line,"Bought power ")) {
					if (NULL == strstr(line,"Bought power set ")) {
						ExtractTokens(tmp,"Bought power %s %s.%s.%s",powerType,powerBase,powersetName,powerName);
						ps = GetIndexFromString(&g_PowerIndexMap,powersetName);
						p = GetIndexFromString(&g_PowerIndexMap, powerName);
						pRP = getRespecPower(ps,p,eaPowers,true);
						pRP->iNewOwn++;
						pRP = getRespecPower(ps,-1,eaPowers,true);
						pRP->iNewSlot++;
//						printf("New: %s %s\n",powersetName,powerName);
//						nPowerNew[p]++;
//						powerSetMap[p] = ps;
//						nPowerSetPowersNew[ps]++;
						addUniqueToList(ps,powerSetsNew,10);
					}
					getNextLine(pFile,line);

				}

				// Skip over inventory enhancements
				while (tmp = strstr(line," Inventory Slot ")) getNextLine(pFile,line);

				// Get old slotting info
				while (tmp = strstr(line," Power ")) {
					ExtractTokens(tmp," Power %s,",powerName);
//					printf("OldE: %s\n",powerName);
//					nPowerSlotPrev[GetIndexFromString(&g_PowerIndexMap, powerName)]++;
					getNextLine(pFile,line);
				}

				// Skip ahead
				getNextLine(pFile,line);
				getNextLine(pFile,line);


				// Get new slotting info
				while (tmp = strstr(line," Power ")) {
					int nslots;
					ExtractTokens(tmp," Power %s has %d slots available",powerName,&nslots);
//					printf("Slot: %s %d\n",powerName,nslots);
//					nPowerSlotNew[GetIndexFromString(&g_PowerIndexMap, powerName)] += nslots;
					getNextLine(pFile,line);
				}

				// Dump powerset stuff
//				printf("Old PowerSets\n");
				for (i=0;i<10;i++) {
					if (powerSetsOld[i] < 0) break;
//					printf("%s\n",GetStringFromIndex(&g_PowerIndexMap,powerSetsOld[i]));
					pRP = getRespecPower(powerSetsOld[i],-1,eaPowers,true);
					pRP->iPrevOwn++;
//					nPowerSetPrev[powerSetsOld[i]]++;
				}
//				printf("New PowerSets\n");
				for (i=0;i<10;i++) {
					if (powerSetsNew[i] < 0) break;
//					printf("%s\n",GetStringFromIndex(&g_PowerIndexMap,powerSetsNew[i]));
					pRP = getRespecPower(powerSetsNew[i],-1,eaPowers,true);
					pRP->iNewOwn++;
//					nPowerSetNew[powerSetsNew[i]]++;
				}

			}
		}
	}

	fprintf(g_RespecScanLog,"Power Sets\t\tPrior Owners\tNew Owners\tOwners Delta\t\tPrior Powers\tNew Powers\tPowers Delta\t\tPrior Pow/Own\tNew Pow/Own\tPow/Own Delta\n");
	for (i = eaSizeUnsafe(&eaPowers)-1; i >= 0; i--) {
//	while (i < MAX_POWER_TYPES && NULL != (pRP = eaPowers[i])) {
		pRP = eaPowers[i];
		if (pRP && pRP->iPower == -1) {
			float perprev = (pRP->iPrevOwn)?(float)pRP->iPrevEnh/(float)pRP->iPrevOwn:0.0;
			float pernew = (pRP->iNewOwn)?(float)pRP->iNewSlot/(float)pRP->iNewOwn:0.0;
			fprintf(g_RespecScanLog,"%s\t\t%d\t%d\t%d\t\t%d\t%d\t%d\t\t%.1f\t%.1f\t%.1f\n",
				GetStringFromIndex(&g_PowerIndexMap, pRP->iPowerSet),
				pRP->iPrevOwn,
				pRP->iNewOwn,
				pRP->iNewOwn - pRP->iPrevOwn,
				pRP->iPrevEnh,
				pRP->iNewSlot,
				pRP->iNewSlot - pRP->iPrevEnh,
				perprev,
				pernew,
				pernew - perprev);
		}
	}

	fprintf(g_RespecScanLog,"\n\nPower Set\tPower\t\tPrior Owners\tNew Owners\tOwners Delta\tPct Delta\t\tPrior Enhancements\tNew Slots\n");
	for (i = eaSizeUnsafe(&eaPowers)-1; i >= 0; i--) {
		pRP = eaPowers[i];
//	while (i < MAX_POWER_TYPES) {
		if (pRP && pRP->iPower >= 0) {
			fprintf(g_RespecScanLog,"%s\t%s\t\t%d\t%d\t%d\t%.1f\t\t%.1f\t%.1f\n",
				GetStringFromIndex(&g_PowerIndexMap,pRP->iPowerSet),
				GetStringFromIndex(&g_PowerIndexMap,pRP->iPower),
				pRP->iPrevOwn,
				pRP->iNewOwn,
				pRP->iNewOwn - pRP->iPrevOwn,
				(pRP->iPrevOwn)?100.0*(float)(pRP->iNewOwn - pRP->iPrevOwn)/(float)pRP->iPrevOwn:100.0,
				(pRP->iPrevOwn)?(float)pRP->iPrevEnh/(float)pRP->iPrevOwn:0.0,
				(pRP->iNewOwn)?(float)pRP->iNewSlot/(float)pRP->iNewOwn:0.0);
		}
	}

	fclose(g_RespecScanLog);
}



int quicktime(char * tm) {
	int yr = digits2Number(&tm[0], 2);
	int mo = digits2Number(&tm[2], 2);
	int dy = digits2Number(&tm[4], 2);
	int hr = digits2Number(&tm[6], 2);
	int mn = digits2Number(&tm[8], 2);
	int sc = digits2Number(&tm[10], 2);
	return sc+60*(mn+60*(hr+24*(dy+31*(mo+12*yr))));
}


// Horrible one-time function for cross-referencing certain kills for retroactive badge rewarding
void SpecialKillScan() {
	char line[MAX_ENTRY_SIZE];
	char line_dam[MAX_ENTRY_SIZE];
	char line_break[MAX_ENTRY_SIZE];
	char line_header[MAX_ENTRY_SIZE];
	char line_team[MAX_ENTRY_SIZE];
	char matchline[MAX_ENTRY_SIZE];
	FILE * pMatchFile = 0;
	int *eaCOTKills, *eaVahzKills, *eaClockKills;
	int *eaCOTIDs, *eaVahzIDs, *eaClockIDs;
	int *eaCOTMatches, *eaVahzMatches, *eaClockMatches;
	int cotfound = 0;
	int vahzfound = 0;
	int clockfound = 0;
	int c;


	// Open the match file
	pMatchFile = fopen(g_SpecialKillLog,"rt");

	// Create arrays
	eaiCreate(&eaCOTKills);
	eaiCreate(&eaVahzKills);
	eaiCreate(&eaClockKills);
	eaiCreate(&eaCOTIDs);
	eaiCreate(&eaVahzIDs);
	eaiCreate(&eaClockIDs);
	eaiCreate(&eaCOTMatches);
	eaiCreate(&eaVahzMatches);
	eaiCreate(&eaClockMatches);

	// Fill arrays
	getNextLine(0, 0);
	while (getNextLine(pMatchFile, matchline)) {
		char date[100], datestr[100];
		int tid;
		int hour, min, sec;
		const char * rest = ExtractTokens(matchline, "%10000s %d:%d:%d", date, &hour, &min, &sec);
		int killtime;
		size_t cc;
		sprintf(datestr,"%s%02d%02d%02d",date,hour,min,sec);
		killtime=quicktime(datestr);

		for (cc = 0; cc<strlen(matchline); cc++) {
			if (matchline[cc] == '[')
				break;
		}
		cc -= 2;
		tid = 0;
		while ((cc > 0) && (matchline[cc] != ' ')) {
			tid++;
			cc--;
		}
		cc++;
		tid=digits2Number(&matchline[cc],tid);


		//rest = ExtractTokens(line, "%s-%s-%s %d:%d:%d", dmonth, dday, dyear, &hour, &min, &sec);

		// loop through all the lines in the match file
		if (strstr(matchline,"Lucion:")) {
			//COT Killed
			eaiPush(&eaCOTKills,killtime);
			eaiPush(&eaCOTIDs,tid);
			eaiPush(&eaCOTMatches,0);
		}
		else if (strstr(matchline,"Ulcer:")) {
			//Vahz Killed
			eaiPush(&eaVahzKills,killtime);
			eaiPush(&eaVahzIDs,tid);
			eaiPush(&eaVahzMatches,0);
		}
		else if (strstr(matchline,"Boresight:")) {
			//Clock Killed
			eaiPush(&eaClockKills,killtime);
			eaiPush(&eaClockIDs,tid);
			eaiPush(&eaClockMatches,0);
		}
	}

	printf("Searching for %d Lucion, %d Ulcer, %d Boresight\n",eaiSize(&eaCOTKills),eaiSize(&eaVahzKills),eaiSize(&eaClockKills));

	logFileOffset = 0;
	// Here we actually do our scanning
	while (logFileOffset < logFileCount) {
		FILE * pFile = 0;
		char *pDied;
		g_filename = logFileList[logFileOffset];
		getNextLine(0, 0);
		g_lineCount = 0;

		printf("Scanning %s..\n",g_filename);

		// supports both ZIP & text formats
		if(strstri(g_filename, ".log.gz"))
			pFile = fopen(g_filename, "rbz");
		else
			pFile = fopen(g_filename, "rt");

		while(getNextLine(pFile, line)){
			if (pDied = strstr(line,"Defeated:")) {
				char *pComma;
				int dateval;
				int iDied = 0;
				// Skip to who actually died
				pDied = &(pDied[10]);
				// Switch the comma to a null
				pComma = strchr(pDied,',');
				pComma[0] = 0;

				if (strcmp(pDied,"CircleOfThorns_The_Circle_Madness") == 0) {
					//Might be a Lucion
					int i;
					int mo = digits2Number(&line[0], 2);
					int dy = digits2Number(&line[3], 2);
					int yr = digits2Number(&line[8], 2);
					int hr = digits2Number(&line[11], 2);
					int mn = digits2Number(&line[14], 2);
					int sc = digits2Number(&line[17], 2);
					dateval = sc+60*(mn+60*(hr+24*(dy+31*(mo+12*yr))));
//					printf(" COT: %d (from %02d%02d%02d%02d%02d%02d)\n",dateval,yr,mo,dy,hr,mn,sc);
					for (i=0; i<eaiSize(&eaCOTKills); i++) {
						if (eaCOTKills[i] == dateval) {
							int tid,cc;
							getNextLine(pFile,line_dam);
							getNextLine(pFile,line_break);
							getNextLine(pFile,line_header);
							getNextLine(pFile,line_team);
							// if this line is empty, break
							if (!(line_team[0] && line_team[1]))
								break;
							// skip the spaces
							cc = 20;
							while (line_team[cc] == ' ') cc++;
							// get the teamid
							ExtractTokens(&line_team[cc],"%d",&tid);

							if ((tid <= 0) || (tid != eaCOTIDs[i])) {
								// skip the rest of the entry and get out
								while(line[0] && line[1])
									getNextLine(pFile,line);
								break;
							}

							// matched the team id and time and kill type, should be legit
							eaCOTMatches[i] += 1;
							cotfound++;
							fprintf(g_RewardScanLog,"%s\n",line);
							fprintf(g_RewardScanLog,"%s\n",line_dam);
							fprintf(g_RewardScanLog,"%s\n",line_break);
							fprintf(g_RewardScanLog,"%s\n",line_header);
							fprintf(g_RewardScanLog,"%s\n",line_team);
							while(line[0] && line[1]) {
								getNextLine(pFile,line);
								fprintf(g_RewardScanLog,"%s\n",line);
							}
						}
						else if (eaCOTKills[i] > dateval)
							break;
					}
				}
				if (strcmp(pDied,"Vahzilok_Mire_Eidolon") == 0) {
					//Might be a Ulcer
					int i;
					int mo = digits2Number(&line[0], 2);
					int dy = digits2Number(&line[3], 2);
					int yr = digits2Number(&line[8], 2);
					int hr = digits2Number(&line[11], 2);
					int mn = digits2Number(&line[14], 2);
					int sc = digits2Number(&line[17], 2);
					dateval = sc+60*(mn+60*(hr+24*(dy+31*(mo+12*yr))));
					for (i=0; i<eaiSize(&eaVahzKills); i++) {
						if (eaVahzKills[i] == dateval) {
							int tid,cc;
							getNextLine(pFile,line_dam);
							getNextLine(pFile,line_break);
							getNextLine(pFile,line_header);
							getNextLine(pFile,line_team);
							// if this line is empty, break
							if (!(line_team[0] && line_team[1]))
								break;
							// skip the spaces
							cc = 20;
							while (line_team[cc] == ' ') cc++;
							// get the teamid
							ExtractTokens(&line_team[cc],"%d",&tid);

							if ((tid <= 0) || (tid != eaVahzIDs[i])) {
								// skip the rest of the entry and get out
								while(line[0] && line[1])
									getNextLine(pFile,line);
								break;
							}

							// matched the team id and time and kill type, should be legit
							eaVahzMatches[i] += 1;
							vahzfound++;
							fprintf(g_RewardScanLog,"%s\n",line);
							fprintf(g_RewardScanLog,"%s\n",line_dam);
							fprintf(g_RewardScanLog,"%s\n",line_break);
							fprintf(g_RewardScanLog,"%s\n",line_header);
							fprintf(g_RewardScanLog,"%s\n",line_team);
							while(line[0] && line[1]) {
								getNextLine(pFile,line);
								fprintf(g_RewardScanLog,"%s\n",line);
							}
						}
						else if (eaVahzKills[i] > dateval)
							break;
					}
				}
				if (strcmp(pDied,"Clockwork_Lieutenant_Cannon") == 0) {
					//Might be a Boresight
					int i;
					int mo = digits2Number(&line[0], 2);
					int dy = digits2Number(&line[3], 2);
					int yr = digits2Number(&line[8], 2);
					int hr = digits2Number(&line[11], 2);
					int mn = digits2Number(&line[14], 2);
					int sc = digits2Number(&line[17], 2);
					dateval = sc+60*(mn+60*(hr+24*(dy+31*(mo+12*yr))));
					for (i=0; i<eaiSize(&eaClockKills); i++) {
						if (eaClockKills[i] == dateval) {
							int tid,cc;
							getNextLine(pFile,line_dam);
							getNextLine(pFile,line_break);
							getNextLine(pFile,line_header);
							getNextLine(pFile,line_team);
							// if this line is empty, break
							if (!(line_team[0] && line_team[1]))
								break;
							// skip the spaces
							cc = 20;
							while (line_team[cc] == ' ') cc++;
							// get the teamid
							ExtractTokens(&line_team[cc],"%d",&tid);

							if ((tid <= 0) || (tid != eaClockIDs[i])) {
								// skip the rest of the entry and get out
								while(line[0] && line[1])
									getNextLine(pFile,line);
								break;
							}

							// matched the team id and time and kill type, should be legit
							eaClockMatches[i] += 1;
							clockfound++;
							fprintf(g_RewardScanLog,"%s\n",line);
							fprintf(g_RewardScanLog,"%s\n",line_dam);
							fprintf(g_RewardScanLog,"%s\n",line_break);
							fprintf(g_RewardScanLog,"%s\n",line_header);
							fprintf(g_RewardScanLog,"%s\n",line_team);
							while(line[0] && line[1]) {
								getNextLine(pFile,line);
								fprintf(g_RewardScanLog,"%s\n",line);
							}
						}
						else if (eaClockKills[i] > dateval)
							break;
					}
				}
				else {
					// Speed up the scanning
					while(line[0] && line[1]) getNextLine(pFile,line);
				}
			}
		}

		fclose(pFile);

		// Increment
		logFileOffset++;
	}

	

	printf("Found %d Lucion, %d Ulcer, %d Boresight\n",cotfound,vahzfound,clockfound);
	for (c=0; c < eaiSize(&eaCOTKills); c++) {
		if (eaCOTMatches[c] != 1)
			fprintf(g_RewardScanLog,"Lucion (%d) at %d has %d matches\n",eaCOTIDs[c],eaCOTKills[c],eaCOTMatches[c]);
	}
	for (c=0; c < eaiSize(&eaVahzKills); c++) {
		if (eaVahzMatches[c] != 1)
			fprintf(g_RewardScanLog,"Ulcer (%d) at %d has %d matches\n",eaVahzIDs[c],eaVahzKills[c],eaVahzMatches[c]);
	}
	for (c=0; c < eaiSize(&eaClockKills); c++) {
		if (eaClockMatches[c] != 1)
			fprintf(g_RewardScanLog,"Boresight (%d) at %d has %d matches\n",eaClockIDs[c],eaClockKills[c],eaClockMatches[c]);
	}

	fclose(g_RewardScanLog);
	printf("Done\n");
}


void parseArgs(int argc, char **argv) {
	int i;
	// Parse the command line
	for(i=1;i<argc;i++)
	{
		// Web directory: -webdir dirname
		if (stricmp(argv[i],"-webdir")==0)
			g_WebDir = argv[++i];
		// Post processing script: -script scriptname
		else if (stricmp(argv[i],"-script")==0)
			strcpy_unsafe(postProcessScript, argv[++i]);
		// Map directory: -mapdir dirname
		else if (stricmp(argv[i],"-mapdir")==0)
			strcpy_unsafe(mapImageDir, argv[++i]);
		// Mem monitor tracking: -memmonitor
		else if(stricmp(argv[i],"-memmonitor")==0)
			g_bMemMonitor = TRUE;
		// Base directory: -basedir dirname
		else if (stricmp(argv[i],"-basedir")==0)
			strcpy_unsafe(g_baseDir, argv[++i]);
		// Quick scan: -quickscan pattern outputlogfile starttime endtime
		else if (	(stricmp(argv[i],"-quickscan")==0) ||
					(stricmp(argv[i],"-iquickscan")==0)) {

			g_bQuickScan = TRUE;
			if (stricmp(argv[i],"-iquickscan")==0) {
				g_bQuickScanInsensitive = TRUE;
				g_bCaseSensitive = FALSE;
			}
			else 
				g_bCaseSensitive = TRUE;
			
			g_scanListSize = multiargparse(argv[++i],g_scanList,MAX_SCAN_PATTERNS);
			g_QuickScanLog = fopen(argv[++i], "w");
			assert(g_QuickScanLog);
			g_QuickScanStartTime = GetTimeFromEntry(argv[++i]);
			g_QuickScanFinishTime = GetTimeFromEntry(argv[++i]);
			printf("Performing %sQuickScan from %d to %d\n",g_bQuickScanInsensitive?"Case-Insensitive ":"",g_QuickScanStartTime,g_QuickScanFinishTime);
		}
		// Quick trade_accept scan: -quicktascan outputlogfile starttime endtime
		else if (stricmp(argv[i],"-quicktascan")==0) {

			g_bQuickTradeAcceptScan = TRUE;
			
			g_QuickScanLog = fopen(argv[++i], "w");
			assert(g_QuickScanLog);
			g_QuickScanStartTime = GetTimeFromEntry(argv[++i]);
			g_QuickScanFinishTime = GetTimeFromEntry(argv[++i]);
			printf("Performing trade_accept QuickScan from %d to %d\n",g_QuickScanStartTime,g_QuickScanFinishTime);
		}
		// Reward scan: -quickrewardscan outputlogfile
		else if ((stricmp(argv[i],"-quickrewardscan")==0) || 
				(stricmp(argv[i],"-iquickrewardscan")==0)) {
			g_bQuickRewardScan = TRUE;
			g_bCaseSensitive = (stricmp(argv[i],"-quickrewardscan")==0);
			g_scanListSize = multiargparse(argv[++i],g_scanList,MAX_SCAN_PATTERNS);
			g_RewardScanLog = fopen(argv[++i], "w");
			assert(g_RewardScanLog);
			printf("Will performing entry-based reward scan...\n");
		}
		// Reward scan: -rewardscan outputlogfile
		else if (stricmp(argv[i],"-rewardscan")==0) {
			g_bRewardScan = TRUE;
			g_RewardScanLog = fopen(argv[++i], "w");
			assert(g_RewardScanLog);
			printf("Performing Reward Scan\n");
		}
		// Reward player scan: -rewardplayerscan outputlogfile
		else if (stricmp(argv[i],"-rewardplayerscan")==0) {
			g_bRewardPlayerScan = TRUE;
			g_RewardScanLog = fopen(argv[++i], "w");
			assert(g_RewardScanLog);
			printf("Performing Reward Player Scan\n");
		}
		// Reward critter scan: -rewardcritterscan outputlogfile
		else if (stricmp(argv[i],"-rewardcritterscan")==0) {
			g_bRewardCritterScan = TRUE;
			g_RewardScanLog = fopen(argv[++i], "w");
			assert(g_RewardScanLog);
			printf("Performing Reward Critter Scan\n");
		}
		// Respec scan: -respecscan outputlogfile
		else if (stricmp(argv[i],"-respecscan")==0) {
			g_bRespecScan = TRUE;
			printf("%s\n",argv[i+1]);
			g_RespecScanLog = fopen(argv[++i], "w");
			assert(g_RespecScanLog);
			printf("Performing Respec Scan\n");
		}
		// Reward scan: -rewardkillscan whodied outputlogfile
		else if (stricmp(argv[i],"-rewardkillscan")==0) {
			g_bRewardScanDied = TRUE;
			g_scanListSize = multiargparse(argv[++i],g_scanList,MAX_SCAN_PATTERNS);
			g_RewardScanLog = fopen(argv[++i], "w");
			assert(g_RewardScanLog);

			printf("Performing Reward Scan for Deaths\n");
		}
		// Special case Reward scan: -specialkillscan matchfile outputlogfile
		else if (stricmp(argv[i],"-specialkillscan")==0) {
			g_bSpecialKillScan = TRUE;
			g_SpecialKillLog = argv[++i];
			assert(g_SpecialKillLog);
			g_RewardScanLog = fopen(argv[++i], "w");
			assert(g_RewardScanLog);
		}
		// General State Saving
		else if (stricmp(argv[i],"-statesave")==0) {
			g_bStateSave = TRUE;
			printf("State Saving is On...\n");
		}
		// Frequent State Saving
		else if(stricmp(argv[i],"-frequentstatesave")==0) {
			g_bStateSave = TRUE;
			g_bFrequentStateSave = TRUE;
			printf("Frequent State Saving is On...\n");
		}
	}
}

BOOL buildLogFileList(int argc, char **argv) {
	int i;
	char path[MAX_FILENAME_SIZE];
	int handle;
	struct _finddata_t finddata;
	char filename[MAX_FILENAME_SIZE];
	// Build the log file list

	printf("Building log file list...");

	for(i = 1; i < argc; i++)
	{
		// Jump to the -log command line entry
		if(0 != stricmp(argv[i],"-log"))
			continue;

		// Copy the file glob following -log and get the directory prefix
		strcpy_unsafe(path, argv[++i]);
		getDirectoryName(&path[0]);

		// Build a list of the files indicated by the log
		handle = _findfirst(argv[i], &finddata);
		if(handle != -1) 
		{
			do
			{
				if (strcmp(finddata.name, ".")==0 || strcmp(finddata.name, "..")==0)
					continue;
				concatpath(path, finddata.name, filename);
				backSlashes(filename);

				logFileList[logFileCount] = malloc(sizeof(char) * MAX_FILENAME_SIZE);
				strcpy_unsafe(logFileList[logFileCount++], filename);

				assert(logFileCount <= MAX_LOG_FILES);
		
			} while(_findnext(handle, &finddata) == 0);
				_findclose(handle);
		}
		else
		{
			printf("Bad File name: %s", argv[i]);
			return 0;
		}
	}

	printf("found %d potential log files\n",logFileCount);

	return 1;
}






int main(int argc,char **argv)
{
	char filename[MAX_FILENAME_SIZE];
	LogEntry entry;
	E_Parse_Mode mode = PARSE_MODE_TOTAL;
	int parseTest = 0;		// default is "production" mode
	char * webDir = "C:\\Sample Web Page\\City of Heroes"; // default
	char teamStatsFilename[MAX_FILENAME_SIZE] = "team.txt";
	char errorLogFilename[MAX_FILENAME_SIZE] = "LogParserError.log";

	EntryBuffer * eb = malloc(sizeof(EntryBuffer));

	EXCEPTION_HANDLER_BEGIN
    WAIT_FOR_DEBUGGER // allows -WaitForDebugger on command line
	DO_AUTO_RUNS  

	// Default the base directory to the local directory, can be changed with -basedir
	strcpy_unsafe(g_baseDir,".\\");
	g_WebDir = webDir;

	parseArgs(argc, argv);
	if (!buildLogFileList(argc, argv))
		return 1;
	
	if (g_bMemMonitor) memMonitorInit();

	// initialize stats
	printf("Initializing stashes...\n");
	g_AllPlayers		= stashTableCreateWithStringKeys(200000, StashDeepCopyKeys);
	g_AllTeams			= stashTableCreateWithStringKeys( 10000, StashDeepCopyKeys);
	g_AllZones			= stashTableCreateWithStringKeys(  1000, StashDeepCopyKeys);
	g_PowerSets			= stashTableCreateWithStringKeys(   500, StashDeepCopyKeys);
	g_UnknownEntries	= stashTableCreateWithStringKeys(   300, StashDeepCopyKeys);
	g_DamageTracker		= stashTableCreateWithStringKeys( 50000, StashDeepCopyKeys);
	g_PetTracker		= stashTableCreateWithStringKeys(  5000, StashDeepCopyKeys);
	g_MissionCompletion = stashTableCreateWithStringKeys(   500, StashDeepCopyKeys);


	printf("Initializing string maps...\n");
	InitIndex2StringMap(&g_PowerIndexMap, MAX_POWER_TYPES);
	InitIndex2StringMap(&g_VillainIndexMap, MAX_VILLAIN_TYPES);
	InitIndex2StringMap(&g_ItemIndexMap, MAX_ITEM_TYPES);
	InitIndex2StringMap(&g_ContactIndexMap, MAX_CONTACTS);
	InitIndex2StringMap(&g_DamageTypeIndexMap,MAX_DAMAGE_TYPES);

	// Rebuild the state (if needed)
	printf("Rebuilding state...\n");
	RebuildState();

	// set up mapping from 
	printf("Initializing entry map and global stats...\n");
	InitEntryMap();
	InitGlobalStats();

	// init daily stats
	printf("Initializing daily stats...\n");
	eaCreate(&g_DailyStats);

	printf("Checking special scan states...\n");
	// If we're quick scanning the rewardfile
	if (g_bQuickRewardScan) {
		printf("Calling QuickRewardScan()...\n");
		QuickRewardScan();
		return 0;
	}

	// If we're just scanning the rewardfile for team stuff
	if (g_bRewardScan) {
		RewardTeamScan();
		return 0;
	}

	// If we're scanning the rewardfile for player stuff
	if (g_bRewardPlayerScan) {
		RewardPlayerScan();
		return 0;
	}

	// If we're scanning the rewardfile for critter stuff
	if (g_bRewardCritterScan) {
		RewardCritterScan();
		return 0;
	}

	// If we're just scanning for rewardfile deaths
	if (g_bRewardScanDied) {
		RewardScanDied();
		return 0;
	}

	// If we're scanning the respec log
	if (g_bRespecScan) {
		RespecScan();
		return 0;
	}

	if (g_bQuickTradeAcceptScan) {
		QuickTradeAcceptScan();
		return 0;
	}

	// If we're doing our super-special rewardfile death crosscheck crap
	if (g_bSpecialKillScan) {
		SpecialKillScan();
		return 0;
	}

	printf("Sorting and renaming log file list...\n");
	// now, prepare the list of files... we do the correcting internally during the sort now, rather
	//  than actually renaming the files, as renaming causes conflicts with the replication system
//	CorrectFilenames(logFileList, logFileCount);
//	qsort(logFileList, logFileCount, sizeof(char*), compareLogFilenamesFunc);
	qsort(logFileList, logFileCount, sizeof(char*), compareCorrectedLogFilenamesFunc);

	printf("Sorted...\n");

	g_processed=0;

	// If we are just quick-scanning, do the scan stuff and exit
	if (g_bQuickScan) {
		QuickDBLogScan();
		return 0;
	}

	// init error logs
	printf("Initializing error logs...\n");
	sprintf(filename, "%s\\LogParser_Error.log",g_baseDir);
	g_LogParserErrorLog = fopen(filename,"a");
	assert(g_LogParserErrorLog);
	sprintf(filename, "%s\\LogParser_Unhandled.log",g_baseDir);
	g_LogParserUnhandledLog = fopen(filename,"a");
	assert(g_LogParserUnhandledLog);

	while(ReadNextEntry(&entry)) {
		if(!ProcessEntry(entry.text)) {
			printf("ERR: %s\n", entry.text);
			fprintf(g_LogParserErrorLog,"(%s line %d) %s\n",g_filename,g_lineCount,entry.text);
		}
		g_processed++;
		free(entry.text);
		UpdateTitleBar();
	}

	// Close error logs
	fclose(g_LogParserErrorLog);
	fclose(g_LogParserUnhandledLog);

	// Finalize the data we've been tracking
	DisconnectActivePlayers();
	saveAllDamageStats();
	DailyStatsFinalizeAll();

	OutputHTML(g_WebDir, postProcessScript, mapImageDir);

    EXCEPTION_HANDLER_END

	return 0;
}
