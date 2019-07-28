#ifndef ANIMLIST_H
#define ANIMLIST_H

#include "stdtypes.h"
#include "StashTable.h"

typedef struct Entity Entity;
typedef struct SeqType SeqType;

typedef struct AnimBitList{
	char* listName;
	char** listContents;
	char* fileName;
}AnimBitList;

void loadAnimLists(void);
void alForEachAnimList(StashElementProcessor proc);
char * alGetFxNameFromAnimList( Entity * e, char * varName );
bool alSetFavoriteWeaponAnimListOnEntity( Entity * e, const char * animListName );
bool alSetAIAnimListOnEntity( Entity * e, const char * animListName );
char * entTypeVarReplacement( char * varName, const SeqType * type );
void alClientAssignAnimList( Entity * e, const char * newAnimList ); 
AnimBitList* alGetList(char* name);

int alDebugGetAnimListCount(void);
char* alDebugGetAnimListStr(int idx);

#endif
