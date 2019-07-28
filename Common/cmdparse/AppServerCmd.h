/***************************************************************************
 *     Copyright (c) 2006-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: $
 * $Revision: 1.36 $
 * $Date: 2006/02/07 06:13:59 $
 *
 * Module Description:
 *
 * Revision History:
 *
 ***************************************************************************/
#ifndef APPSERVERCMD_H
#define APPSERVERCMD_H

#include "stdtypes.h"

typedef struct GenericHashTableImp *GenericHashTable;
typedef struct HashTableImp *HashTable;

#include "stdtypes.h"

typedef struct AppServerCmdState
{
	int idEnt;
	int idSgrp;
	char strs[2][9000];		// Less than CMDMAXLENGTH, which is 10000
	int ints[8];
} AppServerCmdState;
extern AppServerCmdState g_appserver_cmd_state;

#endif //APPSERVERCMD_H
