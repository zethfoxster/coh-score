/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <assert.h>

#include "entity.h"
#include "character_base.h"
#include "character_level.h"
#include "character_combat.h"
#include "character_pet.h"

#include "sendToClient.h"   // inexplicably for sendInfoBox
#include "dbcomm.h"
#include "svr_base.h"
#include "svr_chat.h"
#include "svr_player.h"
#include "comm_game.h"
#include "langServerUtil.h"

#include "team.h"
#include "entPlayer.h"
#include "taskforce.h"
#include "storyarcprivate.h"
#include "cmdserver.h"
#include "entity.h"
#include "badges_server.h"

#include "buddy.h"
#include "buddy_server.h"

#include "arenamap.h"
#include "dbnamecache.h"
#include "scriptengine.h"
#include "sgraid.h"
#include "power_system.h"


/**********************************************************************func*
 * character_IsTFExemplar
 *
 */
bool character_IsTFExemplar(Character *p)
{
	if( p->entParent && p->entParent->taskforce )
	{
		if( character_CalcExperienceLevel(p) > p->entParent->taskforce->exemplar_level )
			return true;
		if( g_ArchitectTaskForce && character_CalcExperienceLevel(p) > gArchitectMapMaxLevel )
			return true;
	}

	return false;
}

/**********************************************************************func*
 * character_CalcCombatLevel
 *
 */
int character_CalcCombatLevel(Character *p)
{
	Entity *ent = p->entParent;
	p->iCombatLevel = character_CalcExperienceLevel(p);

	if( ArenaMapLevelOverride(p) || ScriptLevelOverride(p) || RaidMapLevelOverride(p))
	{
		return p->iCombatLevel;
	}

	if( character_IsSidekick(p,0) )
	{
		p->iCombatLevel = character_TeamLevel(p) - 1;
	}
	else if(character_IsExemplar(p,0))
	{
		p->iCombatLevel = character_TeamLevel(p);
	}

	// auto-examplar tasforce clamp
	if(ent && ent->taskforce && !TaskForceIsOnArchitect(ent))
	{
		if( !TaskForceIsOnFlashback(ent) ) // Standard taskforce cannot change level anymore
		{
			p->iCombatLevel = CLAMP( p->iCombatLevel, ent->taskforce->level-2, ent->taskforce->level-1 );
		}
		else
		{
			// clamp player to taskforce level
			if( p->iCombatLevel >= ent->taskforce->exemplar_level )
			{
				// if they are sidekicked, they have to be one lower than mentor, so 
				// cap them to exemplar level -1
				if( character_CalcExperienceLevel(p) < ent->taskforce->exemplar_level )
					p->iCombatLevel = ent->taskforce->exemplar_level - 1;
				else
					p->iCombatLevel = ent->taskforce->exemplar_level;
			}
		}
	}

	// Clamp for Architect Mission Map Limit g_ArchitectTaskForce
	if( g_ArchitectTaskForce )
	{
		int adjust = 0;
		if( g_activemission )
			adjust = g_activemission->difficulty.levelAdjust;

		if( gArchitectMapMinLevel > gArchitectMapMaxLevel ) // failsafe
			gArchitectMapMinLevel = gArchitectMapMaxLevel;

 		if( ENTTYPE(ent) == ENTTYPE_PLAYER )
 		{
			p->iCombatLevel = CLAMP(p->iCombatLevel, CLAMP(gArchitectMapMinLevel-adjust, 0, MAX_PLAYER_LEVEL-1), 
												     CLAMP(gArchitectMapMaxLevel-adjust, 0, MAX_PLAYER_LEVEL-1) );
		}
		else if( ENTTYPE(ent) == ENTTYPE_CRITTER )
		{
			Entity *creator = 0;
			if( ent->erCreator )
				creator = erGetEnt(ent->erCreator);

			if( creator && ENTTYPE(creator) == ENTTYPE_PLAYER )
			{
				p->iCombatLevel = CLAMP(p->iCombatLevel, CLAMP(gArchitectMapMinLevel-adjust, 0, MAX_PLAYER_LEVEL-1), 
														 CLAMP(gArchitectMapMaxLevel-adjust, 0, MAX_PLAYER_LEVEL-1) );
			}
			else
			{
				p->iCombatLevel = CLAMP(p->iCombatLevel, 0, CLAMP(gArchitectMapMaxLevel, 0, 53) );
			}
		}
	}

	p->iCombatLevel = CLAMP(p->iCombatLevel, 0, 53);

	return p->iCombatLevel;
}

/**********************************************************************func*
 * character_HandleCombatLevelChange
 *
 */
void character_HandleCombatLevelChange(Character *p)
{
	if(p->buddy.iLastCombatLevel<0)
	{
		if (g_ArchitectTaskForce)
		{
			//	When the player enters the map, figure out what was their level before they entered the map
			//	To do this, ignore their AE task force (since this can adjust the combat level)
			//	and ignore the pending status (since this can affect correctly identifying sidekicking)
			TaskForce *tmp = g_ArchitectTaskForce;
			int type = p->buddy.eType;
			g_ArchitectTaskForce = NULL;
			p->buddy.eType = kBuddyType_None;
			p->buddy.iLastCombatLevel = character_CalcCombatLevel(p);
			g_ArchitectTaskForce = tmp;
			p->buddy.eType = type;
			p->iCombatLevel = character_CalcCombatLevel(p);
		}
		else
		{
			p->buddy.iLastCombatLevel = p->iCombatLevel;
		}
	}
	else if (p->iCombatLevel != p->buddy.iLastCombatLevel)
	{
		// Any change in Max HP is now handled by AccrueMods
		sendInfoBox(p->entParent, INFO_COMBAT_SPAM, "CombatLevelChanged", p->iCombatLevel+1);
		//sendChatMsg(p->entParent, ach, INFO_EMOTE, p->entParent->db_id);
		p->bNoModsLastTick = false;
		p->bRecalcStrengths = true;
		if (p->iCombatLevel < p->buddy.iLastCombatLevel)
		{
			character_DestroyAllPetsEvenWithoutAttribMod(p);
		}
		p->buddy.iLastCombatLevel = p->iCombatLevel;
		character_ActivateAllAutoPowers(p);
	}
}

void character_UpdateTeamMentor(Character *p)
{
	Entity * e = p->entParent;

	if( e && e->teamup && e->teamup->team_mentor  )
	{
		Entity * mentor = entFromDbId(e->teamup->team_mentor);
		if(mentor)
		{
			int mentor_level =  character_CalcExperienceLevel(mentor->pchar);
			if( mentor_level != e->teamup->team_level )
			{
				if( teamLock(e, CONTAINER_TEAMUPS) )
				{
					e->teamup->team_level = mentor_level;
					teamUpdateUnlock(e,CONTAINER_TEAMUPS);
				}
			}
		}
		else if( !e->teamup->activetask->sahandle.context ) // make the team leader the mentor
		{
			int mentorChangeToLeader = (e->teamup->team_mentor != e->teamup->members.leader) ? 1 : 0;
			Entity *leader = entFromDbId(e->teamup->members.leader);
			int leaderLevel = 0, leaderLeveled = 0;
			if(leader)
			{
				leaderLevel = character_CalcExperienceLevel(leader->pchar);
				leaderLeveled = (e->teamup->team_level != leaderLevel) ? 1 : 0;
			}
			if ( mentorChangeToLeader || leaderLeveled )
			{
				if( teamLock(e, CONTAINER_TEAMUPS) )
				{
					e->teamup->team_mentor = e->teamup->members.leader;
					if (leaderLeveled)
						e->teamup->team_level = leaderLevel;
					teamUpdateUnlock(e,CONTAINER_TEAMUPS);
				}
			}
		}
	}
}


/**********************************************************************func*
 * Buddy_Tick
 *
 */
void Buddy_Tick(Character *p, float fRate)
{
	char *ach = 0;
	EntPlayer *pl = p->entParent->pl;
	U32 ulNow = dbSecondsSince2000();
	int iLevel = character_CalcExperienceLevel(p);
	int iTeamLevel = character_TeamLevel(p);

	// update and deal with any state changes
	character_UpdateTeamMentor(p);

	switch( p->buddy.eType )
	{
		xcase kBuddyType_None:
		{
			if( character_IsMentor(p) )
			{
				p->buddy.eType = kBuddyType_Mentor;
				p->buddy.uTimeStart = ulNow;
			}
			else if( character_IsSidekick(p,0) )
			{
				p->buddy.uTimeStart = ulNow;
				if( iTeamLevel - iLevel < pl->autoAcceptTeamAbove )
					p->buddy.eType = kBuddyType_Sidekick;
				else
				{
					p->buddy.eType = kBuddyType_Pending;
					p->buddy.iLastTeamLevel = iTeamLevel;
					if( !p->entParent->taskforce || TaskForceIsOnArchitect(p->entParent) )
						sendDialogLevelChange( p->entParent, iTeamLevel, iTeamLevel-1 );
				}
			}
			else if( character_IsExemplar(p,0) )
			{
				p->buddy.uTimeStart = ulNow;
 				if( iLevel - iTeamLevel < pl->autoAcceptTeamAbove )
				{
					p->buddy.eType = kBuddyType_Exemplar;
				}
				else
				{
					p->buddy.eType = kBuddyType_Pending;
					p->buddy.iLastTeamLevel = iTeamLevel;
					if( !p->entParent->taskforce || TaskForceIsOnArchitect(p->entParent) )
						sendDialogLevelChange( p->entParent, iTeamLevel, iTeamLevel );
				}
			}
		}
		xcase kBuddyType_Pending:
		{
			if (p->entParent->teamup)
			{
				if( p->buddy.iLastTeamLevel != iTeamLevel )
				{
					sendDialogLevelChange( p->entParent, iTeamLevel, iTeamLevel-(character_IsSidekick(p,0)?1:0) );
					p->buddy.iLastTeamLevel = iTeamLevel;
				}
				if( PENDING_BUDDY_TIME < ulNow - p->buddy.uTimeStart ) // they've waited long enough, free them.
				{
					if( character_IsSidekick(p,1) )
						p->buddy.eType = kBuddyType_Sidekick;
					if( character_IsExemplar(p,1) )
					{
						p->buddy.eType = kBuddyType_Exemplar;
					}
				}
			}
			else
			{
				p->buddy.eType = kBuddyType_None;
			}
		}
		xcase kBuddyType_Mentor:
		{
			if( !character_IsMentor(p) )
			{
				p->buddy.eType = kBuddyType_None;
				badge_RecordMentorTime(p->entParent, ulNow - p->buddy.uTimeStart);
				badge_RecordAspirantTime(p->entParent, ulNow - p->buddy.uTimeStart);
			}
			else if(p->buddy.uTimeStart>0 && (p->buddy.uTimeStart+60)<ulNow)
			{
				badge_RecordMentorTime(p->entParent, ulNow - p->buddy.uTimeStart);
				badge_RecordAspirantTime(p->entParent, ulNow - p->buddy.uTimeStart);
				p->buddy.uTimeStart = ulNow;
			}
		}
		xcase	kBuddyType_Sidekick:
		{
			if( !character_IsSidekick(p,0) )
			{
				p->buddy.eType = kBuddyType_None;
				badge_RecordSidekickTime(p->entParent, ulNow - p->buddy.uTimeStart);
				character_DestroyAllPendingPets(p);
				p->buddy.uTimeLastMentored = ulNow;
				p->buddy.iCombatLevelLastMentored = p->iCombatLevel;
			}
			else if(p->buddy.uTimeStart>0 && (p->buddy.uTimeStart+60)<ulNow)
			{
				badge_RecordSidekickTime(p->entParent, ulNow - p->buddy.uTimeStart);
				p->buddy.uTimeStart = ulNow;
			}
		}
		xcase kBuddyType_Exemplar:
		{
			if( !character_IsExemplar(p,0) )
			{
				p->buddy.eType = kBuddyType_None;
				badge_RecordExemplarTime(p->entParent, ulNow - p->buddy.uTimeStart);
			}
			else if(p->buddy.uTimeStart>0 && (p->buddy.uTimeStart+60)<ulNow)
			{
				badge_RecordSidekickTime(p->entParent, ulNow - p->buddy.uTimeStart);
				p->buddy.uTimeStart = ulNow;
			}
		}
		default: break;
	}

	character_CalcCombatLevel(p);
	character_HandleCombatLevelChange(p);
}


/**********************************************************************func*
* Buddy_Tick
*
*/
void character_AcceptLevelChange(Character *p, int levelAccepted)
{
	int iTeamLevel = character_TeamLevel(p);

	if( levelAccepted != iTeamLevel)
		return; // level must've changed in the mean time

	if( p->buddy.eType != kBuddyType_Pending )
		return; // already changed level
	
	p->buddy.uTimeStart = dbSecondsSince2000();
	if( character_IsSidekick(p,1) )
		p->buddy.eType = kBuddyType_Sidekick;
	else if( character_IsExemplar(p,1) )
	{
		p->buddy.eType = kBuddyType_Exemplar;
	}
	else if( character_IsMentor(p) )
		p->buddy.eType = kBuddyType_Mentor;
	else
		p->buddy.eType = kBuddyType_None; // team dissolved?  
}

void character_updateTeamLevel(Character *p)
{
	Entity * e = p->entParent;

	if( e && e->teamup && teamLock(e, CONTAINER_TEAMUPS ) )
	{
		e->teamup->team_level = character_CalcExperienceLevel(p);
		teamUpdateUnlock(e, CONTAINER_TEAMUPS);
	}
	else
	{
		// this is not good
	}
}


/* End of File */
