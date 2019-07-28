#include "processes.h"
#include "stashtable.h"
#include "builderizer.h"
#include "utils.h"
#include <Tlhelp32.h>


static StashTable processes = NULL;

#define STRIP_LEADING_DOLLARSIGN(str) ((str) = (str)[0] == '$' ? &(str)[1] : (str))

PROCESS_INFORMATION *procGetInfo(char *name)
{
	PROCESS_INFORMATION *pi;
	STRIP_LEADING_DOLLARSIGN(name);
	stashFindPointer(processes, name, &pi);
	return pi;
}

int procGetExitCode(char *name, int *exitCode)
{
	PROCESS_INFORMATION *pi = procGetInfo(name);
	if (pi && GetExitCodeProcess(pi->hProcess, exitCode))
	{
		setBuildStateVar("ERRORLEVEL", STACK_SPRINTF("%d", exitCode));
		return 1;
	}
	return 0;
}

void procWait(HANDLE *proc)
{
	WaitForSingleObject(proc, INFINITE);
}

int procIsRunning(char *name)
{
	PROCESS_INFORMATION *pi;
	int ret = WAIT_OBJECT_0;
	STRIP_LEADING_DOLLARSIGN(name);
	pi = procGetInfo(name);
	if ( pi )
		ret = WaitForSingleObject(pi->hProcess, 0);
	return ret != WAIT_OBJECT_0;
}

int procStore(char *name, PROCESS_INFORMATION *pi)
{
	PROCESS_INFORMATION *newPi = NULL;
	STRIP_LEADING_DOLLARSIGN(name);

	if ( !processes )
		processes = stashTableCreateWithStringKeys(25, StashDeepCopyKeys);
	
	if ( stashFindPointer(processes, name, &newPi) )
	{
		if ( procIsRunning(name) )
		{
			builderizerError("procStore - There is already a process \'%s\'\n", name);
			return 0;
		}
	}

	if ( !newPi )
	{
		newPi = malloc(sizeof(PROCESS_INFORMATION));
		stashAddPointer(processes, name, newPi, 0);
	}
	memcpy(newPi, pi, sizeof(PROCESS_INFORMATION));
	return 1;
}

int procFindProcesses(char *exe, HANDLE **hProcs)
{
	DWORD				dwOwnerPID = GetCurrentProcessId();
	PROCESSENTRY32		pe32 = {0};
	HANDLE				hProcessSnap;
	S32					notDone;

	// Find all the processs in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(hProcessSnap != INVALID_HANDLE_VALUE)
	{
		pe32.dwSize = sizeof(pe32);
		
		for(notDone = Process32First(hProcessSnap, &pe32); notDone; notDone = Process32Next(hProcessSnap, &pe32))
		{
			U32		pid = pe32.th32ProcessID;
			HANDLE	hProcess;
			
			if(pid == dwOwnerPID){
				continue;
			}
			
			if(!strstriConst(pe32.szExeFile, exe)){
				continue;
			}
			
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
			
			if(!hProcess){
				statePrintf(1, COLOR_RED, "Can't get handle to process %5d: %s...\n", pid, pe32.szExeFile);
				continue;
			}

			eaPush(hProcs, hProcess);
		}
		
		CloseHandle(hProcessSnap);
	}

	return eaSize(hProcs);
}

void procKillHandle(HANDLE *proc)
{
	TerminateProcess(proc, 0);
	// wait for proc to die
	procWait(proc);
	CloseHandle(proc);
}

void procKill(PROCESS_INFORMATION *pi)
{
	procKillHandle(pi->hProcess);
	CloseHandle(pi->hThread);
}

void procFreeProcesses(HANDLE **hProcs)
{
	while (eaSize(hProcs))
	{
		CloseHandle((HANDLE)eaPop(hProcs));
	}
}
