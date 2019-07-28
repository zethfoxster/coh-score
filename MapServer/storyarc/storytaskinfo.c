/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "storytaskinfo.h"
#include "storyarcprivate.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"

MP_DEFINE(StoryTaskInfo);
StoryTaskInfo* storyTaskInfoAlloc()
{
	MP_CREATE(StoryTaskInfo, 10); 
	return MP_ALLOC(StoryTaskInfo);
}

void storyTaskInfoFree(StoryTaskInfo* info) 
{ 
	MP_FREE(StoryTaskInfo, info); 
}

void storyTaskInfoDestroy(StoryTaskInfo* info)
{
	storyTaskInfoFree(info);
}
