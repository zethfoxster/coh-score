/***************************************************************************
*     Copyright (c) 2003-2005, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
*
* Module Description:
*    Manages a data pool - a block of data with variable length entries, such as strings
*    (but handles nulls in the data) or headers of files.
*    Used heavily by the Pig system.
***************************************************************************/
#pragma once
#ifndef _DATAPOOL_H
#define _DATAPOOL_H

#include "stdtypes.h"
#include "file.h"

typedef struct StashTableImp* StashTable;

typedef struct DataPool {
	U32 header_flag;
	S32 num_elements;
	U32 total_data_size;
	U8 *data;
	// The following are created on the fly at load time, not while assembling one in memory
	U8 **data_list; // lookup list of pointers to the data
	U32 *size_list; // list of sizes
	StashTable reverse_lookup;
} DataPool;

void DataPoolCreate(DataPool *handle); // Creates an empty data pool
void DataPoolDestroy(DataPool *handle);
S32 DataPoolRead(FILE *f, DataPool *handle); // reads a data pool, and returns the number of bytes read, -1 on fail
S32 DataPoolLoadNamedShared(FILE *f, DataPool *handle, const char *dataPoolName); // Loads a named data pool from shared memory, or loads it from file if it doesn't exist.  Advances the file* regardless

U32 DataPoolGetDiskUsage(DataPool *handle);
U32 DataPoolGetMemoryUsage(DataPool *handle); // Excludes the hashtable (created dynamically)

const char *DataPoolGetString(DataPool *handle, S32 id);
const void *DataPoolGetData(DataPool *handle, S32 id, U32 *size);
S32 DataPoolGetIDFromString(DataPool *handle, const char *s);

// Only used in creating/writing DataPools:
S32 DataPoolWrite(FILE *f, DataPool *handle); // writes a data pool, and returns the number of bytes written, -1 on fail
U32 DataPoolAdd(DataPool *handle, const void *data, U32 size); // returns the index of the newly added data
int DataPoolAssembleLists(DataPool *handle); // Assembles the lists (size and data) by parsing the data block // returns 0 on success


#endif