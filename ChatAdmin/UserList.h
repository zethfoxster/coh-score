#ifndef __USER_LIST_H__
#define __USER_LIST_H__

#include "StashTable.h"
#include "ChatAdmin.h"

typedef struct CAUser{

	char	*handle;
	char	*filterHandle;	// used for fast filtering, contains no spaces, underscores, all lower case
	bool	online;
	U32		silenced;

}CAUser;

CAUser * CAUserAdd(char * handle, bool online, U32 silenced, bool sortAndFilter);
void CAUserRemove(CAUser * user);
void CAUserRemoveAll();
void CAUserRename(CAUser * user, char * handle);
void CAUserUpdate(CAUser * user);

CAUser * GetUserByHandle(char * handle);


void UserListInit();

void UserListReset();

LRESULT CALLBACK DlgUserListProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

void UserListFilter();

void UserListUpdate();	// notify user list that it should update all items being displayed
void UserListUpdateCount();

void UserListStatusUpdate(char * handle, char * auth_id, char * shard, char * silencedMins, char *args[], int count, char *channels[], int channel_count);

void onAcceptRenameUser(char * oldHandle, char * newHandle);

extern StashTable gHandleToUserHash;
extern CAUser ** gAllUsers;



#endif //__USER_LIST_H__