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
#ifndef XACTSERVERINTERNAL_H
#define XACTSERVERINTERNAL_H

#include "stdtypes.h"

typedef struct GenericHashTableImp *GenericHashTable;
typedef struct HashTableImp *HashTable;

// @todo -AB: get rid of this :11/20/06
typedef struct XactServer
{
	bool in_recovery;
} XactServer;
extern XactServer g_XactServer;

#endif //XACTSERVERINTERNAL_H
