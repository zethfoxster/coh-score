#ifndef _LEAGUE_H
#define _LEAGUE_H

typedef struct Entity Entity;
typedef struct Packet Packet;

int league_getTeamNumberFromIndex(Entity *e, int index);
int league_IsLeader( Entity* leaguemate, int db_id  );
int league_AcceptOffer( Entity *leader, int new_dbid, int team_leader_id, int teamLockStatusJoiner );
int league_AcceptOfferTurnstile(int instanceID, int db_id, char *invitee_name, int desiredTeam, int isTeamLeader);
void league_MemberQuit( Entity *quitter, int voluntaryLeave );
void league_KickRelay(Entity* e, int kicked_by);
void league_KickMember( Entity *leader, int kickedID, char *player_name );
void league_changeLeader( Entity *e, char * name );
void league_sendList( Packet *pak, Entity * e, int full_update );
void leagueInfoDestroy(Entity* e);
void leagueSendMapUpdate(Entity *e);
void leagueUpdateMaplist( Entity * e, char *map_num, int member_id );
void league_teamswapLockToggle(Entity *e);
void league_updateTeamLeaderTeam(Entity *e, int old_leader_id, int new_leader_id);
void league_updateTeamLeaderSolo(Entity *e, int db_id, int new_leader_id);
void league_updateTeamLockStatus(Entity *e, int team_leader_id, int lock_status);
#endif