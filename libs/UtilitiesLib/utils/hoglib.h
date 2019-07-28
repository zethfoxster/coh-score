/***************************************************************************
*     Copyright (c) 2005-2006, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
*/

#include "stdtypes.h"

#define HOG_VERSION 10 // start Hog at 10 incase pig needs to increment
#define HOG_INVALID_INDEX ((U32)-1)
#define HOG_HEADER_FLAG 0xDEADF00D // arbitrary

typedef struct NewPigEntry NewPigEntry;
typedef struct HogFile HogFile;
typedef U32 HogFileIndex;

int hogCreateFromMem(NewPigEntry *entries, int count, const char *fname);

void hogFileDumpInfo(HogFile *handle, int verbose, int debug_verbose);
HogFile *hogFileAlloc(void);
void hogFileCreate(HogFile *handle);
void hogFileDestroy(HogFile *handle);
HogFile *hogFileReadOrCreate(const char *filename, bool *bCreated); // Load a .HOGG file or creates a new empty one if it does not exist
int hogFileRead(HogFile *handle, const char *filename); // Load a .HOGG file
HogFileIndex hogFileFind(HogFile *handle, const char *relpath); // Find an entry in a Hog

typedef void (*HogFileScanProcessor)(HogFile *handle, const char* filename);
void hogScanAllFiles(HogFile *handle, HogFileScanProcessor processor);

typedef void (*HogCallbackDeleted)(void *userData, const char* path);
typedef void (*HogCallbackNew)(void *userData, const char* path, U32 filesize, U32 timestamp);
typedef void (*HogCallbackUpdated)(void *userData, const char* path, U32 filesize, U32 timestamp);
void hogFileSetCallbacks(HogFile *handle, void *userData,
			 HogCallbackDeleted fileDeleted, HogCallbackNew fileNew, HogCallbackUpdated fileUpdated);

void hogFileLock(HogFile *handle); // Acquires the multi-process mutex
void hogFileUnlock(HogFile *handle);
void hogFileCheckForModifications(HogFile *handle);

const char *hogFileGetArchiveFileName(HogFile *handle);
U32 hogFileGetNumFiles(HogFile *handle);
const char *hogFileGetFileName(HogFile *handle, int index); // Returns NULL if an empty slot
U32 hogFileGetFileTimestamp(HogFile *handle, int index);
U32 hogFileGetFileChecksum(HogFile *handle, int index);
U32 hogFileGetFileSize(HogFile *handle, int index);
bool hogFileIsZipped(HogFile *handle, int index);
bool hogFileIsSpecialFile(HogFile *handle, int index);
const U8 *hogFileGetHeaderData(HogFile *handle, int index, U32 *header_size); // Warning: pointer returned is volatile, and may be invalid after any hog file changes
F32 hogFileCalcFragmentation(HogFile *handle);

void *hogFileExtract(HogFile *handle, HogFileIndex file, U32 *count);
U32 hogFileExtractBytes(HogFile *handle, HogFileIndex file, void *buf, U32 pos, U32 size);
void *hogFileExtractCompressed(HogFile *handle, HogFileIndex file, U32 *count); // pulled from external utilities lib

// HogFileModify functions for modifying a hog_file (in memory and on disk)
int hogFileModifyDelete(HogFile *handle, HogFileIndex file);
int hogFileModifyAdd2(HogFile *handle, NewPigEntry *entry);
int hogFileModifyUpdate(HogFile *handle, HogFileIndex file, U8 *data, U32 size, U32 timestamp, bool safe_update);
int hogFileModifyUpdate2(HogFile *handle, HogFileIndex file, NewPigEntry *entry, bool safe_update);
int hogFileModifyUpdateTimestamp(HogFile *handle, HogFileIndex file, U32 timestamp);
int hogFileModifyUpdateNamed(HogFile *handle, const char *relpath, U8 *data, U32 size, U32 timestamp);
int hogFileModifyFlush(HogFile *handle);

void hogThreadingInit(void);
