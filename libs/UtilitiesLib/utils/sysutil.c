#include "sysutil.h"
#include "utils.h"
#include <stdio.h>
#include <process.h>
#include <assert.h>
#include "fileutil.h"
#include "error.h"
#include "fpmacros.h"
#include "mathutil.h"
#include "sock.h"
#include "osdependent.h"

#ifndef _XBOX
#include "psapi.h"

#pragma comment(lib, "Version.lib")

char* getComputerName(){
	static char buffer[1024];
	int bufferSize = 1024;

	if(!buffer[0])
	{
		GetComputerName(buffer, &bufferSize);
		buffer[bufferSize] = '\0';
	}
	
	return buffer;
}

char* getExecutableName(){
	int result;
	static char moduleFilename[MAX_PATH];
	char long_path_name[MAX_PATH];
	result = GetModuleFileName(NULL, moduleFilename, MAX_PATH);
	assert(result);		// Getting the executable filename should not fail.

	//The above can return an 8.3 path, if so, convert it
	makeLongPathName(moduleFilename, long_path_name);
	strcpy(moduleFilename, long_path_name);
	
	return moduleFilename;
}

char *getExecutableDir(char *buf)
{
	GetModuleFileName(NULL, buf, MAX_PATH);
	forwardSlashes(buf);
	return getDirectoryName(buf);
}

char* getExecutableVersion(int dots){
	return getExecutableVersionEx(getExecutableName(), dots);
}

char* getExecutableVersionEx(char* executableName, int dots){
	int result;
	void* fileVersionInfo;
	char* moduleFilename = executableName;
	int fileVersionInfoSize;
	VS_FIXEDFILEINFO* fileInfo;
	int fileInfoSize;
	static char versionStr[128];

	fileVersionInfoSize = GetFileVersionInfoSize(moduleFilename, 0);

	// If the file doesn't have any version information...
	if(!fileVersionInfoSize)
		return NULL;

	// Allocate some buffer space and retrieve version information.
	fileVersionInfo = calloc(1, fileVersionInfoSize);
	result = GetFileVersionInfo(moduleFilename, 0, fileVersionInfoSize, fileVersionInfo);
	assert(result);

	result = VerQueryValue(fileVersionInfo, "\\", &fileInfo, &fileInfoSize);

	#define HIBITS(x) x >> 16
	#define LOWBITS(x) x & ((1 << 16) - 1)
	switch(dots){
	default:
	case 4:
		sprintf_s(SAFESTR(versionStr), "%i.%i.%i.%i", HIBITS(fileInfo->dwFileVersionMS), LOWBITS(fileInfo->dwFileVersionMS), HIBITS(fileInfo->dwFileVersionLS), LOWBITS(fileInfo->dwFileVersionLS));
		break;
	case 3:
		sprintf_s(SAFESTR(versionStr), "%i.%i.%i", HIBITS(fileInfo->dwFileVersionMS), LOWBITS(fileInfo->dwFileVersionMS), HIBITS(fileInfo->dwFileVersionLS));
		break;
	case 2:
		sprintf_s(SAFESTR(versionStr), "%i.%i", HIBITS(fileInfo->dwFileVersionMS), LOWBITS(fileInfo->dwFileVersionMS));
		break;
	case 1:
		sprintf_s(SAFESTR(versionStr), "%i", HIBITS(fileInfo->dwFileVersionMS));
		break;
	}
	
	free(fileVersionInfo);
	return versionStr;
}

/* Function versionCompare()
 *	Determines which of the given versions is newer.
 *	Note that this function will destructively modify the given strings.
 *
 *	It is assumed that the given version numbers are in the xx.xx... format.
 *	There can be as many sub-version number as the filename length will
 *	allow.
 *
 *	FIXME!!! This is copied right out of PatchClient\PatchDlg.c.  It's probably
 *	a bad idea to keep two copies of this thing.
 *	
 *	Returns:
 *		
 *		-1 - Version 2 is newer.
 *		 0 - The two versions are equal.
 *		 1 - Version 1 is newer.
 */
int versionCompare(char* version1, char* version2){
	int v1Num;
	int v2Num;
	char* v1Token;
	char* v2Token;

	char v1Buffer[512];
	char v2Buffer[512];

	strcpy(v1Buffer, version1);
	strcpy(v2Buffer, version2);

	version1 = v1Buffer;
	version2 = v2Buffer;

	while(1){
		// Grab the next version number.
		v1Token = strsep(&version1, ".");
		v2Token = strsep(&version2, ".");

		// The loop has not ended.  It means that a definite answer has
		// not been produced yet.  Therefore, the two versions are currently
		// equal.

		// If version 1 ended first, then version 2 is definitely newer.
		if(!v1Token && v2Token){
			return -1;
		}

		// If version 2 ended first, then version 1 is defintely newer.
		if(v1Token && !v2Token){
			return 1;
		}

		// If both version ended at the same time, no further comparison can be done.
		// Due to our assumption that the two versions have been "equal" so far, we
		// come to the conclusion that they must be equal.
		if(!v1Token && !v2Token){
			return 0;
		}

		// Both versions still have some sub-version numbers for comparison.
		v1Num = atoi(v1Token);
		v2Num = atoi(v2Token);

		if(v1Num > v2Num)
			return 1;
		if(v1Num < v2Num)
			return -1;
	}
}

#endif
unsigned long getPhysicalMemory(unsigned long *max, unsigned long *avail ) {
	MEMORYSTATUS memoryStatus;
	ZeroMemory(&memoryStatus,sizeof(MEMORYSTATUS));
	memoryStatus.dwLength = sizeof (MEMORYSTATUS);

	GlobalMemoryStatus (&memoryStatus);
	if (max) 
		*max = memoryStatus.dwTotalPhys;
	if (avail)
		*avail = memoryStatus.dwAvailPhys;

	return memoryStatus.dwTotalPhys;
}

// give correct CR/LF pairs
void expandCRLF(char* target, const char* source)
{
	while (*source)
	{
		if (*source == '\n')
		{
			*target++ = '\r';
			*target++ = '\n';
		}
		else
			*target++ = *source;
		source++;
	}
	*target = 0;
}

#ifndef _XBOX
void winCopyToClipboard(const char* s)
{
	HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, strlen(s)*2 + 1); // some extra space to handle CR/LF

	if(handle)
	{
		char* handleCopy;
		handleCopy = GlobalLock(handle);

		if(handleCopy)
		{
			int error;

			// need to switch to correct eoln's
			expandCRLF(handleCopy, s);

			GlobalUnlock(handle);

			if(OpenClipboard(NULL))
			{
				EmptyClipboard();

				handle = SetClipboardData(CF_TEXT, handle);

				if(!handle)
				{			
					error = GetLastError();
				}

				CloseClipboard();
			}
		}
	}
}

const char *winCopyFromClipboard(void) // Returns pointer to static buffer
{
	static char *buf=NULL;
	if(OpenClipboard(NULL))
	{
		HANDLE handle = GetClipboardData(CF_TEXT);

		if(handle){
			char* data = GlobalLock(handle);
			size_t len = strlen(data)+1;
			buf = realloc(buf, len);
			strcpy_s(buf, len, data);
			GlobalUnlock(handle);
		} else {
			SAFE_FREE(buf);
		}
		CloseClipboard();
	} else {
		SAFE_FREE(buf);
	}
	return buf;
}

static HWND hwnd=NULL;
HWND compatibleGetConsoleWindow(void)
{
	if(isGuiDisabled() || IsUsingCider()) return NULL;

	if (!hwnd) {
		typedef HWND (WINAPI *tGetConsoleWindow)(void);
		tGetConsoleWindow pGetConsoleWindow = NULL;
		HINSTANCE hKernel32Dll = LoadLibrary( "kernel32.dll" );
		if (hKernel32Dll)
		{
			pGetConsoleWindow = (tGetConsoleWindow) GetProcAddress(hKernel32Dll, "GetConsoleWindow");
			
			// This FreeLibrary is okay, since if kernel32.dll exists, then its dynamically linked refcount is > 1 (I think).
			
			FreeLibrary(hKernel32Dll);
		}
		if (pGetConsoleWindow) {
			hwnd = pGetConsoleWindow();
		}
		if (!hwnd) { // Try manual way
			char buf[1024], buf2[1024];
			int tries=6;
			sprintf_s(SAFESTR(buf), "TempConsoleTitle: %d", _getpid());
			GetConsoleTitle(buf2, ARRAY_SIZE(buf2)-1);
			SetConsoleTitle(buf);
			while (hwnd==NULL && tries) {
				hwnd = FindWindow(NULL, buf);
				if (!hwnd && tries == 1) {
					printf("Warning: couldn't find window named %s\n", buf);
					Sleep(100);
				}
				tries--;
			}
			SetConsoleTitle(buf2);
		}
	}
	return hwnd;
}

void hideConsoleWindow(void) {
	compatibleGetConsoleWindow();
	if (hwnd!=NULL) {
		ShowWindow(hwnd, SW_HIDE);
	}
}

void showConsoleWindow(void) {
	compatibleGetConsoleWindow();
	if (hwnd!=NULL) {
		ShowWindow(hwnd, SW_SHOW);
	}
}

int WasLaunchedInNTDebugger(void) {
	long *data;
	int ret=0;
	// When some debug flags are set on the heap, NT clears the allocated
	// memory with 0xbaadf00d, so we check this to see if we were launched
	// in a debugger
	data = HeapAlloc(GetProcessHeap(), 0, 8);
	if (*data == 0xbaadf00d) {
		ret=1;
	}
	HeapFree(GetProcessHeap(), 0, data);
	return ret;

}

void disableRtlHeapChecking(HANDLE heap) {
	extern HANDLE _crtheap;
#ifdef _WIN64
	int heapFlagOffset = 6;
#else
	int heapFlagOffset = 4;
#endif

	// Better solution: run gflags.exe (Included in Microsft Debbuging Tools for Windows)
	//   Enter in the application name, choose Image File Options, hit Apply,
	//   this will turn it off for all instances of the program and also fix the
	//   problem with Frees being slow that this hack doesn't fix.

	// This is dependent on the current implementation of the XP (and 2K and NT?) heap
	// which stores a number of flags 16 bytes into the heap, and checks the bitmask
	// 0x7D030F60 when deciding whether or not to clear the memory with 0xbaadf00d,
	// so, we're clearing all of the bits that may cause the clearing to happen
	if (heap == NULL)
		heap = _crtheap;
	if (WasLaunchedInNTDebugger()) {
		// This assert is basically because these 2 values are the only values I saw,
		// if there are any values for these flags, they might mean something special,
		// and we should take a closer look to see if we're clearing anything
		// important. 
		// On Vista these values are not correct, so do nothing.
		if (*((long*)heap + heapFlagOffset) == 0x40000061 ||
			*((long*)heap + heapFlagOffset) == 0x40000060)  // <-- this one indicates "Heap Free Checking" "Heap Tail Checking" "Heap Parameter Checking"
		{
			// Clear the bad bits!
			*((long*)heap + heapFlagOffset) &= ~0x7D030F60;
		}
	}
}


// This function doesn't actually work... the numbers are always way off it seems.  Not even sure what ones should match up to what.
unsigned long getProcessImageSize()
{
	MEMORY_BASIC_INFORMATION mbi;
	void *addr=0, *lastaddr=0;
	SIZE_T sum_committed=0;
	SIZE_T sum_free=0;
	SIZE_T sum_reserved=0;
	SIZE_T sum_image=0;		// statics show up here
	SIZE_T sum_private=0;	// mallocs show up here
	SIZE_T sum_mapped=0;	// DLLS?
	SIZE_T sum_image0=0;	// statics show up here
	SIZE_T sum_private0=0;	// mallocs show up here
	SIZE_T sum_mapped0=0;	// DLLS?
	SIZE_T sum_image1=0;	// statics show up here
	SIZE_T sum_private1=0;	// mallocs show up here
	SIZE_T sum_mapped1=0;	// DLLS?
	// Private seems to relate to amount malloced
	int count=0;
	while (addr < (void*)0x7f000000 && count < 1024) { // Don't want this to spin out of control!
		VirtualQuery(addr, &mbi, sizeof(mbi));
		if (mbi.State == MEM_COMMIT) {
			sum_committed += mbi.RegionSize; //TM: 25164/28612K
			if (mbi.Type & MEM_IMAGE) {
				sum_image += mbi.RegionSize;	// 36769792
				sum_image0 += mbi.RegionSize;
			}
			if (mbi.Type & MEM_MAPPED) {
				sum_mapped += mbi.RegionSize;	// 16568320
				sum_mapped0 += mbi.RegionSize;
			}
			if (mbi.Type & MEM_PRIVATE) {
				sum_private += mbi.RegionSize;	// 25788416
				sum_private0 += mbi.RegionSize;
			}
		} else if (mbi.State == MEM_RESERVE) {
			sum_reserved += mbi.RegionSize;
			if (mbi.Type & MEM_IMAGE) {
				sum_image += mbi.RegionSize;
				sum_image1 += mbi.RegionSize;
			}
			if (mbi.Type & MEM_MAPPED) {
				sum_mapped += mbi.RegionSize;
				sum_mapped1 += mbi.RegionSize;
			}
			if (mbi.Type & MEM_PRIVATE) {
				sum_private += mbi.RegionSize;
				sum_private1 += mbi.RegionSize;
			}
		} else if (mbi.State == MEM_FREE) {
			sum_free += mbi.RegionSize;
		}
		lastaddr = addr;
		addr = (char*)addr + mbi.RegionSize;
		count++;
	}
	return sum_image;
}

typedef BOOL (__stdcall *tGetProcessMemoryInfo)( 
	HANDLE Process,
	PPROCESS_MEMORY_COUNTERS ppsmemCounters,
	DWORD cb);

// Doesn't work on Win9X
unsigned long getProcessPageFileUsage()
{
	PROCESS_MEMORY_COUNTERS pmc={0};
	HINSTANCE hPsApiDll = LoadLibrary( "psapi.dll" );
	tGetProcessMemoryInfo pGetProcessMemoryInfo = NULL;
	if (hPsApiDll)
	{
		pGetProcessMemoryInfo = (tGetProcessMemoryInfo) GetProcAddress(hPsApiDll, "GetProcessMemoryInfo");
	}
	if (pGetProcessMemoryInfo) {
		pGetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	}
	return pmc.PagefileUsage;
}

void preloadDLLs(int silent)
{
	if (!silent)
	{
		loadstart_printf("Preloading DLLs...");
	}

	// These are dlls that we try to load no matter what

	LoadLibrary("C:\\Program Files\\Google\\Google Desktop Search\\GoogleDesktopNetwork1.dll"); // just in case our other trick didn't work
	
 	LoadLibrary("PhysXCooking.dll");
 	LoadLibrary("PhysXCore.dll");
	LoadLibrary("PhysXLoader.dll");


	// these are just so that the debugger doesn't get confused when you load a dll while debugging
#if 0 // MSVS2005 doesn't have the bug that requires this.
	if (IsDebuggerPresent()) {
		LoadLibrary("uxtheme.dll");
		LoadLibrary("version.dll");
		LoadLibrary("msctfime.ime");
		LoadLibrary("ole32.dll");
		LoadLibrary("comctl32.dll");
		LoadLibrary("winmm.dll");
		LoadLibrary("comdlg32.dll");
		LoadLibrary("shlwapi.dll");
		LoadLibrary("shell32.dll");
		LoadLibrary("Syncor11.dll");
		LoadLibrary("DbgHelp.dll");
		LoadLibrary("CrashRpt.dll");
		LoadLibrary("hid.dll");
		// check to see if nVidia fixed their DLL yet
		//{ unsigned int fp; SAVE_FP_CONTROL_WORD(fp); assert((fp & 0xffff)==0x007f); }
		//LoadLibrary("nvoglnt.dll");
		//{ unsigned int fp; SAVE_FP_CONTROL_WORD(fp); assert((fp & 0xffff)==0x007f); }
		//SET_FP_CONTROL_WORD_DEFAULT
		LoadLibrary("mcd32.dll");
		LoadLibrary("setupapi.dll");
		LoadLibrary("wintrust.dll");
		LoadLibrary("crypt32.dll");
		LoadLibrary("msasn1.dll");
		LoadLibrary("imagehlp.dll");
		LoadLibrary("ntmarta.dll");
		LoadLibrary("wldap32.dll");
		LoadLibrary("samlib.dll");
		LoadLibrary("mscms.dll");
		LoadLibrary("winspool.drv");
		LoadLibrary("icm32.dll");
		LoadLibrary("opengl32.dll");
		LoadLibrary("msvcrt.dll");
		LoadLibrary("advapi32.dll");
		LoadLibrary("rpcrt4.dll");
		LoadLibrary("gdi32.dll");
		LoadLibrary("user32.dll");
		LoadLibrary("glu32.dll");
		LoadLibrary("ddraw.dll");
		LoadLibrary("dciman32.dll");
		LoadLibrary("dsound.dll");
		LoadLibrary("dinput8.dll");
		LoadLibrary("ws2_32.dll");
		LoadLibrary("ws2help.dll");
		LoadLibrary("imm32.dll");
		LoadLibrary("shimeng.dll");
		LoadLibrary("acgenral.dll");
		LoadLibrary("oleaut32.dll");
		LoadLibrary("msacm32.dll");
		LoadLibrary("userenv.dll");
		LoadLibrary("lpk.dll");
		LoadLibrary("usp10.dll");
		LoadLibrary("psapi.dll");
		LoadLibrary("wsock32.dll");
		LoadLibrary("wdmaud.drv");
		LoadLibrary("msacm32.drv");
		LoadLibrary("midimap.dll");
		LoadLibrary("ksuser.dll");
		LoadLibrary("mswsock.dll");
		LoadLibrary("dnsapi.dll");
		LoadLibrary("winrnr.dll");
		LoadLibrary("rasadhlp.dll");
		LoadLibrary("secur32.dll");
		LoadLibrary("hnetcfg.dll");
		LoadLibrary("wshtcpip.dll");
		LoadLibrary("apphelp.dll");
		LoadLibrary("clbcatq.dll");
		LoadLibrary("comres.dll");
		LoadLibrary("cscui.dll");
		LoadLibrary("cscdll.dll");
		LoadLibrary("browseui.dll");
		LoadLibrary("ntshrui.dll");
		LoadLibrary("atl.dll");
		LoadLibrary("netapi32.dll");
		LoadLibrary("shdocvw.dll");
		LoadLibrary("cryptui.dll");
		LoadLibrary("wininet.dll");
		LoadLibrary("riched20.dll");
		LoadLibrary("mpr.dll");
		LoadLibrary("drprov.dll");
		LoadLibrary("ntlanman.dll");
		LoadLibrary("netui0.dll");
		LoadLibrary("netui1.dll");
		LoadLibrary("netrap.dll");
		LoadLibrary("davclnt.dll");
		LoadLibrary("endpnp.dll");
		LoadLibrary("EBUtil2.dll");
		LoadLibrary("C:\\Program Files\\Logitech\\iTouch\\itchhk.dll");
		LoadLibrary("C:\\Program Files\\Common Files\\Logitech\\Scrolling\\LGMSGHK.DLL");
		LoadLibrary("console.dll");
		LoadLibrary("msctf.dll");
		LoadLibrary("imjp81.ime");
		LoadLibrary("imjp81k.dll");
		LoadLibrary("C:\\WINDOWS\\ime\\imjp8_1\\DICTS\\imjpcd.dic");
	}
#endif

	if (!silent)
	{
		loadend_printf("done.");
	}
}


HANDLE CreateFileMappingSafe( DWORD lpProtect, int size, const char* handleName, int silent)
{
	HANDLE hMapFile = NULL;
	int iNumTriesLeft = 5; // try 5 times

	while ( !hMapFile && iNumTriesLeft > 0)
	{
		hMapFile = CreateFileMapping(NULL, NULL, lpProtect, 0, size, handleName);
		if ( !hMapFile )
		{
			iNumTriesLeft--;
			// wait a second or so, plus some noise
			if (!silent)
				printf("Failed to map file %s, trying again in 1 second. Tries Left = %d\n", handleName, iNumTriesLeft);
			Sleep(1000 + (qrand() % 200));
		}
	}

	if (!hMapFile && !silent) // tried for 5 seconds, no go, so report error
	{
		if (IsUsing64BitWindows())
		{
			TCHAR cBuf[1000];
			char cFullErrorMessage[1000];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, cBuf, 1000, NULL);
			strcpy(cFullErrorMessage, "Failed to map file %s. Windows system error message: ");
			strcat(cFullErrorMessage, cBuf);
			Errorf(cFullErrorMessage, handleName);
		}
		else
		{
			Errorf("Failed to map file %s. Shared memory does not work on 32-bit Windows.", handleName);
		}
	}

	return hMapFile;
}

static void showFileMappingError(const char* handleName)
{
	TCHAR cBuf[1000];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, cBuf, 1000, NULL);
	Errorf("Failed to map file %s. Windows system error message: %s", handleName, cBuf);
}

HANDLE OpenFileMappingSafe(DWORD dwDesiredAccess, bool bInheritHandle, const char* handleName, int silent )
{
	HANDLE hMapFile = NULL;
	int iNumTriesLeft = 5; // try 5 times

	while ( !hMapFile && iNumTriesLeft > 0)
	{
		hMapFile = OpenFileMapping(dwDesiredAccess, bInheritHandle, handleName);
		if ( !hMapFile )
		{
			iNumTriesLeft--;
			// wait a second or so, plus some noise
			if (!silent && isProductionMode())
				printf("Failed to open map of file %s, trying again in 1 second. Tries Left = %d\n", handleName, iNumTriesLeft);
			Sleep(1000 + (qrand() % 200));
		}
	}

	if (!hMapFile && !silent) // tried for 5 seconds, no go, so report error
	{
		showFileMappingError(handleName);
	}

	return hMapFile;
}


LPVOID MapViewOfFileExSafe(HANDLE handle, const char* handleName, void* desiredAddress, int silent)
{
	LPVOID lpMapAddress = NULL;
	int iNumTriesLeft = 5; // try 5 times


	while ( !lpMapAddress && iNumTriesLeft > 0 )
	{
		lpMapAddress = MapViewOfFileEx(handle, // handle to mapping object 
			FILE_MAP_ALL_ACCESS,               // read/write permission 
			0,                                 // Start at 0
			0,                                 // Start at 0
			0,                                 // map entire file
			desiredAddress);				// Base address to map to

		if ( !lpMapAddress )
		{
			iNumTriesLeft--;
			// wait a second or so, plus some noise
			if (!silent && isProductionMode())
				printf("Failed to map view of file %s, trying again in 1 second. Tries Left = %d\n", handleName, iNumTriesLeft);
			Sleep(1000 + (qrand() % 200));
		}
	}

	if (!lpMapAddress && !silent) // tried for 5 seconds, no go, so report error
	{
		showFileMappingError(handleName);
	}

	return lpMapAddress;
}

void trickGoogleDesktopDll(int silent)
{
	HANDLE hMapFile = NULL;
	void* startingAddress = (void*)0xA0000000;
	int size = 0x30000000; // 0xA0... to 0xD0...  768MB
	SOCKET dummySock;
	LPVOID lpMapAddress;

	if (IsUsingWine())		// don't muck with memory map just yet
		return;

	// First, map a large swath of virtual memory, so that dlls (like googledesktop) don't insert themselves
	// where we want to map shared memory

	hMapFile = CreateFileMappingSafe(PAGE_READWRITE, size, "MemMapTrickGoogle", silent);
	if ( !hMapFile )
		return;

	lpMapAddress = MapViewOfFileExSafe(hMapFile, "MemMapTrickGoogle", startingAddress, silent);

	if ( !lpMapAddress )
	{
		CloseHandle(hMapFile);
		return;
	}



	// Now, make winsock load (which brings googledesktop, and possibly other unsavory elements)
	sockStart();
	dummySock = socket(AF_INET,SOCK_DGRAM,0);


	// since the address space above is mapped, the dlls must go elsewhere or perish, so we have effectively
	// reserverd that address space for our shared memory sytems


	// Just in case, let's preload the other dlls here too
	preloadDLLs(silent);

	// clean up
	closesocket(dummySock);
	UnmapViewOfFile(lpMapAddress);
	CloseHandle(hMapFile);
}

#else

#include "sysutil.h"
#include "utils.h"
#include <stdio.h>
#include <process.h>
#include <assert.h>
#include "fileutil.h"
#include "error.h"
#include "fpmacros.h"
#include "mathutil.h"
#include "sock.h"

char* getExecutableName(){

	return "GameXenon"; //Figure out how to actually do this
}

char *getExecutableDir(char *buf)
{
	strcpy_unsafe(buf, "game:\\");
	return buf;
}

void disableRtlHeapChecking(void *heap)
{

}

#endif
