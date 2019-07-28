#ifndef UI_CHAT_TAB_H__
#define UI_CHAT_TAB_H__

#include "stdtypes.h"

typedef struct ChatFilter ChatFilter;
typedef struct ChatChannel ChatChannel;
typedef struct ChatWindow ChatWindow;

void		chatTabOpen(ChatFilter * filter, ChatWindow * newWindow);
void		chatTabClose(bool save);
void		chatTabCleanup();
int			chatTabWindow();
void		channelDeleteNofication(ChatChannel * channel);
bool		chatOptionsAvailable();
ChatFilter* currentChatOptionsFilter();

#endif   //  UI_CHAT_TAB_H__