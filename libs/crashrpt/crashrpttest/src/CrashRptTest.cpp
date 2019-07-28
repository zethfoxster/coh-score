// CrashRptTest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CrashRptTest.h"
#include "CrashRptTestDlg.h"
#include <direct.h>

#ifndef _CRASH_RPT_
#include "../../src/crashrpt.h"

#ifdef _M_X64
#pragma comment(lib, "../../lib/crashrpt64")
#else
#pragma comment(lib, "../../lib/crashrpt")
#endif
#endif // _CRASH_RPT_

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCrashRptTestApp

BEGIN_MESSAGE_MAP(CCrashRptTestApp, CWinApp)
	//{{AFX_MSG_MAP(CCrashRptTestApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrashRptTestApp construction

CCrashRptTestApp::CCrashRptTestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCrashRptTestApp object

CCrashRptTestApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCrashRptTestApp initialization


BOOL WINAPI CrashCallback(LPVOID lpvState)
{
   CrashRptAddFile(lpvState, "dummy.log", "Dummy Log File");
   CrashRptAddFile(lpvState, "dummy.ini", "Dummy INI File");

   return TRUE;
}

BOOL CCrashRptTestApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

   m_lpvState = CrashRptInstall(CrashCallback);
   //CrashRptSetFTPConduit(m_lpvState, _T("localhost"), _T("jonathan"), _T("none"));
   CrashRptSetFTPConduit(m_lpvState, _T("errors.coh.com"), _T("errors"), _T("kicks"));
   

   CCrashRptTestDlg dlg;
   dlg.DoModal();

   return FALSE;
}

void CCrashRptTestApp::generateErrorReport()
{
   //CrashRptGenerateErrorReport(m_lpvState, NULL);

   __try {
	   int zero=0;
	   int crashme = 1/zero;
		//RaiseException(0, 0, 0, 0);
   } __except(CrashRptGenerateErrorReport2(m_lpvState, GetExceptionInformation(), "dummy", "dummy", "dummy", "dummy", "dummy", "duq\"t\tr\rn\nmmy  OUTDATED VIDEO CARD DRIVER", "UnknownOpenGLFile", "UnknownLauncherFile")){}
}
