#ifndef _XBOX

#include <windows.h>
#include "osdependent.h"

int g_isUsingWin2korXP = -1;
static int g_isUsingVistaOrLater = -1;
static int g_isUsingWin7orLater = -1;

int IsUsingWin2kOrXp(void)
{
	if (g_isUsingWin2korXP == -1)
		InitOSInfo();
	return g_isUsingWin2korXP;
}

int IsUsingWin9x(void)
{
	return !IsUsingWin2kOrXp();
}

int IsUsingVistaOrLater(void)
{
	if (g_isUsingVistaOrLater == -1)
		InitOSInfo();
	return g_isUsingVistaOrLater;
}

int IsUsingWin7orLater(void)
{
	if (g_isUsingWin7orLater == -1)
		InitOSInfo();
	return g_isUsingWin7orLater;
}

int IsUsing64BitWindows(void)
{
#if defined(_WIN64)
	return 1;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
	// 32-bit programs run on both 32-bit and 64-bit Windows
	// so must sniff
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(LoadLibrary("kernel32"), "IsWow64Process");
	if (fnIsWow64Process != NULL)
	{
		int f64;
		return fnIsWow64Process(GetCurrentProcess(), &f64) && f64;
	}
	return 0;
#else
	return 0; // Win64 does not support Win16
#endif
}

int IsUsingCider()
{
	static int result = -1;
	
	if (result < 0)
	{
		// These function definitions came from an August, 2008, email from
		// Transgaming's Blair Yakimovich
		HMODULE ntdllModule = GetModuleHandle(TEXT("ntdll.dll"));
		typedef BOOL (WINAPI* IsTransgamingFunc)();
		IsTransgamingFunc IsTransgaming;
		typedef const char* (WINAPI* TGGetOSFunc)();
		TGGetOSFunc TGGetOS;

		if (!ntdllModule)
		{
			result = 0;
			return result;
		}

		IsTransgaming = 
			(IsTransgamingFunc)GetProcAddress(ntdllModule, "IsTransgaming");
		
		if (!IsTransgaming || !IsTransgaming())
		{
			result = 0;
			return result;
		}

		TGGetOS = (TGGetOSFunc)GetProcAddress(ntdllModule, "TGGetOS");
		result = TGGetOS && !strcmp(TGGetOS(), "MacOSX");
	}

	return result;
}

int IsUsingWine(void)
{
	static int result = -1;

	if (result < 0)
	{
		HMODULE ntdllModule = GetModuleHandle(TEXT("ntdll.dll"));
		void* wine_get_version;

		if (!ntdllModule)
		{
			// Win9x? lololol
			result = 0;
			return result;
		}

		wine_get_version = GetProcAddress(ntdllModule, "wine_get_version");

		result = wine_get_version ? 1 : 0;
	}

	return result;
}

int IsUsingXbox(void)
{
	return false;
}

void InitOSInfo(void)
{
	OSVERSIONINFO os_version_info;

	os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx( &os_version_info );

	g_isUsingWin2korXP = (os_version_info.dwPlatformId == VER_PLATFORM_WIN32_NT);
	g_isUsingVistaOrLater = (os_version_info.dwMajorVersion >= 6);
	g_isUsingWin7orLater = (os_version_info.dwMajorVersion > 6) ||
		(os_version_info.dwMajorVersion == 6 && os_version_info.dwMinorVersion >= 1);
}

void FillCommandLine(int *argc, char ***argv)
{
	// XBOX only
}

#else

#include "wininclude.h"
#include "utils.h"

int IsUsingWin2kOrXp(void)
{
	return false;
}

int IsUsingWin9x(void)
{
	return false;
}

int IsUsingXbox(void)
{
	return true;
}

int IsUsingVistaOrLater(void)
{
	return false;
}

void InitOSInfo(void)
{
	
}

// Fills in the xbox command line info. This WON'T work in retail
void FillCommandLine(int *argc, char ***argv)
{
	static char* cbuffer;
	static char *args[50];
	char *cline = GetCommandLine();
	cbuffer = strdup(cline);
	*argc = tokenize_line(cbuffer,args,0);
	*argv = args;
}



#endif