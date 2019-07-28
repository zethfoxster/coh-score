/**
* Exposes undocumented NT API calls.
*/

#ifndef _NTKEYEDEVENT_H
#define _NTKEYEDEVENT_H

#include "wininclude.h"
#include <winternl.h>

C_DECLARATIONS_BEGIN

typedef struct _OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

// Alerts
typedef NTSTATUS (NTAPI *PNtAlertThread)(IN HANDLE ThreadHandle);
typedef NTSTATUS (NTAPI *PNtTestAlert)();
typedef NTSTATUS (NTAPI *PNtDelayExecution)(IN BOOLEAN Alertable, IN PLARGE_INTEGER DelayInterval);

// Keyed Events
typedef NTSTATUS (NTAPI *PNtCreateKeyedEvent)(OUT PHANDLE handle, IN ACCESS_MASK access, IN POBJECT_ATTRIBUTES attr, IN ULONG flags);
typedef NTSTATUS (NTAPI *PNtOpenKeyedEvent)(OUT PHANDLE handle, IN ACCESS_MASK access, IN POBJECT_ATTRIBUTES attr);
typedef NTSTATUS (NTAPI *PNtWaitForKeyedEvent)(IN HANDLE handle, IN PVOID key, IN BOOLEAN alertable, IN PLARGE_INTEGER mstimeout);
typedef NTSTATUS (NTAPI *PNtReleaseKeyedEvent)(IN HANDLE handle, IN PVOID key, IN BOOLEAN alertable, IN PLARGE_INTEGER mstimeout);

void init_ntapiext();

extern PNtAlertThread NtAlertThread;
extern PNtTestAlert NtTestAlert;
extern PNtDelayExecution NtDelayExecution;
extern PNtCreateKeyedEvent NtCreateKeyedEvent;
extern PNtOpenKeyedEvent NtOpenKeyedEvent;
extern PNtWaitForKeyedEvent NtWaitForKeyedEvent;
extern PNtReleaseKeyedEvent NtReleaseKeyedEvent;

extern HANDLE g_keyed_event_handle;

static INLINEDBG bool have_keyed_event() {
	return g_keyed_event_handle != INVALID_HANDLE_VALUE;
}

C_DECLARATIONS_END

#endif

