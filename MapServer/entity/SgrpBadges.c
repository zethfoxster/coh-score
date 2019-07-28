/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "SgrpBadges.h"
#include "error.h"
#include "utils.h"
#include "assert.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"
#include "eval.h"
#include "Supergroup.h"
#include "SgrpStats.h"
#include "BadgeStats.h"
#include "Bitfield.h"
#include "logcomm.h"
#include "badges_load.h"





/**********************************************************************func*
 * s_AddSafe
 * small helper specifically for badge_StatAdd
 */
static void s_AddSafe(int *v, int dv)
{
	if( verify( v ))
	{
		int i = *v + dv;
		if(i>=0)
		{
			*v = i;
		}
		else
		{
			// Overflow, just clamp to max
			*v = 0x7fffffff;
		}
	}
}


//------------------------------------------------------------
//  sgrp_BadgeStatAdd
//   adds a stat to a supergroup without locking or updating other
//  servers.
//----------------------------------------------------------
bool sgrp_BadgeStatAdd(Supergroup *sg, const char *pchStat, int iAdd)
{
	bool res = false;
	int idx;
	if( verify( sg && sg->pBadgeStats && pchStat ) 
		&& stashFindInt(g_BadgeStatsSgroup.idxStatFromName, pchStat, &idx))
	{	
		if(verify(AINRANGE( idx, sg->pBadgeStats->aiStats )))
		{	
			s_AddSafe( &sg->pBadgeStats->aiStats[idx], iAdd );		
			MarkModifiedBadges( g_BadgeStatsSgroup.aIdxBadgesFromName, sg->badgeStates.eaiStates, pchStat);
			
			// --------------------
			// finally
			
			res = true;
		}
		else
		{
			dbLog("SgrpBadges:BadgeStatAdd",NULL,"stat named '%s' has invalid index '%d'", pchStat, idx);
		}		
	}		

	return res;
}




void sgrpbadge_ServerInit(void)
{
	static bool bInitDone = false;
	char *filename = NULL;
	if(bInitDone)
		return;
	
	filename = "server/db/templates/supergroup_badgestats.attribute";
	g_BadgeStatsSgroup.idxStatFromName = stashTableCreateWithStringKeys(128, StashDeepCopyKeys);
	g_BadgeStatsSgroup.statnameFromId = stashTableCreateInt(128);
	
	g_BadgeStatsSgroup.idxStatMax = LoadBadgeStatNames(filename, "server/db/templates/supergroup_badgestats.attribute", g_BadgeStatsSgroup.idxStatFromName, g_BadgeStatsSgroup.statnameFromId );
	
	if(g_BadgeStatsSgroup.idxStatMax >= BADGE_SG_MAX_STATS)
	{
		g_BadgeStatsSgroup.idxStatMax = BADGE_SG_MAX_STATS-1;
		ErrorFilenamef( filename, "Too many badge stats used!");
	}
	g_BadgeStatsSgroup.aIdxBadgesFromName = LoadBadgeStatUsage(g_BadgeStatsSgroup.aIdxBadgesFromName, &g_SgroupBadges, "defs/supergroup_badges.def" );

	//sgrpbadges_EvalInit();
}


// bool sgrpbadge_GetIdxFromName(char *pch, int *pi)
// {
// 	BadgeDef *pdef;
// 	if(stashFindInt(g_SgroupBadges.hashByName, pch, (int *)&pdef))
// 	{
// 		if( pi )
// 		{
// 			*pi = pdef->iIdx;
// 		}
// 		return true;
// 	}

// 	return false;
// }

//------------------------------------------------------------
//  fetch stat value for given stat
//----------------------------------------------------------
int sgrp_BadgeStatGet(Supergroup *sg, const char *pchStat)
{
	int res = 0;
	int idx;
	if(verify( sg && sg->pBadgeStats && sg->pBadgeStats->aiStats ))
	{
		if( !stashFindInt( g_BadgeStatsSgroup.idxStatFromName, pchStat, &idx))
		{
			ErrorFilenamef("Defs/supergroup_badges.def","unknown stat %s", pchStat);
		}
		else if( idx > g_BadgeStatsSgroup.idxStatMax || !AINRANGE( idx, sg->pBadgeStats->aiStats ) )
		{
			ErrorFilenamef("Defs/supergroup_badges.def","invalid index %d for stat %s", idx, pchStat);
		}
		else
		{
			res = sg->pBadgeStats->aiStats[idx];
		}
	}

	// --------------------
	// finally

	return res;
}

//----------------------------------------
// keep track of the state of the sgrp
// badges. the id may be either an entid (mapserver) or dbid (statserver).
//----------------------------------------
U32 *sgrpbadge_eaiStatesFromId(int id)
{
	static StashTable eaBadgeStatesFromId = NULL; // to persist the badge states
	U32 *res = NULL;
		
	if( !eaBadgeStatesFromId )
	{
		eaBadgeStatesFromId = stashTableCreateInt(128);
	}

	
	if( !stashIntFindPointer(eaBadgeStatesFromId, id, &res) )
	{
		eaiCreate( &res );
		eaiSetSize( &res, BADGE_SG_MAX_STATS );
		stashIntAddPointer(eaBadgeStatesFromId, id, res, false);
	}
	
	// --------------------
	// finally
	
	return res;
}
