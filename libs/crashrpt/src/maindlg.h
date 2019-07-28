///////////////////////////////////////////////////////////////////////////////
//
//  Module: maindlg.h
//
//    Desc: Main crash report dialog, responsible for gathering additional
//          user information and allowing user to examine crash report.
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MAINDLG_H_
#define _MAINDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Utility.h"
#include "DeadLink.h"
#include "detaildlg.h"
#include "aboutdlg.h"

//
// RTF load callback
//
DWORD CALLBACK LoadRTFString(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CString *sText = (CString*)dwCookie;
	LONG lLen = sText->GetLength();

	for (*pcb = 0; *pcb < cb && *pcb < lLen; (*pcb)++)
	{  
		pbBuff[*pcb] = sText->GetAt(*pcb);
	}

	return 0;
}


////////////////////////////// Class Definitions /////////////////////////////

// ===========================================================================
// CMainDlg
// 
// See the module comment at top of file.
//
class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	CString     m_sEmail;         // Email: From
	CString     m_sDescription;   // Email: Body
	char		m_sModuleName[1024];
	const char	*m_sMessage;
	CDeadLink   m_link;           // Dead link
	CDeadLink   m_support_link;   // Dead link
	TStrStrMap  *m_pUDFiles;      // Files <name,desc>

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_LINK, OnLinkClick)
		COMMAND_ID_HANDLER(IDC_SUPPORT_LINK, OnSupportLinkClick)
		COMMAND_ID_HANDLER(IDOK, OnSend)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()


	//-----------------------------------------------------------------------------
	// CMainDlg
	//
	// Loads RichEditCtrl library
	//
	CMainDlg() 
	{
		LoadLibrary(CRichEditCtrl::GetLibraryName());
	};


	//-----------------------------------------------------------------------------
	// OnInitDialog
	//
	// 
	//
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		//
		// Set title using app name
		//
		SetWindowText(_T("City of Heroes"));

		//
		// Use app icon
		//
		CStatic icon;
		HICON hIcon;
		icon.Attach(GetDlgItem(IDI_APPICON));
		{
			hIcon = ::LoadIcon((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME));
			if(!hIcon)
				hIcon = ::LoadIcon((HINSTANCE)GetModuleHandle("CrashRpt.dll"), MAKEINTRESOURCE(IDI_CRYPTIC));
		}
		icon.SetIcon(hIcon);
		icon.Detach();

		SetIcon(hIcon);

		//
		// Set failure heading
		//
		EDITSTREAM es;
		es.pfnCallback = LoadRTFString;

		CString sText;
		if (_stricmp(m_sModuleName, "CityOfHeroes.exe")==0) {
			sText.Format(IDS_HEADER, _T("City of Heroes"), "");
		}else if (_stricmp(m_sModuleName, "CostumeCreator.exe")==0) {
			sText.Format(IDS_HEADER, _T("City of Heroes - Costume Creator"), "");
		} else {
			// External crash!
			char buf[4096];
			sprintf_s(buf, sizeof(buf), "  The crash occurred in %s", m_sModuleName);
			sText.Format(IDS_HEADER, _T("City of Heroes"), buf);
			// Check to see if they additionally have old drivers
			if (m_sMessage && strstr(m_sMessage, "OUTDATED VIDEO CARD DRIVER")) {
				char vendorURL[1024] = "www.nvidia.com, www.ati.com, or www.intel.com";
				if (_strnicmp(m_sModuleName, "ati", 3)==0) {
					strcpy_s(vendorURL, sizeof(vendorURL), "www.ati.com");
				} else if (_strnicmp(m_sModuleName, "nv", 2)==0) {
					strcpy_s(vendorURL, sizeof(vendorURL), "www.nvidia.com");
				} else if (_strnicmp(m_sModuleName, "vt", 2)==0) {
					strcpy_s(vendorURL, sizeof(vendorURL), "www.viaarena.com");
				} else if (_strnicmp(m_sModuleName, "sis", 3)==0) {
					strcpy_s(vendorURL, sizeof(vendorURL), "www.sis.com");
				} else if (_strnicmp(m_sModuleName, "s3g", 3)==0) {
					strcpy_s(vendorURL, sizeof(vendorURL), "www.s3graphics.com");
				}
				sprintf_s(buf, sizeof(buf), "\
City of Heroes has encountered a problem and needs to close.  The crash appears to have \
occurred in a third party DLL or device driver (specifically \"%s\").  Additionally, it \
has been detected that your computer is using older video card drivers that may have had \
known issues with City of Heroes.  Updating your video card drivers may increase the stability of City of Heroes. \n\
\n\
Go to %s to update your driver. In some cases, \
such as laptops, you may need to get the latest drivers from your computer manufacturer.",
					m_sModuleName,
					vendorURL);
				MessageBox(buf, "Outdated video card drivers detected", MB_OK | MB_SYSTEMMODAL);
			}
		}
		es.dwCookie = (DWORD)&sText;

		CRichEditCtrl re;
		re.Attach(GetDlgItem(IDC_HEADING_TEXT));
		re.StreamIn(SF_RTF, es);
		re.Detach();

		//
		// Hook dead link
		//
		m_link.SubclassWindow(GetDlgItem(IDC_LINK));   
		m_support_link.SubclassWindow(GetDlgItem(IDC_SUPPORT_LINK));   

		return TRUE;
	}


	//-----------------------------------------------------------------------------
	// OnLinkClick
	//
	// Display details dialog instead of opening URL
	//
	LRESULT OnLinkClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CDetailDlg dlg;
		dlg.m_pUDFiles = m_pUDFiles;
		dlg.DoModal();
		return 0;
	}


	//-----------------------------------------------------------------------------
	// OnSupportLinkClick
	//
	// Display the support website
	//
	LRESULT OnSupportLinkClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShellExecute(NULL,NULL,"https://support.cityofheroes.com/",NULL,NULL,0);
		return 0;
	}


	//-----------------------------------------------------------------------------
	// OnSend
	//
	// Send handler, validates email address entered, if one, and returns.
	//
	LRESULT OnSend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		HWND     hWndEmail	= GetDlgItem(IDC_EMAIL);
		HWND     hWndDesc	= GetDlgItem(IDC_DESCRIPTION);
		int      nEmailLen	= ::GetWindowTextLength(hWndEmail)+1;
		int      nDescLen	= ::GetWindowTextLength(hWndDesc)+1;

		LPTSTR lpStr = m_sEmail.GetBufferSetLength(nEmailLen);
		::GetWindowText(hWndEmail, lpStr, nEmailLen);
		m_sEmail.ReleaseBuffer();

		lpStr = m_sDescription.GetBufferSetLength(nDescLen);
		::GetWindowText(hWndDesc, lpStr, nDescLen);
		m_sDescription.ReleaseBuffer();

		//
		// If an email address was entered, verify that
		// it [1] contains a @ and [2] the last . comes
		// after the @.
		//
		if (m_sEmail.GetLength() &&
			(m_sEmail.Find(_T('@')) < 0 ||
			m_sEmail.ReverseFind(_T('.')) < m_sEmail.Find(_T('@'))))
		{
			// alert user
			TCHAR szBuf[256];
			::LoadString(_Module.GetResourceInstance(), IDS_INVALID_EMAIL, szBuf, 255);
			MessageBox(szBuf, CUtility::getAppName(), MB_OK);
			// select email
			::SetFocus(hWndEmail);
		}
		else
		{
			EndDialog(wID);
		}

		return 0;
	}

	//-----------------------------------------------------------------------------
	// OnCancel
	//
	// 
	//
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif	// #ifndef _MAINDLG_H_
