#ifndef _HTML_UTIL_H
#define _HTML_UTIL_H

#include "StashTable.h"
#include "Statistic.h"
#include "player_stats.h"
#include <stdio.h>



void PAGE_BEGIN(FILE * file);
void PAGE_END(FILE * file);

void TABLE_BEGIN(FILE * file, const char * title, int width);
void TABLE_END();
void TABLE_HEADER_CELL(const char * val);
void TABLE_ROW_BEGIN();
void TABLE_ROW_END();
void TABLE_ROW_CELL(const char * val);
void TABLE_ROW_2(const char * col0, const char * col1);
void TABLE_ROW_LINK(char * title, char * filename);
void TABLE_RESUME(FILE * file); // hack to allow FILE ptrs to not need to be set every time a TABLE* function is called

void TABLE_STATISTIC_HEADER(const char * col0, const char * col1);
void TABLE_TIME_STATISTIC_ROW(const char * title, Statistic * stat);


void CreateTimeTableFromStatisticStash(const char * title, const char * label0, const char * label1, StashTable table);
void OutputLink(FILE * file, char * title, char * linkFilename);

void OutputPlayerPage(FILE * file, const char * playerName);

char** GetSortedKeyList(StashTable table, int * sizeOut);
void FreeKeyList(char **list, int count);

FILE * CreateLinkedPage(FILE * mainFile, char * linkText, char * linkFilename);
FILE * CreateAndLinkFile(FILE * mainFile, char * linkText, char * linkFilename);

void InsertImage(FILE * file, const char * imageFilename);


void MakeHtmlDir(const char * dir);

int compare( const void *arg1, const void *arg2 );


#endif  // _HTML_UTIL_H