/* uiFocus.h
 *	A silly mechanism for focus arbitration.
 *
 *	Objects that wish to participate in focus arbitration must call uiGetFocus() when
 *	they gain focus.  When checking for input, the object must call uiInFocus() to determine
 *	if it still has focus.  The object should proceed with input processing only if it still 
 *	has focus.
 */
#ifndef UIFOCUS_H
#define UIFOCUS_H


typedef int (*LostFocusFunction)(void* focusOwner, void* focusRequester);
// Returns:
//	0 - refuse to give up focus
//  1 - agree to give up focus

int uiFocusRefuseAllRequests(void*, void*);


int uiGetFocusDbg(void* obj, bool isEditable, char* file, char* function, int line);
int uiGetFocusExDbg(void* obj, bool isEditable, LostFocusFunction onLostFocus, char* file, char* function, int line);
#define uiGetFocus(obj, isEditable) uiGetFocusDbg(obj, isEditable, __FILE__, __FUNCTION__, __LINE__)
#define uiGetFocusEx(obj, isEditable, onLostFocus) uiGetFocusExDbg(obj, isEditable, onLostFocus, __FILE__, __FUNCTION__, __LINE__)
// Requests focus on behalf of the specified object.
// Returns:
//	0 - focus denied.
//	1 - focus granted.

void uiReturnFocus(void* obj);
void uiReturnAllFocus(void);

int uiInFocus(void* obj);
// Tests to see if the said object is in focus.

int uiNoOneHasFocus(void);

bool uiFocusIsEditable(void);
// Tests if the item in focus (if there is one) said it was editable when it
// was given focus.

#endif