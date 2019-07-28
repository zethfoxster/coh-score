/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BADGES_SERVER_H__
#define BADGES_SERVER_H__

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct Entity Entity;
typedef struct StuffBuff StuffBuff;
typedef struct Packet Packet;
typedef struct BadgeDefs BadgeDefs;
typedef struct Supergroup Supergroup;
typedef struct BadgeStats BadgeStats;
typedef struct SgrpBadges SgrpBadges;
typedef struct BadgeDef BadgeDef;
typedef int VillainGroupEnum;

// badges_load.c
extern StashTable g_hashBadgeStatNames;
extern StashTable g_hashBadgeStatUsage;
extern int g_iMaxBadgeStatIdx;

void badge_ServerInit(void);
	// Must be called at server startup

// badges_db.c
void badge_UnpackLine(Entity *e, char *table, char *field, char *val, int idx); // deprecated, entplayer only. BadgeStats
void badge_UnpackOwned(Entity *e, char *val);

void badge_Package(Entity *e, StuffBuff *psb); // deprecated, for entplayer only. use BadgeStats
void badge_PackageOwned(Entity *e, StuffBuff *psb);

char *badge_Names(const BadgeDefs *badgeDefs);				// Used for making templates
char *badge_StatNames(StashTable hashBadgeStatUsage);	// Used for making templates
void badge_Template(StuffBuff *psb);					// Used for making templates

// badges_server.c

void badge_EvalInit(void);
void badge_Tick(Entity *e);
void badge_UpdateBadgesCollection(Entity *e);
void badge_TicksComplete(void);
	// Notifies badge system that all the players have had a badge tick

void sgrpbadges_Tick(Entity *e);
void entity_SendBadgeUpdate(Entity *e, Packet *pak, bool full_update);

int badge_StatGetIdx(const char *pchBadge);
	// Gets the index for a given badge or 0 if there isn't one.

bool badge_StatSet(Entity *e, const char *pchStat, int iVal);
bool badge_StatAddNoFixup(Entity *e, const char *pchStat, int iAdd);
bool badge_StatAdd(Entity *e, const char *pchStat, int iAdd);
int badge_StatGet(Entity *e, const char *pchStat);

bool badge_StatAddArchitectAllowed(Entity *e, const char *pchStat, int iAdd, int iAllowCheat);


void badge_RecordSystemChange(void);
void badge_RecordKill(Entity *e, Entity *eVictim, F32 fPercent);
void badge_RecordDefeat(Entity *e, Entity *eVictim);
void badge_RecordPlaque(Entity *e, const char *pchName);
void badge_RecordTime(Entity *e, int iTime);
void badge_RecordGhostTrapped(Entity *e);

void badge_RecordLevelChange(Entity *e);

bool badge_OwnsBadge(Entity *e, const char *pchBadge);
	// Checks to see if the player owns the specified badge.

bool badge_Award(Entity *e, const char *pchBadge, bool bNotify);
	// Awards the given badge to the player. If notify is true, then
	//    send fanfares and other visuals to player.

bool badge_ButtonUse(Entity *e, int iIdx);
	// Claims a badge's button reward

bool badge_Revoke(Entity *e, const char *pchBadge);
bool badgestates_Revoke(SgrpBadges *bs, const BadgeDef *pdef, StashTable idxBadgesFromName );
	// Revokes ownership of a badge.

static INLINEDBG void badge_RecordDamageTaken(Entity *e, int i)
	{ if(i<0) i=-i; badge_StatAdd(e, "damage.taken", i); }

static INLINEDBG void badge_RecordDamageGiven(Entity *e, int i)
	{ if(i<0) i=-i; badge_StatAdd(e, "damage.given", i); }

static INLINEDBG void badge_RecordHealingTaken(Entity *e, int i)
	{ if(i<0) i=-i; badge_StatAdd(e, "healing.taken", i); }

static INLINEDBG void badge_RecordHealingGiven(Entity *e, int i)
	{ if(i<0) i=-i; badge_StatAdd(e, "healing.given", i); }

static INLINEDBG void badge_RecordExperience(Entity *e, int i)
    { badge_StatAdd(e, "xp", i); }

static INLINEDBG void badge_RecordExperienceDebt(Entity *e, int i)
   {if(i>0) badge_StatAdd(e, "xpdebt", i);}

static INLINEDBG void badge_RecordExperienceDebtRemoved(Entity *e, int i)
   {if(i>0) badge_StatAdd(e, "xpdebt.removed", i);}

static INLINEDBG void badge_RecordWisdom(Entity *e, int i)
	{ badge_StatAdd(e, "wisdom", i); }

static INLINEDBG void badge_RecordInfluence(Entity *e, int i)
	{ badge_StatAdd(e, "influence", i); }

static INLINEDBG void badge_RecordAuctionInfluenceEarned(Entity *e, int i)
	{ badge_StatAdd(e, "influence.auction.earned", i); }

static INLINEDBG void badge_RecordAuctionInfluenceSpent(Entity *e, int i)
	{ badge_StatAdd(e, "influence.auction.spent", i); }

static INLINEDBG void badge_RecordPrestige(Entity *e, int i)
	{ badge_StatAdd(e, "prestige", i); }

static INLINEDBG void badge_RecordPrestigeBought(Entity *e, int i)
	{ badge_StatAdd(e, "prestige.bought", i); }

static INLINEDBG void badge_RecordInfluenceGiven(Entity *e, int i)
	{ badge_StatAdd(e, "influence.given", i); }

static INLINEDBG void badge_RecordInfluenceTaken(Entity *e, int i)
	{ badge_StatAdd(e, "influence.taken", i); }

static INLINEDBG void badge_RecordMentorTime(Entity *e, int i)
	{ badge_StatAdd(e, "time.mentor", i); }

static INLINEDBG void badge_RecordSidekickTime(Entity *e, int i)
	{ badge_StatAdd(e, "time.sidekick", i); }

static INLINEDBG void badge_RecordExemplarTime(Entity *e, int i)
	{ badge_StatAdd(e, "time.exemplar", i); }

static INLINEDBG void badge_RecordAspirantTime(Entity *e, int i)
	{ badge_StatAdd(e, "time.aspirant", i); }

static INLINEDBG void badge_RecordPresentsCollected(Entity *e, int i)
	{ badge_StatAdd(e, "scripts.PresentsCollected", i); }

static INLINEDBG void badge_RecordRocketsLaunched(Entity *e, int i)
	{ badge_StatAdd(e, "scripts.RocketsLaunched", i); }

static INLINEDBG void badge_RecordRVPillbox(Entity *e, int i)
	{ badge_StatAdd(e, "scripts.RVPillbox", i); }

static INLINEDBG void badge_RecordRVHeavyPet(Entity *e, int i)
	{ badge_StatAdd(e, "scripts.RVHeavyPet", i); }

static INLINEDBG void badge_RecordBBOreConvert(Entity *e, int i)
	{ badge_StatAdd(e, "scripts.BBOreConvert", i); }

static INLINEDBG void badge_RecordRWZBombPlace(Entity *e, int i)
	{ badge_StatAdd(e, "scripts.RWZBombPlace", i); }

static INLINEDBG void badge_RecordTaskComplete(Entity *e, int i)
{ badge_StatAdd(e, "task.complete", i); }

static INLINEDBG void badge_RecordTaskCompleteSuccess(Entity *e, int i)
{ badge_StatAdd(e, "task.success", i); }

static INLINEDBG void badge_RecordTaskCompleteFailed(Entity *e, int i)
{ badge_StatAdd(e, "task.failed", i); }


void badge_RecordRewardToken(Entity *e);
void badge_RecordAccountInventoryUpdate(Entity *e);
void badge_RecordSouvenirClue(Entity *e, const char *pchClue);

// TODO: Remove debugging for these badge stats
extern void dbLog(char const *cmdname,Entity *e, FORMAT fmt, ...);

static INLINEDBG void badge_RecordHeldTime(Entity *e, int i)
	{ if(i<120) badge_StatAdd(e, "time.held", i); else dbLog("Badge:Debug", e, "Held time very long (and ignored): %d (0x%08x)", i, i); }

static INLINEDBG void badge_RecordSleepTime(Entity *e, int i)
	{ if(i<120) badge_StatAdd(e, "time.sleep", i); else dbLog("Badge:Debug", e, "Sleep time very long (and ignored): %d (0x%08x)", i, i); }

static INLINEDBG void badge_RecordImmobilizedTime(Entity *e, int i)
	{ if(i<120) badge_StatAdd(e, "time.immobilized", i); }


static INLINEDBG void badge_RecordStunnedTime(Entity *e, int i)
	{ if(i<120) badge_StatAdd(e, "time.stunned", i); else dbLog("Badge:Debug", e, "Stunned time very long (and ignored): %d (0x%08x)", i, i); }

void badge_RecordMissionComplete(Entity *e, const char *missionName, VillainGroupEnum vgrp);
void badge_RecordInventionCreated(Entity *e, const char *recipename, bool dontGiveGenericCredit);

void badgeMonitor_SendToClient(Entity *e);

#endif /* #ifndef BADGES_SERVER_H__ */


/* End of File */

