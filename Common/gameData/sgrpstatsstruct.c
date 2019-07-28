/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "sgrpstatsstruct.h"
#include "badges.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"

#include "sgrpstatsstruct.h"
#include <stdio.h>
#include "memorypool.h"
#include "container/dbcontainerpack.h"
#include "comm_backend.h"
#include "BadgeStats.h"
#include "badgestats_db.h"

#define MAX_BUF 16384

static LineDesc s_SgrpStatsHeader[] =
{
	{ PACKTYPE_STR_UTF8,	SIZEOF2(stat_SgrpStats,name),	"name",					OFFSET(stat_SgrpStats, name)	},
	{ PACKTYPE_CONREF,		CONTAINER_SUPERGROUPS,			"idSgrp",				OFFSET(stat_SgrpStats, idSgrp)	},
	{ 0 },
};

static StructDesc s_SgrpStats_desc[] =
{
	sizeof(stat_SgrpStats),
	{AT_NOT_ARRAY,{0}},
	s_SgrpStatsHeader
};

MP_DEFINE(stat_SgrpStats);

stat_SgrpStats* stat_SgrpStats_Create( int id, int idSgrp, char *name )
{
	stat_SgrpStats *res = NULL;

	// create the pool, arbitrary number
	MP_CREATE(stat_SgrpStats, 64);
	res = MP_ALLOC( stat_SgrpStats );
	if( verify( res ))
	{
		res->id = id;
		res->idSgrp = idSgrp;
		Strncpyt(res->name, name ? name : "");
	}
	// --------------------
	// finally, return

	return res;
}

void stat_SgrpStats_Destroy(stat_SgrpStats *pStats)
{
	MP_FREE(stat_SgrpStats,pStats);
}

void stat_SgrpStats_Unpack(stat_SgrpStats *pStats, char *buff)
{
	int idx;
	char    table[100],field[200],idxstr[MAX_BUF],*val;

	while(decodeLine(&buff,table,field,&val,&idx,idxstr))
	{
		int i = badgestats_GetIndex(BADGESTATSTYPE_SUPERGROUP,table,field);
		if(i >= 0) // stat table and field
		{
			if(i > 0 && i < BADGE_SG_MAX_STATS)
				pStats->badgeStats.aiStats[i] = atoi(val);
		}
		else
			lineToStruct((char *)pStats,s_SgrpStats_desc,table,field,val,idx);
	}
}

char* stat_SgrpStats_Package(stat_SgrpStats *pStats)
{
	int i;
	StuffBuff sb;
	initStuffBuff( &sb, MAX_BUF );

	// Write out header
	structToLine(&sb,(char *)pStats,0,s_SgrpStats_desc,s_SgrpStatsHeader,0);
	structToLine(&sb,(char *)pStats,0,s_SgrpStats_desc,s_SgrpStatsHeader+1,0);

	for(i=1; i <= g_BadgeStatsSgroup.idxStatMax; i++)
		addStringToStuffBuff(&sb, "%s %d\n", badgestats_ColumnString(BADGESTATSTYPE_SUPERGROUP,i), pStats->badgeStats.aiStats[i]);

	return sb.buff; // caller takes ownership, this is the same behavior as cstorePackage
}

char* stat_SgrpStats_Template() 
{
	StuffBuff sb;
	initStuffBuff( &sb, MAX_BUF );

	structToLineTemplate(&sb,0,&s_SgrpStatsHeader[0],1);
	structToLineTemplate(&sb,0,&s_SgrpStatsHeader[1],1);
	badgestats_Subtable(BADGESTATSTYPE_SUPERGROUP,&sb);

	return sb.buff;
}
