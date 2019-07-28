/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#include "auctionhistory.h"
#include "auction.h"
#include "BinHeap.h"
#include "XactServer.h"
#include "auctiondb.h"
#include "stringcache.h"
#include "auctionserver.h"
#include "auction.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "estring.h"
#include "AsyncFileWriter.h"
#include "timing.h"
#include "file.h"
#include "StashTable.h"
#include "textcrcdb.h"
#include "utils.h"
#include "AuctionSql.h"
#include "log.h"

#define AUCTION_HISTORY_INITIAL_CAPACITY 50000

StashTable g_AuctionHistory = NULL;
StashTable g_AuctionCatHistory = NULL;
static AsyncFileWriter *fileWriter = NULL;
static int fileWriteTimer = 0;
static char *historyString = NULL;

TokenizerParseInfo parse_AuctionItemHistoryDetail[] = 
{
	{ "Buyer",		TOK_STRING(AuctionHistoryItemDetail,buyer,0),	0},
	{ "Seller",		TOK_STRING(AuctionHistoryItemDetail,seller,0),	0},
	{ "Date",		TOK_INT(AuctionHistoryItemDetail,date,0),		0,	TOK_FORMAT_DATESS2000},
	{ "Price",		TOK_INT(AuctionHistoryItemDetail,price,0),		0}, 
	{ "End",		TOK_END,			0},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_AuctionItemHistory[] = 
{
	{ "Item",			TOK_POOL_STRING|TOK_STRING(AuctionHistoryItem,pchIdentifier,0),				0},
	{ "Histories",		TOK_STRUCT(AuctionHistoryItem,histories,parse_AuctionItemHistoryDetail),			0},
	{ "End",			TOK_END,			0},
	{ "", 0, 0 }
};

// this struct is only used to load the auction history to/from a disk
typedef struct LoadedAuctionHistory
{
	AuctionHistoryItem **loadedHistItems;
	U32 uHistoryVer;
} LoadedAuctionHistory;

static LoadedAuctionHistory s_loadedHist = {0};

// sorts backwards, in order to put newest history at the top
int compareHistDetail(const AuctionHistoryItemDetail **lhs, const AuctionHistoryItemDetail **rhs)
{
	if ((*lhs)->date == (*rhs)->date)
		return 0;
	if ((*lhs)->date < (*rhs)->date)
		return 1;
	return -1;
}

void auctionhistory_AddInv(AuctionInvItem *itm, char *buyer, char *seller)
{
	AuctionHistoryItem *histItm = NULL;
	AuctionHistoryItem *catItm = NULL;
	AuctionHistoryItemDetail *detail = NULL;
	const char *pchKey;
	int iHistory;
	char *estr_history = NULL;
	bool executed;

	if(!devassert(itm && buyer && seller))
		return;

	pchKey = auctionHashKey(itm->pchIdentifier);

	if ( !stashFindPointer(g_AuctionHistory, itm->pchIdentifier, &histItm) )
	{
		// create new AuctionHistoryItem
		histItm = newAuctionHistoryItem(itm->pchIdentifier);
		eaPush(&s_loadedHist.loadedHistItems, histItm);
		stashAddPointer(g_AuctionHistory, histItm->pchIdentifier, histItm, 0);
	}

	if ( !stashFindPointer(g_AuctionCatHistory, pchKey, &catItm) )
	{
		// create new AuctionHistoryItem
		catItm = newAuctionHistoryItem(pchKey);
		stashAddPointer(g_AuctionCatHistory, pchKey, catItm, 0);
	}

#if AUCTION_HISTORY_SIZE > 0
	iHistory = eaSize(&histItm->histories);
	if(iHistory == AUCTION_HISTORY_SIZE)
	{
		detail = histItm->histories[AUCTION_HISTORY_SIZE-1];
		eaSetSize(&histItm->histories, AUCTION_HISTORY_SIZE-1);
	}
	else if(iHistory > AUCTION_HISTORY_SIZE)
		printf("Leaking Memory, Auction History Too Big\n");

	if(detail)
		setAuctionHistoryItemDetail(detail, buyer, seller, timerSecondsSince2000(), itm->infPrice);
	else 
		detail = newAuctionHistoryItemDetail(buyer, seller, timerSecondsSince2000(), itm->infPrice);

	if(!histItm->histories)
	{
		eaCreate(&histItm->histories);
		eaSetCapacity(&histItm->histories,AUCTION_HISTORY_SIZE);
	}
	eaPushFront(&histItm->histories, detail);

	// TODO: write new auctionserver ;)
	if(eaSize(&catItm->histories) >= AUCTION_HISTORY_SIZE)
	{
		eaSetSize(&catItm->histories, AUCTION_HISTORY_SIZE-1);
	}

	if(!catItm->histories)
	{
		eaCreate(&catItm->histories);
		eaSetCapacity(&catItm->histories,AUCTION_HISTORY_SIZE);
	}
	eaPushFront(&catItm->histories, detail);

	ParserWriteText(&estr_history,parse_AuctionItemHistory,histItm,0,0);
	executed = aucsql_insert_or_update_history(histItm->pchIdentifier, &estr_history);
	devassert(executed);
	estrDestroy(&estr_history);
#endif
}

AuctionHistoryItem *auctionhistory_Get(const char *pchIdentifier)
{
	AuctionHistoryItem *histItm = NULL;
	if(!g_AuctionHistory || !stashFindPointer(g_AuctionHistory, pchIdentifier, &histItm))
		return NULL;
	return histItm;
}

AuctionHistoryItem *auctionhistory_GetCategory(const char *pchKey)
{
	AuctionHistoryItem *histItm = NULL;
	if(!g_AuctionCatHistory || !stashFindPointer(g_AuctionCatHistory, pchKey, &histItm))
		return NULL;
	return histItm;
}

void AuctionHistory_Load(void)
{
	int i,j;
	U32 pti_crc;
	if (s_loadedHist.loadedHistItems)
	{
		FatalErrorf("AuctionHistory_Load called twice");
	}

	if (!aucsql_read_history_ver(&s_loadedHist.uHistoryVer))
	{
		FatalErrorf("Could not read version number from AuctionServer SQL DB! Please see console/logs.");
	}

	pti_crc = ParseTableCRC(parse_AuctionItemHistory, NULL);
	if(s_loadedHist.uHistoryVer != pti_crc)
	{
		LOG(LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "Warning: History had its parse table changed, ignoring crc.  This is normal for a new publish.");
		printf("Updating parse table version for history.\n");
		aucsql_write_history_ver(pti_crc);
	}

	if(!aucsql_read_history(parse_AuctionItemHistory, &s_loadedHist.loadedHistItems))
	{
		FatalErrorf("History table could not be read! Could not load. Please see console/logs.");
	}

	devassert(!g_AuctionHistory);
	g_AuctionHistory = stashTableCreateWithStringKeys(AUCTION_HISTORY_INITIAL_CAPACITY, 0);
	g_AuctionCatHistory = stashTableCreateWithStringKeys(AUCTION_HISTORY_INITIAL_CAPACITY, 0);

	for ( i = 0; i < eaSize(&s_loadedHist.loadedHistItems); ++i )
	{
		AuctionHistoryItem *catItm;
		AuctionHistoryItem *histItm = s_loadedHist.loadedHistItems[i];
		stashAddPointer(g_AuctionHistory, histItm->pchIdentifier, histItm, 0);

		if (!stashFindPointer(g_AuctionCatHistory, auctionHashKey(histItm->pchIdentifier), &catItm)) {
			catItm = newAuctionHistoryItem(auctionHashKey(histItm->pchIdentifier));
			stashAddPointer(g_AuctionCatHistory, auctionHashKey(histItm->pchIdentifier), catItm, false);
		}
		for (j = 0; j < eaSize(&histItm->histories); j++) {
			eaSortedInsert(&catItm->histories, compareHistDetail, histItm->histories[j]);
		}
		if (eaSize(&catItm->histories) > AUCTION_HISTORY_SIZE)
			eaSetSize(&catItm->histories, AUCTION_HISTORY_SIZE);
	}
}
