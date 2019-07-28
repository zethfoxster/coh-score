#include "stdtypes.h"

typedef struct TFCRequest TFCRequest;
typedef void (*FinishCallback)(TFCRequest* request, int success, int total_bytes);
typedef void (*ProgressCallback)(TFCRequest* request, int bytes, int total);
typedef void (*ThreadStatusCallback)(int requests_remaining, int numThreads, int *threadCounts);

typedef enum TFCRequestFlags {
	TFC_NORMAL=0,
	TFC_DO_NOT_WRITE=1<<0,
} TFCRequestFlags;

typedef struct TFCRequest {
	char src[MAX_PATH];
	char dst[MAX_PATH];
	ProgressCallback progressCallback;
	FinishCallback finishCallback;
	void *userData;
	U8 *data;
	int data_size;
	TFCRequestFlags flags;
} TFCRequest;

TFCRequest *createTFCRequest(void);
TFCRequest *createTFCRequestEx(char *src, char *dst, TFCRequestFlags flags, ProgressCallback progressCallback, FinishCallback finishCallback, void *userData);
void destroyTFCRequest(TFCRequest *request);

void threadedFileCopyInit(int maxThreads);
//todo: void threadedFileCopyChangeMaxThreads(int maxThreads);
void threadedFileCopyStartCopy(TFCRequest *request);

void threadedFileCopyPoll(ThreadStatusCallback statusCallback); // Checks for progress/completetion
void threadedFileCopyWaitForCompletion(ThreadStatusCallback statusCallback); // Waits for all pending copies to complete
int threadedFileCopyNumPending(void);

U32 threadedFileCopyBPS(void);

// For multiple threads writing to files, try not to thrash the disk
void enterFileWriteCriticalSection(void);
void leaveFileWriteCriticalSection(void);

//todo: void threadedFileCopyShutdown(void); // Stops all copies

// For memory usage throttling in the "old version control system":
void tfcRecordMemoryUsed(U64 size);
void tfcRecordMemoryFreed(U64 size);
