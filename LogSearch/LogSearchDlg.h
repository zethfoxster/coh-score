// LogSearchDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CLogSearchDlg dialog
class CLogSearchDlg : public CDialog
{
// Construction
public:
	CLogSearchDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LOGSEARCH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	void saveSearch();
	void getConfigValue(int id, CString name, CString defaultStr, CWinApp *app);
	void setConfigValue(int id, CString name, CWinApp *app);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOK(void);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStartSearch();
	afx_msg void OnBnClickedStopSearch();
	afx_msg void OnBnClickedSavesearch();
	CEdit ceDirectory;
	CEdit ceEdit;
	CEdit ceOutput;
	CEdit ceSearch;
	CEdit ceStart;
	CEdit ceEnd;
	CEdit ceExeName;
	CEdit ceStdOut;
	CButton cbCaseSensitive;
	CButton cbWildcards;
	CProgressCtrl cpPercentDone;
	CButton cbSearch;
	CButton cbStop;
};
