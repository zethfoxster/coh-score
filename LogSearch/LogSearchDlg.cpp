// LogSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LogSearch.h"
#include "LogSearchDlg.h"
#include "Redirect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLogSearchDlg dialog




CLogSearchDlg::CLogSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogSearchDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLogSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DIRECTORY, ceDirectory);
	DDX_Control(pDX, IDC_FILESPEC, ceEdit);
	DDX_Control(pDX, IDC_OUTPUT, ceOutput);
	DDX_Control(pDX, IDC_SEARCH, ceSearch);
	DDX_Control(pDX, IDC_START, ceStart);
	DDX_Control(pDX, IDC_STOP, ceEnd);
	DDX_Control(pDX, IDC_EXENAME, ceExeName);
	DDX_Control(pDX, IDC_CASESENSITIVE, cbCaseSensitive);
	DDX_Control(pDX, IDC_WILDCARDS, cbWildcards);
	DDX_Control(pDX, IDC_STDOUT, ceStdOut);
	DDX_Control(pDX, IDC_PERCENTDONE, cpPercentDone);
	DDX_Control(pDX, IDC_STARTSEARCH, cbSearch);
	DDX_Control(pDX, IDC_STOPSEARCH, cbStop);
}

BEGIN_MESSAGE_MAP(CLogSearchDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_STARTSEARCH, &CLogSearchDlg::OnBnClickedStartSearch)
	ON_BN_CLICKED(IDC_STOPSEARCH, &CLogSearchDlg::OnBnClickedStopSearch)
	ON_BN_CLICKED(IDC_SAVESEARCH, &CLogSearchDlg::OnBnClickedSavesearch)
END_MESSAGE_MAP()


// CLogSearchDlg message handlers
void CLogSearchDlg::getConfigValue(int id, CString name, CString defaultStr, CWinApp *app)
{
	CString strValue;
	CEdit *ce = reinterpret_cast<CEdit *>(GetDlgItem(id));
	strValue = app->GetProfileString("config", name, defaultStr);
	ce->SetWindowText(strValue);
}


BOOL CLogSearchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CWinApp *pApp = AfxGetApp();
	getConfigValue(IDC_EXENAME,		"exename",		"logParser3_64.exe",				pApp);
	getConfigValue(IDC_DIRECTORY,	"directory",	"\\\\nccohbkup01\\cohlogs",			pApp);
	getConfigValue(IDC_FILESPEC,	"fileSpec",		"entity_2011-##-##-##-##-##.log.gz", pApp);
	getConfigValue(IDC_OUTPUT,		"outfile",		"mySearchOutput.txt",				pApp);
	getConfigValue(IDC_SEARCH,		"searchStr",	"All Hail the Television",			pApp);
	getConfigValue(IDC_START,		"startTime",	"110401 00:00:00",					 pApp);
	getConfigValue(IDC_STOP,		"endTime",		"110501 00:00:00",					 pApp);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLogSearchDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLogSearchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CRedirect *app = 0;
void CLogSearchDlg::OnBnClickedStartSearch()
{
	if(app)
		return;

	cbSearch.EnableWindow(false);
	cbStop.EnableWindow(true);

	CString directory;
	ceDirectory.GetWindowText(directory);
	ceDirectory.UpdateData(true);
	CString fileSpec;
	ceEdit.GetWindowText(fileSpec);
	ceEdit.UpdateData(true);
	CString outputFile;
	ceOutput.GetWindowText(outputFile);
	ceOutput.UpdateData(true);
	CString searchTerm;
	ceSearch.GetWindowText(searchTerm);
	ceSearch.UpdateData(true);
	CString startDate;
	ceStart.GetWindowText(startDate);
	ceStart.UpdateData(true);
	CString endDate;
	ceEnd.GetWindowText(endDate);
	ceEnd.UpdateData(true);
	CString exeLoc;
	ceExeName.GetWindowText(exeLoc);
	ceExeName.UpdateData(true);
	

	CString commandLine;
	CLogSearchApp::makeCommandLineParams(commandLine, exeLoc, directory, fileSpec, outputFile, searchTerm, startDate, endDate, 6, 250, 6, 10000, cbCaseSensitive.GetCheck()==1, cbWildcards.GetCheck()==1);
	//theApp.procId = theApp.startProcess(exeLoc, commandLine);
	app= new CRedirect(commandLine, &ceStdOut, &cpPercentDone);
	app->Run();
	delete app;
	app = 0;
	cbSearch.EnableWindow(true);
	cbStop.EnableWindow(false);
	//ceDirectory.
}

void CLogSearchDlg::OnBnClickedStopSearch()
{
	//theApp.killProcess(theApp.procId);
	if(app)
		app->Stop();

	cbSearch.EnableWindow(true);
	cbStop.EnableWindow(false);
}

void CLogSearchDlg::OnOK(void)
{
	OnBnClickedStartSearch();
}

void CLogSearchDlg::setConfigValue(int id, CString name, CWinApp *app)
{
	CString strValue;
	CEdit *ce = reinterpret_cast<CEdit *>(GetDlgItem(id));
	ce->GetWindowText(strValue);
	ce->UpdateData(true);
	app->WriteProfileString("config", name, strValue);
}

void CLogSearchDlg::saveSearch(void)
{
	CWinApp *pApp = AfxGetApp();
	setConfigValue(IDC_EXENAME, "exename", pApp);
	setConfigValue(IDC_DIRECTORY, "directory", pApp);
	setConfigValue(IDC_FILESPEC, "fileSpec", pApp);
	setConfigValue(IDC_OUTPUT, "outfile", pApp);
	setConfigValue(IDC_SEARCH, "searchStr", pApp);
	setConfigValue(IDC_START, "startTime", pApp);
	setConfigValue(IDC_STOP, "endTime", pApp);
}

void CLogSearchDlg::OnBnClickedSavesearch()
{
	saveSearch();
}
