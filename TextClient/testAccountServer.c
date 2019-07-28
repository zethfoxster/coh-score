#include "testAccountServer.h"
#include "SuperAssert.h"
#include "timing.h"
#include "AccountCatalog.h"
#include "AccountData.h"
#include "chatter.h"
#include "player.h"
#include "entity.h"
#include "dbclient.h"
#include "clientcomm.h"
#include "net_packet.h"
#include "EntVarUpdate.h"
#include "EArray.h"
#include "testUtil.h"
#include "initClient.h"
#include "costume_data.h"
#include "utils.h"
#include "uiNet.h"
#include "cmdgame.h"
#include "character_inventory.h"
#include "uiChat.h"
#include "MessageStoreUtil.h"
#include "textClientInclude.h"
#include "DetailRecipe.h"
#include "basedata.h"
#include "estring.h"
#include "entPlayer.h"
#include "character_eval.h"

void testAccountServer(void)
{
}

void testAccountAddCharacterCount(char *shard, int count)
{
}

void testAccountReceiveCharacterCountsDone(void)
{
}

void testAccountCharacterCountsError(char *errmsg)
{
}

bool testAccountIsTransferPending(void)
{
	return 0;
}

char *testAccountGetNewCharName(void)
{
	return 0;
}

char *testAccountGetOriginalCharName(void)
{
	return 0;
}

void testAccountRevertCharName(void)
{
}

bool IsTestAccountServer_SuperPacksOptionActivated(void)
{
	return 0;
}

// This is needed for logging back in to the DBServer.
extern int loginWrapper(void);
extern char character_name[256];
extern int gPlayerNumber;

void testAccountLoginReconnect(void)
{
}
