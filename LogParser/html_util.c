//
// html_util.c
//

#include "html_util.h"
#include "parse_util.h"
#include "overall_stats.h"
#include "plot_util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <direct.h>
#include <assert.h>

static FILE * s_file;
extern char g_WebFolder[MAX_FILENAME_SIZE];	// folder where web page is built
extern GlobalStats g_GlobalStats;

		//HTML EXAMPLE
		//
		//	<table border="1" cellspacing="2" cellpadding="2" width="100%" align="left" style="color: white">
		//	<caption align="TOP">Time Spent per Map Type</caption>
		//	<tr><th>Type</th><th>Time</th><th>%</th></tr>
		//	<tr><td>city</td><td>114:15:07</td><td>76.22</td></tr>
		//	<tr><td>hazard</td><td>4:51:36</td><td>3.24</td></tr>
		//	<tr><td>mission</td><td>30:08:47</td><td>20.11</td></tr>
		//	<tr><td>trial</td><td>38:47</td><td>0.43</td></tr>
		//	<tr><td>TOTAL</td><td>149:54:17</td><td>100.00</td></tr>
		//	</table>



void OutputLink(FILE * file, char * title, char * linkFilename)
{
	fprintf(file, "<p><A HREF=\"%s\">%s</A></p>\n", linkFilename, title);
}


void PAGE_BEGIN(FILE * file)
{
	fprintf(file, "\n<HTML>");
	fprintf(file, "\n<p>START: %s</p>", sec2DateTime(g_GlobalStats.startTime));
	fprintf(file, "\n<p>END:   %s</p>", sec2DateTime(g_GlobalStats.lastTime)); 
}

void PAGE_END(FILE * file)
{
	fprintf(file, "\n</HTML>");
}

void TABLE_BEGIN(FILE * file, const char * title, int width)
{
	TABLE_RESUME(file);
	fprintf(s_file, "\n<TABLE BORDER=1 cellspacing=1 cellpadding=1 WIDTH=%d%% align=left style=\"color: black\">", width);
	fprintf(s_file, "\n\t<caption align=\"TOP\">%s</caption>", title);
}

void TABLE_RESUME(FILE * file)
{
	s_file = file;
}

void TABLE_END()
{
	fprintf(s_file, "\n</TABLE>\n<BR CLEAR=LEFT><BR><BR><BR><BR><BR>");
	s_file = 0;	// make sure it's reset for next table
}

void TABLE_HEADER_CELL(const char * val)
{
	fprintf(s_file, "\n\t<th>%s<th>", val);
}

void TABLE_ROW_BEGIN()
{
	fprintf(s_file, "\n\t\t<tr>");
}

void TABLE_ROW_END()
{
	fprintf(s_file, "</tr>");
}

void TABLE_ROW_CELL(const char * val)
{
	fprintf(s_file, "<td>%s<td>", val);
}


void TABLE_ROW_2(const char * col0, const char * col1)
{
	TABLE_ROW_BEGIN();
		TABLE_ROW_CELL(col0);
		TABLE_ROW_CELL(col1);
	TABLE_ROW_END();
}


void TABLE_ROW_LINK(char * title, char * filename)
{
	char buf[MAX_FILENAME_SIZE];
	sprintf(buf, "<A HREF=\"%s\">%s</A>", filename, title);
	TABLE_ROW_CELL(buf);
}


void TABLE_STATISTIC_HEADER(const char * col0, const char * col1)
{
	TABLE_ROW_BEGIN();
		TABLE_HEADER_CELL(col0);
		TABLE_HEADER_CELL(col1);
		TABLE_HEADER_CELL("Total");
		TABLE_HEADER_CELL("Avg");
		TABLE_HEADER_CELL("Min");
		TABLE_HEADER_CELL("Max");
		TABLE_HEADER_CELL("Std Dev");
	TABLE_ROW_END();
}



void TABLE_TIME_STATISTIC_ROW(const char * title, Statistic * stat)
{
	TABLE_ROW_BEGIN();
		TABLE_ROW_CELL(title);
		TABLE_ROW_CELL(int2str(stat->count));
		TABLE_ROW_CELL(sec2StopwatchTime(stat->total));
		TABLE_ROW_CELL(sec2StopwatchTime((int)StatisticGetAverage(stat)));
		TABLE_ROW_CELL(sec2StopwatchTime(stat->min));
		TABLE_ROW_CELL(sec2StopwatchTime(stat->max));
		TABLE_ROW_CELL(sec2StopwatchTime((int)StatisticGetStandardDeviation(stat)));
	TABLE_ROW_END();
}






void CreateTimeTableFromStatisticStash(const char * title, const char * label0, const char * label1, StashTable table)
{
	char** list;
	int count;
	int i;
	char fullTitle[1000];

	list = GetSortedKeyList(table, &count);


	sprintf(fullTitle, "Worksheet: %s", title);
	PLOT_LINE_ENTRY(fullTitle);
	PLOT_STATISTIC_HEADER_ROW(label0, label1);

	for(i=0; i<count; i++)
	{
		Statistic * stat;
		stashFindPointer(table, list[i], &stat);
		PLOT_STATISTIC_ROW(list[i], stat);
	}

	FreeKeyList(list,count);
}



// used by "GetSortedKeyList()"
int compare( const void *arg1, const void *arg2 )
{
	/* Compare all of both strings: */
	return _stricmp( * ( char** ) arg1, * ( char** ) arg2 );
}


// return an array of keys sorted alphabetically. Caller is responsible for deallocation
char** GetSortedKeyList(StashTable table, int * sizeOut)
{
	StashElement element;
	StashTableIterator it;
	char ** list;
	int size = stashGetValidElementCount(table);
	int i = 0;

	list = malloc(size * sizeof(char*));

	stashGetIterator(table, &it);
	while(stashGetNextElement(&it,&element))
	{
		list[i++] = strdup(stashElementGetKey(element));
	}

	// sort the entries
	qsort(list, size, sizeof(char*), compare);

	*sizeOut = size;
	return list;
}

void FreeKeyList(char **list, int count)
{
	int i;
	for(i=0; i<count; i++) SAFE_FREE(list[i]);
	SAFE_FREE(list);
}


void OutputPlayerPage(FILE * file, const char * playerName)
{
	int i;
	PlayerStats * pStats = GetPlayer(playerName);
	
	PAGE_BEGIN(file);

		TABLE_BEGIN(file, "General", 90);

			TABLE_ROW_2("Name",				pStats->playerName);
			TABLE_ROW_2("AuthName",			pStats->authName);
			TABLE_ROW_2("Current Level",	int2str(pStats->currentLevel));
			TABLE_ROW_2("Last Map",			pStats->currentMap);
			TABLE_ROW_2("XP",				int2str(pStats->currentXP));
			TABLE_ROW_2("Origin",			GetOriginTypeName(pStats->originType));
			TABLE_ROW_2("Class",			GetClassTypeName(pStats->classType));
			TABLE_ROW_2("First Entry",		sec2DateTime(pStats->firstMessageTime));
			TABLE_ROW_2("Last Entry",		sec2DateTime(pStats->lastEntryTime));
			TABLE_ROW_2("Total Time",		sec2StopwatchTime(pStats->totalTime));

		TABLE_END();

		TABLE_BEGIN(file, "Level-Specific Stats", 50);

			TABLE_ROW_BEGIN();
				TABLE_HEADER_CELL("Level");
				TABLE_HEADER_CELL("Time");
				TABLE_HEADER_CELL("XP");
				TABLE_HEADER_CELL("Influence");
				TABLE_HEADER_CELL("Debt Paid");
				TABLE_HEADER_CELL("Deaths");
				TABLE_HEADER_CELL("Kills");
			TABLE_ROW_END();

			for(i = 0; i <= pStats->currentLevel; i++)
			{
				int time = pStats->totalLevelTime[i];
				if(time)
				{
					TABLE_ROW_BEGIN();
						TABLE_ROW_CELL(int2str(i));
						TABLE_ROW_CELL(sec2StopwatchTime(time));
						TABLE_ROW_CELL(int2str(pStats->xpPerLevel[i]));
						TABLE_ROW_CELL(int2str(pStats->influencePerLevel[i]));
						TABLE_ROW_CELL(int2str(pStats->debtPayedPerLevel[i]));
						TABLE_ROW_CELL(int2str(pStats->deathsPerLevel[i]));
						TABLE_ROW_CELL(int2str(pStats->killsPerLevel[i]));
					TABLE_ROW_END();
				}
				else
				{/*
					assert(    0 == pStats->xpPerLevel[i]
							&& 0 == pStats->influencePerLevel[i]
							&& 0 == pStats->debtPayedPerLevel[i]
							&& 0 == pStats->deathsPerLevel[i]
							&& 0 == pStats->killsPerLevel[i]);
				*/
				}
			}

		TABLE_END();

	PAGE_END(file);
}


FILE * CreateLinkedPage(FILE * mainFile, char * linkText, char * linkFilename)
{
	FILE * linkFile;
	char fullFilename[MAX_FILENAME_SIZE];
	char fullPath[MAX_FILENAME_SIZE];

	sprintf(fullFilename, "%s.html", linkFilename);

	sprintf(fullPath, "%s\\%s", g_WebFolder, fullFilename);

	//printf("Link Filename: %s (%s)\n", fullPath, linkFilename);

	OutputLink(mainFile, linkText, fullFilename);

	linkFile = fopen(fullPath, "w");

	return linkFile;
}

FILE * CreateAndLinkFile(FILE * mainFile, char * linkText, char * linkFilename)
{
	FILE * linkFile;
	char fullFilename[MAX_FILENAME_SIZE];
	char fullPath[MAX_FILENAME_SIZE];

	sprintf(fullFilename, "%s", linkFilename);

	sprintf(fullPath, "%s\\%s", g_WebFolder, fullFilename);

	//printf("Link Filename: %s (%s)\n", fullPath, linkFilename);

	OutputLink(mainFile, linkText, fullFilename);

	linkFile = fopen(fullPath, "w");

	return linkFile;
}

void MakeHtmlDir(const char * dir)
{
	char fullDir[MAX_FILENAME_SIZE];
	sprintf(fullDir, "%s\\%s", g_WebFolder, dir);
	mkdir(fullDir);
}



void InsertImage(FILE * file, const char * imageFilename)
{
	fprintf(file, "\n<IMG SRC=\"%s\" ALT=\"missing image file %s\"><BR><BR>\n", imageFilename, imageFilename);
}


