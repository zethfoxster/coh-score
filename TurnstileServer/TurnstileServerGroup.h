// TurnstileServerGroup.h
//

#pragma once

typedef struct NetLink NetLink;
typedef struct MissionInstance MissionInstance;

// Bit values placed in eventReadyFlags
#define		EVENT_READY_ACKED			(1 << 0)
#define		EVENT_READY_RESPONDED		(1 << 1)
#define		EVENT_READY_ACCEPTED		(1 << 2)

#define		ACK_WAIT_DELAY				5				// Seconds to wait for ack that client got the event ready message
#define		SERVER_RESPONSE_DELAY		(EVENT_READY_BASE_DELAY	+ 5)	// Seconds to wait for player to respond,  This is slightly longer than the 

// clientside value, so that a client timeout does the right thing

typedef struct QueuedPlayer
{
	int db_id;
	int auth_id;
	int teamNumber;
	int desiredTeamNumber;
	int isTeamLeader;
	int iLevel;
	U32 eventReadyTimer;								// timerSecondsSince2000() value used during event ready processing
	U8 eventReadyFlags;									// bitmask of flags used with the above timer
	int ignoredAuthId[MAX_IGNORES];
	int numIgnores;
} QueuedPlayer;


typedef struct QueueGroup
{
	struct QueueGroup *next;							// next and prev pointers, used for doubly linked list to get
	struct QueueGroup *prev;							// sorted access by join time
	int owner_dbid;										// dbid of the group leader - used as key for stashTable access
	int inFreeListQueue;
	U32 joinTime;										// secondsSince2000() we joined at
	int numEvent;										// Count of events.  Valid even for firstAvailable
	int eventList[MAX_EVENT];							// List of candidate events
	int numPlayer;										// Count of players in group
	QueuedPlayer playerList[MAX_LEAGUE_MEMBERS];		// List of players
	NetLink *link;										// Netlink this request came in on.
	float currentWait;									// Estimated wait time for this group.
	MissionInstance *parentInstance;
	int flags;
	int *ignoreAuths;
	// ...
} QueueGroup;

void GroupInit();
QueueGroup *GroupAlloc();
void GroupDestroy(QueueGroup *group);
void GroupAddToQueue(QueueGroup *group, int insertAtHead);
void GroupRemoveGroupFromQueue(int owner_dbid, NetLink *link, bool generateStatus);
void GroupRemovePlayerFromGroup(int owner_dbid, int player_dbid, NetLink *link);
void GroupChangeGroupOwner(int old_owner_dbid, int new_owner_dbid);
QueueGroup *GroupExtractGroupFromQueue(int owner_dbid);
QueueGroup *GroupFindGroupByOwner(int owner_dbid);
bool GroupIsPlayerInGroup(QueueGroup *group, int player_dbid);
void GroupEventReadyAck(U32 instanceID, int owner_dbid, int player_dbid);
void GroupEventResponse(U32 instanceID, int owner_dbid, int player_dbid, int accept);
void GroupMapID(U32 instanceID, int mapID, Vec3 pos);
void GroupTick();