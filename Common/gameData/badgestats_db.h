#ifndef BADGESTATS_DB_H
#define BADGESTATS_DB_H

#include "stdtypes.h"

typedef enum 
{
	BADGESTATSTYPE_ENTITY,
	BADGESTATSTYPE_SUPERGROUP,
	BADGESTATSTYPE_MAXTYPES,
} BadgeStatsType;

char* badgestats_ColumnString(BadgeStatsType type, int idx);
void  badgestats_Subtable(BadgeStatsType type, StuffBuff *psb);
int badgestats_GetIndex(BadgeStatsType type, char *table, char *field);

#endif