#ifndef _OSDEPENDENT_H
#define _OSDEPENDENT_H

C_DECLARATIONS_BEGIN

void InitOSInfo(void);
int IsUsingWin2kOrXp(void);
int IsUsingWin9x(void);
int IsUsingXbox(void);
int IsUsingVistaOrLater(void);
int IsUsingWin7orLater(void);
int IsUsing64BitWindows(void);
int IsUsingCider();
int IsUsingWine();

// If we need to fabricate command line, like on xbox, do it with this function
void FillCommandLine(int *argc, char ***argv);

// Call the correct SetDlgItemText function based on the user's OS
#define SET_DLG_ITEM_TEXT( hDlg, dlgItemID, str ) \
	IsUsingWin2kOrXp() ? SetDlgItemTextW(hDlg, dlgItemID, xlateToUnicode(str)) : SetDlgItemTextA(hDlg, dlgItemID, str) 

// Call the correct SetWindowText function based on the user's OS
#define SET_WINDOW_TEXT( hWnd, str ) \
	IsUsingWin2kOrXp() ? SetWindowTextW(hWnd, xlateToUnicode(str)) : SetWindowTextA(hWnd, str) 

// Call the correct GetWindowText function based on the user's OS
// pass in a wide character buffer 
#define GET_WINDOW_TEXT( hWnd, wc_buf, buf_size ) \
	IsUsingWin2kOrXp() ? GetWindowTextW(hWnd, wc_buf, buf_size) : GetWindowText(hWnd, (char*)wc_buf, buf_size >> 1)

// Call the correct DrawText function based on the user's OS
// pass in a wide character buffer 
#define DRAW_TEXT( hDC, wc_str, rect, format ) \
	IsUsingWin2kOrXp() ? DrawTextW(hDC, wc_str, wcslen(wc_str), rect, format) : \
	DrawText(hDC, (char*)wc_str, strlen((char*)wc_str), rect, format)

C_DECLARATIONS_END

#endif