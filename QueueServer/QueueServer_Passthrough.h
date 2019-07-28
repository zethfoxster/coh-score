#include "net_link.h"
#include "..\common\ClientLogin\clientcommLogin.h"

//client to db
void clientPassLogin(Packet *pak_in, Packet *pak_out, GameClientLink *client, U32 ipAddress, U32 linkId);
void clientPassCanStartStaticMap(Packet *pak_in, Packet *pak_out);
void clientPassChoosePlayer(Packet *pak_in, Packet *pak_out);
void clientPassChooseVisitingPlayer(Packet *pak_in, Packet *pak_out);
void clientPassMakePlayerOnline(Packet *pak_in, Packet *pak_out);
void clientPassDeletePlayer(Packet *pak_in, Packet *pak_out);
void clientPassGetCostume(Packet *pak_in, Packet *pak_out);
void clientPassGetPowersetName(Packet *pak_in, Packet *pak_out);
void clientPassCharacterCounts(Packet *pak_in, Packet *pak_out);
void clientPassCatalog(Packet *pak_in, Packet *pak_out);
void clientPassLoyaltyBuy(Packet *pak_in, Packet *pak_out);
void clientPassLoyaltyRefund(Packet *pak_in, Packet *pak_out);
void clientPassShardXferTokenRedeem(Packet *pak_in, Packet *pak_out);
void clientPassRename(Packet *pak_in, Packet *pak_out);
void clientPassRenameTokenRedeem(Packet *pak_in, Packet *pak_out);
void clientPassResendPlayers(Packet *pak_in, Packet *pak_out);
void clientPassCheckName(Packet *pak_in, Packet *pak_out);
void clientPassRedeemSlot(Packet *pak_in, Packet *pak_out);
void clientPassUnlockCharacter(Packet *pak_in, Packet *pak_out);
void clientPassAccountCmd(Packet *pak_in, Packet *pak_out);
void clientPassQuitClient(Packet *pak_in, Packet *pak_out);
void clientPassSignNDA(Packet *pak_in, Packet *pak_out);
void clientPassGetPlayNCAuthKey(Packet *pak_in, Packet *pak_out);

//db to client
void passPlayers(Packet *pak_in, Packet *pak_out);
void passSendPlayers(Packet *pak_in, Packet *pak_out);
void passSendCharacterCounts(Packet *pak_in, Packet *pak_out);
void passSendCostume(Packet *pak_out, Packet *pak_in);
void passSendPowersetNames(Packet *pak_in, Packet *pak_out);
void passSendAccountServerCatalog(Packet *pak_in, Packet *pak_out);
void passAccountServerUnavailable(Packet *pak_in, Packet *pak_out);
void passAccountServerInventoryDB(Packet *pak_out, Packet *pak_in);
void passdbOrMapConnect(Packet *pak_in, Packet *pak_out);
void passCanStartStaticMapResponse(Packet *pak_in, Packet *pak_out);
void passAccountServerClientAuth(Packet *pak_in, Packet *pak_out);
void passAccountServerPlayNCAuthKey(Packet *pak_in, Packet *pak_out);
void passAccountServerNotifyRequest(Packet *pak_in, Packet *pak_out);
void passAccountServerInventory(Packet *pak_in, Packet *pak_out);
