#ifndef _WORKERTHREAD_H_
#define _WORKERTHREAD_H_

#include "wininclude.h"


/*************************************************************************
/  Datatype to manage creating a data processing thread (ie for rendering).
/  The spawned worker thread sleeps when no commands are queued.
/  Commands go to the worker and messages come back.
*************************************************************************/


typedef struct WorkerThread WorkerThread;

typedef void (*WTDebugCallback)(void *data);
typedef void (*WTDispatchCallback)(void *user_data, int type, void *data);
typedef void (*WTStallCallback)(void);

enum
{
	// built in commands
	WT_CMD_NOP=1,
	WT_CMD_DEBUGCALLBACK,

	// user commands start here
	WT_CMD_USER_START
};


// creation/destruction
WorkerThread *wtCreate(WTDispatchCallback cmd_dispatch, int max_cmds, WTDispatchCallback msg_dispatch, WTStallCallback stall_callback, int max_msgs, void *user_data);
void wtDestroy(WorkerThread *wt);

// set threading and start worker thread
void wtStart(WorkerThread *wt);
void wtSetThreaded(WorkerThread *wt,int run_threaded);
void wtSetProcessor(WorkerThread *wt, int processor_idx);
int wtIsThreaded(WorkerThread *wt);
HANDLE wtGetThreadHandle(WorkerThread *wt);
U32 wtGetThreadID(WorkerThread *wt);

// flush and monitor functions for main thread
void wtFlush(WorkerThread *wt);			// wait for worker thread to finish its commands, dispatch messages from worker thread
void wtMonitor(WorkerThread *wt);		// dispatch messages from worker thread

// flush for worker thread
void wtFlushMessages(WorkerThread *wt);	// wait for main thread to dispatch messages

// insert a debugging callback into the worker thread queue
void wtQueueDebugCmd(WorkerThread *wt,WTDebugCallback callback_func,void *data,int size);

// send commands to worker thread
void wtQueueCmd(WorkerThread *wt,int cmd_type,const void *cmd_data,int size);
void *wtAllocCmd(WorkerThread *wt,int cmd_type,int size);
void wtCancelCmd(WorkerThread *wt);
void wtSendCmd(WorkerThread *wt);

// send message from worker thread to main thread
void wtQueueMsg(WorkerThread *wt,int msg_type,void *msg_data,int size);

// check if the current thread is the worker
bool wtInWorkerThread(WorkerThread *wt);
void wtAssertWorkerThread(WorkerThread *wt);
void wtAssertNotWorkerThread(WorkerThread *wt);

// debugging function
void wtSetFlushRequested(WorkerThread *wt, int flush_requested);

#endif


