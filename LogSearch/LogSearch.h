// LogSearch.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLogSearchApp:
// See LogSearch.cpp for the implementation of this class
//

class CLogSearchApp : public CWinApp
{
public:
	CLogSearchApp();
	DWORD startProcess(CString exeLocation, CString commandLine);
	void killProcess(DWORD processId);
	static BOOL CALLBACK CloseWindowsByProc(HWND hwnd, LPARAM pid);
	static void makeCommandLineParams(CString &ret,
		CString exeloc,
		CString dir,
		CString file,
		CString out,
		CString search,
		CString start,
		CString stop,
		unsigned int threshold,
		unsigned int maxLine,
		unsigned int nThreads,
		unsigned int bufferSize,
		bool caseSensitive,
		bool useWildcards);

	DWORD  procId;	//eventually replace this with an erray of open procids

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CLogSearchApp theApp;