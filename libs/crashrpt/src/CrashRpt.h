///////////////////////////////////////////////////////////////////////////////
//
//  Module: CrashRpt.h
//
//    Desc: Defines the interface for the CrashRpt.DLL.
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _CRASHRPT_H_
#define _CRASHRPT_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <windows.h>
#include "CrashRptConf.h"

#ifdef __cplusplus
extern "C" {
#endif

// Client crash callback
typedef BOOL (CALLBACK *LPGETLOGFILE) (LPVOID lpvState);

//-----------------------------------------------------------------------------
// Install
//    Initializes the library and optionally set the client crash callback and
//    sets up the email details.
//
// Parameters
//    pfn         Client crash callback
//    lpTo        Email address to send crash report
//    lpSubject   Subject line to be used with email
//
// Return Values
//    If the function succeeds, the return value is a pointer to the underlying
//    crash object created.  This state information is required as the first
//    parameter to all other crash report functions.
//
// Remarks
//    Passing NULL for lpTo will disable the email feature and cause the crash 
//    report to be saved to disk.
//
CREXTERN 
LPVOID
CREXPORT
CrashRptInstall(
	IN LPGETLOGFILE pfn OPTIONAL                // client crash callback
);
typedef LPVOID (CREXPORT *pfnCrashRptInstall)(IN LPGETLOGFILE pfn OPTIONAL);

//-----------------------------------------------------------------------------
// Uninstall
//    Uninstalls the unhandled exception filter set up in Install().
//
// Parameters
//    lpState     State information returned from Install()
//
// Return Values
//    void
//
// Remarks
//    This call is optional.  The crash report library will automatically 
//    deinitialize when the library is unloaded.  Call this function to
//    unhook the exception filter manually.
//
CREXTERN 
void
CREXPORT
CrashRptUninstall(
	IN LPVOID lpState                            // State from Install()
);
typedef void (CREXPORT *pfnCrashRptUninstall)(IN LPVOID lpState);

//-----------------------------------------------------------------------------
// AddFile
//    Adds a file to the crash report.
//
// Parameters
//    lpState     State information returned from Install()
//    lpFile      Fully qualified file name
//    lpDesc      Description of file, used by details dialog
//
// Return Values
//    void
//
// Remarks
//    This function can be called anytime after Install() to add one or more
//    files to the generated crash report.
//
CREXTERN 
void
CREXPORT
CrashRptAddFile(
	IN LPVOID lpState,                           // State from Install()
	IN LPCTSTR lpFile,                           // File name
	IN LPCTSTR lpDesc                            // File desc
);
typedef void (CREXPORT *pfnCrashRptAddFile)(IN LPVOID lpState, IN LPCTSTR lpFile, IN LPCTSTR lpDesc);

//-----------------------------------------------------------------------------
// GenerateErrorReport
//    Generates the crash report.
//
// Parameters
//    lpState     State information returned from Install()
//    pExInfo     Pointer to an EXCEPTION_POINTERS structure
//
// Return Values
//    void
//
// Remarks
//    Call this function to manually generate a crash report.
//
CREXTERN 
void 
CREXPORT
CrashRptGenerateErrorReport(
	IN LPVOID lpState,
	IN PEXCEPTION_POINTERS pExInfo
);
typedef void (CREXPORT *pfnCrashRptGenerateErrorReport)(IN LPVOID lpState, IN PEXCEPTION_POINTERS pExInfo);

CREXTERN 
int 
CREXPORT
CrashRptGenerateErrorReport2(
							IN LPVOID lpState,
							IN PEXCEPTION_POINTERS pExInfo,
							IN const char *szAuth,
							IN const char *szEntity,
							IN const char *szShard,
							IN const char *szShardTime,
							IN const char* szVersion,
							IN const char* szMessage,
							IN const char* glReportFileName,
							IN const char* launcherLogFileName
							);
typedef int (CREXPORT *pfnCrashRptGenerateErrorReport2)(IN LPVOID lpState, IN PEXCEPTION_POINTERS pExInfo, IN const char *szAuth, IN const char *szEntity, IN const char *szShard, IN const char *szShardTime, IN const char* szVersion, IN const char* szMessage, IN const char* glReportFileName, IN const char* launcherLogFileName);

CREXTERN 
int 
CREXPORT
CrashRptGenerateErrorReport3(
							 IN LPVOID lpState,
							 IN PEXCEPTION_POINTERS pExInfo,
							 IN const char *szAuth,
							 IN const char *szEntity,
							 IN const char *szShard,
							 IN const char *szShardTime,
							 IN const char* szVersion,
							 IN const char* szMessage,
							 IN const char* glReportFileName,
							 IN const char* launcherLogFileName,
							 IN DWORD dwThreadID
							 );

typedef int (CREXPORT *pfnCrashRptGenerateErrorReport3)(IN LPVOID lpState, IN PEXCEPTION_POINTERS pExInfo, IN const char *szAuth, IN const char *szEntity, IN const char *szShard, IN const char *szShardTime, IN const char* szVersion, IN const char* szMessage, IN const char* glReportFileName, IN const char* launcherLogFileName, DWORD dwThreadID);

CREXTERN 
void 
CREXPORT
CrashRptAbortErrorReport(
	IN LPVOID lpState
);
typedef void (CREXPORT *pfnCrashRptAbortErrorReport)(IN LPVOID lpState);

CREXTERN 
void 
CREXPORT
CrashRptSetFTPConduit(
	IN LPVOID lpState,		// State from Install()
	IN LPCTSTR hostname,    
	IN LPCTSTR username,	
	IN LPCTSTR password	
);
typedef void (CREXPORT *pfnCrashRptSetFTPConduit)(IN LPVOID lpState, IN LPCTSTR hostname, IN LPCTSTR username, IN LPCTSTR password);

#ifdef __cplusplus
}
#endif

#endif
