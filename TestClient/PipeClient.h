#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef enum {
	PCS_DISCONNECTED,
	PCS_CONNECTED,
} PipeClientState;

typedef struct PipeClient_t {
	HANDLE hPipe; 
	char buf[100000]; // buffer for incoming messages
	PipeClientState connected;
} PipeClient_t;

typedef PipeClient_t *PipeClient;

PipeClient PipeClientCreate(const char *pipe_name, const char *pipe_server); // Server == "." for local machine
void PipeClientDestroy(PipeClient p);
void PipeClientSendMessage(PipeClient p, const char *fmt, ...);
char *PipeClientGetMessage(PipeClient pc);
