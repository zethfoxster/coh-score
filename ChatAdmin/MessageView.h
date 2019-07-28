#ifndef __MESSAGE_VIEW_H__
#define __MESSAGE_VIEW_H__

#include "wininclude.h"

typedef struct{
	char * line;
	U32	time;
}TextEntry;

typedef struct{

	HWND hWndView;

	TextEntry **msgs;

	int maxMsgs;

	bool showTime;

}MessageView;



MessageView * MessageViewCreate(HWND hWndView, int maxMsgs);
void MessageViewDestroy(MessageView * mv);

//void MessageViewProcessCmd(MessageView * mv, WPARAM wParam, LPARAM lParam);

void MessageViewEmpty(MessageView * mv);
void MessageViewAdd(MessageView * mv, char * msg, int type);
void MessageViewShowTime(MessageView * mv, bool showTime);

#endif //__MESSAGE_VIEW_H__