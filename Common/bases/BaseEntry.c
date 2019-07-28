/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "BaseEntry.h"
#include "SgrpBasePermissions.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"
#include "supergroup.h"

BaseAccess sgrp_BaseAccessFromSgrp(Supergroup *sg, SgrpBaseEntryPermission bep)
{
	BaseAccess res = kBaseAccess_None;
	if (sg)
	{
		if( sg->entryPermission & (1<<bep) )
		{
			res = kBaseAccess_Allowed;
		}
		else
		{
			res = kBaseAccess_PermissionDenied;
		} 
	}
	return res;
}

char *baseaccess_ToStr(BaseAccess s)
{
	char *strs[] = {
		"kBaseAccess_None",
		"kBaseAccess_Allowed",
		"kBaseAccess_PermissionDenied",
		"kBaseAccess_RentOwed",
		"kBaseAccess_RaidScheduled",
		"kBaseAccess_Count",
	};
	return AINRANGE( s, strs ) ? strs[s] : "invalid enum";
}
