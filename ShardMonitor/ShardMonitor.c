#include <windows.h>

int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					LPSTR lpCmdLine, int nCmdShow )
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	if (!CreateProcess(NULL, "ServerMonitor.exe -shardmonitor", NULL, NULL, 0, 0, NULL, NULL, &si, &pi))
	{
		MessageBox(NULL, "Error spawning ServerMonitor.exe -shardmonitor", "Error", MB_OK);
	}
	return 0;
}
