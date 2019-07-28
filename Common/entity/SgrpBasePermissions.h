/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SGRPBASEPERMISSIONS_H
#define SGRPBASEPERMISSIONS_H

#include "stdtypes.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef enum SgrpBaseEntryPermission
{
	kSgrpBaseEntryPermission_None,
// 	kSgrpBaseEntryPermission_All,
	kSgrpBaseEntryPermission_Coalition,
	kSgrpBaseEntryPermission_LeaderTeammates,

	// last
	kSgrpBaseEntryPermission_Count,
} SgrpBaseEntryPermission;

bool sgrpbaseentrypermission_Valid( SgrpBaseEntryPermission e );
char *sgrpbaseentrypermission_ToMenuMsg(SgrpBaseEntryPermission e );


#endif //SGRPBASEPERMISSIONS_H
