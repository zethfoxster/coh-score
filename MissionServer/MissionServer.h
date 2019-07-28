#pragma once

#include "MissionSearch.h" // MissionSearchStatus, MissionSearchHeader
#include "net_link.h"

typedef struct StashTableImp* StashTable;
typedef struct PersistConfig PersistConfig;

// We track the first N unique people to complete an arc so we can award the
// 'early bird' badge.
#define MISSION_MAX_UNIQUE_PLAYERS		10
#define MISSIONSERVER_INVALIDATIONCOUNT	(4)	// i was going to make this a live var, but there are issues with this number being reduced
#define MISSIONSERVER_PLAYABLEINVALIDATIONCOUNT	(2)
#define MISSIONSERVER_MAX_KEYWORDS		32

AUTO_ENUM;
typedef enum MissionComplaintType
{
	kMissionComplaint_Complaint,
	kMissionComplaint_Comment,
	kMissionComplaint_BanMessage,
	kMissionComplaint_Count,
}MissionComplaintType;
#define missioncomplaint_toString(type)	StaticDefineIntRevLookup(MissionComplaintTypeEnum, type)


AUTO_STRUCT AST_SINGLELINE;
typedef struct MissionComplaint
{
	U32 complainer_id;
	AuthRegion complainer_region;
	char *comment;
	MissionComplaintType type;
	int bSent;
	char *globalName;
} MissionComplaint;

AUTO_STRUCT AST_SINGLELINE;
typedef struct MissionAuth
{
	U32 id;
	AuthRegion region;
} MissionAuth;



AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End")
AST_IGNORE("TotalStars") AST_IGNORE("totalVotes")
AST_IGNORE("TopKeywords");
typedef struct MissionServerArc
{
	U32 id;								AST(PRIMARY_KEY AUTO_INCREMENT STRUCTPARAM)

	int creator_id;						AST(NAME("Creator")) // user->authid
	AuthRegion creator_region;			AST(NAME("CreatorServer"))

	U32 created;						AST(NAME("Created") FORMAT_DATESS2000)
	U32 updated;						AST(NAME("Updated") FORMAT_DATESS2000)
	U32 pulled;							AST(NAME("Pulled") FORMAT_DATESS2000)
	U32 unpublished;					AST(NAME("Unpublished") FORMAT_DATESS2000)
	U32 invalidated;					AST(NAME("Invalidated") FORMAT_DATESS2000)
	U32 playable_errors;				AST(NAME("PlayableErrors") FORMAT_DATESS2000)
	U32 uncirculated;					AST(NAME("Uncirculated") FORMAT_DATESS2000)

	U8 *data;							NO_AST // persisted in MissionServerArcData.c
	U32 size;							AST(NAME("DataSize"))
	U32 zsize;							AST(NAME("DataZippedSize"))

	int votes[MISSIONRATING_VOTETYPES];	AST(NAME("Votes"))
	int total_votes;					NO_AST //set in s_arcsPostLoad()

	MissionAuth **first_to_complete;	AST(NAME("FirstToComplete"))
	int total_stars;					NO_AST	//set in s_arcsPostLoad()
	int total_completions;				AST(NAME("TotalCompletions"))
	int total_starts;					AST(NAME("TotalStarts"))

	MissionRating rating;				AST(NAME("Rating") DEFAULT(MISSIONRATING_UNRATED))
	MissionHonors honors;				AST(NAME("Honors"))
	U8 permanent;						AST(NAME("Permanent") BOOLFLAG)
	MissionBan ban;						AST(NAME("BanStatus"))
	MissionComplaint **complaints;		AST(NAME("Complaint") REDUNDANT_STRUCT("BanComment", parse_legacy_MissionComplaint))
	int new_complaints;					AST(NAME("NewComplaints"))

	MissionSearchHeader header;			AST(NAME("MetaData"))
	char *header_estr;					NO_AST // cached serialized header

	int keyword_votes[MISSIONSERVER_MAX_KEYWORDS];	AST(NAME("KeyWordVotes"))
	U32 top_keywords;					NO_AST//AST(NAME("TopKeywords"))
	U32 initial_keywords;				AST(NAME("InitialKeywords"))
} MissionServerArc;

// some shorthand for consistent messages
#define ARC_PRINTF_FMT	"%d(%s.%d)"
#define ARC_PRINTF(arc)	arc->id, authregion_toString(arc->creator_region), arc->creator_id

static INLINEDBG int missionserverarc_isIndexed(MissionServerArc *arc) { return !arc->unpublished && arc->invalidated < MISSIONSERVER_INVALIDATIONCOUNT && !arc->uncirculated; }

typedef struct MissionServerInventory
{
	// Persisted, add them to parse_MissionServerInventory[] in MissionServer.c
	U32 tickets;
	U32 lifetime_tickets;
	U32 overflow_tickets;
	U32 created_completions;
	U32 badge_flags;
	U32 best_arc_stars;
	U32 publish_slots;
	U32 uncirculated_slots;

	U32 paid_publish_slots;
	char **costume_keys;

	// Not persisted.
	bool dirty;
} MissionServerInventory;

typedef struct MissionServerKeywordVote
{
	int arcid;
	U32 vote;
}MissionServerKeywordVote;

typedef struct MissionServerUser
{
	int auth_id;
	AuthRegion auth_region;

	char *authname; // for petitions
	int autobans;
	U32 banned_until;

	int *authids_voted;
	AuthRegion *authids_voted_region;
	int *authids_voted_timestamp;

	int *arcids_votes[MISSIONRATING_VOTETYPES]; // structparser doesn't like this one :P
	int *arcids_best_votes[MISSIONRATING_VOTETYPES];
	int *arcids_started;
	int *arcids_started_timestamp;
	int *arcids_played;
	int *arcids_played_timestamp;
	int *arcids_complaints;

	MissionServerKeywordVote **ppKeywordVotes;

	MissionServerInventory inventory;

	int total_votes_earned[MISSIONRATING_VOTETYPES];

	// not persisted
	int *arcids_published;
	int *arcids_uncirculated;
	int *arcids_permanent;
	int *arcids_unpublished;
	int *arcids_invalid;
	int *arcids_playableErrors;

	U32 shard_dbid;
	NetLink *shard_link;
} MissionServerUser;

extern ParseTable parse_MissionServerInventory[];
extern ParseTable parse_MissionServerUser[];

// some shorthand for consistent messages
#define USER_PRINTF_FMT				"%s.%d"
#define USER_PRINTF_EX(id,region)	authregion_toString(region), id
#define USER_PRINTF(user)			USER_PRINTF_EX(user->auth_id, user->auth_region)


AUTO_STRUCT AST_SINGLELINE;
typedef struct MissionServerShard
{
	const char *name;
	char *ip;
	AuthRegion auth_region; // differentiates incoming authids

	// internal (not read from config)
	NetLink link;		NO_AST
	int failed_connect;	NO_AST
	int data_ok;		NO_AST
} MissionServerShard;


AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("");
typedef struct MissionServerCfg
{
	MissionServerShard **shards;	AST(NAME("Shard"))

	PersistConfig *arcpersist;		AST(NAME("ArcDb"))
	PersistConfig *userpersist;		AST(NAME("UserDb"))
	PersistConfig *searchpersist;	AST(NAME("SearchDb"))
	PersistConfig *composedpersist;	AST(NAME("ComposedDb"))
	PersistConfig *substringpersist;	AST(NAME("SubstringDb"))
	PersistConfig *quotedpersist;	AST(NAME("QuotedDb"))
	PersistConfig *varspersist;		AST(NAME("VarsDb"))
	//used for search caching
	S64 memory_reserved;			AST(DEFAULT(((sizeof(void*)==8?2<<30:0))) NAME("ReserveMemory"))//guess for non mission server use.  default 2 gigs
	int maxComposedCaches;			AST(DEFAULT(333) NAME("MaxComposedCaches"))

} MissionServerCfg;


AUTO_STRUCT AST_STARTTOK("") AST_ENDTOK("End");
typedef struct MissionServerVars
{
	int rating_threshold;		AST(DEFAULT(1))
	int instaban_grace_min;		AST(DEFAULT(5)		ADDNAMES("instaban_threshold"))
	F32 instaban_grace_speed;	AST(DEFAULT(5))		// multiplier for additional grace
	F32 instaban_grace_base;	AST(DEFAULT(10))	// log base for additional grace
	int autobans_till_userban;	AST(DEFAULT(5))
	int halloffame_votes;		AST(DEFAULT(1000)	ADDNAMES("halloffame_threshold"))
	F32 halloffame_rating;							// default set in missionserver_load()
	int days_till_unpublish;	AST(DEFAULT(60))	// how long to wait before auto-unpublishing a banned arc
	int days_till_delete;		AST(DEFAULT(90))	// how long to wait before deleting an unpublished arc
	int days_till_revote;		AST(DEFAULT(1))		// can't re-earn tickets from an auth in this time period
	int ticket_arcs_per_auth;	AST(DEFAULT(2))		// how many arcs from an auth you can vote for and they get tickets
	int tickets_0_stars;		AST(DEFAULT(0))
	int tickets_1_star;			AST(DEFAULT(0)		ADDNAMES("tickets_1_stars"))
	int tickets_2_stars;		AST(DEFAULT(0))
	int tickets_3_stars;		AST(DEFAULT(3))
	int tickets_4_stars;		AST(DEFAULT(4))
	int tickets_5_stars;		AST(DEFAULT(5))
} MissionServerVars;


typedef struct MissionServerPetition
{
	char *authname;
	char *character;
	char *summary;
	char *message;
	char *notes;
} MissionServerPetition;

typedef struct MissionServerState
{
	MissionServerCfg cfg;		// Read via TextParser
	MissionServerVars *vars;	// Managed by the missionserver persist layer

	char cmd_line[1024];
	char exe_name[MAX_PATH];
	char dir[MAX_PATH];

	int tick_timer;
	int tick_count;
	int packet_timer;
	int packet_count;

	MissionServerShard **shards;	// shallow copy of cfg.shards
	int shard_count;
	int next_retry_shard;
	U32 last_failed_time;
	StashTable shardsByName;
	U32 forceLoadDb:1;
	U32 dumpStats:1;

	MissionServerPetition *petitions[AUTHREGION_COUNT];	// for petitions when a link isn't readily available.

} MissionServerState;

extern MissionServerState g_missionServerState;
void missionserver_updateLegacyTimeStamps(MissionServerUser *user, int journalChange);
//#define TEST 1
//#define CORRECTNESS_TEST 1

#include "AutoGen/MissionServer_h_ast.h"
