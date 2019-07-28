#include "uiChatUtil.h"
#include "StashTable.h"
#include "EArray.h"
#include "assert.h"
#include "wdwbase.h"
#include "clientcomm.h"
#include "uiConsole.h"
#include "uiChatOptions.h"
#include "ttFontUtil.h"
#include "entplayer.h"
#include "uiWindows.h"
#include "uiChat.h"
#include "uiChannel.h"
#include "uiDialog.h"
#include "cmdgame.h"
#include "sprite_text.h"
#include "uiUtil.h"
#include "mathutil.h"
#include "chatdb.h"
#include "uiContextMenu.h"
#include "MessageStoreUtil.h"
#include "player.h"
#include "entity.h"

static ContextMenu *tabContextMenu = 0;

char tmpChannelName[MAX_CHANNELNAME];

/***********************************************************************
* ChatUser	(member of a chat channel)
*
*/

ChatUser * ChatUserCreate(char * handle, int db_id, int flags)
{
	ChatUser * user = calloc(sizeof(ChatUser),1);

	user->handle = strdup(handle);
	user->flags = flags;
	user->db_id = db_id;

	return user;
}

void ChatUserRename(ChatUser * user, char * newHandle)
{
	user->handle = realloc(user->handle, (strlen(newHandle)+1));
	strcpy(user->handle, newHandle);
}

void ChatUserDestroy(ChatUser * user)
{
	free(user->handle);
	free(user);
}


/***********************************************************************
* ChatChannel
*
*/



ChatChannel ** gChatChannels = NULL;
ChatChannel ** gReservedChatChannels = NULL;

ChatChannel * ChatChannelCreate(const char * name, int reserved)
{
	ChatChannel * channel = calloc(1, sizeof(ChatChannel));
	Entity *e = playerPtr();
	ChatSettings *settings;
	int i;

	if( e && e->pl )
		settings = &e->pl->chat_settings;
	else
		return 0;

	channel->name = strdup(name);
	channel->isReserved = reserved;

	eaCreate(&channel->users);

	if(reserved)
		eaPush(&gReservedChatChannels, channel);
	else
		eaPush(&gChatChannels, channel);

	// look for saved color
	for(i = 0; i < MAX_CHAT_CHANNELS; i++ )
	{
		if( settings->channels[i].name && stricmp(settings->channels[i].name, name) == 0)
		{
			channel->color = settings->channels[i].color1;
			channel->color2 = settings->channels[i].color2;
		}
	}

	return channel;
}

void ChatChannelDestroy(ChatChannel * channel)
{
	// now get rid of it

	assert(GetChannel(channel->name) || GetReservedChannel(channel->name));

	eaRemove(&gChatChannels, eaFind(&gChatChannels, channel));
	eaFindAndRemove(&gReservedChatChannels, channel);

	eaDestroyEx(&channel->users, ChatUserDestroy);
	
	free(channel->name);
	free(channel);
}

ChatChannel * GetChannel(const char * name)
{
	int i, size = eaSize(&gChatChannels);
	for(i=0;i<size;i++)
	{
		ChatChannel * cc = gChatChannels[i];
		if(cc && !stricmp(cc->name, name))
			return cc;
	}
	return 0;
}

ChatChannel * GetReservedChannel(const char * name)
{
	int i, size = eaSize(&gReservedChatChannels);
	for(i=0;i<size;i++)
	{
		ChatChannel * cc = gReservedChatChannels[i];
		if(cc && !stricmp(cc->name, name))
			return cc;
	}
	return 0;
}

// remove chat channel from all filters (need to make copy since ChatFilterRemoveChannel() modifieds the channel's filter list)
void ChatChannelCloseAndDestroy(ChatChannel * channel)
{
	ChatChannelDestroy(channel);

}



ChatUser * GetChannelUser(ChatChannel * channel, char * handle)
{
	int i;

	if(!channel || !handle)
		return 0;

	for(i=0;i<eaSize(&channel->users);i++)
	{
		if(!stricmp(channel->users[i]->handle, handle))
		{
			return channel->users[i];
		}
	}
	return 0;
}


ChatUser * GetChannelUserByDbId(ChatChannel * channel, int db_id)
{
	int i;

	if(!channel || db_id <= 0)
		return 0;

	for(i=0;i<eaSize(&channel->users);i++)
	{
		if(channel->users[i]->db_id == db_id)
			return channel->users[i];
	}
	return 0;
}

// called to add user or to update existing user
ChatUser * ChatChannelUpdateUser(ChatChannel * channel, char * handle, int db_id, int flags)
{
	ChatUser * user = GetChannelUser(channel, handle);
	if(!user)
	{
		user = ChatUserCreate(handle, db_id, flags);
		eaPush(&channel->users, user);
	}
	else
	{
		user->db_id = db_id;	// this should not change, but just in case...
		user->flags = flags;
	}

	return user;
}

void ChatChannelRemoveUser(ChatChannel * channel, char * handle)
{
	ChatUser * user = GetChannelUser(channel, handle);
	if(user)
	{		
		eaRemove(&channel->users, eaFind(&channel->users, user));
		ChatUserDestroy(user);
	}
}


// -----------------------------------------------------------------------
// Chat Handle Dialogs (used to display & change)
// -----------------------------------------------------------------------
#include "chatClient.h"

static void changeChatHandleDialog(void * data)
{
	char buf[987];
	printf("%s\n> ", textStd("ChangeChatHandleDialog"));
	gets(buf);
	if (buf[0] && buf[0] != '\n' && buf[0] != '\r') {
		char buf2[1000];
		sprintf(buf2, "changehandle %s", buf);
		cmdParse(buf2);
	}
}

void displayChatHandleDialog(void * foo)
{
	printf("Current chat handle: %s\n", game_state.chatHandle);
}

void changeChatHandleAgainDialog(char * badHandle)
{
	changeChatHandleDialog(badHandle);
}

void changeChatHandleSucceeded()
{
	displayChatHandleDialog(0);
}
