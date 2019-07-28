///////////////////////////////////////////////////////////////////////////////
//
//  Module: CrashHandler.cpp
//
//    Desc: See CrashHandler.h
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrashHandler.h"
#include "zlibcpp.h"
#include "excprpt.h"
#include "process.h"
#include "mailmsg.h"
#include <stdio.h>
#include "maindlg.h"
#include "dx8Diagnostics.h"

// global app module
CAppModule _Module;

// maps crash objects to processes
CSimpleMap<int, CCrashHandler*> _crashStateMap;

// unhandled exception callback set with SetUnhandledExceptionFilter()
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
	CCrashHandler* handler= _crashStateMap.Lookup(_getpid());
	handler->GenerateErrorReport(pExInfo, "UnknownAuth", "UnknownEntity", "UnknownShard", "UnknownShardTime", "0.0", "CustomUnhandledExceptionFilter", "UnknownGLFileName", "UnknownLauncherLogFile", GetCurrentThreadId());
	//handler->HandleException(pExInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

CCrashHandler::CCrashHandler(LPGETLOGFILE lpfn /*=NULL*/)
{
	// wtl initialization stuff...
	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	hRes = _Module.Init(NULL, GetModuleHandle("CrashRpt.dll"));
	ATLASSERT(SUCCEEDED(hRes));

	::DefWindowProc(NULL, 0, 0, 0L);

	// initialize member data
	m_lpfnCallback	= NULL;
	m_oldFilter		= NULL;
	m_reportConduit	= NULL;
	mainDlg = new CMainDlg();

	// save user supplied callback
	if (lpfn)
		m_lpfnCallback = lpfn;

	// add this filter in the exception callback chain
	m_oldFilter = SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

	// attach this handler with this process
	m_pid = _getpid();
	_crashStateMap.Add(m_pid, this);

}

CCrashHandler::~CCrashHandler()
{
	delete mainDlg;

	// reset exception callback
	if (m_oldFilter)
		SetUnhandledExceptionFilter(m_oldFilter);

	_crashStateMap.Remove(m_pid);

	// uninitialize
	_Module.Term();
	::CoUninitialize();

}

void CCrashHandler::AddFile(LPCTSTR lpFile, LPCTSTR lpDesc)
{
	// make sure the file exist
	HANDLE hFile = ::CreateFile(
		lpFile,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);
	if (hFile)
	{
		// add file to report
		m_files[lpFile] = lpDesc;
		::CloseHandle(hFile);
	}
}

static bool isEmtpyString(const char* str)
{
	const char* cursor = str;
	if(!str)
		return false;

	while(*cursor)
	{
		if(!isspace((unsigned char)*cursor))
			return false;
		cursor++;
	}

	return true;
}
void CCrashHandler::GenerateErrorReport(PEXCEPTION_POINTERS pExInfo, const char *szAuth, const char *szEntity, const char *szShard, const char *szShardTime, const char *szVersion, const char *szMessage, const char *glReportFileName, const char *launcherLogFileName, DWORD dwThreadID)
{
	CExceptionReport	rpt(pExInfo, dwThreadID);
	CZLib				zlib;
	CString				sTempFileName = CUtility::getTempFileName();
	unsigned int		i;
	bool				sendReport = false;
	static int			generatingReport = 0;

	// don't allow recursion
	if (generatingReport)
		return;		// we're not doing anything smart, just not showing the second crash dialog
	generatingReport = 1;

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;	
	WSAStartup(wVersionRequested, &wsaData); 

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	// let client add application specific files to report
	if (m_lpfnCallback && !m_lpfnCallback(this))
	{
		::SetCursor(NULL);
		generatingReport = 0;
		return;
	}

	// add crash files to report
	m_files[rpt.getCrashFile()] = CString((LPCTSTR)IDS_CRASH_DUMP);
	m_files[rpt.getCrashLog(szAuth, szEntity, szShard, szShardTime, szVersion, szMessage)] = CString((LPCTSTR)IDS_CRASH_LOG);


	// pack up the DX diagnostics info (done after user selects OK, as this takes a few seconds)
	char filename[] = "dxdiag.html";
	if (dx8DiagnosticsStartup() == S_OK)
	{
		dx8Diagnostics *diagnostics;
		dx8DiagnosticsCreate(&diagnostics, "City of Heroes Crash Report");
		HRESULT r = dx8DiagnosticsWrite(diagnostics, filename);
		dx8DiagnosticsDestroy(&diagnostics);
		dx8DiagnosticsShutdown();
		if(r == S_OK) {
			m_files[filename] = CString((LPCTSTR)IDS_DXDIAG_FILE);
		}
	}

	char openglFilename[] = "opengl.txt";
	if (glReportFileName && CopyFile(glReportFileName, openglFilename, FALSE) != 0)
	{
		m_files[openglFilename] = CString((LPCTSTR)IDS_OPENGL_FILE);
	}

	char newLauncherLogFilename[] = "NClauncherlog.txt";
	if (launcherLogFileName && CopyFile(launcherLogFileName, newLauncherLogFilename, FALSE) != 0)
	{
		m_files[newLauncherLogFilename] = CString((LPCTSTR)IDS_LAUNCHERLOG_FILE);
	}

	// add symbol files to report
 	for (i = 0; i < (UINT)rpt.getNumSymbolFiles(); i++)
		m_files[(LPCTSTR)rpt.getSymbolFile(i)] = 
		CString((LPCTSTR)IDS_SYMBOL_FILE);

	// zip the report
	if (!zlib.Open(sTempFileName))
	{
		::SetCursor(NULL);
		generatingReport = 0;
		return;
	}

	// add report files to zip
	TStrStrMap::iterator cur = m_files.begin();
	for (i = 0; i < m_files.size(); i++, cur++)
		zlib.AddFile((*cur).first);


	::SetCursor(NULL);

	// display main dialog
	mainDlg->m_pUDFiles = &m_files;
	strcpy_s(mainDlg->m_sModuleName, sizeof(mainDlg->m_sModuleName), rpt.m_sModuleOfCrash);
	mainDlg->m_sMessage = szMessage;
	if (IDOK == mainDlg->DoModal())
	{
		// Pack up crash description into a file.
		if(!isEmtpyString(mainDlg->m_sDescription))
		{
			char* filename = "CrashDescription.txt";
			FILE* crashDesc;
			fopen_s(&crashDesc, filename, "wt");
			if(crashDesc)
			{
				fwrite(mainDlg->m_sDescription, 1, mainDlg->m_sDescription.GetLength(), crashDesc);
				fclose(crashDesc);
				zlib.AddFile(filename);
			}
		}

		sendReport = true;
	}

	zlib.Close();
	
	if(sendReport)
	{
		int save_it = GetAsyncKeyState(VK_SHIFT) & 0x8000000;
		if (m_reportConduit && !m_reportConduit->send(sTempFileName) || save_it)
		{
			SaveReport(rpt, sTempFileName);
		}
	}
	DeleteFile(sTempFileName);
	generatingReport = 0;
}

void CCrashHandler::AbortErrorReport()
{
	mainDlg->EndDialog(0);
}

BOOL CCrashHandler::SaveReport(CExceptionReport&, LPCTSTR lpcszFile)
{
	// let user more zipped report
	return (CopyFile(lpcszFile, CUtility::getSaveFileName(), TRUE));
}

void CCrashHandler::HandleException(PEXCEPTION_POINTERS pExInfo)
{
	if(this->m_oldFilter)
		this->m_oldFilter(pExInfo);
}
