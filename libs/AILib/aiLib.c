#include "aiLib.h"
#include "aiBehaviorInterface.h"
#include "aiStruct.h"


void aiLibStartup()
{
	aiBehaviorRebuildLookupTable();
}

void aiTick(BaseEntity* e, AIVarsBase* aibase)
{
	aiBehaviorProcess((Entity*)e, aibase, &aibase->behaviors);
}