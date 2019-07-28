#ifndef _INDEX_2_STRING_MAP
#define _INDEX_2_STRING_MAP

#include "StashTable.h"
#include <stdlib.h>
#include <assert.h>

typedef struct Index2StringMap
{
	int max;		// This value provides basic bounds checking for fixed-sized arrays that uses this structure
	StashTable table;
} Index2StringMap;

void InitIndex2StringMap(Index2StringMap * pMap, int max); 
int GetIndexFromString(Index2StringMap * map, const char * name);
const char * GetStringFromIndex(Index2StringMap * map, int index);


#endif // _INDEX_2_STRING_MAP