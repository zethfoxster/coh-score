/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIEMOTE_H__
#define UIEMOTE_H__

#ifndef ENTVARUPDATE_H__
#include "entVarUpdate.h" // For INFO_BOX_MSGS
#endif

typedef struct Entity Entity;
typedef struct ChatFilter ChatFilter;

void ParseChatText(char *pchOrig, INFO_BOX_MSGS msg, float duration, int svr_idx, Vec2 position);

char * stripPlayerHTML(char *pchOrig, int profanity_too );
char * dealWithPercents( char *instring );

void LoadFilth(void);
int IsFilth(const char *pch, int iLen);

#endif /* #ifndef UIEMOTE_H__ */

/* End of File */

