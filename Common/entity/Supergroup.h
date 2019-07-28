/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SUPERGROUP_H
#define SUPERGROUP_H

#include "teamCommon.h"

#define SG_NAME_LEN		64
#define SG_TITLE_LEN	64
#define SG_MESSAGE_LEN	256
#define SG_MOTTO_LEN	256
#define SG_DESCRIPTION_LEN 512
#define MAX_SG_INVRECIPEID 1024 // db limit. need new row after that
#include "badges.h"
#include "SgrpBasePermissions.h" // enum SgrpBaseEntryPermission

typedef struct Detail Detail;
typedef struct DetailRecipe DetailRecipe;
typedef struct RewardToken RewardToken;
typedef struct EvalContext EvalContext;
typedef struct NetLink NetLink;
typedef struct SupergroupStats SupergroupStats;
typedef struct SgrpBadgeStats SgrpBadgeStats;
typedef struct StaticMapInfo StaticMapInfo;
typedef struct StoryTaskInfo StoryTaskInfo;
typedef struct TaskStatus TaskStatus;

#define MAX_SUPER_GROUP_MEMBERS	150
#define MAX_SUPERGROUP_ALLIES	10

#define NUM_SG_RANKS									7
#define NUM_SG_RANKS_VISIBLE							6
#define NUM_SG_RANKS_WITH_VARIABLE_PERMISSIONS			5

#define MAX_SUPERGROUP_PRESTIGE	2000000000

typedef enum sgroupPermissionEnum
{
	SG_PERM_ANYONE = -1,
	SG_PERM_ALLIANCECHAT = 0,
	SG_PERM_DESCRIPTION,
	SG_PERM_MOTTO,
	SG_PERM_PROMOTE,
	SG_PERM_DEMOTE,
	SG_PERM_KICK,
	SG_PERM_INVITE,
	SG_PERM_TITHING,
	SG_PERM_RAID_SCHEDULE,
	SG_PERM_MOTD,
	SG_PERM_ALLIANCE,
	SG_PERM_AUTODEMOTE,
	SG_PERM_BASE_EDIT,
	SG_PERM_RANKS,
	SG_PERM_PAYRENT,
	SG_PERM_GET_SGMISSION,
	SG_PERM_CAN_SET_BASEENTRY_PERMISSION,
	SG_PERM_ARENA,
	SG_PERM_COSTUME,
	SG_PERM_BASESTORAGE_ALLOWED_TO_TAKE_FROM,
	SG_PERM_BASESTORAGE_ALLOWED_TO_ADD_TO,
	SG_PERM_RAID_POINTS_ALLOWED_TO_SPEND,
	// ONLY ADD ENTRIES RIGHT HERE AT THE END OF THE LIST, THIS GETS SAVED TO THE DATABASE
	SG_PERM_COUNT
} sgroupPermissionEnum;

typedef struct sgroupPermissionName
{
	sgroupPermissionEnum permission;
	char* name;
	U32 used	: 1;
} sgroupPermissionName;

extern sgroupPermissionName sgroupPermissions[SG_PERM_COUNT];

typedef struct SupergroupRank
{
	char name[SG_TITLE_LEN];

	U32 permissionInt[(SG_PERM_COUNT+31) / 32];
} SupergroupRank;

typedef struct SupergroupAlly
{
	int			db_id;				// ally SG dbid, 0 if no ally in this slot
	int			minDisplayRank;		// alliance chat: rank a person from this ally must be to be heard by my SG
	int			dontTalkTo;			// alliance chat: don't allow any chat from my SG to go to this ally
	char		name[SG_NAME_LEN];	// ally name, for UI
} SupergroupAlly;

typedef struct RecipeInvItem
{
	DetailRecipe const *recipe;
	U32 count;            // Count of recipes. If -1, then there is an unlimited number.
} RecipeInvItem;


typedef struct SupergroupMemberInfo
{
	int dbid;
	int rank;
} SupergroupMemberInfo;


#define DETAIL_FLAGS_IN_BASE     (1<<0)
	// If the detail is placed in the base
#define DETAIL_FLAGS_AUTOPLACE   (1<<1)
	// If the detail should be automatically placed at the first opportunity.
#define DETAIL_FLAGS_ACTIVATED   (1<<2)
	// If the detail is in the correct state to actually be used
#define DETAIL_FLAGS_RAIDSERVER_OWNED_ITEM_OF_POWER   (1<<3)
	// If the detail is an Item of Power, and the raid server is the authority on which ones the supergroup owns
#define DETAIL_FLAGS_NEW_IOP_RAIDSERVER_NEEDS_TO_KNOW_ABOUT   (1<<4)
	// If the detail is an Item of Power, and the raid server is the authority on which ones the supergroup owns
#define DETAIL_FLAGS_RAIDSERVER_REMOVE_THIS_ITEM_OF_POWER   (1<<5)
	// If the detail is an Item of Power, and the raid server is the authority on which ones the supergroup owns

#define SG_FLAGS_BONUS_PRESTIGE_APPLIED (1<<0)
	// Set if the supergroup has had their bonus prestige for number of members
	// applied.
#define SG_FLAGS_BONUS_COUNT_APPLIED (1<<1)
	// Set if the supergroup has had their bonus prestige count set properly.
#define SG_FLAGS_FIXED_BAD_BASE_PRESTIGE (1<<2)
	// for fixing the not added value of the base.
#define SG_FLAGS_LOCKED_PLAYERTYPE (1<<3)
	// for locking playerType in preparation for Going Rogue

typedef struct SpecialDetail
{
	const Detail *pDetail;	// Pointer to detail definition
	U32 creation_time;		// seconds since 2000 when the detail was made
	U32 expiration_time;	// time when this detail expires, if 0 it is permanent
	U32 iFlags;				// Flags (see DETAIL_FLAGS... above)
	bool bValid;			// Used for internal bookkeeping, not persisted
} SpecialDetail;

typedef struct Supergroup
{
	TeamMembers		members;
	char			name[SG_NAME_LEN];
	char			leaderTitle[SG_TITLE_LEN];
	char			captainTitle[SG_TITLE_LEN];
	char			memberTitle[SG_TITLE_LEN];
	char			msg[SG_MESSAGE_LEN];
	char			motto[SG_MOTTO_LEN];
	char			description[SG_DESCRIPTION_LEN];
	char			emblem[128];
	int				colors[2];
	SupergroupAlly	allies[MAX_SUPERGROUP_ALLIES];
	int				alliance_minTalkRank;			// alliance chat: rank a person in my SG must be to talk
	int				tithe;					// in basis points
	int				demoteTimeout;			// in seconds
	int				influence;
	int				playerType;				//corresponds to PlayerType enum

	int				spacesForItemsOfPower; // The number of available spaces for IoPs in the base. (includes Mounts and Placed IoPs)

	int				prestige;				// liquid prestige
	int				prestigeBase;			// prestige tied up in the base details
	int				prestigeAddedUpkeep;	// extra cost of upkeep of items
						// prestige is a liquid asset, while
						// prestigeBase represents the money invested
						// in property. prestigeAddedUpkeep is a
						// straight expense based on the sum of all the
						// asset's upkeep field
	int				prestigeBonusCount;		// The number of members which the prestige bonus has been given to.
	int				praetBonusIds[MAX_SUPER_GROUP_MEMBERS];		// All ex-Praetorians whose membership grants a bonus
	int				dateCreated;
	int				passcode;
	char			music[50];
	U32				flags;

	struct SgrpBaseUpkeep
	{
		int prtgRentDue;
		U32 secsRentDueDate;
	} upkeep;
	
	SgrpBaseEntryPermission entryPermission;
	
	RewardToken		**rewardTokens;
	SpecialDetail	**specialDetails;
	SupergroupMemberInfo **memberranks;
	SupergroupRank	rankList[NUM_SG_RANKS];

	SgrpBadges		badgeStates; // what badges does the group own
	SgrpBadgeStats	*pBadgeStats; // the latest stats received

	RecipeInvItem **invRecipes;

#if SERVER | STATSERVER | RAIDSERVER
    StoryTaskInfo*	activetask;
#endif
#if CLIENT
	TaskStatus* sgMission;
#endif
} Supergroup;

typedef struct SupergroupStats
{
	int  db_id;
	int  rank;
	int  total_time_played;
	int  member_since;
	int  last_online;
	int  online;
	int	 level;
	int  map_id;
	int  prestige;
	int  playerType;
	char zone[256];
	char name[32];
	char origin[32];
	char arch[32];
	StaticMapInfo* static_info;
} SupergroupStats;

void sgroup_verifyPermissionTable();
int sgroup_hasPermissionByName( Entity* e, const char *permission);
int sgroup_hasPermission( Entity *e, int permission);
int sgroup_hasStoragePermission( Entity* e, U32 permissions, int amount);
int sgroup_rankHasPermission( Entity* e, int rank, sgroupPermissionEnum permission);

bool sgrpbadge_GetIdxFromName(const char *pch, int *pi);

void sgroup_setDefaultPermissions(Supergroup* sg);

Supergroup *createSupergroup(void);
void destroySupergroup(Supergroup *supergroup);
void clearSupergroup(Supergroup *supergroup);
SupergroupStats *createSupergroupStats(void);
void destroySupergroupStats(SupergroupStats *stats);

SupergroupMemberInfo* createSupergroupMemberInfo(void);
void destroySupergroupMemberInfo(SupergroupMemberInfo* member);

RecipeInvItem *CreateRecipeInvItem(void);
void DestroyRecipeInvItem(RecipeInvItem *p);
RecipeInvItem *sgrp_FindRecipeInvItem(Supergroup *sg, DetailRecipe const *recipe);

// These modify the supergroup recipe list and should only be called inside of a teamlock when used on server
void sgrp_SetRecipeNoLock(Supergroup *sg, DetailRecipe const *recipe, int count, bool isUnlimited); // note: assumes lock already held
void sgrp_AdjRecipeNoLock(Supergroup *sg, DetailRecipe const *recipe, int count, bool isUnlimited); // note: assumes lock already held

void sgrp_TrackDbId(Supergroup *sg, int idSgrp);
void sgrp_UnTrackDbId(int idSgrp);
Supergroup *sgrpFromDbId(int idSgrp);
Supergroup *sgrpFromSgId(int idSgrp);

int sgrp_emptyMounts( Supergroup *sg );

int sgroup_isPlayerDark(Entity *e);

#endif //SUPERGROUP_H
