#include "aiStruct.h"

#include "earray.h"
#include "MemoryPool.h"
#include "stashtable.h"
#include "stdtypes.h"
#include "StringCache.h"

typedef struct AIMessage
{
	const char* tag;
	U32* times;
}AIMessage;

MP_DEFINE(AIMessage);

void aiMessageReceive(Entity* e, AIVarsBase* aibase, const char* tag)
{
	AIMessage* msg;

	if(!aibase->messages)
		aibase->messages = stashTableCreateWithStringKeys(4, StashDefault);
	if(!stashFindPointer(aibase->messages, tag, &msg))
	{
		MP_CREATE(AIMessage, 16);
		msg = MP_ALLOC(AIMessage);
		msg->tag = allocAddString(tag);
		stashAddPointer(aibase->messages, tag, msg, false);
	}

	eaiPush(&msg->times, 0 /*TODO: ABS_TIME*/);
}

int aiMessageCheck(Entity* e, AIVarsBase* aibase, const char* tag, AIMessage** retMsg)
{
	AIMessage* msg;

	stashFindPointer(aibase->messages, tag, &msg);

	if(retMsg)
		*retMsg = msg;

	if(msg)
		return eaiSize(&msg->times);
	else
		return 0;
}

void aiMessageDestroyHelper(void* message)
{
	AIMessage* msg = (AIMessage*) message;

	MP_FREE(AIMessage, msg);
}

void aiMessageDestroyAll(Entity* e, AIVarsBase* aibase)
{
	stashTableDestroyEx(aibase->messages, NULL, aiMessageDestroyHelper);
}