/*\
 *
 *	raidstruct.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Common structures for supergroup bases and base raids
 *
 */

#ifndef RAIDSTRUCT_H
#define RAIDSTRUCT_H

#include "stdtypes.h"
#include "container/container_store.h"

#define BASERAID_MAX_PARTICIPANTS	32

/////////////////////////////////// ScheduledBase Raid
// raidserver structure for talking about base raids
typedef struct ScheduledBaseRaid
{
	U32			id;
	U32			attackersg;
	U32			defendersg;
	U32			time;
	U32			length;				// length of raid in seconds
	U32			complete_time;		// if set, raid is complete
	U32			scheduled_time;		// when base raid was scheduled
	int			attackers_won;
	int			instant;
	int			forfeit_checked;
	int			max_participants;
	U32			attacker_participants[BASERAID_MAX_PARTICIPANTS];
	U32			defender_participants[BASERAID_MAX_PARTICIPANTS];
} ScheduledBaseRaid;

char* SGBaseConstructInfoString(int sgid);
int SGBaseDeconstructInfoString(char* mapinfo, int *sgid, int *userid);
int RaidDeconstructInfoString(char *mapinfo, int *sgid1, int *sgid2);

#define BASERAID_SUNDAY			(1 << 0)
#define BASERAID_MONDAY			(1 << 1)
#define BASERAID_TUESDAY		(1 << 2)
#define BASERAID_WEDNESDAY		(1 << 3)
#define BASERAID_THURSDAY		(1 << 4)
#define BASERAID_FRIDAY			(1 << 5)
#define BASERAID_SATURDAY		(1 << 6)

#define BASERAID_ENTRY_PHASE		900			// length of time that attackers don't have to maintain entry
#define MIN_BASERAID_LENGTH			600			// minimum length in seconds of a full base raid
#define MAX_BASERAID_LENGTH			3600		// maximum length in seconds of a full base raid
#define BASERAID_INSTANT_DELAY		300			// seconds before starting an instant raid
#define BASERAID_DEFAULT_LENGTH		1800
#define BASERAID_DEFAULT_SIZE		16
#define BASERAID_WINDOW_PER_ITEM	2			// hours of window per item
#define BASERAID_MAX_WINDOW			10			// can't have a raid window this long
#define BASERAID_MIN_RAID_SIZE		8
#define BASERAID_DEFAULT_WINDOW		20
#define BASERAID_DEFAULT_WINDOW_DAYS (BASERAID_SATURDAY | BASERAID_WEDNESDAY)

#define BASERAID_SCHEDULE_COST		100000
#define BASERAID_SCHEDULE_REFUND	25000

CONTAINERSTRUCT_INTERFACE(ScheduledBaseRaid);

int BaseRaidNumParticipants(ScheduledBaseRaid* raid, int attacker);
int BaseRaidIsParticipant(ScheduledBaseRaid* raid, U32 dbid, int attacker);
int BaseRaidAddParticipant(ScheduledBaseRaid* raid, U32 dbid, int attacker); // returns success
int BaseRaidRemoveParticipant(ScheduledBaseRaid* raid, U32 dbid, int attacker); // returns success
void BaseRaidForEachSorted(int (*func)(ScheduledBaseRaid*, U32)); // sorts into time order
int BaseRaidCountAttacking(int supergroup_id);
int BaseRaidCountDefending(int supergroup_id);
void BaseRaidClearAll(void);

/////////////////////////////////// ScheduledBase Raid

// bunch of stuff for sort-of optimized raid list
#define BASERAID_DAYS_BITS		7		// bits to represent days of raid window
#define BASERAID_WINDOW_BITS	5		// bits to represent length of raid window (in hours)
#define BASERAID_TEAMS_BITS		3		// bits to represent # of raid teams
#define BASERAID_PARTS_BITS		(BASERAID_TEAMS_BITS+3+1)		// bits to represent # of participants
#define BASERAID_TEAM_SIZE		8		// standard size of a raid team

// per-supergroup structure for raid info
typedef struct SupergroupRaidInfo
{
	// saved to database
	U32			id;				// supergroup id
	U32			playertype;
	U32			window_hour;	
	U32			window_days;	// bitfield for days of week
	U32			open_mount;
	U32			max_raid_size;

	// only sent to client, derived from items of power
	U32			window_length:BASERAID_WINDOW_BITS;		// hours of length
} SupergroupRaidInfo;

CONTAINERSTRUCT_INTERFACE(SupergroupRaidInfo);
void SupergroupRaidInfoSend(Packet* pak, SupergroupRaidInfo* info);
void SupergroupRaidInfoGet(Packet* pak, SupergroupRaidInfo* info);
void SupergroupRaidInfoGetWindowTimes(SupergroupRaidInfo* info, U32 fromtime, U32* first, U32* second);
U32 SupergroupRaidInfoWindowMask(U32 window_days, int maskfirst, int masksecond); // remove selected bits
U32 RaidWindowRotate(U32 window_days, int direction);

///////Item of Power Game //////////////////////////////////////////////////////
#define MAX_IOP_NAME_LEN 128

typedef struct ItemOfPower
{
	U32		id;			//This Particular Item of Power 
	char	name[MAX_IOP_NAME_LEN];	//Make this a Attr?
	U32		superGroupOwner;
	U32		creationTime;
} ItemOfPower;

CONTAINERSTRUCT_INTERFACE(ItemOfPower);

typedef enum eIoPGameState
{
	IOP_GAME_NEVER_STARTED		= 0, 
	IOP_GAME_CATHEDRAL_OPEN		= (1<<0),
	IOP_GAME_CATHEDRAL_CLOSED	= (1<<1),
	IOP_GAME_NOT_RUNNING		= (1<<2),

	IOP_GAME_DEBUG_ALLOW_RAIDS_AND_TRIALS = (1<<8), //This is the only one state allowed to be ored
} eIoPGameState;

typedef struct ItemOfPowerGame
{
	U32		id;			
	U32		startTime;
	U32		state;

	// stuff for raid game
	U32		raid_length;
	U32		raid_size;
} ItemOfPowerGame;

CONTAINERSTRUCT_INTERFACE(ItemOfPowerGame);

char * stringFromGameState( eIoPGameState state );

/////// Raid stats //////////////////////////////////////////////////////
extern char* g_raidstatnames[4];
#define RAID_STAT(isattacker, attackerswin) (g_raidstatnames[((attackerswin)?1:0) | ((isattacker)?2:0)])

#endif // RAIDSTRUCT_H