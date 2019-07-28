#ifndef _XBOX

#include "DirMonitor.h"
#include "ArrayOld.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>
#include "MemoryPool.h"
#include "utils.h"

typedef struct{
	DirChangeInfo info;

	HANDLE		dir;
	OVERLAPPED	ol;
	char		*dirChangeBuffer;
	int			dirChangeBufferSizeOrig;
	DWORD		dirChangeBufferSize;
	U32			completionHandle;
} DirChangeInfoImp;


Array dirChangeInfos;
static HANDLE dirMonCompletionPort;

static int dir_mon_flags = 0;
static int dir_mon_buffer_size = 1024*1024;

static void dirMonOperationCompleted(DirMonCompletionPortCallbackParams* params);

void dirMonSetFlags(int flags) {
	dir_mon_flags = flags;
}

void dirMonSetBufferSize(int size)
{
	dir_mon_buffer_size = size;
}

int dirMonStop(){
	return 0;	
}

int dirMonIsRunning(){
	if(dirChangeInfos.size)
		return 1;
	else
		return 0;
}

// Returns 0 on failure
static int dirMonStartAsyncMonitor(DirChangeInfoImp* info){
	int result;
	result = ReadDirectoryChangesW(
		info->dir,							// handle to directory
		info->dirChangeBuffer,				// read results buffer
		info->dirChangeBufferSizeOrig,		// length of buffer
		1,									// monitoring option
		FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
		FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | // FILE_NOTIFY_CHANGE_LAST_ACCESS |
		FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY,                               // filter conditions
		&info->dirChangeBufferSize,		// bytes returned
		&info->ol,						// overlapped buffer
		NULL							// completion routine
		);

	//assert(result && "DirMonitor init failed - not available on Network drives - dynamic reloading disabled, press Ignore.");
	return result;
}

static void dirMonFreeChangeInfo(DirChangeInfoImp* info){
	dirMonCompletionPortRemoveObject(info->completionHandle);
	CloseHandle(info->dir);
	free(info->dirChangeBuffer);
	free(info->info.userData);
	free(info);
}

typedef void (*DirMonCompletionPortCallback)(DirMonCompletionPortCallbackParams* params);

typedef struct DirMonCompletionPortObject {
	U16								selfIndex;
	HANDLE							handle;
	DirMonCompletionPortCallback	callback;
	void*							userData;
	const char*						fileName;
	S32								lineNumber;
	U16								uid;
} DirMonCompletionPortObject;

struct {
	struct {
		DirMonCompletionPortObject*	object;
		S32							count;
		S32							maxCount;
	} objects;
		
	struct {
		U32*						index;
		U32							count;
		U32							maxCount;
	} available;
	
	U16								curUID;
} dirMonCompletionObjects;

U32 dirMonCompletionPortAddObjectDbg(	HANDLE handle,
										void* userData,
										DirMonCompletionPortCallback callback,
										const char* fileName,
										S32 lineNumber)
{
	DirMonCompletionPortObject*			object;
	U32 								completionHandle;
	U32									index;
	
	assert(dirMonCompletionObjects.objects.count < (1 << 16));
	
	if(dirMonCompletionObjects.available.count){
		assert(dirMonCompletionObjects.available.count > 0);
		
		index = dirMonCompletionObjects.available.index[--dirMonCompletionObjects.available.count];
		
		assert(index < (U32)dirMonCompletionObjects.objects.count);
		
		object = dirMonCompletionObjects.objects.object + index;
		
		assert(!object->handle);
	}else{
		index = dirMonCompletionObjects.objects.count;
		
		object = dynArrayAddStructType(	DirMonCompletionPortObject,
										&dirMonCompletionObjects.objects.object,
										&dirMonCompletionObjects.objects.count,
										&dirMonCompletionObjects.objects.maxCount);
	}
	
	while(!++dirMonCompletionObjects.curUID);
	
	object->uid = dirMonCompletionObjects.curUID;
	
	completionHandle = (index) | ((U32)object->uid << 16);

	object->selfIndex = index;
	object->callback = callback;
	object->handle = handle;
	object->userData = userData;
	object->fileName = fileName;
	object->lineNumber = lineNumber;
	
	//printf("Added 0x%8.8x: %s:%d\n", object, fileName, lineNumber);

	dirMonCompletionPort = CreateIoCompletionPort(handle, dirMonCompletionPort, (ULONG_PTR)completionHandle, 0);
	
	assert(dirMonCompletionPort);
	
	return completionHandle;
}

static DirMonCompletionPortObject* decodeCompletionHandle(U32 handle, S32* indexOut){
	S32 index = handle & 0xffff;
	S32 uid = (handle >> 16) & 0xffff;
	
	if(index >= 0 && index < dirMonCompletionObjects.objects.count){
		DirMonCompletionPortObject* object = dirMonCompletionObjects.objects.object + index;
		
		if(object->uid == uid){
			if(indexOut){
				*indexOut = index;
			}
			
			return object;
		}
	}
	
	return NULL;
}

void dirMonCompletionPortRemoveObject(U32 handle){
	S32							index;
	DirMonCompletionPortObject* object = decodeCompletionHandle(handle, &index);
	S32*						availableIndex;
	
	if(!object){
		return;
	}
	
	//printf("Removed 0x%8.8x: %s:%d\n", object, object->fileName, object->lineNumber);

	assert(object->selfIndex == index);
	
	if(!CancelIo(object->handle)){
		printf("CancelIo failed: %d\n", GetLastError());
	}else{
		//printf("CancelIo success: 0x%8.8x (%s:%d) error %d\n", object, object->fileName, object->lineNumber, GetLastError());
	}
	
	availableIndex = dynArrayAddStructType(	S32,
											&dirMonCompletionObjects.available.index,
											&dirMonCompletionObjects.available.count,
											&dirMonCompletionObjects.available.maxCount);
											
	*availableIndex = index;
	
	ZeroStruct(object);
}

int dirMonAddDirectory(const char* dirname, int monitorSubDirs, void* userData){
	
	DirChangeInfoImp* info;
	HANDLE dir;

	assert(dirname);
	assert(strlen(dirname) <= MAX_PATH);

	// Start an async dir change monitor operation.
	//	Open the directory for monitoring.
	//		Get a handle to the directory to be watched.

	//		Make sure we share the directory in such a way that other programs
	//		are allowed full access to all files + directories.
	dir = CreateFile(
		dirname,											// pointer to the file name
		FILE_LIST_DIRECTORY,								// access (read/write) mode
		FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,  // share mode
		NULL,												// security descriptor
		OPEN_EXISTING,										// how to create
		FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,	// file attributes
		NULL												// file with attributes to copy
		);

	//		Does the specified directory exist at all?
	//		If not, it cannot be watched.
	if(INVALID_HANDLE_VALUE == dir){
		return 0;
	}


	//	Create a new dir change info.
	info = (DirChangeInfoImp*)calloc(1, sizeof(DirChangeInfoImp));
	info->dirChangeBuffer = (char*)calloc(1, dir_mon_buffer_size);
	info->dirChangeBufferSizeOrig = dir_mon_buffer_size;
	arrayPushBack(&dirChangeInfos, info);
	strcpy(info->info.dirname, dirname);
	info->dir = dir;

	//	Watch for changes on the specified directory.
	if (!dirMonStartAsyncMonitor(info)) {
		// Could not start it, free memory, and return
		arrayPopBack(&dirChangeInfos);
		dirMonFreeChangeInfo(info);
		return 0;
	}

	// Add the new async operation to the the IO completion port.
	
	info->completionHandle = dirMonCompletionPortAddObject(info->dir, info, dirMonOperationCompleted);
	
	info->info.userData = userData;
	
	return 1;
}

void dirMonRemoveDirectory(const char* dirname){
	S32 i;
	for(i = 0; i < dirChangeInfos.size; i++){
		DirChangeInfoImp* info = (DirChangeInfoImp*)dirChangeInfos.storage[i];
		
		if(!stricmp(info->info.dirname, dirname)){
			arrayRemoveAndFill(&dirChangeInfos, i);
			dirMonFreeChangeInfo(info);
			break;
		}
	}
}

static FILE_NOTIFY_INFORMATION *fniExtractFilename(FILE_NOTIFY_INFORMATION *fileChangeInfo, char *filename) {
	unsigned int i;
	// Turn the returned filename into a null terminating string.
	for(i = 0; i < fileChangeInfo->FileNameLength >> 1; i++){
		filename[i] = (char)fileChangeInfo->FileName[i];
	}
	filename[i] = '\0';
	if (fileChangeInfo->NextEntryOffset) {
		return (FILE_NOTIFY_INFORMATION *)((char*)fileChangeInfo + fileChangeInfo->NextEntryOffset);
	} else {
		return NULL;
	}
}

static struct {
	DirChangeInfoImp*			info;
	FILE_NOTIFY_INFORMATION*	fileChangeInfo;
	DirChangeInfo**				bufferOverrun;
} curDirMonOp;

static void dirMonOperationCompleted(DirMonCompletionPortCallbackParams* params){
	if(params->result){
		curDirMonOp.info = (DirChangeInfoImp*)params->userData;
		
		if(params->bytesTransferred) {
			curDirMonOp.fileChangeInfo = (FILE_NOTIFY_INFORMATION*)curDirMonOp.info->dirChangeBuffer;
		} else {
			// Buffer overrun!
			curDirMonOp.info->info.filename = NULL;
			curDirMonOp.info->info.lastChangeTime = curDirMonOp.info->info.changeTime;
			curDirMonOp.info->info.changeTime = time(NULL);
			if(curDirMonOp.bufferOverrun){
				*curDirMonOp.bufferOverrun = &curDirMonOp.info->info;
			}
			dirMonStartAsyncMonitor(curDirMonOp.info);
		}
	}
	else if(params->aborted){
		printfColor(COLOR_BRIGHT|COLOR_RED, "Aborted dirMon operation!\n");
	}
}

DirChangeInfo* dirMonCheckDirs(int timeout, DirChangeInfo** bufferOverrun){
	curDirMonOp.bufferOverrun = bufferOverrun;

	if(bufferOverrun){
		*bufferOverrun = NULL;
	}

	if (!curDirMonOp.fileChangeInfo) {
		int result;
		DWORD bytesTransferred;
		OVERLAPPED* ol;
		ULONG_PTR handle = 0;
		
		// Poll the IO completion port for directory changes.

		result = GetQueuedCompletionStatus(dirMonCompletionPort, &bytesTransferred, &handle, &ol, timeout);
		
		if(handle){
			DirMonCompletionPortObject* object = decodeCompletionHandle(handle, NULL);
			
			if(object){
				DirMonCompletionPortCallbackParams params;
				
				params.result = result;
				params.aborted = !result && GetLastError() == ERROR_OPERATION_ABORTED;
				params.bytesTransferred = bytesTransferred;
				params.handle = object->handle;
				params.overlapped = ol;
				params.userData = object->userData;
				
				object->callback(&params);
			}
		}
	}
	
	// Intentionally absent "else" because "curDirMonOp.fileChangeInfo" can change in the io completion callback.
	
	if(curDirMonOp.fileChangeInfo){
		static char filename[512];
		
		// There is still file change info to process.
		
		struct stat fileInfo;

		assert(curDirMonOp.info);

		
		curDirMonOp.info->info.isDelete =	curDirMonOp.fileChangeInfo->Action == FILE_ACTION_REMOVED ||
											curDirMonOp.fileChangeInfo->Action == FILE_ACTION_RENAMED_OLD_NAME;

		curDirMonOp.info->info.isCreate =	curDirMonOp.fileChangeInfo->Action == FILE_ACTION_ADDED ||
											curDirMonOp.fileChangeInfo->Action == FILE_ACTION_RENAMED_NEW_NAME;

		curDirMonOp.fileChangeInfo = fniExtractFilename(curDirMonOp.fileChangeInfo, filename);
		
		curDirMonOp.info->info.filename = filename;

		if (!(dir_mon_flags & DIR_MON_NO_TIMESTAMPS)) {
			curDirMonOp.info->info.lastChangeTime = curDirMonOp.info->info.changeTime;

			// If the file still exists, use the file time as the last change time...
			if(0 == stat(filename, &fileInfo)){
				curDirMonOp.info->info.changeTime = fileInfo.st_mtime;
			}else{
				// Otherwise, use the current system time as the last change time.
				curDirMonOp.info->info.changeTime = time(NULL);
			}
		}
		if (curDirMonOp.fileChangeInfo == NULL) { // We've returned all there is to return
			dirMonStartAsyncMonitor(curDirMonOp.info);
		}
		return &curDirMonOp.info->info;
	}
	return NULL;
}

#else

#include "DirMonitor.h"

// Stub out the xbox implementation for now

int dirMonAddDirectory(const char* dirname, int monitorSubDirs, void* userData)
{
	return 0;
}
void dirMonRemoveDirectory(const char* dirname)
{
}

DirChangeInfo* dirMonCheckDirs(int timeout, DirChangeInfo** bufferOverrun)
{
	if(bufferOverrun){
		*bufferOverrun = NULL;
	}
	return 0;
}

void dirMonSetFlags(int flags)
{
}


#endif

