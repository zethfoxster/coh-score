#ifndef __CHANNEL_LIST_H__
#define __CHANNEL_LIST_H__

#include "StashTable.h"
#include "ChatAdmin.h"

typedef struct CAUser CAUser;

typedef struct CAChannelMember
{
	CAUser	*user;
	int		flags;

}CAChannelMember;

typedef struct CAChannel
{
	char			*name;
	char			*filterName;	// contains only alphanumeric for faster filtering
	int				flags;
	CAChannelMember	**members;

}CAChannel;


CAChannel * CAChannelAdd(char * name, bool sortAndFilter);
void CAChannelRemove(CAChannel * channel);
void CAChannelRemoveAll();
void CAChannelRename(CAChannel * channel, char * newName);

void CAChannelAddMember(CAChannel * channel, CAUser * user, int flags);
void CAChannelRemoveMember(CAChannel * channel, CAUser * user);


CAChannel * GetChannelByName(char * name);

void ChannelListInit();
void ChannelListUpdate();
void ChannelListUpdateCount();
void ChannelListFilter();

void ChannelListReset();

LRESULT CALLBACK DlgChannelListProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

extern StashTable gNameToChannelHash;


#endif //__CHANNEL_LIST_H__