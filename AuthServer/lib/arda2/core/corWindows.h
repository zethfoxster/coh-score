/*****************************************************************************
	created:	2001/07/31
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Wrapper around including <windows.h> and friends
*****************************************************************************/

#ifndef   INCLUDED_corWindows
#define   INCLUDED_corWindows
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Note: must be included after corPlatform.h!

// for Windows only
#if CORE_SYSTEM_WINAPI

// make windows.h have strict types of handles
#if !defined STRICT
#define STRICT 1
#endif

// trim down the amount of stuff windows.h includes using standard #defines
#define WIN32_LEAN_AND_MEAN
#define NOMCX             // Modem Configuration Extensions
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOCRYPT           // WinCrypt
#define NOCOMM            // COMM driver routines
#define NOATOM            // Atom Manager routines
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
//#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOKANJI           // Kanji support stuff.

/*
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
#define NONLS             // All NLS defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501     // Windows XP
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410   // Windows 98
#endif

#pragma warning( push )

// suppress non-standard extension warnings from the windows headers
//#pragma warning(disable: 4201)

// suppress function not inlined warning
//#pragma warning(disable: 4710)

#if CORE_SYSTEM_WIN32
#include "windows.h"
#undef RegisterClass
#undef GetObject
#elif CORE_SYSTEM_WIN64
#include "windows.h"
#undef RegisterClass
#undef GetObject
#elif CORE_SYSTEM_XENON
// Xenon include
#include "xtl.h" 
#else
#error "unknown windows system"
#endif

// restore default warning levels
#pragma warning( pop )


// WCHAR-based one is Win32 only
void corOutputDebugString(const WCHAR* format, ...);

#else // CORE_SYSTEM_WIN32

// Compatibility definitions for non-Win32 platforms.


#define TRUE 1
#define FALSE 0

typedef long BOOL;
typedef unsigned long DWORD;

typedef struct tagSIZE
{
	long cx;
	long cy;
} SIZE;

typedef struct tagPOINT
{
	long x;
	long y;
} POINT;

typedef struct tagRECT
{
	long left;
	long top;
	long right;
	long bottom;
} RECT;

#endif



// Define a debugging output function that takes varargs (as opposed to OutputDebugString).

void corOutputDebugString(const char* format, ...);

#endif // INCLUDED_corWindows
