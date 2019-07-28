#ifndef DIRMONITOR_H
#define DIRMONITOR_H

#include <stdlib.h>
#include "wininclude.h"

typedef struct{
	char	dirname[MAX_PATH];
	int		changeTime;
	int		lastChangeTime;
	char*	filename; // this points to a static var in dirMonCheckDirs, and is destroyed after each subsequent call
	int		isDelete;
	int		isCreate;
	void*	userData;
} DirChangeInfo;

typedef struct DirMonCompletionPortCallbackParams {
	S32				result;
	S32				aborted;
	HANDLE			handle;
	void*			userData;
	U32				bytesTransferred;
	OVERLAPPED*		overlapped;
} DirMonCompletionPortCallbackParams;

typedef struct DirMonCompletionPortObject DirMonCompletionPortObject;

typedef void (*DirMonCompletionPortCallback)(DirMonCompletionPortCallbackParams* params);

U32 dirMonCompletionPortAddObjectDbg(HANDLE handle, void* userData, DirMonCompletionPortCallback callback, const char* fileName, S32 lineNumber);
#define dirMonCompletionPortAddObject(handle, userData, callback) dirMonCompletionPortAddObjectDbg(handle, userData, callback, __FILE__, __LINE__)
void dirMonCompletionPortRemoveObject(U32 handle);

int dirMonStop();
int dirMonIsRunning();

int dirMonAddDirectory(const char* dirname, int monitorSubDirs, void* userData);
void dirMonRemoveDirectory(const char* dirname);

DirChangeInfo* dirMonCheckDirs(int timeout, DirChangeInfo** bufferOverrun);

enum {
	DIR_MON_NORMAL = 0,
	DIR_MON_NO_TIMESTAMPS = 1 << 0,
};
void dirMonSetFlags(int flags);
void dirMonSetBufferSize(int size); // Defaults to 1MB

#endif