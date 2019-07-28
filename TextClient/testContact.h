#include "uiCompass.h" // for Destination
#include "uiContactDialog.h"

typedef enum TaskType
{
	TASK_NOTYPE,
	TASK_MISSION,
	TASK_RANDOMENCOUNTER,
	TASK_RANDOMNPC,
	TASK_KILLX,
	TASK_DELIVERITEM,
	TASK_VISITLOCATION,
	TASK_COMPLETED,
} TaskType;


void printContactInfo(void);

TaskType getCurrentTaskType(void);
bool isDestOnThisMap(Destination *dest);
bool isTaskDestOnThisMap(void);
Destination *findTaskDest(void);
Destination *findContactLocation(void);
void completeCurrentTask(void);
