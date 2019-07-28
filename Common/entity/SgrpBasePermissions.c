/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "SgrpBasePermissions.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"


bool sgrpbaseentrypermission_Valid( int perm )
{
	int max = (1<<kSgrpBaseEntryPermission_Count) - 1;
	return (perm >= 0 && perm <= max);
}

//----------------------------------------
//  Get the menu message for the base entry permissions
//----------------------------------------
char *sgrpbaseentrypermission_ToMenuMsg(SgrpBaseEntryPermission e )
{
	static char *s_strs[] = 
		{
			"SgBaseEntryPermissionNone",
			"SgBaseEntryPermissionCoalition",
			"SgBaseEntryPermissionLeaderTeammates",
		};
	STATIC_INFUNC_ASSERT( ARRAY_SIZE( s_strs ) == kSgrpBaseEntryPermission_Count );
	return AINRANGE( e, s_strs ) ? s_strs[e] : NULL;
}
