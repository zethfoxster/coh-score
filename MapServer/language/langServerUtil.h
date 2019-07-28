#ifndef LANGSERVERUTIL_h
#define LANGSERVERUTIL_h

#include <stdarg.h>
#include "MultiMessageStore.h"
typedef struct Entity Entity;
typedef struct ClientLink ClientLink;

const char* clientPrintf( ClientLink * client, const char* str, ...);
char* localizedPrintf(const Entity * e, const char* messageID, ...);
char* localizedPrintfVA(const Entity * e, const char* messageID, va_list arg);

int isLocalizeable(const Entity *e, const char *messageID);

int translateFlag(const Entity *e);

//char *cmdTranslated(char *str);
//char *cmdHelpTranslated(char *str);

extern MultiMessageStore* svrMenuMessages;
extern MessageStore*	cmdMessages;
void loadServerMessageStores(void);
void setCurrentMenuMessages(void);

#endif
