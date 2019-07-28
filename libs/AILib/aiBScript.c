#include "aiStruct.h"

typedef struct AIBSTransition
{
	int blah;
	const char* transitionName;
}AIBSTransition;

typedef struct AIBSFluffHint
{
	const char* sayText;
	const char* soundCueName;
	const char* FXName;
}AIBSFluffHint;

typedef struct AIBScript
{
	const char* BScriptName;

	struct AIBScript* subBScript;
	const char* behaviorStr;

	AIBSTransition** transitions;

	const AIBSFluffHint* onFirstEntry;
	const AIBSFluffHint* onEntry;
	const AIBSFluffHint* onEveryTick;
}AIBScript;