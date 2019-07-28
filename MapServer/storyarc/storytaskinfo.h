/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STORYTASKINFO_H
#define STORYTASKINFO_H

#include "stdtypes.h"

typedef struct GenericHashTableImp *GenericHashTable;
typedef struct HashTableImp *HashTable;
typedef struct StoryTaskInfo StoryTaskInfo;

StoryTaskInfo* storyTaskInfoAlloc();
void storyTaskInfoFree(StoryTaskInfo* info);
void storyTaskInfoDestroy(StoryTaskInfo* info);


#endif //STORYTASKINFO_H
