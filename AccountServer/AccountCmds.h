/***************************************************************************
 *     Copyright (c) 2007-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: $
 * $Revision: 40606 $
 * $Date: 2007-06-07 12:09:46 -0700 (Thu, 07 Jun 2007) $
 *
 * Module Description:
 *
 * Revision History:
 *
 ***************************************************************************/
#ifndef ACCOUNTCMDS_H
#define ACCOUNTCMDS_H

typedef struct NetLink NetLink;
typedef struct Account Account;
typedef struct AccountServerShard AccountServerShard;
typedef struct Packet Packet;

void AccountServer_SlashInit();
void AccountServer_HandleSlashCmd(NetLink *src_link, int message_dbid, U32 auth_id, int dbid, char *cmd);
void AccountServer_SlashNotifyAccountFound(const char *search_string, U32 found_auth_id);
void AccountServer_SlashNotifyAccountLoaded(Account *account);
void AccountServer_RedeemedCountByShardReply(AccountServerShard *shard, Packet *pak_in);
void AccountServer_RedeemedCountByShardTick(void);

#endif //ACCOUNTCMDS_H
