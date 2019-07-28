// Note: these files are also included in various other projects (GetTex)
#include "process_util.h"
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "psapi.lib")

static char killName[MAX_PATH];
static char killName2[MAX_PATH];

int kill(DWORD pid) {
	HANDLE h;
	int ret;
	h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (!h) {
		printf("Access denied when opening process (Error code: %d)\n", GetLastError());
		return 0;
	}
	ret = TerminateProcess(h, 0);
	CloseHandle(h);
	return ret;
}

int getParentPIDIfLauncher()
{
	char launcherName[] = "NCLauncher.exe";
	DWORD pid = GetCurrentProcessId();
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	if( Process32First(h, &pe)) {
		do {
			if (pe.th32ProcessID == pid) {
				
				printf("PID: %i, PPID: %i\n", pid, pe.th32ParentProcessID);
				if (ProcessNameMatch(pe.th32ParentProcessID, launcherName))
				{
					return pe.th32ParentProcessID;
				}
				else
				{
					printf("Found parent process PID: %i, but process name isn't %s\n", pe.th32ParentProcessID, launcherName);
					return -1;
				}

				
			}
		} while( Process32Next(h, &pe));
	}

	// we didn't find our own process ID??
	return -1;
}

int killParentIfLauncher()
{
	return kill(getParentPIDIfLauncher());
}

void CheckProcessNameAndID( DWORD processID)
{
	char szProcessName[MAX_PATH] = "unknown";

	// Get a handle to the process.

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID );

	// Get the process name.

	if (NULL != hProcess )
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
			&cbNeeded) )
		{
			GetModuleBaseName( hProcess, hMod, szProcessName, 
				sizeof(szProcessName) );
		}
		else 
		{
			CloseHandle( hProcess );
			return;
		}
	}
	else return;

	// Print the process name and identifier.

	if (stricmp(szProcessName, killName)==0 ||
		stricmp(szProcessName, killName2)==0)
	{
		printf( "Killing PID %6u: %s\n", processID, szProcessName );
		kill(processID);
	}

	CloseHandle( hProcess );
}


void killall(const char * module)
{
	KillAllEx(module, false);
}

void KillAllEx(const char * module, bool bSkipSelf)
{
	// Get the list of process identifiers.
	DWORD aProcesses[1024], cbNeeded, cProcesses, currentPID;
	unsigned int i;


	strcpy(killName, module);
	strcpy(killName2, killName);
	strcat(killName2, ".exe");

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return;

	// Calculate how many process identifiers were returned.

	cProcesses = cbNeeded / sizeof(DWORD);

	// Print the name and process identifier for each process.

	currentPID = GetCurrentProcessId();
	for ( i = 0; i < cProcesses; i++ )
	{
		if(!(bSkipSelf && (aProcesses[i] == currentPID)))
			CheckProcessNameAndID( aProcesses[i] );
	}
}

BOOL ProcessNameMatch( DWORD processID , char * targetName)
{
	char szProcessName[MAX_PATH] = "unknown";

	// Get a handle to the process.

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID );

	// Get the process name.

	if (NULL != hProcess )
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
			&cbNeeded) )
		{
			GetModuleBaseName( hProcess, hMod, szProcessName, 
				sizeof(szProcessName) );
		}
		else {
			CloseHandle( hProcess );
			return FALSE;
		}
	}
	else return FALSE;

	// Print the process name and identifier.
	CloseHandle( hProcess );

	if (stricmp(szProcessName, targetName)==0)
		return TRUE;
	else
		return FALSE;
}

int ProcessCount(char * procName)
{
	// Get the list of process identifiers.
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	int count = 0;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return 0;

	// Calculate how many process identifiers were returned.

	cProcesses = cbNeeded / sizeof(DWORD);

	// Print the name and process identifier for each process.

	for ( i = 0; i < cProcesses; i++ )
	{
		if(ProcessNameMatch( aProcesses[i] , procName))
			count++;
	}
	return count;
}

void procMemorySizes(size_t *wsbytes, size_t *peakwsbytes, size_t *pfbytes, size_t *peakpfbytes)
{
	PROCESS_MEMORY_COUNTERS info, zero_info = {0};
	if(!GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info)))
		info = zero_info;

	if(wsbytes)
		*wsbytes = info.WorkingSetSize;
	if(peakwsbytes)
		*peakwsbytes = info.PeakWorkingSetSize;
	if(pfbytes)
		*pfbytes = info.PagefileUsage;
	if(peakpfbytes)
		*peakpfbytes = info.PeakPagefileUsage;
}
