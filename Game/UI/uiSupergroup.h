#ifndef UISUPERGROUP_H
#define UISUPERGROUP_H

typedef struct SupergroupStats SupergroupStats;

void sgListPrepareUpdate();
void sgListReset();

int supergroupWindow();

int allianceCanInviteTarget(void *nodata);
void allianceInviteTarget(void *nodata);
void raidInviteTarget(void *nodata);
int raidCanInviteTarget(void *nodata);

int sgroup_exists(void *foo);
int sgroup_canOfferMembership(void *foo);
int sgroup_higherRanking(void *foo);
int sgroup_canKick(void* foo);
int sgroup_canPromote(void* foo);
int sgroup_canDemote(void* foo);
int sgroup_isMember(Entity *e, int db_id );

void sgroup_invite(void *foo);
void sgroup_kick(void *foo);
void sgroup_promote(void *foo);
void sgroup_demote(void *foo);

void sgroup_kickName( char * name );

#define SG_MEMBER_ARCHETYPE				"SG Arch"
#define SG_MEMBER_ORIGIN				"SG Origin"
#define SG_MEMBER_NAME_COLUMN			"SG Name"
#define SG_MEMBER_TITLE_COLUMN			"SG Title"
#define SG_MEMBER_ZONE					"SG Zone"
#define SG_MEMBER_LEVEL					"SG Level"
#define SG_MEMBER_LAST_ONLINE_COLUMN	"SG Last Online"
#define SG_MEMBER_DATE_JOINED_COLUMN	"SG Date Joined"
#define SG_MEMBER_PRESTIGE				"SG Prestige"

SupergroupStats* sgGetPlayerSGStats(void);

#endif
