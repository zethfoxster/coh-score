/***************************************************************************
*     Copyright (c) 2003-2006, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
*
* Module Description:
*    Manages a data pool - a block of data with variable length entries, such as strings
*    (but handles nulls in the data) or headers of files.
*    Used heavily by the Pig system.
*
***************************************************************************/
#include "datapool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stdtypes.h"
#include "SharedMemory.h"
#include "assert.h"
#include "StashTable.h"
#include <limits.h>


static int print_datapool_messages=0;

void DataPoolCreate(DataPool *handle) { // Creates an empty data pool
	memset(handle, 0, sizeof(DataPool));
}

void DataPoolDestroy(DataPool *handle) {
	SM_SAFE_FREE(handle->data);
	SM_SAFE_FREE(handle->data_list);
	SM_SAFE_FREE(handle->size_list);
	handle->num_elements = 0;
	if (handle->reverse_lookup) {
		stashTableDestroy(handle->reverse_lookup);
		handle->reverse_lookup = 0;
	}
}

S32 DataPoolReadInternal(FILE *f, DataPool *handle) // reads a data pool, and returns the number of bytes read
{
	S32 ret=0;

	SM_SAFE_FREE(handle->data);

	ret+=fread(&handle->header_flag, 1, sizeof(handle->header_flag), f);
	ret+=fread(&handle->num_elements, 1, sizeof(handle->num_elements), f);
	ret+=fread(&handle->total_data_size, 1, sizeof(handle->total_data_size), f);
	handle->data = malloc(handle->total_data_size);
	ret+=fread(handle->data, 1, handle->total_data_size, f);

	return ret;
}

S32 DataPoolRead(FILE *f, DataPool *handle)
{
	S32 ret=0;

	ret += DataPoolReadInternal(f, handle);

	SM_SAFE_FREE(handle->data_list);
	SM_SAFE_FREE(handle->size_list);

	if (0!=DataPoolAssembleLists(handle)) {
		return -1;
	}

	return ret;
}

U32 DataPoolGetDiskUsage(DataPool *handle) {
	return sizeof(U32)*3 + handle->total_data_size;
}

U32 DataPoolGetMemoryUsage(DataPool *handle) {
	return sizeof(DataPool) + handle->total_data_size + (sizeof(U8*) + sizeof(U32*))*handle->num_elements;
}

S32 DataPoolLoadNamedShared(FILE *f, DataPool *handle, const char *dataPoolName) // Loads a named data pool from shared memory, or loads it from file if it doesn't exist.  Advances the file* regardless
{
	S32 ret=0;
	SM_AcquireResult smret;
	SharedMemoryHandle *shared_memory=NULL;

	SM_SAFE_FREE(handle->data);

	smret = sharedMemoryAcquire(&shared_memory, dataPoolName);
	if (smret==SMAR_Error) {
		// Fata error, pass through to default reader
		sharedMemoryDestroy(shared_memory);
		return DataPoolRead(f, handle);
	} else if (smret==SMAR_DataAcquired) {
		if (print_datapool_messages)
			printf("Acquired data pool %s from shared memory\n", dataPoolName);
		// Data is already there!  Yay!
		// just assign local pointers by copying from the struct in memory
		*handle = *(DataPool*)(sharedMemoryGetDataPtr(shared_memory));
		handle->reverse_lookup = 0; // Hashtables are created locally (for now?)

		// Find number of bytes read
		ret = DataPoolGetDiskUsage(handle);

		// Advance the file pointer
		fseek(f, ret, SEEK_CUR);

	} else if (smret==SMAR_FirstCaller) {
		U8 *data;
		DataPool *header;

		if (print_datapool_messages)
			printf("Data pool %s not in shared memory, creating...\n", dataPoolName);
		// Read the data from the file and advance the pointer
		ret = DataPoolReadInternal(f, handle);
		// Allocate a shared memory block
		data = sharedMemorySetSize(shared_memory, DataPoolGetMemoryUsage(handle));
		if (data==NULL) {
			// error, just return the data
			sharedMemoryDestroy(shared_memory);
			SM_SAFE_FREE(handle->data_list);
			SM_SAFE_FREE(handle->size_list);
			if (0!=DataPoolAssembleLists(handle))
				ret = -1;
			return ret;
		}
		header = (DataPool*)sharedMemoryAlloc(shared_memory, sizeof(DataPool));
		// Copy over basic elements
		header->header_flag = handle->header_flag;
		header->num_elements = handle->num_elements;
		header->total_data_size = handle->total_data_size;
		// Copy data block
		header->data = sharedMemoryAlloc(shared_memory, header->total_data_size);
		memcpy(header->data, handle->data, header->total_data_size);
		// Make new lookup tables
		header->data_list = sharedMemoryAlloc(shared_memory, sizeof(U8*)*header->num_elements);
		header->size_list = sharedMemoryAlloc(shared_memory, sizeof(U32*)*header->num_elements);
		if (0!=DataPoolAssembleLists(header))
			ret = -1;
		// Now, copy the stuff from the static one to the local one
		// But first, free the existing data
		SM_SAFE_FREE(handle->data);
		*handle = *header;
		// Unlock the memory chunk since it's now filled in!
		sharedMemoryUnlock(shared_memory);
	}
	return ret;
}



U32 DataPoolAdd(DataPool *handle, const void *data, U32 size) { // returns the index of the newly added data
	handle->data = realloc(handle->data, handle->total_data_size + size + sizeof(U32));
	memcpy(handle->data + handle->total_data_size, &size, sizeof(size));
	handle->total_data_size += sizeof(size);
	memcpy(handle->data + handle->total_data_size, data, size);
	handle->total_data_size += size;
	return handle->num_elements++;
}

int DataPoolAssembleLists(DataPool *handle) { // Assembles the lists (size and data) by parsing the data block
	int i;
	U8* p;
	S32 s;

	if (!handle->data_list)
		handle->data_list = malloc(sizeof(void *) * handle->num_elements);
	if (!handle->size_list)
		handle->size_list = malloc(sizeof(void *) * handle->num_elements);
	for (i=0, p=handle->data; i<handle->num_elements; i++) {
		if (p >= handle->data + handle->total_data_size ||
			p < handle->data)
		{
			// overflow!
			return -1;
		}
		s = *(S32*)p;
		handle->size_list[i] = s;
		p+=sizeof(S32);
		handle->data_list[i] = p;
		p+=s;
	}
	return 0;
}

S32 DataPoolWrite(FILE *f, DataPool *handle) { // writes a data pool, and returns the number of bytes written, -1 on fail
	S32 ret=0;

	ret+=fwrite(&handle->header_flag, 1, sizeof(handle->header_flag), f);
	ret+=fwrite(&handle->num_elements, 1, sizeof(handle->num_elements), f);
	ret+=fwrite(&handle->total_data_size, 1, sizeof(handle->total_data_size), f);
	ret+=fwrite(handle->data, 1, handle->total_data_size, f);

	if (ret!=(S32)DataPoolGetDiskUsage(handle)) {
		printf("Error while writing!  Wrote %d bytes, expected %d!\n", ret, DataPoolGetDiskUsage(handle));
		return -1;
	} else {
		return ret;
	}
}

const char *DataPoolGetString(DataPool *handle, S32 id) {
	if (id<0) return NULL;
	return (const char*)handle->data_list[id];
}

const void *DataPoolGetData(DataPool *handle, S32 id, U32 *size) {
	if (id<0) {
		*size=0;
		return NULL;
	}
	*size = handle->size_list[id];
	return handle->data_list[id];
}

S32 DataPoolGetIDFromString(DataPool *handle, const char *s) {
	S32 i;
	if (handle->reverse_lookup) {
		uintptr_t ret = (uintptr_t)stashFindPointerReturnPointer(handle->reverse_lookup, s);
		assert(ret <= INT_MAX);
		i = (S32)ret;
		return i-1;
	}
	assert(handle->num_elements);

	// Genereate reverse_lookup
	handle->reverse_lookup = stashTableCreateWithStringKeys(handle->num_elements, StashDefault );
	for (i=0; i<handle->num_elements; i++) {
		stashAddInt(handle->reverse_lookup, handle->data_list[i], i+1, false);
	}
	return DataPoolGetIDFromString(handle, s);
}

