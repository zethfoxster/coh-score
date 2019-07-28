#ifndef UITEAM_H
#define UITEAM_H

typedef struct Entity Entity;

int team_TargetOnMap(void *foo);
int team_targetIsMember( Entity * e );
int team_CanOfferMembership(void *foo);
int team_CanKickMember(void *foo);
int team_CanCreateLeagueMember(void *foo);
int team_CanQuit(void);

void team_OfferMembership(void *foo);
void team_KickMember(void *foo);
void team_Quit(void * unused);
void team_CreateLeagueMember(void *foo);

int team_makeLeaderVisible(void *unused);
void team_makeLeader(void *unused);

int team_PlayerIsOnTeam();
#endif

