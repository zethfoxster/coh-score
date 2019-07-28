#include "uiFocus.h"
#include "stdtypes.h"
#include "cmdcommon.h"

typedef struct 
{
	void* obj;
	bool isEditable;
	LostFocusFunction onLostFocus;

	char* file;			// Who asked for focus?
	char* function;
	int line;
} UIFocusRequest;

UIFocusRequest activeRequest;


static void uiFocusRecord(UIFocusRequest* req, void* obj, bool isEditable, LostFocusFunction func)
{
	if(!req)
		return;
	req->obj = obj;
	req->isEditable = isEditable;
	req->onLostFocus = func;
}

static void uiFocusRecordDbgInfo(UIFocusRequest* req, char* file, char* function, int line)
{
	if(!req)
		return;
	req->file = file;
	req->function = function;
	req->line = line;
}

int uiFocusRefuseAllRequests(void* focusOwner, void* focusRequester)
{
	return 0;
}

/* Function uiGetFocusXXX
 *	These functions allows an object to request for input focus.
 */
int uiGetFocusDbg(void* obj, bool isEditable, char* file, char* function, int line)
{
	return uiGetFocusExDbg(obj, isEditable, NULL, file, function, line);
}

int uiGetFocusExDbg(void* obj, bool isEditable, LostFocusFunction onLostFocus, char* file, char* function, int line)
{
	int canHaveFocus = 1;

	// Does the requesting party already have focus?
	if(obj == activeRequest.obj)
	{
		// They may just be updating the editability of the item.
		activeRequest.isEditable = isEditable;
		return 1;
	}

	// If the current focus owner has a "Lost Focus" callback, call it now
	// to determine if caller can have focus.
	if(activeRequest.onLostFocus)
	{
		canHaveFocus = activeRequest.onLostFocus(activeRequest.obj, obj);
	}

	// Deny failed focus requests.
	if(!canHaveFocus)
		return 0;
	
	uiFocusRecord(&activeRequest, obj, isEditable, onLostFocus);
	uiFocusRecordDbgInfo(&activeRequest, file, function, line);
	return 1;
}

void uiReturnFocus(void* obj)
{
	if(obj == activeRequest.obj)
	{
		if(activeRequest.onLostFocus)
			activeRequest.onLostFocus(activeRequest.obj, NULL);
		memset(&activeRequest, 0, sizeof(UIFocusRequest));
	}
}

void uiReturnAllFocus(void)
{
	if(activeRequest.obj)
	{
		if(activeRequest.onLostFocus)
			activeRequest.onLostFocus(activeRequest.obj, NULL);
		memset(&activeRequest, 0, sizeof(UIFocusRequest));
	}
}

/* Function uiInFocus()
 *	This function can be used to test if an object current has focus or not.
 */
int uiInFocus(void* obj)
{
	if(activeRequest.obj == obj)
		return 1;
	else
		return 0;
}

int uiNoOneHasFocus(void)
{
	return uiInFocus(NULL);
}

bool uiFocusIsEditable(void)
{
	bool retval = FALSE;

	if(activeRequest.obj)
	{
		retval = activeRequest.isEditable;
	}

	return retval;
}