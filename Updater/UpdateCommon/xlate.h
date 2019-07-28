#ifndef _XLATE_H
#define _XLATE_H

#include "MessageStore.h"

extern MessageStore	*g_msg_store;
int xlatePrint(char *buffer, int bufferLength, const char *msg);
int xlatePrintf(char* outputBuffer, int bufferLength, const char* messageID, ...);
char *xlateLoad(void *hInstance);
int xlateGetLoadedLocale();
char *xlateQuick(char *msg);
char *xlateQuickUTF8(char *msg);

// returns temporary buffer
wchar_t *xlateToUnicode(char *mbcs);

// uses passed in buffer
wchar_t *xlateToUnicodeBuffer(const char *mbcs, wchar_t *buf, size_t buflen);

#endif
