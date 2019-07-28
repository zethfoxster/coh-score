#ifndef PLAYERCREATEDSTORYARCSERVER_H
#define PLAYERCREATEDSTORYARCSERVER_H

#include "MissionSearch.h" // enums

typedef struct PlayerCreatedStoryArc PlayerCreatedStoryArc;
typedef struct Entity Entity;
typedef struct StoryInfo StoryInfo;
typedef struct StoryContactInfo StoryContactInfo;
typedef struct ContactTaskPickInfo ContactTaskPickInfo;
typedef struct StoryArc StoryArc;
typedef struct TaskForce TaskForce;

#define ARCHITECT_GANG_ALLY   1
#define ARCHITECT_GANG_ENEMY  2
#define ARCHITECT_GANG_ROGUE  3
#define ARCHITECT_GANG_RUMBLE 4

void testStoryArc(Entity *e, char *str);

bool playerCreatedStoryarc_Available(Entity *e);
int playerCreatedStoryarc_GetTask(Entity* player, StoryInfo* info, StoryContactInfo* contactInfo, ContactTaskPickInfo* pickinfo);
int playerCreatedStoryarc_TaskAdd(Entity *player, StoryContactInfo* contactInfo, ContactTaskPickInfo *pickInfo);

PlayerCreatedStoryArc* playerCreatedStoryarc_Add(char *str, int id, int validate, char **failErrors, char **playableErrors, int *hasErrors );
void playerCreatedStoryArc_SetContact( PlayerCreatedStoryArc *pArc, TaskForce * pTaskforce );

StoryArc* playerCreatedStoryArc_GetStoryArc( int id );
PlayerCreatedStoryArc* playerCreatedStoryArc_GetPlayerCreatedStoryArc( int id );
void playerCreatedStoryArc_Update( Entity *player );
void playerCreatedStoryArc_CleanStash(void);

void missionserver_map_publishArc(Entity *e, int arcid, const char *arcstr); // arcid for republish
void missionserver_map_startArc(Entity *e, int arcid);
void missionserver_map_unpublishArc(Entity *e, int arcid);
void missionserver_map_voteForArc(Entity *e, int arcid, MissionRating vote);
void missionserver_map_setKeywordsForArc(Entity *e, int arcid, int vote );
void missionserver_map_reportArc(Entity *e, int arcid, int type, const char *comment, const char * global);
void missionserver_map_printUserInfo(Entity *e, int message_id, char *authname, char *region);
void missionserver_map_printUserInfoById(Entity *e, int message_id, U32 authid, char *region);

void missionserver_map_comment(Entity *e, int arcid, const char * msg);

void missionserver_map_requestSearchPage(Entity *e, MissionSearchPage category, const char *context, int page, int viewedFilter, int levelFilter, int authorId);
void missionserver_map_requestArcInfo(Entity *e, int arcid);
void missionserver_map_requestBanStatus(int arcid);
void missionserver_map_requestArcData_mapTest(int arcid);
void missionserver_map_requestArcData_playArc(Entity *e, int arcid, int test, int devchoice); // response handled by missionserver_map_receiveArcData()
void missionserver_map_requestArcData_viewArc(Entity *e, int arcid); // response handled by missionserver_map_receiveArcData()
void missionserver_map_requestArcData_otherUser(Entity *e, int arcid);
void missionserver_map_requestInventory(Entity *e);
void missionserver_map_claimTickets(Entity *e, int tickets);
void missionserver_map_grantTickets(Entity *e, int tickets);
void missionserver_map_grantOverflowTickets(Entity *e, int tickets);
void missionserver_map_setBestArcStars(Entity *e, int stars);
void missionserver_map_setPaidPublishSlots(Entity *e, int paid_slots);

void missionserver_map_handlePendingStatus(Entity *e);

void missionserver_map_receiveSearchPage(Packet *pak_in);
void missionserver_map_receiveArcInfo(Packet *pak_in);
void missionserver_map_receiveBanStatus(Packet *pak_in);
void missionserver_map_receiveArcData(Packet *pak_in);
void missionserver_map_receiveArcDataOtherUser(Packet *pak_in);
void missionserver_map_receiveInventory(Packet *pak_in);
void missionserver_map_receiveTickets(Packet *pak_in);
void missionserver_map_itemBought(Packet *pak_in);
void missionserver_map_itemRefund(Packet *pak_in);
void missionserver_map_overflowGranted(Packet *pak_in);

void architectKiosk_SetOpen(Entity *e, int open);
void architectKiosk_recieveClick(Entity *e, Packet *pak);
void architectForceQuitTaskforce(Entity *e);

void playerCreatedStoryArc_Reward(Entity *e, int storyarc_complete);
void playerCreatedStoryArc_CalcReward(Entity *e, int iVictimLevel, const char *pchRank, F32 fScale );
void playerCreatedStoryArc_RewardTickets(Entity *e, int iAmount, int bEndOfMission);
void playerCreatedStoryArc_BuyUpgrade(Entity *e, char *pchUnlockKey, int iPublishSlots, int iTicketCost);

U32 playerCreatedStoryArc_GetPublishSlots(Entity *e);

// This is called from entProcess() and checks if pending orders have gone stale.
// Don't call it yourself please!
void InventoryOrderTick(Entity *e);

#define MAPSERVER_LOADTESTING 0
#endif
