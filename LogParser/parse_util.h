//
// parseutil.h
//

#ifndef _PARSE_UTIL_H
#define _PARSE_UTIL_H

#include "process_entry.h"
#include <stdio.h>
#include <time.h>

#include <windows.h>

#define MAX_FILENAME_SIZE	1000
#define MAX_ENTRY_SIZE		10000
#define MAX_DIGIT_LENGTH	100


const char * ParseEntry(const char * entry, int * time, char * mapName, char *mapIp, char * entityName, int * teamID, E_Action_Type * action);

const char * ExtractTokens(const char * source, const char * fmt, ...);

const char * ExtractStringToken(const char * source, char * out, int (*IsCharacterFunction) (int), int maxCount);

const char * ExtractIntToken(const char * source, int * out);
const char * ExtractFloatToken(const char * source, float * out);

int getNextLine(FILE * pFile, char * line);
//int getNextLine(HANDLE hFile, char * line);


int digits2Number(const char * numberString, int count);

char * PercentString(int, int);

const char * int2str(int i);
const char * float2str(float f);

int dateTime2Sec(char * date, int hour, int min, int sec);
const char * sec2DateTime(time_t time);

const char * sec2Date(time_t time);

char * sec2StopwatchTime(int time);
char * floatSec2StopwatchTime(float time);
int stopwatchTime2Sec(const char * timeStr);

const char * sec2FormattedHours(int time);

const char * GetOutDirName(time_t startTime, time_t endTime);

// strip the directory and extension from the file (copied from storyarcutil.c)
void saUtilFilePrefix(char *buffer, char* source);
void saUtilFileNoDir(char *buffer, char* source);

int CorrectTimeZone(int time, bool bReset);
int getTimeFromFilename(const char * filename);

#endif  // _PARSE_UTIL_H
