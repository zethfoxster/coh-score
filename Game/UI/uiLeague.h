#ifndef UILEAGUE_H
#define UILEAGUE_H

typedef struct Entity Entity;
typedef struct League League;
void league_OfferMembership(void *foo);
int league_CanOfferMembership(void *foo);

void league_KickMember(void *foo);
int league_CanKickMember(void *foo);
void league_SetLeftSwapMember(void *foo);
void league_SwapMembers(void *foo);
int league_CanSwapMember(void *foo);
int league_CanSwapMembers(void *foo);
int league_CanMoveMember(void *foo);
void league_MoveMembers(void *foo);
int league_targetIsMember( Entity * e );
int league_CanAddToFocus(void *foo);
void league_AddToFocus(void *foo);
int league_CanRemoveFromFocus(void *foo);
void league_RemoveFromFocus(void *foo);
int league_CanMakeLeader(void *foo);
void league_makeLeader(void *unused);
void leagueOrganizeTeamList(League *league);
void leagueCheckFocusGroup(Entity *e);
void leagueClearFocusGroup(Entity *e);
#endif