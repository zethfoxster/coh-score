/*\
 *
 *	taskforce.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	deals with taskforce-specific aspects of storyarc stuff
 *
 */
#include "log.h"
#include "logcomm.h"
#include "taskforce.h"
#include "storyarcprivate.h"
#include "team.h"
#include "entPlayer.h"
#include "containerbroadcast.h"
#include "dbcontainer.h"
#include "cmdserver.h"
#include "dbcomm.h"
#include "character_level.h"
#include "entity.h"
#include "dbnamecache.h"
#include "svr_base.h"
#include "svr_chat.h"
#include "character_pet.h"
#include "origins.h"
#include "powers.h"
#include "TeamReward.h"
#include "TaskforceParams.h"
#include "MessageStoreUtil.h"
#include "comm_game.h"
#include "entGameActions.h"
#include "playerCreatedStoryarcServer.h"
#include "playerCreatedStoryarc.h"
#include "MissionServer/missionserver_meta.h"
#include "TeamTask.h"
#include "league.h"


void teamMateEnterArchitectTaskforce(Entity *teammate, int taskforce_id, int owner_id )
{
	if (teammate->db_id != owner_id)
	{
		teamAddMember(teammate, CONTAINER_TASKFORCES, taskforce_id, owner_id);
		TaskForceModeChange(teammate, TASKFORCE_ARCHITECT); 
		TaskForceLogCharacter(teammate);
	}

	// Set any player state as a consequence of being on the taskforce
	// with the parameters (eg. turn off enhancements).
	TaskForceSetEffects(teammate, 0, 0, 0);
	TaskForceInitDebuffs(teammate);
	StoryArcSendFullRefresh(teammate);
}


int immune_to_invalidation = ARCHITECT_DEVCHOICE | ARCHITECT_FEATURED | ARCHITECT_HALLOFFAME;

int TaskForceCreate(Entity* player, const ContactDef* contactdef,
	TFParamDeaths maxdeaths, U32 timelimit, bool timecounter, bool debuff,
	TFParamPowers disablepowers, bool disableenhancements, bool buff,
	bool disableinspirations, bool isFlashback, 
	char * pchPlayerArc, int mission_id, int flags, int authorid )
{
	const char* name;
	int i;
	int tf_mode = TASKFORCE_DEFAULT;

	// create the taskforce
	if (!player->teamup || !player->teamup_id)
	{
		log_printf("storyerrs.log", "TaskForceCreate: I was asked to create a task force for %s, but he doesn't have a team",
			player->name);
		return 0;
	}
	if (!teamCreate(player, CONTAINER_TASKFORCES))
		return 0;
	
	// name it
	if (!teamLock(player, CONTAINER_TASKFORCES))
		return 0;

	if( pchPlayerArc )
	{
		PlayerCreatedStoryArc * pArc;
		int validate = !(flags&immune_to_invalidation);
		int hasErrors = 0;
		char *failErrors = estrTemp();
		char *playableErrors = estrTemp();
		
		pArc = playerCreatedStoryarc_Add(pchPlayerArc, player->taskforce_id, validate, &failErrors, &playableErrors, &hasErrors );

		if(!pArc)
		{
			if( !(flags&ARCHITECT_TEST) ) // they tried to play a published mission which is no longer valid
				missionserver_invalidateArc(mission_id, 1, failErrors);

			teamUpdateUnlock(player, CONTAINER_TASKFORCES);
			TaskForce_PlayerLeaving(player);
			return 0;
		}
		else if(hasErrors)
		{
			missionserver_invalidateArc(mission_id, 0, playableErrors);
		}
		estrDestroy(&failErrors);
		estrDestroy(&playableErrors);

		estrPrintCharString(&player->taskforce->playerCreatedArc, packAndEscape(playerCreatedStoryArc_ToString(pArc)));
		tf_mode = TASKFORCE_ARCHITECT;
		player->taskforce->architect_id = mission_id;
		player->taskforce->architect_flags = flags;
		player->taskforce->architect_authid = authorid;
		playerCreatedStoryArc_SetContact( pArc, player->taskforce );
	}

	if (contactdef)
	{
		ScriptVarsTable vars = {0};
		// ScriptVarsTable HACK: fire and forget seed
		saBuildScriptVarsTable(&vars, player, player->db_id, ContactGetHandleFromDefinition(contactdef), NULL, NULL, NULL, NULL, NULL, 0, (unsigned int)time(NULL), false);
		name = saUtilTableLocalize(contactdef->taskForceName, &vars);
		if (!name)
		{
			if(pchPlayerArc)
				name = "Architect";
			else
				name = "Ouroboros";
		}

		if( !pchPlayerArc ) // set the level to the leaders level, this will not change again
		{
			player->taskforce->level = CLAMP( character_CalcExperienceLevel(player->pchar)+1, contactdef->minPlayerLevel, contactdef->maxPlayerLevel );
			player->taskforce->exemplar_level = player->taskforce->level-1;
		}

		player->taskforce->levelAdjust = contactdef->taskForceLevelAdjust;

		if( isFlashback  || pchPlayerArc  )
			player->taskforce->exemplar_level = contactdef->maxPlayerLevel-1;
	}
	else
	{
		name = "Alpha"; // for debugging only
	}
	strncpyt(player->taskforce->name, name, TF_NAME_LEN);

	// set up all the parameters
	TaskForceInitParameters(player, maxdeaths, timelimit, timecounter,
		debuff, disablepowers, disableenhancements, buff, disableinspirations);

	teamUpdateUnlock(player, CONTAINER_TASKFORCES);

	// log
	LOG_ENT(player, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Taskforce:Create Taskforce ID: %d", player->taskforce_id);
	TaskForceLogCharacter(player);

	// get everyone on team into it & change modes
	for (i = 0; i < player->teamup->members.count; i++)
	{
		
		Entity* teammate = entFromDbId(player->teamup->members.ids[i]);

		if(!teammate)
		{
			if( tf_mode == TASKFORCE_ARCHITECT )
			{
				cmdOldServerParsef(0, "relay_architect_join %i %i %i", player->teamup->members.ids[i], player->taskforce_id, player->db_id  );
				continue;
			}
			else
			{
				log_printf("storyerrs.log", "TaskForceCreate: I was creating a task force for %s(%i), but his teammate %i isn't on the map, aborting",
					player->name, player->db_id, player->teamup->members.ids[i]);
				if (player->taskforce_id)
					teamDestroy(CONTAINER_TASKFORCES, player->taskforce_id, "(TaskForceDestroy)");
				return 0;
			}
		}

		if( character_CalcExperienceLevel(teammate->pchar) > player->taskforce->exemplar_level )
		{
			character_DestroyAllPets(teammate->pchar);
			if (isFlashback)
				chatSendToPlayer(teammate->db_id, localizedPrintf(teammate,"YouAreFBExemplared"), INFO_SVR_COM, 0 );
			else
				chatSendToPlayer(teammate->db_id, localizedPrintf(teammate,"YouAreTFExemplared"), INFO_SVR_COM, 0 );
		}

		if (teammate->db_id != player->db_id)
		{
	        teamAddMember(teammate, CONTAINER_TASKFORCES, player->taskforce_id, player->db_id);
			TaskForceModeChange(teammate, tf_mode); 
			TaskForceLogCharacter(teammate);
		}

		// Set any player state as a consequence of being on the taskforce
		// with the parameters (eg. turn off enhancements).
		TaskForceSetEffects(teammate, disableenhancements, disablepowers, disableinspirations);
		TaskForceInitDebuffs(teammate);
	}

	// make sure story info is cleared
	player->pl->taskforce_mode = tf_mode;
	StoryArcInfoResetUnlocked(player);
	player->pl->taskforce_mode = 0;

	// everything from now on needs to be in task force mode
	StoryArcUnlockInfo(player);
	TaskForceModeChange(player, tf_mode); 
	StoryArcLockInfo(player);
	return 1; 
}

void TaskForceCreateDebug(Entity* player, ClientLink* client)
{
	int i;

	if (!player->teamup_id)
	{
		conPrintf(client, localizedPrintf(player,"TeamForTF"));
		return;
	}
	for (i = 0; i < player->teamup->members.count; i++)
	{
		Entity* teammate = entFromDbId(player->teamup->members.ids[i]);
		if (!teammate)
		{
			conPrintf(client, localizedPrintf(player,"TeamOnMapTF"));
			return;
		}
	}

	StoryArcLockInfo(player);
	TaskForceCreate(player, NULL, TFDEATHS_INFINITE, 0, false, false, 0, false, false, false, false,0,0,0,0);
	StoryArcUnlockInfo(player);
}

// change the player to the selected mode
// NOTE: player may not be owned at this point
void TaskForceModeChange(Entity* e, int on)
{
	char buf[100];
	RewardToken *pToken = NULL;

	if ((on == TASKFORCE_ARCHITECT) && e && e->taskforce)
	{
		missionserver_map_startArc(e, e->taskforce->architect_id );
	}
	if (e && e->pl && on)
		e->pl->pendingArchitectTickets = 0;					//	reset their pending tickets to 0 since they joined a Architect TF

	if (e->pl->taskforce_mode == on || e->pl->taskforce_mode>0 && on>0)
		return; 
	if (on && !e->taskforce_id)	 
		return;

	if (!e->owned)
	{
		sprintf(buf, "tf_quit_relay %i", e->db_id);
		serverParseClient(buf, NULL);
		return;
	}

	if (!on && e->taskforce_id)
	{
		teamDelMember(e, CONTAINER_TASKFORCES, 0, 0);
	}
	e->pl->taskforce_mode = on;

	if (e->league)
	{
		if (on)
		{
			league_updateTeamLockStatus(e, e->teamup ? e->teamup->members.leader : e->db_id, on);
		}
		else
		{
			if (e->teamup)
			{
				league_updateTeamLockStatus(e, e->teamup->members.leader, e->teamup->teamSwapLock);
			}
			else
			{
				league_updateTeamLockStatus(e, e->db_id, e->teamup && e->teamup->teamSwapLock);
			}
		}
	}
	// make sure we quit team at same time - important if we are last member of TF
	if( !on && e->teamup_id && e->teamup && (e->teamup->members.count <= 1 ||  
	    (e->teamup->members.count <= MIN_TASKFORCE_SIZE && getRewardToken(e, "OnFlashback") == NULL && getRewardToken(e, "OnArchitect") == NULL)) )
	{
		teamDelMember(e, CONTAINER_TEAMUPS, 0, 0);
	}

	// clean up flashback token if they have one
	removeRewardToken(e, "OnFlashback");
	removeRewardToken(e, "OnArchitect");

	e->tf_params_update = 1;

	StoryArcSendFullRefresh(e);	// send him a full refresh regardless
}

// shut down the task force and pull everybody out of the mode
void TaskForceDestroy(Entity* player)
{
	// players will be taken out of task force mode on db update
	if (player->taskforce_id)
	{
		TaskForce_HandleExit(player);

		if (teamLock(player, CONTAINER_TASKFORCES))
		{
			player->taskforce->deleteMe = 1;
			teamUpdateUnlock(player, CONTAINER_TASKFORCES);
		}
	}
}

int TaskForce_PlayerLeaving(Entity* player)
{
	// 
	LOG_ENT(player, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Taskforce:Leave Taskforce ID: %d)", player->taskforce_id);
	if (player->taskforce_id && player->taskforce && player->pl->taskforce_mode == TASKFORCE_ARCHITECT)
	{
		LOG_ENT(player, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Taskforce:Architect Player left architect taskforce. Taskforce ID: %d Arc ID: %d", player->taskforce_id, player->taskforce->architect_id);
	}

	// should we destroy the entire task force?
	if (!player->taskforce) 
		return 1;

	TaskForce_HandleExit(player);

	if (player->taskforce->deleteMe)
		return 0; // the taskforce is already shutting down, _don't recurse_

	if (player->taskforce->members.count <= MIN_TASKFORCE_SIZE &&
		getRewardToken(player, "OnFlashback") == NULL && getRewardToken(player, "OnArchitect") == NULL)
	{
		// ask the entire taskforce to shut down
		TaskForceDestroy(player);
		return 1; // the only reason to stop now is that I probably deleted myself from the
				// taskforce when I set the deleteMe flag
	}

	return 0;
}

void TaskForceResumeTeam(Entity* player)
{
	char buf[1000];

	// our strategy is to try and broadcast to the container,
	// if no one responds, we don't have anyone to team with yet
	if (!player->taskforce_id) return;

	// 
	LOG_ENT(player, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Taskforce:Resume Taskforce ID: %d)", player->taskforce_id);
	if (player->taskforce_id && player->taskforce && player->taskforce->architect_id > 0)
	{
		LOG_ENT(player, LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "Taskforce:Architect Player resumed architect taskforce. Taskforce ID: %d Arc ID: %d", player->taskforce_id, player->taskforce->architect_id);
	}

	// we speculatively make ourselves the taskforce leader first.. if no one adds
	// us to their team this is correct, otherwise we fix it when we get added
	TaskForceMakeLeader(player, player->db_id);
	if (player->pl && player->taskforce && (player->pl->taskforce_mode == TASKFORCE_ARCHITECT) && (player->taskforce->members.count <= 1) && !player->teamup_id)
		teamCreate(player, CONTAINER_TEAMUPS);
	sprintf(buf, "%s%i", DBMSG_TASKFORCE_RESUMETEAM, player->db_id);
	dbBroadcastMsg(CONTAINER_TASKFORCES, &player->taskforce_id, INFO_SVR_COM, 0, 1, buf);
}

// force this player to be the 'leader' of the task force
// DO NOT call this function while inside a teamup lock!
void TaskForceMakeLeader(Entity* member, int new_db_id)
{
	if (!member->taskforce) 
		return;

	if (teamLock(member, CONTAINER_TASKFORCES))
	{
		TaskForceMakeLeaderUnlocked(member, new_db_id);
		teamUpdateUnlock(member, CONTAINER_TASKFORCES);
	}
}

void TaskForceMakeLeaderUnlocked(Entity* member, int new_db_id)
{
	StoryInfo* info;

	member->taskforce->members.leader = new_db_id;
	info = StoryInfoFromPlayer(member);
	TaskReassignAll(new_db_id, info);
	LOG_ENT(entFromDbId(new_db_id), LOG_ENTITY, LOG_LEVEL_IMPORTANT, 0, "TaskForce:Leader %s is new leader", 
		dbPlayerNameFromId(new_db_id));
}

void TaskForceSelectTask(Entity* leader)
{
	Teamup *team = leader->teamup;
	StoryTaskInfo* task;
	StoryInfo* info = StoryInfoFromPlayer(leader);
	int alliance = SA_ALLIANCE_BOTH;
	const char **teamRequires = NULL; 

	if (!leader->teamup || !leader->taskforce || !info->taskforce || !eaSize(&info->tasks)) 
		return;

	task = info->tasks[0];
	if (leader->teamup->activetask->assignedDbId == task->assignedDbId && isSameStoryTask( &leader->teamup->activetask->sahandle, &task->sahandle) )
		return;

	// ask everyone to see if they can access this task
	alliance = storyTaskInfoGetAlliance(task);
	teamRequires = storyTaskHandleGetTeamRequires(&task->sahandle, NULL);
	if (alliance != SA_ALLIANCE_BOTH || teamRequires != NULL)
	{
		// send relay command to validate mission to all members of the team
		Entity *pTeammate = NULL;
		int i;

		TeamTaskSelectClear(team);

		team->taskSelect.activeTask.context = task->sahandle.context;
		team->taskSelect.activeTask.subhandle = task->sahandle.subhandle;
		team->taskSelect.activeTask.compoundPos = task->sahandle.compoundPos;
		team->taskSelect.activeTask.bPlayerCreated = task->sahandle.bPlayerCreated;
		team->taskSelect.team_mentor_dbid = leader->db_id;

		for(i = 0; i < team->members.count; i++)
		{
			char cmdbuf[2000];
			sprintf(cmdbuf, "taskselectvalidate %i %i %i %i %i", team->members.ids[i], SAHANDLE_ARGSPOS(task->sahandle));
			serverParseClientEx(cmdbuf, NULL, ACCESS_INTERNAL);

		}
		return;
	}

	MissionAttachTeamup(leader, task, 1);
	TeamBroadcastTeamTaskSelected(leader->teamup_id);
}

void TaskForceSendSelect(Entity* player)
{
	if (!player->pl || !player->pl->taskforce_mode) 
		return;
	SendSelectTask(player, 0); 
}

// instead of doing a normal TaskForceSelectTask, we're just patching the
// assignedDbId if we've got a team task already - appropriate if team is
// already locked and we just did a TaskForceMakeLeader
// ARM: As far as I can tell, this code can be called before TaskForceMakeLeader
// this code should be called inside of a teamup lock,
// but TaskForceMakeLeader CANNOT be called inside of a teamup lock.
void TaskForcePatchTeamTask(Entity* member, int new_db_id)
{
	if (!member->taskforce) 
		return;
	if (!member->teamup->activetask->sahandle.context ) 
		return;
	if (!member->teamup->activetask->sahandle.subhandle) 
		return;
	member->teamup->activetask->assignedDbId = new_db_id;
}


/////////////////////////////////////////////////////////////////////////////////////
// Ask each member of the team if they would like to start the specified taskforce
//		Used for Flashback and Challenge missions
void TaskForcePollMembersForStart(Entity *leader)
{
	Teamup *pTeam = NULL;
	int i, count;

	if (!leader || !leader->teamup)
		return;

	pTeam = leader->teamup;
	count = pTeam->members.count;
	for (i = 0; i < count; i++)
	{
		Entity* teammate = entFromDbId(pTeam->members.ids[i]);

		
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
int TaskForceIsOnFlashback(Entity *player)
{
	if (!PlayerInTaskForceMode(player))
		return false;

	return (getRewardToken(player, "OnFlashback") != NULL);
}

/////////////////////////////////////////////////////////////////////////////////////
//
int TaskForceIsOnArchitect(Entity *player)
{
	if (!PlayerInTaskForceMode(player))
		return false;

	if (getRewardToken(player, "OnArchitect") != NULL)
		return 1;

	if(player->pl->taskforce_mode == TASKFORCE_ARCHITECT)
		return 1;

	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////
//
int TaskForceIsScalable(Entity *player)
{
	bool retval;

	retval = false;

	// 
	if (PlayerInTaskForceMode(player) &&
		player->taskforce && player->taskforce->storyInfo &&
		player->taskforce->storyInfo->storyArcs[0] &&
		StoryArcIsScalable(player->taskforce->storyInfo->storyArcs[0]))
	{
		retval = true;
	}

	return retval;
}


//------------------------------------------------------------------------------------------------------------

// Helper function that takes a number of seconds and turns it into a string
static char* TaskForceCalcTimeString(int seconds)
{
	static char timer[9];
	timer[0] = 0;
	// Put a limit on what we display xx:xx:xx
	seconds = MAX(seconds, 0);
	seconds = MIN(seconds, 359999);
	strcatf(timer, "%i:", seconds/3600);
	seconds %= 3600;
	strcatf(timer, "%d" , seconds/600);
	seconds %= 600;
	strcatf(timer, "%d:", seconds/60);
	seconds %= 60;
	strcatf(timer, "%d", seconds/10);
	seconds %= 10;
	strcatf(timer, "%d", seconds);
	return timer;
}

/////////////////////////////////////////////////////////////////////////////////////
//
void TaskForceCompleteStatus(Entity *player)
{
	char	*text = NULL;
	int		i;
	int		challenge = false;

	// don't do anything if the player isn't in a taskforce
	if (!PlayerInTaskForceMode(player))
		return;

	if( TaskForceIsOnArchitect(player) )
	{
		return;
	}

	// If it's a flashback, check to see if the player has the contact in their list. If so, update its contact points and relationship status.
	if(TaskForceIsOnFlashback(player))
	{
		ContactHandle handle = PlayerTaskForceContact(player);
		StoryInfo *info = player->storyInfo;
		if(handle && info)
		{
			for(i = eaSize(&info->contactInfos)-1; i >= 0; i--)
			{
				StoryContactInfo *contact = info->contactInfos[i];
				if(contact && contact->handle == handle)
				{
					contact->contactPoints += player->taskforce->storyInfo->contactInfos[0]->contactPoints;
					ContactDetermineRelationship(contact);
					ContactMarkDirty(player, info, contact);
					break;
				}
			}
		}
	}

	estrCreate(&text);
	estrConcatStaticCharArray(&text, "<font face=title color=Darkorange outline=2><span align=center><i>");

	if (TaskForceIsOnFlashback(player))
		estrConcatCharString(&text, textStd("OuroborosCompleteString"));
	else
	{
		if (ENT_IS_ON_BLUE_SIDE(player))
			estrConcatCharString(&text, textStd("TaskForceCompleteString"));
		else
			estrConcatCharString(&text, textStd("StrikeForceCompleteString"));
	}

	// check to see if ANY challenges are set
	for (i = TFPARAM_DEATHS; i < TFPARAM_NUM; i++)
	{
		if (i != TFPARAM_TIME_COUNTER && TaskForceIsParameterEnabled(player, i))
		{
			challenge = true;
			break;
		}
	}

	estrConcatStaticCharArray(&text, "</i></span></font><font outline=1><br>");

	// table header
	estrConcatStaticCharArray(&text, "<table><font outline=1><tr><td border=0></td><td align=right border=0>");
	if (challenge)
	{
		estrConcatf(&text, "<b>%s</b>", textStd("ChallengeString"));
	}
	estrConcatf(&text, "</td><td border=0 align=right><b>%s</b></td><td></td></tr>", textStd("ActualString"));

	// time elapsed
	estrConcatf(&text, "<tr border=0><td border=0>%s</td><td border=0></td><td align=right border=0>%s</td></tr>",
		textStd("TimeElapsedString"), TaskForceCalcTimeString(TaskForceGetParameter(player, TFPARAM_TIME_COUNTER)));

	// defeats
	estrConcatf(&text, "<tr border=0><td border=0>%s</td><td align=right border=0>", textStd("DefeatsString"));
	if (TaskForceIsParameterEnabled(player, TFPARAM_DEATHS))
	{
		estrConcatf(&text, "%d", TaskForceGetParameter(player, TFPARAM_DEATHS_MAX_VALUE));
	}
	estrConcatf(&text, "</td><td align=right border=0>");
	if (TaskForceIsParameterEnabled(player, TFPARAM_DEATHS))
	{
		if (TaskForceCheckFailureBit(player, TFPARAM_DEATHS))
		{
			estrConcatStaticCharArray(&text, "<font color=FireBrick>");
		} else {
			estrConcatStaticCharArray(&text, "<font color=LightGreen>");
		}
		estrConcatf(&text, "%d</font></td><td align=right border=0>", TaskForceGetParameter(player, TFPARAM_DEATHS));
		if (TaskForceCheckFailureBit(player, TFPARAM_DEATHS))
		{
			estrConcatf(&text, "<font color=FireBrick>%s", textStd("FailedString"));
		} else {
			estrConcatf(&text, "<font color=LightGreen>%s", textStd("SucceededString"));
		}
		estrConcatStaticCharArray(&text, "</font>");
	} else {
		estrConcatf(&text, "%d</td><td align=right border=0>", TaskForceGetParameter(player, TFPARAM_DEATHS));
	}
	estrConcatStaticCharArray(&text, "</td></tr>");

	// time limit
	if (TaskForceIsParameterEnabled(player, TFPARAM_TIME_LIMIT))
	{
		estrConcatf(&text, "<tr border=0><td border=0>%s</td><td align=right border=0>%s</td><td align=right border=0>",
			textStd("TimeLimitColonString"), TaskForceCalcTimeString(TaskForceGetParameter(player, TFPARAM_TIME_LIMIT_SETUP)));
		if (TaskForceCheckFailureBit(player, TFPARAM_TIME_LIMIT))
		{
			estrConcatStaticCharArray(&text, "<font color=FireBrick>");
		} else {
			estrConcatStaticCharArray(&text, "<font color=LightGreen>");
		}
		estrConcatf(&text, "%s</font></td><td align=right border=0>", TaskForceCalcTimeString(TaskForceGetParameter(player, TFPARAM_TIME_COUNTER)));
		if (TaskForceCheckFailureBit(player, TFPARAM_TIME_LIMIT))
		{
			estrConcatf(&text, "<font color=FireBrick>%s", textStd("FailedString"));
		} else {
			estrConcatf(&text, "<font color=LightGreen>%s", textStd("SucceededString"));
		}
		estrConcatStaticCharArray(&text, "</font>");
	}

	// end of table
	estrConcatStaticCharArray(&text, "</td></tr></table><br><br>");

	if (TaskForceIsParameterEnabled(player, TFPARAM_DISABLE_POWERS))
	{
		estrConcatStaticCharArray(&text, "<br>");
		switch(TaskForceGetParameter(player, TFPARAM_DISABLE_POWERS))
		{
			// No temporary/veteran powers.
			case TFPOWERS_NO_TEMP:
				estrConcatCharString(&text, textStd("NoTempPowersString"));
				break;

			// No travel powers, whether from pool powers, temporaries, etc.
			case TFPOWERS_NO_TRAVEL:
				estrConcatCharString(&text, textStd("NoTravelPowersString"));
				break;

			// Disables just pool powers. Epic, patron and temporary powers
			// are allowed.
			case TFPOWERS_NO_EPIC_POOL:
				estrConcatCharString(&text, textStd("NoEpicPoolPowersString"));
				break;

			// Disables pool, epic, patron and temporary powers. Accolades like the
			// Crey Pistol are unaffected, however.
			case TFPOWERS_ONLY_AT:
				estrConcatCharString(&text, textStd("OnlyATPowersString"));
				break;
		}
	} else {
		estrConcatCharString(&text, textStd("NoPowerLimitsString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_DEBUFF_PLAYERS))
	{
		estrConcatf(&text, "<br>%s", textStd("DebuffPlayersString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_BUFF_ENEMIES))
	{
		estrConcatf(&text, "<br>%s", textStd("BuffEnemiesString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_DISABLE_ENHANCEMENTS))
	{
		estrConcatf(&text, "<br>%s", textStd("NoEnhancementsEndString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_NO_INSPIRATIONS))
	{
		estrConcatf(&text, "<br>%s", textStd("NoInspirationsEndString"));
	}

	estrConcatStaticCharArray(&text, "</font>");


	START_PACKET( pak1, player, SERVER_RECEIVE_DIALOG_WIDTH );
	pktSendString( pak1, text);
	pktSendF32( pak1, 450.0f);
	END_PACKET

	estrDestroy(&text);
}


/////////////////////////////////////////////////////////////////////////////////////
//
void TaskForceStartStatus(Entity *player)
{
	char	*text = NULL;
	int		challenge = false;
	char	buf[100]; 

	// don't do anything if the player isn't in a taskforce
	if (!PlayerInTaskForceMode(player) || TaskForceIsOnArchitect(player) )
		return;

	estrCreate(&text);
	estrConcatStaticCharArray(&text, "<font face=title color=Darkorange outline=2><span align=center><i>");

	estrConcatCharString(&text, textStd("ChallengeSettingsString"));

	// check to see if ANY challenges are set
	if (TaskForceIsParameterEnabled(player, TFPARAM_DEATHS) || 
		TaskForceIsParameterEnabled(player, TFPARAM_TIME_LIMIT))
	{
		challenge = true;
	}

	estrConcatStaticCharArray(&text, "</i></span></font><font outline=1><br>");

	// table header
	if (challenge)
	{
		estrConcatStaticCharArray(&text, "<table><font outline=1><tr><td></td><td align=right border=0><b>");
		estrConcatCharString(&text, textStd("ChallengeString"));
		estrConcatStaticCharArray(&text, "</b></td></tr>");

		// defeats
		if (TaskForceIsParameterEnabled(player, TFPARAM_DEATHS))
		{
			estrConcatStaticCharArray(&text, "<tr border=0><td border=0>");
			estrConcatCharString(&text, textStd("DefeatsString"));
			estrConcatStaticCharArray(&text, "</td><td align=right border=0>");
			sprintf_s(buf, 100, "%d", TaskForceGetParameter(player, TFPARAM_DEATHS_MAX_VALUE));
			estrConcatCharString(&text, buf);
			estrConcatStaticCharArray(&text, "</td></tr>");
		}

		// time limit
		if (TaskForceIsParameterEnabled(player, TFPARAM_TIME_LIMIT))
		{
			estrConcatStaticCharArray(&text, "<tr border=2><td border=0>");
			estrConcatCharString(&text, textStd("TimeLimitColonString"));
			estrConcatStaticCharArray(&text, "</td><td align=right border=0>");
			estrConcatCharString(&text, TaskForceCalcTimeString(TaskForceGetParameter(player, TFPARAM_TIME_LIMIT_SETUP)));
			estrConcatStaticCharArray(&text, "</td></tr>");
		}

		// end of table
		estrConcatStaticCharArray(&text, "</table><br><br>");
	} else {
		estrConcatStaticCharArray(&text, "<br>");
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_DISABLE_POWERS))
	{
		estrConcatStaticCharArray(&text, "<br>");
		switch(TaskForceGetParameter(player, TFPARAM_DISABLE_POWERS))
		{
			// No temporary/veteran powers.
		case TFPOWERS_NO_TEMP:
			estrConcatCharString(&text, textStd("NoTempPowersString"));
			break;

			// No travel powers, whether from pool powers, temporaries, etc.
		case TFPOWERS_NO_TRAVEL:
			estrConcatCharString(&text, textStd("NoTravelPowersString"));
			break;

		case TFPOWERS_NO_EPIC_POOL:
			estrConcatCharString(&text, textStd("NoEpicPoolPowersString"));
			break;

			// Disables pool, epic, patron and temporary powers. Accolades like the
			// Crey Pistol are unaffected, however.
		case TFPOWERS_ONLY_AT:
			estrConcatCharString(&text, textStd("OnlyATPowersString"));
			break;
		}
	} else {
		estrConcatCharString(&text, textStd("NoPowerLimitsString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_DEBUFF_PLAYERS))
	{
		estrConcatStaticCharArray(&text, "<br>");
		estrConcatCharString(&text, textStd("DebuffPlayersString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_BUFF_ENEMIES))
	{
		estrConcatStaticCharArray(&text, "<br>");
		estrConcatCharString(&text, textStd("BuffEnemiesString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_DISABLE_ENHANCEMENTS))
	{
		estrConcatStaticCharArray(&text, "<br>");
		estrConcatCharString(&text, textStd("NoEnhancementsEndString"));
	}

	if (TaskForceIsParameterEnabled(player, TFPARAM_NO_INSPIRATIONS))
	{
		estrConcatStaticCharArray(&text, "<br>");
		estrConcatCharString(&text, textStd("NoInspirationsEndString"));
	}

	estrConcatStaticCharArray(&text, "</font><br><br>");

	estrConcatCharString(&text, textStd("ClickTheBigCrystalString"));


	START_PACKET( pak1, player, SERVER_RECEIVE_DIALOG_WIDTH );
	pktSendString( pak1, text);
	pktSendF32( pak1, 350.0f);
	END_PACKET

	estrDestroy(&text);
}

/////////////////////////////////////////////////////////////////////////////////////
//
void TaskForceResetDesaturate(Entity *player)
{
	serveDesaturateInformation(player, 0.0f, 0.0f, 0.0f, 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////
//
void TaskForceMissionEntryDesaturate(Entity *player)
{
	if (OnMissionMap() && TaskForceIsOnFlashback(player))
	{
		serveDesaturateInformation(player, 10.0f, 0.8f, 0.0f, 2.0f);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
void TaskForceMissionCompleteDesaturate(Entity *player)
{
	if (OnMissionMap() && TaskForceIsOnFlashback(player) && g_activemission &&
		g_activemission->def && !(g_activemission->def->flags & MISSION_NOTELEPORTONCOMPLETE))
	{

		serveDesaturateInformation(player, 0.0f, 0.0f, 0.8f, 2.0f);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
int TaskForceJoin(Entity* player, char* targetPlayerName)
{
	int targetID;
	Entity* target;

	if (!player)
		return 0;

	targetID = dbPlayerIdFromName(targetPlayerName);
	target = entFromDbId(targetID);
	
	if (!target || player==target)
		return 0;
	
	TaskForceSetEffects(player, 
		TaskForceGetParameter(target, TFPARAM_DISABLE_ENHANCEMENTS), 
		TaskForceGetParameter(target, TFPARAM_DISABLE_POWERS),
		TaskForceGetParameter(target, TFPARAM_NO_INSPIRATIONS));
	TaskForceInitDebuffs(player);

	if (!target->teamup)
		teamCreate(target, CONTAINER_TEAMUPS);

	if (player->teamup != target->teamup)
		teamAddMember(player, CONTAINER_TEAMUPS, target->teamup_id, targetID);

	teamAddMember(player, CONTAINER_TASKFORCES, target->taskforce_id, targetID);
	TaskForceModeChange(player, 1); 

	TaskForceLogCharacter(player);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
//
void TaskForceLogCharacter(Entity *player)
{

	int iset;
	char powers[10000];
	char line[10000];

	powers[0] = 0;

	if (player->pchar == NULL || player->pchar->porigin == NULL ||
		player->pchar->pclass == NULL)
		return;

	for(iset=eaSize(&player->pchar->ppPowerSets)-1; iset>=0; iset--)
	{
		int ipow;
		PowerSet *pset = player->pchar->ppPowerSets[iset];

		if (pset && pset->psetBase && pset->ppPowers) {
			for(ipow=eaSize(&pset->ppPowers)-1; ipow>=0; ipow--)
			{				
				if (pset->ppPowers[ipow])
				{
					sprintf(line, "PowerSet: %s PowerName: %s;", pset->psetBase->pchName, pset->ppPowers[ipow]->ppowBase->pchName);
					Strncatt(powers, line);
				}
			}
		}
	}

	LOG_ENT(player, LOG_ENTITY, LOG_LEVEL_VERBOSE, 0, "Taskforce:Character  %s %s, Level:%d XPLevel:%d XP:%d HP:%.2f End:%.2f Debt: %d Inf:%d AccessLvl: %d WisLvl: %d Wis: %d Powers (%s)",
		player->pchar->porigin->pchName,
		player->pchar->pclass->pchName,
		player->pchar->iLevel+1,
		character_CalcExperienceLevel(player->pchar)+1,
		player->pchar->iExperiencePoints,
		player->pchar->attrCur.fHitPoints,
		player->pchar->attrCur.fEndurance,
		player->pchar->iExperienceDebt,
		player->pchar->iInfluencePoints,
		player->access_level,
		player->pchar->iWisdomPoints,
		player->pchar->iWisdomLevel,
		powers);

}
