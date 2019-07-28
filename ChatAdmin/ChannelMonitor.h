#ifndef __CHANNEL_MONITOR_H__
#define __CHANNEL_MONITOR_H__

#include "wininclude.h"
#include "MessageView.h"
#include "ChannelList.h"
#include "UserList.h"
#include "VirtualListView.h"

typedef struct CAChanMon CAChanMon;

typedef struct{

	CAUser * user;
	int flags;

	CAChanMon * monitor;	// pointer back to channel monitor struct that contains this member

}CAChanMember;

typedef struct CAChanMon{

	CAChannel	*channel;
	CAUser		**members; // for fast/easy lookups

	MessageView	*mv;
	VListView	*memberList;

	char		memberFilter[100];
	int			memberFilterLength;

	BOOL		isVisible;

	char		*lastOperator;	// these two cache the current strings used for buttons, to reduce computation time as buttons are checked for updates frequently (hackish :)
	char		*lastSilence;

	HWND hDlg;

}CAChanMon;

void ChanMonChannelMsg(char * channelName, char * msg, int type); 

void ChanMonTabCreate(CAChannel * channel);
void ChanMonTabRemove(CAChannel * channel);

void ChanMonJoin(CAChannel * channel, CAUser * user, int flags);
void ChanMonLeave(CAChannel * channel, CAUser * user);
void ChanMonUpdateCount(CAChanMon * cm);
void ChanMonUpdateUser();

void ChanMonShowTimestamps(bool show);

void ChanMonReset();

CAChanMon * GetChanMon(CAChannel * channel);

#endif //__CHANNEL_MONITOR_H__