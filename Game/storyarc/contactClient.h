#ifndef CONTACTCLIENT_H
#define CONTACTCLIENT_H

#include "gametypes.h"
#include "contactCommon.h"
#include "uiCompass.h"
#include "storyarcCommon.h"
#include "zowieCommon.h"

#ifndef TEST_CLIENT
#include "smf_main.h"
#else
typedef struct SMFBlock SMFBlock;
#endif

typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct Supergroup Supergroup;


//------------------------------------------------------------------------
// Contact Interaction
//------------------------------------------------------------------------
void ContactResponseOptionDestroy(ContactResponseOption* option);
ContactResponseOption* ContactResponseOptionCreate();
void ContactResponseReceive(ContactResponse* response, Packet* pak);
void ContactListDebugPrint(void);	// show all client contact info
void TaskListDebugPrint(void);


typedef struct
{
	ContactResponse response;
	ContactResponseOption** responseOptions;
	int waitingForServerReply;
} ContactDialogContext;

extern ContactDialogContext dialogContext;

#define CONTACT_LOCATIONS			-200	// base for destination id numbers
#define TASK_LOCATIONS				-400	// base for destination id numbers
#define TASK_SET_LOCATIONS			-100	// loc = TASK_LOCATIONS + TASK_SET_LOCATIONS * list_index
#define SG_LOCATION					 200	// supergroup destination id number
#define MISSION_WAYPOINT_LOCATION	 400	// offset for mission waypoint locations
#define MISSION_KEYDOOR_LOCATION	 600	// offset for mission keydoor locations

//------------------------------------------------------------------------
// Contact Status info
//------------------------------------------------------------------------
typedef struct ContactStatus
{
	char* name;						// Display name.  Already translated by server.
	char* locationDescription;		// Description of contact's location.  Already translated by server
	char* filename;					// Filename.  Only exists in development mode
	char* callOverride;				// text used instead of "investigate" in UI
	char* askAboutOverride;			// text used instead of "Ask about available missions"
	char* leaveOverride;			// text used instead of "leave" in UI
	char* imageOverride;			// description of image if other than contact.
	int npcNum;						// NPC costume number
	int handle;						// for contacting server

	int taskcontext;				// associated task in task array, 0 if none
	int tasksubhandle;

	U32 timeAutoRemokes;			//the time when the contact would auto-revoke if you stayed online indefinitely.

	int currentCP;					// current contact points
	int friendCP;					// contact points for friend
	int confidantCP;				// contact points for confidant
	int completeCP;					// contact points for complete
	int heistCP;					// contact points for heist offer

	unsigned int tipType;			// see TipType enum for further details

	unsigned int notifyPlayer : 1;	// Wants to talk to the player.
	unsigned int hasTask : 1;		// Has task for the player.
	unsigned int canUseCell : 1;	// player can call contact
	unsigned int hasLocation : 1;	// location field is set
	unsigned int isNewspaper : 1;	// this is the newspaper contact
	unsigned int onStoryArc : 1;	// this player currently has a story arc open from this contact
	unsigned int onMiniArc : 1;		// this player currently has a mini story arc open from this contact
	unsigned int wontInteract : 1;	// failed the interactionRequires statement
	unsigned int metaContact : 1;	// MetaContacts won't display a relationship bar and will always be active

	unsigned int currentOverheadIconState; // not persisted, client-driven

	Destination location;			// only has valid info if hasLocation set
	ContactAlliance alliance;
	ContactUniverse universe;
} ContactStatus;


// All contacts known to the player
extern ContactStatus** contacts;
extern ContactStatus** contactsAccessibleNow;
extern int contactsDirty;

extern U32 timeWhenCanEarnAlignmentPoint[ALIGNMENT_POINT_MAX_COUNT];
extern U32 earnedAlignments[ALIGNMENT_POINT_MAX_COUNT];

// Contacts window, Tips tab
extern U32 heroAlignmentPoints_self;
extern U32 vigilanteAlignmentPoints_self;
extern U32 villainAlignmentPoints_self;
extern U32 rogueAlignmentPoints_self;

// Player Info window, Alignment tab
extern U32 heroAlignmentPoints_other;
extern U32 vigilanteAlignmentPoints_other;
extern U32 villainAlignmentPoints_other;
extern U32 rogueAlignmentPoints_other;

extern U32 lastTimeWasSaved;
extern U32 timePlayedAsCurrentAlignmentAsOfTimeWasLastSaved;
extern U32 lastTimeAlignmentChanged;
extern U32 countAlignmentChanged;
extern U32 alignmentMissionsCompleted_Paragon;
extern U32 alignmentMissionsCompleted_Hero;
extern U32 alignmentMissionsCompleted_Vigilante;
extern U32 alignmentMissionsCompleted_Rogue;
extern U32 alignmentMissionsCompleted_Villain;
extern U32 alignmentMissionsCompleted_Tyrant;
extern U32 alignmentTipsDismissed;
extern U32 switchMoralityMissionsCompleted_Paragon;
extern U32 switchMoralityMissionsCompleted_Hero;
extern U32 switchMoralityMissionsCompleted_Vigilante;
extern U32 switchMoralityMissionsCompleted_Rogue;
extern U32 switchMoralityMissionsCompleted_Villain;
extern U32 switchMoralityMissionsCompleted_Tyrant;
extern U32 reinforceMoralityMissionsCompleted_Paragon;
extern U32 reinforceMoralityMissionsCompleted_Hero;
extern U32 reinforceMoralityMissionsCompleted_Vigilante;
extern U32 reinforceMoralityMissionsCompleted_Rogue;
extern U32 reinforceMoralityMissionsCompleted_Villain;
extern U32 reinforceMoralityMissionsCompleted_Tyrant;
extern U32 moralityTipsDismissed;

void assignContactIcon(ContactStatus *status);
void ContactStatusListReceive(Packet* pak);
void ContactStatusAccessibleListReceive(Packet *pak);
ContactStatus* ContactStatusGet(int handle);

//------------------------------------------------------------------------
// Task Status info
//------------------------------------------------------------------------

typedef struct TaskStatus
{
	int ownerDbid;					// Task UID
	int context;
	int subhandle;
	int level;						// these can support task conning, but we're not doing that yet
	int alliance;			
	StoryDifficulty	difficulty;

	char description[256];			// description of what the task is about
	char owner[MAX_NAME_LEN+1];		// optional - mission owner if not this player
	char state[256];				// optional - current progress of task
	char detail[LEN_CONTACT_DIALOG+1];	// detail page for task - client side cache
	char intro[LEN_MISSION_INTRO];	// Mission Intro - client side cache
	char filename[LEN_MISSION_INTRO];	// Task Filename, only used in debug

	unsigned int isComplete : 1;			// has task been completed yet?
	unsigned int isMission : 1;				// is this task a mission?
	unsigned int hasRunningMission : 1;		// Is a mission map running for the mission?
	unsigned int isSGMission : 1;			// Is this a supergroup mission?
	unsigned int hasLocation : 1;			// is the location field valid?
	unsigned int detailInvalid : 1;			// if set, the detail text is invalid and must be queried for again
	unsigned int zoneTransfer : 1;			// is this a zone transfer task?
	unsigned int teleportOnComplete : 1;	// are we allowed to teleport at end of task?
	unsigned int enforceTimeLimit : 1;		// Does the task still expire even after completion?
	unsigned int isAbandonable : 1;			// Can this task be abandoned?
	unsigned int isZowie : 1;				// Is this a zowie task?

	Destination location;			// only has valid info is hasLocation is set
	U32 timeout;					// when the task has to be completed by, 0 if no requirement
	U32 timezero;
	S32 timerType;
	U32 failOnTimeout;
	U32 iAskedWhen;					// Last time the task detail was updated

	U32	zinfo;						// option zowie info
	U32 zseed;						// seed used to select available zowies
	int zmap_id;
	char ztype[MAX_ZOWIE_NAME];		// optional acceptable zowie type(s)
	int zpoolSize[MAX_ZOWIE_TYPE];	// optional pool size counts for the types
	char zname[MAX_ZOWIE_NAME];		// optional zowie display name (for minimap)
	int	zIndices[MAX_GLOWING_ZOWIE_COUNT];  // indicies of the actually glowing zowies for this task

	SMFBlock *pSMFBlock;
} TaskStatus;

void TaskStatusListReceive(Packet* pak);
TaskStatus* TaskStatusFromContact(ContactStatus* contact); // get back active task if any from contact

TaskStatus** TaskStatusGetPlayerTasks();
void TaskStatusRemoveTeammates();

void TaskStatusListRemoveTeammates(Packet* pak);

typedef struct TaskStatusSet
{
	int dbid;
	StoryDifficulty difficulty;
	TaskStatus** list;
} TaskStatusSet;

extern TaskStatusSet** teamTaskSets;
TaskStatusSet* TaskStatusSetGet(int dbid);
int TaskStatusSetIndex(int dbid); // as above, but we just want a unique index returned
TaskStatusSet* TaskStatusSetGetPlayer();
TaskStatus* PlayerGetActiveTask();
TaskStatus* TaskStatusGet(int dbid, int context, int subhandle);
TaskStatus* TaskStatusCreate();
void TaskStatusDestroy(TaskStatus* task);
StoryDifficulty* TaskStatusGetNotoriety(int dbid);

typedef struct StoryDifficulty StoryDifficulty;
char * difficultyString( StoryDifficulty *pDiff );

//------------------------------------------------------------------------
// other stuff
//------------------------------------------------------------------------

// contact selection
void ContactSelect(int index, int userSelected);		// internal, select this contact
void ContactSelectReceive(Packet* pak);	// message from server for selection
void TaskSelectReceive(Packet* pak); // message from server for selection
void TaskDetailReceive(Packet* pak);
void TaskStatusReceive(TaskStatus* task, int index, int listid, Packet* pak);


// what locations the player can currently click on
extern char** visitLocations;

int TaskStatusIsTeamTask(TaskStatus* task);
int TaskStatusIsSame(TaskStatus* task, TaskStatus* task2);
int PlayerOnRealTeam();		// are we logically on a team? (team members >= 2)
int PlayerOnMyTeam(int dbid);
TaskStatus* GetUISelectedTeamTask();	// return status pointed at by selectedTeamTask
void TaskUpdateColor(Destination* dest);
int DestinationIsAContact(Destination *dest);
void TaskUpdateDestination(Destination* dest);

#endif
