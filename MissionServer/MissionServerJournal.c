#include "MissionServerJournal.h"
#include "MissionServer.h"
#include "persist.h"
#include "utils.h"
#include "earray.h"
#include "error.h"
#include "autogen/missionserverjournal_c_ast.h"
#include "mathutil.h"
#include "log.h"

// FIXME: the persist layer should allow journaling across types (and other cross-type communication)

AUTO_ENUM;
typedef enum ArcJournalType
{
	ARCJOURNAL_UNPUBLISH,
	ARCJOURNAL_INVALIDATE,
	ARCJOURNAL_HONORS,
	ARCJOURNAL_BANPULLED,
	ARCJOURNAL_VOTESRATING,
	ARCJOURNAL_TOTALCOMPLETIONS,
	ARCJOURNAL_KEYWORDS,
	ARCJOURNAL_PLAYABLEINVALIDATE,
	ARCJOURNAL_TOTALSTARTS,
	ARCJOURNAL_TYPES
} ArcJournalType;

AUTO_STRUCT;
typedef struct ArcJournal
{
	int arcid; // TODO: the persist layer could manage finding the object instead
	ArcJournalType type;
	U32 time;
	MissionRating rating;
	MissionHonors honors;
	MissionBan ban;
	int votes[MISSIONRATING_VOTETYPES]; AST(RAW) // not human readable, but smaller and not written if it's empty
	int completions;
	int keyword_votes[MISSIONSERVER_MAX_KEYWORDS];
	int top_keywords;
	int starts;
} ArcJournal;


static int s_playbackArcJournal(ArcJournal *journal);

int missionserver_registerJournaledArcPersist(PersistConfig *config)
{
	return plRegisterJournaled(	MissionServerArc, parse_MissionServerArc,
								parse_ArcJournal, sizeof(ArcJournal),
								s_playbackArcJournal, config, PERSIST_ORDERED );
}


void missionserver_journalArc(MissionServerArc *arc)
{
	plDirty(MissionServerArc, arc);
}

void missionserver_journalArcUnpublish(MissionServerArc *arc)
{
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_UNPUBLISH;
	journal.arcid = arc->id;
	journal.time = arc->unpublished;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcInvalidate(MissionServerArc *arc)
{
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_INVALIDATE;
	journal.arcid = arc->id;
	journal.time = arc->invalidated;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcPlayableInvalidate(MissionServerArc *arc)
{
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_PLAYABLEINVALIDATE;
	journal.arcid = arc->id;
	journal.time = arc->playable_errors;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcHonors(MissionServerArc *arc)
{
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_HONORS;
	journal.arcid = arc->id;
	journal.honors = arc->honors;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcBanPulled(MissionServerArc *arc)
{
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_BANPULLED;
	journal.arcid = arc->id;
	journal.ban = arc->ban;
	journal.time = arc->pulled;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcVotesRating(MissionServerArc *arc)
{
	int i;
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_VOTESRATING;
	journal.arcid = arc->id;
	for(i = 0; i < MISSIONRATING_VOTETYPES; i++)
		journal.votes[i] = arc->votes[i];
	journal.rating = arc->rating;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcKeywords(MissionServerArc *arc)
{
	int i;
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_KEYWORDS;
	journal.arcid = arc->id;
	for( i = 0; i < MISSIONSERVER_MAX_KEYWORDS; i++  )
		journal.keyword_votes[i] = arc->keyword_votes[i];
	journal.top_keywords = arc->top_keywords;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcTotalCompletions(MissionServerArc *arc)
{
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_TOTALCOMPLETIONS;
	journal.arcid = arc->id;
	journal.completions = arc->total_completions;
	plJournal(MissionServerArc, arc, &journal);
}

void missionserver_journalArcTotalStarts(MissionServerArc *arc)
{
	ArcJournal journal = {0};
	journal.type = ARCJOURNAL_TOTALSTARTS;
	journal.arcid = arc->id;
	journal.starts = arc->total_starts;
	plJournal(MissionServerArc, arc, &journal);
}

static int s_playbackArcJournal(ArcJournal *journal)
{
	MissionServerArc *arc = plFind(MissionServerArc, journal->arcid);
	if(!arc || !INRANGE0(journal->type, ARCJOURNAL_TYPES))
		return 0;

	switch(journal->type)
	{
		//remember to update here, then change the static assert
		STATIC_ASSERT(ARCJOURNAL_TYPES == 9);

		xcase ARCJOURNAL_UNPUBLISH:
			arc->unpublished = journal->time;

		xcase ARCJOURNAL_INVALIDATE:
			arc->invalidated = journal->time;

		xcase ARCJOURNAL_PLAYABLEINVALIDATE:
			arc->playable_errors = journal->time;

		xcase ARCJOURNAL_HONORS:
			arc->honors = journal->honors;
			if(missionhonors_isPermanent(arc->honors))
				arc->permanent = 1;

		xcase ARCJOURNAL_BANPULLED:
			arc->ban = journal->ban;
			arc->pulled = journal->time;

		xcase ARCJOURNAL_VOTESRATING:
		{
			int i;
			arc->total_votes = 0;
			arc->total_stars = 0;
			for(i = 0; i < MISSIONRATING_VOTETYPES; i++)
				arc->votes[i] = journal->votes[i];
			arc->rating = journal->rating;
		}
		xcase ARCJOURNAL_KEYWORDS:
		{
			int i;
			arc->top_keywords = journal->top_keywords;
			for( i = 0; i < MISSIONSERVER_MAX_KEYWORDS; i++  )
				arc->keyword_votes[i] = journal->keyword_votes[i]; 
		}
		xcase ARCJOURNAL_TOTALCOMPLETIONS:
		{
			arc->total_completions = journal->completions;
		}
		xcase ARCJOURNAL_TOTALSTARTS:
		{
			arc->total_starts = journal->starts;
		}
	}

	return 1;
}

AUTO_ENUM;
typedef enum UserJournalType
{
	USERJOURNAL_COMPLAINT,
	USERJOURNAL_BAN,
	USERJOURNAL_VOTE,
	USERJOURNAL_BEST_VOTE,
	USERJOURNAL_INVENTORY,
	USERJOURNAL_AUTHID_VOTED,
	USERJOURNAL_TOTAL_VOTES_EARNED,
	USERJOURNAL_ARC_COMPLETED,
	USERJOURNAL_DELETE_VOTE,
	USERJOURNAL_DELETE_BEST_VOTE,
	USERJOURNAL_DELETE_AUTHID_VOTED,
	USERJOURNAL_KEYWORD,
	USERJOURNAL_ARC_COMPLETED_TIMESTAMP_UPDATE,
	USERJOURNAL_ARC_STARTED,
	USERJOURNAL_ARC_STARTED_TIMESTAMP_UPDATE,
	USERJOURNAL_TYPES
} UserJournalType;

AUTO_STRUCT;
typedef struct UserJournal
{
	int auth_id;
	AuthRegion auth_region;
	UserJournalType type;

	int authid_voted;
	AuthRegion authid_voted_region;
	int authid_voted_timestamp;
	int arcid;
	int arcid_played_timestamp;
	int arcid_started_timestamp;
	U32 banned_until;
	int autobans;
	MissionRating vote;
	int	keywords;	AST(NAME("KeywordVote"))
	MissionServerInventory inventory; AST(STRUCT(parse_MissionServerInventory))
	MissionServerKeywordVote ppKeywords; AST(STRUCT(parse_MissionServerKeywordVote))

	
	// These are separate because autostruct will choke on them if they're
	// made into an array.
	int total_votes0;
	int total_votes1;
	int total_votes2;
	int total_votes3;
	int total_votes4;
	int total_votes5;
} UserJournal;


static int s_playbackUserJournal(UserJournal *journal);

int missionserver_registerJournaledUserPersist(PersistConfig *config)
{
	return plRegisterJournaled(	MissionServerUser, parse_MissionServerUser,
								parse_UserJournal, sizeof(UserJournal),
								s_playbackUserJournal, config, PERSIST_STRICTDEFAULT );
}


void missionserver_journalUser(MissionServerUser *user)
{
	plDirty(MissionServerUser, user);
}

void missionserver_journalUserComplaint(MissionServerUser *user, int arcid)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_COMPLAINT;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserBan(MissionServerUser *user)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_BAN;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.banned_until = user->banned_until;
	journal.autobans = user->autobans;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserArcVote(MissionServerUser *user, int arcid, MissionRating vote)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_VOTE;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	journal.vote = vote;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserArcKeywordVote(MissionServerUser *user, int arcid, int keywords)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_KEYWORD;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	journal.keywords = keywords;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserArcBestVote(MissionServerUser *user, int arcid, MissionRating bestvote)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_BEST_VOTE;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	journal.vote = bestvote;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserInventory(MissionServerUser *user)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_INVENTORY;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	StructCopy(parse_MissionServerInventory, &(user->inventory), &(journal.inventory), 0, 0);
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserRecentUserVote(MissionServerUser *user, int authid, AuthRegion authregion, U32 timestamp)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_AUTHID_VOTED;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.authid_voted = authid;
	journal.authid_voted_region = authregion;
	journal.authid_voted_timestamp = timestamp;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserTotalVotesEarned(MissionServerUser *user)
{
	UserJournal journal = {0};

	STATIC_ASSERT(MISSIONRATING_VOTETYPES == 6);

	journal.type = USERJOURNAL_TOTAL_VOTES_EARNED;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.total_votes0 = user->total_votes_earned[0];
	journal.total_votes1 = user->total_votes_earned[1];
	journal.total_votes2 = user->total_votes_earned[2];
	journal.total_votes3 = user->total_votes_earned[3];
	journal.total_votes4 = user->total_votes_earned[4];
	journal.total_votes5 = user->total_votes_earned[5];
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserArcCompleted(MissionServerUser *user, int arcid, U32 timestamp)
{
	UserJournal journal = {0};

	journal.type = USERJOURNAL_ARC_COMPLETED;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	journal.arcid_played_timestamp = timestamp;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserArcCompletedUpdate(MissionServerUser *user, int arcid, U32 timestamp)
{
	UserJournal journal = {0};

	journal.type = USERJOURNAL_ARC_COMPLETED_TIMESTAMP_UPDATE;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	journal.arcid_played_timestamp = timestamp;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserArcStarted(MissionServerUser *user, int arcid, U32 timestamp)
{
	UserJournal journal = {0};

	journal.type = USERJOURNAL_ARC_STARTED;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	journal.arcid_started_timestamp = timestamp;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserArcStartedUpdate(MissionServerUser *user, int arcid, U32 timestamp)
{
	UserJournal journal = {0};

	journal.type = USERJOURNAL_ARC_STARTED_TIMESTAMP_UPDATE;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	journal.arcid_started_timestamp = timestamp;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserDeleteArcVote(MissionServerUser *user, int arcid)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_DELETE_VOTE;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserDeleteArcBestVote(MissionServerUser *user, int arcid)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_DELETE_BEST_VOTE;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.arcid = arcid;
	plJournal(MissionServerUser, user, &journal);
}

void missionserver_journalUserDeleteUserVote(MissionServerUser *user, int authid, int authregion)
{
	UserJournal journal = {0};
	journal.type = USERJOURNAL_DELETE_AUTHID_VOTED;
	journal.auth_id = user->auth_id;
	journal.auth_region = user->auth_region;
	journal.authid_voted = authid;
	journal.authid_voted_region = authregion;
	plJournal(MissionServerUser, user, &journal);
}

static int s_playbackUserJournal(UserJournal *journal)
{
	MissionServerUser *user = plFind(MissionServerUser, journal->auth_id, journal->auth_region);
	if(!user || !INRANGE0(journal->type, USERJOURNAL_TYPES))
		return 0;

	switch(journal->type)
	{
		STATIC_ASSERT(USERJOURNAL_TYPES == 15);

		xcase USERJOURNAL_COMPLAINT:
			if(eaiSortedFind(&user->arcids_complaints, journal->arcid) >= 0)
				LOG( LOG_MISSIONSERVER, LOG_LEVEL_VERBOSE, 0, "Warning: duplicate complaint on user "USER_PRINTF_FMT" for arcid %d", USER_PRINTF(user), journal->arcid);
			eaiSortedPushUnique(&user->arcids_complaints, journal->arcid);

		xcase USERJOURNAL_BAN:
			user->banned_until = journal->banned_until;
			user->autobans = journal->autobans;

		xcase USERJOURNAL_VOTE:
		{
			int i;
			for(i = 0; i < MISSIONRATING_VOTETYPES; i++) // remove existing vote
				eaiSortedFindAndRemove(&user->arcids_votes[i], journal->arcid);

			eaiSortedPush(&user->arcids_votes[journal->vote], journal->arcid);
		}
		xcase USERJOURNAL_KEYWORD:
		{
			int i;
			MissionServerKeywordVote *pVote;
			for( i = 0; i < eaSize(&user->ppKeywordVotes); i++ )
			{
				if(user->ppKeywordVotes[i]->arcid == journal->arcid )
				{
					user->ppKeywordVotes[i]->vote = journal->keywords;
					break;
				}
			}
			
			pVote = StructAllocRaw(sizeof(MissionServerKeywordVote));
			pVote->arcid = journal->arcid;
			pVote->vote = journal->keywords;
			eaPush(&user->ppKeywordVotes, pVote);
		}
		xcase USERJOURNAL_BEST_VOTE:
		{
			int i;
			for(i = 0; i < MISSIONRATING_VOTETYPES; i++)
			{
				if(eaiSortedFindAndRemove(&user->arcids_best_votes[i], journal->arcid) >= 0)
					break;
			}
			eaiSortedPush(&user->arcids_best_votes[journal->vote], journal->arcid);
		}
		xcase USERJOURNAL_INVENTORY:
			StructCopy(parse_MissionServerInventory, &(journal->inventory), &(user->inventory), 0, 0);
		xcase USERJOURNAL_AUTHID_VOTED:
		{
			int i;
			bool found = false;

			// If the voted-on user is already in the list (region must match
			// as well as authname hash!) we can just update the timestamp.
			for(i = 0; i < eaiSize(&user->authids_voted); i++)
			{
				if(user->authids_voted[i] == journal->authid_voted &&
					user->authids_voted[i] == journal->authid_voted_region)
				{
					user->authids_voted_timestamp[i] = journal->authid_voted_timestamp;
					found = true;
					break;
				}
			}

			// We scanned the whole array and didn't find a matching entry so
			// this is a new auth they voted on (for them).
			if(!found)
			{
				eaiPush(&user->authids_voted, journal->authid_voted);
				eaiPush((int**)(&user->authids_voted_region), journal->authid_voted_region);
				eaiPush(&user->authids_voted_timestamp, journal->authid_voted_timestamp);
			}
		}
		xcase USERJOURNAL_TOTAL_VOTES_EARNED:
		{
			user->total_votes_earned[0] = journal->total_votes0;
			user->total_votes_earned[1] = journal->total_votes1;
			user->total_votes_earned[2] = journal->total_votes2;
			user->total_votes_earned[3] = journal->total_votes3;
			user->total_votes_earned[4] = journal->total_votes4;
			user->total_votes_earned[5] = journal->total_votes5;
		}
		xcase USERJOURNAL_ARC_STARTED:
		{
			int index = eaiSortedPushUnique(&user->arcids_started, journal->arcid);
			eaiInsert(&user->arcids_started_timestamp, journal->arcid_started_timestamp, index);
		}
		xcase USERJOURNAL_ARC_STARTED_TIMESTAMP_UPDATE:
		{
			int index = eaiSortedFind(&user->arcids_started, journal->arcid);
			assert(index >= 0);
			if (index >= 0)
			{
				user->arcids_started_timestamp[index] = MAX(user->arcids_started_timestamp[index], journal->arcid_started_timestamp);
			}
		}
		xcase USERJOURNAL_ARC_COMPLETED:
		{
			int index = 0;
			missionserver_updateLegacyTimeStamps(user, 0);		//	has to be before the push
			index = eaiSortedPushUnique(&user->arcids_played, journal->arcid);
			eaiInsert(&user->arcids_played_timestamp, journal->arcid_played_timestamp, index);
		}
		xcase USERJOURNAL_ARC_COMPLETED_TIMESTAMP_UPDATE:
		{
			int index = 0;
			missionserver_updateLegacyTimeStamps(user, 0);
			index = eaiSortedFind(&user->arcids_played, journal->arcid);
			assert(index >= 0);
			if (index >= 0)
			{
				user->arcids_played_timestamp[index] = MAX(user->arcids_played_timestamp[index], journal->arcid_played_timestamp);
			}
		}
		xcase USERJOURNAL_DELETE_VOTE:
		{
			int i;
			for(i = 0; i < MISSIONRATING_VOTETYPES; i++)
			{
				if(eaiSortedFindAndRemove(&user->arcids_votes[i], journal->arcid) >= 0)
					break;
			}
		}
		xcase USERJOURNAL_DELETE_BEST_VOTE:
		{
			int i;
			for(i = 0; i < MISSIONRATING_VOTETYPES; i++)
			{
				if(eaiSortedFindAndRemove(&user->arcids_best_votes[i], journal->arcid) >= 0)
					break;
			}
		}
		xcase USERJOURNAL_DELETE_AUTHID_VOTED:
		{
			int i;
			for(i = 0; i < eaiSize(&user->authids_voted); i++)
			{
				if(user->authids_voted[i] == journal->authid_voted && user->authids_voted_region[i] == journal->authid_voted_region)
				{
					eaiRemove(&user->authids_voted, i);
					eaiRemove((int**)(&user->authids_voted_region), i);
					eaiRemove(&user->authids_voted_timestamp, i);
					break;
				}
			}
		}
	}

	return 1;
}

#include "autogen/missionserverjournal_c_ast.c"
