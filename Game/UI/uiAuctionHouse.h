#ifndef UIAUCTIONHOUSE_H
#define UIAUCTIONHOUSE_H

#include "formatter/smf_render.h"

typedef struct Packet Packet;
typedef struct Entity Entity;

typedef struct SalvageItem SalvageItem;
typedef struct BasePower BasePower;
typedef struct DetailRecipe DetailRecipe;
typedef struct AuctionItem AuctionItem;
//typedef struct SMFBlock SMFBlock;

typedef enum TrayItemType TrayItemType;
typedef enum AuctionInvItemStatus AuctionInvItemStatus;

int auctionHouseWindow(void);
void uiAuctionHouseItemInfoReceive(Packet *pak);
void uiAuctionHouseBatchItemInfoReceive(Packet *pak);
void uiAuctionHandleBannedUpdate(Packet *pak);
void auctionItemMakeName(AuctionItem *item);
int auctionItemGetInfoRequestId(AuctionItem *item);
void auctionInitializeData(void);

#endif
