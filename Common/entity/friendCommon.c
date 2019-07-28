#include "friendCommon.h"
#include "stdtypes.h"
#include "memcheck.h"

void friendDestroy(Friend *f)
{
	if (!f)
		return;
	SAFE_FREE(f->mapname);
	SAFE_FREE(f->name);
	f->origin = NULL; // Don't free, it's allocated with allocAddString
	f->arch = NULL;
}