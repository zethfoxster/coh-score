#include "aiBehaviorInterface.h"
#include "entaiBehaviorStruct.h"
#include "entaiBehaviorCoh.h"

#include "arenamap.h"
#include "beaconPath.h"
#include "beaconPrivate.h"
#include "character_base.h"
#include "cmdcommon.h"
#include "cmdserver.h"
#include "earray.h"
#include "entaiCritterPrivate.h"
#include "entaiLog.h"
#include "entaiprivate.h"
#include "entGameActions.h"
#include "entity.h"
#include "EString.h"
#include "mathutil.h"
#include "pet.h"
#include "seq.h"
#include "seqstate.h"
#include "textparser.h"
#include "character_animfx.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// Pet
//////////////////////////////////////////////////////////////////////////
StaticDefineInt PetStateStrings[] = 
{
	DEFINE_INT
	{ "Attack (No Leash)",	PET_STATE_ATTACK_FREELY	},
	{ "Attack (Leash)",		PET_STATE_ATTACK_LEASH	},
	{ "Be In Area",			PET_STATE_BE_IN_AREA	},
	{ "Follow",				PET_STATE_FOLLOW		},
	{ "None",				PET_STATE_NONE			},
	{ "Resummon",			PET_STATE_RESUMMON		},
	{ "Use Power",			PET_STATE_USE_POWER		},
	DEFINE_END
};

StaticDefineInt PetActionStrings[] =
{
	DEFINE_INT
	{ "Attack",				kPetAction_Attack		},
	{ "Dismiss",			kPetAction_Dismiss		},
	{ "Follow",				kPetAction_Follow		},
	{ "Goto",				kPetAction_Goto			},
	{ "Stay",				kPetAction_Stay			},
	{ "Use Power",			kPetAction_Special		},
	DEFINE_END
};

StaticDefineInt PetStanceStrings[] =
{
	DEFINE_INT
	{ "Aggressive",			kPetStance_Aggressive	},
	{ "Defensive",			kPetStance_Defensive	},
	{ "Passive",			kPetStance_Passive		},
	DEFINE_END
};

#define CHANGE_STATE(newstate)				\
{											\
	mydata->oldState = mydata->state;		\
	mydata->state = newstate;				\
	AI_LOG(AI_LOG_SPECIAL, (e, "Changed to state %s from %s\n",				\
			StaticDefineIntRevLookup(PetStateStrings, mydata->state),		\
			StaticDefineIntRevLookup(PetStateStrings, mydata->oldState)));	\
}

#define REVERT_STATE()						\
{											\
	mydata->state = mydata->oldState;		\
	mydata->oldState = PET_STATE_NONE;		\
	AI_LOG(AI_LOG_SPECIAL, (e, "Changed to state %s from %s\n",				\
			StaticDefineIntRevLookup(PetStateStrings, mydata->state),		\
			StaticDefineIntRevLookup(PetStateStrings, mydata->oldState)));	\
}

#define CHANGE_ACTION(newaction)			\
{											\
	mydata->action = newaction;				\
	petUpdateClientUI(e, newaction, mydata->stance);\
}

#define PET_DEFENSIVE_TIME		10
#define PET_MINIMUM_LISTEN_TIME	10
#define PET_FOLLOW_RADIUS		15
#define PET_LEASH_DISTANCE		100
#define PET_RESUMMON_DISTANCE	500

void aiBFPetDebugStr(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, char** estr, char* indent)
{
	AIBDPet* mydata = (AIBDPet*) behavior->data;
	estrConcatf(estr, "%sState: ^8%s^0(^sprev:^s^8%s^n^0), Action: ^8%s^0, Stance: ^8%s", indent,
		StaticDefineIntRevLookup(PetStateStrings, mydata->state),
		StaticDefineIntRevLookup(PetStateStrings, mydata->oldState),
		StaticDefineIntRevLookup(PetActionStrings, mydata->action),
		StaticDefineIntRevLookup(PetStanceStrings, mydata->stance));
}

void aiBFPetFinish(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
{
	AIBDPet* mydata = (AIBDPet*) behavior->data;

	estrDestroy(&mydata->specialPetPower);
}

int aiBFPetString(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, TokenizerFunctionCall* param)
{
	AIVars* ai = ENTAI(e);
	char* cmdStr = param->function + 6;
	bool valid_command = false;
	AIBDPet* mydata = (AIBDPet*) behavior->data;
	EntityRef prevTarget = 0;

	if(!e || !erGetEnt(e->erOwner))
		return 1;

	if(!strnicmp("Action", param->function, 5))
	{
		if(!stricmp("Attack", cmdStr))
		{
			AIProxEntStatus* status;
			Entity* target;

			if(eaSize(&param->params) != 1)
				return 0;

			if(mydata->attackTarget)
				prevTarget = mydata->attackTarget;

			sscanf(param->params[0]->function, "%I64x", &mydata->attackTarget);
			target = erGetEnt(mydata->attackTarget);

			// doing this to make sure we don't add stuff (specifically NPCs) to our
			// status table (just in case someone assumes a status->entity to have a pchar)
			if(target && target->pchar)
				status = aiGetProxEntStatus(ai->proxEntStatusTable, target, 1);

			if(!target || !target->pchar || !entAlive(target) ||
				status && ABS_TIME_SINCE(status->placate.begin) < status->placate.duration)
			{
				mydata->attackTarget = 0;
				petUpdateClientUI(e, mydata->action, mydata->stance);
				return 1;
			}


			if(prevTarget && prevTarget != mydata->attackTarget)
			{
				AIProxEntStatus* prevStatus = aiGetProxEntStatus(ai->proxEntStatusTable, erGetEnt(prevTarget), 0);

				if(prevStatus)
				{
					prevStatus->neverForgetMe = 0;
					prevStatus->primaryTarget = 0;
				}
			}

			mydata->action = kPetAction_Attack;
			CHANGE_STATE(PET_STATE_ATTACK_FREELY);
			aiSetAttackTarget(e, target, status, "Got ActionAttack command", 0, 0);
			status->neverForgetMe = 1;
			status->primaryTarget = 1;
			status->time.posKnown = ABS_TIME;
			ai->doNotChangeAttackTarget = 1;
			ai->spawnPosIsSelf = 1;
			ai->alwaysAttack = 1;

			valid_command = true;
		}
		else if(!stricmp("Dismiss", cmdStr))
		{ 
			SETB(e->seq->sticky_state, STATE_DISMISS);
			dieNow(e, kDeathBy_HitPoints, false, e->seq->type->ticksToLingerAfterDeath);
			return 1;
		}
		else if(!stricmp("Follow", cmdStr))
		{
			mydata->action = kPetAction_Follow;
			CHANGE_STATE(PET_STATE_FOLLOW);

			valid_command = true;
		}
		else if(!stricmp("Stay", cmdStr))
		{
			mydata->action = kPetAction_Stay;
			CHANGE_STATE(PET_STATE_BE_IN_AREA);
			ai->pet.useStayPos = 1;
			copyVec3(ENTPOS(e), mydata->gotoPos);
			copyVec3(ENTPOS(e), ai->pet.stayPos);

			aiDiscardFollowTarget(e, "Being told to stay.", false);

			valid_command = true;
		}
		else if(!stricmp("Goto", cmdStr))
		{
			if(eaSize(&param->params) != 3)
				return 0;

			mydata->action = kPetAction_Goto;
			CHANGE_STATE(PET_STATE_BE_IN_AREA);
			mydata->gotoPos[0] = atof(param->params[0]->function);
			mydata->gotoPos[1] = atof(param->params[1]->function);
			mydata->gotoPos[2] = atof(param->params[2]->function);
			ai->pet.useStayPos = 1;
			copyVec3(mydata->gotoPos, ai->pet.stayPos);

			aiSetFollowTargetPosition(e, ai, mydata->gotoPos, AI_FOLLOWTARGETREASON_BEHAVIOR);
			ai->followTarget.standoffDistance = 1;
			ai->preventShortcut = 0; // this is necessary to allow it to shortcut to its target if it's
									 // within 15 ft
			aiBFMoveToPosDo(e, ai->base, mydata->gotoPos, PET_FOLLOW_RADIUS);

			valid_command = true;
		}
		else if(!stricmp("UsePower", cmdStr))
		{
			if(eaSize(&param->params) != 1)
				return 0;

			mydata->action = kPetAction_Special;
			CHANGE_STATE(PET_STATE_USE_POWER);
			estrPrintCharString(&mydata->specialPetPower, param->params[0]->function);
			ai->wasToldToUseSpecialPetPower = 1;
			aiCritterSetCurrentPowerByName( e, mydata->specialPetPower );
			mydata->specialPowerCount = ai->power.current->usedCount;

			valid_command = true;
		}
	}
	else if(!strnicmp("Stance", param->function, 6))
	{
		if(!stricmp("Aggressive", cmdStr))
		{
			mydata->stance = kPetStance_Aggressive;
			petUpdateClientUI(e, mydata->action, mydata->stance);
			return 1;
		}
		else if(!stricmp("Defensive", cmdStr))
		{
			mydata->stance = kPetStance_Defensive;
			petUpdateClientUI(e, mydata->action, mydata->stance);
			return 1;
		}
		else if(!stricmp("Passive", cmdStr))
		{
			mydata->stance = kPetStance_Passive;
			petUpdateClientUI(e, mydata->action, mydata->stance);
			return 1;
		}
	}
	else if(!strnicmp("NotAggressive", param->function, 13))
	{
		if(mydata->stance == kPetStance_Aggressive)
		{
			mydata->stance = kPetStance_Defensive;
			petUpdateClientUI(e, mydata->action, mydata->stance);
		}
		return 1;
	}

	if(valid_command)
	{
		mydata->lastCommandTime = ABS_TIME;
		petUpdateClientUI(e, mydata->action, mydata->stance);

		if(mydata->action != kPetAction_Attack && mydata->attackTarget)
		{
			AIProxEntStatus* status =
				aiGetProxEntStatus(ai->proxEntStatusTable, erGetEnt(mydata->attackTarget), 0);
			ai->doNotChangeAttackTarget = 0;
			ai->spawnPosIsSelf = 0;
			ai->alwaysAttack = 0;
			mydata->attackTarget = 0;

			if(status)
			{
				status->neverForgetMe = 0;
				status->primaryTarget = 0;
			}
		}
		if(mydata->action != kPetAction_Stay && mydata->action != kPetAction_Goto)
			ai->pet.useStayPos = 0;
		if(mydata->action != kPetAction_Special)
			ai->wasToldToUseSpecialPetPower = 0;

		return 1;
	}

	if(!stricmp(param->function, "NoTeleport"))
	{
		mydata->noTeleport = 1;
		return 1;
	}

	if(!stricmp(param->function, "FollowDistance") && eaSize(&param->params))
	{
		mydata->followDistance = aiBehaviorParseFloat(param->params[0]);
		return 1;
	}

	return 0;
}

Entity* aiBFPetCheckOverride(Entity* e, AIVars* ai, AIBehavior* behavior)
{
	AIBDPet* mydata = (AIBDPet*)behavior->data;

	if(mydata->stance == kPetStance_Aggressive && ai->attackTarget.entity)
	{
		AI_LOG(AI_LOG_SPECIAL, (e, "Had an attacktarget (%s:%d) and was aggressive\n",
			ai->attackTarget.entity->name, ai->attackTarget.entity->owner));
		return ai->attackTarget.entity;
	}

	if((mydata->stance == kPetStance_Defensive || mydata->stance == kPetStance_Aggressive) &&
		ABS_TIME_SINCE(ai->time.friendWasAttacked) < SEC_TO_ABS(PET_DEFENSIVE_TIME))
	{
		AIProxEntStatus* status;
		Entity* bestTarget = NULL;
		F32 bestScore = -FLT_MAX;
		static char* checkedEntityIndices = NULL;

		if(AI_LOG_ENABLED(AI_LOG_SPECIAL))
		{
			if(!checkedEntityIndices)
				estrCreate(&checkedEntityIndices);
			estrClear(&checkedEntityIndices);
		}

		// scan through the team table for anyone who attacked me or my teammates
		for(status = ai->proxEntStatusTable->statusHead; status; status = status->tableList.next)
		{
			if((ABS_TIME_SINCE(status->time.attackMe) < SEC_TO_ABS(PET_DEFENSIVE_TIME) ||
				ABS_TIME_SINCE(status->time.powerUsedOnMyFriend) < SEC_TO_ABS(PET_DEFENSIVE_TIME)) &&
				status->entity && status->entity->pchar->attrCur.fHitPoints > 0 &&
				ABS_TIME_SINCE(status->placate.begin) > status->placate.duration)
			{
				if(status->dangerValue > bestScore)
				{
					bestScore = status->dangerValue;
					bestTarget = status->entity;
				}
				if(AI_LOG_ENABLED(AI_LOG_SPECIAL))
				{
					estrConcatf(&checkedEntityIndices, "%d ", status->entity->owner);
				}
			}
		}
		if(bestTarget)
			ai->magicallyKnowTargetPos = 1;

		AI_LOG(AI_LOG_SPECIAL, (e, "Evaluated %s and ended up with %s\n",
			checkedEntityIndices, bestTarget ? bestTarget->name : "no one"));

		return bestTarget;
	}

	if(AI_LOG_ENABLED(AI_LOG_SPECIAL) && !(ABS_TO_SEC(2 * behavior->timeRun) % 5))
		AI_LOG(AI_LOG_SPECIAL, (e, "Had no reason to go through my status table\n"));
	return NULL;
}

#define CHECK_OVERRIDES_AND_BUFF														\
{																						\
	if(mydata->state != PET_STATE_FOLLOW ||												\
		ABS_TIME_SINCE(mydata->lastCommandTime) > SEC_TO_ABS(PET_MINIMUM_LISTEN_TIME))	\
	{																					\
		if(aiBFPetCheckOverride(e, ai, behavior))										\
		{																				\
			CHANGE_STATE(mydata->stance == kPetStance_Aggressive ?						\
				PET_STATE_ATTACK_FREELY : PET_STATE_ATTACK_LEASH);						\
			keepRunning = true;															\
			continue;																	\
		}																				\
		else if(mydata->stance != kPetStance_Passive)									\
			aiCritterDoDefaultBuffs(e, ai);												\
	}																					\
}

void aiBFPetRun(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* behavior)
{
	AIVars* ai = ENTAI(e);
	AIBDPet* mydata = (AIBDPet*)behavior->data;
	Entity* owner = erGetEnt(e->erOwner);
	bool keepRunning = true;
	F32 distanceFromOwnerSQR = 0;

	if(!owner)
	{
		//behavior->finished = 1; 
		return;
	}

	if(mydata->stance == kPetStance_Defensive && mydata->action != kPetAction_Attack)
		ai->amBodyGuard = 1;
	else
		ai->amBodyGuard = 0;

	if(ai->canFly)
		distanceFromOwnerSQR = distance3Squared(ENTPOS(e), ENTPOS(owner));
	else
		distanceFromOwnerSQR = distance3SquaredXZ(ENTPOS(e), ENTPOS(owner));

	if(!mydata->noTeleport && mydata->state != PET_STATE_RESUMMON &&
		distanceFromOwnerSQR > SQR(PET_RESUMMON_DISTANCE))
	{
		CHANGE_STATE(PET_STATE_RESUMMON);
		SETB( e->seq->sticky_state, STATE_DISMISS );
		return;
	}

	while(keepRunning)
	{
		keepRunning = false;
		switch(mydata->state)
		{
		case PET_STATE_ATTACK_LEASH:
			if(distanceFromOwnerSQR > SQR(PET_LEASH_DISTANCE))
			{
				CHANGE_STATE(PET_STATE_FOLLOW);
				keepRunning = true;
				continue;
			}
			// deliberately no break here
		case PET_STATE_ATTACK_FREELY:
			{
				Entity* attacktarget = NULL;
				AIProxEntStatus* status = NULL;

				if(mydata->attackTarget)
				{
					attacktarget = erGetEnt(mydata->attackTarget);
					status = aiGetProxEntStatus(ai->proxEntStatusTable, attacktarget, 0);

					if(status && ABS_TIME_SINCE(status->placate.begin) < status->placate.duration)
					{
						status->neverForgetMe = 0;
						status->primaryTarget = 0;
						attacktarget = NULL;
					}
					
					// check the hit points in addition to entAlive to catch the case where this is right before
					// the tick where they get registered as being dead // TODO: verify this is true :)
					if(!attacktarget || !entAlive(attacktarget) || attacktarget->pchar->attrCur.fHitPoints <= 0)
					{
						mydata->attackTarget = 0;
						ai->doNotChangeAttackTarget = 0;
						aiDiscardAttackTarget(e, "Killed my assigned target");
						aiCritterCheckProximity(e, 1);
						attacktarget = NULL;
					}
				}
//				if(mydata->attackTarget && (!(attacktarget = erGetEnt(mydata->attackTarget)) || attacktarget->pchar->attrCur.fHitPoints <= 0))
//				{
//					mydata->attackTarget = 0;
//					ai->doNotChangeAttackTarget = 0;
//					aiDiscardAttackTarget(e, "Killed my assigned target");
//					aiCritterCheckProximity(e, 1);
//					attacktarget = NULL;
//				}

				if(!attacktarget)
					attacktarget = aiBFPetCheckOverride(e, ai, behavior);

				if(attacktarget)
				{
					if(!ai->attackTarget.entity && !attacktarget->untargetable)
						aiSetAttackTarget(e, attacktarget, NULL, "Pet.AttackFreely", 1, 0);
					aiCritterActivityCombat(e, ai);
					return;
				}
				else
				{
					if(mydata->action == kPetAction_Stay || mydata->action == kPetAction_Goto)
					{
						CHANGE_STATE(PET_STATE_BE_IN_AREA);
					}
					else
					{
						CHANGE_ACTION(kPetAction_Follow);
						CHANGE_STATE(PET_STATE_FOLLOW);
					}

					// transition out of combat stance
					if(!ai->doNotChangePowers)
					{
						aiDiscardCurrentPower(e);
					}
					CLRB(e->seq->sticky_state, STATE_COMBAT);

					keepRunning = true;
					continue;
				}
			}
		xcase PET_STATE_FOLLOW:
			{
				float followRadius = mydata->followDistance > 0 ? mydata->followDistance : PET_FOLLOW_RADIUS;
				if(distanceFromOwnerSQR > SQR(followRadius))
				{
					if(!ai->navSearch->path.head && ai->motion.type == AI_MOTIONTYPE_NONE)
					{
						ai->followTarget.standoffDistance = followRadius;
						aiSetFollowTargetEntity(e, ai, owner, AI_FOLLOWTARGETREASON_LEADER);
						aiConstructPathToTarget(e);
						aiStartPath(e, 0, ai->doNotRun);
					}
				}
				else
				{
					CHECK_OVERRIDES_AND_BUFF;
				}
			}
		xcase PET_STATE_BE_IN_AREA:
			if(distance3Squared(ENTPOS(e), mydata->gotoPos) > SQR(PET_FOLLOW_RADIUS))
			{
				aiSetFollowTargetPosition(e, ai, mydata->gotoPos, AI_FOLLOWTARGETREASON_BEHAVIOR);
				ai->followTarget.standoffDistance = 1;
				aiBFMoveToPosDo(e, ai->base, mydata->gotoPos, PET_FOLLOW_RADIUS);
			}
			else
			{
				CHECK_OVERRIDES_AND_BUFF;
			}
		xcase PET_STATE_RESUMMON:
			if(stricmp(e->seq->animation.move->name, "dismiss"))
			{
				U32 spawnLoc[2] = {2, 0};
				int result;
				entUpdatePosInstantaneous(e, ENTPOS(owner));
				RotateSpawn(e, spawnLoc, &result);
				CLRB(e->seq->sticky_state, STATE_DISMISS);
//				SETB( e->seq->state, STATE_SPAWNEDASPET );
//				CHANGE_STATE(PET_STATE_FOLLOW);

				if(!result)
				{
					//zdrop
					Vec3 newpos;
					F32 dy;
					dy = beaconGetDistanceYToCollision(ENTPOS(e), -1000);
					copyVec3(ENTPOS(e), newpos);
					vecY(newpos) -= dy;
					entUpdatePosInstantaneous(e, newpos);
				}
				SETB( e->seq->state, STATE_SPAWNEDASPET );
				CHANGE_STATE(PET_STATE_FOLLOW);
			}
		xcase PET_STATE_USE_POWER:
			if(ai->power.current->usedCount > mydata->specialPowerCount)
			{
				mydata->state = mydata->oldState;
				keepRunning = true;
				continue;
			}
			else
				aiCritterActivityCombat(e, ai);
		}
	}
}

void aiBFPetSet(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
{
	aiSetMessageHandler(e, aiCritterMsgHandlerCombat);
}

void aiBFPetStart(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
{
	AIBDPet* mydata = (AIBDPet*) behavior->data;
	AIVars* ai = ENTAI(e);

	behavior->overrideExempt = 1;
	mydata->state = PET_STATE_FOLLOW;
	mydata->action = kPetAction_Follow;
	if (mydata->stance == kPetStance_Unknown)
		mydata->stance = kPetStance_Defensive;
	ai->stickyTarget = 1;
}

void aiBFPetUnset(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
{
	AIVars* ai = ENTAI(e);
	AIBDPet* mydata = (AIBDPet*) behavior->data;
	Entity* attackTarget = erGetEnt(mydata->attackTarget);

	if(attackTarget)
	{
		AIProxEntStatus* status = aiGetProxEntStatus(ai->proxEntStatusTable, attackTarget, 0);

		if(status)
		{
			status->neverForgetMe = 0;
			status->primaryTarget = 0;
		}
	}

	ai->stickyTarget = 0;
	aiDiscardAttackTarget(e, "Unsetting pet behavior");
}
