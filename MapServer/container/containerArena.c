/*\
 *
 *	containerArena.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Structure defs and container pack/unpack for arena stuff
 *
 */


#include "containerArena.h"
#include "dbcomm.h"
#include "container/dbcontainerpack.h"
#include "file.h"
#include "earray.h"
#include "gametypes.h"
#include "comm_backend.h"

// *********************************************************************************
//  Arena events
// *********************************************************************************

LineDesc participant_line_desc[] =
{
	{{ PACKTYPE_CONREF,		CONTAINER_ENTS,			"DbId",				OFFSET(ArenaParticipant, dbid)				},
		"dbid of this participant"},

	{{ PACKTYPE_ATTR,	MAX_ATTRIBNAME_LEN,		"Archetype",		OFFSET(ArenaParticipant, archetype)			},
		"this participant's archetype"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Level",			OFFSET(ArenaParticipant, level)				},
		"this participants level"},

	{{ PACKTYPE_CONREF,		CONTAINER_SUPERGROUPS,			"SgId",				OFFSET(ArenaParticipant, sgid)				},
		"id of this participant's supergroup, only used in supergroup events"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"SgLeader",			OFFSET(ArenaParticipant, sgleader)			},
		"leader of this participant's supergroup"},


	{{ PACKTYPE_INT,		SIZE_INT32,			"Side",				OFFSET(ArenaParticipant, side)				},
		"team this participant is on"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Paid",				OFFSET(ArenaParticipant, paid)				},
		"whether this participant has paid his entryfee"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Entryfee",			OFFSET(ArenaParticipant, entryfee)			},
		"entryfee for this event"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Dropped",			OFFSET(ArenaParticipant, dropped)			},
		"flag that's set when the participant has dropped from the event"},


	// assumes ARENA_MAX_ROUNDS == 11
	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat0",			OFFSET(ArenaParticipant, seats[0])			},
		"which battle this participant was in in round 0"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat1",			OFFSET(ArenaParticipant, seats[1])			},
		"which battle this participant was in in round 1"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat2",			OFFSET(ArenaParticipant, seats[2])			},
		"which battle this participant was in in round 2"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat3",			OFFSET(ArenaParticipant, seats[3])			},
		"which battle this participant was in in round 3"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat4",			OFFSET(ArenaParticipant, seats[4])			},
		"which battle this participant was in in round 4"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat5",			OFFSET(ArenaParticipant, seats[5])			},
		"which battle this participant was in in round 5"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat6",			OFFSET(ArenaParticipant, seats[6])			},
		"which battle this participant was in in round 6"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat7",			OFFSET(ArenaParticipant, seats[7])			},
		"which battle this participant was in in round 7"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat8",			OFFSET(ArenaParticipant, seats[8])			},
		"which battle this participant was in in round 8"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat9",			OFFSET(ArenaParticipant, seats[9])			},
		"which battle this participant was in in round 9"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Seat10",			OFFSET(ArenaParticipant, seats[10])			},
		"which battle this participant was in in round 10"},


	{{ PACKTYPE_INT,		SIZE_INT32,			"RoundLastFloated", OFFSET(ArenaParticipant, roundlastfloated)	},
		"last round this participant went to the next group up in a swiss draw tournament"},

	{ 0 },
};
StructDesc participant_desc[] =
{
	sizeof(ArenaParticipant),
	{AT_EARRAY, OFFSET(ArenaEvent, participants)},
	participant_line_desc,
	"TODO"
};

LineDesc seating_line_desc[] =
{
	{{ PACKTYPE_INT,		SIZE_INT32,			"MapId",		OFFSET(ArenaSeating, mapid)			},
		"map used for this seat"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"Round",		OFFSET(ArenaSeating, round)			},
		"round this seat is for"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"WinningSide",	OFFSET(ArenaSeating, winningside)	},
		"who won this seat"},

	{{ PACKTYPE_INT,		SIZE_INT32,			"MatchTime",	OFFSET(ArenaSeating, matchtime)		},
		"time this match ended"},

	{ 0 },
};
StructDesc seating_desc[] =
{
	sizeof(ArenaSeating),
	{AT_EARRAY, OFFSET(ArenaEvent, seating)},
	seating_line_desc,
	"TODO"
};

LineDesc arenaevent_line_desc[] =
{
	{{ PACKTYPE_INT,			SIZE_INT32,							"UniqueId",			OFFSET(ArenaEvent, uniqueid)			},
		"this event's unique id"},


	{{ PACKTYPE_CONREF,			CONTAINER_ENTS,						"LastUpdateDbid",	OFFSET(ArenaEvent, lastupdate_dbid)		},
		"dbid of last person to update the event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LastUpdateCmd",	OFFSET(ArenaEvent, lastupdate_cmd)		},
		"last command issued regarding this event"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, lastupdate_name), "LastUpdateName",	OFFSET(ArenaEvent, lastupdate_name)		},
		"name of last person to update the event"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"EventType",		OFFSET(ArenaEvent, teamStyle)			},
		"team style (old game type) for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"TeamType",			OFFSET(ArenaEvent, teamType)			},
		"team type for event (sg/single/team)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Weight",			OFFSET(ArenaEvent, weightClass)			},
		"level range allowed in this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Duration",			OFFSET(ArenaEvent, victoryType)			},
		"ending conditions/duration for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Custom",			OFFSET(ArenaEvent, custom)				},
		"whether this is a custom event or not"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Listed",			OFFSET(ArenaEvent, listed)				},
		"whether this is a private (or still being setup) event or a listed event"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Phase",			OFFSET(ArenaEvent, phase)				},
		"what phase the event is in"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Round",			OFFSET(ArenaEvent, round)				},
		"what round the event is in"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"StartTime",		OFFSET(ArenaEvent, start_time)			},
		"event start time"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundStart",		OFFSET(ArenaEvent, round_start)			},
		"start time of this round"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Scheduled",		OFFSET(ArenaEvent, scheduled)			},
		"whether this is an immediate or scheduled event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"ServerEvent",		OFFSET(ArenaEvent, serverEventHandle)	},
		"whether this is a server run or player run event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DemandPlay",		OFFSET(ArenaEvent, demandplay)			},
		"whether the event should start when there are enough people or wait until the scheduled time"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Sanctioned",		OFFSET(ArenaEvent, sanctioned)			},
		"whether this event counts for ratings or not"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"VerifySanctioned",	OFFSET(ArenaEvent, verify_sanctioned )	},
		"whether the event creator wants this event to be sanctioned"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"InviteOnly",		OFFSET(ArenaEvent, inviteonly)			},
		"players need an invitation to join this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NumSides",			OFFSET(ArenaEvent, numsides)			},
		"number of teams participating"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"MinPlayers",		OFFSET(ArenaEvent, minplayers)			},
		"minimum number of players needed to start event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"MaxPlayers",		OFFSET(ArenaEvent, maxplayers)			},
		"maximum number of players allowed in this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoSetBonus",		OFFSET(ArenaEvent, no_setbonus)		},
		"whether set bonuses are usable"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"NoEnd",			OFFSET(ArenaEvent, no_end)				},
		"whether endurance drain is turned off for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoPool",			OFFSET(ArenaEvent, no_pool)				},
		"whether pool powers are usable"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoStealth",		OFFSET(ArenaEvent, no_stealth)			},
		"whether stealth powers are usable"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoTravel",			OFFSET(ArenaEvent, no_travel)			},
		"whether travel powers are usable"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoObserve",		OFFSET(ArenaEvent, no_observe)			},
		"whether observers are allowed"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoChat",		OFFSET(ArenaEvent, no_chat)			},
		"whether chat is allowed"},


	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, description),	"Description",		OFFSET(ArenaEvent, description)			},
		"event description"},

	{{ PACKTYPE_CONREF,			CONTAINER_ENTS,							"CreatorId",		OFFSET(ArenaEvent, creatorid)			},
		"dbid of event creator"},

	{{ PACKTYPE_CONREF,			CONTAINER_SUPERGROUPS,							"CreatorSg",		OFFSET(ArenaEvent, creatorsg)			},
		"supergroup of event creator"},

	{{ PACKTYPE_CONREF,			CONTAINER_SUPERGROUPS,							"OtherSg",			OFFSET(ArenaEvent, othersg)				},
		"opponent supergroup"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, mapname),		"MapName",			OFFSET(ArenaEvent, mapname)				},
		"name of map to be used"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, creatorname),	"CreatorName",		OFFSET(ArenaEvent, creatorname)			},
		"name of creator (for kiosk display purposes"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, cancelreason),	"CancelReason",		OFFSET(ArenaEvent, cancelreason)		},
		"reason this event got canceled"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"CannotStart",		OFFSET(ArenaEvent, cannot_start)		},
		"flag that says this event has sanctioning problems"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, eventproblems[0]), "EventProblems0",	OFFSET(ArenaEvent, eventproblems[0]) },
		"reason #1 this event cannot be sanctioned"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, eventproblems[1]), "EventProblems1",	OFFSET(ArenaEvent, eventproblems[1]) },
		"reason #2 this event cannot be sanctioned"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, eventproblems[2]), "EventProblems2",	OFFSET(ArenaEvent, eventproblems[2]) },
		"reason #3 this event cannot be sanctioned"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, eventproblems[3]), "EventProblems3",	OFFSET(ArenaEvent, eventproblems[3]) },
		"reason #4 this event cannot be sanctioned"},

	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaEvent, eventproblems[4]), "EventProblems4",	OFFSET(ArenaEvent, eventproblems[4]) },
		"reason #5 this event cannot be sanctioned"},


	// assumes ARENA_MAX_ROUNDS == 11
	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType0",	OFFSET(ArenaEvent, roundtype[0])	},
		"whether round 0 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType1",	OFFSET(ArenaEvent, roundtype[1])	},
		"whether round 1 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType2",	OFFSET(ArenaEvent, roundtype[2])	},
		"whether round 2 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType3",	OFFSET(ArenaEvent, roundtype[3])	},
		"whether round 3 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType4",	OFFSET(ArenaEvent, roundtype[4])	},
		"whether round 4 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType5",	OFFSET(ArenaEvent, roundtype[5])	},
		"whether round 5 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType6",	OFFSET(ArenaEvent, roundtype[6])	},
		"whether round 6 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType7",	OFFSET(ArenaEvent, roundtype[7])	},
		"whether round 7 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType8",	OFFSET(ArenaEvent, roundtype[8])	},
		"whether round 8 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType9",	OFFSET(ArenaEvent, roundtype[9])	},
		"whether round 9 of this event is swiss draw or single elimination (swiss draw events only)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"RoundType10",	OFFSET(ArenaEvent, roundtype[10])	},
		"whether round 10 of this event is swiss draw or single elimination (swiss draw events only)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Entryfee",		OFFSET(ArenaEvent, entryfee)		},
		"entry fee for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"PetBattle",	OFFSET(ArenaEvent, use_gladiators)		},
		"whether this event is a Gladiator match or not"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"MapType",		OFFSET(ArenaEvent, mapid)			},
		"map for this event if set"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoTravelSuppression",	OFFSET(ArenaEvent, no_travel_suppression)	},
		"whether travel suppression is turned off for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoDiminishingReturns",	OFFSET(ArenaEvent, no_diminishing_returns)	},
		"whether diminishing returns on attributes is turned off for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"VictoryValue",	OFFSET(ArenaEvent, victoryValue)	},
		"the value used to determine whether victory has been achieved.  Tied tightly to duration/victoryType"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoHealDecay",	OFFSET(ArenaEvent, no_heal_decay)	},
		"whether diminishing returns on heals is turned off for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"NoInspirations",	OFFSET(ArenaEvent, inspiration_setting)	},
		"whether inspirations have been disabled for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"EnableTempPowers",	OFFSET(ArenaEvent, enable_temp_powers)	},
		"whether temp powers have been enabled for this event"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"TournamentType",	OFFSET(ArenaEvent, tournamentType)	},
		"the type of tournament that this event is"},


	{{ PACKTYPE_EARRAY, (int)ArenaParticipantCreate,				"Participants",	(int)participant_desc				},
		"subtable of the participants in this event"},

	{{ PACKTYPE_EARRAY, (int)ArenaSeatingCreate,					"Seating",		(int)seating_desc					},
		"subtable of seatings used"},


	{ 0 },
};

StructDesc arenaevent_desc[] =
{
	sizeof(ArenaEvent),
	{AT_NOT_ARRAY,{0}},
	arenaevent_line_desc,
	"TODO"
};


char *arenaEventTemplate()
{
	return dbContainerTemplate(arenaevent_desc);
}

char *arenaEventSchema()
{
	return dbContainerSchema(arenaevent_desc, "ArenaEvents");
}

char *packageArenaEvent(ArenaEvent* ae)
{
	char* result = dbContainerPackage(arenaevent_desc,ae);

	if (debugFilesEnabled())
	{
		FILE* f;
		f = fopen(fileDebugPath("arenaevent_package.txt"), "w");
		if (f) {
			char buf[30];
			sprintf(buf, "--Event %i ----\n", ae->eventid);
			fwrite(buf, 1, strlen(buf), f);
			fwrite(result, 1, strlen(result), f);
			fclose(f);
		}
	}

	return result;
}

void unpackArenaEvent(ArenaEvent* ae,char *mem,U32 id)
{
	ArenaEventDestroyContents(ae);
	memset(ae, 0, sizeof(*ae));
	ae->eventid = id;

	if (debugFilesEnabled())
	{
		FILE* f;
		f = fopen(fileDebugPath("arenaevent_unpack.txt"), "w");
		if (f) {
			char buf[30];
			sprintf(buf, "--Event %i ----\n", ae->eventid);
			fwrite(buf, 1, strlen(buf), f);
			fwrite(mem, 1, strlen(mem), f);
			fclose(f);
		}
	}

	dbContainerUnpack(arenaevent_desc,mem,ae);
	ArenaEventCountPlayers(ae);
	ae->detailed = 1;
}

// *********************************************************************************
//  Arena players
// *********************************************************************************

// Currently assumes 10 Event Types, 3 Team Types, 14 Durations, 11 Weight Classes, and 10 Rating Indexes.
LineDesc arenaplayer_line_desc[] =
{
	{{ PACKTYPE_INT,			SIZE_INT32,							"PrizeMoney",	OFFSET(ArenaPlayer, prizemoney)	},
		"prizemoney awarded to this player"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Connected",	OFFSET(ArenaPlayer, connected)	},
		"whether this player is connected right now or not"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LastConnected",OFFSET(ArenaPlayer, lastConnected)	},
		"last time this player was connected"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins0",		OFFSET(ArenaPlayer, winsByRatingIndex[0])	},
		"total number of wins in category 0 (SINGLE DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins1",		OFFSET(ArenaPlayer, winsByRatingIndex[1])	},
		"total number of wins in category 1 (TEAM DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins2",		OFFSET(ArenaPlayer, winsByRatingIndex[2])	},
		"total number of wins in category 2 (SUPERGROUP DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins3",		OFFSET(ArenaPlayer, winsByRatingIndex[3])	},
		"total number of wins in category 3 (SINGLE SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins4",		OFFSET(ArenaPlayer, winsByRatingIndex[4])	},
		"total number of wins in category 4 (SG SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins5",		OFFSET(ArenaPlayer, winsByRatingIndex[5])	},
		"total number of wins in category 5 (SINGLE FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins6",		OFFSET(ArenaPlayer, winsByRatingIndex[6])	},
		"total number of wins in category 6 (TEAM FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins7",		OFFSET(ArenaPlayer, winsByRatingIndex[7])	},
		"total number of wins in category 7 (STAR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws0",		OFFSET(ArenaPlayer, drawsByRatingIndex[0])	},
		"total number of draws in category 0 (SINGLE DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws1",		OFFSET(ArenaPlayer, drawsByRatingIndex[1])	},
		"total number of draws in category 1 (TEAM DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws2",		OFFSET(ArenaPlayer, drawsByRatingIndex[2])	},
		"total number of draws in category 2 (SUPERGROUP DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws3",		OFFSET(ArenaPlayer, drawsByRatingIndex[3])	},
		"total number of draws in category 3 (SINGLE SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws4",		OFFSET(ArenaPlayer, drawsByRatingIndex[4])	},
		"total number of draws in category 4 (SG SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws5",		OFFSET(ArenaPlayer, drawsByRatingIndex[5])	},
		"total number of draws in category 5 (SINGLE FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws6",		OFFSET(ArenaPlayer, drawsByRatingIndex[6])	},
		"total number of draws in category 6 (TEAM FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws7",		OFFSET(ArenaPlayer, drawsByRatingIndex[7])	},
		"total number of draws in category 7 (STAR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses0",		OFFSET(ArenaPlayer, lossesByRatingIndex[0])	},
		"total number of losses in category 0 (SINGLE DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses1",		OFFSET(ArenaPlayer, lossesByRatingIndex[1])	},
		"total number of losses in category 1 (TEAM DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses2",		OFFSET(ArenaPlayer, lossesByRatingIndex[2])	},
		"total number of losses in category 2 (SUPERGROUP DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses3",		OFFSET(ArenaPlayer, lossesByRatingIndex[3])	},
		"total number of losses in category 3 (SINGLE SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses4",		OFFSET(ArenaPlayer, lossesByRatingIndex[4])	},
		"total number of losses in category 4 (SG SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses5",		OFFSET(ArenaPlayer, lossesByRatingIndex[5])	},
		"total number of losses in category 5 (SINGLE FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses6",		OFFSET(ArenaPlayer, lossesByRatingIndex[6])	},
		"total number of losses in category 6 (TEAM FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses7",		OFFSET(ArenaPlayer, lossesByRatingIndex[7])	},
		"total number of losses in category 7 (STAR)"},


	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating0",		OFFSET(ArenaPlayer, ratingsByRatingIndex[0])	},
		"total rating for category 0 (SINGLE DUEL)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating1",		OFFSET(ArenaPlayer, ratingsByRatingIndex[1])	},
		"total rating for category 1 (TEAM DUEL)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating2",		OFFSET(ArenaPlayer, ratingsByRatingIndex[2])	},
		"total rating for category 2 (SUPERGROUP DUEL)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating3",		OFFSET(ArenaPlayer, ratingsByRatingIndex[3])	},
		"total rating for category 3 (SINGLE SWISS DRAW)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating4",		OFFSET(ArenaPlayer, ratingsByRatingIndex[4])	},
		"total rating for category 4 (SG SWISS DRAW)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating5",		OFFSET(ArenaPlayer, ratingsByRatingIndex[5])	},
		"total rating for category 5 (SINGLE FFA)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating6",		OFFSET(ArenaPlayer, ratingsByRatingIndex[6])	},
		"total rating for category 6 (TEAM FFA)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating7",		OFFSET(ArenaPlayer, ratingsByRatingIndex[7])	},
		"total rating for category 7 (STAR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional0",	OFFSET(ArenaPlayer, provisionalByRatingIndex[0])	},
		"whether this player's rating is regarded provisional in category 0 (SINGLE DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional1",	OFFSET(ArenaPlayer, provisionalByRatingIndex[1])	},
		"whether this player's rating is regarded provisional in category 1 (TEAM DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional2",	OFFSET(ArenaPlayer, provisionalByRatingIndex[2])	},
		"whether this player's rating is regarded provisional in category 2 (SUPERGROUP DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional3",	OFFSET(ArenaPlayer, provisionalByRatingIndex[3])	},
		"whether this player's rating is regarded provisional in category 3 (SINGLE SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional4",	OFFSET(ArenaPlayer, provisionalByRatingIndex[4])	},
		"whether this player's rating is regarded provisional in category 4 (SG SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional5",	OFFSET(ArenaPlayer, provisionalByRatingIndex[5])	},
		"whether this player's rating is regarded provisional in category 5 (SINGLE FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional6",	OFFSET(ArenaPlayer, provisionalByRatingIndex[6])	},
		"whether this player's rating is regarded provisional in category 6 (TEAM FFA)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional7",	OFFSET(ArenaPlayer, provisionalByRatingIndex[7])	},
		"whether this player's rating is regarded provisional in category 7 (STAR)"},


	{{ PACKTYPE_BIN_STR,		RAID_BITFIELD_SIZE*4,				"RaidParticipant", OFFSET(ArenaPlayer, raidparticipant) },
		"information for this character if he's a raid participant"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins8",		OFFSET(ArenaPlayer, winsByRatingIndex[8])	},
		"total number of wins in category 8 (SINGLE GLADIATOR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Wins9",		OFFSET(ArenaPlayer, winsByRatingIndex[9])	},
		"total number of wins in category 9 (TEAM GLADIATOR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws8",		OFFSET(ArenaPlayer, drawsByRatingIndex[8])	},
		"total number of draws in category 8 (SINGLE GLADIATOR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Draws9",		OFFSET(ArenaPlayer, drawsByRatingIndex[9])	},
		"total number of draws in category 9 (TEAM GLADIATOR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses8",		OFFSET(ArenaPlayer, lossesByRatingIndex[8])	},
		"total number of losses in category 8 (SINGLE GLADIATOR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Losses9",		OFFSET(ArenaPlayer, lossesByRatingIndex[9])	},
		"total number of losses in category 9 (TEAM GLADIATOR)"},


	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating8",		OFFSET(ArenaPlayer, ratingsByRatingIndex[8])	},
		"total rating for category 8 (SINGLE GLADIATOR)"},

	{{ PACKTYPE_FLOAT,			SIZE_FLOAT32,								"Rating9",		OFFSET(ArenaPlayer, ratingsByRatingIndex[9])	},
		"total rating for category 9 (TEAM GLADIATOR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional8",	OFFSET(ArenaPlayer, provisionalByRatingIndex[8])	},
		"whether this player's rating is regarded provisionalin category 8 (SINGLE GLADIATOR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"Provisional9",	OFFSET(ArenaPlayer, provisionalByRatingIndex[9])	},
		"whether this player's rating is regarded provisional in category 9 (TEAM GLADIATOR)"},


	{{ PACKTYPE_STR_UTF8,			SIZEOF2(ArenaPlayer, arenaReward),	"ArenaReward",	OFFSET(ArenaPlayer, arenaReward)	},
		"rewards awarded to this player but not yet claimed"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsTotal",	OFFSET(ArenaPlayer, wins)	},
		"total number of wins in all arena matches"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsTotal",	OFFSET(ArenaPlayer, draws)	},
		"total number of draws in all arena matches"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesTotal",	OFFSET(ArenaPlayer, losses)	},
		"total number of losses in all arena matches"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent0",	OFFSET(ArenaPlayer, winsByEventType[0])	},
		"total number of wins in event type 0 (DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent1",	OFFSET(ArenaPlayer, winsByEventType[1])	},
		"total number of wins in event type 1 (SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent2",	OFFSET(ArenaPlayer, winsByEventType[2])	},
		"total number of wins in event type 2 (FREE FOR ALL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent3",	OFFSET(ArenaPlayer, winsByEventType[3])	},
		"total number of wins in event type 3 (STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent4",	OFFSET(ArenaPlayer, winsByEventType[4])	},
		"total number of wins in event type 4 (VILLAIN STAR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent0",	OFFSET(ArenaPlayer, drawsByEventType[0])	},
		"total number of draws in event type 0 (DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent1",	OFFSET(ArenaPlayer, drawsByEventType[1])	},
		"total number of draws in event type 1 (SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent2",	OFFSET(ArenaPlayer, drawsByEventType[2])	},
		"total number of draws in event type 2 (FREE FOR ALL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent3",	OFFSET(ArenaPlayer, drawsByEventType[3])	},
		"total number of draws in event type 3 (STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent4",	OFFSET(ArenaPlayer, drawsByEventType[4])	},
		"total number of draws in event type 4 (VILLAIN STAR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent0",	OFFSET(ArenaPlayer, lossesByEventType[0])	},
		"total number of losses in event type 0 (DUEL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent1",	OFFSET(ArenaPlayer, lossesByEventType[1])	},
		"total number of losses in event type 1 (SWISS DRAW)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent2",	OFFSET(ArenaPlayer, lossesByEventType[2])	},
		"total number of losses in event type 2 (FREE FOR ALL)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent3",	OFFSET(ArenaPlayer, lossesByEventType[3])	},
		"total number of losses in event type 3 (STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent4",	OFFSET(ArenaPlayer, lossesByEventType[4])	},
		"total number of losses in event type 4 (VILLAIN STAR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByTeam0",	OFFSET(ArenaPlayer, winsByTeamType[0])	},
		"total number of wins in team type 0 (SINGLE)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByTeam1",	OFFSET(ArenaPlayer, winsByTeamType[1])	},
		"total number of wins in team type 1 (TEAM)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByTeam2",	OFFSET(ArenaPlayer, winsByTeamType[2])	},
		"total number of wins in team type 2 (SUPERGROUP)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByTeam0",	OFFSET(ArenaPlayer, drawsByTeamType[0])	},
		"total number of draws in team type 0 (SINGLE)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByTeam1",	OFFSET(ArenaPlayer, drawsByTeamType[1])	},
		"total number of draws in team type 1 (TEAM)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByTeam2",	OFFSET(ArenaPlayer, drawsByTeamType[2])	},
		"total number of draws in team type 2 (SUPERGROUP)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByTeam0",	OFFSET(ArenaPlayer, lossesByTeamType[0])	},
		"total number of losses in team type 0 (SINGLE)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByTeam1",	OFFSET(ArenaPlayer, lossesByTeamType[1])	},
		"total number of losses in team type 1 (TEAM)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByTeam2",	OFFSET(ArenaPlayer, lossesByTeamType[2])	},
		"total number of losses in team type 2 (SUPERGROUP)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByVictoryType0",	OFFSET(ArenaPlayer, winsByVictoryType[0])	},
		"total number of wins in victory type 0 (TIMED)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByVictoryType1",	OFFSET(ArenaPlayer, winsByVictoryType[1])	},
		"total number of wins in victory type 1 (TEAM STOCK)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByVictoryType2",	OFFSET(ArenaPlayer, winsByVictoryType[2])	},
		"total number of wins in victory type 2 (LAST MAN STANDING)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByVictoryType3",	OFFSET(ArenaPlayer, winsByVictoryType[3])	},
		"total number of wins in victory type 3 (INDIVIDUAL KILLS)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByVictoryType4",	OFFSET(ArenaPlayer, winsByVictoryType[4])	},
		"total number of wins in victory type 4 (INDIVIDUAL STOCK)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByVictoryType0",	OFFSET(ArenaPlayer, drawsByVictoryType[0])	},
		"total number of draws in victory type 0 (TIMED)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByVictoryType1",	OFFSET(ArenaPlayer, drawsByVictoryType[1])	},
		"total number of draws in victory type 1 (TEAM STOCK)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByVictoryType2",	OFFSET(ArenaPlayer, drawsByVictoryType[2])	},
		"total number of draws in victory type 2 (LAST MAN STANDING)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByVictoryType3",	OFFSET(ArenaPlayer, drawsByVictoryType[3])	},
		"total number of draws in victory type 3 (INDIVIDUAL KILLS)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByVictoryType4",	OFFSET(ArenaPlayer, drawsByVictoryType[4])	},
		"total number of draws in victory type 4 (INDIVIDUAL STOCK)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByVictoryType0",	OFFSET(ArenaPlayer, lossesByVictoryType[0])	},
		"total number of losses in victory type 0 (TIMED)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByVictoryType1",	OFFSET(ArenaPlayer, lossesByVictoryType[1])	},
		"total number of losses in victory type 1 (TEAM STOCK)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByVictoryType2",	OFFSET(ArenaPlayer, lossesByVictoryType[2])	},
		"total number of losses in victory type 2 (LAST MAN STANDING)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByVictoryType3",	OFFSET(ArenaPlayer, lossesByVictoryType[3])	},
		"total number of losses in victory type 3 (INDIVIDUAL KILLS)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByVictoryType4",	OFFSET(ArenaPlayer, lossesByVictoryType[4])	},
		"total number of losses in victory type 4 (INDIVIDUAL STOCK)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass0",	OFFSET(ArenaPlayer, winsByWeightClass[0])	},
		"total number of wins in weight class 0 (ANY)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass1",	OFFSET(ArenaPlayer, winsByWeightClass[1])	},
		"total number of wins in weight class 1 (STRAWWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass2",	OFFSET(ArenaPlayer, winsByWeightClass[2])	},
		"total number of wins in weight class 2 (FLYWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass3",	OFFSET(ArenaPlayer, winsByWeightClass[3])	},
		"total number of wins in weight class 3 (BANTAMWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass4",	OFFSET(ArenaPlayer, winsByWeightClass[4])	},
		"total number of wins in weight class 4 (FEATHERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass5",	OFFSET(ArenaPlayer, winsByWeightClass[5])	},
		"total number of wins in weight class 5 (LIGHTWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass6",	OFFSET(ArenaPlayer, winsByWeightClass[6])	},
		"total number of wins in weight class 6 (WELTERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass7",	OFFSET(ArenaPlayer, winsByWeightClass[7])	},
		"total number of wins in weight class 7 (MIDDLEWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass8",	OFFSET(ArenaPlayer, winsByWeightClass[8])	},
		"total number of wins in weight class 8 (CRUISERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass9",	OFFSET(ArenaPlayer, winsByWeightClass[9])	},
		"total number of wins in weight class 9 (HEAVYWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByWeightClass10",	OFFSET(ArenaPlayer, winsByWeightClass[10])	},
		"total number of wins in weight class 10 (SUPERHEAVYWEIGHT)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass0",	OFFSET(ArenaPlayer, drawsByWeightClass[0])	},
		"total number of draws in weight class 0 (ANY)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass1",	OFFSET(ArenaPlayer, drawsByWeightClass[1])	},
		"total number of draws in weight class 1 (STRAWWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass2",	OFFSET(ArenaPlayer, drawsByWeightClass[2])	},
		"total number of draws in weight class 2 (FLYWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass3",	OFFSET(ArenaPlayer, drawsByWeightClass[3])	},
		"total number of draws in weight class 3 (BANTAMWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass4",	OFFSET(ArenaPlayer, drawsByWeightClass[4])	},
		"total number of draws in weight class 4 (FEATHERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass5",	OFFSET(ArenaPlayer, drawsByWeightClass[5])	},
		"total number of draws in weight class 5 (LIGHTWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass6",	OFFSET(ArenaPlayer, drawsByWeightClass[6])	},
		"total number of draws in weight class 6 (WELTERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass7",	OFFSET(ArenaPlayer, drawsByWeightClass[7])	},
		"total number of draws in weight class 7 (MIDDLEWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass8",	OFFSET(ArenaPlayer, drawsByWeightClass[8])	},
		"total number of draws in weight class 8 (CRUISERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass9",	OFFSET(ArenaPlayer, drawsByWeightClass[9])	},
		"total number of draws in weight class 9 (HEAVYWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByWeightClass10",	OFFSET(ArenaPlayer, drawsByWeightClass[10])	},
		"total number of draws in weight class 10 (SUPERHEAVYWEIGHT)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass0",	OFFSET(ArenaPlayer, lossesByWeightClass[0])	},
		"total number of losses in weight class 0 (ANY)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass1",	OFFSET(ArenaPlayer, lossesByWeightClass[1])	},
		"total number of losses in weight class 1 (STRAWWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass2",	OFFSET(ArenaPlayer, lossesByWeightClass[2])	},
		"total number of losses in weight class 2 (FLYWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass3",	OFFSET(ArenaPlayer, lossesByWeightClass[3])	},
		"total number of losses in weight class 3 (BANTAMWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass4",	OFFSET(ArenaPlayer, lossesByWeightClass[4])	},
		"total number of losses in weight class 4 (FEATHERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass5",	OFFSET(ArenaPlayer, lossesByWeightClass[5])	},
		"total number of losses in weight class 5 (LIGHTWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass6",	OFFSET(ArenaPlayer, lossesByWeightClass[6])	},
		"total number of losses in weight class 6 (WELTERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass7",	OFFSET(ArenaPlayer, lossesByWeightClass[7])	},
		"total number of losses in weight class 7 (MIDDLEWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass8",	OFFSET(ArenaPlayer, lossesByWeightClass[8])	},
		"total number of losses in weight class 8 (CRUISERWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass9",	OFFSET(ArenaPlayer, lossesByWeightClass[9])	},
		"total number of losses in weight class 9 (HEAVYWEIGHT)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByWeightClass10",	OFFSET(ArenaPlayer, lossesByWeightClass[10])	},
		"total number of losses in weight class 10 (SUPERHEAVYWEIGHT)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent5",	OFFSET(ArenaPlayer, winsByEventType[5])	},
	"total number of wins in event type 5 (VERSUS STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent6",	OFFSET(ArenaPlayer, winsByEventType[6])	},
	"total number of wins in event type 6 (7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent7",	OFFSET(ArenaPlayer, winsByEventType[7])	},
	"total number of wins in event type 7 (VILLAIN 7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent8",	OFFSET(ArenaPlayer, winsByEventType[8])	},
	"total number of wins in event type 8 (VERSUS 7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"WinsByEvent9",	OFFSET(ArenaPlayer, winsByEventType[9])	},
	"total number of wins in event type 9 (GLADIATOR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent5",	OFFSET(ArenaPlayer, drawsByEventType[5])	},
	"total number of draws in event type 5 (VERSUS STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent6",	OFFSET(ArenaPlayer, drawsByEventType[6])	},
	"total number of draws in event type 6 (7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent7",	OFFSET(ArenaPlayer, drawsByEventType[7])	},
	"total number of draws in event type 7 (VILLAIN 7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent8",	OFFSET(ArenaPlayer, drawsByEventType[8])	},
	"total number of draws in event type 8 (VERSUS 7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"DrawsByEvent9",	OFFSET(ArenaPlayer, drawsByEventType[9])	},
	"total number of draws in event type 9 (GLADIATOR)"},


	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent5",	OFFSET(ArenaPlayer, lossesByEventType[5])	},
	"total number of losses in event type 5 (VERSUS STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent6",	OFFSET(ArenaPlayer, lossesByEventType[6])	},
	"total number of losses in event type 6 (7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent7",	OFFSET(ArenaPlayer, lossesByEventType[7])	},
	"total number of losses in event type 7 (VILLAIN 7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent8",	OFFSET(ArenaPlayer, lossesByEventType[8])	},
	"total number of losses in event type 8 (VERSUS 7STAR)"},

	{{ PACKTYPE_INT,			SIZE_INT32,							"LossesByEvent9",	OFFSET(ArenaPlayer, lossesByEventType[9])	},
	"total number of losses in event type 9 (GLADIATOR)"},


	{ 0 },
};

StructDesc arenaplayer_desc[] =
{
	sizeof(ArenaPlayer),
	{AT_NOT_ARRAY,{0}},
	arenaplayer_line_desc,
	"TODO"
};

char *arenaPlayerTemplate()
{
	return dbContainerTemplate(arenaplayer_desc);
}

char *arenaPlayerSchema()
{
	return dbContainerSchema(arenaplayer_desc, "ArenaPlayers");
}

char *packageArenaPlayer(ArenaPlayer* ap)
{
	char* result = dbContainerPackage(arenaplayer_desc,ap);

	if (debugFilesEnabled())
	{
		FILE* f;
		f = fopen(fileDebugPath("arenaplayer_package.txt"), "w");
		if (f) {
			char buf[30];
			sprintf(buf, "--Player %i ----\n", ap->dbid);
			fwrite(buf, 1, strlen(buf), f);
			fwrite(result, 1, strlen(result), f);
			fclose(f);
		}
	}

	return result;
}

void unpackArenaPlayer(ArenaPlayer* ap, char* mem, U32 id)
{
	memset(ap, 0, sizeof(*ap));
	ap->dbid = id;

	if (debugFilesEnabled())
	{
		FILE* f;
		f = fopen(fileDebugPath("arenaplayer_unpack.txt"), "w");
		if (f) {
			char buf[30];
			sprintf(buf, "--Player %i ----\n", ap->dbid);
			fwrite(buf, 1, strlen(buf), f);
			fwrite(mem, 1, strlen(mem), f);
			fclose(f);
		}
	}

	dbContainerUnpack(arenaplayer_desc,mem,ap);
}

