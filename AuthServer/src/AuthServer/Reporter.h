// Reporter.h: interface for the CReporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REPORTER_H__72F4DC5F_B703_4027_BCAB_45FEE863782B__INCLUDED_)
#define AFX_REPORTER_H__72F4DC5F_B703_4027_BCAB_45FEE863782B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CReporter  
{
public:
	CReporter();
	virtual ~CReporter();

    long m_InGameUser;
	long m_UserCount;
    long m_SocketCount;
	long m_SocketEXObjectCount;
	void SetWnd(HWND wnd);
    HWND GetWnd();
	void Redraw();
	void Resize(int width, int height);
    void Reset();

protected:
	HWND m_wnd;
	HFONT m_font;
	RECT m_clientRect;
	HBRUSH m_brush;
	SIZE m_fontSize;
    SYSTEMTIME m_st;
};

extern CReporter reporter;
#endif // !defined(AFX_REPORTER_H__72F4DC5F_B703_4027_BCAB_45FEE863782B__INCLUDED_)