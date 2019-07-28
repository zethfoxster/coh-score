#include "MRUList.h"
#include "RegistryReader.h"
#include "stdtypes.h"
#include "mathutil.h"
#include <string.h>
#include <stdio.h>
#include "utils.h"

#define MAX_MRU_STRING 1024

static const char *regGetString(MRUList *mru, const char *key, const char *deflt) {
	static char buf[MAX_MRU_STRING];
	strcpy(buf, deflt);
	rrReadString(mru->rr, key, buf, ARRAY_SIZE(buf));
	return buf;
}

MRUList *createMRUList(const char *regKey, const char *name, int maxEntries, int maxStringSize)
{
	int i;
	MRUList *mru;
	char key[256];
	mru = calloc(sizeof(MRUList),1);
	mru->maxStringSize = MIN(MAX_MRU_STRING, maxStringSize);
	mru->count = 0;
	mru->maxCount = maxEntries;
	mru->name = strdup(name);
	mru->values = calloc(sizeof(char*)*maxEntries, 1);
	for (i=0; i<mru->maxCount; i++) {
		mru->values[i] = calloc(mru->maxStringSize,1);
	}
	sprintf_s(SAFESTR(key), "HKEY_CURRENT_USER\\SOFTWARE\\Cryptic\\%s", regKey);
	mru->rr = createRegReader();
	initRegReader(mru->rr, key);
	mruUpdate(mru);
	return mru;
}

void destroyMRUList(MRUList *mru)
{
	int i;
	SAFE_FREE(mru->name);
	for (i=0; i<mru->maxCount; i++) {
		SAFE_FREE(mru->values[i]);
	}
	SAFE_FREE(mru->values);
	destroyRegReader(mru->rr);
	SAFE_FREE(mru);
}

void mruUpdate(MRUList *mru)
{
	char key[256];
	char *badvalue="BadValue";
	const char *value;
	int i;
	mru->count = 0;
	for(i=0; i < mru->maxCount; i++) {
		sprintf_s(SAFESTR(key), "%s%d", mru->name, i);
		value = regGetString(mru, key, badvalue);
		if (strcmp(value, badvalue) != 0)
			strncpyt(mru->values[mru->count++], value, mru->maxStringSize);
	}
}

void mruSaveList(MRUList *mru)
{
	char key[256];
	int i;
	for (i=0; i<mru->count; i++) {
		sprintf_s(SAFESTR(key), "%s%d", mru->name, i);
		rrWriteString(mru->rr, key, mru->values[i]);
	}
}

void mruAddToList(MRUList *mru, char *_newEntry)
{
	int i;
	char key[256];
	char *newEntry = _alloca(mru->maxStringSize);
	strcpy_s(newEntry, mru->maxStringSize, _newEntry);

	mruUpdate(mru);

	for (i=0; i<mru->count; i++) {
		if (stricmp(mru->values[i], newEntry)==0) {
			// Move to end
			for (; i<mru->count-1; i++) {
				strcpy_s(mru->values[i], mru->maxStringSize, mru->values[i+1]);
			}
			strcpy_s(mru->values[mru->count-1], mru->maxStringSize, newEntry);
			mruSaveList(mru);
			return;
		}
	}

	if(mru->count >= mru->maxCount) {
		// Delete oldest
		for (i=1; i<mru->maxCount; i++) 
			strcpy_s(mru->values[i-1], mru->maxStringSize, mru->values[i]);
		mru->count--;
		mruSaveList(mru);
	}

	sprintf_s(SAFESTR(key), "%s%d", mru->name, mru->count);
	rrWriteString(mru->rr, key, newEntry);

	strcpy_s(mru->values[mru->count++], mru->maxStringSize, newEntry);
}
