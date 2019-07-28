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
#ifndef ASYNCFILEWRITER_H
#define ASYNCFILEWRITER_H

#include "stdtypes.h"
#include "winutil.h"

typedef struct GenericHashTableImp *GenericHashTable;
typedef struct HashTableImp *HashTable;

#define ASYNCFILEWRITER_MAX_WRITE (1<<24) // 32MB is kernel limit, so play safe

#pragma warning (push, 1) // C++ compilation
typedef struct AsyncFileWriter
{
	char *buf;     // estring. accumulate data to write here. not written until 'Write' call made

	// info
	bool writing;
	DWORD last_write;
	DWORD total_written;

	// internals
	HANDLE hfile;
	char const * const output; // never touch this 
	OVERLAPPED ol;
} AsyncFileWriter;
#pragma warning (pop)

AsyncFileWriter* AsyncFileWriter_Create(char *fname,bool concat);
void AsyncFileWriter_Destroy( AsyncFileWriter *hItem );
void AsyncFileWriter_Update(AsyncFileWriter *fw, bool until_done);

// only writes what is currently in the buffer. does not append further additions to it.
bool AsyncFileWriter_Write(AsyncFileWriter *fw); 
void AsyncFileWriter_Stream(AsyncFileWriter *fw, bool until_done);

#endif //ASYNCFILEWRITER_H
