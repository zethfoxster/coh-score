/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#ifndef AUCTIONFULFILL_H
#define AUCTIONFULFILL_H

typedef struct AuctionInvItem AuctionInvItem;
typedef struct AuctionInventory AuctionInventory;

int auctionfullfiller_CatsCount(void);

void auctionfulfiller_Init(void);
void auctionfulfiller_Tick(void);

void auctionfulfiller_AddInvItm(AuctionInvItem *itm);
void auctionfulfiller_RemoveInvItm(AuctionInvItem *itm);
void auctionfulfiller_AddInv(AuctionInventory *inv);
void auctionfulfiller_RemoveInv(AuctionInventory *inv);

bool auctionfulfiller_InvItmIsInFulfiller(AuctionInvItem *itm);
void auctionfulfiller_GetItemInfo(const char *pchIdentifier, int *pBuying, int *pSelling);
void auctionfulfiller_GetItemInfoByIdx(int idx, const char **ppchIdentifier, int *pBuying, int *pSelling);

#endif //AUCTIONFULFILL_H
