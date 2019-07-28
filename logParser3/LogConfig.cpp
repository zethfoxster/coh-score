#include "LogConfig.h"

#define BUFFER_SIZE 100000000
#define MIN_BUFFER_SIZE 10000
#define MAX_LINE_SIZE 256

void LpConfig::initLpConfig()
{
	search[0] = 0;
	filePattern[0] = 0;
	startTime = 0;
	endTime = 0;
	abortThreshold = 0;
	directoryName[0] = 0;
	outputName[0] = 0;
	bufferSize = BUFFER_SIZE;
	nThreads = 1;
	maxLineSize = MAX_LINE_SIZE;
	caseSensitive = false;
	useWildcards = false;
}

LpConfig::LpConfig(){initLpConfig();}
LpConfig::~LpConfig(){}

bool LpConfig::isValidLpConfig()
{
	bool searchReady = !!search[0];
	bool fileReady = !!filePattern[0];
	bool directoryReady = !!directoryName[0];
	bool outReady = !!outputName[0];
	bool startReady = startTime!= 0 && LogTime::isDateReasonable(startTime);
	bool endReady= endTime !=0 && LogTime::isDateReasonable(endTime);
	bool abortReady = abortThreshold !=0;
	bool timesOK = (startReady && endReady) || 
		(!endReady && !startReady && !abortReady);
	if(!searchReady || !fileReady || !directoryReady ||
		!outReady || !timesOK)
	{
		printf("Input error: you're missing some necessary command line options:\n");
		if(!searchReady)
			printf("-search\n");
		if(!fileReady)
			printf("-directory\n");
		if(!directoryReady)
			printf("-file\n");
		if(!outReady)
			printf("-out\n");
		if(!timesOK)
			printf("You must either specify both a start and\
				   end time or no times at all.\n-start, -end\
				   , -abort\n");
		return false;
	}
	return true;
}

int LpConfig::readConfigOption(int argc, char **args, int idx, DatePattern &dateReader)
{
	char *option = args[idx];
	if(option)
	{
		if(!stricmp(option, "-search"))
		{
			if(argc>idx+1)
			{
				strcpy(search, args[idx+1]);
				return 2;
			}
			else
			{
				printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
				printf("try: -search \"The Television\"\n");
				return -1;
			}
		}
		else if(!stricmp(option, "-file"))
		{
			if(argc>idx+1)
			{
				strcpy(filePattern, args[idx+1]);
				return 2;
			}
			else
			{
				printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
				printf("try: -file \"entity_2011-##-##-##-##-##.log\"\n");
				printf("# is a wildcard for any number.  @ is a wildcard for any alphabetical character.\n");
				return -1;
			}
		}
		else if(!stricmp(option, "-directory"))
		{
			if(argc>idx+1)
			{
				strcpy(directoryName, args[idx+1]);
				return 2;
			}
			else
			{
				printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
				printf("try: -directory \"\\\\prgfolders2\\ncncftp_Root\\upload\"\n");
				return -1;
			}
		}
		else if(!stricmp(option, "-output"))
		{
			if(argc>idx+1)
			{
				strcpy(outputName, args[idx+1]);
				return 2;
			}
			else
			{
				printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
				printf("try: -output \"TheTelevisionSearch.txt\"\n");
				return false;
			}
		}
		else if(!stricmp(option, "-start"))
		{
			if(argc>idx+1)
			{
				log_time time;

				if(dateReader.dateToTime(args[idx+1], time, strlen(args[idx+1])) && LogTime::isDateReasonable(time))
				{
					startTime = time;
					return 2;
				}
			}

			printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
			printf("try: -start \"YYMMDD hh:mm:ss\"\n");
			printf("example: -start \"110421 17:30:00\"\n");
			return -1;
		}
		else if(!stricmp(option, "-end"))
		{
			if(argc>idx+1)
			{
				log_time time;

				if(dateReader.dateToTime(args[idx+1], time, strlen(args[idx+1])) && LogTime::isDateReasonable(time))
				{
					endTime = time;
					return 2;
				}
			}
			printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
			printf("try: -end \"YYMMDD hh:mm:ss\"\n");
			printf("example: -end \"110421 17:30:00\"\n");
			return -1;
		}
		else if(!stricmp(option, "-threshold"))
		{
			if(argc>idx+1)
			{
				unsigned int hours = atoi(args[idx+1]);

				if(hours < 48)
				{
					log_time threshold;
					LogTime::makeTime(threshold, 0, 0, 0, hours, 0,0);
					abortThreshold = threshold;
					return 2;
				}
			}
			printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
			printf("try: -threshold <number of hours>\n");
			printf("example: -threshold 6\n  \
				   This would mean that we would continue to search a log file \
				   as long as we don't find a date more than 6 hours from your \
				   specified start and end dates.");
			return-1;
		}
		else if(!stricmp(option, "-buffer"))
		{
			if(argc>idx+1)
			{
				unsigned int buffer = atoi(args[idx+1]);

				if(buffer >= MIN_BUFFER_SIZE)
				{
					bufferSize = buffer;
					return 2;
				}
			}
			printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
			printf("try: -buffer <size of per-thread buffer in bytes>\n");
			printf("example: -threshold 10000000\n  \
				   The minimum value is %d.\n", MIN_BUFFER_SIZE);
			return-1;
		}
		else if(!stricmp(option, "-nThreads"))
		{
			if(argc>idx+1)
			{
				unsigned int nThreads = atoi(args[idx+1]);

				if(nThreads > 0)
				{
					nThreads = nThreads;
					return 2;
				}
			}
			printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
			printf("try: -nThreads <number of threads you want to run per search>\n");
			printf("example: -nThreads 4\n");
			return-1;
		}
		else if(!stricmp(option, "-lineSize"))
		{
			if(argc>idx+1)
			{
				unsigned int line = atoi(args[idx+1]);

				if(line >= 30)
				{
					maxLineSize = line;
					return 2;
				}
			}
			printf("Bad command line parameter: %s %s\n", args[idx], args[idx+1]);
			printf("try: -lineSize <max number of characters in a line...>=30>\n");
			printf("example: -lineSize \n");
			return-1;
		}
		else if(!stricmp(option, "-caseSensitive"))
		{
			caseSensitive = true;
			return 1;
		}
		else if(!stricmp(option, "-wildCards"))
		{
			useWildcards = true;
			return 1;
		}
	}
	else
		throw "NULL option provided to readConfigOption";
	return -1;
}

bool LpConfig::readArguments(int argc, char **args)
{
	
	int argsRead = 1;
	DatePattern dateReader(0);
	for(int i = 1; i < argc && argsRead > 0; i+=argsRead)
	{
		argsRead = readConfigOption(argc, args, i, dateReader);
	}
	if(!isValidLpConfig())
	{
		printf("Usage: %s -directory <directoryName> -file <filePattern> ", args[0]);
		printf("-output <outputFilename> -search <search term> -start <YYMMDD hh:mm:ss>");
		printf("-end <YYMMDD hh:mm:ss> [OPIONAL]-threshold <hours>");
		printf("[OPTIONAL]-caseSensitive [OPTIONAL]-wildCards\n");
		return false;
	}
	return true;
}