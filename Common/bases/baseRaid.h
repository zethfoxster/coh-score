/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#ifndef _BASERAID_H
#define _BASERAID_H

#define MAX_SG_IOP					4
#define	RAID_MAP_ARRAY_SIZE			128
#define MAX_SGS_IN_RAID				2

typedef struct Base Base;
typedef struct Entity Entity;

typedef struct RaidMapData
{
	int minx;
	int minz;
	int maxx;
	int maxz;
	char mapArray[RAID_MAP_ARRAY_SIZE][RAID_MAP_ARRAY_SIZE];
} RaidMapData;

typedef enum eRaidType
{
	RAID_TYPE_IOP = 0,
	RAID_TYPE_FFA,
	RAID_TYPE_CTF,
	RAID_TYPE_MAYHEM,
	RAID_TYPE_COUNT
} eRaidType;

extern RaidMapData g_sgRaidMapData;
extern int g_sgRaidSGIDs[MAX_SGS_IN_RAID];
extern int g_sgRaidRaidIDs[MAX_SGS_IN_RAID];
extern unsigned int g_sgRaidNumSgInRaid;
extern int g_isBaseRaid;
extern Base **g_raidbases;
extern eRaidType g_baseRaidType;

void sgRaidConvertRaidBasesToMapData();
int sgRaidGetTeamNumber(Entity *e);
int sgRaidGetGangIDByIndex(unsigned int index);
void sgRaidCommonSetup();

#endif
