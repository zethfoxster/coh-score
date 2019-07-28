/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIIME_H
#define UIIME_H

#include <wininclude.h>
#include "stdtypes.h"

typedef struct UIEdit UIEdit;

bool uiIME_MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

// --------------------
// display

//void uiIME_SetState(bool active);
void uiIME_RenderOnUIEdit(UIEdit *edit);
void uiIME_OnLoseFocus(UIEdit *edit);
void uiIME_OnFocus(UIEdit *edit);

#endif //UIIME_H
