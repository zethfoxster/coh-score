#include "containerloadsave.h"
#include "dbcontainer.h"
#include "comm_backend.h"
#include "entity.h"
#include "entPlayer.h"
#include "character_base.h"
#include "timing.h"
#include "container/containerEventHistory.h"
#include "dbcomm.h"
#include "netio.h"
#include "sendToClient.h"

#include "EventHistory.h"

KarmaEventHistory *eventHistory_CreateFromEnt(Entity *ent)
{
	KarmaEventHistory *retval;

	retval = eventHistory_Create();

	if (retval && ent && ent->pl && ent->pchar)
	{
		retval->dbid = ent->db_id;
		retval->playerLevel = ent->pchar->iLevel;
		retval->authName = strdup(ent->auth_name);
		retval->playerName = strdup(ent->name);
		retval->playerClass = strdup(ent->pchar->pclass->pchName);
		retval->playerPrimary = strdup(ent->pl->primarySet);
		retval->playerSecondary = strdup(ent->pl->secondarySet);
	}

	return retval;
}

void eventHistory_SendToDB(KarmaEventHistory *event)
{
	if (event)
	{
		char *packed;

		packed = packageEventHistory(event);

		if (packed)
		{
			dbAsyncContainerUpdate(CONTAINER_EVENTHISTORY, -1, CONTAINER_CMD_CREATE, packed, 0);
		}

		SAFE_FREE(packed);
	}
}

void eventHistory_FindOnDB(U32 sender_dbid, U32 min_time, U32 max_time, char *auth, char *player)
{
	Packet *pak = NULL;
	
	pak = pktCreateEx(&db_comm_link, DBCLIENT_EVENTHISTORY_FIND);
	pktSendBitsAuto(pak, sender_dbid);
	pktSendBitsAuto(pak, min_time);
	pktSendBitsAuto(pak, max_time);

	if (auth)
		pktSendString(pak, auth);
	else
		pktSendString(pak, "");

	if (player)
		pktSendString(pak, player);
	else
		pktSendString(pak, "");

	pktSend(&pak, &db_comm_link);
}

// XXX: Debugging for container saving.
#if 0
void test_container_save(int rows)
{
	KarmaEventHistory *k;
	int count;
	int stage;
	char buf[200];

	for (count = 0; count < rows; count++)
	{
		k = eventHistory_Create();
		k->eventName = strdup("Container test");

		k->timeStamp = timerSecondsSince2000() - (rand() % 179*60*60*24);

		sprintf(buf, "auth%03d", rand() % 1000);
		k->authName = strdup(buf);
		k->dbid = 5000 + rand() % 500;
		sprintf(buf, "player%03d", rand() % 500);
		k->playerName = strdup(buf);
		switch (rand() % 6)
		{
			case 0:
				k->playerClass = strdup("Class_Scrapper");
				break;
			case 1:
				k->playerClass = strdup("Class_Controller");
				break;
			case 2:
				k->playerClass = strdup("Class_Peacebringer");
				break;
			case 3:
				k->playerClass = strdup("Class_Stalker");
				break;
			case 4:
				k->playerClass = strdup("Class_Brute");
				break;
			case 5:
				k->playerClass = strdup("Class_Arachnos_Widow");
				break;
		}
		switch (rand() % 4)
		{
			case 0:
				k->playerPrimary = strdup("Kinetic_Attack");
				break;
			case 1:
				k->playerPrimary = strdup("Ice_Blast");
				break;
			case 2:
				k->playerPrimary = strdup("Training_Gadgets");
				break;
			case 3:
				k->playerPrimary = strdup("Dark_Armor");
				break;
		}
		switch (rand() % 4)
		{
			case 0:
				k->playerSecondary = strdup("Dual_Blades");
				break;
			case 1:
				k->playerSecondary = strdup("Psychic_Assault");
				break;
			case 2:
				k->playerSecondary = strdup("Plant_Control");
				break;
			case 3:
				k->playerSecondary = strdup("Radiation_Emission");
				break;
		}

		k->playerLevel = 5 + rand() % 45;

		k->playedStages = 1 + rand() % 29;

		k->bandTable = strdup("BandRewards");
		k->percentileTable = strdup("PercentRewards");
		k->whichTable = rand() % 2 ? KARMA_TABLE_PERCENTILE : KARMA_TABLE_BAND;
		k->rewardChosen = strdup("SomeKarmaItem");

		if ((rand() % 7) == 0)
		{
			k->pauseDuration = 17 + rand() % 76;
			k->pausedStagesCredited = rand() % 2;
		}

		for (stage = 0; stage < k->playedStages; stage++)
		{
			k->stageScore[stage] = rand() % 75;
			k->stageThreshold[stage] = rand() % 75;

			if (k->stageScore[stage] >= k->stageThreshold[stage])
				k->qualifiedStages++;

			k->stageRank[stage] = rand() % 20;
			k->stagePercentile[stage] = rand() % 100;

			k->avgStagePoints += k->stageScore[stage];
			k->avgStagePercentile += k->stagePercentile[stage];

			k->stageSamples[stage] = 7 + rand() % 19;
		}

		k->avgStagePoints = k->avgStagePoints / k->playedStages;
		k->avgStagePercentile = k->avgStagePercentile / k->playedStages;

		eventHistory_SendToDB(k);
		eventHistory_Destroy(k);
	}
}
#endif

void eventHistory_ReceiveResults(Packet *pak)
{
	U32 dbid = 0;
	char *str = NULL;
	Entity *e = NULL;

	dbid = pktGetBitsAuto(pak);
	e = entFromDbId(dbid);
	str = pktGetString(pak);

	while (str && strlen(str))
	{
		if (e && e->client)
			conPrintf(e->client, "%s", str);

		str = pktGetString(pak);
	}
}
