#pragma once

#include "gametypes.h" // MAX_NAME_LEN

#define MISSIONSEARCH_MAX_VILLAINGROUPS		1000	//I need to know how many villain groups we have for search. "I hope we have less than 1000 villain groups"
#define MISSIONSEARCH_AUTHORLEN				32
#define MISSIONSEARCH_AUTHORSHARDLEN		16
#define MISSIONSEARCH_TITLELEN				128
#define MISSIONSEARCH_TOTALTEXTPARTLEN		512
#define MISSIONSEARCH_SUMMARYLEN			(MISSIONSEARCH_TOTALTEXTPARTLEN - MISSIONSEARCH_AUTHORLEN - MISSIONSEARCH_AUTHORSHARDLEN - MISSIONSEARCH_TITLELEN)
#define MISSIONSEARCH_MAX_MISSIONS			3
#define MISSIONSEARCH_MAX_UNCIRCULATED		6
#define MISSIONSEARCH_VG_BITFIELDSIZE		((MISSIONSEARCH_MAX_VILLAINGROUPS+31)/32)	// VG_MAX is determined on the fly, I hope we have less than 1000 villain groups
#define MISSIONSEARCH_MAX_SEARCH_STRING		128
#define MISSIONSEARCH_ARCDATA_VIEWONLY		2		// 0 play, 1 test
#define MISSIONSEARCH_ARCDATA_MAPTEST		3
#define MISSIONSEARCH_MAX_LEVEL				54 


// this one should probably go somewhere else...
AUTO_ENUM;
typedef enum AuthRegion
{
	AUTHREGION_DEV,
	AUTHREGION_WW, // world wide
	AUTHREGION_COUNT
} AuthRegion;
#define AUTHREGION_ALL ((1<<AUTHREGION_COUNT)-1)
#define authregion_toString(auth_region)	StaticDefineIntRevLookup(AuthRegionEnum, auth_region)


typedef enum MissionSearchPage
{
	MISSIONSEARCH_NOPAGE = -1,

	// architect browser tabs
	MISSIONSEARCH_ALL,
	MISSIONSEARCH_MYARCS,
	MISSIONSEARCH_ARC_AUTHOR,

	// cs slash commands
	MISSIONSEARCH_CSR_AUTHORED,
	MISSIONSEARCH_CSR_COMPLAINED,
	MISSIONSEARCH_CSR_VOTED,
	MISSIONSEARCH_CSR_PLAYED,

	MISSIONSEARCH_PAGETYPES
} MissionSearchPage;

//The following can be included in the searchParams to order search results.
//This enum is also used for breaking up a search into its component parts.
//Because of this, MSSEARCH_TEXT is necessary for breaking up the search,
//but using it int the MissionSearchParameters will organize your results by
//the words in your search rather than alphabetical order.
typedef enum MissionServerSearchType {
	MSSEARCH_ALL_ARC_IDS = 0,
	MSSEARCH_HONORS,
	MSSEARCH_RATING,
	MSSEARCH_ARC_LENGTH,
	MSSEARCH_MORALITY,
	MSSEARCH_LOCALE,
	MSSEARCH_BAN,
	MSSEARCH_MISSION_TYPE,
	MSSEARCH_VILLAIN_GROUPS,
	MSSEARCH_TEXT,
	MSSEARCH_AUTHOR,
	MSSEARCH_STATUS,
	MSSEARCH_KEYWORDS,
	MSSEARCH_COUNT,
} MissionServerSearchType;
extern int g_missionsearchpage_accesslevel[MISSIONSEARCH_PAGETYPES];


AUTO_STRUCT;
typedef struct MissionSearchParams
{
	char *text;
	char *author;
	int author_id;

	U32 villainGroups[MISSIONSEARCH_VG_BITFIELDSIZE]; AST(RAW)
	int arcid;
	MissionServerSearchType sort;

	// Bitfields
	U32 rating;
	U32 honors;
	U32 ban;
	U32 morality;
	U32 arcLength;
	U32 missionTypes;
	U32 locale_id;
	U32 status;
	U32 keywords;
} MissionSearchParams;

AUTO_STRUCT;
typedef struct MissionHeaderDescription
{
	int detailTypes;
	int min;
	int max;
	int size;
} MissionHeaderDescription;

typedef enum MissionHeaderHazards
{
	HAZARD_ARCHVILLAIN,
	HAZARD_ELITEBOSS,
	HAZARD_EXTREME_ARCHVILLAIN,
	HAZARD_EXTREME_ELITEBOSS,
	HAZARD_EXTREME_BOSS,
	HAZARD_EXTREME_LT,
	HAZARD_EXTREME_MINIONS,
	HAZARD_HIGHLEVEL_DOWN,
	HAZARD_CUSTOM_POWERS,
	HAZARD_GIANT_MONSTER,
	HAZARD_COUNT,
}MissionHeaderHazards;

// MissionSearchHeader contains static information on an arc.
// Its contents only change when the arc is republished or edited.
AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End")
AST_IGNORE("minlevel") AST_IGNORE("maxlevel")
AST_IGNORE("levelRangeLow") AST_IGNORE("levelRangeHigh");
typedef struct MissionSearchHeader
{
	char author[MISSIONSEARCH_AUTHORLEN];
	char author_shard[MISSIONSEARCH_AUTHORSHARDLEN];
	char title[MISSIONSEARCH_TITLELEN];
	char summary[MISSIONSEARCH_SUMMARYLEN];
	char *guestAuthorBio;	//Don't waste space for everyone since only a few authors will get this.

	
	int morality;
	int arcLength;
	U32 date_created;
	int missionType;
	U32 villaingroups[MISSIONSEARCH_VG_BITFIELDSIZE];
	int locale_id;
	int missionCount;
	int hazards;
	int arcStatus;
	int levelRangeLow; NO_AST
	int levelRangeHigh; NO_AST
	MissionHeaderDescription **missionDesc;

	//totaled from mission characteristics
	//TODO: change MISSIONSEARCH_MAX_MISSIONS to already existing (?) enum
} MissionSearchHeader;

AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End");
typedef struct MissionSearchGuestBio
{
	char *imageName;
	char *bio;
} MissionSearchGuestBio;

AUTO_ENUM;
typedef enum MissionRating
{
	MISSIONRATING_0_STARS,
	MISSIONRATING_1_STAR, // :P
	MISSIONRATING_1_STARS = MISSIONRATING_1_STAR,
	MISSIONRATING_2_STARS,
	MISSIONRATING_3_STARS,
	MISSIONRATING_4_STARS,
	MISSIONRATING_5_STARS,
	MISSIONRATING_MAX_STARS_PROMOTED,
	MISSIONRATING_UNRATED,
	MISSIONRATING_RATINGTYPES,
	MISSIONRATING_VOTETYPES = MISSIONRATING_MAX_STARS_PROMOTED,
	MISSIONRATING_MAX_STARS = MISSIONRATING_VOTETYPES-1,
} MissionRating;
#define MISSIONRATING_ALL ((1<<MISSIONRATING_RATINGTYPES)-1)
#define MISSIONRATING_ALL_SEARCHED (MISSIONRATING_ALL & ~(1<<MISSIONRATING_UNRATED))

#define MAPLENGTH_ALL ((1<<4)-1)

AUTO_ENUM;
typedef enum MissionHonors
{
	MISSIONHONORS_NONE,
	MISSIONHONORS_FORMER_DC,
	MISSIONHONORS_FORMER_DC_HOF,
	MISSIONHONORS_HALLOFFAME,
	MISSIONHONORS_DEVCHOICE,
	MISSIONHONORS_DC_AND_HOF,
	MISSIONHONORS_CELEBRITY,
	MISSIONHONORS_TYPES
} MissionHonors;

#define MISSIONHONORS_ALL ((1<<MISSIONHONORS_TYPES)-1)
static INLINEDBG int missionhonors_isInvalidationImmune(U32 honors)
{
	int immune = honors >=MISSIONHONORS_DEVCHOICE;
	return immune;
}

AUTO_ENUM;
typedef enum MissionBan
{
	MISSIONBAN_NONE,
	MISSIONBAN_PULLED,
	MISSIONBAN_SELFREVIEWED,
	MISSIONBAN_SELFREVIEWED_PULLED,
	MISSIONBAN_SELFREVIEWED_SUBMITTED,
	MISSIONBAN_CSREVIEWED_BANNED,
	MISSIONBAN_CSREVIEWED_APPROVED,		ENAMES(CSREVIEWED_APPROVED CSREVIEWED_OK)
	MISSIONBAN_TYPES
} MissionBan;

#define MISSIONBAN_ALL ((1<<MISSIONBAN_TYPES)-1)

#define missionrating_toString(rating)	StaticDefineIntRevLookup(MissionRatingEnum, rating)
#define missionhonors_toString(honors)	StaticDefineIntRevLookup(MissionHonorsEnum, honors)
#define missionban_toString(ban)		StaticDefineIntRevLookup(MissionBanEnum, ban)

static INLINEDBG int missionrating_cmp(MissionRating a, MissionRating b)
{
	return (a == MISSIONRATING_UNRATED ? -1 : a) - (b == MISSIONRATING_UNRATED ? -1 : b);
}

static INLINEDBG int missionhonors_isPermanent(MissionHonors honors)
{
	return honors != MISSIONHONORS_NONE;
}

static INLINEDBG int missionban_isVisible(MissionBan ban)
{
	return	ban != MISSIONBAN_PULLED &&
			ban != MISSIONBAN_SELFREVIEWED_PULLED &&
			ban != MISSIONBAN_SELFREVIEWED_SUBMITTED &&
			ban != MISSIONBAN_CSREVIEWED_BANNED;
}

static INLINEDBG int missionban_locksSlot(MissionBan ban)
{
	return	ban == MISSIONBAN_SELFREVIEWED_PULLED ||
			ban == MISSIONBAN_SELFREVIEWED_SUBMITTED ||
			ban == MISSIONBAN_CSREVIEWED_BANNED;
}

static INLINEDBG void missionban_removeInvisible(U32 *ban)
{
	(*ban) &= ~((1<<MISSIONBAN_PULLED)|(1<<MISSIONBAN_SELFREVIEWED_PULLED)
		|(1<<MISSIONBAN_SELFREVIEWED_SUBMITTED)|(1<<MISSIONBAN_CSREVIEWED_BANNED));
}

static INLINEDBG void missionban_setVisible(U32 *ban)
{
	(*ban) |= (MISSIONBAN_ALL & (~((1<<MISSIONBAN_PULLED)|(1<<MISSIONBAN_SELFREVIEWED_PULLED)
		|(1<<MISSIONBAN_SELFREVIEWED_SUBMITTED)|(1<<MISSIONBAN_CSREVIEWED_BANNED))));
}

typedef enum MissionSearchCharactersAllowed
{
	MSSEARCHCHARS_ALHA_NUMERIC = 0,
	MSSEARCHCHARS_ALHA_NUMERIC_PUNCTUATION,
	MSSEARCHCHARS_NONSPACE,
	MSSEARCHCHARS_ALL,	//this means that quoted searches are handled by substrings
	MSSEARCHCHARS_COUNT
}MissionSearchCharactersAllowed;

void missionsearch_formatSearchString(char **estring, int allowQuoted);
void missionsearch_getUniqueTokens(char **estring, char ***tokens, int allowQuoted, int alphabetize);
void missionsearch_stripNonSearchCharacters(char **, int length, MissionSearchCharactersAllowed rule, int allowQuotes);

// MissionSearchInfo contains dynamic information on an arc.
// It can change, for example, whenever someone votes on an arc.
typedef union MissionSearchInfo
{
	struct {
		MissionRating rating	: 6;
		MissionHonors honors	: 6;
		MissionBan ban			: 6;
		U32 permanent			: 1;
		U32 unpublished			: 1;
		U32 invalidated			: 1;
		U32 playableErrors		: 1;
	};
	U32 i;
} MissionSearchInfo;
STATIC_ASSERT(sizeof(MissionSearchInfo)==sizeof(U32)); // the networking code assumes this
STATIC_ASSERT(MISSIONRATING_RATINGTYPES < 32); // 6 signed bits
STATIC_ASSERT(MISSIONHONORS_TYPES < 32); // 6 signed bits
STATIC_ASSERT(MISSIONBAN_TYPES < 32); // 6 signed bits

typedef enum MissionSearchViewedFilter
{
	kMissionSearchViewedFilter_All = 0,
	kMissionSearchViewedFilter_Played_Show,
	kMissionSearchViewedFilter_Played_NoShow,
	kMissionSearchViewedFilter_Voted_Show,
	kMissionSearchViewedFilter_Voted_NoShow,
	kMissionSearchViewedFilter_Level,
	kMissionSearchViewedFilter_Random,
	kMissionSearchViewedFilter_Count
}MissionSearchViewedFilter;

// These are all account-wide badges, mostly gained by others playing your
// storyarcs.
typedef enum MissionServerBadgeFlags
{
	// 0 through 4 are free to use. I'm not adjusting the others so we keep
	// database compatibility.
	kBadgeArcDevChoice				= (1 << 5),
	kBadgeArcHallOfFame				= (1 << 6),
	kBadgeArcVoteRating1			= (1 << 7),
	kBadgeArcVoteRating2			= (1 << 8),
	kBadgeArcVoteRating3			= (1 << 9),
	kBadgeArcVoteRating4			= (1 << 10),
	kBadgeArcVoteRating5			= (1 << 11),
	kBadgeArcTotalStars100			= (1 << 12),
	kBadgeArcTotalStars250			= (1 << 13),
	kBadgeArcTotalStars500			= (1 << 14),
	kBadgeArcTotalStars1000			= (1 << 15),
	kBadgeArcTotalStars2000			= (1 << 16),
	kBadgeArcFirstXPlayers			= (1 << 17),
	kBadgeArcCustomBossPublish		= (1 << 18),
} MissionServerBadgeFlags;

typedef enum MissionServerPurchaseFlags
{
	kPurchaseUnlockKey				= (1 << 0),
	kPurchasePublishSlots			= (1 << 1),
	kPurchaseFail					= (1 << 2),
} MissionServerPurchaseFlags;

typedef enum MissionServerPublishFlags
{
	kPublishCustomBoss				= (1 << 0),
} MissionServerPublishFlags;

char* missionMakerPath(void);
char *getMissionPath(char * filename);
char* missionMakerExt(void);

#define MSSEARCH_QUOTED_ALLOWED 0
#define MSSEARCH_CHARACTER_RULE MSSEARCHCHARS_ALHA_NUMERIC_PUNCTUATION

#if !defined(NO_TEXT_PARSER) && !defined(UNITTEST)
#include "AutoGen/MissionSearch_h_ast.h"
#endif
