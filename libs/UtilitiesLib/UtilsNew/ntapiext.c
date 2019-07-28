#include "ntapiext.h"

static HMODULE s_ntdll = INVALID_HANDLE_VALUE;

PNtAlertThread NtAlertThread;
PNtTestAlert NtTestAlert;
PNtDelayExecution NtDelayExecution;
PNtCreateKeyedEvent NtCreateKeyedEvent;
PNtOpenKeyedEvent NtOpenKeyedEvent;
PNtWaitForKeyedEvent NtWaitForKeyedEvent;
PNtReleaseKeyedEvent NtReleaseKeyedEvent;

HANDLE g_keyed_event_handle = INVALID_HANDLE_VALUE;

void init_ntapiext() {
	if (s_ntdll != INVALID_HANDLE_VALUE)
		return;

	s_ntdll = LoadLibrary("ntdll");
	if (s_ntdll == INVALID_HANDLE_VALUE) {
		s_ntdll = NULL; // prevent us from trying again
		return;
	}

	NtAlertThread = (PNtAlertThread)GetProcAddress(s_ntdll, "NtAlertThread");
	NtTestAlert = (PNtTestAlert)GetProcAddress(s_ntdll, "NtTestAlert");
	NtDelayExecution = (PNtDelayExecution)GetProcAddress(s_ntdll, "NtDelayExecution");

	NtCreateKeyedEvent = (PNtCreateKeyedEvent)GetProcAddress(s_ntdll, "NtCreateKeyedEvent");
	NtOpenKeyedEvent = (PNtOpenKeyedEvent)GetProcAddress(s_ntdll, "NtOpenKeyedEvent");
	NtWaitForKeyedEvent = (PNtWaitForKeyedEvent)GetProcAddress(s_ntdll, "NtWaitForKeyedEvent");
	NtReleaseKeyedEvent = (PNtReleaseKeyedEvent)GetProcAddress(s_ntdll, "NtReleaseKeyedEvent");

	if (NtCreateKeyedEvent && NtOpenKeyedEvent && NtWaitForKeyedEvent && NtReleaseKeyedEvent) {
		NtCreateKeyedEvent(&g_keyed_event_handle, EVENT_ALL_ACCESS, NULL, 0);
	}
}
