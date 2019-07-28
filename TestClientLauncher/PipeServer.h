#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct MessageList {
	struct MessageList *next;
	struct MessageList *prev;
	int uid;
	char *data;
} MessageList;

typedef struct PipeServer_t {
	char *pipename; 
	HANDLE hWatcherThread;
	HANDLE hInstanceThread;
	HANDLE **eaPipes; // Pipes to all clients
	HANDLE *eaPipes_data;
	CRITICAL_SECTION critsect;
	MessageList *messagelist;
} PipeServer_t;

typedef PipeServer_t *PipeServer;

PipeServer PipeServerCreate(const char *pipe_name);
void PipeServerDestroy(PipeServer p);

// UID is an incremental, unique id identifying the client, msg is the message, or NULL if it's a disconnect message
typedef void (*PipeServerCallback)(int UID, const char *msg);
void PipeServerQuery(PipeServer p, PipeServerCallback callback);

void PipeServerSendMessage(PipeServer p, int uid, const char *fmt, ...);
void PipeServerClosePipe(PipeServer p, int uid);


