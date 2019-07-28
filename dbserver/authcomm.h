#ifndef _AUTHCOMM_H
#define _AUTHCOMM_H

#include "auth.h"

int authValidLogin(U32 auth_id,U32 cookie,char *name,U8 user_data[AUTH_BYTES],U32 *loyalty,U32 *loyaltyLegacy,U32 *payStat);
void authSendPlayOK(int auth_id, int one_time_pwd );
void authSendPlayGame(int auth_id);
void authSendQuitGame(int auth_id, short reason, int usetime);
void authSendKickAccount(int auth_id,short reason);
void authSendBanAccount(int auth_id,short reason);
void authSendServerNumOnline(int num_online);
void authSendVersion(int version);
void authSendUserData(int auth_id, U8 data[AUTH_BYTES]);
void authSendPing(U32 id);
void authProcess(void);
int authCommInit(char *server,int port);
int authDisconnected(void);
void authCommDisconnect(void);
void authReconnectOnePlayer(char *account,U32 auth_id);
void authSendShardTransfer(int auth_id, int shard);
int handleShardTransferRequest(int auth_id, char *name, unsigned char *auth_data);

#endif
