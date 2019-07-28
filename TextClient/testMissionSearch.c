#include "earray.h"
#include "mathutil.h"
#include "clientcomm.h"
#include "textparser.h"
#include "playerCreatedStoryarc.h"
#include "playerCreatedStoryarcValidate.h"
#include "player.h"
#include "EString.h"
#include "entity.h"
#include "VillainDef.h"
#include "bitfield.h"
#include "gamedata/randomName.h"
#include "missionMapCommon.h"
#include "structDefines.h"
#include "pnpcCommon.h"
#include "clientcomm.h"
#include "utils.h"



#include "uiNet.h"

int testMissionSearch_hasAllIds()
{
	return 1;
}

int *testMissionSearch_getAllPublishedArcIds()
{
	return 0;
}

void testMissionSearch_searchAll(int page)
{
}

void testMissionSearch_registerPages(int page, int pages)
{
}



void testMissionSearch_requestAllPublishedArcIds()
{
}

void testMissionSearch_testVote(int arcId)
{
}

void testMissionSearch_testVoteRandom(int nVotes)
{
}

void testMissionSearch_testPlayRandom()
{
}

void testMissionSearch_forcePlay()
{
}

void testMissionSearch_testSearch()
{
}


void testMissionSearch_registerHeader(int id, char *header, int rating)
{
}

void testMissionSearch_receiveArc(int arcId, char *headerStr, int rating)
{
}

void testMissionSearchDelete()
{
}

PlayerCreatedStoryArc * missionMakerTestCreateRandomArc(PlayerCreatedStoryArc *pArc)
{
	return 0;
};

void missionMakerTestPublish()
{
}

void testMissionSearchTestPublish()
{
}
MissionSearchParams *MissionMakerTestCreateRandomSearch(MissionSearchParams *search)
{
	return 0;
}

void testMissionSearch_forceSearch()
{
}

void testMissionSearch_forceVote()
{
}

void testMissionSearch_registerData(char *arc){};