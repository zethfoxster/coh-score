#ifndef _RT_PERF_H
#define _RT_PERF_H

#include "stdtypes.h"
#include "rt_model.h"

typedef struct
{
	int		test_num;
	int		loop_count;
	int		repeats;
	F32		elapsed;
	int		trisDrawn;
} RdrPerfIO;

typedef struct
{
	int			num_models;
	struct
	{
		RdrModel	model;
		RdrTexList	texlists[4];
	} models[8];
} RdrPerfInfo;

#ifdef RT_PRIVATE
void rtPerfTest(RdrPerfInfo *info);
void rtPerfSet(RdrPerfIO **perfio);
#endif

#endif
