/*\
 *
 *	dbstub.c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Stub functions for servers that connect to the db as mapservers
 *  and need a bunch of stubs for dbcomm.c and dbcontainer.c
 *
 */

#include "dbcomm.h"
#include "dbcontainer.h"
#include "error.h"
#include "entity.h"
#include "entsend.h"
#include "comm_backend.h"
#include "svr_player.h"

// Externs
int last_db_error_code;
char last_db_error[256];
PlayerEnts player_ents[PLAYER_ENTTYPE_COUNT];
//Entity *entities[MAX_ENTITIES_PRIVATE];
U8 entity_state[MAX_ENTITIES_PRIVATE];
int entities_max;
EntityInfo entinfo[MAX_ENTITIES_PRIVATE];

char* teamTypeName(ContainerType type) { return ""; }
char *dbg_FillNameStr(char *pchDest, size_t iDestSize, Entity *e) { return ""; }
StaticMapInfo* staticMapInfoFind(int mapID) { return NULL; }
Entity *entFromDbId(int db_id) { return NULL; }

void cmdOldServerHandleRelay(char *msg) {;}
void dbGatherMapLinkInfo() {;}
void forceLogout(int db_id,int reason) {;}
void svrLogPerf(void) {}
void handleDbNewMapLoading(Packet *pak) {;}
void handleDbNewMapReady(Packet *pak) {;}
void handleDbNewMap(Packet *pak) {;}
void handleDbNewMapFail(Packet *pak) {;}
void handleForceLogout(Packet *pak) {;}
void handleMapRegisterCallback(Packet *pak) {;}
void handleServerDoorUpdate(Packet *pak) {;}
void MissionRequestShutdown() {;}
void returnAllPlayers(int disconnect,char *msg) {;}
void sendDoorsToDbServer(int is_static) {;}
void serverSynchTime(void) {;}
void staticMapInfoResetPlayerCount() {;}
void teamlogPrintf(const char* func, const char* s, ...) {;}
typedef struct StoryArc StoryArc;
void MissionSetInfoString(char* info, StoryArc *pArc) {;}
void ArenaSetInfoString(char* info) {;}
void SGBaseSetInfoString(char* info) {;}
void RaidSetInfoString(char *info) {}
void EndGameRaidSetInfoString(char *info) {}
void dealWithLostMembership(int list_id, int container_id, int ent_id) {;}
void dealWithBroadcast(int container,int *members,int count,char *msg,int msg_type,int senderID,int container_type) {;}
void dealWithEntAck(int list_id,int id) {;}
void dealWithShardAccountAck(int list_id,int id) {;}
void mapGroupInit(char* uniquename) {;}
void dbUpdateAllOnlineInfo() {;}
void dbUpdateAllOnlineInfoComments() {;}
void handleOnlineEnts(Packet *pak) {;}
void dbListDeletedChars_HandleRslt(Packet *pak) {;}
void dbRestoreDeletedChar_HandleRslt(Packet* pak) {;}
void dbOfflineCharacter_HandleRslt(Packet* pak) {;}
void handleOnlineEntComments(Packet *pak) {;}
void dbQueryReceiveInfo(Packet *pak) {;}
void handleArenaAddress(Packet* pak) {;}
void getAuthId(Entity *e) {;}
void handleSgChannelInviteHelper(Packet *pak) {;}
void beaconRequestSetMasterServerAddress(char* serverAddress){;}
void handleDbBackupSearch(Packet *pak){;}
void handleDbBackupApply(Packet *pak){;}
void handleDbBackupView(Packet *pak){;}

// *********************************************************************************
//  for dblog
// *********************************************************************************
void shardCommReconnect() {} 
int gChatServerTestClients = 0;
void shardCommMonitor(){}


#ifndef CONTAINER_SERVER
void dbNameTableInit() {;}
void handleSendEntNames(Packet *pak) {;}
void handleSendGroupNames(Packet *pak) {;}
void handleClearPnameCacheEntry(Packet *pak) {;}
#endif
