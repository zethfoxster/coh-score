#include "structnet.h"

typedef struct
{
	char *step;
	float time_in_seconds;
	bool in_progress;
	bool error;
} BuildStepStatus;

typedef struct
{
	int build_failed;
	BuildStepStatus **stepStatus;
} BuildStatus;

static ParseTable tpiBuildStepStatus[] =
{
	{"Step", TOK_STRING(BuildStepStatus, step, 0), 0, TOK_FORMAT_LVWIDTH(255)},
	{"Time", TOK_F32(BuildStepStatus, time_in_seconds, 0)},
	{"InProgress", TOK_BOOL(BuildStepStatus, in_progress, 0)},
	{"Error", TOK_BOOL(BuildStepStatus, error, 0)},
	{0}
};

static ParseTable tpiBuildStatus[] =
{
	{"Failed", TOK_INT(BuildStatus, build_failed, 0) },
	{"StepStatus", TOK_STRUCT(BuildStatus, stepStatus, tpiBuildStepStatus)},
	{0}
};

void buildStatusInit();
void buildEnterCritSection();
void buildLeaveCritSection();
void buildStatusUpdateStep(char *step, float time_in_seconds, int in_progress, int error);
void buildStatusUpdateStatus(int build_failed);
void buildStatusGetStatus(BuildStatus *buildStatus);
// free a single BuildStatus struct
void buildStatusFree(BuildStatus *bstat);
void buildStatusFreeAll();