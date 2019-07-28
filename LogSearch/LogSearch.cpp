// LogSearch.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "LogSearch.h"
#include "LogSearchDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLogSearchApp

BEGIN_MESSAGE_MAP(CLogSearchApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CLogSearchApp construction

CLogSearchApp::CLogSearchApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CLogSearchApp object

CLogSearchApp theApp;


// CLogSearchApp initialization

BOOL CLogSearchApp::InitInstance()
{
	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("CoH CS Tools"));

	CLogSearchDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

DWORD CLogSearchApp::startProcess(CString exeLocation, CString commandLine)
{
	BOOL bWorked;
	STARTUPINFO suInfo;
	PROCESS_INFORMATION procInfo;
	memset (&suInfo, 0, sizeof(suInfo));
	suInfo.cb = sizeof(suInfo);

	bWorked = ::CreateProcess(exeLocation,
		commandLine.GetBuffer(),      // can also be NULL
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&suInfo,
		&procInfo);

	if (procInfo.dwThreadId = NULL)
	{
		//MessageBox("nope");
	}

	return procInfo.dwProcessId;
}

BOOL CALLBACK CLogSearchApp::CloseWindowsByProc(HWND hwnd, LPARAM pid)
{
	DWORD wndPid;
	CString Title;
	GetWindowThreadProcessId(hwnd, &wndPid);
	CWnd::FromHandle( hwnd )->GetWindowText(Title);
	if ( wndPid == (DWORD)pid && Title.GetLength() != 0)
	{
		//  Please kindly close this process
		::PostMessage(hwnd, WM_CLOSE, 0, 0);
		return false;
	}
	else
	{
		// Keep enumerating
		return true;
	}
}

void CLogSearchApp::killProcess(DWORD pid)
{
	HANDLE ps = OpenProcess( SYNCHRONIZE|PROCESS_TERMINATE, 
		FALSE, pid);
	// processPid = procInfo.dwProcessId;

	// This function enumerates all widows,
	// using the EnumWindowsProc callback
	// function, passing the PID of the process
	// you started earlier.
	EnumWindows(CloseWindowsByProc, pid);

	CloseHandle(ps) ;
}

void CLogSearchApp::makeCommandLineParams(CString &ret,
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
										  bool useWildcards)
{
	ret.Format(_T("%s -directory \"%s\" -file \"%s\" -output \"%s\" \
			   -search \"%s\" -start \"%s\" -end \"%s\" \
			   -threshold %d -nthreads %d -maxline %d -buffer %d %s %s"),
			   exeloc, dir, file, out, search, start, stop, threshold, 
			   nThreads, maxLine, bufferSize,
			   caseSensitive?"-caseSensitive":"", useWildcards?"-wildCards":"");
}
