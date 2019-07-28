// Reporter.cpp: implementation of the CReporter class.
//
//////////////////////////////////////////////////////////////////////
#include "GlobalAuth.h"
#include "util.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CReporter reporter;

CReporter::CReporter()
{
	m_font = 0;
    GetLocalTime(&m_st);
	m_UserCount = 0;
	m_SocketCount = 0;
	m_SocketEXObjectCount = 0;
	m_InGameUser = 0;
}

CReporter::~CReporter()
{
}

void CReporter::SetWnd(HWND wnd)
{
	m_wnd = wnd;
}

HWND CReporter::GetWnd()
{
    return m_wnd;
}

void CReporter::Reset()
{
    GetLocalTime(&m_st);
    InvalidateRect(m_wnd, NULL, FALSE);
}

void CReporter::Redraw()
{
	PAINTSTRUCT paint;
	HDC hdc = BeginPaint(m_wnd, &paint);
	if (m_font == 0) {
		m_font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
		SelectObject(hdc, m_font);
		GetClientRect(m_wnd, &m_clientRect);
		m_brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		GetTextExtentPoint32(hdc, "H", 1, &m_fontSize);
	} else {
		SelectObject(hdc, m_font);
	}
	FillRect(hdc, &m_clientRect, m_brush);
	char str[128];
#ifndef _USE_ACCEPTEX
	sprintf(str, " user %-6d  | socket %-4d | connects %-3d  | RunningThread %-3d ", 
      m_UserCount, m_SocketCount, g_Accepts, g_nRunningThread );
#else
	sprintf(str, " user %-6d  | socket %-4d | AcceptCall %-3d  | RunningThread %-3d ", 
      m_UserCount, m_SocketCount, g_AcceptExThread, g_nRunningThread );
#endif
	SetTextColor(hdc, RGB(0, 0, 0));
	TextOut(hdc, 0, 0, str, (int)strlen(str));

	EndPaint(m_wnd, &paint);
}

void CReporter::Resize(int width, int height)
{
	m_clientRect.right = width;
	m_clientRect.bottom = height;
	InvalidateRect(m_wnd, NULL, FALSE);
}
