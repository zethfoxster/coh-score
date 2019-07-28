// LWC_pipe_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>


static HANDLE s_pipe_handle = INVALID_HANDLE_VALUE;

void PrintError(const WCHAR  *message)
{
	WCHAR cBuf[1000];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, cBuf, 1000, NULL);
	wprintf( L"%s. Windows system error message: %s", message, cBuf);
}

void SendMessage(const char *message)
{
	DWORD bytesWritten;
	if (WriteFile(s_pipe_handle, message, strlen(message) + 1, &bytesWritten, 0))
	{
		printf("Successfully wrote: %s\n", message);
	}
	else
	{
		PrintError( L"WriteFile Failed" );
	}
}

void HandleMessages(void)
{
	char inBuffer[1000];
	DWORD bytesRead;

	while(ReadFile(s_pipe_handle, inBuffer, 1000 - 1, &bytesRead, NULL))
	{

		if (bytesRead)
		{
			// Make sure it is NULL terminated
			if (inBuffer[bytesRead - 1] != 0)
				inBuffer[bytesRead] = 0;

			printf("Received pipe message: %s\n", inBuffer);
		}
	}
}

int main(int argc, char* argv[])
{
	int i;
	char message[1000] = "";

	/*
	if (argc < 2)
	{
		printf("Please specify some arguments to send over the pipe\n");
		return 0;
	}
	*/

	if (WaitNamedPipe(L"\\\\.\\pipe\\Coh_NCLauncher_Pipe", NMPWAIT_WAIT_FOREVER) == 0)
	{
		PrintError(L"WaitNamedPipe failed");
		return -1;
	}
	
	s_pipe_handle = CreateFile(L"\\\\.\\pipe\\Coh_NCLauncher_Pipe", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);

	if (s_pipe_handle == INVALID_HANDLE_VALUE)
	{
		PrintError( L"Failed to open named pipe" );

		return -1;
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT; 
	BOOL fSuccess;
	fSuccess = SetNamedPipeHandleState( s_pipe_handle,    // pipe handle 
										&dwMode,  // new pipe mode 
										NULL,     // don't set maximum bytes 
										NULL);    // don't set maximum time 
	if ( ! fSuccess) 
	{
		PrintError(L"SetNamedPipeHandleState failed");
		return -1;
	}
 

	printf("Successfully opened pipe\n");

	for (i = 1; i < argc; i++)
	{
		SendMessage(argv[i]);
	}

	do
	{
		gets(message);
		SendMessage(message);
		HandleMessages();
	}
	while (strcmp(message, "END") != 0);

	CloseHandle(s_pipe_handle);

	return 0;
}

