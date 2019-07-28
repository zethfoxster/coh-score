/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#ifndef AUCTIONHISTORY_H
#define AUCTIONHISTORY_H

#include "stdtypes.h"

typedef struct AuctionInvItem AuctionInvItem;
typedef struct AuctionInventory AuctionInventory;
typedef struct AuctionHistoryItem AuctionHistoryItem;

void auctionhistory_AddInv(AuctionInvItem *itm, char *buyer, char *seller);
AuctionHistoryItem *auctionhistory_Get(const char *pchIdentifier);
AuctionHistoryItem *auctionhistory_GetCategory(const char *pchKey);
void AuctionHistory_Load(void);

#endif //AUCTIONHISTORY_H
