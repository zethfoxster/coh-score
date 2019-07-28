/***************************************************************************
 *     Copyright (c) 2007-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: $
 * $Revision: 58054 $
 * $Date: 2010-07-12 12:22:11 -0700 (Mon, 12 Jul 2010) $
 *
 * Module Description:
 *
 * Revision History:
 *
 ***************************************************************************/
#ifndef ACCOUNTSERVERCOMM_H
#define ACCOUNTSERVERCOMM_H

#include "account/AccountTypes.h"
#include "account/AccountData.h"
#include "ClientLogin/clientcommLogin.h"

typedef struct Packet Packet;
typedef struct NetLink NetLink;
typedef struct AccountInventorySet AccountInventorySet;
typedef struct ShardAccountCon ShardAccountCon;

int accountServerSecondsSinceUpdate();
NetLink *accountLink();
bool accountServerResponding();
int accountInventoryFindBonusCharacterSlots(U32 auth_id);
U8 *accountInventoryGetLoyaltyStatus(U32 auth_id);
void accountInventoryRelayForRelay(NetLink *link_out, ShardAccountCon *con, int dbid);
void accountInventoryRelayToClient(U32 auth_id, int shardXferTokens, int shardXferFreeOnly, int virtualCurrencyBal, U32 lastEmailTime, U32 lastNumEmailsSent, int authoritative);
void account_sendCatalogUpdateToNewMap(NetLink *link);

void accountMonitorInit(void);
void accountAuctionShardXfer(OrderId order_id, int dbid);
void accountMarkSaved(U32 auth_id, OrderId **order_ids);
void accountMarkOneSaved(U32 auth_id, OrderId order_id);

void handleSendAccountCmd(Packet *pak_in, NetLink *link);
void handleAccountShardXfer(Packet *pak_in);
void handleAccountOrderRename(Packet *pak_in);
void handleAccountOrderRespec(Packet *pak_in);
void handleRequestPlayNCAuthKey(Packet *pak);
void relayAccountCharCount(Packet *pak_in);
void relayAccountGetInventory(Packet *pak_in);
void relayAccountInventoryChange(Packet *pak_in);
void relayAccountUpdateEmailStats(Packet *pak_in);
void relayAccountCertificationTest(Packet * pak_in);
void relayAccountCertificationGrant(Packet * pak_in);
void relayAccountCertificationClaim(Packet * pak_in);
void relayAccountCertificationRefund(Packet *pak);
void relayAccountMultiGameTransaction(Packet *pak_in);
void relayAccountTransactionFinish(Packet * pak_in);
void relayAccountRecoverUnsaved(Packet *pak_in);
void relayAccountLoyaltyChange(Packet *pak_in);
void relayAccountLoyaltyEarnedChange(Packet *pak_in);
void relayAccountLoyaltyReset(Packet *pak_in);

void account_handleProductCatalog(GameClientLink *client);

void account_handleAccountRegister(U32 auth_id, char *auth_name); // This must be sent first
void account_handleAccountLogout(U32 auth_id);
void account_handleGameClientCharacterCounts(U32 auth_id, bool vip);
void account_handleShardXferTokenClaim(U32 auth_id, U32 dbid, char *destshard, int isVIP);
void account_handleGenericClaim(U32 auth_id, AccountRequestType reqType, U32 dbid);
void account_handleLoyaltyAuthUpdate(U32 auth_id, U32 payStat, U32 loyaltyEarned, U32 loyaltyLegacy, int vip);
void account_handleLoyaltyChange(U32 auth_id, int nodeIndex, bool nodeDelta);
void account_handleLoyaltyEarnedChange(U32 auth_id, int delta);
void account_handleLoyaltyReset(U32 auth_id);
void account_handleAccountInventoryRequest(U32 auth_id);
void account_handleAccountInventoryChange(U32 auth_id, SkuId sku_id, int delta);
void account_handleAccountUpdateEmailStats(U32 auth_id, U32 lastEmailTime, U32 lastNumEmailsSent);
void account_handleAccountCertificationGrant(U32 auth_id, SkuId sku_id, int dbid, int count);
void account_handleAccountCertificationClaim(U32 auth_id, U32 db_id, SkuId sku_id);
void account_handleAccountCertificationRefund(U32 auth_id, SkuId sku_id, int dbid);
void account_handleAccountTransactionFinish(U32 auth_id, OrderId order_id, bool success);
void account_handleAccountRecoverUnsaved(U32 auth_id, U32 dbid);
void account_handleSlotTokenClaim(const char *ip_str, const char *account_name, U32 auth_id);
void account_handleAccountSetInventory(U32 auth_id, SkuId sku_id, U32 value, bool resendInventory);
void account_handleGetPlayNCAuthKey(U32 auth_id, int request_key, char* auth_name, char* digest_data );

#endif //ACCOUNTSERVERCOMM_H
