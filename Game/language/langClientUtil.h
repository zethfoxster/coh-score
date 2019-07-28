#ifndef LANGCLIENTUTIL_H
#define LANGCLIENTUTIL_H

#include "MessageStore.h"
extern MessageStore* cmdMessages;
extern MessageStore* texWordsMessages;
extern MessageStore* loadingTipMessages;

void reloadClientMessageStores(int localeID);



#endif