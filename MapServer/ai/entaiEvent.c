
#include "entaiprivate.h"
#include <stddef.h>
#include "encounter.h"
#include "textparser.h"
#include "motion.h"
#include "svr_base.h"
#include "cmdcontrols.h"
#include "storyarcutil.h"
#include "entity.h"
#include "megaGrid.h"
#include "powers.h"
#include "character_combat.h"
#include "character_base.h"
#include "timing.h"
#include "cmdserver.h"
#include "mathutil.h"
#include "position.h"
#include "entaiLog.h"
#include "entai.h"
#include "earray.h"
#include "scriptvars.h"

extern TokenizerParseInfo ParseCharacterAttributes[];

char *dbg_GetParseName(TokenizerParseInfo atpi[], int offset);

// AIEvent ---------------------------------------------------------------------------

MP_DEFINE(AIEvent);

static AIEvent* createAIEvent(){
	AIEvent* event;
	
	MP_CREATE(AIEvent, 1000);
	
	event = MP_ALLOC(AIEvent);
	
	return event;
}

static void destroyAIEvent(AIEvent* event){
	if(!event)
		return;

	MP_FREE(AIEvent, event);
}

static void aiEventClearEventSet(AIEventSet* set){
	AIEvent* event = set->events;
	
	while(event){
		AIEvent* next = event->next;
		destroyAIEvent(event);
		event = next;
	}
	
	ZeroStruct(set);
}

void aiEventClearHistory(AIVars* ai){
	AIEventHistory* eventHistory = &ai->eventHistory;
	int i;
	
	for(i = 0; i < AI_EVENT_HISTORY_SET_COUNT; i++){
		aiEventClearEventSet(eventHistory->eventSet + i);
	}
	
	eventHistory->curEventSet = 0;
	eventHistory->curEventSetStartTime = ABS_TIME;
}

void aiEventUpdateEvents(AIVars* ai){
	AIEventHistory* eventHistory = &ai->eventHistory;
	unsigned int timeDiff = ABS_TIME - eventHistory->curEventSetStartTime;
	unsigned int sets = timeDiff / SEC_TO_ABS(AI_EVENT_HISTORY_SET_TIME);
	
	if(sets && sets < AI_EVENT_HISTORY_SET_COUNT){
		// Kill any events that are so far back that I don't care about them anymore.
	
		eventHistory->curEventSetStartTime += sets * SEC_TO_ABS(AI_EVENT_HISTORY_SET_TIME);

		while(sets--){
			eventHistory->curEventSet = (eventHistory->curEventSet + 1) & (AI_EVENT_HISTORY_SET_COUNT - 1);

			aiEventClearEventSet(eventHistory->eventSet + eventHistory->curEventSet);
		}
	}else{
		// Kill the entire history cuz no events are recent enough.
	
		aiEventClearHistory(ai);
	}
}

static AIEventSet* aiEventAddEvent(AIVars* ai, AIEvent* event){
	AIEventSet* set;
	AIEventHistory* eventHistory = &ai->eventHistory;
	unsigned int eventType;
	unsigned int target;
	
	destroyAIEvent(event);
	
	return NULL;

	aiEventUpdateEvents(ai);
	
	set = eventHistory->eventSet + eventHistory->curEventSet;
	
	event->timeStamp = ABS_TIME;
	event->next = set->events;
	set->events = event;
	set->eventCount++;
	
	// Increment values for this set.

	eventType = AI_EVENT_GET_FLAG(event->flags, TYPE);
	target = AI_EVENT_GET_FLAG(event->flags, TARGET);
	
	if(eventType == AI_EVENT_TYPE_ATTACKED){
		switch(target){
			case AI_EVENT_TARGET_ME:
				set->attackedCount.me++;
				set->damageTaken.me += event->attack.hp;
				break;
			case AI_EVENT_TARGET_FRIEND:
				set->attackedCount.myFriend++;
				set->damageTaken.myFriend += event->attack.hp;
				break;
			case AI_EVENT_TARGET_ENEMY:
				set->attackedCount.enemy++;
				set->damageTaken.enemy += event->attack.hp;
				break;
		}
	}
	else if(eventType == AI_EVENT_TYPE_MISSED){
		switch(target){
			case AI_EVENT_TARGET_ME:
				set->wasMissedCount.me++;
				break;
			case AI_EVENT_TARGET_FRIEND:
				set->wasMissedCount.myFriend++;
				break;
			case AI_EVENT_TARGET_ENEMY:
				set->wasMissedCount.enemy++;
				break;
		}
	}
	
	return set;
}

typedef enum aiAttribType {
	ATTRIB_UNUSED,
	ATTRIB_HITPOINTS,
	ATTRIB_IMMOBILIZE,
	ATTRIB_FEAR,
	ATTRIB_TAUNT,
	ATTRIB_PUSH,
	ATTRIB_AVOID,
	ATTRIB_PLACATE,

	ATTRIB_COUNT,
} aiAttribType;

#define GET_STATUS(var,entity) if(!var) var = aiGetProxEntStatus(ai->proxEntStatusTable, entity, 1)

#define FEELMOD(feelingID, levelBegin, levelEnd, timeBegin, timeEnd) aiFeelingAddModifierEx(e, ai, feelingID, levelBegin, levelEnd, timeBegin, timeEnd)
#define FEELSET(feelingSet, feelingID, amount) aiFeelingSetValue(e, ai, feelingSet, feelingID, amount)
#define FEELADD(feelingSet, feelingID, amount) aiFeelingAddValue(e, ai, feelingSet, feelingID, amount)

// Static variables for these event handler functions to use.  Yes this is incredibly ugly, but useful.

static AIEventNotifyParams params;

static struct {
	aiAttribType				attribType;
	int						isNegative;

	struct {
		AIProxEntStatus* 	source;
		AIProxEntStatus* 	target;
		AIProxEntStatus* 	sourceOwner;
	} status;
} powerSucceeded;

static struct {
	aiAttribType	attribType;
} powerEnded;

static struct {
	Entity*		entity;
	EntityRef	entityRef;
	AIVars*		ai;
	int			isMe;
	int			isMyFriend;
	int			isMeOrMyFriend;
	int			isMyEnemy;
	int			isMyOwner;
	Entity* 	owner;
	AIVars* 	aiOwner;
} source, target;

static void aiEventSetRelationShips(Entity* e, AIVars* ai){
	// Set source information.

	source.isMe = e == source.entity;

	if(source.entity){
		source.isMyOwner =	source.entityRef && e->erOwner == source.entityRef;

		if(	!source.isMe && source.ai &&
			((source.isMyOwner && source.entity->pchar && e->pchar && source.entity->pchar->iGangID == e->pchar->iGangID) || 
			source.ai->teamMemberInfo.team == ai->teamMemberInfo.team))
		{
			source.isMyFriend =	aiIsMyFriend(e, source.entity);
		}
		else
		{
			source.isMyFriend =	0;
		}
							
		source.isMeOrMyFriend = source.isMe || source.isMyFriend;

		source.isMyEnemy =	source.isMeOrMyFriend ? 0 : aiIsMyEnemy(e, source.entity);
	}else{
		source.isMyFriend = source.isMeOrMyFriend = source.isMyEnemy = source.isMyOwner = 0;
	}
	
	// Set target information.
	
	target.isMe = e == target.entity;

	if(target.entity){
		target.isMyOwner =	target.entityRef && e->erOwner == target.entityRef;

		if(	!target.isMe && target.ai &&
			(target.isMyOwner || target.ai->teamMemberInfo.team == ai->teamMemberInfo.team))
		{
			target.isMyFriend =	aiIsMyFriend(e, target.entity);
		}
		else
		{
			target.isMyFriend =	0;
		}

		target.isMeOrMyFriend = target.isMe || target.isMyFriend;

		target.isMyEnemy =	target.isMeOrMyFriend ? 0 : aiIsMyEnemy(e, target.entity);
	}else{
		target.isMyFriend = target.isMeOrMyFriend = target.isMyEnemy = target.isMyOwner = 0;
	}
}

static void aiEventHandleAttacked(Entity* e, AIVars* ai){
	if(target.isMe){
		// Set my proximity bothered level to full when I'm attacked.
		
		FEELSET(AI_FEELINGSET_PERMANENT, AI_FEELING_BOTHERED, 1);

		if(!ai->teamMemberInfo.alerted && !source.entity->untargetable){
			AI_LOG(	AI_LOG_ENCOUNTER,
					(e,
					"^2ALERTING ^dself because I was hit.\n"));

			EncounterAISignal(e, ENCOUNTER_ALERTED);
			
			ai->teamMemberInfo.alerted = 1;
			
			ai->time.checkedProximity = 0;
		}
		
		if(params.powerSucceeded.revealSource || powerSucceeded.attribType==ATTRIB_PLACATE)
		{
			if(!ai->wasAttacked){
				ai->wasAttacked = 1;
				
				ai->time.firstAttacked = ABS_TIME;
			}
			
			ai->time.wasAttacked = ABS_TIME;
			
			if(ai->teamMemberInfo.team){
				ai->teamMemberInfo.team->time.wasAttacked = ABS_TIME;
			}
			
			if(params.powerSucceeded.ppowBase && params.powerSucceeded.ppowBase->fRange <= 8.5){
				ai->time.wasAttackedMelee = ABS_TIME;
			}

			if(	powerSucceeded.attribType == ATTRIB_HITPOINTS &&
				powerSucceeded.isNegative)
			{
				ai->time.wasDamaged = ABS_TIME;
				
				if(	ai_state.alwaysKnockPlayers &&
					ENTTYPE(target.entity) == ENTTYPE_PLAYER &&
					target.entity->client)
				{
					FuturePush* fp = addFuturePush(&target.entity->client->controls, SEC_TO_ABS(0));
					Vec3 diff;
					
					fp->do_knockback_anim = 1;
					
					subVec3(ENTPOS(target.entity), ENTPOS(source.entity), diff);
					diff[1] = 0;
					normalVec3(diff);
					scaleVec3(diff, 2, diff);
					diff[1] = 1;
					
					copyVec3(diff, fp->add_vel);
				}
			}
			
			GET_STATUS(powerSucceeded.status.source, source.entity);
		
			powerSucceeded.status.source->time.posKnown = ABS_TIME;
			powerSucceeded.status.source->time.attackMe = ABS_TIME;
			powerSucceeded.status.source->hasAttackedMe = 1;
			
			if(source.owner){
				GET_STATUS(powerSucceeded.status.sourceOwner, source.owner);

				powerSucceeded.status.sourceOwner->time.posKnown = ABS_TIME;
				powerSucceeded.status.sourceOwner->time.attackMe = ABS_TIME;
				powerSucceeded.status.sourceOwner->hasAttackedMe = 1;
			}
		
			switch(powerSucceeded.attribType){
				case ATTRIB_TAUNT:{
					U32 duration = SEC_TO_ABS(params.powerSucceeded.fAmount / 30.0);
					AIProxEntStatus* targetStatus = ai->attackTarget.status;

					if(	!targetStatus
						||
						(	ABS_TIME_SINCE(targetStatus->taunt.begin) >= targetStatus->taunt.duration ||
							targetStatus->taunt.begin && duration > 2 * (targetStatus->taunt.duration - ABS_TIME_SINCE(targetStatus->taunt.begin))))
					{
						powerSucceeded.status.source->taunt.duration = duration;
						powerSucceeded.status.source->taunt.begin = ABS_TIME;
					}
					else
					{
						AI_LOG(	AI_LOG_COMBAT,
							(e,
							"Sorry, %s^0, that wasn't enough taunt.\n",
							AI_LOG_ENT_NAME(source.entity))); 
					}
					break;
				}
				case ATTRIB_PLACATE:
				{
					U32 duration = SEC_TO_ABS(params.powerSucceeded.fAmount / 30.0);
					AIProxEntStatus* targetStatus = ai->attackTarget.status;

					if(	!targetStatus
						||
						(	ABS_TIME_SINCE(targetStatus->placate.begin) >= targetStatus->placate.duration ||
							targetStatus->placate.begin && duration > 2 * (targetStatus->placate.duration - ABS_TIME_SINCE(targetStatus->placate.begin))))
					{
						powerSucceeded.status.source->placate.duration = duration;
						powerSucceeded.status.source->placate.begin = ABS_TIME;
						ZeroStruct(&powerSucceeded.status.source->time);
						ZeroStruct(&powerSucceeded.status.source->damage);
						aiDiscardAttackTarget(powerSucceeded.status.source->entity, "Was placated.");
					}
					else
					{
						AI_LOG(	AI_LOG_COMBAT,
							(e,
							"Sorry, %s^0, that wasn't enough placate.\n",
							AI_LOG_ENT_NAME(source.entity))); 
					}
					break;
				}
			}
			ai->time.friendWasAttacked = ABS_TIME;
		}
	}else{
		// Target is my friend.
		
		int alertMe = 0;
		
		if(params.powerSucceeded.revealSource){
			if(ai->teamMemberInfo.alerted){
				alertMe = 1;
			}
			else if(powerSucceeded.attribType == ATTRIB_PUSH && target.ai &&
					distance3Squared(ENTPOS(e), ENTPOS(target.entity)) <= SQR(target.ai->shoutRadius))
			{
				alertMe = 1;

				AI_LOG(	AI_LOG_ENCOUNTER,
						(e,
						"^2ALERTING ^dself because my friend was knocked over by %s^d.\n",
						AI_LOG_ENT_NAME(source.entity)));

				EncounterAISignal(e, ENCOUNTER_ALERTED);
				
				ai->teamMemberInfo.alerted = 1;

				FEELSET(AI_FEELINGSET_PERMANENT, AI_FEELING_BOTHERED, 1);
			}
		}

		if(alertMe){
			GET_STATUS(powerSucceeded.status.source, source.entity);
				
			powerSucceeded.status.source->time.attackMyFriend = ABS_TIME;
			powerSucceeded.status.source->time.powerUsedOnMyFriend = ABS_TIME;
			powerSucceeded.status.source->hasAttackedMyFriend = 1;

			if(	target.isMyOwner &&
				(	ENTTYPE(source.entity) == ENTTYPE_PLAYER
					||
					source.owner &&
					ENTTYPE(source.owner) == ENTTYPE_PLAYER))
			{
				powerSucceeded.status.source->time.posKnown = ABS_TIME;
			}

			if(source.owner){
				GET_STATUS(powerSucceeded.status.sourceOwner, source.owner);

				powerSucceeded.status.sourceOwner->time.attackMyFriend = ABS_TIME;
				powerSucceeded.status.sourceOwner->time.powerUsedOnMyFriend = ABS_TIME;
				powerSucceeded.status.sourceOwner->hasAttackedMyFriend = 1;
			}

			ai->time.friendWasAttacked = ABS_TIME;
		}
	}
}

static void aiEventNotifyPowerSucceeded(Entity* e, AIVars* ai)
{
	F32 amount = params.powerSucceeded.fAmount;

	aiEventSetRelationShips(e, ai);
	
	// Initialize the statuses.
	
	ZeroStruct(&powerSucceeded.status);
	
	if(target.isMe && powerSucceeded.attribType == ATTRIB_AVOID)
	{
		// When an avoid succeeds:
		//   params.powerSucceeded.fAmount    = highest relative level enemy that should avoid me.
		//   params.powerSucceeded.fRadius    = radius around me that should be avoided.
		//   params.powerSucceeded.uiInvokeID = unique id for this instance of the power.
		
		aiAddAvoidInstance(	e, ai,
							params.powerSucceeded.fAmount,
							params.powerSucceeded.fRadius,
							params.powerSucceeded.uiInvokeID);
	}

	if(target.isMeOrMyFriend && source.isMyEnemy)
	{
		aiEventHandleAttacked(e, ai);
	}
	else if(params.powerSucceeded.revealSource && source.isMe && target.isMyEnemy)
	{
		if(target.entity == ai->attackTarget.entity)
		{
			ai->attackTarget.hitCount++;
		}
	}

	if(	!params.powerSucceeded.revealSource	||	target.isMyFriend && target.ai && target.ai->teamMemberInfo.doNotShareInfoWithMembers)
	{
		return;
	}

	if(powerSucceeded.attribType == ATTRIB_HITPOINTS)
	{
		AIEvent event = {0};
		int killEvent = 0;
		
		// Set the event type.
		
		if(powerSucceeded.isNegative){
			// Someone was hit.
			
			AI_EVENT_SET_FLAG(event.flags, TYPE, ATTACKED);

			event.attack.hp = -amount;
			
			if(target.isMeOrMyFriend){
				GET_STATUS(powerSucceeded.status.source, source.entity);
				
				//EncounterAISignal(e, ENCOUNTER_ALERTED);

				if(target.isMe){
					// I'm being damaged.
					
					float hp_ratio;
					
					// Get the HP ratio.
					
					hp_ratio = -amount / target.entity->pchar->attrMax.fHitPoints;

					if(hp_ratio > 1)
						hp_ratio = 1;
					
					if(ai->feeling.trackFeelings){
						FEELMOD(AI_FEELING_LOYALTY,
								-hp_ratio * 0.1 * ai->feeling.scale.meAttacked,
								0,
								ABS_TIME,
								ABS_TIME + SEC_TO_ABS(10));

						FEELMOD(AI_FEELING_FEAR,
								0.1 * ai->feeling.scale.meAttacked,
								0,
								ABS_TIME,
								ABS_TIME + SEC_TO_ABS(5));
					}

					powerSucceeded.status.source->damage.toMe += -amount;

					if(source.owner){
						GET_STATUS(powerSucceeded.status.sourceOwner, source.owner);

						powerSucceeded.status.sourceOwner->damage.toMe += -amount * 0.1;
					}
				}else{
					// My friend is being damaged.
					
					// If a lieutenant is being attacked, and I'm a minion, take it personally
					if (target.ai && target.ai->brain.type == AI_BRAIN_BANISHED_PANTHEON_LT &&
						ai->brain.type == AI_BRAIN_BANISHED_PANTHEON)
					{
						powerSucceeded.status.source->damage.toMe += -amount * 2;
						if(source.owner){
							GET_STATUS(powerSucceeded.status.sourceOwner, source.owner);
							powerSucceeded.status.sourceOwner->damage.toMe += -amount * 0.1 * 2;
						}
					}
					if(ai->ignoreFriendsAttacked){						   
						killEvent = 1;
					}else{
						float hp_ratio = -amount / target.entity->pchar->attrMax.fHitPoints;

						if(ai->feeling.trackFeelings){
							FEELMOD(AI_FEELING_LOYALTY,
									hp_ratio * 0.2 * ai->feeling.scale.friendAttacked,
									0,
									ABS_TIME,
									ABS_TIME + SEC_TO_ABS(10));
						}

						powerSucceeded.status.source->damage.toMyFriends += -amount;

						if(source.owner){
							GET_STATUS(powerSucceeded.status.sourceOwner, source.owner);

							powerSucceeded.status.sourceOwner->damage.toMyFriends += -amount * 0.1;
						}
					}
				}
			}
			else if(target.isMyEnemy){
				// Lower my permanent fear when an enemy is attacked.  This makes no sense.
				
				FEELADD(AI_FEELINGSET_PERMANENT, AI_FEELING_FEAR, 0.1 * amount / target.entity->pchar->attrMax.fHitPoints);

				if(source.isMe){
					// I'm damaging someone.
					
					GET_STATUS(powerSucceeded.status.target, target.entity);
					
					if(!target.entity->invincible){
						powerSucceeded.status.target->damage.fromMe += -amount;
					}

					if(ai->feeling.trackFeelings){
						FEELMOD(AI_FEELING_CONFIDENCE,
								-amount / target.entity->pchar->attrMax.fHitPoints,
								0,
								ABS_TIME,
								ABS_TIME + SEC_TO_ABS(10));
					}
				}
				else if(source.isMyFriend){
					// My friend is damaging someone.
					
					if(ai->feeling.trackFeelings){
						FEELMOD(AI_FEELING_CONFIDENCE,
								0.1 * -amount / target.entity->pchar->attrMax.fHitPoints,
								0,
								ABS_TIME,
								ABS_TIME + SEC_TO_ABS(10));
					}
				}
			}
		}else{
			// Someone is being healed.
			
			AI_EVENT_SET_FLAG(event.flags, TYPE, HEALED);

			event.heal.hp = amount;
		}

		// Set the target.
		
		if(target.isMe){
			AI_EVENT_SET_FLAG(event.flags, TARGET, ME);
		}
		else if(target.isMyFriend){
			AI_EVENT_SET_FLAG(event.flags, TARGET, FRIEND);
		}
		else if(target.isMyEnemy){
			AI_EVENT_SET_FLAG(event.flags, TARGET, ENEMY);
		}
		else{
			killEvent = 1;
		}
		
		// Set the source.
		
		if(source.isMe){
			AI_EVENT_SET_FLAG(event.flags, SOURCE, ME);
		}
		else if(source.isMyFriend){
			AI_EVENT_SET_FLAG(event.flags, SOURCE, FRIEND);
		}
		else if(source.isMyEnemy){
			AI_EVENT_SET_FLAG(event.flags, SOURCE, ENEMY);
		}
		else{
			killEvent = 1;
		}
		
		// Add the event.
		
		if(!killEvent)
		{
			AIEvent* newEvent = createAIEvent();
			*newEvent = event;
			aiEventAddEvent(ai, newEvent);
		}
	}
}

static void aiEventNotifyPowerFailed(Entity* e, AIVars* ai){
	AIEvent event;

	aiEventSetRelationShips(e, ai);

	if(params.powerFailed.revealSource)
	{
		AI_EVENT_SET_FLAG(event.flags, TYPE, MISSED);
		
		// Set the target.
		
		if(target.isMe){
			AIProxEntStatus* status = aiGetProxEntStatus(ai->proxEntStatusTable, source.entity, 1);

			AI_EVENT_SET_FLAG(event.flags, TARGET, ME);
			
			status->time.posKnown = ABS_TIME;
			status->time.attackMe = ABS_TIME;
			status->hasAttackedMe = 1;
		}
		else if(target.isMyFriend)
		{
			AIProxEntStatus* status = aiGetProxEntStatus(ai->proxEntStatusTable, source.entity, 1);

			AI_EVENT_SET_FLAG(event.flags, TARGET, FRIEND);
			
			status->time.powerUsedOnMyFriend = ABS_TIME;
			status->hasAttackedMyFriend = 1;
		}
		else if(target.isMyEnemy)
		{
			AI_EVENT_SET_FLAG(event.flags, TARGET, ENEMY);
		}

		// Set the source.
		
		if(source.isMe){
			AI_EVENT_SET_FLAG(event.flags, SOURCE, ME);

			if(!target.isMe && !target.isMyFriend){
				float pctSourceHP = source.entity->pchar->attrCur.fHitPoints / source.entity->pchar->attrMax.fHitPoints;
				float pctTargetHP = target.entity->pchar->attrCur.fHitPoints / source.entity->pchar->attrMax.fHitPoints;
				float diff = pctTargetHP - pctSourceHP;
				
				if(ai->feeling.trackFeelings){
					FEELMOD(AI_FEELING_CONFIDENCE, -0.1, 0, ABS_TIME, ABS_TIME + SEC_TO_ABS(5));
				}
			}
		}
		else if(source.isMyFriend){
			AI_EVENT_SET_FLAG(event.flags, SOURCE, FRIEND);
		}
		else if(source.isMyEnemy){
			AI_EVENT_SET_FLAG(event.flags, SOURCE, ENEMY);

			if(target.isMe){
				// Add some permanent bother when someone misses me with an attack.
			
				FEELADD(AI_FEELINGSET_PERMANENT, AI_FEELING_BOTHERED, 1);
			}
			if(target.isMeOrMyFriend){
				ai->time.friendWasAttacked = ABS_TIME;
			}
		}

		// Add the event.

		{
			AIEvent* newEvent = createAIEvent();
			*newEvent = event;
			aiEventAddEvent(ai, newEvent);
		}
	}
	
	if(source.isMe && params.powerFailed.targetNotOnGround && params.powerFailed.ppow){
		AIPowerInfo* info = params.powerFailed.ppow->pAIPowerInfo;
		
		if(info){
			info->doNotChoose.startTime = ABS_TIME;
			info->doNotChoose.duration = SEC_TO_ABS(10);
		}
	}
}

static void aiEventNotifyPowerEnded(Entity* e, AIVars* ai){
	aiEventSetRelationShips(e, ai);
	
	if(target.isMe && powerEnded.attribType == ATTRIB_AVOID){
		aiRemoveAvoidInstance(e, ai, params.powerEnded.uiInvokeID);
	}
}

static void aiEventNotifyEntityKilled(Entity* e, AIVars* ai){
	AIProxEntStatus*	sourceStatus = NULL;
	int					targetIsMyAttackTarget;
	int					responsePowerGroup = 0;
	int					doResponseDialog = 0;
	Entity*				responseEnt = NULL;

	aiEventSetRelationShips(e, ai);

	// Get stuff about the target entity.
	
	if(target.entity){
		targetIsMyAttackTarget = ai->attackTarget.entity == target.entity;
	}else{
		targetIsMyAttackTarget = 0;
	}
	
	if(target.isMe){
		GET_STATUS(sourceStatus, source.entity);
		
		sourceStatus->hasAttackedMe = 1;
		sourceStatus->damage.toMe += 0;
		sourceStatus->time.posKnown = ABS_TIME;

		// I have been defeated!  Ungh...
		
		// TODO: this will make the count insane for masterminds over time, check if you're not someone's pet?
		if(ai->teamMemberInfo.team){
			ai->teamMemberInfo.team->members.killedCount++;
		}
		
		FEELADD(AI_FEELINGSET_PERMANENT, AI_FEELING_BOTHERED, 1);
	}
	else if(target.isMyFriend){
		if(source.isMyEnemy){
			// You killed my friend, EEK!!

			if(	!ai->ignoreFriendsAttacked
				&& target.ai
				&& 
				(	ai->teamMemberInfo.alerted
					||
					distance3Squared(ENTPOS(e), ENTPOS(target.entity)) < SQR(target.ai->shoutRadius))
				)
			{
				float scale = ai->feeling.scale.friendAttacked;
				float distSQR;

				if(!ai->teamMemberInfo.alerted){
					AI_LOG(	AI_LOG_ENCOUNTER,
							(e,
							"Alerting encounter because my friend was killed.\n"));

					EncounterAISignal(e, ENCOUNTER_ALERTED);
					
					ai->teamMemberInfo.alerted = 1;
				}
				
				GET_STATUS(sourceStatus, source.entity);

				sourceStatus->time.posKnown = ABS_TIME;
				sourceStatus->time.attackMyFriend = ABS_TIME;
				sourceStatus->time.powerUsedOnMyFriend = ABS_TIME;

				// Scale by the number of live teammates.
				
				if(	ai->teamMemberInfo.team &&
					ai->teamMemberInfo.team->members.aliveCount)
				{
					scale /= (float)ai->teamMemberInfo.team->members.aliveCount;
				}
				
				// Scale by distance from the target.
				
				distSQR = distance3Squared(ENTPOS(e), ENTPOS(target.entity));
				
				if(distSQR < SQR(100)){
					scale *= 3.0 * (SQR(100) - distSQR) / SQR(100);
				}else{
					scale = 0;
				}
				
				// Scale by how much health I have.
				
				scale *= 1.0 - 0.9 * e->pchar->attrCur.fHitPoints / e->pchar->attrMax.fHitPoints;

				if(ai->feeling.trackFeelings){
					FEELMOD(AI_FEELING_FEAR,
							0.8 * scale,
							0.7 * scale,
							ABS_TIME + SEC_TO_ABS(.5),
							ABS_TIME + SEC_TO_ABS(10 + (rand() % 10) * 0.3f));
											
					FEELMOD(AI_FEELING_CONFIDENCE,
							-0.5 * scale,
							-0.3 * scale,
							ABS_TIME,
							ABS_TIME + SEC_TO_ABS(5));
				}

				FEELADD(AI_FEELINGSET_PERMANENT, AI_FEELING_BOTHERED, 1);
			}	
		}
	}
	else if (source.isMe && target.isMyEnemy) {
		// I just killed something!  Wee!
		responsePowerGroup = AI_POWERGROUP_VICTOR;
		responseEnt = source.entity;
		doResponseDialog = 1;
	}
	if (responsePowerGroup) {
		AIPowerInfo* powInfo = aiChoosePower(responseEnt, ENTAI(responseEnt), responsePowerGroup);
		if(	powInfo) {
			Power* ppow = powInfo->power;
			// We have a power we want to execute upon victory
			if (responseEnt->pchar->ppowQueued) {
				character_KillQueuedPower(responseEnt->pchar);
			}
			if(!ppow->bActive && ppow->fTimer == 0){
				aiUsePower(responseEnt, powInfo, NULL);
			}
		}
	}
	//MW added: say what you say when you kill a hero 
	if( doResponseDialog )
	{
		if( responseEnt->sayOnKillHero && !responseEnt->IAmInEmoteChatMode)
		{
			ScriptVarsTable vars = {0};

			if( target.entity )
				responseEnt->audience = erGetRef(target.entity);
			
			if(EncounterVarsFromEntity(responseEnt))
				ScriptVarsTableScopeCopy(&vars, EncounterVarsFromEntity(responseEnt));
			saBuildScriptVarsTable(&vars, target.entity, SAFE_MEMBER(target.entity, db_id), 0, NULL, NULL, NULL, NULL, NULL, 0, 0, false);

			saUtilEntSays(responseEnt, 0, saUtilTableAndEntityLocalize(responseEnt->sayOnKillHero, &vars, target.entity));
		}
	}
}

static aiAttribType aiEventGetCurrentAttribType(){
	if(IS_HITPOINTS((size_t)params.powerSucceeded.offAttrib)){
		return ATTRIB_HITPOINTS;
	}else{
		#define ATTRIB_OFFSET(a) offsetof(CharacterAttributes, a)
		switch(params.powerSucceeded.offAttrib){
			case ATTRIB_OFFSET(fImmobilized):
				return ATTRIB_IMMOBILIZE;
			case ATTRIB_OFFSET(fAfraid):
				return ATTRIB_FEAR;
			case ATTRIB_OFFSET(fTaunt):
				return ATTRIB_TAUNT;
			case ATTRIB_OFFSET(fPlacate):
				return ATTRIB_PLACATE;
			case ATTRIB_OFFSET(fKnockback):
			case ATTRIB_OFFSET(fKnockup):
			case ATTRIB_OFFSET(fRepel):
				return ATTRIB_PUSH;
			case kSpecialAttrib_Avoid:
				return ATTRIB_AVOID;
			default:
				return ATTRIB_UNUSED;
		}
		#undef ATTRIB_OFFSET
	}
}

static int aiEventNotifySetupNotification(int* notifyAreaOut){
	int		notifyArea = 1;

	// Setup source entity.
	
	source.entity =		params.source;
	source.entityRef =	erGetRef(source.entity);
	source.ai =			source.entity ? ENTAI(source.entity) : NULL;
	source.owner =		erGetEnt(source.entity->erOwner);
	source.aiOwner =	source.owner ? ENTAI(source.owner) : NULL;
	
	// Setup target entity.

	target.entity =		params.target;
	target.entityRef =	erGetRef(target.entity);
	target.ai =			target.entity ? ENTAI(target.entity) : NULL;
	target.owner =		erGetEnt(target.entity->erOwner);
	target.aiOwner =	target.owner ? ENTAI(target.owner) : NULL;

	switch(params.notifyType)
	{
		case AI_EVENT_NOTIFY_POWER_SUCCEEDED:
			{

			if(!source.entity || !target.entity)
				return 0;
		
			powerSucceeded.attribType = aiEventGetCurrentAttribType();

			if(powerSucceeded.attribType != ATTRIB_AVOID)
			{
				if(source.entity == target.entity)
					return 0;
				
				if(params.powerSucceeded.fAmount == 0 && (!target.entity || !target.ai->perceiveAttacksWithoutDamage))
					return 0;
			}

			powerSucceeded.isNegative = params.powerSucceeded.fAmount < 0;
			if(!params.powerSucceeded.revealSource && params.powerSucceeded.offAttrib != offsetof(CharacterAttributes, fPlacate) )
				notifyArea = 0;
						
			break;
		}

		case AI_EVENT_NOTIFY_POWER_FAILED:{
			if(!source.entity || !target.entity){
				return 0;
			}

			if(!params.powerFailed.revealSource){
				notifyArea = 0;
			}
			
			if(params.powerFailed.ppow){
				AIPowerInfo* info = params.powerFailed.ppow->pAIPowerInfo;
				if(info){
					info->time.lastFail = ABS_TIME;
					
					if(info->teamInfo){
						info->teamInfo->time.lastFail = ABS_TIME;
					}
				}
			}

			break;
		}
		
		case AI_EVENT_NOTIFY_POWER_ENDED:{
			powerEnded.attribType = aiEventGetCurrentAttribType();
			notifyArea = 0;
			break;
		}

		case AI_EVENT_NOTIFY_PET_CREATED:{
			if(!source.entity || !target.entity){
				return 0;
			}

			if(	source.ai &&
				source.ai->brain.type == AI_BRAIN_DEVOURING_EARTH_LT &&
				source.ai->brain.devouringearth.state == DES_WAITING_FOR_PET)
			{
				assert(target.entity);
				source.ai->brain.devouringearth.pet = erGetRef(target.entity);
				source.ai->brain.devouringearth.state = DES_HAS_PET;
			}
			notifyArea = 0;
			break;
		}
	}
	
	*notifyAreaOut = notifyArea;
	return 1;
}

void aiEventNotify(AIEventNotifyParams* paramsLocal)
{
	float	radiusSQR = SQR(100);
	int		entArray[MAX_ENTITIES_PRIVATE];
	int		entCount;
	int		i;
	int		j;
	int		foundSource = 0;
	int		foundTarget = 0;
	int		notifyArea;

	// Set the static variables.
	
	params = *paramsLocal;

	// Pre-calculate some stuff.

	PERFINFO_AUTO_START("aiEventNotify", 1);

	if(!aiEventNotifySetupNotification(&notifyArea)){
		PERFINFO_AUTO_STOP();
		return;
	}
	
	// Check if we are actually going to notify any of the nearby things

	if(params.notifyType != AI_EVENT_NOTIFY_POWER_SUCCEEDED &&
		params.notifyType != AI_EVENT_NOTIFY_POWER_FAILED &&
		params.notifyType != AI_EVENT_NOTIFY_POWER_ENDED &&
		params.notifyType != AI_EVENT_NOTIFY_ENTITY_KILLED)
	{
		PERFINFO_AUTO_STOP();
		return;
	}
	
	// Notify nearby things.
	
	if(notifyArea){
		entCount = mgGetNodesInRange(0, ENTPOS(target.entity), (void**)entArray, 0);
	}else{
		entCount = 0;
	}
	
	// Make sure the source and the target are both in the notification list.
	
	for(i = 0, j = 0; i < entCount; i++){
		int e_idx = entArray[i];
		Entity* e = entities[e_idx];
		float posDistSQR = distance3Squared(ENTPOS_BY_ID(e_idx), ENTPOS(target.entity));
		
		if(e == source.entity){
			foundSource = 1;
			entArray[j++] = e_idx;
		}
		else if(e == target.entity){
			foundTarget = 1;
			entArray[j++] = e_idx;
		}
		else if(posDistSQR < radiusSQR){
			entArray[j++] = e_idx;
		}
	}
	
	entCount = j;
	
	if(!foundSource)
		entArray[entCount++] = source.entity->owner;
		
	if(!foundTarget && target.entity != source.entity)
		entArray[entCount++] = target.entity->owner;
		
	// Start performance monitor stuff.

	PERFINFO_RUN(
		switch(ENTTYPE(source.entity)){
			xcase ENTTYPE_CRITTER:	PERFINFO_AUTO_START("src:critter", 1);
			xcase ENTTYPE_PLAYER:	PERFINFO_AUTO_START("src:player", 1);
			xcase ENTTYPE_NPC:		PERFINFO_AUTO_START("src:npc", 1);
			xdefault:				PERFINFO_AUTO_START("src:other", 1);
		}
	
		switch(params.notifyType){
			xcase AI_EVENT_NOTIFY_POWER_SUCCEEDED:
				switch(powerSucceeded.attribType){
					xcase ATTRIB_UNUSED:		PERFINFO_AUTO_START("powerSucceed.unused", 1);
					xcase ATTRIB_HITPOINTS:		PERFINFO_AUTO_START("powerSucceed.hitpoints", 1);
					xcase ATTRIB_IMMOBILIZE:	PERFINFO_AUTO_START("powerSucceed.immobilize", 1);
					xcase ATTRIB_FEAR:			PERFINFO_AUTO_START("powerSucceed.fear", 1);
					xcase ATTRIB_TAUNT:			PERFINFO_AUTO_START("powerSucceed.taunt", 1);
					xcase ATTRIB_PUSH:			PERFINFO_AUTO_START("powerSucceed.push", 1);
					xcase ATTRIB_AVOID:			PERFINFO_AUTO_START("powerSucceed.avoid", 1);
					xcase ATTRIB_PLACATE:		PERFINFO_AUTO_START("powerSucceed.placate", 1);
					xdefault:					PERFINFO_AUTO_START("powerSucceed.unknown", 1);
				}
			xcase AI_EVENT_NOTIFY_POWER_FAILED:	PERFINFO_AUTO_START("powerFailed", 1);
			xcase AI_EVENT_NOTIFY_POWER_ENDED:	PERFINFO_AUTO_START("powerEnded", 1);
			xcase AI_EVENT_NOTIFY_ENTITY_KILLED:PERFINFO_AUTO_START("entKilled", 1);
			xcase AI_EVENT_NOTIFY_PET_CREATED:	PERFINFO_AUTO_START("petCreated", 1);
			xdefault:							PERFINFO_AUTO_START("unknownEvent", 1);
		}

		switch(ENTTYPE(target.entity)){
			xcase ENTTYPE_CRITTER:	PERFINFO_AUTO_START("dst:critter", 1);
			xcase ENTTYPE_PLAYER:	PERFINFO_AUTO_START("dst:player", 1);
			xcase ENTTYPE_NPC:		PERFINFO_AUTO_START("dst:npc", 1);
			xdefault:				PERFINFO_AUTO_START("dst:other", 1);
		}
	);
	
	// Notify everyone in the array.
	
	for(i = 0; i < entCount; i++){
		int e_idx = entArray[i];
		Entity* e = entities[e_idx];
		AIVars* ai = ENTAI_BY_ID(e_idx);
		
		if(!ai)
			continue;

		switch(params.notifyType){
			xcase AI_EVENT_NOTIFY_POWER_SUCCEEDED:
				aiEventNotifyPowerSucceeded(e, ai);
			xcase AI_EVENT_NOTIFY_POWER_FAILED:
				aiEventNotifyPowerFailed(e, ai);
			xcase AI_EVENT_NOTIFY_POWER_ENDED:
				aiEventNotifyPowerEnded(e, ai);
			xcase AI_EVENT_NOTIFY_ENTITY_KILLED:
				aiEventNotifyEntityKilled(e, ai);
		}
	}
	
	// Stop performance monitor stuff.

	PERFINFO_RUN(
		// Stop target EntType.
		
		PERFINFO_AUTO_STOP();
		
		// Stop notifyType.
		
		PERFINFO_AUTO_STOP();
		
		// Stop source EntType.
		
		PERFINFO_AUTO_STOP();	
	);

	// Stop master notify timer. 
	PERFINFO_AUTO_STOP();
}


