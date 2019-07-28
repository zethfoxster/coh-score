/*\
 *
 *	storyarcinterface.h - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	This file contains all the external functions of the story arc system.
 *	This includes missions, tasks, contacts, and story arcs.  Include
 *	this file instead of including contact.h, etc.  This does not
 *	include functions for persistent npc's (pnpc.h).
 *
 */

#ifndef __STORYARCINTERFACE_H
#define __STORYARCINTERFACE_H

#include "stdtypes.h"
#include "entityRef.h"
//#include "scriptvars.h"
#include "storyarcCommon.h"
#include "power_system.h"   // for MAX_PLAYER_SECURITY_LEVEL

//////////////////////////////////////////////////////// storyarc handles
// Handles are used for communication with story arc system.  They are range-checked and generally
// 1-based indexes.  They are not valid between mapservers or sessions.  0 is 
// equivalent to NULL.
//
// StoryTask handles are either a valid ContactHandle or a StoryArcHandle.
// Use isStoryarcHandle or IS_CONTACT_HANDLE to determine which.
// The TaskSubHandle is required to fully identify a task.
typedef int ContactHandle;
typedef int TaskHandle;		// really either a ContactHandle or a StoryArcHandle
typedef int TaskSubHandle;
typedef int ClueSubHandle;
typedef int MissionHandle;

typedef struct StoryTaskHandle
{
	TaskHandle context; // storyarc is negative, positive is task pool
	TaskSubHandle subhandle;
	int compoundPos;
	bool bPlayerCreated; // mission architect storyarc
}StoryTaskHandle;

// forward declare a bunch of structs
typedef struct Entity Entity;
typedef struct Teamup Teamup;
typedef struct ClientLink ClientLink;
typedef struct Packet Packet;
typedef struct ScriptVarsTable ScriptVarsTable;
typedef struct PowerNameRef PowerNameRef;
typedef struct BasePower BasePower;
typedef struct RewardSet RewardSet;
typedef struct DefTracker DefTracker;

typedef struct EncounterGroup EncounterGroup;
typedef struct EncounterActiveInfo EncounterActiveInfo;
typedef struct Actor Actor;
typedef enum ActorState ActorState;
typedef struct SpawnDef SpawnDef;

typedef struct MissionDef MissionDef;
typedef struct StoryTask StoryTask;
typedef struct ContactDef ContactDef;
typedef struct Contact Contact;
typedef struct StoryArc StoryArc;
typedef struct StoryInfo StoryInfo;
typedef struct StoryClueInfo StoryClueInfo;
typedef struct StoryTaskInfo StoryTaskInfo;
typedef struct StoryContactInfo StoryContactInfo;
typedef struct StoryArcInfo StoryArcInfo;
typedef struct MissionInfo MissionInfo;
typedef struct MissionObjective MissionObjective;
typedef struct StoryObjectiveInfo StoryObjectiveInfo;
typedef struct StoryReward StoryReward;
typedef struct StoryClue StoryClue;
typedef struct StoryLocation StoryLocation;
typedef struct ContactResponse ContactResponse;
typedef struct ContactTaskPickInfo ContactTaskPickInfo;
typedef struct StoryTaskSet StoryTaskSet;
typedef struct ContactStoreItem ContactStoreItem;
typedef struct ContactStoreItems ContactStoreItems;
typedef struct MissionObjectiveInfo MissionObjectiveInfo;

typedef enum LocationTypeEnum LocationTypeEnum;
typedef int VillainGroupEnum;	// modified to match new version that is not a hardcoded enum - Vince
typedef enum TaskType TaskType;
typedef enum TaskState TaskState;
typedef enum ContactRelationship ContactRelationship;
typedef enum MapSetEnum MapSetEnum;
typedef enum VillainPacingEnum VillainPacingEnum;
typedef enum NewspaperType NewspaperType;
typedef enum DeliveryMethod DeliveryMethod;
typedef enum PvPType PvPType;

typedef enum EncounterAlliance EncounterAlliance;

typedef enum TFParamDeaths TFParamDeaths;
typedef enum TFParamPowers TFParamPowers;



#define MAX_VILLAIN_LEVEL			54

// misc
int PlayerInTaskForceMode(Entity* ent); // returns whether the player is in supergroup mode.  Returns 0 on failure
ContactHandle PlayerTaskForceContact(Entity* ent); // returns the contact for this task force
int* PlayerGetTasksMapIDs(Entity* player); // EArray of all map IDs for current tasks

// general
void StoryPreload();
void StoryLoad();
void StoryTick(Entity* player);  // per-player tick

// story arcs
void StoryArcCheckTimeouts(); // hook into global loop - check task timeouts
void StoryArcNewPlayer(Entity* ent); // set up initial story arc info
void StoryArcLeavingPraetoriaReset(Entity* ent); // wipe Praetorian contacts (preserve souvenirs but nothing else) and grant new Primal contacts
void StoryArcInfoReset(Entity* e); // resets current story arc info, dependent on supergroup mode
void StoryArcInfoResetUnlocked(Entity* e); // same as above without locking
StoryInfo* storyInfoCreate(); // create a story info structure
void storyInfoDestroy(StoryInfo* info); // destroy a story info structure
void StoryArcDebugPrint(ClientLink* client, Entity* player, int detailed); // print current state of story arc
void StoryArcDebugPrintHelper(char ** estr, ClientLink * client, Entity* player, int detailed);
int StoryArcGetEncounter(EncounterGroup* group, Entity* player); // attempt to replace an encounter with one from story arc
void StoryArcEncounterComplete(EncounterGroup* group, int success); // signal that encounter is done
int StoryArcEncounterReward(EncounterGroup* group, Entity* rescuer, StoryReward* reward, Entity* speaker); // signal that reward is being given from encounter
void StoryArcSendTaskRefresh(Entity* e); // refresh info for tasks this client can see
void StoryArcSendFullRefresh(Entity* e); // refresh all info for this client
void StoryArcPlayerEnteredMap(Entity* player); // tell the story system that a player logged in
void StoryArcFlashback(ClientLink* client, char* contactfile, char* storyarcfile);  // create the specified storyarc as a flashback
void StoryArcFlashbackList(Entity* e);  // sends the list of valid flashbacks down to the client
void StoryArcFlashbackDetails(Entity* e, char *fileID); // sends the detail information for the given storyarc to the client
void StoryArcFlashbackCheckEligibility(Entity *e, char *fileID);
void StoryArcFlashbackTeamRequiresValidate(Entity *e, StoryTaskHandle *sahandle);
void StoryArcFlashbackTeamRequiresValidateAck(Entity *e, StoryTaskHandle *sahandle, int result, int responder);
void StoryArcFlashbackAccept(Entity* e, char *fileID, int timeLimits, int limitedLives, int powerLimits, int debuff, int buff, int noEnhancements); // Starts the player on the specified flashback and teleports them to the selected contact for this storyarc
int StoryArcCheckLastEpisodeComplete(Entity* e, StoryInfo* info, StoryContactInfo* contactInfo, StoryTaskInfo* task);
char* StoryArcGetLeafNameByDef(const StoryArc* arc);
char* StoryArcGetLeafName(StoryInfo* info, StoryContactInfo* contactInfo); // gets the leafname of the storyarc filename
int StoryArcIsScalable(StoryArcInfo* arcinfo);	// checks if the 'scalable tf' flag is set

// contacts
void ContactOpen(Entity* player, const Vec3 pos, ContactHandle handle, Entity *contact);	// start the client interaction dialog
void ContactCellCall(Entity* player, ContactHandle handle);	// same as open, but called with cell phone
void ContactSupergroupFail(Entity* player, ContactHandle handle); // client clicked on a contact while in supergroup mode
void ContactStatusSendAll(Entity* e);	// refresh the client contact list
void ContactStatusSendChanged(Entity* e);	// update client contact list as needed
void ContactStatusAccessibleSendAll(Entity* e);	// refresh the client contact list
void TaskStatusSendAll(Entity* e);  // refresh the client task list
void TaskStatusSendUpdate(Entity* e);	// Send the entity's task list to all teammates.
void TaskSelectSomething(Entity* e); // select a task if possible (on connect to server)
void ContactVisitLocationSendChanged(Entity* e);	// update visit location list as needed
void ContactVisitLocationSendAll(Entity* e);	// refresh the visit location list
void ContactDialogResponsePkt(Packet* pakIn, Entity* player); // the client has responded to a dialog
void ContactProcess();	// hook into main loop - check and see if players have exiting dialog range

// tasks
void KillTaskUpdateCount(Entity* player, Entity* victim); // player killed the victim, update tasks as needed
void LocationTaskVisitReceive(Entity* player, Packet *pak); // player has reached a visit location
int DeliveryTaskTargetInteract(Entity* player, Entity* targetEntity, ContactHandle targetHandle); // player has reached an npc
void TaskUpdateLocations(Entity* player);	// update the positions for all tasks
void TaskSelectDefaultLocation(Entity* player);	// reset the compass destination
void ClueSendChanged(Entity* player);	// update client clue list
void ClueSendAll(Entity* player);	// refresh the client clue list
void KeyClueSendChanged(Entity* player);	// update client clue list
void KeyClueSendAll(Entity* player);	// refresh the client clue list
void TaskSetComplete(Entity* player, StoryTaskHandle *sahandle, int any_position, int success); // marks task as completed
void TaskMarkComplete(Entity* player, StoryTaskHandle *sahandle, int success);
void TaskAdvanceCompleted(Entity* player);
void TaskReceiveObjectiveComplete(Entity* player, StoryTaskHandle *sahandle, int objective, int success); // message from mission that objective was completed
void ClueAddRemoveInternal(Entity* player, StoryTaskHandle *sahandle, char* cluename, int add); // force given player to be given or lose a clue
int TaskGetEncounter(EncounterGroup* group, Entity* player);  // attempt to replace an encounter with one from a task
bool TaskSendDetail(Entity* player, int targetDbid, StoryTaskHandle *sahandle);

// missions
void MissionInitMap();			// called when the first player logs onto the map
void MissionSpecReload(void);	// called when geometry loaded
int OnMissionMap();				// returns 1 if server is a mission map
int OnInstancedMap();			// returns 1 if server is a mission map or arena map
int OnPlayerCreatedMissionMap(void);
int MissionAllowBaseHospital(void); // returns 1 if base hospital is allowed
int OnPraetorianTutorialMission(void);	//returns 1 if mission map is the praetorian tutorial
int MissionCompleted();			// returns 1 if players have completed this mission
int MissionSuccess();			// returns 1 if players have completed this mission successfully
int MissionPlayerAllowedTeleportExit(void);	// returns 1 to allow players to teleport out
void MissionEncounterVerify();	// have the mission verify all spawndefs
void MissionProcessEncounterGroup(EncounterGroup* group);	// called on encounter group load
int MissionSetupEncounter(EncounterGroup* group, Entity* player);	// called on encounter setup, returns whether encounter was replaced
void MissionSetEncounterLevel(EncounterGroup* group, Entity* player);	// called on encounter setup
void MissionEncounterComplete(EncounterGroup* group, int success);	// called when encounter is completed
void MissionItemInteract(Entity* objectiveEntity, Entity* interactPlayer);
void MissionItemInterruptInteraction(Entity* playerEntity);
void MissionHostageRename(EncounterGroup* group, char** displayname, const char** model);  // hook for when the encounter system creates an NPC on a mission map
const char* MissionAdjustDialog(const char* dialog, EncounterGroup* group, Entity* ent, const Actor* a, ActorState tostate, ScriptVarsTable* vars); // hook for when encounter system says some text
void MissionSetComplete(int success);		// sets the current mission as complete (only valid on mission map server)
void MissionShutdown(const char *msg);		// shut down this mapserver
void MissionPlayerEnteredMap(Entity* ent); // tell the mission system that a player logged in
void MissionCountAllPlayers(int ticks);	// give everyone on map a participation tick
int MissionCheckTimeout();		// called from main loop.  returns 1 if mission has timed out
void MissionProcess();			// hook into gloop
void MissionPlayerLeavingTeam(Entity* e); // assumes team is locked
void MissionRequestShutdown();	// map is being requested to shut down by dbserver (no teams left)
char* MissionConstructInfoString(int owner, int sgowner, int taskforce, ContactHandle contact, StoryTaskHandle *sahandle, 
								 U32 seed, int level, StoryDifficulty *pDifficulty, U32 completeSideObjectives, EncounterAlliance encAlly, VillainGroupEnum villainGroup); // construct an info string
int MissionDeconstructInfoString(char* str, int* owner, int* sgowner, int* taskforce, ContactHandle* contact, StoryTaskHandle *sahandle, 
								 U32* seed, int* level, StoryDifficulty *pDifficulty, U32* completeSideObjectives, EncounterAlliance* encAlly, VillainGroupEnum * villainGroup);
void MissionSetInfoString(char* info, StoryArc *pArc); // set the startup info string for the mission
int MissionGetLevelAdjust(void);
void ArenaSetInfoString(char* info); // in this header for convenience
char* MissionConstructIdString(int owner, StoryTaskHandle *sahandle); // construct a small string with id numbers for a mission
int MissionDeconstructIdString(const char* str, int* owner, StoryTaskHandle *sahandle); 
int MissionDeconstructObjectiveString(char* str, int* owner, StoryTaskHandle *sahandle, int* objnum, int* success);
int KeyDoorLocked(char* doorname);
void KeyDoorUpdate(MissionObjectiveInfo* objective);
void KeyDoorUnlockUpdate(Entity* player, char* doorname);
void MissionReseed(void);

// mission-teamup stuff
int MissionAttachTeamup(Entity* owner, StoryTaskInfo* mission, int checkmap); // attach the teamup to the mission
void MissionAttachTeamupUnlocked(Entity* teamholder, Entity* owner, StoryTaskInfo* mission, int checkmap);
void MissionMapDetaching(Entity* e); // team is detaching from a mission map
int CheckMissionMap(int map_id, int owner, int sgowner, int taskforce, StoryTaskHandle *sahandle, int* ready); // returns whether this mission is running
void MissionSendRefresh(Entity* e); // update client on current state of mission
void MissionTeamupUpdated(Entity* e); // called whenever teamup gets updated
void MissionTeamupAdvanceTask(Entity* e);	// called when a player logs into the map
void MissionSetTeamupComplete(Entity* e, int success); // set the complete flag on the teamup
void MissionShowState(ClientLink* client); // print team mission status
void MissionVerifyInfo(Entity* e);	// verify teamup attachment
void TaskSetMissionMapId(Entity* owner, StoryTaskHandle *sahandle, int map_id); // set the map id on this player's mission
int TaskMissionRequestShutdown(Entity* owner, StoryTaskHandle *sahandle, int map_id); // mission is asking owner if it can shut down
void TaskMissionForceShutdown(Entity* owner, StoryTaskHandle *sahandle);
void MissionPreparePlayerLeftTeam(Entity* entOnTeam); // must be called before MissionPlayerLeftTeam (non-reentrant pair) - just need a valid entity for somebody on the team
void MissionPlayerLeftTeam(Entity* player, int wasKicked);
void MissionPlayerLeftTaskForce(Entity* player);
void MissionReattachOwner(Entity* player, int map_id); 
void MissionKickPlayer(Entity* player); // if player is on a mission, kick them off map
void MissionUnkickPlayer(Entity* player); // stop trying to kick player
void MissionHandleObjectiveInfo(Entity* player, int cpos, int objnum, int success); // receive info on completed objective
void MissionChangeOwner(int new_owner, StoryTaskHandle *sahandle);

// task forces
int TaskForceCreate(Entity* player, const ContactDef* contactdef, TFParamDeaths maxdeaths,
	U32 timelimit, bool timecounter, bool debuff, TFParamPowers disablepowers,
	bool disableenhancements, bool buff, bool disableinspirations, bool isFlashback, 
	char *pchPlayerArc, int mission_id, int flags, int authorid);
void TaskForceCreateDebug(Entity* player, ClientLink* client); // sets up a task force with no tasks
void TaskForceModeChange(Entity* e, int on);
void TaskForceDestroy(Entity* player);
int TaskForce_PlayerLeaving(Entity* player);		// NOTE: if 1 is returned, task force is being destroyed
void TaskForceResumeTeam(Entity* player);		// puts the player back on his team
void TaskForceMakeLeader(Entity* member, int new_db_id);
void TaskForceMakeLeaderUnlocked(Entity* member, int new_db_id);
void TaskForceSelectTask(Entity* leader);
void TaskForceSendSelect(Entity* player);
void TaskForcePatchTeamTask(Entity* member, int new_db_id);

// unique tasks
int UniqueTaskGetCount();
char* UniqueTaskByIndex(int i);
char* UniqueTaskGetAttribute(const char* logicalname);

// clues
void scRequestDetails(Entity* player, int requestedIndex);	// player has requested details on a clue

// fame strings
void RandomFameAdd(Entity* player, const char* famestring);
void RandomFameTick(Entity* player);
void RandomFameMoved(Entity* player);

// debug commands
void TaskForceGet(ClientLink* client, const char* filename, const char* logicalname);	// gives the player a short task from contact
bool TaskForceGetUnknownContact(ClientLink * client, const char * logicalname);  // gives player task (finds a suitable contact)
void StoryArcForceGet(ClientLink* client, const char* contactfile, const char* storyarcfile); // gives the player the selected story arc
void StoryArcForceGetUnknownContact(ClientLink* client, char* storyarcfile); // gives player selected story arc from an arbitrary contact
void ContactDebugAcceptTask(ClientLink* client, char* contact, int longtask); // get the provided short or long task from the contact
void TaskDebugComplete(Entity* player, int index, int succeeded);	// marks specified task as complete
void TaskDebugCompleteByTask(Entity* player, StoryTaskInfo* taskinfo, int succeeded); // marks specified task as complete
void TaskDebugDetailPage(ClientLink* client, Entity* player, int index); // show detail page for specified task
void ClueDebugAddRemove(Entity* player, int index, char* cluename, int add); // add/remove a clue name to specified task
void UniqueTaskDebugPage(ClientLink* client, Entity* player);
void ContactDebugShowAll(ClientLink* client);	// print all contacts with id's
void ContactDebugShowMine(ClientLink* client, Entity* e); // print contacts belonging to player
void ContactDebugShowMineHelper(char ** estr, ClientLink * client, Entity * e); // dual use -- can be used for /bug and ContactDebugShowMine()
void ContactDebugAdd(ClientLink* client, Entity* e, const char* filename); // add a contact to a player
void ContactDebugShowIntroductions(ClientLink* client, char* filename);
void ContactDebugGetIntroduction(ClientLink* client, char* filename, int nextlevel);
void ContactDebugShowDetail(ClientLink* client, Entity* e, char* filename);
void ContactGoto(ClientLink* client, Entity* e, const char* filename, int select);
void ContactDebugInteract(ClientLink* client, char* contactfile, int cellcall); // start the client interaction dialog from a filename
void ContactFlashbackInteract(ClientLink *client, char *contactfile, char *storyarc);
void ContactBaseInteract( Entity * e, char* contactfile);
//void ContactNewspaperInteract(ClientLink* client); // newspaper interaction screen
void ContactDebugSetCxp(ClientLink* client, Entity* player, char* contactfile, int cxp);
void MissionPrintObjectives(ClientLink* client);
int MissionGotoObjective(ClientLink* client, int i);
int MissionNumObjectives(void);
void MissionForceLoad(int set); // forces the mapserver to start up as if it were loading a mission
void StoryArcReloadAll(Entity* player); // forces a reload of all story arc information
void MissionForceUninit();				// used with Reinit - reinitializes a running mission server
void MissionForceReinit(Entity* player); // used with Uninit
void MissionDebugPrintMapStats(ClientLink* client, Entity* player, char * mapname); // does a reinit as well
void MissionDebugSaveMapStatsToFile(Entity *player, char * mapname); // does a reinit as well
void MissionDebugCheckMapStats(ClientLink* client, Entity* player, int sendpopup, char * mapname); // does a reinit as well
void SpawnObjectiveItem(ClientLink* client, char* model); // spawn an objective item (for debugging)
void MissionSetShowFakeContact(int set); // show the fake contact for all players
bool MissionGotoMissionDoor(ClientLink * client);
void MissionAssignRandom(ClientLink * client);
void MissionAssignRandomWithInstance(ClientLink * client);

StoryTaskHandle * tempStoryTaskHandle( int context, int sub, int cpos, int player_created);
#endif // __STORYARCINTERFACE_H
