#include "containerEventHistory.h"

KarmaEventHistory *eventHistory_Create(void)
{
	KarmaEventHistory *retval;

	retval = malloc(sizeof(KarmaEventHistory));
	memset(retval, 0, sizeof(KarmaEventHistory));

	return retval;
}

void eventHistory_Destroy(KarmaEventHistory *event)
{
	if (event)
	{
		SAFE_FREE(event->eventName);
		SAFE_FREE(event->authName);
		SAFE_FREE(event->playerName);
		SAFE_FREE(event->playerClass);
		SAFE_FREE(event->playerPrimary);
		SAFE_FREE(event->playerSecondary);
		SAFE_FREE(event->bandTable);
		SAFE_FREE(event->percentileTable);
		SAFE_FREE(event->rewardChosen);
		SAFE_FREE(event);
	}
}
