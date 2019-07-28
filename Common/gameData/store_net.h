/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STORE_NET_H__
#define STORE_NET_H__

typedef struct Entity Entity;
typedef struct Packet Packet;
typedef struct MultiStore MultiStore;

bool store_SendOpenStore(Entity *e, int iOwner, int iCnt, const MultiStore *pmulti);
bool store_SendOpenStorePNPC(Entity *e, Entity *npc, const MultiStore *pmulti);
bool store_SendOpenStoreContact(Entity *e, int iCnt, const MultiStore *pmulti);

bool store_ReceiveBuyItem(Packet *pak, Entity *e);
bool store_ReceiveBuySalvage(Packet *pak, Entity *e);
bool store_ReceiveBuyRecipe(Packet *pak, Entity *e);
bool store_ReceiveSellItem(Packet *pak, Entity *e);


#endif /* #ifndef STORE_NET_H__ */

/* End of File */

