/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UICHAT_H__
#define UICHAT_H__

#include "entVarUpdate.h"
#include "chatSettings.h"

#define DEFAULT_BUBBLE_TIME			2.0 //Time before bringing up the next bubble
#define DEFAULT_BUBBLE_DURATION		8.0 //Time this bubble lasts on screen

typedef struct Entity Entity;
typedef struct TTFontRenderParams TTFontRenderParams;
typedef struct ChatFilter ChatFilter;
typedef struct ChatChannel ChatChannel;

void GetTextColorForType(int idx, int rgba[4]);
void GetTextStyleForType(int type, TTFontRenderParams *params);

int okToInput(int scanCode);
void uiChatStartChatting(char *pch, bool bGobbleFirst);
void uiChatSendToCurrentChannel(char *pch);

int chat_Resizers( float x, float y, float z, float wd, float ht );

void addSystemChatMsg( char *txt, INFO_BOX_MSGS msg, int svr_idx );  // You should almost always use this
void addSystemChatMsgEx( char *txt, INFO_BOX_MSGS msg, float duration, int svr_idx, Vec2 position ); 

void addGlobalChatMsg( char *pchChannel, char *pchPlayer, char * pchText, INFO_BOX_MSGS msg ); // one funnel for the global chat
void addChatMsgDirect(char * s, int type); // Skips the channel pre-pend step
void addChatMsgsFloat( char *txt, INFO_BOX_MSGS msg, float duration, int svr_idx, Vec2 position); // If this message has floating potentail
void addChatMsgToQueue( char *txt, INFO_BOX_MSGS msg, ChatFilter * filter ); // Final step in adding a chat message

void InfoBoxDbg(INFO_BOX_MSGS msg_type, char *txt, ...);
void InfoBox(INFO_BOX_MSGS msg_type, char *txt, ...);
void InfoBoxVA(INFO_BOX_MSGS msg_type, char *txt, int debug, va_list arg);

int chat_mode();
void chat_exit();

void chatInitiateTell(void * data);
void chatInitiateTellToTeamLeader(void *data);
void chatInitiateTellToLeagueLeader(void *data);
void chatInitiateReply();
void setChatEditText(char *pch);

int chatWindow(void);
int chatFilterWindow(void);
void clearAllChat();

void GetQuickChatZone(int *pX, int *pY, int *pW, int *pH);

void DebugCopyChatToClipboard(char * tabName);
void chatCleanup();

void chatChannelSet(char * ch );
void chatChannelCycle();
void hidePrimaryChat();
void resetPrimaryChatSize();

void debugChatWindowDump();

void sendSelectedChatTabsToServer();
void sendChatSettingsToServer();
void receiveChatSettingsFromServer(Packet * pak, Entity * e);

void saveChatSettings( char * pchFile );
void loadChatSettings( char * pchFile );

void SetLastPrivate(char * name, bool isHandle);



void initChatUIOnce();	// global startup code

bool canAddChannel();
bool canAddFilter();

void joinChatChannel(char * name);
void leaveChatChannel(char * name);
void chatChannelInviteDenied(char *channel, char *inviter);

char * chatcm_userChannelText(void * data);

extern int gSentRespecMsg;
extern int gChatLogonTimer;

enum 
{
	INFO_CHAT_TEXT = INFO_TOTAL+1,
	INFO_PROFILE_TEXT,
	INFO_HELP_TEXT,
	INFO_STD_TEXT, 
	INFO_SCRAPPER,
	INFO_CHANNEL_TEXT,
	INFO_CHANNEL_TEXT1,
	INFO_CHANNEL_TEXT2,
	INFO_CHANNEL_TEXT3,
	INFO_CHANNEL_TEXT4,
};

void chatLink( char *pchName );
void chatLinkGlobal( char *pchName, char * pchChannel );
void chatLinkChannel( char *pchName );
void channelSetDefaultColor(ChatChannel *channel);
void setHeroVillainEventChannel(int isHero);

#endif /* #ifndef UICHAT_H__ */

/* End of File */
