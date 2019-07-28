#ifndef UICHATUTIL_H__
#define UICHATUTIL_H__

#include "uiScrollBar.h"
#include "ArrayOld.h"
#include "uiTabControl.h"
#include "chatSettings.h"
#include "chatdb.h"



typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct ChatWindow ChatWindow;
typedef struct ContextMenu ContextMenu;

/**********************************************************************func*
* ChatUser & Chat Channel
*
*/

typedef struct ChatUser
{
	char *	handle;
	int		flags;
	int		db_id;
//	int		lastMsgTime;  

}ChatUser;


ChatUser * ChatUserCreate(char * handle, int db_id, int flags);
void ChatUserDestroy(ChatUser * user);
void ChatUserRename(ChatUser * user, char * newHandle);


typedef struct ChatChannel
{
	char *			name;
	ChatUser **		users;
	int				mode;
	int				color;
	int				color2;
	int				flags;
	int				idx;		// used to cache position in gChatChannels array (for saving channel info)
	bool			isReserved;		// indicates that a slot in the channel has been saved for the user, but they are not an official member yet
	bool			isLeaving;		// indicates that the channel is going queued up to leave if we hit OK in the chat tab options menu
	ChatUser *		me;

}ChatChannel;


ChatChannel *	ChatChannelCreate(const char * name, int reserved);
void			ChatChannelDestroy(ChatChannel * channel);
ChatChannel *	GetChannel(const char * name);
ChatChannel *	GetReservedChannel(const char * name);
void			ChatChannelCloseAndDestroy(ChatChannel * channel);

ChatUser *		GetChannelUser(ChatChannel * channel, char * handle);
ChatUser *		GetChannelUserByDbId(ChatChannel * channel, int db_id);
ChatUser * 		ChatChannelUpdateUser(ChatChannel * channel, char * handle, int db_id, int flags);
void			ChatChannelRemoveUser(ChatChannel * channel, char * handle);

extern ChatChannel ** gChatChannels;
extern ChatChannel ** gReservedChatChannels;

extern char tmpChannelName[MAX_CHANNELNAME];


// Chat Handle Dialogs...
void displayChatHandleDialog(void * foo);
void changeChatHandleAgainDialog(char * badHandle);
void changeChatHandleSucceeded();


#endif /* #ifndef UICHAT_H__ */

/* End of File */
