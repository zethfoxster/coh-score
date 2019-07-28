#ifndef ENTAIBEHAVIORALIAS_H
#define ENTAIBEHAVIORALIAS_H

typedef struct AIBParsedString	AIBParsedString;

void aiBehaviorLoadAliases(void);
AIBParsedString* aiBehaviorGetAlias(char* str);

#endif