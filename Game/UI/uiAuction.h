#ifndef UIAUCTION_H
#define UIAUCTION_H

int auctionWindow();

typedef enum AuctionItemType
{
	kAuctionItemType_Power,
	kAuctionItemType_Salvage,
	kAuctionItemType_Recipe,
}AuctionItemType;

void auction_AddToDict( void * item, AuctionItemType type );

typedef struct Packet Packet;

void uiAuctionHouseItemInfoReceive(Packet *pak);
void uiAuctionHouseBatchItemInfoReceive(Packet *pak);
void uiAuctionHouseListItemInfoReceive(Packet *pak);
void uiAuctionHouseHistoryReceive(Packet *pak);

void auction_Init(void);
void addItemToLocalInventory(char * pchIdent, int id, int status, int amtCancelled, int amtStored, int infStored, int amtOther, int infPrice, bool bMergedBid, int make_current);

typedef struct AuctionItem AuctionItem;
void displayAuctionItemIcon( AuctionItem * pItem, F32 x, F32 y, F32 z, F32 sc );
void clearAuctionFields(void);

extern int gAuctionDisabled;

#endif