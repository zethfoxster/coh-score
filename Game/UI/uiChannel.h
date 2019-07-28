#ifndef UI_CHANNEL_H__
#define UI_CHANNEL_H__

#include "stdtypes.h"
typedef struct ChatUser ChatUser;
typedef struct ChatFilter ChatFilter;
typedef struct ChatChannel ChatChannel;
typedef struct ContextMenu ContextMenu;

int channelWindow(float x, float y, float z, float wd, float ht, float sc, int color, int bcolor, void * data);
void resetChannelWindow();
void notifyRemoveUser(ChatUser * user);
void notifyChannelDestroy(ChatChannel *channel);
void notifyRebuildChannelWindow();	
void notifyRebuildChannelUserList(ChatChannel * channel);

// Target context menu
ContextMenu * getChannelInviteMenu();
int canInviteTargetToAnyChannel(void * foo);
bool channelInviteAllowed(ChatChannel * channel);

extern bool channelListRebuildRequired;

void changeChannelMode_Long(char * channel, char * mode);
void changeUserMode_Long(char * handle, char * mode);
bool isOperator(int flags);
int isSilenced(ChatUser * user);

#endif  // UI_CHANNEL_H__