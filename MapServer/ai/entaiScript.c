#include "aiBehaviorPublic.h"
#include "character_base.h"
#include "character_pet.h"
#include "entaiLog.h"
#include "entaiprivate.h"
#include "entity.h"
#include "scriptengine.h"

void aiScriptSetGang(Entity * agent, int iGangID)
{
	if(!agent)
	{
		ScriptError("Entity passed into aiScriptSetGang not valid");
		return;
	}
	else
	{
		char *behaviorStr = NULL;
		estrCreate(&behaviorStr);
		estrPrintf(&behaviorStr, "Gang(%d)", iGangID);
		aiBehaviorAddPermFlag( agent, ENTAI(agent)->base, behaviorStr );
		estrDestroy(&behaviorStr);
	}
}

void aiScriptSetFollower(Entity * leader, Entity * follower)
{
	AIVars* followerAi;
	AIVars* leaderAi;

	if (!leader || !(leaderAi = ENTAI(leader)) || !follower || !(followerAi = ENTAI(follower)))
	{
		ScriptError("Leader and/or follower passed into aiScriptSetFollower invalid");
		return;
	}

	followerAi->returnToSpawnDistance = 20;     
	if (follower->pchar)
		character_BecomePet(follower->pchar, leader, NULL);
	else
	{
		PlayerRemovePet(erGetEnt(follower->erCreator), follower);
		PlayerAddPet(leader, follower);
		follower->erCreator = erGetRef(leader);
		follower->erOwner = erGetRef(leader);
	}
	followerAi->protectEnt = erGetRef(leader); 
	followerAi->HBEsearchdist = 50;

	if (ENTTYPE(leader) != ENTTYPE_PLAYER)
	{
		AI_LOG(AI_LOG_TEAM, (leader, "aiScriptSetFollower is joining \"%s\" to \"%s\"\n", follower->name, leader->name));
		AI_LOG(AI_LOG_TEAM, (follower, "aiScriptSetFollower is joining \"%s\" to \"%s\"\n", follower->name, leader->name));
		aiTeamJoin(&leaderAi->teamMemberInfo, &followerAi->teamMemberInfo);
	}
}

// returns NULL on error or if no entities have hit specified ent.
Entity * aiScriptGetLastHitBy(Entity * e)
{
	AIVars * ai;
	Entity * minEntity = 0;
	U32 minTime = UINT_MAX;
	AIProxEntStatus *status;

	if(!e || !(ai = ENTAI(e)))
	{
		ScriptError("Entity passed into aiScriptGetLastHitBy is not valid");
		return NULL;
	}

	for (status = ai->proxEntStatusTable->statusHead; status; status = status->tableList.next)
	{
		if (   status->entity
			&& status->hasAttackedMe
			&& status->time.attackMe < minTime)
		{
			minTime		= status->time.attackMe;
			minEntity	= status->entity;
		}
	}

	return minEntity;
}