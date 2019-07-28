
#ifndef COMMONLANGUTIL_H
#define COMMONLANGUTIL_H

typedef struct Entity			Entity;
typedef struct StaticDefineInt	StaticDefineInt;
typedef struct MessageStore		MessageStore;

extern StaticDefineInt ParseGender[];
extern StaticDefineInt ParseStatus[];
extern StaticDefineInt ParsePlayerType[];
extern StaticDefineInt ParsePlayerSubType[];
extern StaticDefineInt ParseAlignment[];
extern StaticDefineInt ParsePraetorianProgress[];
extern StaticDefineInt ParseTeamNames[];
extern StaticDefineInt ParseBodyType[];

#ifdef SERVER
char* printLocalizedEnt(const char *msg, Entity* e, ...);
char* printLocalizedEntFromEntLocale(const char *msg, Entity* e, ...);
#else
char* printLocalizedEnt(const char *msg, ...);
char* printLocalizedEntFromEntLocale(const char *msg, ...);
#endif

void installCustomMessageStoreHandlers(MessageStore* store);

#endif
