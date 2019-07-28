/***************************************************************************
 *     Copyright (c) 2008, NCsoft
 *     All Rights Reserved
 *     Confidential Property of NCsoft
 ***************************************************************************/

#include "group.h"
#include "bases.h"
#include "earray.h"
#include "entity.h"
#include "character_target.h"

RaidMapData g_sgRaidMapData;
int g_sgRaidSGIDs[MAX_SGS_IN_RAID];
int g_sgRaidRaidIDs[MAX_SGS_IN_RAID];
unsigned int g_sgRaidNumSgInRaid = 0;
static int gangid[64];
static unsigned int maxGangID;

void sgRaidConvertRaidBasesToMapData()
{
	int i, j, k;
	int x, z;
	int px, pz;
	int minx, minz, maxx, maxz;
	Base *base;
	BaseRoom *room;
	RoomDoor *door;
	
	memset(g_sgRaidMapData.mapArray, 0, sizeof(g_sgRaidMapData.mapArray));
	minx = 100000;
	minz = 100000;
	maxx = -100000;
	maxz = -100000;
	// Scan the bases twice.  The first pass gets the map extents, the second pass
	// places the data in the map array.

	for (i = eaSize(&g_raidbases) - 1; i >= 0; i--)
	{
		base = g_raidbases[i];
		for (j = eaSize(&base->rooms) - 1; j >= 0; j--)
		{
			room = base->rooms[j];
			px = (int) (room->pos[0] + base->ref->mat[3][0] + 1.0f) / 32;
			pz = (int) (room->pos[2] + base->ref->mat[3][2] + 1.0f) / 32;
			for (x = 0; x < room->size[0]; x++)
			{
				for (z = 0; z < room->size[1]; z++)
				{
					F32 *heights = room->blocks[z * room->size[0] + x]->height;
					if (heights[0] < heights[1])
					{
						checkMinMax(px + x, pz + z);
					}
				}
			}
			for (k = eaSize(&room->doors) - 1; k >= 0; k--)
			{
				door = room->doors[k];
				checkMinMax(px + door->pos[0], pz + door->pos[1]);
			}
		}
	}
	g_sgRaidMapData.minx = minx;
	g_sgRaidMapData.minz = minz;
	g_sgRaidMapData.maxx = maxx;
	g_sgRaidMapData.maxz = maxz;
	for (i = eaSize(&g_raidbases) - 1; i >= 0; i--)
	{
		base = g_raidbases[i];
		for (j = eaSize(&base->rooms) - 1; j >= 0; j--)
		{
			room = base->rooms[j];
			px = (int) (room->pos[0] + base->ref->mat[3][0] + 1.0f) / 32;
			pz = (int) (room->pos[2] + base->ref->mat[3][2] + 1.0f) / 32;
			for (x = 0; x < room->size[0]; x++)
			{
				for (z = 0; z < room->size[1]; z++)
				{
					F32 *heights = room->blocks[z * room->size[0] + x]->height;
					if (heights[0] < heights[1])
					{
						g_sgRaidMapData.mapArray[px + x - minx][pz + z - minz] = i + 1;
					}
				}
			}
			for (k = eaSize(&room->doors) - 1; k >= 0; k--)
			{
				door = room->doors[k];
				g_sgRaidMapData.mapArray[px + door->pos[0] - minx][pz + door->pos[1] - minz] = i + 1;
			}
		}
	}
}

// TODO make this use the raid id
int sgRaidGetTeamNumber(Entity *e)
{
	int raidid;

	assert(e && e->pl);
	// Use Raid ID here
	raidid = e->supergroup_id;
	if (raidid == g_sgRaidSGIDs[0])
	{
		return 0;
	}
	if (raidid == g_sgRaidSGIDs[1])
	{
		return 1;
	}
	assert(0);
	return 0;
}

// Hostility table for mayhem raid.  0 & 1 are players, 2 & 3 are objects,
// group 2 object are friendly to group 0 players, etc.
static int sgRaidMayhemGangTableData[4][4] = 
{
	{ 0, 0, 0, 1 },
	{ 0, 0, 1, 0 },
	{ 0, 1, 0, 1 },
	{ 1, 0, 1, 0 },
};

// This exists so that sgRaidGangTest can use a double indirect int pointer without needing to know the
// inner array size.  This permits an arbitrary sized table, as opposed to locking it at 4 by 4.
static int *sgRaidMayhemGangTable[4] = 
{
	sgRaidMayhemGangTableData[0],
	sgRaidMayhemGangTableData[1],
	sgRaidMayhemGangTableData[2],
	sgRaidMayhemGangTableData[3],
};

static int **sgRaidGangTable = NULL;

static int sgRaidGangTest(int gang1, int gang2)
{
	int index1 = -1;
	int index2 = -1;
	int i;
	
	for (i = 0; i < 4; i++)
	{
		if (gang1 == gangid[i])
		{
			index1 = i;
		}
		if (gang2 == gangid[i])
		{
			index2 = i;
		}
	}
	if (index1 == -1 || index2 == -1 || sgRaidGangTable == NULL)
	{
		return gang1 != gang2;
	}
	return sgRaidGangTable[index1][index2];
}

static void sgRaidMayhemSetRaidIDs()
{
	int blueRaidID = g_sgRaidRaidIDs[0];
	int redRaidID = g_sgRaidRaidIDs[1];
	int diff;

	gangid[2] = blueRaidID;
	gangid[3] = redRaidID;
	diff = blueRaidID - redRaidID;
	if (diff > 1000 || diff < -1000)
	{
		diff = 1;
	}
	gangid[0] = blueRaidID + 2 * diff;
	gangid[1] = redRaidID + 2 * diff;
	maxGangID = 4;
}

int sgRaidGetGangIDByIndex(unsigned int index)
{
	devassert(index < maxGangID);
	return gangid[index];
}

void sgRaidCommonSetup()
{
	if (!g_isBaseRaid)
	{
		setGangTestFunction(NULL);
		return;
	}
	switch (g_baseRaidType)
	{
	default:
		devassertmsg(0, "Invalid script type in sgRaidCommonSetup()");
	xcase RAID_TYPE_IOP:
		;
	xcase RAID_TYPE_FFA:
		;
	xcase RAID_TYPE_CTF:
		;
	xcase RAID_TYPE_MAYHEM:
		sgRaidMayhemSetRaidIDs();
		sgRaidGangTable = sgRaidMayhemGangTable;
		setGangTestFunction(sgRaidGangTest);
	}
}
