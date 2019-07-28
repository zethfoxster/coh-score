#include "datalist.h"
#include "earray.h"
#include "assert.h"
#include "StringCache.h"
#include "endian.h"
#include "textparser.h"

typedef enum DataListJournalOp {
	DLJ_INVALID,
	DLJ_ADD_OR_UPDATE, // Must implicitly FREE
	DLJ_FREE,
} DataListJournalOp;

typedef struct DataListHeader {
	U32 version;
	U32 num_entries;
} DataListHeader;
TokenizerParseInfo parseDataListHeader[] = {
	{"version",		TOK_INT(DataListHeader, version,0)},
	{"num_entries", TOK_INT(DataListHeader, num_entries,0)},
	{0,0}
};

void DataListCreate(DataList *handle)
{
	ZeroStruct(handle);
}

void DataListDestroy(DataList *handle)
{
	int i;
	for (i=0; i<eaSize(&handle->data_list); i++) {
		if (!(handle->flags[i] & DLF_DO_NOT_FREEME)) {
			SAFE_FREE(handle->data_list_not_const[i]);
		}
	}
	eaDestroy(&handle->data_list_not_const);
	eaiDestroy(&handle->size_list);
	eaiDestroy(&handle->free_list);
	eaiDestroy(&handle->flags);
	ZeroStruct(handle);
}

// Adds to a specific place (used internally in journal recovery)
static S32 DataListAddSpecific(DataList *handle, S32 index, const void *data, U32 size, DataListJournal *journal) // returns the index of the newly added data, -1 on failure
{
	U8 *localdata;
	assert(size != 0);
	assert(data != 0);
	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));
	assert(!journal);
	if (index<0)
		return -1;
	localdata = malloc(size);
	memcpy(localdata, data, size);
	while (index >= eaiSize(&handle->size_list)) {
		S32 newindex = eaiSize(&handle->size_list);
		eaiPush(&handle->size_list, 0); // Add new entry
		eaiPush(&handle->flags, 0); // Add new entry
		eaPush(&handle->data_list_not_const, NULL); // Add new entry
		eaiPush(&handle->free_list, newindex); // Add to FreeList
	}
	// Free existing data if there is any
	if (DataListGetData(handle, index, NULL)) {
		DataListFree(handle, index, NULL);
	}
	// Remove index from freelist
	eaiFindAndRemove(&handle->free_list, index);

	handle->size_list[index] = size;
	handle->data_list[index] = localdata;
	handle->flags[index] = 0;
	handle->total_data_size += size + sizeof(U32);

	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));
	return index;
}

S32 DataListAdd(DataList *handle, const void *data, U32 size, bool is_string, DataListJournal *journal) // returns the index of the newly added data, -1 on failure
{
	S32 index;
	const U8 *localdata;
	assert(size != 0);
	assert(data != 0);
	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));
	if (is_string) {
		localdata = allocAddString(data);
	} else {
		U8 *temp = malloc(size);
		memcpy(temp, data, size);
		localdata = temp;
	}
	if (eaiSize(&handle->free_list)) {
		index = eaiPop(&handle->free_list);
	} else {
		index = eaiSize(&handle->size_list);
		eaiPush(&handle->size_list, 0); // Add new entry
		eaPush(&handle->data_list_not_const, NULL); // Add new entry
		eaiPush(&handle->flags, 0); // Add new entry
	}
	handle->size_list[index] = size;
	handle->data_list[index] = localdata;
	handle->flags[index] = is_string?DLF_DO_NOT_FREEME:0;
	handle->total_data_size += size + sizeof(U32);
	if (journal) {
		U32 offs=journal->size;
		journal->size += sizeof(U8) + sizeof(S32) + sizeof(U32) + size;
		journal->data = realloc(journal->data, journal->size);
		*(U8*)(journal->data + offs) = DLJ_ADD_OR_UPDATE;
		offs += sizeof(U8);
		*(S32*)(journal->data + offs) = endianSwapIfBig(S32, index);
		offs += sizeof(S32);
		*(U32*)(journal->data + offs) = endianSwapIfBig(U32, size);
		offs += sizeof(U32);
		memcpy(journal->data + offs, data, size);
		offs += size;
		assert(journal->size == offs);
	}

	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));
	return index;
}

S32 DataListUpdate(DataList *handle, S32 oldid, const void *data, U32 size, bool is_string, DataListJournal *journal) // returns the index of the updated data, -1 on failure
{
	S32 newid;
	assert(size != 0);
	assert(data != 0);
	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));
	if (DataListGetData(handle, oldid, NULL)) {
		// Existing data, free it!
		DataListFree(handle, oldid, NULL);
		newid = DataListAdd(handle, data, size, is_string, journal);
		assert(newid == oldid);
	} else {
		// Really just an Add
		newid = DataListAdd(handle, data, size, is_string, journal);
	}
	return newid;
}

void DataListFree(DataList *handle, S32 id, DataListJournal *journal)
{
	if (id < 0 || id >= eaSize(&handle->data_list))
		return;
	if (handle->data_list[id]==NULL) {
		// Already freed
		return;
	}
	handle->total_data_size -= handle->size_list[id] + sizeof(U32);
	if (handle->flags[id] & DLF_DO_NOT_FREEME) {
		handle->data_list[id] = NULL;
	} else {
		SAFE_FREE(handle->data_list_not_const[id]);
	}
	handle->size_list[id] = 0;
	handle->flags[id] = 0;
	eaiPush(&handle->free_list, id);
	if (journal) {
		U32 offs=journal->size;
		journal->size += sizeof(U8) + sizeof(S32);
		journal->data = realloc(journal->data, journal->size);
		*(U8*)(journal->data + offs) = DLJ_FREE;
		offs += sizeof(U8);
		*(S32*)(journal->data + offs) = endianSwapIfBig(S32, id);
		offs += sizeof(S32);
		assert(journal->size == offs);
	}
}

int DataListApplyJournal(DataList *handle, DataListJournal *journal)
{
	U8 *cursor = journal->data;
	U32 size = journal->size;
	while (size>0) {
#define GET(type) endianSwapIfBig(type, (*(type*)cursor)); cursor += sizeof(type); size -= sizeof(type); if (size<0) break;
		DataListJournalOp op;
		S32 id;
		U32 datasize;
		op = GET(U8);
		switch(op) {
		xcase DLJ_FREE:
			id = GET(S32);
			DataListFree(handle, id, NULL);
		xcase DLJ_ADD_OR_UPDATE:
			id = GET(S32);
			datasize = GET(U32);
			if (datasize > size)
				break;
			size -= datasize;
			DataListAddSpecific(handle, id, cursor, datasize, NULL);
			cursor += datasize;
		}
	}
	if (size != 0) {
		// Corrupt!
		return 1;
	}
	return 0;
}



const void *DataListGetData(DataList *handle, S32 id, U32 *size)
{
	if (id < 0 || id >= eaSize(&handle->data_list))
		return NULL;
	if (size)
		*size = handle->size_list[id];
	return handle->data_list[id];
}

const char *DataListGetString(DataList *handle, S32 id)
{
	if (id < 0 || id >= eaSize(&handle->data_list))
		return NULL;
	if (!(handle->flags[id] & DLF_DO_NOT_FREEME))
	{
		// Add this string to the string table so that it is valid for the life of the program
		char *olddata = handle->data_list_not_const[id];
		if (!olddata)
			return NULL;
		handle->data_list[id] = allocAddString(olddata);
		free(olddata);
		handle->flags[id] |= DLF_DO_NOT_FREEME;
	}
	return handle->data_list[id];
}

S32 DataListGetIDFromString(DataList *handle, const char *s) // Slow!  Don't use!
{
	S32 i;
	const char *s2;
	for (i=eaSize(&handle->data_list)-1; i>=0; i--)
	{
		if (!(s2 = handle->data_list[i]))
			continue;
		if (stricmp(s2, s)==0)
			return i;
	}
	return -1;
}

U32 DataListGetNumEntries(DataList *handle)
{
	return eaSize(&handle->data_list) - eaiSize(&handle->free_list);
}

U32 DataListGetNumTotalEntries(DataList *handle)
{
	return eaSize(&handle->data_list);
}

U32 DataListGetDiskUsage(DataList *handle)
{
	return handle->total_data_size + sizeof(DataListHeader) + sizeof(U32)*eaiSize(&handle->free_list);
}

U8 *DataListWrite(DataList *handle, U32 *filesize) // compatcs a data list, and returns the number of bytes and the data to be written
{
	U32 size = DataListGetDiskUsage(handle);
	U8 *data = malloc(size);
	U8 *cursor = data;
	DataListHeader header = {0};
	int i;
	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));

	*filesize = size;
	header.version = 0;
	header.num_entries = eaSize(&handle->data_list);
	memcpy(cursor, &header, sizeof(header));
	if (isBigEndian())
		endianSwapStruct(parseDataListHeader, cursor);
	cursor += sizeof(header);
	for (i=0; i<eaSize(&handle->data_list); i++) {
		U32 datasize = handle->size_list[i];
		if (isBigEndian())
			*(U32*)cursor = endianSwapU32(datasize);
		else
			*(U32*)cursor = datasize;
		cursor += sizeof(U32);
		if (datasize) {
			memcpy(cursor, handle->data_list[i], datasize);
			cursor += datasize;
			// Must not be in the free list
			assert(eaiFind(&handle->free_list, i)==-1);
		} else {
			// Must be in the free list!
			assert(eaiFind(&handle->free_list, i)!=-1);
		}
	}
	assert(cursor == data + size);
	return data;
}

S32 DataListRead(DataList *handle, char *filedata, int filesize) // reads a data list, and returns the number of bytes read, -1 on fail
{
	DataListHeader header = {0};
	U8 * cursor = filedata;
	int offset=0;
	U32 i;
	assert(eaSize(&handle->data_list)==0);
	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));
	handle->total_data_size = 0;
#define SAFE_COPY(ptr, size) {					\
		if (offset + (int)(size) > filesize) {	\
			DataListDestroy(handle);			\
			return -1;							\
		}										\
		memcpy((ptr), cursor, (size));			\
		cursor += size;							\
		offset += size;							\
	}	// End SAFE_COPY

	SAFE_COPY(&header, sizeof(header));
	if (isBigEndian())
		endianSwapStruct(parseDataListHeader, &header);

	if (header.version != 0)
		return -1;

	for (i=0; i<header.num_entries; i++) {
		U32 datasize;
		U8 *data = NULL;
		SAFE_COPY(&datasize, sizeof(U32));
		if (isBigEndian())
			datasize = endianSwapU32(datasize);
		eaiPush(&handle->size_list, datasize);
		eaiPush(&handle->flags, 0);
		if (datasize) {
			data = malloc(datasize);
			SAFE_COPY(data, datasize);
			handle->total_data_size += sizeof(U32) + datasize;
		} else {
			// Free spot
			eaiPush(&handle->free_list, i);
		}
		eaPush(&handle->data_list_not_const, data);
	}
#undef SAFE_COPY
	assert(eaiSize(&handle->size_list) == eaSize(&handle->data_list) &&
		eaiSize(&handle->size_list) == eaiSize(&handle->flags));
	return filesize;
}
