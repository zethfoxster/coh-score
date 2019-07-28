#include "buildstatus.h"
#include "earray.h"
#include "wininclude.h"

static int buildstatus_initialized = 0;
static BuildStatus *buildStatus = NULL;
CRITICAL_SECTION buildstatus_cs;

void buildStatusInit()
{
	if(!buildstatus_initialized)
	{
		buildstatus_initialized = 1;
		InitializeCriticalSection(&buildstatus_cs);
	}
}

void buildStatusAlloc()
{
	buildStatus = (BuildStatus*)malloc(sizeof(BuildStatus));
	buildStatus->build_failed = 0;
	buildStatus->stepStatus = NULL;
	eaCreate(&buildStatus->stepStatus);
}

void buildEnterCritSection()
{
	EnterCriticalSection(&buildstatus_cs);
}

void buildLeaveCritSection()
{
	LeaveCriticalSection(&buildstatus_cs);
}

void buildStatusUpdateStatus(int build_failed)
{
	buildEnterCritSection();

	{
		if ( !buildStatus )
			buildStatusAlloc();
		buildStatus->build_failed = build_failed;
	}

	buildLeaveCritSection();
}

void buildStatusUpdateStep(char *step, float time_in_seconds, int in_progress, int error)
{
	buildEnterCritSection();

	{
		int i;
		BuildStepStatus *bs;

		if ( !buildStatus )
			buildStatusAlloc();

		// TODO: linear search for now, use a hash if it becomes a problem
		for ( i = 0; i < eaSize(&buildStatus->stepStatus); ++i )
		{
			bs = buildStatus->stepStatus[i];
			if ( !stricmp(step, bs->step) )
			{
				bs->time_in_seconds = time_in_seconds;
				bs->in_progress = in_progress;
				bs->error = error;
				// instead of returning, just goto the end of the function, 
				// so there only needs to be one 'LeaveCriticalSection' call
				goto return_from_update; 
			}
		}

		bs = (BuildStepStatus*)malloc(sizeof(BuildStepStatus));
		bs->step = strdup(step);
		bs->time_in_seconds = time_in_seconds;
		bs->in_progress = in_progress;
		bs->error = error;
		eaPush(&buildStatus->stepStatus, bs);
	}

return_from_update:
	buildLeaveCritSection();
}

void buildStatusGetStatus(BuildStatus *buildStatusCopy)
{
	buildEnterCritSection();

	{
		if ( buildStatusCopy && buildStatus )
		{
			buildStatusCopy->build_failed = buildStatus->build_failed;
			eaClear(&buildStatusCopy->stepStatus);
			eaCopy(&buildStatusCopy->stepStatus, &buildStatus->stepStatus);
		}
	}

	buildLeaveCritSection();
}

void buildStatusFreeStep(BuildStepStatus *bstat)
{
	StructDeInit(tpiBuildStepStatus, bstat);
}

void buildStatusFreeAll()
{
	buildEnterCritSection();
	if (buildStatus)
	{
		int i;
		for ( i = 0; i < eaSize(&buildStatus->stepStatus); ++i )
		{
			buildStatusFreeStep(buildStatus->stepStatus[i]);
		}
		eaDestroy(&buildStatus->stepStatus);
		free(buildStatus);
		buildStatus = NULL;
	}
	buildLeaveCritSection();
}