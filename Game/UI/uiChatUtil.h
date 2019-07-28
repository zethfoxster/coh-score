#ifndef UICHATUTIL_H__
#define UICHATUTIL_H__

#include "uiScrollBar.h"
#include "ArrayOld.h"
#include "uiTabControl.h"
#include "chatSettings.h"
#include "chatdb.h"
#include "smf_util.h"



typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct ChatFilter ChatFilter;
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
	ChatFilter **	filters;		// all filters that this channel belongs to
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

/**********************************************************************func*
* ChatFilter
*
*/

typedef struct ChatLine
{
	char *pchText;
	SMFBlock *pBlock;
	int bParsed;
	int ht;
} ChatLine;



typedef struct chatQueue
{
	int     size;
	ChatLine **ppLines;
} chatQueue;


typedef struct ChatFilter
{	
	char *			name;

	// channel info
	ChatChannel **	channels;			// all channels displayed in this filter
	int	systemChannels[SYSTEM_CHANNEL_BITFIELD_SIZE]; // bit flags for which standard channels this filter recieves

	struct {
		ChannelType		type;
		ChatChannel *	user;
		int				system;			// set to sys channel's enum value (ex INFO_NEARBY_COM)
	} defaultChannel;


	char **			pendingChannels;		// stores list of channel names that we're planning on adding to this filter
	char *			pendingDefaultChannel;	// stores name of default (user) channel (if applicable)

	// text buffers
	chatQueue		chatQ;
	int				reformatText;
	
	// scrollbar settings
	int				scrollOffset;		// remembers where scrollbar was for each window
	int				docHt;

	// notifications
	bool			hasNewMsg;			// set to TRUE when new message added, allows tab to be highlighted

}ChatFilter;

extern ChatFilter** gChatFilters;	// global list of all current filters
extern ChatFilter* gActiveChatFilter;	// the active filter gets most system messages & also text input from user
void SelectActiveChatFilter(ChatFilter * filter);



ChatFilter * ChatFilterCreate(const char * name);
void ChatFilterDestroy(ChatFilter * filter);
void ChatFilterAddChannel(ChatFilter * filter, ChatChannel * channel);
void ChatFilterRemoveChannel(ChatFilter * filter, ChatChannel * channel);
void ChatFilterRemoveAllChannels(ChatFilter * filter, bool leaveUnassigned);
bool ChatFilterHasChannel(ChatFilter * filter, ChatChannel * channel);
void ChatFilterRename(ChatFilter * filter, const char * name);
void ChatFilterClear(ChatFilter * filter);		// empty out text contents
void ChooseDefaultActiveFilter(ChatFilter * noVisibleChoice);

// these functions allow channels to be assigned to filters before they actually arrive (since there's a delay between sending & confirmation)
void ChatFilterPrepareAllForUpdate(int full_update);
void ChatFilterAddPendingChannel(ChatFilter * filter, char * channelName, bool isDefault);	
void ChatFilterAssignPendingChannel(ChatChannel * addChannel, char * removeName);


/**********************************************************************func*
* ChatWindow
*
*/


typedef struct ChatWindow
{
//	ChatFilter * activeFilter;
	uiTabControl *	topTabControl;	// stores all filters for top pane or single pane
	uiTabControl *	botTabControl;	// filters for bottom pane
	ScrollBar		topsb;
	ScrollBar		botsb;
	ContextMenu *	menu;			// popup menu
	float			divider;		// paned divider offest
	int				last_mode;
	int				idx;			// self-referencing idx into gChatWindows array
	int				windefIdx;		// index of window in winDefs array
	int				contextPane;	// so context menu knows which tab control to deal with

	int minimizedOffset;		// (PRIMARY ONLY) height that must be added back to primary box when unhiding it. If '==0', chat window is not minimized

}ChatWindow;


extern int gActiveChatWindowIdx;

ChatWindow * GetChatWindow(int idx);
ChatWindow * GetChatWindowFromWindefIndex(int idx);

void ChatWindowInit(int windowIdx);
void ChatWindowAddFilter(ChatWindow * cw, ChatFilter * filter, int bottom);
void ChatWindowRemoveFilter(ChatWindow * cw, ChatFilter * filter);
void ChatWindowRemoveAllFilters(ChatWindow * cw);
void ChatWindowUpdateFilter(ChatFilter * filter);
int  ChatWindowHasFilter(ChatWindow * cw, ChatFilter * filter);
int AnyChatWindowHasFilter();
void ChatWindowSelectFilter(ChatWindow * cw, ChatFilter * filter);
ChatFilter * GetChatFilter(const char * filterName);
ChatWindow * GetFilterParentWindow(ChatFilter * filter);
void chatSendAllTabsToTop(int winid);

// commands
void chatFilterToggle();
void chatFilterOpen(const char * filterName, int windowIdx, int pane);
void chatFilterClose(const char * filterName);
void chatFilterSelect(char * filterName);
void chatFilterCycle(int windowIdx, bool backward);
void chatFilterGlobalCycle(bool backward);
void addChannelSlashCmd(char * cmd, char * channelName, char * option, ChatFilter * filter);
void openChatOptionsMenu(int windowIdx);
void joinChannelDlgHandler(void * data);
void joinChannelDlgHandlerNoEntry(void * data);
void createChannelDlgHandler(void * data);
void createChannelDlgHandlerNoEntry(void * data);
void addFilter(ChatWindow * window);

extern char tmpChannelName[MAX_CHANNELNAME];


// conversion functions to go between chat window indices (0-4) and windef indices (based on windef enum)
// Special functions are necessary since the windef indices are not continuous (due to legacy values)
//int ChatWindowIdxToWindefIdx(int chatWindowIdx);
int WindefToChatWindowIdx(int windefIdx);


// Chat Handle Dialogs...
void displayChatHandleDialog(void * foo);
void changeChatHandleAgainDialog(char * badHandle);
void changeChatHandleSucceeded();

ChatLine* ChatLine_Create(void);
void ChatLine_Destroy( ChatLine *hItem );


#endif /* #ifndef UICHAT_H__ */

/* End of File */
