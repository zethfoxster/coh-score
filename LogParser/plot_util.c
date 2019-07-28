#include "plot_util.h"
#include "utils.h"
#include "parse_util.h"
#include <stdio.h>
#include <assert.h>

static FILE * sg_PlotFile;
const char * PLOT_FILENAME = "plot_commands.txt";

void OpenPlotCommandFile(const char * dir)
{
	char path[MAX_FILENAME_SIZE];

	sprintf(path, "%s\\%s", dir, PLOT_FILENAME);

	sg_PlotFile = fopen(path, "w");
	if(!sg_PlotFile)
	{
		assert(!"Unable to create plot file!");
	}
}


void ClosePlotCommandFile()
{
	fclose(sg_PlotFile);
}

void PLOT_NEW(const char * title, const char * localFolder, const char * filename)
{
	char path[MAX_FILENAME_SIZE];

	if(localFolder)
		sprintf(path, "%s\\%s", localFolder, filename);
	else
		strcpy_unsafe(path, filename); 

	fprintf(sg_PlotFile, "@@BEGIN@@\n%s\n%s\n", title, path);
}

void PLOT_END_LINE()
{
	fprintf(sg_PlotFile, "\n");
}

void PLOT_ENTRY(const char * entry)
{
	fprintf(sg_PlotFile, "%s, ", entry);
}

void PLOT_LINE_ENTRY(const char * entry)
{
	fprintf(sg_PlotFile, "%s\n", entry);
}


void PLOT_STATISTIC_HEADER_ROW(const char * col0, const char * col1)
{
	PLOT_ENTRY(col0);
	PLOT_STATISTIC_HEADER(col1);
	PLOT_END_LINE();
}

void PLOT_STATISTIC_HEADER(const char * col)
{
	PLOT_ENTRY(col);
	PLOT_ENTRY("Total");
	PLOT_ENTRY("Avg");
	PLOT_ENTRY("Min");
	PLOT_ENTRY("Max");
	PLOT_ENTRY("Std Dev");
}

void PLOT_STATISTIC_ROW(const char * title, const Statistic * pStat)
{
	PLOT_ENTRY(title);
	PLOT_STATISTIC(pStat);
	PLOT_END_LINE();
}


void PLOT_STATISTIC(const Statistic * pStat)
{
	PLOT_ENTRY(int2str(pStat->count));
	if(pStat->count)
	{
		PLOT_ENTRY(int2str(pStat->total));
		PLOT_ENTRY(float2str(StatisticGetAverage(pStat)));
		PLOT_ENTRY(int2str(pStat->min));
		PLOT_ENTRY(int2str(pStat->max));
		PLOT_ENTRY(float2str(StatisticGetStandardDeviation(pStat)));
	}
	else
	{
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
	}
}

void PLOT_TIME_STATISTIC(const Statistic * pStat)
{
	PLOT_ENTRY(int2str(pStat->count));
	if(pStat->count)
	{
		PLOT_ENTRY(sec2StopwatchTime(pStat->total));
		PLOT_ENTRY(floatSec2StopwatchTime(StatisticGetAverage(pStat)));
		PLOT_ENTRY(sec2StopwatchTime(pStat->min));
		PLOT_ENTRY(sec2StopwatchTime(pStat->max));
		PLOT_ENTRY(floatSec2StopwatchTime(StatisticGetStandardDeviation(pStat)));
	}
	else
	{
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
		PLOT_ENTRY("");
	}
}

void PLOT_END()
{
	fprintf(sg_PlotFile, "@@END@@\n\n\n");
}
