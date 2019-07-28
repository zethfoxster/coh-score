/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UICONTACTDIALOG_H__
#define UICONTACTDIALOG_H__

#include "contactCommon.h"

typedef struct ContactResponseOption ContactResponseOption;

///////////////////////////////////////////////////////////////////////////////
// client side version of StoryarcAlliance in storyarcprivate.h
typedef enum StoryarcAlliance {
	SA_ALLIANCE_BOTH,
	SA_ALLIANCE_HERO,
	SA_ALLIANCE_VILLAIN,
} StoryarcAlliance;

typedef struct StoryarcFlashbackHeader
{
	char					*name;
	char					*fileID;
	U32						bronzeTime;
	U32						silverTime;
	U32						goldTime;
	int						minLevel;
	int						maxLevel;
	StoryarcAlliance		alliance;
	F32						timeline;
//	char					**completeRequires;
	int						complete;
	int						completeEvaluated;
} StoryarcFlashbackHeader;

extern StoryarcFlashbackHeader **storyarcFlashbackList;
extern int flashbackListRequested;

void uiContactDialogAddTaskDetail(char *pID, char *pText, char *pAccept);

void uiContactAdvanceToChallengeWindow();

void uiContactDialogSetChallengeTimeLimits(U32 bronze, U32 silver, U32 gold);

void cd_SetBasic(CDtype type, char *body, ContactResponseOption ** rep, void ( *fp)( int, void* ), void* voidptr);
void cd_clear(void);
void cd_exit(void);

int contactWindow(void);


#endif /* #ifndef UICONTACTDIALOG_H__ */

/* End of File */

